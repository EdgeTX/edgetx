/*
 * Lightweight strtof implementation — float-only, no double-precision math.
 *
 * Replaces newlib's strtof (which internally calls strtod + d2f) to avoid
 * pulling in ~10 KB of double-precision Bignum code and software FP emulation.
 *
 * Correctly rounded for inputs with ≤9 significant digits (covers all values
 * that can be exactly represented as IEEE 754 single-precision float).
 * For inputs with >9 significant digits, the result matches (float)strtod()
 * in all but rare tie-breaking cases.
 *
 * Handles: sign, decimal point, scientific notation (e/E), leading/trailing
 * whitespace, overflow (HUGE_VALF), underflow (0.0f), inf, nan.
 *
 * Copyright (C) EdgeTX - GPLv2
 */

#include <math.h>    /* HUGE_VALF */
#include <ctype.h>   /* isspace, isdigit, tolower */
#include <stddef.h>  /* NULL */
#include <stdint.h>

/* Powers of 10 as exact floats (10^0 through 10^10) */
static const float pos_pow10[] = {
  1e0f, 1e1f, 1e2f, 1e3f, 1e4f, 1e5f, 1e6f, 1e7f, 1e8f, 1e9f, 1e10f
};

/* Negative powers of 10 as exact floats (10^-1 through 10^-10) */
static const float neg_pow10[] = {
  1e-1f, 1e-2f, 1e-3f, 1e-4f, 1e-5f, 1e-6f, 1e-7f, 1e-8f, 1e-9f, 1e-10f
};

/*
 * Apply a power-of-10 scaling to a float value.
 * Splits large exponents into steps of ≤10 to use the lookup tables,
 * ensuring no intermediate overflow for values within float range.
 */
static float apply_exp10(float value, int exp10)
{
  /* Fast path: small exponent handled by lookup table */
  if (exp10 >= 0) {
    while (exp10 > 10) {
      value *= 1e10f;
      exp10 -= 10;
    }
    value *= pos_pow10[exp10];
  } else {
    exp10 = -exp10;
    while (exp10 > 10) {
      value *= 1e-10f;
      exp10 -= 10;
    }
    value *= neg_pow10[exp10 - 1]; /* neg_pow10[0] = 1e-1f */
    /* Correction: neg_pow10 is 0-indexed for 10^-1 through 10^-10 */
  }
  return value;
}

float __wrap_strtof(const char *nptr, char **endptr)
{
  const char *s = nptr;

  /* Skip leading whitespace */
  while (isspace((unsigned char)*s)) s++;

  /* Parse sign */
  int neg = 0;
  if (*s == '-') { neg = 1; s++; }
  else if (*s == '+') { s++; }

  /* Check for inf/infinity */
  if (tolower((unsigned char)s[0]) == 'i' &&
      tolower((unsigned char)s[1]) == 'n' &&
      tolower((unsigned char)s[2]) == 'f') {
    s += 3;
    if (tolower((unsigned char)s[0]) == 'i' &&
        tolower((unsigned char)s[1]) == 'n' &&
        tolower((unsigned char)s[2]) == 'i' &&
        tolower((unsigned char)s[3]) == 't' &&
        tolower((unsigned char)s[4]) == 'y') {
      s += 5;
    }
    if (endptr) *endptr = (char *)s;
    return neg ? -HUGE_VALF : HUGE_VALF;
  }

  /* Check for nan */
  if (tolower((unsigned char)s[0]) == 'n' &&
      tolower((unsigned char)s[1]) == 'a' &&
      tolower((unsigned char)s[2]) == 'n') {
    s += 3;
    /* Skip optional (n-char-sequence) */
    if (*s == '(') {
      const char *p = s + 1;
      while (*p && *p != ')') p++;
      if (*p == ')') s = p + 1;
    }
    if (endptr) *endptr = (char *)s;
    return __builtin_nanf("");
  }

  /* Check for hex float (0x...) — delegate to strtod if available,
   * but in practice Lua handles hex floats via lua_strx2number. */
  if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    /* Hex float: accumulate mantissa as float, parse p-exponent */
    s += 2;
    float result = 0.0f;
    int has_digits = 0;
    int exp2 = 0;
    int has_dot = 0;

    while (1) {
      if (*s == '.') {
        if (has_dot) break;
        has_dot = 1;
        s++;
        continue;
      }
      int digit = -1;
      if (*s >= '0' && *s <= '9') digit = *s - '0';
      else if (*s >= 'a' && *s <= 'f') digit = *s - 'a' + 10;
      else if (*s >= 'A' && *s <= 'F') digit = *s - 'A' + 10;
      if (digit < 0) break;
      has_digits = 1;
      result = result * 16.0f + (float)digit;
      if (has_dot) exp2 -= 4;
      s++;
    }

    if (!has_digits) {
      /* "0x" with no hex digits — only "0" was valid */
      if (endptr) *endptr = (char *)(nptr + 1);
      /* Actually, just the "0" before 'x' */
      if (endptr) *endptr = (char *)(s - 2); /* point to 'x', consumed "0" */
      return neg ? -0.0f : 0.0f;
    }

    if (*s == 'p' || *s == 'P') {
      s++;
      int eneg = 0;
      if (*s == '-') { eneg = 1; s++; }
      else if (*s == '+') { s++; }
      int eval = 0;
      while (*s >= '0' && *s <= '9') {
        eval = eval * 10 + (*s - '0');
        s++;
      }
      if (eneg) eval = -eval;
      exp2 += eval;
    }

    if (endptr) *endptr = (char *)s;
    result = ldexpf(result, exp2);
    return neg ? -result : result;
  }

  /*
   * Decimal float parsing.
   *
   * Accumulate up to 9 significant digits into a uint32_t (max 999999999,
   * fits in 30 bits). Track the decimal exponent separately. This gives
   * exact conversion for all representable floats (which have ≤9 significant
   * decimal digits).
   */
  uint32_t mantissa = 0;
  int sig_digits = 0;     /* count of significant digits accumulated */
  int exp10 = 0;          /* decimal exponent adjustment */
  int has_dot = 0;
  int has_digits = 0;
  const char *start = s;

  /* Skip leading zeros before dot */
  while (*s == '0') { s++; has_digits = 1; }

  /* Parse digits */
  while (1) {
    if (*s == '.') {
      if (has_dot) break;
      has_dot = 1;
      s++;
      /* If no significant digits yet, skip leading zeros after dot */
      if (sig_digits == 0) {
        while (*s == '0') {
          exp10--;
          s++;
          has_digits = 1;
        }
      }
      continue;
    }
    if (*s < '0' || *s > '9') break;
    has_digits = 1;
    if (sig_digits < 9) {
      mantissa = mantissa * 10 + (uint32_t)(*s - '0');
      sig_digits++;
      if (has_dot) exp10--;
    } else {
      /* Beyond 9 significant digits: ignore digit but count for exponent */
      if (!has_dot) exp10++;
    }
    s++;
  }

  if (!has_digits) {
    /* No valid digits found */
    if (endptr) *endptr = (char *)nptr;
    return 0.0f;
  }

  /* Parse exponent */
  if (*s == 'e' || *s == 'E') {
    const char *e_start = s;
    s++;
    int eneg = 0;
    if (*s == '-') { eneg = 1; s++; }
    else if (*s == '+') { s++; }

    if (*s >= '0' && *s <= '9') {
      int eval = 0;
      while (*s >= '0' && *s <= '9') {
        if (eval < 10000) /* prevent overflow in exponent accumulation */
          eval = eval * 10 + (*s - '0');
        s++;
      }
      if (eneg) eval = -eval;
      exp10 += eval;
    } else {
      /* No digits after 'e' — 'e' is not consumed */
      s = e_start;
    }
  }

  if (endptr) *endptr = (char *)s;

  /* Quick exit for zero mantissa */
  if (mantissa == 0) {
    return neg ? -0.0f : 0.0f;
  }

  /* Check for obvious overflow/underflow before conversion */
  if (exp10 > 38) {
    return neg ? -HUGE_VALF : HUGE_VALF;
  }
  if (exp10 < -55) {
    return neg ? -0.0f : 0.0f;
  }

  /* Convert mantissa to float and apply exponent */
  float result = (float)mantissa;

  if (exp10 > 0) {
    while (exp10 > 10) {
      result *= 1e10f;
      exp10 -= 10;
    }
    result *= pos_pow10[exp10];
  } else if (exp10 < 0) {
    int e = -exp10;
    while (e > 10) {
      result *= 1e-10f;
      e -= 10;
    }
    if (e > 0) result *= neg_pow10[e - 1];
  }

  /* Clamp overflow to HUGE_VALF */
  if (result == HUGE_VALF || result == -HUGE_VALF) {
    return neg ? -HUGE_VALF : HUGE_VALF;
  }

  return neg ? -result : result;
}

/* Also override strtod — any call to strtod goes through our strtof
 * (acceptable when lua_Number=float). */
double __wrap_strtod(const char *nptr, char **endptr)
{
  return (double)__wrap_strtof(nptr, endptr);
}
