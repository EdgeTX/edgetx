/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include "dialog.h"

#include "edgetx.h"
#include "mainwindow.h"
#include "progress.h"
#include "etx_lv_theme.h"

//-----------------------------------------------------------------------------

class BaseDialogForm : public Window
{
 public:
  BaseDialogForm(Window* parent, lv_coord_t width, bool flexLayout) : Window(parent, rect_t{})
  {
    etx_scrollbar(lvobj);
    padAll(PAD_TINY);
    if (flexLayout)
      setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO, width, LV_SIZE_CONTENT);
  }

 protected:
  void onClicked() override { Keyboard::hide(false); }
};

BaseDialog::BaseDialog(const char* title,
                       bool closeIfClickedOutside, lv_coord_t width,
                       lv_coord_t maxHeight, bool flexLayout) :
    ModalWindow(closeIfClickedOutside)
{
  auto content = new Window(this, rect_t{});
  content->setWindowFlag(OPAQUE);
  content->padAll(PAD_ZERO);
  content->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO, width, LV_SIZE_CONTENT);
  etx_solid_bg(content->getLvObj());
  lv_obj_center(content->getLvObj());

  header = new StaticText(content, {0, 0, LV_PCT(100), 0}, title ? title : "", COLOR_THEME_PRIMARY2_INDEX);
  etx_solid_bg(header->getLvObj(), COLOR_THEME_SECONDARY1_INDEX);
  header->padAll(PAD_SMALL);
  header->show(title != nullptr);

  form = new BaseDialogForm(content, width, flexLayout);
  if (maxHeight != LV_SIZE_CONTENT)
    lv_obj_set_style_max_height(form->getLvObj(), maxHeight - EdgeTxStyles::UI_ELEMENT_HEIGHT, LV_PART_MAIN);
}

void BaseDialog::setTitle(const char* title)
{
  header->setText(title);
}

//-----------------------------------------------------------------------------

ProgressDialog::ProgressDialog(const char* title,
                               std::function<void()> onClose) :
    BaseDialog(title, false), onClose(std::move(onClose))
{
  progress = new Progress(form, rect_t{0, 0, LV_PCT(100), 32});
  updateProgress(0);
}

void ProgressDialog::setTitle(std::string title)
{
  header->setText(title);
  header->show();
}

void ProgressDialog::updateProgress(int percentage)
{
  progress->setValue(percentage);
  lv_refr_now(nullptr);
}

void ProgressDialog::closeDialog()
{
  deleteLater();
  onClose();
}

//-----------------------------------------------------------------------------

MessageDialog::MessageDialog(const char* title,
                             const char* message, const char* info,
                             LcdFlags messageFlags, LcdFlags infoFlags) :
    BaseDialog(title, true)
{
  messageWidget = new StaticText(form, {0, 0, LV_PCT(100), LV_SIZE_CONTENT},
                                 message, COLOR_THEME_PRIMARY1_INDEX, messageFlags);

  if (info) {
    infoWidget = new StaticText(form, {0, 0, LV_PCT(100), LV_SIZE_CONTENT},
                                info, COLOR_THEME_PRIMARY1_INDEX, infoFlags);
  }
}

void MessageDialog::onClicked() { deleteLater(); }

//-----------------------------------------------------------------------------

DynamicMessageDialog::DynamicMessageDialog(
    const char* title, std::function<std::string()> textHandler,
    const char* message, const int lineHeight, LcdColorIndex color, LcdFlags textFlags) :
    BaseDialog(title, true)
{
  messageWidget = new StaticText(form, {0, 0, LV_PCT(100), LV_SIZE_CONTENT},
                                 message, COLOR_THEME_PRIMARY1_INDEX, CENTERED);

  infoWidget = new DynamicText(form, {0, 0, LV_PCT(100), LV_SIZE_CONTENT},
                               textHandler, color, textFlags);
}

void DynamicMessageDialog::onClicked() { deleteLater(); }

//-----------------------------------------------------------------------------

ConfirmDialog::ConfirmDialog(const char* title,
                             const char* message,
                             std::function<void(void)> confirmHandler,
                             std::function<void(void)> cancelHandler) :
    BaseDialog(title, false),
    confirmHandler(std::move(confirmHandler)),
    cancelHandler(std::move(cancelHandler))
{
  if (message) {
    new StaticText(form, {0, 0, LV_PCT(100), 0}, message, COLOR_THEME_PRIMARY1_INDEX, CENTERED);
  }

  auto box = new Window(form, rect_t{});
  box->padAll(PAD_TINY);
  box->setFlexLayout(LV_FLEX_FLOW_ROW, 40, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_align(box->getLvObj(), LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_BETWEEN);

  new TextButton(box, rect_t{0, 0, 96, 0}, STR_NO, [=]() -> int8_t {
    onCancel();
    return 0;
  });

  new TextButton(box, rect_t{0, 0, 96, 0}, STR_YES, [=]() -> int8_t {
    this->deleteLater();
    this->confirmHandler();
    return 0;
  });
}

void ConfirmDialog::onCancel()
{
  deleteLater();
  if (cancelHandler) cancelHandler();
}

//-----------------------------------------------------------------------------

LabelDialog::LabelDialog(const char *label, int length, const char* title,
            std::function<void(std::string)> _saveHandler) :
    ModalWindow(false), saveHandler(std::move(_saveHandler))
{
  assert(length <= MAX_LABEL_LENGTH);

  strncpy(this->label, label, length);
  this->label[length] = '\0';

  auto form = new Window(this, rect_t{});
  form->padAll(PAD_ZERO);
  form->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO, LCD_W * 0.8,
                      LV_SIZE_CONTENT);
  etx_solid_bg(form->getLvObj());
  lv_obj_center(form->getLvObj());

  auto hdr = new StaticText(form, {0, 0, LV_PCT(100), 0}, title,
                            COLOR_THEME_PRIMARY2_INDEX);
  etx_solid_bg(hdr->getLvObj(), COLOR_THEME_SECONDARY1_INDEX);
  hdr->padAll(PAD_MEDIUM);

  auto box = new Window(form, rect_t{});
  box->padAll(PAD_MEDIUM);
  box->setFlexLayout(LV_FLEX_FLOW_ROW, 40, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_align(box->getLvObj(), LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_BETWEEN);

  new TextEdit(box, rect_t{0, 0, LV_PCT(100), 0}, this->label, length);

  box = new Window(form, rect_t{});
  box->padAll(PAD_MEDIUM);
  box->setFlexLayout(LV_FLEX_FLOW_ROW, 40, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_align(box->getLvObj(), LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_BETWEEN);

  new TextButton(box, rect_t{0, 0, 96, 0}, STR_CANCEL, [=]() {
    deleteLater();
    return 0;
  });

  new TextButton(box, rect_t{0, 0, 96, 0}, STR_SAVE, [=]() {
    if (saveHandler != nullptr) saveHandler(this->label);
    deleteLater();
    return 0;
  });
}
