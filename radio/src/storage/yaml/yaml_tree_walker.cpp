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

#include <string.h>

#include "debug.h"
#include "yaml_node.h"
#include "yaml_bits.h"
#include "yaml_tree_walker.h"
#include "yaml_parser.h"

#define MIN(a,b) (a < b ? a : b)

static void copy_string(char* dst, uint16_t dst_len, const char* src,
                        uint8_t src_len)
{
  if (src_len < dst_len) {
    memcpy(dst, src, src_len);
    dst[src_len] = '\0';
  } else {
    memcpy(dst, src, dst_len);
  }
}

uint32_t yaml_parse_enum(const struct YamlIdStr* choices, const char* val, uint8_t val_len)
{
    while (choices->str) {

        // we have a match!
        if( strncmp(val, choices->str, val_len) == 0
          && strlen(choices->str) == val_len)
            break;

        choices++;
    }

    return choices->id;
}

static void yaml_set_attr(void* user, uint8_t* ptr, uint32_t bit_ofs,
                          const YamlNode* node, const char* val,
                          uint16_t val_len)
{
  uint32_t i = 0;

  // TRACE("set(%s, %.*s, bit-ofs=%u, bits=%u)\n",
  //       node->tag, val_len, val, bit_ofs, node->size);

  ptr += bit_ofs >> 3UL;
  bit_ofs &= 0x07;

  if (node->type == YDT_STRING) {
    // assert(!bit_ofs);
    copy_string((char*)ptr, node->size >> 3, val, val_len);
    return;
  }

  switch (node->type) {
    case YDT_SIGNED:
        i = node->u._cust.cust_to_uint ? node->u._cust.cust_to_uint(node, val, val_len)
            : (uint32_t)yaml_str2int(val, val_len);
        break;
    case YDT_UNSIGNED:
        i = node->u._cust.cust_to_uint ? node->u._cust.cust_to_uint(node, val, val_len)
            : yaml_str2uint(val, val_len);
        break;
    case YDT_ENUM:
        i = yaml_parse_enum(node->u._enum.choices, val, val_len);
        break;
    case YDT_CUSTOM:
        if (node->u._cust_attr.read)
            node->u._cust_attr.read(user, ptr, bit_ofs, val, val_len);
        return;
    default:
        break;
    }

    yaml_put_bits(ptr, i, bit_ofs, node->size);
}

const char* yaml_output_enum(int32_t i, const struct YamlIdStr* choices)
{
    //TRACE("<choice = %d>", i);
    while(choices->str) {
        if (i == choices->id)
            break;
        choices++;
    }

    return choices->str;
}

static const char hex_digits[] {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

static bool yaml_output_string(const char* str, uint32_t max_len,
                               yaml_writer_func wf, void* opaque)
{
    if (!wf(opaque, "\"", 1))
        return false;

    while(max_len > 0 && *str) {
        if (*str >= 0x20 && *str <= 0x7E && *str != '"') {
            if (!wf(opaque, str++, 1)) return false;
            max_len--;
        }
        else {
            if (!wf(opaque, "\\x", 2)) return false;
            if (!wf(opaque, &(hex_digits[((uint8_t)*str) >> 4]), 1)) return false;
            if (!wf(opaque, &(hex_digits[((uint8_t)*str) & 0x0F]), 1)) return false;
            str++; max_len--;
        }
    }

    if (!wf(opaque, "\"", 1))
        return false;

    return true;
}

static bool yaml_output_attr(void* user, uint8_t* ptr, uint32_t bit_ofs,
                             const YamlNode* node, yaml_writer_func wf,
                             void* opaque)
{
  if (node->type == YDT_NONE) return false;
  if (node->type == YDT_PADDING) return true;
  if (node->type == YDT_CUSTOM && !node->u._cust_attr.write) return true;

  // output tag
  if (!wf(opaque, node->tag, node->tag_len())) return false;

  if (!wf(opaque, ": ", 2)) return false;

  if (ptr) {
    ptr += bit_ofs >> 3UL;
    bit_ofs &= 0x07;

    const char* p_out = NULL;
    if (node->type == YDT_STRING) {
      // assert(!bit_ofs);
      if (!yaml_output_string((const char*)ptr, (node->size) >> 3UL, wf,
                              opaque))
        return false;
    } else if (node->type == YDT_CUSTOM) {
      if (node->u._cust_attr.write &&
          !node->u._cust_attr.write(user, ptr, bit_ofs, wf, opaque))
        return false;
    } else {
      unsigned int i = yaml_get_bits(ptr, bit_ofs, node->size);

      if ((node->type == YDT_SIGNED || node->type == YDT_UNSIGNED) &&
          node->u._cust.uint_to_cust) {
        return node->u._cust.uint_to_cust(node, i, wf, opaque) &&
               wf(opaque, "\r\n", 2);
      } else {
        switch (node->type) {
          case YDT_SIGNED:
            p_out = yaml_signed2str((int)yaml_to_signed(i, node->size));
            break;
          case YDT_UNSIGNED:
            p_out = yaml_unsigned2str(i);
            break;
          case YDT_ENUM:
            p_out = yaml_output_enum(i, node->u._enum.choices);
            break;

          case YDT_ARRAY:
          case YDT_UNION:
          default:
            break;
        }
      }
    }

    if (p_out && !wf(opaque, p_out, strlen(p_out))) return false;
  }

  return wf(opaque, "\r\n", 2);
}

YamlTreeWalker::YamlTreeWalker()
    : stack_level(NODE_STACK_DEPTH),
      virt_level(0),
      anon_union(0)
{
    memset(stack,0,sizeof(stack));
}

void YamlTreeWalker::reset(const YamlNode* node, uint8_t* data)
{
    this->data = data;
    stack_level = NODE_STACK_DEPTH;
    virt_level  = 0;

    push();
    setNode(node);
    rewind();
}

bool YamlTreeWalker::push()
{
    if (full())
        return false;

    stack_level--;
    memset(&(stack[stack_level]), 0, sizeof(State));

    return true;
}

bool YamlTreeWalker::pop()
{
    if (empty())
        return false;

    memset(&(stack[stack_level]), 0, sizeof(State));
    stack_level++;

    return true;
}

// Rewind to the current node's first attribute
// (and reset the bit offset)
void YamlTreeWalker::rewind()
{
    if (getNode()->type == YDT_ARRAY
        || getNode()->type == YDT_UNION) {
        setAttrIdx(0);
        setAttrOfs(getLevelOfs());
    }
}

// Increment the cursor until a match is found or the end of
// the current collection (node of type YDT_NONE) is reached.
//
// return true if a match has been found.
bool YamlTreeWalker::findNode(const char* tag, uint8_t tag_len)
{
    if (virt_level)
        return false;

    rewind();

    const struct YamlNode* attr = getAttr();
    if (isArrayElmt() && attr && attr->type == YDT_IDX) {
        setAttrValue((char*)tag, tag_len);
        return true;
    }

    while(attr && attr->type != YDT_NONE) {

        if ((tag_len == attr->tag_len())
            && !strncmp(tag, attr->tag, tag_len)) {
            return true; // attribute found!
        }

        toNextAttr();
        attr = getAttr();
    }

    return false;
}

// Get the current bit offset
unsigned int YamlTreeWalker::getBitOffset()
{
    return stack[stack_level].getOfs();
}

bool YamlTreeWalker::toParent()
{
    if(virt_level) {
        virt_level--;
        return true;
    }

    if (!pop())
        return false;

    return !empty();
}

bool YamlTreeWalker::toChild()
{
    const struct YamlNode* attr = getAttr();
    if (!attr || isIdxInvalid()
        || (attr->type != YDT_ARRAY
            && attr->type != YDT_UNION
            && !isArrayElmt())) {
        virt_level++;
        return true;
    }

    bool is_array = false;
    if (attr->type == YDT_ARRAY
        && attr->elmts > 1) {
        is_array = true;
    }

    const YamlNode* parent_node = getNode();
    if (isArrayElmt() && attr->type == YDT_IDX) {
        attr = parent_node;
    }

    if (!push()) {
        virt_level++;
        return false;
    }

    setNode(attr);
    setAttrOfs(getLevelOfs());

    attr = getAttr();
    if (!attr)
        return false;
    
    if ((attr->type == YDT_UNION) && (attr->tag_len() == 0)) {
        toChild();
        anon_union++;
    }

    if (is_array)
        setArrayElmt(true);
    
    return true;
}

bool YamlTreeWalker::toNextElmt()
{
    const struct YamlNode* node = getNode();
    if (!virt_level && (node->type == YDT_ARRAY
                        || node->type == YDT_UNION)) {

        if (node->type == YDT_UNION) {
            return false;
        }

        if (isIdxInvalid()) {
            setIdxInvalid(false);
            setElmts(0);
        }
        
        if (getElmts() < node->elmts - 1) {
            incElmts();
            rewind();
        } else {
            return false;
        }
    }

    return true;
}

bool YamlTreeWalker::isElmtEmpty(uint8_t* data)
{
    if (virt_level)
        return true;

    if (!data)
        return false;
    
    const struct YamlNode* node = getNode();
    uint32_t bit_ofs = 0;

    if (node->type == YDT_ARRAY) {

        bit_ofs = ((uint32_t)getElmts())
            * getNode()->size
            + getLevelOfs();

        // assume structs aligned on 8bit boundaries
        if (node->u._array.u.is_active)
            return !node->u._array.u.is_active(this, data, bit_ofs);

        return yaml_is_zero(data, bit_ofs, node->size);
    }
    else if ((node->type == YDT_UNION) && hasParent()) {

        bit_ofs = getLevelOfs();

        TRACE_YAML("<not empty>");
        return false;//node->u._array.u.select_member; //TODO!
            // // assume structs aligned on 8bit boundaries
            // && !node->_array.is_active(data + (bit_ofs >> 3));
    }

    return false;
}

void YamlTreeWalker::toNextAttr()
{
    const struct YamlNode* node = getNode();
    const struct YamlNode* attr = NULL;

    if (node->type != YDT_UNION) {
    
        attr = getAttr();
        uint32_t attr_bit_ofs = getAttrOfs();

        if (attr->type == YDT_ARRAY)
            attr_bit_ofs += ((uint32_t)attr->elmts * attr->size);
        else
            attr_bit_ofs += attr->size;

        setAttrOfs(attr_bit_ofs);
    }

    incAttr();

    // anonymous union handling
    attr = getAttr();
    if ((attr->type == YDT_UNION) && (strlen(attr->tag) == 0)) {
        toChild();
        anon_union++;
    }
    else if ((attr->type == YDT_NONE)
             && (getNode()->type == YDT_UNION)
             && anon_union) {

        anon_union--;
        toParent();
        toNextAttr();
    }
}

void YamlTreeWalker::setAttrValue(char* buf, uint16_t len)
{
    if (!buf || !len || isIdxInvalid())
        return;

    const YamlNode* attr = getAttr();
    if (attr->type == YDT_IDX) {

        uint32_t i = 0;
        if (attr->u._cust_idx.read)
            i = attr->u._cust_idx.read(this, buf, len);
        else
            i = yaml_str2uint(buf, len);

        const YamlNode* node = getNode();
        if (i < node->elmts) {
            setElmts(i);
            rewind();
        } else {
            setIdxInvalid(true);
        }
    }
    else {
        yaml_set_attr(this, data, getBitOffset(), attr, buf, len);
    }
}

bool YamlTreeWalker::generate(yaml_writer_func wf, void* opaque)
{
    bool new_elmt = false;
    
    while (true) {
        const struct YamlNode* attr = getAttr();

        if (attr->type == YDT_PADDING) {
            toNextAttr();
            continue;
        }
        
        // end of this level, go up or die
        if (attr->type == YDT_NONE) {

            const struct YamlNode* node = getNode();
            if (node->type != YDT_ARRAY && node->type != YDT_UNION)
                return false; // Error in the structure (probably)

            // if parent is a union, no need to output the other elements...
            const YamlNode* parent = getParent();
            if (parent && (parent->type == YDT_UNION)) {

                if (!toParent())
                    return false;
            }
            else {

                if (isParentArrayElmt() && (getParent() == node) && !toParent())
                    return false;
                
                // walk to next non-empty element
                while (toNextElmt()) {
                    if (!isElmtEmpty(data)) {
                        new_elmt = true;
                        break;
                    }
                }

                if (new_elmt)
                    continue;
            }

            // no next element, go up
            if (!toParent())
                return true;

            if (getNode()->type == YDT_UNION) {
                if (!toParent()) {
                    return true;
                }
            }
            toNextAttr();
            continue;
        }
        else if (attr->type == YDT_ARRAY || attr->type == YDT_UNION) {

            if (!toChild())
                return false; // TODO: error handling???

            const struct YamlNode* node = getNode();
            if (node->type == YDT_UNION && node->u._array.u.select_member) {

                // output union tag, select member and go up one level

                // output union tag
                for(int i=2; i < getLevel(); i++)
                    if (!wf(opaque, "   ", 3))
                        return false;
                if (!yaml_output_attr(this, NULL, 0, node, wf, opaque))
                    return false; // TODO: error handling???

                // grab attr idx...
                uint8_t idx =
                    node->u._array.u.select_member(this, data, getBitOffset());
                // TRACE("<idx = %d>", idx);
                setAttrIdx(idx);

                attr = getAttr();
                for(int i=1; i < getLevel(); i++)
                    if (!wf(opaque, "   ", 3))
                        return false;
                if (!yaml_output_attr(this, data, getBitOffset(), attr, wf, opaque))
                    return false; // TODO: error handling???

                if (attr->type != YDT_ARRAY
                    && attr->type != YDT_UNION) {

                    if (!toParent())
                        return false;

                    toNextAttr();
                }
                else {
                    if (!toChild() && !toParent())
                        return false;
                }
                continue;
            }

            // walk to next non-empty element
            do {
                if (!isElmtEmpty(data)) {
                    new_elmt = true;
                    break;
                }
                new_elmt = false;
            } while (toNextElmt());

            if (new_elmt) {
                // non-empty element present in a new structure/array
                // let's output the attribute
                for(int i=2; i < getLevel(); i++)
                    if (!wf(opaque, "   ", 3))
                        return false;
                if (!yaml_output_attr(this, NULL, 0, getNode(), wf, opaque))
                    return false; // TODO: error handling???
                continue;
            }

            // no next element, go up
            if (!toParent())
                return true;

            toNextAttr();
            continue;
        }

        // TODO: support for lists (inputs, mixers)
        if (attr->type == YDT_IDX) {

            new_elmt = false;
            for(int i=1; i < getLevel(); i++)
                if (!wf(opaque, "   ", 3))
                    return false;

            if (attr->u._cust_idx.write) {
                if (!attr->u._cust_idx.write(this,wf,opaque))
                    return false;
            }
            else {
                char* idx = yaml_unsigned2str(getElmts());
                if (!wf(opaque, idx, strlen(idx)))
                    return false;
            }

            if (!wf(opaque, ":\r\n", 3))
                return false;

            toChild(); //TODO: error checking
        }
        else {

            // only for lists:
            // - arrays have IDX upfront
            // - structs are not marked as arrays
            if (new_elmt && isArrayElmt()) {
                for(int i=2; i < getLevel(); i++)
                    if (!wf(opaque, "   ", 3))
                        return false;

                if (!wf(opaque, " -\r\n", 4))
                    return false;
            }
                
            new_elmt = false;

            if (attr->type != YDT_PADDING &&
                (attr->type != YDT_CUSTOM || attr->u._cust_attr.write)) {

                for(int i=1; i < getLevel(); i++)
                    if (!wf(opaque, "   ", 3))
                        return false;
            }
            
            if (!yaml_output_attr(this, data, getBitOffset(), attr, wf, opaque))
                return false; // TODO: error handling???
        }

        toNextAttr();
    }

    return true;
}

void YamlTreeWalker::dump_stack()
{
    for (int i=0; i<NODE_STACK_DEPTH; i++) {
        TRACE(" [%p|%u|%i|%i]",
              stack[i].node,stack[i].bit_ofs,
              stack[i].attr_idx,stack[i].elmts);
    }
    TRACE("\n");
}

static bool to_parent(void* ctx)
{
    return ((YamlTreeWalker*)ctx)->toParent();
}

static bool to_child(void* ctx)
{
    return ((YamlTreeWalker*)ctx)->toChild();
}

static bool to_next_elmt(void* ctx)
{
    return ((YamlTreeWalker*)ctx)->toNextElmt();
}

static bool find_node(void* ctx, char* buf, uint8_t len)
{
    return ((YamlTreeWalker*)ctx)->findNode(buf,len);
}

static void set_attr(void* ctx, char* buf, uint16_t len)
{
    ((YamlTreeWalker*)ctx)->setAttrValue(buf,len);
}

const YamlParserCalls YamlTreeWalkerCalls = {
    to_parent,
    to_child,
    to_next_elmt,
    find_node,
    set_attr
};

const YamlParserCalls* YamlTreeWalker::get_parser_calls()
{
    return &YamlTreeWalkerCalls;
}

