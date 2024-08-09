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

#include "colorcustomscreens.h"
#include "customisation_data.h"
#include "appdata.h"

#include <yaml-cpp/yaml.h>

#include <QImage>
#include <QFile>
#include <QString>
#include <QStringList>

#include <string>
#include <sstream>


//  used to screen out raw data that cannot be displayed in a user meaningful format at this time TODO remove
#define SHOW_RAW_INFO  (1==0)

UserInterfacePanel::UserInterfacePanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware):
  ModelPanel(parent, model, generalSettings, firmware)
{
  Board::Type board = firmware->getBoard();

  QString sdPath = QString(g.profile[g.id()].sdPath()).trimmed();

  grid = new QGridLayout(this);

  int col = 0;
  int row = 0;

  std::string themeName = tr("Information unavailable").toStdString();
  std::string themeAuthor = "";
  std::string themeInfo = "";
  std::string themeFolder = "";

  QString selTheme = QString();

  QFile f(sdPath % "/THEMES/selectedtheme.txt");

  if (f.exists()) {
    if (f.open(QFile::ReadOnly | QFile::Text)) {
      QTextStream in(&f);
      if (in.status() == QTextStream::Ok) {
        selTheme = in.readLine();
        if (!(in.status() == QTextStream::Ok)) {
          selTheme = QString();
        }
      }
      f.close();
    }
  }

  if (!selTheme.isEmpty()) {
    QStringList strl = selTheme.split("/");
    if (strl.size() >= 3) {
      themeFolder = "/THEMES/" + strl.at(2).toStdString();
      themeName = strl.at(2).toStdString();
    }

    QString selThemeDetails(sdPath % selTheme);

    if (QFile(selThemeDetails).exists()) {

      try {
        YAML::Node node = YAML::LoadFile(selThemeDetails.toStdString());

        if (node["summary"]) {
          const auto &summary = node["summary"];
          if (summary.IsMap()) {
            if (summary["name"].IsScalar())
              themeName = summary["name"].as<std::string>();
            if (summary["author"].IsScalar())
              themeAuthor = summary["author"].as<std::string>();
            if (summary["info"].IsScalar())
              themeInfo = summary["info"].as<std::string>();
          }
        }
      } catch(const std::runtime_error& e) {
          QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Cannot load %1").arg(selThemeDetails) + ":\n" + QString(e.what()));
      }

    }
  }

  addGridBlankRow(grid, row);

  col = 0;
  addGridLabel(grid, tr("Theme"), row, col++);
  addGridLabel(grid, themeName.c_str(), row++, col++);

  col = 0;
  addGridLabel(grid, tr("Author"), row, col++);
  addGridLabel(grid, themeAuthor.c_str(), row++, col++);

  col = 0;
  addGridLabel(grid, tr("Information"), row, col++);
  addGridLabel(grid, themeInfo.c_str(), row++, col++);

  col = 0;
  addGridBlankRow(grid, row);
  addGridLabel(grid, "", row, col++);

  QLabel * img = new QLabel(this);
  QString path = sdPath + themeFolder.c_str() + "/logo.png";
  QFile fimg(path);
  if (fimg.exists()) {
    QImage image(path);
    if (!image.isNull()) {
      img->setText("");
      img->setFixedSize(QSize(Boards::getCapability(board, Board::LcdWidth) / 2,
                              Boards::getCapability(board, Board::LcdHeight) / 2));
      img->setPixmap(QPixmap::fromImage(image.scaled(Boards::getCapability(board, Board::LcdWidth) / 2,
                                                     Boards::getCapability(board, Board::LcdHeight) / 2)));
    }

    grid->addWidget(img, row++, col++);
  }

  col = 0;
  addGridBlankRow(grid, row);
  addGridLabel(grid, tr("Top Bar"), row, col++);

  int widgetdetailsrow = row;
  int widgetdetailscol = col + 1;
  int firstenabledbtn = -1;

  QGridLayout * tbgrid = new QGridLayout();

  for (int i = 0; i < firmware->getCapability(TopBarZones); i++) {
    ZonePersistentData & zpd = model.topBarData.zones[i];
    QPushButton * btn = new QPushButton(zpd.widgetName, this);
    btn->setProperty("index", i);
    btn->setFixedSize(QSize(90, 30));
    widgetbtns.append(btn);

    if (zpd.widgetName[0] != '\0' && SHOW_RAW_INFO) {
      if (firstenabledbtn < 0)
        firstenabledbtn = i;

      QFrame * wgt = new QFrame();
      wgt->setFrameStyle(QFrame::StyledPanel);
      wgt->setFrameShadow(QFrame::Plain);
      wgt->setVisible(false);
      optswidgets.append(wgt);

      QGridLayout * layout = new QGridLayout(wgt);  // must be owned by QWidget so visibility can be set
      layout->addLayout(addOptionsLayout<WidgetPersistentData>(zpd.widgetData, MAX_WIDGET_OPTIONS, zpd.widgetName), 0, 0);
      optsgrids.append(layout);

      connect(btn, &QPushButton::clicked, this, [&]() {
        bool ok;
        int idx = sender()->property("index").toInt(&ok);
        if (ok) {
          if (currindex >= 0) {
            optswidgets.at(currindex)->setVisible(false);
          }
          optswidgets.at(idx)->setVisible(true);
          currindex = idx;
        }
      });
    }
    else if (SHOW_RAW_INFO) {
      btn->setEnabled(false);
    }

    div_t posn = div(i, MAX_TOPBAR_ZONES);
    tbgrid->addWidget(btn, posn.quot, posn.rem);
  }

  grid->addLayout(tbgrid, row++, col++, Qt::AlignTop);

  addGridBlankRow(grid, row);

  col  = 0;

  //  the grid must be fully built for the rowspan to work as required
  foreach (QWidget * wgt, optswidgets) {
    grid->addWidget(wgt, widgetdetailsrow, widgetdetailscol, 3, Qt::AlignTop);
  }

  addHSpring(grid, grid->columnCount(), 0);
  addVSpring(grid, 0, grid->rowCount());
  disableMouseScrolling();

  if (firstenabledbtn >= 0)
    widgetbtns.at(firstenabledbtn)->click();
}

UserInterfacePanel::~UserInterfacePanel()
{
  foreach (QGridLayout * grid, optsgrids) {
    delete grid;
  }
  foreach (QFrame * wgt, optswidgets) {
    delete wgt;
  }
  foreach (QPushButton * btn, widgetbtns) {
    delete btn;
  }
}

/*
    CustomScreenPanel
*/

CustomScreenPanel::CustomScreenPanel(QWidget * parent, ModelData & model, int index, GeneralSettings & generalSettings, Firmware * firmware):
  ModelPanel(parent, model, generalSettings, firmware)
{
  Board::Type board = firmware->getBoard();
  RadioLayout::CustomScreens & scrns = model.customScreens;

  grid = new QGridLayout(this);

  int col = 0;
  int row = 0;

  addGridBlankRow(grid, row);
  addGridLabel(grid, tr("Layout:"), row, col++);

  //  currently no point continuing if no layout but will change if editing becomes possible
  if (model.customScreens.customScreenData[index].layoutId[0] == '\0') {
    addGridLabel(grid, tr("None"), row, col);
    addHSpring(grid, grid->columnCount(), 0);
    addVSpring(grid, 0, grid->rowCount());
    return;
  }

  QLabel * img = new QLabel(this);

  QString path(QString(":/layouts/mask_%1.png").arg(QString(scrns.customScreenData[index].layoutId).toLower()));
  QFile f(path);

  if (f.exists()) {
    QImage image(path);
    if (!image.isNull()) {
      img->setFixedSize(QSize(Boards::getCapability(board, Board::LcdWidth) / 5,
                              Boards::getCapability(board, Board::LcdHeight) / 5));
      img->setPixmap(QPixmap::fromImage(image.scaled(Boards::getCapability(board, Board::LcdWidth) / 5,
                                                     Boards::getCapability(board, Board::LcdHeight) / 5)));
    }
  }

  grid->addWidget(img, row++, col++);

  LayoutPersistentData & lpd = scrns.customScreenData[index].layoutPersistentData;

  addGridBlankRow(grid, row);

  col = 0;
  addGridLabel(grid, tr("Widgets:"), row, col++);

  int widgetdetailsrow = row;
  int widgetdetailscol = col + 1;
  int firstenabledbtn = -1;

  QGridLayout * tbgrid = new QGridLayout();

  for (int i = 0; i < MAX_LAYOUT_ZONES; i++) {
    ZonePersistentData & zpd = lpd.zones[i];
    QPushButton * btn = new QPushButton(zpd.widgetName, this);
    btn->setProperty("index", i);
    btn->setFixedSize(QSize(90, 30));
    widgetbtns.append(btn);

    if (zpd.widgetName[0] != '\0' && SHOW_RAW_INFO) {
      if (firstenabledbtn < 0)
        firstenabledbtn = i;

      QFrame * wgt = new QFrame();
      wgt->setFrameStyle(QFrame::StyledPanel);
      wgt->setFrameShadow(QFrame::Plain);
      wgt->setVisible(false);
      optswidgets.append(wgt);

      QGridLayout * layout = new QGridLayout(wgt);  // must be owned by QWidget so isibility can be set
      layout->addLayout(addOptionsLayout<WidgetPersistentData>(zpd.widgetData, MAX_WIDGET_OPTIONS, zpd.widgetName), 0, 0, Qt::AlignTop);
      optsgrids.append(layout);

      connect(btn, &QPushButton::clicked, this, [&]() {
        bool ok;
        int idx = sender()->property("index").toInt(&ok);
        if (ok) {
          if (currindex >= 0) {
            optswidgets.at(currindex)->setVisible(false);
          }
          optswidgets.at(idx)->setVisible(true);
          currindex = idx;
        }
      });
    }
    else if (SHOW_RAW_INFO) {
      btn->setEnabled(false);
    }

    div_t posn = div(i, MAX_LAYOUT_ZONES / 2);
    tbgrid->addWidget(btn, posn.quot, posn.rem);
  }

  grid->addLayout(tbgrid, row++, col++, Qt::AlignTop);

  addGridBlankRow(grid, row);

  //  Note: options are defined by each layout but we do not have access to the definitions yet!
  //        We assumed the first five to be the common options and the description of any extras is unknown

  int cnt = SHOW_RAW_INFO ? MAX_LAYOUT_OPTIONS : LAYOUT_OPTION_LAST_DEFAULT + 1;

  for (int i = 0; i < cnt; i++) {
    QString str;
    switch(i) {
      case LAYOUT_OPTION_TOPBAR:
        str = tr("Top bar");
        break;
      case LAYOUT_OPTION_FM:
        str = tr("%1 mode").arg(Boards::getRadioTypeString(firmware->getBoard()));
        break;
      case LAYOUT_OPTION_SLIDERS:
        str = tr("Sliders");
        break;
      case LAYOUT_OPTION_TRIMS:
        str = tr("Trims");
        break;
      case LAYOUT_OPTION_MIRRORED:
        str = tr("Mirror");
        break;
      default:
        str = tr("Option #%1").arg(i + 1);
    }

    col = 0;
    addGridLabel(grid, str, row, col++);

    QCheckBox *chk = nullptr;
    QLabel *lbl = nullptr;

    switch (lpd.options[i].type) {
      case ZOV_Bool:
        chk = new QCheckBox(this);
        chk->setChecked(lpd.options[i].value.boolValue);
        break;
      case ZOV_Unsigned:
        lbl = new QLabel(this);
        lbl->setText(QString("%1").arg(lpd.options[i].value.unsignedValue));
        break;
      case ZOV_Signed:
        lbl = new QLabel(this);
        lbl->setText(QString("%1").arg(lpd.options[i].value.signedValue));
        break;
      case ZOV_String:
        lbl = new QLabel(this);
        lbl->setText(lpd.options[i].value.stringValue);
        break;
      default:
        lbl = new QLabel(this);
        lbl->setText(QString("%1").arg(lpd.options[i].value.unsignedValue));
    }

    if (chk) {
      chk->setEnabled(false);
      grid->addWidget(chk, row++, col++);
    }
    else {
      grid->addWidget(lbl, row++, col++);
    }
  }

  //  the grid must be fully built so the rowspan works
  foreach (QWidget * wgt, optswidgets) {
    grid->addWidget(wgt, widgetdetailsrow, widgetdetailscol, MAX_WIDGET_OPTIONS + 2/*title and column headings*/, Qt::AlignTop);
  }

  addHSpring(grid, grid->columnCount(), 0);
  addVSpring(grid, 0, grid->rowCount());
  disableMouseScrolling();

  if (firstenabledbtn >= 0)
    widgetbtns.at(firstenabledbtn)->click();
}

CustomScreenPanel::~CustomScreenPanel()
{
  foreach (QGridLayout * grid, optsgrids) {
    delete grid;
  }
  foreach (QFrame * wgt, optswidgets) {
    delete wgt;
  }
  foreach (QPushButton * btn, widgetbtns) {
    delete btn;
  }
}

/*
    ColorCustomScreensPanel
*/

ColorCustomScreensPanel::ColorCustomScreensPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                                       CompoundItemModelFactory * sharedItemModels):
  ModelPanel(parent, model, generalSettings, firmware)
{
  grid = new QGridLayout(this);
  tabWidget = new QTabWidget(this);

  UserInterfacePanel * tab = new UserInterfacePanel(tabWidget, model, generalSettings, firmware);
  tab->setProperty("index", 0);
  tabWidget->addTab(tab, getTabName(0));
  panels << tab;

  for (int i = 0; i < MAX_CUSTOM_SCREENS; i++) {
    CustomScreenPanel * tab = new CustomScreenPanel(tabWidget, model, i, generalSettings, firmware);
    tab->setProperty("index", i + 1);
    tabWidget->addTab(tab, getTabName(i + 1));
    panels << tab;
  }

  grid->addWidget(tabWidget, 0, 0, 1, 1);
}

ColorCustomScreensPanel::~ColorCustomScreensPanel()
{
  foreach (GenericPanel * pnl, panels) {
    delete pnl;
  }
}

QString ColorCustomScreensPanel::getTabName(int index)
{
  if (index == 0)
    return tr("User Interface");
  else
    return tr("Main View %1").arg(index);
}

void addGridLine(QGridLayout * grid, int row, int col, int colspan)
{
  QFrame *line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  grid->addWidget(line, row, col, 1, colspan);
}

void addGridLabel(QGridLayout * grid, QString text, int row, int col, int colspan)
{
  QLabel * label = new QLabel();
  label->setText(text);
  grid->addWidget(label, row, col, 1, colspan, Qt::AlignTop);
}

void addGridBlankRow(QGridLayout * grid, int & row)
{
  addGridLabel(grid, QString(), row++, 0);
}

template <class T>
QGridLayout * addOptionsLayout(T & persistentData, int optioncnt, QString title)
{
  QGridLayout * grid = new QGridLayout();

  int row = 0;
  int col = 0;

  if (!title.isEmpty()) {
    QLabel * label = new QLabel();
    label->setFrameShape(QFrame::Panel);
    label->setFrameShadow(QFrame::Plain);
    label->setAlignment(Qt::AlignCenter);
    label->setMinimumHeight(30);
    label->setMaximumHeight(30);
    label->setText(title);
    grid->addWidget(label, row++, col, 1, 2, Qt::AlignTop);
  }

  addGridLabel(grid, "Option", row, col++);
  //addGridLabel(grid, "Type", row, col++);
  addGridLabel(grid, "Value", row++, col++);

  for (int i = 0; i < optioncnt; i++) {
    ZoneOptionValueTyped & zovt = persistentData.options[i];

    int col = 0;
    addGridLabel(grid, QString("#%1").arg(i + 1), row, col++);
    //addGridLabel(grid, QString("%1:%2").arg(zovt.type).arg(zoneOptionValueEnumToString(zovt.type)), row, col++);
    QString val;

    switch (zovt.type) {
      case ZOV_Bool:
        val = QString("%1").arg(zovt.value.boolValue);
        break;
      case ZOV_Unsigned:
        val = QString("%1").arg(zovt.value.unsignedValue);
        break;
      case ZOV_Signed:
        val = QString("%1").arg(zovt.value.signedValue);
        break;
      case ZOV_String:
        val = QString("%1").arg(zovt.value.stringValue);
        break;
      default:
        val = QString("%1").arg(zovt.value.unsignedValue);
    }

    addGridLabel(grid, QString("%1").arg(val), row++, col++);
  }

  return grid;
}
