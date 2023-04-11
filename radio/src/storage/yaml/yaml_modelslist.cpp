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

#include "yaml_modelslist.h"
#include "yaml_parser.h"

#include "storage/modelslist.h"

#include <cstring>

using std::list;

struct modelslist_iter
{
    enum Level {
        Root=0,
        Category=1,
        Model=2
    };

    std::vector<std::string>* filelist;
    uint8_t     level;
    char        current_attr[16]; // set after find_node()
};

static modelslist_iter __modelslist_iter_inst;

void* get_modelslist_iter(std::vector<std::string>* filelist)
{
    __modelslist_iter_inst.filelist = filelist;
    __modelslist_iter_inst.level = 0;

    return &__modelslist_iter_inst;
}

static bool to_parent(void* ctx)
{
    modelslist_iter* mi = (modelslist_iter*)ctx;
    if (mi->level == modelslist_iter::Root) {
        return false;
    }

    mi->level--;
    return true;
}

static bool to_child(void* ctx)
{
    modelslist_iter* mi = (modelslist_iter*)ctx;
    if (mi->level == modelslist_iter::Model) {
        return false;
    }

    mi->level++;
    return true;
}

static bool to_next_elmt(void* ctx)
{
    modelslist_iter* mi = (modelslist_iter*)ctx;
    if (mi->level == modelslist_iter::Root) {
        return false;
    }
    return true;
}

static bool find_node(void* ctx, char* buf, uint8_t len)
{
    modelslist_iter* mi = (modelslist_iter*)ctx;

    if (len > sizeof(modelslist_iter::current_attr)-1)
        len = sizeof(modelslist_iter::current_attr)-1;

    memcpy(mi->current_attr, buf, len);
    mi->current_attr[len] = '\0';

    return true;
}

static void set_attr(void* ctx, char* buf, uint16_t len)
{
  char fnamebuf[LEN_MODEL_FILENAME + 1];
  modelslist_iter* mi = (modelslist_iter*)ctx;

  switch (mi->level) {
    case modelslist_iter::Model:
      if (!strcmp(mi->current_attr, "filename")) {
        if(len <= LEN_MODEL_FILENAME) {
          memcpy(fnamebuf, buf, len);
          fnamebuf[len] = '\0';
          mi->filelist->push_back(fnamebuf);
        }
      }
      break;
  }
}

static const YamlParserCalls modelslistCalls = {
    to_parent,
    to_child,
    to_next_elmt,
    find_node,
    set_attr
};

const YamlParserCalls* get_modelslist_parser_calls()
{
    return &modelslistCalls;
}
