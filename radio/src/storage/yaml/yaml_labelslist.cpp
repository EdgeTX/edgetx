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

#include "yaml_labelslist.h"
#include "yaml_parser.h"

#include "debug.h"

#include "storage/modelslist.h"

#include <cstring>

#ifdef DEBUG_LABELS
#define TRACE_LABELS_YAML(...) TRACE(__VA_ARGS__)
#else
#define TRACE_LABELS_YAML(...)
#endif

using std::list;

struct labelslist_iter
{
    enum SectionType {
      SEC_Unknown=0,
      SEC_Labels=1,
      SEC_Models=2,
      SEC_Sort=3,
    };

    ModelCell   *curmodel;
    bool        modeldatavalid; // Used to determine if reading yaml values is necessary
    uint8_t     level;
    uint8_t     section;
    char        current_attr[LABELS_LENGTH+1]; // set after find_node()
    char        current_label[LABELS_LENGTH+1]; // set after find_node()
};

static labelslist_iter __labelslist_iter_inst;

void* get_labelslist_iter()
{
  __labelslist_iter_inst.modeldatavalid = false;
  __labelslist_iter_inst.curmodel = NULL;
  __labelslist_iter_inst.level = 0;
  __labelslist_iter_inst.section = labelslist_iter::SEC_Unknown;

  TRACE_LABELS_YAML("YAML Label Reader Start %u", 0);

  return &__labelslist_iter_inst;
}

static bool to_parent(void* ctx)
{
    labelslist_iter* mi = (labelslist_iter*)ctx;

    if (mi->level == 0)
        return false;

    mi->level--;

    TRACE_LABELS_YAML("-> YAML To Parent %u", mi->level);

    return true;
}

static bool to_child(void* ctx)
{
    labelslist_iter* mi = (labelslist_iter*)ctx;

    mi->level++;

    TRACE_LABELS_YAML("<- YAML To Child %u", mi->level);
    return true;
}

static bool to_next_elmt(void* ctx)
{
    labelslist_iter* mi = (labelslist_iter*)ctx;
    if (mi->level == 0) {
        return false;
    }

    TRACE_LABELS_YAML("YAML Next Element. Level %u", mi->level);
    return true;
}

static bool find_node(void* ctx, char* buf, uint8_t len)
{
    labelslist_iter* mi = (labelslist_iter*)ctx;

    memcpy(mi->current_attr, buf, len);
    mi->current_attr[len] = '\0';

    TRACE_LABELS_YAML("YAML Found Node. Level = %u, Section= %u, %s", mi->level, mi->section, mi->current_attr);

    // Switch between the different maps in labels.yml
    if (mi->level == 0) {
      if (strcasecmp(mi->current_attr,"labels") == 0) {
        mi->section = labelslist_iter::SEC_Labels;
        TRACE_LABELS_YAML("YAML Set Section to Labels");
      } else if (strcasecmp(mi->current_attr,"sort") == 0) {
        mi->section = labelslist_iter::SEC_Sort;
        TRACE_LABELS_YAML("YAML Set Section to Sort");
      } else  if (strcasecmp(mi->current_attr,"models") == 0) {
        mi->section = labelslist_iter::SEC_Models;
        TRACE_LABELS_YAML("YAML Set Section to Models");
      }
    }

    // Model List
    if(mi->level == 1 && mi->section == labelslist_iter::SEC_Models)  {
      bool found=false;
      for(auto &filehash : modelslist.fileHashInfo) {
        if(filehash.name == mi->current_attr) {
          TRACE_LABELS_YAML("  Model %s has a real file, creating a modelcell", mi->current_attr);
          if(filehash.celladded) {
            TRACE_LABELS_YAML("    Duplicate found labels.yml model cell %s already added", mi->current_attr);
            break;
          }
          ModelCell *model = new ModelCell(mi->current_attr);
          strcpy(model->modelFinfoHash, filehash.hash);
          modelslist.push_back(model);
          filehash.celladded = true;
          if(filehash.curmodel == true)
            modelslist.setCurrentModel(model);
          mi->curmodel = model;
          mi->modeldatavalid = false;
          mi->curmodel->_isDirty = true;
          found = true;
          break;
        }
      }
      if(!found) {
        mi->curmodel = NULL;
        TRACE_LABELS_YAML("File does not exist in /MODELS");
      }
    }

    // Labels List
    if(mi->level == 1 && mi->section == labelslist_iter::SEC_Labels)  {
      TRACE_LABELS_YAML("Label Found -- %s", mi->current_attr);
      modelslabels.addLabel(mi->current_attr);
      strncpy(mi->current_label,mi->current_attr, LABELS_LENGTH);
      mi->current_label[LABELS_LENGTH] = '\0';
    }

    return true;
}

static void set_attr(void* ctx, char* buf, uint16_t len)
{
  char value[LABELS_LENGTH + 1];
  if(len > LABELS_LENGTH) {
    TRACE("ERROR: YAML too long for buffer");
    return;
  }
  memcpy(value, buf, len);
  value[len] = '\0';

  labelslist_iter* mi = (labelslist_iter*)ctx;
  TRACE_LABELS_YAML("YAML Attr Level %u, Section %u, %s = %s", mi->level, mi->section, mi->current_attr, value);

  // Model Section
  if(mi->level == 2 && mi->section == labelslist_iter::SEC_Models && mi->curmodel != NULL) {
    // File Hash
    if(!strcasecmp(mi->current_attr, "hash")) {
      if(!strcmp(mi->curmodel->modelFinfoHash, value)) {
        TRACE_LABELS_YAML("FILE HASH MATCHES, No need to scan this model, just load the settings");
        mi->modeldatavalid = true;
        mi->curmodel->valid_rfData = true;
        mi->curmodel->_isDirty = false;
      } else {
        TRACE_LABELS_YAML("FILE HASH Does not Match, Open model and rebuild modelcell");
        mi->modeldatavalid = false;
        mi->curmodel->_isDirty = true;
      }
    }

    // Last Opened
    // Hack: Always load this as the hash value is (currently) not in synch with the saved model file
    // TODO: Ensure stored hash is up to date.
    if (!strcasecmp(mi->current_attr, "lastopen")) {
        mi->curmodel->lastOpened = (gtime_t)strtol(value, NULL, 0);
        TRACE_LABELS_YAML(" Last Opened %lu", value);
    } else if (mi->modeldatavalid) {
      // Don't bother filling in values below if hash didn't match

      // Model Name
      if(!strcasecmp(mi->current_attr, "name")) {
        mi->curmodel->setModelName(value);
        TRACE_LABELS_YAML(" Set the models name");

      // Model Bitmap
  #if LEN_BITMAP_NAME > 0
      } else if(!strcasecmp(mi->current_attr, "bitmap")) {
        // TODO Check if it exists ?
        strcpy(mi->curmodel->modelBitmap, value);
        TRACE_LABELS_YAML(" Set the models bitmap");
  #endif

      // Model Labels
      } else if(!strcasecmp(mi->current_attr, "labels")) {
        LabelsVector labels = ModelMap::fromCSV(value);
        for(const auto &lbl : labels ) {
          modelslabels.addLabelToModel(lbl,mi->curmodel);
          TRACE_LABELS_YAML("  Adding the label - %s", lbl.c_str());
        }

      // RF Module Data
      } else {
        char cmp[15];
        for(int i=0; i < NUM_MODULES; i++) {
          snprintf(cmp, sizeof(cmp), MODULE_ID_STR, i);
          cmp[sizeof(cmp)-1] = '\0';
          if(!strcasecmp(mi->current_attr, cmp)) {
            mi->curmodel->modelId[i] = strtol(value,NULL,10);
            TRACE_LABELS_YAML( " Set the module %d rfId to %s", i, value);
          }
          snprintf(cmp, sizeof(cmp), MODULE_TYPE_STR, i);
          cmp[sizeof(cmp)-1] = '\0';
          if(!strcasecmp(mi->current_attr, cmp)) {
            mi->curmodel->moduleData[i].type = strtol(value,NULL,10);
            TRACE_LABELS_YAML(" Set the module %d rfType to %s", i, value);
          }
          snprintf(cmp, sizeof(cmp), MODULE_RFPROTOCOL_STR, i);
          cmp[sizeof(cmp)-1] = '\0';
          if(!strcasecmp(mi->current_attr, cmp)) {
            mi->curmodel->moduleData[i].subType = strtol(value,NULL,10);
            TRACE_LABELS_YAML(" Set the module %d rfProtocol to %s", i, value);
          }
        }
      }
    }

  // Label Section
  } else if(mi->level == 2 && mi->section == labelslist_iter::SEC_Labels) {
    if(!strcasecmp(mi->current_attr, "icon")) {
      TRACE_LABELS_YAML(" Label Icon - %s", value);
      // TODO - Check icon exists, or ignore it.
    } else if(!strcasecmp(mi->current_attr, "selected")) {
      TRACE("FOUND %s Label is selected", mi->current_label);
      modelslabels.addFilteredLabel(mi->current_label);
    }

  // Sort Order
  } else if (mi->level == 0 && mi->section == labelslist_iter::SEC_Sort)  {
    TRACE_LABELS_YAML(" Sort Order Found -- %s", value);
    modelslabels.setSortOrder((ModelsSortBy)strtol(value,NULL,10));
  }
}

static const YamlParserCalls labelslistCalls = {
    to_parent,
    to_child,
    to_next_elmt,
    find_node,
    set_attr
};

const YamlParserCalls* get_labelslist_parser_calls()
{
    return &labelslistCalls;
}
