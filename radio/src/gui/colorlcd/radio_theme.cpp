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

#include "libopenui.h"
#include "opentx.h"
#include "file_preview.h"
#include "theme_manager.h"
#include "view_main.h"
#include "color_list.h"
#include "listbox.h"
#include "preview_window.h"

constexpr int COLOR_PREVIEW_SIZE = 18;

#if LCD_W > LCD_H
  constexpr int LIST_WIDTH =          ((LCD_W - 12) / 2 - COLOR_PREVIEW_SIZE);
  constexpr int COLOR_LIST_WIDTH =    ((LCD_W * 3)/10);
#else
  constexpr int LIST_HEIGHT =         (LCD_H / 2 - 38);
  constexpr int COLOR_LIST_HEIGHT =   (LCD_H / 2 - 24);
#endif

#if LCD_W > LCD_H
constexpr int BUTTON_WIDTH  = 75;
constexpr rect_t detailsDialogRect = {30, 30, LCD_W - 60, LCD_H - 60};

constexpr int COLOR_BOX_WIDTH = 45;
#else
constexpr int BUTTON_WIDTH  = 65;
constexpr rect_t detailsDialogRect = {10, 50, LCD_W - 20, 220};

constexpr int COLOR_BOX_WIDTH = 55;
#endif

constexpr int BUTTON_HEIGHT = 30;
constexpr int COLOR_BOX_HEIGHT = 30;

constexpr int BOX_MARGIN = 2;
constexpr int MAX_BOX_WIDTH = 15;


class ThemeColorPreview : public FormWindow
{
  public:
    ThemeColorPreview(Window *parent, const rect_t &rect, std::vector<ColorEntry> colorList) :
      FormWindow(parent, rect, NO_FOCUS),
      colorList(colorList)
    {
      padAll(0);
#if LCD_W > LCD_H
      setFlexLayout(LV_FLEX_FLOW_COLUMN, BOX_MARGIN);
#else
      setFlexLayout(LV_FLEX_FLOW_ROW, BOX_MARGIN);
#endif
      build();
    }

    void build()
    {
      clear();
      setBoxWidth();
      int size = (boxWidth + BOX_MARGIN) * colorList.size() - BOX_MARGIN;
#if LCD_W > LCD_H
      padTop((height() - size) / 2);
#else
      padLeft((width() - size) / 2);
#endif
      for (auto color: colorList) {
        auto c = lv_obj_create(lvobj);
        lv_obj_set_size(c, boxWidth, boxWidth);
        lv_obj_set_style_border_width(c, 1, 0);
        lv_obj_set_style_border_color(c, makeLvColor(COLOR2FLAGS(BLACK)), 0);
        lv_obj_set_style_border_opa(c, LV_OPA_100, 0);
        lv_obj_set_style_bg_color(c, makeLvColor(COLOR2FLAGS(color.colorValue)), 0);
        lv_obj_set_style_bg_opa(c, LV_OPA_100, 0);
      }
    }

    void setColorList(std::vector<ColorEntry> colorList)
    {
      this->colorList = colorList;
      build();
    }

  protected:
    std::vector<ColorEntry> colorList;
    int boxWidth = MAX_BOX_WIDTH;

    void setBoxWidth()
    {
#if LCD_W > LCD_H
      boxWidth = (height() - (colorList.size() - 1) * BOX_MARGIN) / colorList.size();
#else
      boxWidth = (width() - (colorList.size() - 1) * BOX_MARGIN) / colorList.size();
#endif
      boxWidth = min(boxWidth, MAX_BOX_WIDTH);
    }
};


static const lv_coord_t d_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(3),
                                       LV_GRID_TEMPLATE_LAST};
static const lv_coord_t b_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                       LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

class ThemeDetailsDialog: public Dialog
{
  public:
    ThemeDetailsDialog(Window *parent, ThemeFile theme, std::function<void (ThemeFile theme)> saveHandler = nullptr) :
      Dialog(parent, STR_EDIT_THEME_DETAILS, detailsDialogRect),
      theme(theme),
      saveHandler(saveHandler)
    {
      content->form.padAll(4);
      content->form.padLeft(8);
      content->form.padRight(8);

      lv_obj_set_style_bg_color(content->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY3), 0);
      lv_obj_set_style_bg_opa(content->getLvObj(), LV_OPA_100, LV_PART_MAIN);
      auto form = new FormWindow(&content->form, rect_t{});
      form->setFlexLayout();

      FlexGridLayout grid(d_col_dsc, row_dsc, 2);

      auto line = form->newLine(&grid);
      line->padAll(0);

      new StaticText(line, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
      auto te = new TextEdit(line, rect_t{}, this->theme.getName(), SELECTED_THEME_NAME_LEN);
      lv_obj_set_grid_cell(te->getLvObj(), LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

      line = form->newLine(&grid);
      line->padAll(0);
      line->padTop(2);

      new StaticText(line, rect_t{}, STR_AUTHOR, 0, COLOR_THEME_PRIMARY1);
      te = new TextEdit(line, rect_t{}, this->theme.getAuthor(), AUTHOR_LENGTH);
      lv_obj_set_grid_cell(te->getLvObj(), LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

      FlexGridLayout grid2(b_col_dsc, row_dsc, 2);

      line = form->newLine(&grid2);
      line->padAll(0);

      new StaticText(line, rect_t{}, STR_DESCRIPTION, 0, COLOR_THEME_PRIMARY1);
      line = form->newLine(&grid2);
      line->padAll(0);
      te = new TextEdit(line, rect_t{}, this->theme.getInfo(), INFO_LENGTH);
      lv_obj_set_grid_cell(te->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);

      line = form->newLine(&grid2);
      line->padAll(0);
      line->padTop(10);

      auto button = new TextButton(line, rect_t{0, 0, lv_pct(30), 32}, STR_SAVE, [=] () {
        if (saveHandler != nullptr)
          saveHandler(this->theme);
        deleteLater();
        return 0;
      });
      lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

      button = new TextButton(line, rect_t{0, 0, lv_pct(30), 32}, STR_CANCEL, [=] () {
        deleteLater();
        return 0;
      });
      lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    }

  protected:
    ThemeFile theme;
    std::function<void(ThemeFile theme)> saveHandler = nullptr;
};

class ColorSquare : public Window
{
  public:
    ColorSquare(Window *window, const rect_t &rect, uint32_t color, WindowFlags windowFlags = 0) :
      Window(window, rect, windowFlags | NO_FOCUS),
      _color(color)
    {
      swatch = lv_obj_create(lvobj);
      lv_obj_set_size(swatch, width(), height());
      lv_obj_set_style_border_width(swatch, 1, 0);
      lv_obj_set_style_border_color(swatch, makeLvColor(COLOR2FLAGS(BLACK)), 0);
      lv_obj_set_style_border_opa(swatch, LV_OPA_100, 0);
      lv_obj_set_style_bg_color(swatch, makeLvColor(COLOR2FLAGS(_color)), 0);
      lv_obj_set_style_bg_opa(swatch, LV_OPA_100, 0);
    }

    void setColorToEdit(uint32_t colorEntry)
    {
      _color = colorEntry;
      lv_obj_set_style_bg_color(swatch, makeLvColor(COLOR2FLAGS(_color)), 0);
    }

  protected:
    uint32_t _color;
    lv_obj_t *swatch;
};

class ColorEditPage : public Page
{
  public:
    ColorEditPage(ThemeFile *theme, LcdColorIndex indexOfColor, 
                  std::function<void ()> updateHandler = nullptr) :
      Page(ICON_RADIO_EDIT_THEME),
      _updateHandler(std::move(updateHandler)),
      _indexOfColor(indexOfColor),
      _theme(theme)
    {
      buildBody(&body); 
      buildHead(&header);
    }

    void setActiveColorBar(int activeTab)
    {
      if (activeTab >= 0 && _activeTab <= (int)_tabs.size()) {
        _activeTab = activeTab;
        for (auto i = 0; i < (int) _tabs.size(); i++)
          _tabs[i]->check(i == _activeTab);
        _colorEditor->setColorEditorType(_activeTab == 1 ? HSV_COLOR_EDITOR : RGB_COLOR_EDITOR);
      }
    }

  protected:
    std::function<void ()> _updateHandler;
    LcdColorIndex _indexOfColor;
    ThemeFile *_theme;
    TextButton *_cancelButton;
    ColorEditor *_colorEditor;
    PreviewWindow *_previewWindow = nullptr;
    std::vector<Button *> _tabs;
    int _activeTab = 0;
    ColorSquare *_colorSquare = nullptr;
    StaticText *_hexBox = nullptr;

    void deleteLater(bool detach = true, bool trash = true) override
    {
      if (_updateHandler != nullptr)
        _updateHandler();
      Page::deleteLater(detach, trash);
    }

    void setHexStr(uint32_t rgb)
    {
      if (_hexBox) {
        auto r = GET_RED(rgb), g = GET_GREEN(rgb), b = GET_BLUE(rgb);
        char hexstr[8];
        snprintf(hexstr, sizeof(hexstr), "%02X%02X%02X",
                 (uint16_t)r, (uint16_t)g, (uint16_t)b);
        _hexBox->setText(hexstr);
      }
    }

    void buildBody(FormWindow* window)
    {
      window->padAll(4);
      FormWindow *form = new FormWindow(window, {0, 0, window->width() - 8, window->height() - 8});
      form->padAll(0);
#if LCD_W > LCD_H
      form->setFlexLayout(LV_FLEX_FLOW_ROW, 4);
      rect_t r = {0, 0, COLOR_LIST_WIDTH, form->height()};
#else
      form->setFlexLayout(LV_FLEX_FLOW_COLUMN, 4);
      rect_t r = {0, 0, form->width(), COLOR_LIST_HEIGHT};
#endif

      FormWindow *colForm = new FormWindow(form, r);
      colForm->padAll(0);
      colForm->setFlexLayout(LV_FLEX_FLOW_COLUMN, 4);
      colForm->setWidth(r.w);

#if LCD_W > LCD_H
      r.w = form->width() - COLOR_LIST_WIDTH - 4;
#else
      r.h = form->height() - COLOR_LIST_HEIGHT - 4;
#endif
      _previewWindow = new PreviewWindow(form, r, _theme->getColorList());

      r.w = colForm->width();
      r.h = COLOR_BOX_HEIGHT;

      FormWindow *colBoxForm = new FormWindow(colForm, r);
      colBoxForm->padAll(0);
      colBoxForm->setFlexLayout(LV_FLEX_FLOW_ROW, 2);

      r.h = colForm->height() - COLOR_BOX_HEIGHT - 4;

      _colorEditor = new ColorEditor(colForm, r, _theme->getColorEntryByIndex(_indexOfColor)->colorValue,
        [=](uint32_t rgb) {
          _theme->setColor(_indexOfColor, rgb);
          if (_colorSquare != nullptr) {
            _colorSquare->setColorToEdit(rgb);
          }
          if (_previewWindow != nullptr) {
            _previewWindow->setColorList(_theme->getColorList());
          }
          setHexStr(rgb);
        });
      _colorEditor->setColorEditorType(HSV_COLOR_EDITOR);
      _activeTab = 1;

      r.w = COLOR_BOX_WIDTH;
      r.h = COLOR_BOX_HEIGHT;
      _colorSquare = new ColorSquare(colBoxForm, r, _theme->getColorEntryByIndex(_indexOfColor)->colorValue);

      // hexBox
      r.w = 95;
      _hexBox = new StaticText(colBoxForm, r, "", 0, COLOR_THEME_PRIMARY1 | FONT(L) | RIGHT);
      setHexStr(_theme->getColorEntryByIndex(_indexOfColor)->colorValue);
    }

    void buildHead(PageHeader* window)
    {
      // page title
      header.setTitle(STR_EDIT_COLOR);
      auto t2 = header.setTitle2(ThemePersistance::getColorNames()[(int)_indexOfColor]);
#if LCD_H > LCD_W
      t2->setFont(FONT(XS));
#else
      LV_UNUSED(t2);
#endif

      // page tabs
      rect_t r = { LCD_W - 2*(BUTTON_WIDTH + 5), 6, BUTTON_WIDTH, BUTTON_HEIGHT };
      _tabs.emplace_back(
        new TextButton(window, r, "RGB",
          [=] () {
            setActiveColorBar(0);
            return 1;
          }));
      r.x += (BUTTON_WIDTH + 5);
      _tabs.emplace_back(
        new TextButton(window, r, "HSV", 
          [=] () {
            setActiveColorBar(1);
            return 1;
          }));
      _tabs[1]->check(true);
    }
};

class ThemeEditPage : public Page
{
  public:
    explicit ThemeEditPage(ThemeFile *theme, std::function<void (ThemeFile &theme)> saveHandler = nullptr) :
      Page(ICON_RADIO_EDIT_THEME),
      _theme(*theme),
      page(this),
      saveHandler(std::move(saveHandler))
    {
      buildBody(&body);
      buildHeader(&header);
    }

    void onCancel() override
    {
      if (_dirty) {
        new ConfirmDialog(
            this, STR_SAVE_THEME, _theme.getName(),
            [=]() {
              if (saveHandler != nullptr) {
                saveHandler(_theme);
              }
              deleteLater();
            },
            [=]() {
              deleteLater();
            });
      } else {
        deleteLater();
      }
    }

    void checkEvents() override
    {
      if (!started && _themeName) {
        started = true;
        _themeName->setText(_theme.getName());
      }
      Window::checkEvents();
    }

    void editColorPage()
    {
      auto colorEntry = _cList->getSelectedColor();
      new ColorEditPage(&_theme, colorEntry.colorNumber, 
          [=] () {
            _dirty = true;
            _cList->setColorList(_theme.getColorList());
            _previewWindow->setColorList(_theme.getColorList());
          });
    }

    void buildHeader(FormWindow *window)
    {
      // page title
      header.setTitle(STR_EDIT_THEME);
      _themeName = header.setTitle2(_theme.getName());
#if LCD_H > LCD_W
      _themeName->setFont(FONT(XS));
#endif

      // save and cancel
      rect_t r = {LCD_W - (BUTTON_WIDTH + 5), 6, BUTTON_WIDTH, BUTTON_HEIGHT };
      new TextButton(window, r, STR_DETAILS, [=] () {
        new ThemeDetailsDialog(page, _theme, [=] (ThemeFile t) {
          _theme.setAuthor(t.getAuthor());
          _theme.setInfo(t.getInfo());
          _theme.setName(t.getName());

          // update the theme name
          _themeName->setText(_theme.getName());
          _dirty = true;
        });
        return 0;
      });
    }

    void buildBody(FormWindow *window)
    {
      window->padAll(4);
      FormWindow *form = new FormWindow(window, {0, 0, window->width() - 8, window->height() - 8});
      form->padAll(0);
#if LCD_W > LCD_H
      form->setFlexLayout(LV_FLEX_FLOW_ROW, 4);
      rect_t r = {0, 0, COLOR_LIST_WIDTH, form->height()};
#else
      form->setFlexLayout(LV_FLEX_FLOW_COLUMN, 4);
      rect_t r = {0, 0, form->width(), COLOR_LIST_HEIGHT};
#endif

      _cList = new ColorList(form, r, _theme.getColorList());
      _cList->setLongPressHandler([=] () { editColorPage(); });
      _cList->setPressHandler([=] () { editColorPage(); });

#if LCD_W > LCD_H
      r.w = form->width() - COLOR_LIST_WIDTH - 4;
#else
      r.h = form->height() - COLOR_LIST_HEIGHT - 4;
#endif
      _previewWindow = new PreviewWindow(form, r, _theme.getColorList());
    }

  protected:
    bool _dirty = false;
    bool started = false;
    ThemeFile _theme;
    Page *page;
    std::function<void(ThemeFile &theme)> saveHandler = nullptr;
    PreviewWindow *_previewWindow = nullptr;
    ColorList *_cList = nullptr;
    StaticText *_themeName = nullptr;
};

ThemeSetupPage::ThemeSetupPage() : PageTab(STR_THEME_EDITOR, ICON_RADIO_EDIT_THEME) {}

void ThemeSetupPage::setAuthor(ThemeFile *theme)
{
  char author[256] = "";
  if (theme && (strlen(theme->getAuthor()) > 0)) {
    strcpy(author, "By: ");
    strcat(author, theme->getAuthor());
  }
  authorText->setText(author);
}

void ThemeSetupPage::setName(ThemeFile *theme)
{
  if (theme && (strlen(theme->getName()) > 0)) {
    nameText->setText(theme->getName());
  } else {
    nameText->setText("");
  }
}

bool isTopWindow(Window *window)
{
  Window *parent = window->getParent();
  if (parent != nullptr) {
    parent = parent->getParent();
    return parent == Layer::back();
  }
  return false;
}

void ThemeSetupPage::checkEvents()
{
  PageTab::checkEvents();

  if (fileCarosell)
    fileCarosell->pause(!isTopWindow(pageWindow));

  if (!started && nameText && authorText) {
    auto theme = ThemePersistance::instance()->getCurrentTheme();
    started = true;
    setName(theme);
    setAuthor(theme);
  }
}

void ThemeSetupPage::displayThemeMenu(Window *window, ThemePersistance *tp)
{
  auto menu = new Menu(listBox,false);

  // you can't activate the active theme
  if (listBox->getSelected() != tp->getThemeIndex()) {
    menu->addLine(STR_ACTIVATE, [=]() {
      auto idx = listBox->getSelected();
      tp->applyTheme(idx);
      tp->setDefaultTheme(idx);
      nameText->setTextFlags(COLOR_THEME_PRIMARY1);
      authorText->setTextFlags(COLOR_THEME_PRIMARY1);
      listBox->setActiveItem(idx);
      TabsGroup::refreshTheme();
    });
  }

  // you can't edit the default theme
  if (listBox->getSelected() != 0) {
    menu->addLine(STR_EDIT, [=]() {
      auto themeIdx = listBox->getSelected();
      if (themeIdx < 0) return;

      auto theme = tp->getThemeByIndex(themeIdx);
      if (theme == nullptr) return;

      new ThemeEditPage(theme, [=](ThemeFile &editedTheme) {
        // update cached theme data
        *theme = editedTheme;

        theme->serialize();

        // if the theme info currently displayed
        // were changed, update the UI
        if (themeIdx == currentTheme) {
          setAuthor(theme);
          setName(theme);
          listBox->setName(currentTheme, theme->getName());
          themeColorPreview->setColorList(theme->getColorList());
        }

        // if the active theme changed, re-apply it
        if (themeIdx == tp->getThemeIndex()) {
          // Update saved them name in case it was changed.
          tp->setDefaultTheme(themeIdx);
          theme->applyTheme();
          TabsGroup::refreshTheme();
        }
      });
    });
  }

  menu->addLine(STR_DUPLICATE, [=] () {
    ThemeFile newTheme;

    new ThemeDetailsDialog(window, newTheme, [=](ThemeFile theme) {
      if (strlen(theme.getName()) != 0) {
        char name[SELECTED_THEME_NAME_LEN + 20];
        strncpy(name, theme.getName(), SELECTED_THEME_NAME_LEN + 19);
        removeAllWhiteSpace(name);

        // use the selected themes color list to make the new theme
        auto themeIdx = listBox->getSelected();
        if (themeIdx < 0) return;

        auto selTheme = tp->getThemeByIndex(themeIdx);
        if (selTheme == nullptr) return;

        for (auto color : selTheme->getColorList())
          theme.setColor(color.colorNumber, color.colorValue);

        tp->createNewTheme(name, theme);
        tp->refresh();
        listBox->setNames(tp->getNames());
        listBox->setSelected(currentTheme);
      }
    });
  });

  // you can't delete the default theme or the currently active theme
  if (listBox->getSelected() != 0 && listBox->getSelected() != tp->getThemeIndex()) {
    menu->addLine(STR_DELETE, [=] () {
      new ConfirmDialog(
          window, STR_DELETE_THEME,
          tp->getThemeByIndex(listBox->getSelected())->getName(), [=] {
            tp->deleteThemeByIndex(listBox->getSelected());
            listBox->setNames(tp->getNames());
            currentTheme = min<int>(currentTheme, tp->getNames().size() - 1);
            listBox->setSelected(currentTheme);
          });
    });
  }
}

void ThemeSetupPage::setSelected(ThemePersistance *tp)
{
  auto value = listBox->getSelected();
  if (themeColorPreview && authorText && nameText && fileCarosell) {
    ThemeFile *theme = tp->getThemeByIndex(value);
    if (theme) {
      themeColorPreview->setColorList(theme->getColorList());
      setAuthor(theme);
      setName(theme);
      fileCarosell->setFileNames(theme->getThemeImageFileNames());
    }
  currentTheme = value;
  }
}

void ThemeSetupPage::setupListbox(Window *window, rect_t r, ThemePersistance *tp)
{
  listBox = new ListBox(window, r, tp->getNames());
  lv_obj_set_scrollbar_mode(listBox->getLvObj(), LV_SCROLLBAR_MODE_AUTO);
  listBox->setAutoEdit(true);
  listBox->setSelected(currentTheme);
  listBox->setActiveItem(tp->getThemeIndex());
  listBox->setLongPressHandler([=] () {
      setSelected(tp);
      displayThemeMenu(window, tp);
    });
  listBox->setPressHandler([=] () {
      setSelected(tp);
    });
}

void ThemeSetupPage::build(FormWindow *window)
{
  window->padAll(4);
  pageWindow = window;

#if LCD_W > LCD_H
  window->setFlexLayout(LV_FLEX_FLOW_ROW, 2);
#else
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, 2);
#endif

  auto tp = ThemePersistance::instance();
  auto theme = tp->getCurrentTheme();
  currentTheme = tp->getThemeIndex();

  themeColorPreview = nullptr;
  listBox = nullptr;
  fileCarosell = nullptr;
  nameText = nullptr;
  authorText = nullptr;

  // create listbox and setup menus
#if LCD_W > LCD_H
  rect_t r = { 0, 0, LIST_WIDTH, window->height() - 8};
#else
  rect_t r = { 0, 0, window->width() - 8, LIST_HEIGHT };
#endif
  setupListbox(window, r, tp);

#if LCD_W > LCD_H
  r.w = COLOR_PREVIEW_SIZE;
#else
  r.h = COLOR_PREVIEW_SIZE;
#endif

  // setup ThemeColorPreview()
  auto colorList = theme != nullptr ? theme->getColorList() : std::vector<ColorEntry>();
  themeColorPreview = new ThemeColorPreview(window, r, colorList);
  themeColorPreview->setWidth(r.w);

#if LCD_W > LCD_H
  r.w = window->width() - LIST_WIDTH - COLOR_PREVIEW_SIZE - 12;
  r.h = window->height() - 8;
#else
  r.w = window->width() - 8;
  r.h = window->height() - LIST_HEIGHT - COLOR_PREVIEW_SIZE - 12;
#endif

  auto rw = new FormWindow(window, r);
  rw->padAll(0);
  rw->setFlexLayout(LV_FLEX_FLOW_COLUMN, 2);
  rw->setWidth(r.w);

  r.h -= 46;

  // setup FileCarosell()
  auto fileNames = theme != nullptr ? theme->getThemeImageFileNames() : std::vector<std::string>();
  fileCarosell = new FileCarosell(rw, r, fileNames);

  r.h = 20;

  // author and name of theme on right side of screen
  nameText = new StaticText(rw, r, "", 0, COLOR_THEME_PRIMARY1);
  lv_label_set_long_mode(nameText->getLvObj(), LV_LABEL_LONG_DOT);
  authorText = new StaticText(rw, r, "", 0, COLOR_THEME_PRIMARY1);
  lv_label_set_long_mode(authorText->getLvObj(), LV_LABEL_LONG_DOT);
}
