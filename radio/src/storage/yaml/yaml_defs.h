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

#if defined(YAML_GENERATOR)

/* private definitions */
#define _yaml_note(label) #label
#define _yaml_attribute(attr) __attribute__((annotate(attr)))

/* public definitions */
#define ENUM(label) _yaml_attribute("enum:" _yaml_note(label))
#define SKIP _yaml_attribute("skip:true")
#define USE_IDX _yaml_attribute("idx:true")
#define NO_IDX _yaml_attribute("idx:false")
#define FUNC(name) _yaml_attribute("func:" _yaml_note(name))
#define NAME(label) _yaml_attribute("name:" _yaml_note(label))

#define CUST(read, write)                   \
  _yaml_attribute("read:" _yaml_note(read)) \
      _yaml_attribute("write:" _yaml_note(write))

#define ARRAY(elmt_size, elmt_type, fcn)                         \
  _yaml_attribute("array:" _yaml_note(elmt_size) "|" _yaml_note( \
      elmt_type) "|" _yaml_note(fcn))

#define CUST_ATTR(tag, read, write)                                  \
  int _dummy_##tag[0] _yaml_attribute("raw:YAML_CUSTOM(" _yaml_note( \
      #tag) "," _yaml_note(read) "," _yaml_note(write) ")")

#define CUST_ARRAY(tag, elmt_type, max, fcn)                            \
  int _dummy_##tag[0] _yaml_attribute("raw:YAML_ARRAY(" _yaml_note(     \
      #tag) ", 0, " #max ", " _yaml_note(elmt_type) ", " _yaml_note(fcn) ")")

#define CUST_IDX(tag, read, write)                                     \
  int _dummy_##tag[0] _yaml_attribute("raw:YAML_IDX_CUST(" _yaml_note( \
      #tag) "," _yaml_note(read) "," _yaml_note(write) ")")

#else

#define ENUM(label)
#define SKIP
#define USE_IDX
#define NO_IDX
#define FUNC(name)
#define NAME(label)
#define CUST(read, write)
#define ARRAY(elmt_size, elmt_type, fcn)
#define CUST_ATTR(tag, read, write)
#define CUST_ARRAY(tag, elmt_type, max, fcn)
#define CUST_IDX(tag, read, write)

#endif
