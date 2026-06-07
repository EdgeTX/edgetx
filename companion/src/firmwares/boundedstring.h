/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#pragma once

#include <string>
#include <string_view>

#include <QString>

// A self-limiting string for Companion's data-model fields.
//
// N is the maximum number of bytes the value may hold, EXCLUDING the implicit
// NUL of the legacy `char[]` fields it replaces. So a legacy `char foo[16+1]`
// (16 usable chars) maps to `BoundedString<16>`.
//
// Assignment always truncates to N bytes, mirroring the byte-level truncation
// the YAML char-array reader does today (yaml_ops.h: `str.copy(value, N-1)`).
// Truncation is byte-based on purpose: it must match the radio firmware (and the
// legacy code path) exactly, including the case where a multi-byte UTF-8
// sequence is split at the boundary. Do not "improve" it to be codepoint-aware.
template <size_t N>
class BoundedString
{
  static_assert(N > 0, "BoundedString capacity must be greater than zero");

  std::string s_;

 public:
  static constexpr size_t capacity() { return N; }

  BoundedString() = default;
  BoundedString(const BoundedString&) = default;
  BoundedString& operator=(const BoundedString&) = default;

  BoundedString(std::string_view v) { assign(v); }
  BoundedString(const char* v) { assign(v ? std::string_view(v) : std::string_view()); }
  BoundedString(const QString& v) { assign(v); }

  BoundedString& operator=(std::string_view v) { assign(v); return *this; }
  BoundedString& operator=(const char* v) { assign(v ? std::string_view(v) : std::string_view()); return *this; }
  BoundedString& operator=(const QString& v) { assign(v); return *this; }

  void assign(std::string_view v) { s_.assign(v.substr(0, N)); }
  // NOTE: QString is serialised as UTF-8 here, matching the YAML layer. Some
  // legacy UI call sites stored Latin-1 via QString::toLatin1(); Phase 1 must
  // confirm the encoding at each migrated boundary before relying on this.
  void assign(const QString& v) { assign(v.toStdString()); }

  void clear() { s_.clear(); }
  bool empty() const { return s_.empty(); }
  size_t size() const { return s_.size(); }
  size_t length() const { return s_.size(); }

  const std::string& str() const { return s_; }
  const char* c_str() const { return s_.c_str(); }
  QString toQString() const { return QString::fromStdString(s_); }

  // Implicit conversion so a migrated field still drops into APIs taking a
  // std::string (and, via QString's std::string ctor, much of the Qt UI).
  operator const std::string&() const { return s_; }

  // Explicit comparison overloads (std::string's own operator== is a
  // non-member template, so the implicit conversion above can't drive it).
  // The const char* overload binds legacy char[]/literals via array-to-pointer
  // (a standard conversion), so it wins cleanly over the others rather than
  // tying with them and producing an ambiguity.
  bool operator==(const BoundedString& o) const { return s_ == o.s_; }
  bool operator!=(const BoundedString& o) const { return s_ != o.s_; }
  bool operator==(const std::string& o) const { return s_ == o; }
  bool operator!=(const std::string& o) const { return s_ != o; }
  bool operator==(const char* o) const { return o ? s_ == o : s_.empty(); }
  bool operator!=(const char* o) const { return o ? s_ != o : !s_.empty(); }
};
