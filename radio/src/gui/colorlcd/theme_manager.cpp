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
#include "theme_manager.h"

#include "../../storage/sdcard_common.h"
#include "../../storage/yaml/yaml_bits.h"
#include "../../storage/yaml/yaml_tree_walker.h"
#include "theme.h"
#include "themes/etx_lv_theme.h"
#include "view_main.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

#define MAX_FILES 9

constexpr const char* RGBSTRING = "RGB(";

ThemePersistance themePersistance;

// prototype for SD card function to read a YAML file
// TODO: should this be in sdcard header file?
enum class ChecksumResult;
extern const char* readYamlFile(const char* fullpath,
                                const YamlParserCalls* calls, void* parser_ctx,
                                ChecksumResult* checksum_result);

static uint32_t r_color(const YamlNode* node, const char* val, uint8_t val_len)
{
  if ((strncmp(val, RGBSTRING, strlen(RGBSTRING)) == 0) &&
      (val[val_len - 1] == ')')) {
    int r, g, b;
    int numTokens = sscanf(val, "RGB(%i,%i,%i)", &r, &g, &b);

    if (numTokens == 3) return RGB(r, g, b);

  } else if (val_len > 2 && val[0] == '0' && (val[1] == 'x' || val[1] == 'X')) {
    val += 2;
    val_len -= 2;

    auto rgb24 = yaml_hex2uint(val, val_len);
    return RGB((rgb24 & 0xFF0000) >> 16, (rgb24 & 0xFF00) >> 8, rgb24 & 0xFF);
  }

  TRACE("Theme: Invalid color value");
  return 0;
}

static bool w_color(const YamlNode* node, uint32_t val, yaml_writer_func wf,
                    void* opaque)
{
  uint32_t color = (uint32_t)GET_RED(val) << 16 |
                   (uint32_t)GET_GREEN(val) << 8 | (uint32_t)GET_BLUE(val);

  if (!wf(opaque, "0x", 2)) return false;
  return wf(opaque, yaml_rgb2hex(color), 3 * 2);
}

PACK(struct YAMLThemeSummary {
  char name[SELECTED_THEME_NAME_LEN + 1];
  char author[AUTHOR_LENGTH + 1];
  char info[INFO_LENGTH + 1];
});

PACK(struct YAMLThemeColors {
  uint32_t colors[LCD_COLOR_COUNT - 1];  // We don't use CUSTOM
});

PACK(struct YAMLTheme {
  struct YAMLThemeSummary summary;
  struct YAMLThemeColors colors;

  YAMLTheme() { memset(this, 0, sizeof(YAMLTheme)); }
});

static const struct YamlNode struct_YAMLThemeSummary[] = {
    YAML_STRING("name", (SELECTED_THEME_NAME_LEN + 1)),
    YAML_STRING("author", (AUTHOR_LENGTH + 1)),
    YAML_STRING("info", (INFO_LENGTH + 1)), YAML_END};

static const struct YamlNode struct_YAMLThemeColors[] = {
    YAML_UNSIGNED_CUST("PRIMARY1", 32, r_color, w_color),
    YAML_UNSIGNED_CUST("PRIMARY2", 32, r_color, w_color),
    YAML_UNSIGNED_CUST("PRIMARY3", 32, r_color, w_color),
    YAML_UNSIGNED_CUST("SECONDARY1", 32, r_color, w_color),
    YAML_UNSIGNED_CUST("SECONDARY2", 32, r_color, w_color),
    YAML_UNSIGNED_CUST("SECONDARY3", 32, r_color, w_color),
    YAML_UNSIGNED_CUST("FOCUS", 32, r_color, w_color),
    YAML_UNSIGNED_CUST("EDIT", 32, r_color, w_color),
    YAML_UNSIGNED_CUST("ACTIVE", 32, r_color, w_color),
    YAML_UNSIGNED_CUST("WARNING", 32, r_color, w_color),
    YAML_UNSIGNED_CUST("DISABLED", 32, r_color, w_color),
    YAML_END};

// This hack is required to ensure the YAML file written is backward compatible
// with the old parser. Otherwise older versions of EdgeTX will not load the
// theme files.
// TODO: Remove this sometime in the future
static const struct YamlNode w_struct_YAMLTheme[] = {
    YAML_STRUCT("---\r\nsummary",
                (SELECTED_THEME_NAME_LEN + AUTHOR_LENGTH + INFO_LENGTH + 3) * 8,
                struct_YAMLThemeSummary, NULL),
    YAML_STRUCT("colors", (LCD_COLOR_COUNT - 1) * 32, struct_YAMLThemeColors,
                NULL),
    YAML_END};
static const struct YamlNode r_struct_YAMLTheme[] = {
    YAML_STRUCT("summary",
                (SELECTED_THEME_NAME_LEN + AUTHOR_LENGTH + INFO_LENGTH + 3) * 8,
                struct_YAMLThemeSummary, NULL),
    YAML_STRUCT("colors", (LCD_COLOR_COUNT - 1) * 32, struct_YAMLThemeColors,
                NULL),
    YAML_END};

static const char* const colorNames[LCD_COLOR_COUNT] = {
    STR_THEME_COLOR_PRIMARY1,   STR_THEME_COLOR_PRIMARY2,
    STR_THEME_COLOR_PRIMARY3,   STR_THEME_COLOR_SECONDARY1,
    STR_THEME_COLOR_SECONDARY2, STR_THEME_COLOR_SECONDARY3,
    STR_THEME_COLOR_FOCUS,      STR_THEME_COLOR_EDIT,
    STR_THEME_COLOR_ACTIVE,     STR_THEME_COLOR_WARNING,
    STR_THEME_COLOR_DISABLED,   STR_THEME_COLOR_CUSTOM,
};

ThemeFile::ThemeFile(std::string themePath, bool loadYAML) : path(themePath)
{
  if (loadYAML && path.size()) {
    deSerialize();
  }

  auto found = path.rfind('/');
  if (found != std::string::npos) {
    int n = 0;
    while (n < MAX_FILES) {
      auto baseFileName(path.substr(0, found + 1) +
                        (n != 0 ? "screenshot" + std::to_string(n) : "logo") +
                        ".png");
      if (isFileAvailable(baseFileName.c_str(), true)) {
        _imageFileNames.emplace_back(baseFileName);
      } else {
        break;
      }

      n++;
    }
  }
}

ThemeFile& ThemeFile::operator=(const ThemeFile& theme)
{
  if (this == &theme) return *this;

  path = theme.path;
  strAppend(name, theme.name, SELECTED_THEME_NAME_LEN);
  strAppend(author, theme.author, AUTHOR_LENGTH);
  strAppend(info, theme.info, INFO_LENGTH);
  colorList.assign(theme.colorList.begin(), theme.colorList.end());
  _imageFileNames.assign(theme._imageFileNames.begin(),
                         theme._imageFileNames.end());

  return *this;
};

std::vector<std::string> ThemeFile::getThemeImageFileNames()
{
  return _imageFileNames;
}

void ThemeFile::serialize()
{
  struct YAMLTheme yt;
  struct YamlNode themeRootNode = YAML_ROOT(w_struct_YAMLTheme);

  strAppend(yt.summary.name, name, SELECTED_THEME_NAME_LEN);
  strAppend(yt.summary.author, author, AUTHOR_LENGTH);
  strAppend(yt.summary.info, info, INFO_LENGTH);
  for (auto colorEntry : colorList) {
    yt.colors.colors[colorEntry.colorNumber] = colorEntry.colorValue;
  }

  auto err = writeFileYaml(path.c_str(), &themeRootNode, (uint8_t*)&yt, 0);
  if (err != nullptr) {
    ALERT(STR_WARNING, err, AU_WARNING1);
  }
}

void ThemeFile::deSerialize()
{
  struct YAMLTheme yt;
  struct YamlNode themeRootNode = YAML_ROOT(r_struct_YAMLTheme);

  YamlTreeWalker tree;
  tree.reset(&themeRootNode, (uint8_t*)&yt);
  auto err = readYamlFile(path.c_str(), YamlTreeWalker::get_parser_calls(),
                          &tree, nullptr);

  if (err == nullptr) {
    strAppend(name, yt.summary.name, SELECTED_THEME_NAME_LEN);
    strAppend(author, yt.summary.author, AUTHOR_LENGTH);
    strAppend(info, yt.summary.info, INFO_LENGTH);
    for (int i = 0; i < LCD_COLOR_COUNT - 1; i += 1) {
      colorList.emplace_back(
          ColorEntry{(LcdColorIndex)(i), yt.colors.colors[i]});
    }
  } else {
    ALERT(STR_WARNING, err, AU_WARNING1);
  }
}

void ThemeFile::setColor(LcdColorIndex colorIndex, uint32_t color)
{
  auto colorEntry =
      std::find(colorList.begin(), colorList.end(), ColorEntry{colorIndex, 0});
  if (colorEntry != colorList.end())
    colorEntry->colorValue = color;
  else
    colorList.emplace_back(ColorEntry{colorIndex, color});
}

void ThemeFile::applyColors()
{
  for (auto color : colorList) {
    lcdColorTable[color.colorNumber] = color.colorValue;
  }
}

void ThemeFile::applyBackground()
{
  auto instance = EdgeTxTheme::instance();
  std::string backgroundImageFileName(getPath());
  auto pos = backgroundImageFileName.rfind('/');
  if (pos != std::string::npos) {
    auto rootDir = backgroundImageFileName.substr(0, pos + 1);
    rootDir = rootDir + "background_" + std::to_string(LCD_W) + "x" +
              std::to_string(LCD_H) + ".png";

    if (isFileAvailable(rootDir.c_str())) {
      instance->setBackgroundImageFileName((char*)rootDir.c_str());
      return;
    }
  }

  // Use EdgeTxTheme default background
  // TODO: This needs to be made user configurable
  instance->setBackgroundImageFileName("");
}

void ThemeFile::applyTheme()
{
  applyColors();
  applyBackground();
  EdgeTxTheme::instance()->update();
}

// avoid leaking memory
void ThemePersistance::clearThemes()
{
  for (auto theme : themes) {
    delete theme;
  }
  themes.clear();
}

void ThemePersistance::scanThemeFolder(char* fullPath)
{
  strncat(fullPath, "/theme.yml", FF_MAX_LFN);
  if (isFileAvailable(fullPath, true)) {
    TRACE("scanForThemes: found file %s", fullPath);
    themes.emplace_back(new ThemeFile(fullPath));
  }
}

void ThemePersistance::scanForThemes()
{
  clearThemes();

  DIR dir;
  FILINFO fno;

  char fullPath[FF_MAX_LFN + 1];

  strAppend(fullPath, THEMES_PATH, FF_MAX_LFN);

  TRACE("opening directory: %s", fullPath);
  FRESULT res = f_opendir(&dir, fullPath);  // Open the directory
  if (res == FR_OK) {
    TRACE("scanForThemes: open successful");
    // read all entries
    bool firstTime = true;
    for (;;) {
      res = sdReadDir(&dir, &fno, firstTime);

      if (res != FR_OK || fno.fname[0] == 0)
        break;  // Break on error or end of dir

      if (strlen((const char*)fno.fname) > SD_SCREEN_FILE_LENGTH) continue;
      if (fno.fattrib & AM_DIR) {
        char themePath[FF_MAX_LFN + 1];
        char* s = strAppend(themePath, fullPath, FF_MAX_LFN);
        s = strAppend(s, "/", FF_MAX_LFN - (s - themePath));
        strAppend(s, fno.fname, FF_MAX_LFN - (s - themePath));
        scanThemeFolder(themePath);
      }
    }

    f_closedir(&dir);
    std::sort(themes.begin(), themes.end(), [](ThemeFile* a, ThemeFile* b) {
      return strcmp(a->getName(), b->getName()) < 0;
    });
  }
}

void ThemePersistance::refresh()
{
  scanForThemes();
  insertDefaultTheme();
}

void ThemePersistance::loadDefaultTheme()
{
  refresh();

  int index = 0;
  bool found = false;

  // Load theme from 'selectedtheme.txt' file, if not set in radio settings
  // TODO: remove this sometime in the future
  if (g_eeGeneral.selectedTheme[0] == 0) {
    constexpr const char* SELECTED_THEME_FILE =
        THEMES_PATH "/selectedtheme.txt";

    FIL file;
    FRESULT status = f_open(&file, SELECTED_THEME_FILE, FA_READ);

    if (status == FR_OK) {
      char line[256];
      unsigned int len;

      status = f_read(&file, line, 256, &len);
      if (status == FR_OK) {
        line[len] = '\0';

        for (auto theme : themes) {
          if (theme->getPath() == std::string(line)) {
            found = true;
            break;
          }
          index++;
        }

        // Force default if no match for last selected theme
        if (!found) index = 0;
      }

      f_close(&file);

      // Delete old file
      f_unlink(SELECTED_THEME_FILE);
    }

    // Save selected theme (sets to default if nothing found)
    setDefaultTheme(index);

    // Reset values (used below);
    index = 0;
    found = false;
  }

  for (auto theme : themes) {
    if (strncmp(theme->getName(), g_eeGeneral.selectedTheme,
                SELECTED_THEME_NAME_LEN) == 0) {
      found = true;
      break;
    }
    index++;
  }

  // Force default if no match for last selected theme
  if (!found) index = 0;

  applyTheme(index);
  setThemeIndex(index);
}

char** ThemePersistance::getColorNames() { return (char**)colorNames; }

bool ThemePersistance::deleteThemeByIndex(int index)
{
  // greater than 0 is intentional here.  cant delete default theme.
  if (index > 0 && index < (int)themes.size()) {
    ThemeFile* theme = themes[index];

    char newFile[FF_MAX_LFN + 10];
    strAppend(newFile, theme->getPath().c_str(), FF_MAX_LFN);
    strcat(newFile, ".deleted");

    if (isFileAvailable(newFile, true)) {
      // .deleted file already exists, remove it
      f_unlink(newFile);
    }

    // for now we are just renaming the file so we don't find it
    FRESULT status = f_rename(theme->getPath().c_str(), newFile);
    refresh();

    // make sure currentTheme stays in bounds
    if (getThemeIndex() >= (int)themes.size()) setThemeIndex(themes.size() - 1);

    return status == FR_OK;
  }
  return false;
}

bool ThemePersistance::createNewTheme(std::string name, ThemeFile& theme)
{
  char fullPath[FF_MAX_LFN + 1];
  char* s = strAppend(fullPath, THEMES_PATH, FF_MAX_LFN);
  s = strAppend(s, "/", FF_MAX_LFN - (s - fullPath));
  s = strAppend(s, name.c_str(), FF_MAX_LFN - (s - fullPath));

  if (!isFileAvailable(THEMES_PATH)) {
    FRESULT result = f_mkdir(THEMES_PATH);
    if (result != FR_OK) return false;
  }

  FRESULT result = f_mkdir(fullPath);
  s = strAppend(s, "/", FF_MAX_LFN - (s - fullPath));
  strAppend(s, "theme.yml", FF_MAX_LFN - (s - fullPath));
  if (result == FR_EXIST) {
    if (isFileAvailable(fullPath, true)) {
      POPUP_WARNING(STR_THEME_EXISTS);
      return false;
    }
  } else if (result != FR_OK) return false;
  theme.setPath(fullPath);
  theme.serialize();
  return true;
}

void ThemePersistance::setDefaultTheme(int index)
{
  if (index >= 0 && index < (int)themes.size()) {
    strAppend(g_eeGeneral.selectedTheme, themes[index]->getName(),
              SELECTED_THEME_NAME_LEN);
    SET_DIRTY();
    currentTheme = index;
  }
}

class DefaultEdgeTxTheme : public ThemeFile
{
 public:
  DefaultEdgeTxTheme() : ThemeFile(THEMES_PATH "/EdgeTX/", false)
  {
    setName("EdgeTX Default");
    setAuthor("EdgeTX Team");
    setInfo("Default EdgeTX Color Scheme");

    // initializze the default color table
    for (uint8_t i = COLOR_THEME_PRIMARY1_INDEX;
         i <= COLOR_THEME_DISABLED_INDEX; i += 1)
      colorList.emplace_back(ColorEntry{(LcdColorIndex)i, defaultColors[i]});
  }
};

void ThemePersistance::insertDefaultTheme()
{
  auto themeFile = new DefaultEdgeTxTheme();
  themes.insert(themes.begin(), themeFile);
}
