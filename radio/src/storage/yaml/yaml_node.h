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

#ifndef _node_h_
#define _node_h_

#include <stdint.h>
#include <stddef.h>

#include "yaml_parser.h"

#define NODE_STACK_DEPTH 12

enum YamlDataType {
    YDT_NONE=0,
    YDT_IDX,
    YDT_SIGNED,
    YDT_UNSIGNED,
    YDT_STRING,
    YDT_ARRAY,
    YDT_ENUM,
    YDT_UNION,
    YDT_PADDING,
    YDT_CUSTOM
};

struct YamlIdStr
{
    int          id;
    const char*  str;
};

// return false if error
typedef bool (*yaml_writer_func)(void* opaque, const char* str, size_t len);

struct YamlNode
{
  typedef bool (*is_active_func)(void* user, uint8_t* data, uint32_t bitoffs);

  typedef uint32_t (*cust_to_uint_func)(const YamlNode* node, const char* val,
                                        uint8_t val_len);

  typedef bool (*uint_to_cust_func)(const YamlNode* node, uint32_t val,
                                    yaml_writer_func wf, void* opaque);

  typedef uint8_t (*select_member_func)(void* user, uint8_t* data,
                                        uint32_t bitoffs);

  typedef uint32_t (*cust_idx_read_func)(void* user, const char* val,
                                         uint8_t val_len);
  typedef bool (*cust_idx_write_func)(void* user, yaml_writer_func wf,
                                      void* opaque);

  typedef void (*cust_read_func)(void* user, uint8_t* data, uint32_t bitoffs,
                                 const char* val, uint8_t val_len);
  typedef bool (*cust_write_func)(void* user, uint8_t* data, uint32_t bitoffs,
                                  yaml_writer_func wf, void* opaque);

  uint8_t type;
  uint32_t size;  // bits
  uint8_t tag_len;
  const char* tag;
  union {
    struct {
      const YamlNode* child;
      union {
        struct {
          is_active_func is_active;
          uint16_t elmts;  // maximum number of elements
        } _a;
        select_member_func select_member;
      } u;
    } _array;

    struct {
      const YamlIdStr* choices;
    } _enum;

    struct {
      cust_to_uint_func cust_to_uint;
      uint_to_cust_func uint_to_cust;
    } _cust;

    struct {
      cust_idx_read_func read;
      cust_idx_write_func write;
    } _cust_idx;

    struct {
      cust_read_func read;
      cust_write_func write;
    } _cust_attr;
  } u;
};

#if !defined(_MSC_VER)
#define YAML_TAG(str)                           \
    .tag_len=(sizeof(str)-1), .tag=(str)

#define YAML_IDX                                \
    { .type=YDT_IDX, .size=0, YAML_TAG("idx") }

#define YAML_IDX_CUST(tag, f_read, f_write)                             \
    { .type=YDT_IDX, .size=0, YAML_TAG(tag), .u={._cust_idx={.read=(f_read), .write=(f_write) }} }

#define YAML_SIGNED(tag, bits)                          \
    { .type=YDT_SIGNED, .size=(bits), YAML_TAG(tag) }

#define YAML_UNSIGNED(tag, bits)                        \
    { .type=YDT_UNSIGNED, .size=(bits), YAML_TAG(tag) }

#define YAML_SIGNED_CUST(tag, bits, f_cust_to_uint, f_uint_to_cust)     \
    { .type=YDT_SIGNED, .size=(bits), YAML_TAG(tag), .u={._cust={ .cust_to_uint=f_cust_to_uint, .uint_to_cust=f_uint_to_cust }} }

#define YAML_UNSIGNED_CUST(tag, bits, f_cust_to_uint, f_uint_to_cust)   \
    { .type=YDT_UNSIGNED, .size=(bits), YAML_TAG(tag), .u={._cust={ .cust_to_uint=f_cust_to_uint, .uint_to_cust=f_uint_to_cust }} }

#define YAML_STRING(tag, max_len)                               \
    { .type=YDT_STRING, .size=((max_len)<<3), YAML_TAG(tag) }

#define YAML_STRUCT(tag, bits, nodes, f_is_active)                     \
    { .type=YDT_ARRAY, .size=(bits), YAML_TAG(tag), .u={._array={ .child=(nodes), .u={ ._a={.is_active=(f_is_active), .elmts=1 }}}} }

#define YAML_ARRAY(tag, bits, max_elmts, nodes, f_is_active)           \
    { .type=YDT_ARRAY, .size=(bits), YAML_TAG(tag), .u={._array={ .child=(nodes), .u={ ._a={.is_active=(f_is_active), .elmts=(max_elmts) }}}} }

#define YAML_ENUM(tag, bits, id_strs)                                   \
    { .type=YDT_ENUM, .size=(bits), YAML_TAG(tag), .u={._enum={ .choices=(id_strs) }} }

#define YAML_UNION(tag, bits, nodes, f_sel_m)                       \
    { .type=YDT_UNION, .size=(bits), YAML_TAG(tag), .u={._array={ .child=(nodes), .u={.select_member=(f_sel_m) }}} }

#define YAML_PADDING(bits)                      \
    { .type=YDT_PADDING, .size=(bits) }

#define YAML_CUSTOM(tag, f_read, f_write)       \
    { .type=YDT_CUSTOM, .size=0, YAML_TAG(tag), .u={._cust_attr={.read=(f_read), .write=(f_write) }} }

#define YAML_END                                \
    { .type=YDT_NONE }

#define YAML_ROOT(nodes)                                                \
    { .type=YDT_ARRAY, .size=0, .tag_len=0, .tag=NULL,                  \
            .u={                                                        \
            ._array={ .child=(nodes),                                   \
                      .u={._a={.is_active=NULL, .elmts=1 }}             \
            }}                                                          \
    }

#else // MSVC++ compat

#define YAML_TAG(str)                           \
    (sizeof(str)-1), (str)

#define YAML_IDX                                \
    { YDT_IDX , 0, YAML_TAG("idx") }

#define YAML_SIGNED(tag, bits)                          \
    { YDT_SIGNED, (bits), YAML_TAG(tag) }

#define YAML_UNSIGNED(tag, bits)                        \
    { YDT_UNSIGNED, (bits), YAML_TAG(tag) }

#define YAML_SIGNED_CUST(tag, bits, f_cust_to_uint, f_uint_to_cust)     \
    { YDT_SIGNED, (bits), YAML_TAG(tag), {{ (const YamlNode*)f_cust_to_uint, {{ (YamlNode::is_active_func)f_uint_to_cust, 0 }}}} }

#define YAML_UNSIGNED_CUST(tag, bits, f_cust_to_uint, f_uint_to_cust)   \
    { YDT_UNSIGNED, (bits), YAML_TAG(tag), {{ (const YamlNode*)f_cust_to_uint, {{ (YamlNode::is_active_func)f_uint_to_cust, 0}}}} }

#define YAML_STRING(tag, max_len)                               \
    { YDT_STRING, ((max_len)<<3), YAML_TAG(tag) }

#define YAML_STRUCT(tag, bits, nodes, f_is_active)                     \
    { YDT_ARRAY, (bits), YAML_TAG(tag), {{ (nodes), {{ (f_is_active), 1 }}}} }

#define YAML_ARRAY(tag, bits, max_elmts, nodes, f_is_active)           \
    { YDT_ARRAY, (bits), YAML_TAG(tag), {{ (nodes), {{ (f_is_active), (max_elmts) }}}} }

#define YAML_ENUM(tag, bits, id_strs)                                   \
    { YDT_ENUM, (bits), YAML_TAG(tag), {{ (const YamlNode*)(id_strs) }} }

#define YAML_UNION(tag, bits, nodes, f_sel_m)                       \
    { YDT_UNION, (bits), YAML_TAG(tag), {{ (nodes), {{ (YamlNode::is_active_func)(f_sel_m), 0 }}}} }

#define YAML_PADDING(bits)                      \
    { YDT_PADDING, (bits) }

#define YAML_END                                \
    { YDT_NONE }

#define YAML_ROOT(nodes)                                                \
    { YDT_ARRAY, 0, 0, NULL, {{ (nodes), {{ NULL, 1 }}}} }

#endif

#endif
