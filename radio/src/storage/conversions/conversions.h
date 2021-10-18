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

// Entry points for converting radio & model data
void convertBinRadioData(const char * path, int version);
const char* convertBinModelData(const char * path, int version);

bool eeConvert();
void eeConvertModel(int id, int version);

// // Conversions 216 to 217
// void convertModelData_216_to_217(ModelData &model);
// void convertRadioData_216_to_217(RadioData &settings);

// // Conversions 217 to 218
// void convertModelData_217_to_218(ModelData &model);
// void convertRadioData_217_to_218(RadioData &settings);

// // Conversions 218 to 219
// void convertModelData_218_to_219(ModelData &model);
// void convertRadioData_218_to_219(RadioData &settings);


// Conversions 219 to 220
const char* convertModelData_219_to_220(uint8_t id);
const char* convertModelData_219_to_220(const char* filename);

const char* convertRadioData_219_to_220();
const char* convertRadioData_219_to_220(const char* path);

// Conversions 220 to 221
const char* convertModelData_220_to_221(uint8_t id);
const char* convertModelData_220_to_221(const char* filename);

const char* convertRadioData_220_to_221();
const char* convertRadioData_220_to_221(const char* path);
