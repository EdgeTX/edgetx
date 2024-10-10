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

#include <stdint.h>

#define MAX_STR 256
#define MAX_DEPTH 16 // 12 real + 4 virtual

struct YamlParserCalls
{
    bool (*to_parent)    (void* ctx);
    bool (*to_child)     (void* ctx);
    bool (*to_next_elmt) (void* ctx);
    bool (*find_node)    (void* ctx, char* buf, uint8_t len);
    void (*set_attr)     (void* ctx, char* buf, uint16_t len);
};

class YamlParser
{
    enum ParserState {
        ps_Indent=0,
        ps_Dash,
        ps_Attr,
        ps_AttrQuo,
        ps_AttrSP,
        ps_Sep,
        ps_Val,
        ps_ValEsc,
        ps_ValQuo,
        ps_ValEsc1,
        ps_ValEsc2,
        ps_ValEsc3,
        ps_CRLF
    };

    // last indents for each level
    uint8_t indents[MAX_DEPTH];

    // current indent
    uint8_t indent;

    // current level
    uint8_t level;

    // parser state
    uint8_t state;
    uint8_t saved_state;

    // scratch buffer w/ 16 bytes
    // used for attribute and values
    char    scratch_buf[MAX_STR];
    uint16_t scratch_len;

    bool node_found;
    bool eof;

    // tree iterator state
    const YamlParserCalls* calls;
    void*                  ctx;

    // Reset parser state for next line
    void reset();

    bool    toChild();
    bool    toParent();
    uint8_t getLastIndent();

public:

    enum YamlResult {
        DONE_PARSING,
        CONTINUE_PARSING,
        STRING_OVERFLOW
    };

    YamlParser();

    void init(const YamlParserCalls* parser_calls, void* parser_ctx);

    YamlResult parse(const char* buffer, unsigned int size);

    void set_eof() { eof = true; }
};
