/*
 * Float-only strtof for EdgeTX: correctly rounded, no double arithmetic.
 *
 * Replaces newlib's strtof (which internally calls strtod + d2f) to avoid
 * pulling in newlib's double-precision Bignum code and software FP emulation.
 *
 * The decimal (or hex) digits are read into a small fixed-size integer
 * bignum, and the result is produced by one exact integer division, so the
 * returned float is the nearest one to the input (ties to even), for any
 * input length. No floating-point arithmetic is used at all.
 *
 * Up to MAX_DEC_DIGITS significant decimal digits are kept; further nonzero
 * digits only set a "sticky" flag. That is still exact: every float, and
 * every point halfway between two floats, has at most 112 significant
 * decimal digits, so digits beyond that can only decide "above or below a
 * halfway point", which the sticky flag records.
 *
 * Handles: leading whitespace, sign, decimal point, e/E exponent, hex floats
 * (0x..., optional p/P exponent), inf/infinity, nan, overflow (HUGE_VALF,
 * ERANGE) and underflow (0 or subnormal, ERANGE when the result is 0).
 *
 * Copyright (C) EdgeTX - GPLv2
 */

#include <ctype.h>   /* isspace */
#include <errno.h>
#include <math.h>    /* HUGE_VALF */
#include <stddef.h>
#include <stdint.h>
#include <string.h>  /* memset, memcpy */

#define MAX_DEC_DIGITS 120   /* significant decimal digits kept exactly */
#define MAX_HEX_DIGITS 32    /* significant hex digits kept exactly */

/* 20 x 32 = 640 bits. Worst cases: 120 digits (399 bits) times 10^38, or
 * 10^165 shifted against a numerator scaled by up to 2^150, plus 27 bits of
 * quotient headroom. */
#define BIG_LIMBS 20

typedef struct {
  uint32_t w[BIG_LIMBS];
} big_t;

static void big_set(big_t* a, uint32_t v)
{
  memset(a, 0, sizeof(*a));
  a->w[0] = v;
}

/* a = a * m + add */
static void big_mul_add(big_t* a, uint32_t m, uint32_t add)
{
  uint64_t carry = add;
  for (int i = 0; i < BIG_LIMBS; i++) {
    uint64_t t = (uint64_t)a->w[i] * m + carry;
    a->w[i] = (uint32_t)t;
    carry = t >> 32;
  }
}

static int big_bitlen(const big_t* a)
{
  for (int i = BIG_LIMBS - 1; i >= 0; i--) {
    if (a->w[i]) return i * 32 + 32 - __builtin_clz(a->w[i]);
  }
  return 0;
}

static void big_shl(big_t* a, int s)
{
  int ws = s >> 5, bs = s & 31;
  for (int i = BIG_LIMBS - 1; i >= 0; i--) {
    uint32_t hi = (i - ws >= 0) ? a->w[i - ws] : 0;
    uint32_t lo = (i - ws - 1 >= 0) ? a->w[i - ws - 1] : 0;
    a->w[i] = bs ? (hi << bs) | (lo >> (32 - bs)) : hi;
  }
}

static int big_cmp(const big_t* a, const big_t* b)
{
  for (int i = BIG_LIMBS - 1; i >= 0; i--) {
    if (a->w[i] != b->w[i]) return a->w[i] > b->w[i] ? 1 : -1;
  }
  return 0;
}

/* a -= b, requires a >= b */
static void big_sub(big_t* a, const big_t* b)
{
  uint32_t borrow = 0;
  for (int i = 0; i < BIG_LIMBS; i++) {
    uint64_t t = (uint64_t)a->w[i] - b->w[i] - borrow;
    a->w[i] = (uint32_t)t;
    borrow = (uint32_t)(t >> 63);
  }
}

static float float_from_bits(uint32_t bits)
{
  float f;
  memcpy(&f, &bits, sizeof(f));
  return f;
}

static float overflow(void)
{
  errno = ERANGE;
  return HUGE_VALF;
}

/*
 * The float nearest to num / den * 2^p2, rounding ties to even. When
 * 'sticky' is set the true value is slightly larger than num / den * 2^p2
 * (digits were dropped), which only matters for exact halfway cases.
 * num must be nonzero; num and den are clobbered.
 */
static float big_to_float(big_t* num, big_t* den, int p2, int sticky)
{
  int span = big_bitlen(num) - big_bitlen(den);

  /* num/den lies in (2^(span-1), 2^(span+1)) */
  if (span - 1 + p2 >= 128) return overflow();

  /* Scale so the quotient has 25 or 26 bits: 24 mantissa bits plus a
   * rounding bit. Below the normal range keep the rounding bit at 2^-150. */
  int b = span - 25;
  if (b + p2 < -150) b = -150 - p2;
  if (b > 0) big_shl(den, b);
  else if (b < 0) big_shl(num, -b);

  uint32_t q = 0;
  big_t t;
  for (int i = 26; i >= 0; i--) {
    t = *den;
    big_shl(&t, i);
    if (big_cmp(num, &t) >= 0) {
      big_sub(num, &t);
      q |= 1u << i;
    }
  }
  if (big_bitlen(num)) sticky = 1;

  int e = b + p2;  /* weight of q's lowest bit */
  while (q >= (1u << 25)) {
    sticky |= q & 1;
    q >>= 1;
    e++;
  }

  uint32_t m = q >> 1;
  if ((q & 1) && (sticky || (m & 1))) m++;
  e++;  /* weight of m's lowest bit */
  if (m >= (1u << 24)) {  /* rounding carried into a new power of two */
    m >>= 1;
    e++;
  }

  if (m == 0) {
    errno = ERANGE;
    return 0.0f;
  }
  if (m < (1u << 23)) return float_from_bits(m);  /* subnormal, e == -149 */

  int biased = e + 23 + 127;
  if (biased >= 255) return overflow();
  return float_from_bits(((uint32_t)biased << 23) | (m & 0x7FFFFFu));
}

static int match_word(const char* s, const char* word)
{
  for (; *word; s++, word++) {
    if (tolower((unsigned char)*s) != *word) return 0;
  }
  return 1;
}

/* Reads [+-]digits after an exponent letter. Returns the position after the
 * last digit, or NULL when no digit follows (the letter is then not part of
 * the number). The value saturates, which keeps huge exponents from
 * overflowing an int. */
static const char* parse_exponent(const char* s, int* value)
{
  int neg = 0, v = 0;
  if (*s == '+' || *s == '-') neg = (*s++ == '-');
  if (*s < '0' || *s > '9') return NULL;
  while (*s >= '0' && *s <= '9') {
    if (v < 100000) v = v * 10 + (*s - '0');
    s++;
  }
  *value = neg ? -v : v;
  return s;
}

static int hex_value(char c)
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

static float parse_hex(const char* zero, char** endptr)
{
  /* zero points at the '0' of "0x" */
  const char* s = zero + 2;
  big_t num, den;
  big_set(&num, 0);
  int ndig = 0, p2 = 0, sticky = 0, any = 0, dot = 0;

  for (;; s++) {
    if (*s == '.') {
      if (dot) break;
      dot = 1;
      continue;
    }
    int v = hex_value(*s);
    if (v < 0) break;
    any = 1;
    if (ndig == 0 && v == 0) {
      if (dot) p2 -= 4;
    } else if (ndig < MAX_HEX_DIGITS) {
      big_mul_add(&num, 16, (uint32_t)v);
      ndig++;
      if (dot) p2 -= 4;
    } else {
      if (v) sticky = 1;
      if (!dot) p2 += 4;
    }
  }

  if (!any) {
    /* "0x" without hex digits: only the "0" is a number */
    if (endptr) *endptr = (char*)(zero + 1);
    return 0.0f;
  }

  if (*s == 'p' || *s == 'P') {
    int e;
    const char* end = parse_exponent(s + 1, &e);
    if (end) {
      s = end;
      p2 += e;
    }
  }
  if (endptr) *endptr = (char*)s;
  if (ndig == 0) return 0.0f;

  int lead = big_bitlen(&num) - 1 + p2;  /* exponent of the leading bit */
  if (lead > 127) return overflow();
  if (lead < -150) {
    errno = ERANGE;
    return 0.0f;
  }
  big_set(&den, 1);
  return big_to_float(&num, &den, p2, sticky);
}

static float parse_decimal(const char* start, const char* nptr, char** endptr)
{
  const char* s = start;
  big_t num, den;
  big_set(&num, 0);
  int ndig = 0, exp10 = 0, sticky = 0, any = 0, dot = 0;

  for (;; s++) {
    if (*s == '.') {
      if (dot) break;
      dot = 1;
      continue;
    }
    if (*s < '0' || *s > '9') break;
    any = 1;
    if (ndig == 0 && *s == '0') {
      if (dot) exp10--;
    } else if (ndig < MAX_DEC_DIGITS) {
      big_mul_add(&num, 10, (uint32_t)(*s - '0'));
      ndig++;
      if (dot) exp10--;
    } else {
      if (*s != '0') sticky = 1;
      if (!dot) exp10++;
    }
  }

  if (!any) {
    if (endptr) *endptr = (char*)nptr;
    return 0.0f;
  }

  if (*s == 'e' || *s == 'E') {
    int e;
    const char* end = parse_exponent(s + 1, &e);
    if (end) {
      s = end;
      exp10 += e;
    }
  }
  if (endptr) *endptr = (char*)s;
  if (ndig == 0) return 0.0f;

  int lead = ndig - 1 + exp10;  /* decimal exponent of the leading digit */
  if (lead > 38) return overflow();
  if (lead < -46) {  /* below 1e-46, under half the smallest subnormal */
    errno = ERANGE;
    return 0.0f;
  }

  big_set(&den, 1);
  big_t* scaled = exp10 >= 0 ? &num : &den;
  int k = exp10 >= 0 ? exp10 : -exp10;
  for (; k >= 9; k -= 9) big_mul_add(scaled, 1000000000u, 0);
  for (; k > 0; k--) big_mul_add(scaled, 10, 0);

  return big_to_float(&num, &den, 0, sticky);
}

float __wrap_strtof(const char* nptr, char** endptr)
{
  const char* s = nptr;
  char* end;
  while (isspace((unsigned char)*s)) s++;

  int neg = 0;
  if (*s == '+' || *s == '-') neg = (*s++ == '-');

  float result;
  if (match_word(s, "inf")) {
    s += match_word(s, "infinity") ? 8 : 3;
    end = (char*)s;
    result = HUGE_VALF;
  } else if (match_word(s, "nan")) {
    s += 3;
    if (*s == '(') {  /* optional (n-char-sequence) */
      const char* p = s + 1;
      while (isalnum((unsigned char)*p) || *p == '_') p++;
      if (*p == ')') s = p + 1;
    }
    end = (char*)s;
    result = __builtin_nanf("");
  } else if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    result = parse_hex(s, &end);
  } else {
    result = parse_decimal(s, nptr, &end);
  }
  if (endptr) *endptr = end;
  if (end == nptr) return 0.0f;  /* no conversion: +0, even after '-' */
  return neg ? -result : result;
}

/* Also override strtod: any call to strtod goes through our strtof
 * (acceptable when lua_Number=float). */
double __wrap_strtod(const char* nptr, char** endptr)
{
  return (double)__wrap_strtof(nptr, endptr);
}
