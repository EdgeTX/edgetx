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

#ifdef DEBUG_LABELS_YAML
#define TRACE_LABELS_YAML(...) TRACE(__VA_ARGS__)
#else
#define TRACE_LABELS_YAML(...)
#endif

using std::list;

struct labelslist_iter
{
    enum ItemData {
        Root=0,
        ModelsRoot=1,
        ModelName=2,
        ModelData=3,
        LabelsRoot=4,
        LabelName=5,
        LabelData=6,
    };

    ModelCell   *curmodel;
    bool        modeldatavalid; // Used to determine if reading yaml values is necessary
    uint8_t     level;
    char        current_attr[LABEL_LENGTH+1]; // set after find_node()
    char        current_label[LABEL_LENGTH+1]; // set after find_node()
};

static labelslist_iter __labelslist_iter_inst;

void* get_labelslist_iter()
{
  // Thoughts... record the time here.
  // If time > xxx while parsing models. pop up a window showing updating?

  __labelslist_iter_inst.modeldatavalid = false;
  __labelslist_iter_inst.curmodel = NULL;
  __labelslist_iter_inst.level = 0;

  TRACE_LABELS_YAML("YAML Label Reader Start %u", 0);

  return &__labelslist_iter_inst;
}

static bool to_parent(void* ctx)
{
    labelslist_iter* mi = (labelslist_iter*)ctx;

    if (mi->level == labelslist_iter::Root)
        return false;

    if(mi->level == labelslist_iter::LabelName) {
      TRACE_LABELS_YAML("Forced Models Root");
      mi->level = labelslist_iter::ModelsRoot;
    }
    else
      mi->level--;

    TRACE_LABELS_YAML("YAML To Parent %u", mi->level);

    return true;
}

static bool to_child(void* ctx)
{
    labelslist_iter* mi = (labelslist_iter*)ctx;

    mi->level++;

    TRACE_LABELS_YAML("YAML To Child");
    TRACE_LABELS_YAML("YAML Level %u", mi->level);
    return true;
}

static bool to_next_elmt(void* ctx)
{
    labelslist_iter* mi = (labelslist_iter*)ctx;
    if (mi->level == labelslist_iter::Root) {
        return false;
    }

    TRACE_LABELS_YAML("YAML To Next Element");
    TRACE_LABELS_YAML("YAML Current Level %u", mi->level);
    return true;
}

static bool find_node(void* ctx, char* buf, uint8_t len)
{
    labelslist_iter* mi = (labelslist_iter*)ctx;

    memcpy(mi->current_attr, buf, len);
    mi->current_attr[len] = '\0';

    TRACE_LABELS_YAML("YAML On Node %s", mi->current_attr);
    TRACE_LABELS_YAML("YAML Current Level %u", mi->level);

    // If in the labels node, force to labelsroot enum
    if(mi->level == labelslist_iter::ModelsRoot && strcasecmp(mi->current_attr,"labels") == 0) {
      TRACE_LABELS_YAML("Forced root");
      mi->level = labelslist_iter::LabelsRoot;
      TRACE_LABELS_YAML("YAML New Level %u", mi->level);
    }

    if(mi->level == labelslist_iter::LabelsRoot && strcasecmp(mi->current_attr,"models") == 0) {
      TRACE_LABELS_YAML("Forced root");
      mi->level = labelslist_iter::ModelsRoot;
      TRACE_LABELS_YAML("YAML New Level %u", mi->level);
    }

    // Model List
    if(mi->level == labelslist_iter::ModelName)  {
      bool found=false;
      for(auto &filehash : modelslist.fileHashInfo) {
        if(filehash.name == mi->current_attr) {
          TRACE_LABELS_YAML("  Model %s has a real file, creating a modelcell");
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
    if(mi->level == labelslist_iter::LabelName)  {
      TRACE_LABELS_YAML("Label Found -- %s", mi->current_attr);
      modelslabels.addLabel(mi->current_attr);
      strncpy(mi->current_label,mi->current_attr, LABEL_LENGTH);
      mi->current_label[LABEL_LENGTH] = '\0';
    }

    return true;
}

static void set_attr(void* ctx, char* buf, uint8_t len)
{
  char value[40];
  memcpy(value, buf, len);
  value[len] = '\0';

  labelslist_iter* mi = (labelslist_iter*)ctx;
  TRACE_LABELS_YAML("YAML Attr Level %u, %s = %s", mi->level, mi->current_attr, value);

  // Model Section
  if(mi->level == labelslist_iter::ModelData) {

    // File Hash
    if(!strcasecmp(mi->current_attr, "hash")) {
      if(mi->curmodel != NULL) {
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

    // Model Name
    } else if(mi->modeldatavalid && !strcasecmp(mi->current_attr, "name")) {
      if(mi->curmodel != NULL) {
        mi->curmodel->setModelName(value);
        TRACE_LABELS_YAML("Set the models name");
      }

    // Last Opened
    } else if(mi->modeldatavalid && !strcasecmp(mi->current_attr, "lastopen")) {
      if(mi->curmodel != NULL) {
        mi->curmodel->lastOpened = (gtime_t)strtol(value, NULL, 0);
        TRACE_LABELS_YAML("Last Opened %lu", value);
      }

    // Model Bitmap
#if LEN_BITMAP_NAME > 0
    } else if(mi->modeldatavalid && !strcasecmp(mi->current_attr, "bitmap")) {
      if(mi->curmodel != NULL) {
        // TODO Check if it exists ?
        strcpy(mi->curmodel->modelBitmap, value);
        TRACE_LABELS_YAML("Set the models bitmap");
      }
#endif

    // Model Labels
    } else if(mi->modeldatavalid && !strcasecmp(mi->current_attr, "labels")) {
      if(mi->curmodel != NULL) {
        char *cma;
        cma = strtok(value, ",");
        int numTokens = 0;
        while(cma != NULL) {
          modelslabels.addLabelToModel(cma,mi->curmodel);
          TRACE_LABELS_YAML(" Adding the label - %s", cma);
          cma = strtok(NULL, ",");
          numTokens++;
        }
      }

    // RF Module Data
    } else {
      char cmp[15];
      for(int i=0; i < NUM_MODULES; i++) {
        snprintf(cmp, sizeof(cmp), MODULE_ID_STR, i);
        cmp[sizeof(cmp)-1] = '\0';
        if(mi->curmodel != NULL && mi->modeldatavalid && !strcasecmp(mi->current_attr, cmp)) {
          mi->curmodel->modelId[i] = atoi(value);
          TRACE_LABELS_YAML("Set the module %d rfId to %s", i, value);
        }
        snprintf(cmp, sizeof(cmp), MODULE_TYPE_STR, i);
        cmp[sizeof(cmp)-1] = '\0';
        if(mi->curmodel != NULL && mi->modeldatavalid && !strcasecmp(mi->current_attr, cmp)) {
          mi->curmodel->moduleData[i].type = atoi(value);
          TRACE_LABELS_YAML("Set the module %d rfType to %s", i, value);
        }
        snprintf(cmp, sizeof(cmp), MODULE_RFPROTOCOL_STR, i);
        cmp[sizeof(cmp)-1] = '\0';
        if(mi->curmodel != NULL && mi->modeldatavalid && !strcasecmp(mi->current_attr, cmp)) {
          mi->curmodel->moduleData[i].subType = atoi(value);
          TRACE_LABELS_YAML("Set the module %d rfProtocol to %s", i, value);
        }

      }
    }


  // Label Section
  } else if(mi->level == labelslist_iter::LabelData) {
    if(!strcasecmp(mi->current_attr, "icon")) {
      TRACE_LABELS_YAML("Label Icon - %s", value);
      // TODO - Check icon exists, or ignore it.
    } else if(!strcasecmp(mi->current_attr, "selected")) {
      TRACE("FOUND %s Label is selected", mi->current_label);
      modelslabels.addFilteredLabel(mi->current_label);
    }
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
