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

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "yaml_parser.h"
#include "debug.h"

YamlParser::YamlParser()
{
}

void YamlParser::init(const YamlParserCalls* parser_calls, void* parser_ctx)
{
    indent = 0;
    level  = 0;
    memset(indents, 0, sizeof(indents));
    saved_state = 0;

    calls = parser_calls;
    ctx   = parser_ctx;
    reset();
    eof = false;
}

void YamlParser::reset()
{
    state = saved_state;
    if (state != ps_Dash) {
        indents[level] = indent;
        state = saved_state = ps_Indent;
    }
    indent = scratch_len  = 0;
    node_found = false;
}

bool YamlParser::toChild()
{
    bool ret = calls->to_child(ctx);
    if (ret) level++;
    return ret;
}

bool YamlParser::toParent()
{
    if (!level)
        return false;
    
    bool ret = calls->to_parent(ctx);
    if (ret) level--;
    return ret;
}

uint8_t YamlParser::getLastIndent()
{
    return indents[level];
}

YamlParser::YamlResult
YamlParser::parse(const char* buffer, unsigned int size)
{

#define CONCAT_STR(s,s_len,c)                   \
    {                                           \
        if(s_len < MAX_STR)                     \
            s[s_len++] = c;                     \
        else {                                  \
            TRACE_YAML("STRING_OVERFLOW");      \
            return STRING_OVERFLOW;             \
        }                                       \
    }

    const char* c   = buffer;
    const char* end = c + size;

    while(c < end) {

        switch(state) {

        case ps_Indent:
            if (*c == '-') {
                state = ps_Dash;
                ++indent;
                break;
            }
            // trap
        case ps_Dash:
            if (*c == ' ') { // skip space(s), should be only one??
                ++indent;
                break;
            }

            if (*c == '\r' || *c == '\n') {
                saved_state = state;
                state = ps_CRLF;
                continue;
            }

            if (indent < getLastIndent()) {
                // go up as many levels as necessary
                do {
                    if (!toParent()) {
                        TRACE_YAML("STOP (no parent)!\n");
                        return DONE_PARSING;
                    }
                } while (indent < getLastIndent());

                // attribute on same line as dash
                if (state == ps_Dash) {
                    if (!calls->to_next_elmt(ctx)) {
                        return DONE_PARSING;
                    }
                }
            }
            // go down one level
            else if (indent > getLastIndent()) {
                // TODO: check dash?
                if (!toChild()) {
                    TRACE_YAML("STOP (stack full)!\n");
                    return DONE_PARSING; // ERROR
                }
            }
            // same level, next element
            else if (state == ps_Dash || saved_state == ps_Dash) {
                // attribute on same line or next line as dash
                if (!calls->to_next_elmt(ctx)) {
                    return DONE_PARSING;
                }
            }

            state = ps_Attr;
            if (*c == '\"') {
                state = ps_AttrQuo;
                break;
            }
            CONCAT_STR(scratch_buf, scratch_len, *c);
            break;

        case ps_AttrQuo:
            if (*c == '\"') {
                state = ps_Attr;
                break;
            }
            CONCAT_STR(scratch_buf, scratch_len, *c);
            break;

        case ps_Attr:
            if (*c == '\"') {
                state = ps_AttrQuo;
                break;
            }
            if ((*c != ':') && (*c != '\r') && (*c != '\n'))
                CONCAT_STR(scratch_buf, scratch_len, *c);
            // trap
        case ps_AttrSP:
            if (*c == '\r' || *c == '\n') {
                if (state == ps_Attr) {
                    // TODO: trim spaces at the end?
                    node_found = calls->find_node(ctx, scratch_buf, scratch_len);
                    if (!node_found) {
                        TRACE_YAML("YAML_PARSER: Could not find node '%.*s' (2)\n",
                              scratch_len, scratch_buf);
                    }
                }
                saved_state = state;
                state = ps_CRLF;
                continue;
            }
            if (*c == ':') {
                if (state == ps_Attr) {
                    // TODO: trim spaces at the end?
                    node_found = calls->find_node(ctx, scratch_buf, scratch_len);
                    if (!node_found) {
                        TRACE_YAML("YAML_PARSER: Could not find node '%.*s' (3)\n",
                              scratch_len, scratch_buf);
                    }
                }
                state = ps_Sep;
                break;
            }
            break;

        case ps_Sep:
            if (*c == ' ')
                break;
            if (*c == '\r' || *c == '\n'){
                saved_state = state;
                state = ps_CRLF;
                continue;
            }
            state = ps_Val;
            scratch_len = 0;
            if (*c == '\"') {
                state = ps_ValQuo;
                break;
            }
            if (*c == '\\') {
                state = ps_ValEsc;
                break;
            }
            CONCAT_STR(scratch_buf, scratch_len, *c);
            break;

        case ps_ValQuo:
            if (*c == '\"') {
                state = ps_Val;
                break;
            }
            if (*c == '\\') {
                state = ps_ValEsc1;
                break;
            }
            CONCAT_STR(scratch_buf, scratch_len, *c);
            break;

        case ps_ValEsc1:
            if (*c == 'x') {
                state = ps_ValEsc2;
                break;
            }
            CONCAT_STR(scratch_buf, scratch_len, *c);
            state = ps_ValQuo;
            break;

        case ps_ValEsc2:
            if(scratch_len >= MAX_STR) {
                TRACE_YAML("STRING_OVERFLOW");
                return STRING_OVERFLOW;
            }
            else if (*c >= '0' && *c <= '9') {
                scratch_buf[scratch_len] = (*c - '0') << 4;
                state = ps_ValEsc3;
                break;
            }
            else if (*c >= 'A' && *c <= 'F') {
                scratch_buf[scratch_len] = (*c - 'A' + 10) << 4;
                state = ps_ValEsc3;
                break;
            }
            TRACE_YAML("wrong hex digit '%c'",*c);
            return DONE_PARSING;

        case ps_ValEsc3:
            if (*c >= '0' && *c <= '9') {
                scratch_buf[scratch_len++] |= (*c - '0');
                state = ps_ValQuo;
                break;
            }
            else if (*c >= 'A' && *c <= 'F') {
                scratch_buf[scratch_len++] |= (*c - 'A' + 10);
                state = ps_ValQuo;
                break;
            }
            TRACE_YAML("wrong hex digit '%c'",*c);
            return DONE_PARSING;
            
        case ps_Val:
            if (*c == '\r' || *c == '\n') {
                // set attribute
                if (node_found) {
                    // TODO: trim spaces at the end?
                    calls->set_attr(ctx, scratch_buf, scratch_len);
                }
                saved_state = state;
                state = ps_CRLF;
                continue;
            }
            if (*c == '\\') {
                state = ps_ValEsc;
                break;
            }
            CONCAT_STR(scratch_buf, scratch_len, *c);
            break;

        case ps_ValEsc:
            state = ps_Val;
            CONCAT_STR(scratch_buf, scratch_len, *c);
            break;
                
        case ps_CRLF:
            // Skip blank lines
            while (c < end && (*c == '\r' || *c == '\n'))
                c += 1;
            // reset state at EOL (unless we have run out of buffer, in case EOL continues in next buffer)
            if (c < end)
                reset();
            continue;
        }

        c++;
    } // for each char

    if ((state == ps_Val) && eof && node_found) {
        // TODO: trim spaces at the end?
        calls->set_attr(ctx, scratch_buf, scratch_len);
    }
    
    return CONTINUE_PARSING;
}

