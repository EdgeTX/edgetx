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
#include "radio_theme.h"
#include <algorithm>

#include "bitmapbuffer.h"
#include "libopenui.h"
#include "opentx.h"
#include "file_preview.h"
#include "theme_manager.h"
#include "view_main.h"
#include "widget_settings.h"
#include "sliders.h"
#include "color_list.h"
#include "color_editor_popup.h"
#include "color_editor.h"
#include "listbox.h"
#include "preview_window.h"

#if (LCD_W > LCD_H)
  #define COLOR_PREVIEW_WIDTH 18
  #define COLOR_PREVIEW_HEIGHT  (LCD_H - TOPBAR_HEIGHT - 30)
  #define LEFT_LIST_WIDTH (LCD_W / 2) - COLOR_PREVIEW_WIDTH
  #define LEFT_LIST_HEIGHT (LCD_H - TOPBAR_HEIGHT - 37)
#else
  #define COLOR_PREVIEW_WIDTH 18
  #define COLOR_PREVIEW_HEIGHT  (LCD_H - TOPBAR_HEIGHT - 30)
  #define LEFT_LIST_WIDTH LCD_W
  #define LEFT_LIST_HEIGHT (LCD_H / 2 - 38)
#endif

#define MARGIN_WIDTH 5

#if (LCD_W > LCD_H)
constexpr int BUTTON_HEIGHT = 30;
constexpr int BUTTON_WIDTH  = 75;
constexpr LcdFlags textFont = FONT(STD);
constexpr rect_t detailsDialogRect = {50, 50, 400, 170};
constexpr int labelWidth = 150;

constexpr int COLOR_BOX_LEFT = 3;
constexpr int COLOR_BOX_WIDTH = 45;
constexpr int COLOR_BOX_HEIGHT = 27;
#else
constexpr int BUTTON_HEIGHT = 25;
constexpr int BUTTON_WIDTH  = 50;
constexpr LcdFlags textFont = FONT(XS);
constexpr rect_t detailsDialogRect = {5, 50, LCD_W - 10, 340};
constexpr int labelWidth = 120;

constexpr int COLOR_BOX_LEFT = 3;
constexpr int COLOR_BOX_WIDTH = 55;
constexpr int COLOR_BOX_HEIGHT = 32;

constexpr int COLOR_BUTTON_WIDTH = COLOR_BOX_WIDTH;
constexpr int COLOR_BUTTON_HEIGHT = 20;
#endif

#define COLOR_LIST_WIDTH ((LCD_W * 3)/10)
#define COLOR_LIST_HEIGHT (LCD_H - TOPBAR_HEIGHT)


class ThemeDetailsDialog: public Dialog
{
  public:
    ThemeDetailsDialog(Window *parent, ThemeFile theme, std::function<void (ThemeFile theme)> saveHandler = nullptr) :
      Dialog(parent, STR_EDIT_THEME_DETAILS, detailsDialogRect),
      theme(theme),
      saveHandler(saveHandler)
    {
      FormGridLayout grid(detailsDialogRect.w);
      grid.setLabelWidth(labelWidth);
      grid.spacer(8);

      new StaticText(&content->form, grid.getLabelSlot(), STR_NAME, 0, COLOR_THEME_PRIMARY1);
      new TextEdit(&content->form, grid.getFieldSlot(), this->theme.getName(), NAME_LENGTH);
      grid.nextLine();
      new StaticText(&content->form, grid.getLabelSlot(), STR_AUTHOR, 0, COLOR_THEME_PRIMARY1);
      new TextEdit(&content->form, grid.getFieldSlot(), this->theme.getAuthor(), AUTHOR_LENGTH);
      grid.nextLine();
      new StaticText(&content->form, grid.getLineSlot(), STR_DESCRIPTION, 0, COLOR_THEME_PRIMARY1);
      grid.nextLine();
      new TextEdit(&content->form, grid.getLineSlot(), this->theme.getInfo(), INFO_LENGTH);
      grid.nextLine();

      rect_t r = {detailsDialogRect.w - (BUTTON_WIDTH + 5), grid.getWindowHeight() + 5, BUTTON_WIDTH, BUTTON_HEIGHT };
      new TextButton(&content->form, r, STR_SAVE, [=] () {
        if (saveHandler != nullptr)
          saveHandler(this->theme);
        deleteLater();
        return 0;
      }, BUTTON_BACKGROUND | OPAQUE, textFont);
      r.x -= (BUTTON_WIDTH + 5);
      new TextButton(&content->form, r, STR_CANCEL, [=] () {
        deleteLater();
        return 0;
      }, BUTTON_BACKGROUND | OPAQUE, textFont);
    }

    void onEvent(event_t event) override
    {
      if (event == EVT_KEY_BREAK(KEY_EXIT)) {
        deleteLater();
      }
    }

  protected:
    ThemeFile theme;
    std::function<void(ThemeFile theme)> saveHandler = nullptr;
};

class ColorSquare : public FormField
{
  public:
    ColorSquare(Window *window, const rect_t &rect, uint32_t color, WindowFlags windowFlags = 0)
      : FormField(window, rect, windowFlags | NO_FOCUS),
      _color(color)
    {
    }

    void paint (BitmapBuffer *dc) override
    {
      dc->drawSolidFilledRect(0, 0, rect.w, rect.h, COLOR2FLAGS(_color));
      dc->drawSolidRect(0, 0, rect.w, rect.h, 1, COLOR2FLAGS(BLACK));
    }

    void setColorToEdit(uint32_t colorEntry)
    {
      _color = colorEntry;

      invalidate();
    }

  protected:
    uint32_t _color;
};

void setHexStr(StaticText* hexBox, uint32_t rgb)
{
  auto r = GET_RED(rgb), g = GET_GREEN(rgb), b = GET_BLUE(rgb);
  char hexstr[80];
  sprintf(hexstr, "%02X%02X%02X\n", (uint16_t)r, (uint16_t)g, (uint16_t)b);
  hexBox->setText(hexstr);
}

class ColorEditPage : public Page
{
public:
  ColorEditPage(ThemeFile theme, LcdColorIndex indexOfColor, 
                std::function<void (uint32_t rgb)> setValue = nullptr) :
    Page(ICON_RADIO_EDIT_THEME),
    _setValue(std::move(setValue)),
    _indexOfColor(indexOfColor),
    _theme(theme)
  {
    buildHead(&header);
    buildBody(&body); 
  }

  void onEvent(event_t event ) override
  {
    if (event == EVT_KEY_BREAK(KEY_TELEM)) {
      _colorEditor->setNextFocusBar();
    } else if (event == EVT_KEY_BREAK(KEY_PGUP) || event == EVT_KEY_BREAK(KEY_PGDN)) {
      int direction = event == EVT_KEY_BREAK(KEY_PGUP) ? -1 : 1;
      _activeTab = _activeTab + direction;
      if (_activeTab < 0) _activeTab = 1;
      if (_activeTab >= (int) _tabs.size()) _activeTab = 0;
      for (auto i = 0; i < (int) _tabs.size(); i++)
        _tabs[i]->check(i == _activeTab);
      _colorEditor->setColorEditorType(_activeTab == 1 ? HSV_COLOR_EDITOR : RGB_COLOR_EDITOR);
      onKeyPress();
    } else {
      Page::onEvent(event);
    }
  }

protected:
  std::function<void (uint32_t rgb)> _setValue;
  LcdColorIndex _indexOfColor;
  ThemeFile  _theme;
  TextButton *_cancelButton;
  ColorEditor *_colorEditor;
  PreviewWindow *_previewWindow;
  std::vector<MyButton *> _tabs;
  int _activeTab = 0;
  ColorSquare *_colorSquare;
  StaticText *_hexBox;

  void buildBody(FormWindow* window)
  {
    rect_t r;

    // hexBox
    r = { COLOR_BOX_LEFT + COLOR_BOX_WIDTH + 5, 4, 90, 30 };
    _hexBox = new StaticText(window, r, "", 0, COLOR_THEME_PRIMARY1 | FONT(L) | RIGHT);
    setHexStr(_hexBox, _theme.getColorEntryByIndex(_indexOfColor)->colorValue);

    r = { COLOR_BOX_LEFT, COLOR_BOX_HEIGHT + 7, COLOR_LIST_WIDTH, window->height() - COLOR_BOX_HEIGHT - 7};
    _colorEditor = new ColorEditor(window, r, _theme.getColorEntryByIndex(_indexOfColor)->colorValue,
      [=](uint32_t rgb) {
        _theme.setColor(_indexOfColor, rgb);
        if (_colorSquare != nullptr) {
          _colorSquare->setColorToEdit(rgb);
        }
        if (_previewWindow != nullptr) {
          _previewWindow->setColorList(_theme.getColorList());
        }
        if (_hexBox != nullptr) {
          setHexStr(_hexBox, rgb);
        }
        if (_setValue != nullptr)
          _setValue(rgb);
      });
    _colorEditor->setColorEditorType(HSV_COLOR_EDITOR);
    _activeTab = 1;

    r = { COLOR_BOX_LEFT, 4, COLOR_BOX_WIDTH, COLOR_BOX_HEIGHT };
    _colorSquare = new ColorSquare(window, r, _theme.getColorEntryByIndex(_indexOfColor)->colorValue);

    r = LCD_W > LCD_H ?
          rect_t { COLOR_LIST_WIDTH + MARGIN_WIDTH, 4, LCD_W - COLOR_LIST_WIDTH - MARGIN_WIDTH * 2, COLOR_LIST_HEIGHT } :
          rect_t { 0, LEFT_LIST_HEIGHT + 4,  LEFT_LIST_WIDTH, LEFT_LIST_HEIGHT - 4 };
    _previewWindow = new PreviewWindow(window, r, _theme.getColorList());
  }

  void buildHead(PageHeader* window)
  {
    LcdFlags flags = 0;
    if (LCD_W < LCD_H) {
      flags = FONT(XS);
    }

    // page title
    new StaticText(window,
      { PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
       PAGE_LINE_HEIGHT },
      "Edit Color", 0, COLOR_THEME_PRIMARY2 | flags);
    new StaticText(window,
      { PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
       LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT },
      ThemePersistance::getColorNames()[(int)_indexOfColor], 0, COLOR_THEME_PRIMARY2 | flags);

    // page tabs
    rect_t r = { LCD_W - (BUTTON_WIDTH + 5), 6, BUTTON_WIDTH, BUTTON_HEIGHT };
    _tabs.emplace_back(new MyButton(window, r, "RGB", 0, COLOR_THEME_PRIMARY1));
    r.x -= (BUTTON_WIDTH + 5);
    _tabs.emplace_back(new MyButton(window, r, "HSV", 0, COLOR_THEME_PRIMARY1));
    _tabs[1]->check(true);
  }
};

class ThemeEditPage : public Page
{
  public:
    explicit ThemeEditPage(ThemeFile theme, std::function<void (ThemeFile &theme)> saveHandler = nullptr) :
      Page(ICON_RADIO_EDIT_THEME),
      _theme(theme),
      page(this),
      saveHandler(std::move(saveHandler))
    {
      buildBody(&body);
      buildHeader(&header);
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override
    {
      if (event == EVT_KEY_BREAK(KEY_PGUP)) {
        onKeyPress();
        FormField *focus = dynamic_cast<FormField *>(getFocus());
        if (focus != nullptr && focus->getPreviousField()) {
          focus->getPreviousField()->setFocus(SET_FOCUS_BACKWARD, focus);
        }
      } else if (event == EVT_KEY_BREAK(KEY_PGDN)) {
        onKeyPress();
        FormField *focus = dynamic_cast<FormField *>(getFocus());
        if (focus != nullptr && focus->getNextField()) {
          focus->getNextField()->setFocus(SET_FOCUS_FORWARD, focus);
        }
      } else {
        Window::onEvent(event);
      }
    }
#endif

    void deleteLater(bool detach = true, bool trash = true) override
    {
      if (_dirty) {
        if (confirmationDialog("Save Theme?", _theme.getName())) {
          if (saveHandler != nullptr) {
            saveHandler(_theme);
          }
        }
      }

      Page::deleteLater(detach, trash);
    }

    void editColorPage()
    {
      auto colorEntry = _cList->getSelectedColor();
      new ColorEditPage(_theme, colorEntry.colorNumber, 
      [=] (uint32_t color) {
        _dirty = true;
        _theme.setColor(colorEntry.colorNumber, color);
        _cList->setColorList(_theme.getColorList());
        _previewWindow->setColorList(_theme.getColorList());
      });
    }

    void buildHeader(FormGroup *window)
    {
      LcdFlags flags = 0;
      if (LCD_W < LCD_H) {
        flags = FONT(XS);
      }

      // page title
      new StaticText(window,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
                     STR_EDIT_THEME, 0, COLOR_THEME_PRIMARY2 | flags);
      _themeName = new StaticText(window,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                      LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                     _theme.getName(), 0, COLOR_THEME_PRIMARY2 | flags);

      // save and cancel
      rect_t r = {LCD_W - (BUTTON_WIDTH + 5), 6, BUTTON_WIDTH, BUTTON_HEIGHT };
      _detailButton = new TextButton(window, r, STR_DETAILS, [=] () {
        new ThemeDetailsDialog(page, _theme, [=] (ThemeFile t) {
          _theme.setAuthor(t.getAuthor());
          _theme.setInfo(t.getInfo());
          _theme.setName(t.getName());

          // update the theme name
          _themeName->setText(_theme.getName());
          _dirty = true;
        });
        return 0;
      }, BUTTON_BACKGROUND | OPAQUE, textFont);

      // setup the prev next controls so save and details are in the mix
      _cList->setNextField(_detailButton);
      _cList->setPreviousField(_detailButton);
      _detailButton->setNextField(_cList);
      _detailButton->setPreviousField(_cList);
    }

    void buildBody(FormGroup *window)
    {
      rect_t r = { 0, 4, COLOR_LIST_WIDTH, COLOR_LIST_HEIGHT};
      _cList = new ColorList(window, r, _theme.getColorList());
      _cList->setLongPressHandler([=] (event_t event) { editColorPage(); });
      _cList->setPressHandler([=] (event_t event) { editColorPage(); });

      if (LCD_W > LCD_H) {
        r = { COLOR_LIST_WIDTH + MARGIN_WIDTH, 4, LCD_W - COLOR_LIST_WIDTH - MARGIN_WIDTH*2, COLOR_LIST_HEIGHT };
      } else {
        r = { 0, LEFT_LIST_HEIGHT + 4,  LEFT_LIST_WIDTH, LEFT_LIST_HEIGHT - 4};
      }
      _previewWindow = new PreviewWindow(window, r, _theme.getColorList());
    }

  protected:
    bool _dirty = false;
    ThemeFile _theme;
    Page *page;
    std::function<void(ThemeFile &theme)> saveHandler = nullptr;
    PreviewWindow *_previewWindow = nullptr;
    ColorList *_cList = nullptr;
    StaticText *_themeName = nullptr;
    TextButton *_saveButton;
    TextButton *_detailButton;
};

ThemeSetupPage::ThemeSetupPage() : PageTab(STR_THEME_EDITOR, ICON_RADIO_EDIT_THEME) {}
ThemeSetupPage::~ThemeSetupPage()
{
}

void ThemeSetupPage::displayThemeMenu(Window *window, ThemePersistance *tp)
{
  auto menu = new Menu(listBox,false);

  // you cant activate the active theme
  if (listBox->getSelected() != tp->getThemeIndex()) {
    menu->addLine(STR_ACTIVATE, [=]() {
      tp->applyTheme(listBox->getSelected());
      tp->setDefaultTheme(listBox->getSelected());
      nameText->setTextFlags(COLOR_THEME_PRIMARY1);
      authorText->setTextFlags(COLOR_THEME_PRIMARY1);
      listBox->setActiveIndex(tp->getThemeIndex());
    });
  }

  // you cant edit the default theme
  if (listBox->getSelected() != 0) {
    menu->addLine(STR_EDIT,
      [=] () {
        auto theme = tp->getThemeByIndex(currentTheme);
        if (theme == nullptr) return;
        
        new ThemeEditPage(*theme, 
          [=](ThemeFile &theme) {
            auto curTheme = tp->getThemeByIndex(currentTheme);
            if (curTheme != nullptr) {
              curTheme->setName(theme.getName());
              curTheme->setAuthor(theme.getAuthor());
              curTheme->setInfo(theme.getInfo());

              // update the colors that were edited
              int n = 0;
              for (auto color : theme.getColorList()) {
                curTheme->setColorByIndex(n, color.colorValue);
                n++;
              }

              // save it to disk so it will come back the next time.
              curTheme->serialize();

              // if the active theme was edited then activate it.
              if (listBox->getSelected() == tp->getThemeIndex())
                tp->applyTheme(listBox->getSelected());

              // the list of theme names might have changed
              listBox->setNames(tp->getNames());
            }
          });
    });
  }

  menu->addLine(STR_DUPLICATE, [=] () {
    ThemeFile newTheme;

    new ThemeDetailsDialog(window, newTheme, [=] (ThemeFile theme) {
      if (strlen(theme.getName()) != 0) {
        char path[NAME_LENGTH + 20];
        strncpy(path, theme.getName(), NAME_LENGTH + 19);
        removeAllWhiteSpace(path);
        strncat(path, ".yml", NAME_LENGTH + 19);
        theme.setPath(path);

        // use the current themes color list to make the new theme
        auto curTheme = tp->getCurrentTheme();
        for (auto color : curTheme->getColorList())
          theme.setColor(color.colorNumber, color.colorValue);

        theme.serialize();
        tp->refresh();
        listBox->setNames(tp->getNames());
        listBox->setSelected(currentTheme);
      }
    });
  });

  // you cant delete the default theme or the currently active theme
  if (listBox->getSelected() != 0 && listBox->getSelected() != tp->getThemeIndex()) {
    menu->addLine(STR_DELETE, [=] () {
      if (confirmationDialog("Delete Theme?", tp->getThemeByIndex(listBox->getSelected())->getName())) {
        tp->deleteThemeByIndex(listBox->getSelected());
        listBox->setNames(tp->getNames());
        listBox->setSelected(currentTheme);
      }
    });
  }
}

void ThemeSetupPage::setupListbox(FormWindow *window, rect_t r, ThemePersistance *tp)
{
  listBox = new ListBox(
    window, r, tp->getNames(),
      [=]() { 
        return currentTheme;
      },
      [=](uint8_t value) {
        if (themeColorPreview && authorText && nameText && fileCarosell) {
          ThemeFile *theme = tp->getThemeByIndex(value);
          themeColorPreview->setColorList(theme->getColorList());
          authorText->setText(theme->getAuthor());
          nameText->setText(theme->getName());
          fileCarosell->setFileNames(theme->getThemeImageFileNames());
        }
        currentTheme = value;
      });
  listBox->setActiveIndex(tp->getThemeIndex());
  listBox->setTitle(STR_THEME + std::string("s"));
  listBox->setLongPressHandler([=] (event_t event) { displayThemeMenu(window, tp); });
  listBox->setPressHandler([=] (event_t event) { displayThemeMenu(window, tp); });
}

void ThemeSetupPage::build(FormWindow *window)
{
  auto tp = ThemePersistance::instance();
  auto theme = tp->getCurrentTheme();
  currentTheme = tp->getThemeIndex();

  themeColorPreview = nullptr;
  listBox = nullptr;
  fileCarosell = nullptr;
  nameText = nullptr;
  authorText = nullptr;
  
  // create listbox and setup menus
  rect_t r = { 2, 3, LEFT_LIST_WIDTH, LEFT_LIST_HEIGHT };
  setupListbox(window, r, tp);

  if (LCD_W > LCD_H) {
    r.x = LEFT_LIST_WIDTH + MARGIN_WIDTH;
    r.w = LCD_W - r.x;
  } else {
    r.y += LEFT_LIST_HEIGHT + 4;
    r.x = 0;
    r.w = LCD_W;
  }

  // setup ThemeColorPreview()
  auto colorList = theme != nullptr ? theme->getColorList() : std::vector<ColorEntry>();
  rect_t colorPreviewRect = {LEFT_LIST_WIDTH + 6, 0, COLOR_PREVIEW_WIDTH, COLOR_PREVIEW_HEIGHT};
  themeColorPreview = new ThemeColorPreview(window, colorPreviewRect, colorList);
  
  // setup FileCarosell()
  r.h = 130;
  r.w -= COLOR_PREVIEW_WIDTH;
  r.x += COLOR_PREVIEW_WIDTH;
  auto fileNames = theme != nullptr ? theme->getThemeImageFileNames() : std::vector<std::string>();
  fileCarosell = new FileCarosell(window, r, fileNames, listBox);

  // author and name of theme on right side of screen
  r.x += 7;
  r.y += 130;
  r.h = 20;
  nameText = new StaticText(window, r, theme != nullptr ? theme->getName() : "", 0, 
                            COLOR_THEME_PRIMARY1);
  if (theme != nullptr && strlen(theme->getAuthor())) {
    r.y += 20;
    char author[256];
    strcpy(author, "By: ");
    strcat(author, theme->getAuthor());
    authorText = new StaticText(window, r, author, 0, COLOR_THEME_PRIMARY1);
  }
}
