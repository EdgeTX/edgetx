/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "mdichild.h"
#include "ui_mdichild.h"
#include "modeledit/modeledit.h"
#include "generaledit/generaledit.h"
#include "burnconfigdialog.h"
#include "printdialog.h"
#include "flasheepromdialog.h"
#include "helpers.h"
#include "appdata.h"
#include "wizarddialog.h"
#include "flashfirmwaredialog.h"
#include "storage.h"
#include "radiointerface.h"
#include "radiodataconversionstate.h"

#include <algorithm>
#include <ExportableTableView>

MdiChild::MdiChild(QWidget * parent, QWidget * parentWin, Qt::WindowFlags f):
  QWidget(parent, f),
  ui(new Ui::MdiChild),
  modelsListModel(NULL),
  parentWindow(parentWin),
  radioToolbar(NULL),
  categoriesToolbar(NULL),
  modelsToolbar(NULL),
  firmware(getCurrentFirmware()),
  lastSelectedModel(-1),
  isUntitled(true),
  showCatToolbar(true),
  forceCloseFlag(false),
  stateDataVersion(1)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("open.png"));
  setAttribute(Qt::WA_DeleteOnClose);
  setContextMenuPolicy(Qt::CustomContextMenu);
  if (parentWindow)
    parentWindow->setWindowIcon(windowIcon());

  setupNavigation();
  initModelsList();

  ui->modelsList->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->modelsList->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->modelsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->modelsList->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  ui->modelsList->setDragEnabled(true);
  ui->modelsList->setAcceptDrops(true);
  ui->modelsList->setDragDropOverwriteMode(false);
  ui->modelsList->setDropIndicatorShown(true);
  ui->modelsList->setDragDropMode(QAbstractItemView::DragDrop);
  ui->modelsList->setStyle(new ItemViewProxyStyle(ui->modelsList->style()));
  ui->modelsList->setStyleSheet("QTreeView::item {margin: 2px 0;}");  // a little more space for our drop indicators

  retranslateUi();

  connect(this, &MdiChild::customContextMenuRequested, this, &MdiChild::showContextMenu);
  connect(ui->modelsList, &QTreeView::activated, this, &MdiChild::onItemActivated);
  connect(ui->modelsList, &QTreeView::customContextMenuRequested, this, &MdiChild::showModelsListContextMenu);
  connect(ui->modelsList, &QTreeView::pressed, this, &MdiChild::onItemSelected);
  connect(ui->modelsList->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MdiChild::onCurrentItemChanged);
  connect(QGuiApplication::clipboard(), &QClipboard::dataChanged, this, &MdiChild::updateNavigation);

  if (!(isMaximized() || isMinimized())) {
    QByteArray geo = g.mdiWinGeo();
    if (geo.isEmpty())
      adjustSize();
    else if (geo.size() < 10 && geo == "maximized") {
      if (!parentWindow)  // otherwise we let the MdiArea manage the window maximizing
        setWindowState(windowState() ^ Qt::WindowMaximized);
    }
    else if (parentWindow)
      parentWindow->restoreGeometry(geo);
    else
      restoreGeometry(geo);
  }
  if (!g.mdiWinState().isEmpty()) {
    QByteArray state = g.mdiWinState();
    QDataStream stream(&state, QIODevice::ReadOnly);
    quint16 ver;
    stream >> ver;
    if (ver <= stateDataVersion) {
      bool visMdl, visGen;
      stream >> showCatToolbar >> visMdl >> visGen;
      categoriesToolbar->setVisible(showCatToolbar);
      modelsToolbar->setVisible(visMdl);
      radioToolbar->setVisible(visGen);
    }
  }
}

MdiChild::~MdiChild()
{
  delete ui;
}

void MdiChild::closeEvent(QCloseEvent *event)
{
  if (!maybeSave() && !forceCloseFlag) {
    event->ignore();
    return;
  }
  event->accept();

  if (!isMinimized()) {
    QByteArray geo;
    if (isMaximized())
      geo.append("maximized");
    else if (parentWindow)
      geo = parentWindow->saveGeometry();
    else
      geo = saveGeometry();
    g.mdiWinGeo(geo);
  }

  if (!isVisible())
    return;

  QByteArray state;
  QDataStream stream(&state, QIODevice::WriteOnly);
  stream << stateDataVersion
         << (firmware->getCapability(Capability::HasModelCategories) ? categoriesToolbar->isVisible() : showCatToolbar)
         << modelsToolbar->isVisible()
         << radioToolbar->isVisible();
  g.mdiWinState(state);
}

void MdiChild::resizeEvent(QResizeEvent * event)
{
  QWidget::resizeEvent(event);
  adjustToolbarLayout();
}

QSize MdiChild::sizeHint() const
{
  QWidget * p;
  if (parentWindow)
    p = parentWindow->parentWidget();
  else
    p = parentWidget();
  if (!p)
    return QWidget::sizeHint();
  // use toolbar as a gauge for width, and take all the height availabe.
  int w = qMax(ui->topToolbarLayout->sizeHint().width(), ui->botToolbarLayout->sizeHint().width());
  return QSize(w + 30, qMin(p->height(), 1000));
}

void MdiChild::changeEvent(QEvent * event)
{
  QWidget::changeEvent(event);
  switch (event->type()) {
    case QEvent::LanguageChange:
      retranslateUi();
      break;
    default:
      break;
  }
}

QAction * MdiChild::addAct(Actions actId, const QString & icon, const char * slot, const QKeySequence & shortcut, QObject * slotObj)
{
  QAction * newAction = new QAction(this);
  newAction->setMenuRole(QAction::NoRole);
  if (!icon.isEmpty())
    newAction->setIcon(CompanionIcon(icon));
  if (!shortcut.isEmpty())
    newAction->setShortcut(shortcut);
  if (slotObj == NULL)
    slotObj = this;
  if (slot)
    connect(newAction, SIGNAL(triggered()), slotObj, slot);
  action.replace(actId, newAction);
  return newAction;
}

void MdiChild::setupNavigation()
{
  foreach (QAction * act, action) {
    if (act)
      act->deleteLater();
  }
  action.clear();
  action.fill(NULL, ACT_ENUM_END);

  addAct(ACT_GEN_EDT, "edit.png",     SLOT(generalEdit()),          tr("Alt+Shift+E"));
  addAct(ACT_GEN_CPY, "copy.png",     SLOT(copyGeneralSettings()),  tr("Ctrl+Alt+C"));
  addAct(ACT_GEN_PST, "paste.png",    SLOT(pasteGeneralSettings()), tr("Ctrl+Alt+V"));
  addAct(ACT_GEN_SIM, "simulate.png", SLOT(radioSimulate()),        tr("Alt+Shift+S"));

  addAct(ACT_ITM_EDT, "edit.png",  SLOT(edit()),          Qt::Key_Enter);
  addAct(ACT_ITM_DEL, "clear.png", SLOT(confirmDelete()), QKeySequence::Delete);

  addAct(ACT_CAT_ADD, "add.png",   SLOT(categoryAdd()),   tr("Alt+C"));
  //addAct(ACT_CAT_EDT, "edit.png",  SLOT(edit()),          Qt::Key_Enter);
  //addAct(ACT_CAT_DEL, "clear.png", SLOT(confirmDelete()), QKeySequence::Delete);
  action[ACT_CAT_SEP] = new QAction(this);
  action[ACT_CAT_SEP]->setSeparator(true);

  addAct(ACT_MDL_ADD, "add.png",    SLOT(modelAdd()),   tr("Alt+A"));
  addAct(ACT_MDL_RTR, "open.png",   SLOT(loadBackup()), tr("Alt+R"));
  addAct(ACT_MDL_WIZ, "wizard.png", SLOT(wizardEdit()), tr("Alt+W"));

  addAct(ACT_MDL_DFT, "currentmodel.png", SLOT(setDefault()),     tr("Alt+U"));
  addAct(ACT_MDL_PRT, "print.png",        SLOT(print()),          QKeySequence::Print);
  addAct(ACT_MDL_SIM, "simulate.png",     SLOT(modelSimulate()),  tr("Alt+S"));
  addAct(ACT_MDL_DUP, "duplicate.png",    SLOT(modelDuplicate()), QKeySequence::Underline);

  addAct(ACT_MDL_CUT, "cut.png",   SLOT(cut()),           QKeySequence::Cut);
  addAct(ACT_MDL_CPY, "copy.png",  SLOT(copy()),          QKeySequence::Copy);
  addAct(ACT_MDL_PST, "paste.png", SLOT(paste()),         QKeySequence::Paste);
  addAct(ACT_MDL_INS, "list.png",  SLOT(insert()),        QKeySequence::Italic);

  addAct(ACT_MDL_MOV, "arrow-right.png");
  QMenu * catsMenu = new QMenu(this);
  action[ACT_MDL_MOV]->setMenu(catsMenu);

  // set up the toolbars

  QToolButton * btn;
  QSize tbIcnSz(16, 16);
  QString tbCss = "QToolBar {border: 1px solid palette(midlight);}";

  if (categoriesToolbar)
    categoriesToolbar->deleteLater();
  categoriesToolbar = new QToolBar(this);
  categoriesToolbar->setObjectName("TB_CATEGORIES");
  categoriesToolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  categoriesToolbar->setFloatable(false);
  categoriesToolbar->setIconSize(tbIcnSz);
  categoriesToolbar->setStyleSheet(tbCss);
  categoriesToolbar->addAction(getAction(ACT_CAT_ADD));
  ui->topToolbarLayout->addWidget(categoriesToolbar);

  if (radioToolbar)
    radioToolbar->deleteLater();
  radioToolbar = new QToolBar(this);
  radioToolbar->setObjectName("TB_GENERAL");
  radioToolbar->setFloatable(false);
  radioToolbar->setIconSize(tbIcnSz);
  radioToolbar->setStyleSheet(tbCss);
  radioToolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  radioToolbar->addActions(getGeneralActions());
  if ((btn = qobject_cast<QToolButton *>(radioToolbar->widgetForAction(action[ACT_GEN_EDT])))) {
    btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  }
  if ((btn = qobject_cast<QToolButton *>(radioToolbar->widgetForAction(action[ACT_GEN_SIM])))) {
    btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  }
  // add spacer to right-align the buttons
  QWidget * sp = new QWidget(this);
  sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  radioToolbar->insertWidget(radioToolbar->actions().first(), sp);
  ui->topToolbarLayout->addWidget(radioToolbar);

  if (modelsToolbar)
    modelsToolbar->deleteLater();
  modelsToolbar = new QToolBar(this);
  modelsToolbar->setObjectName("TB_MODELS");
  modelsToolbar->setFloatable(false);
  modelsToolbar->setIconSize(tbIcnSz);
  modelsToolbar->setStyleSheet(tbCss);
  modelsToolbar->addActions(getEditActions(false));
  modelsToolbar->addSeparator();
  modelsToolbar->addActions(getModelActions());
  if ((btn = qobject_cast<QToolButton *>(modelsToolbar->widgetForAction(action[ACT_MDL_ADD])))) {
    btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  }
  if ((btn = qobject_cast<QToolButton *>(modelsToolbar->widgetForAction(action[ACT_MDL_MOV])))) {
    btn->setPopupMode(QToolButton::InstantPopup);
  }
  ui->botToolbarLayout->addWidget(modelsToolbar);

  connect(categoriesToolbar, &QToolBar::visibilityChanged, this, &MdiChild::adjustToolbarLayout);
  connect(radioToolbar, &QToolBar::visibilityChanged, this, &MdiChild::adjustToolbarLayout);
  connect(modelsToolbar, &QToolBar::visibilityChanged, this, &MdiChild::adjustToolbarLayout);
}

void MdiChild::updateNavigation()
{
  const int modelsSelected = countSelectedModels();
  const int catsSelected = countSelectedCats();
  const bool singleModelSelected = (modelsSelected == 1);
  const bool hasModelSlotSelcted = (getCurrentModel() > -1);
  const bool hasCats = firmware->getCapability(Capability::HasModelCategories);
  const bool hasCatSelected = hasCats && modelsListModel->isCategoryType(getCurrentIndex());
  const int numOnClipbrd = modelsListModel->countModelsInMimeData(QApplication::clipboard()->mimeData());
  const QString modelsRemvTxt = tr("%n Model(s)", "As in \"Copy 3 Models\" or \"Cut 1 Model\" or \"Delete 3 Models\" action).", modelsSelected);
  const QString modelsAddTxt = tr("%n Model(s)", "As in \"Paste 3 Models\" or \"Insert 1 Model.\"", numOnClipbrd);
  const QString catsRemvTxt = tr("%n Category(ies)", "As in \"Delete 3 Categories\" or \"Delete 1 Category.\"", catsSelected);
  static const QString noSelection = tr("Nothing selected");
  static const QString sp = " ";
  static const QString ns;

  categoriesToolbar->setVisible(hasCats && showCatToolbar);

  action[ACT_GEN_PST]->setEnabled(hasClipboardData(1));

  action[ACT_ITM_EDT]->setEnabled(singleModelSelected || catsSelected == 1);
  action[ACT_ITM_EDT]->setText((hasCatSelected ? tr("Rename Category") : modelsSelected ? tr("Edit Model") : noSelection));
  action[ACT_ITM_DEL]->setEnabled(modelsSelected || catsSelected);
  action[ACT_ITM_DEL]->setText((action[ACT_ITM_DEL]->isEnabled() ? tr("Delete") % " " % (hasCatSelected ? catsRemvTxt : modelsRemvTxt) : noSelection));

  action[ACT_CAT_ADD]->setVisible(hasCats);
  action[ACT_CAT_SEP]->setVisible(hasCats);
//  action[ACT_CAT_EDT]->setVisible(hasCats);
//  action[ACT_CAT_EDT]->setEnabled(catsSelected == 1);
//  action[ACT_CAT_DEL]->setVisible(hasCats);
//  action[ACT_CAT_DEL]->setEnabled(catsSelected);
//  action[ACT_CAT_DEL]->setText((hasCatSelected ? tr("Delete") % " " % catsRemTxt : noSelection));

  action[ACT_MDL_CUT]->setEnabled(modelsSelected);
  action[ACT_MDL_CUT]->setText(tr("Cut") % (modelsSelected ? sp % modelsRemvTxt : ns));
  action[ACT_MDL_CPY]->setEnabled(modelsSelected);
  action[ACT_MDL_CPY]->setText(tr("Copy") % (modelsSelected ? sp % modelsRemvTxt : ns));
  action[ACT_MDL_PST]->setEnabled(numOnClipbrd);
  action[ACT_MDL_PST]->setText(tr("Paste") % (numOnClipbrd ? sp % modelsAddTxt : ns));
  action[ACT_MDL_INS]->setEnabled(numOnClipbrd && (hasModelSlotSelcted || catsSelected));
  action[ACT_MDL_INS]->setText(tr("Insert") % QString(action[ACT_MDL_INS]->isEnabled() ? sp % modelsAddTxt : ns));

  if (hasCats && action[ACT_MDL_MOV]->menu()) {
    action[ACT_MDL_MOV]->setVisible(true);
    QModelIndex modelIndex = getCurrentIndex();
    QMenu * catsMenu = action[ACT_MDL_MOV]->menu();
    catsMenu->clear();
    if (modelsSelected && modelsListModel && radioData.categories.size() > 1) {
      action[ACT_MDL_MOV]->setEnabled(true);
      for (unsigned i=0; i < radioData.categories.size(); ++i) {
        QAction * act = catsMenu->addAction(QString(radioData.categories[i].name), this, SLOT(onModelMoveToCategory()));
        act->setProperty("categoryId", i);
        if ((int)i < modelsListModel->getCategoryIndex(modelIndex))
          act->setIcon(CompanionIcon("moveup.png"));
        else if ((int)i > modelsListModel->getCategoryIndex(modelIndex))
          act->setIcon(CompanionIcon("movedown.png"));
        else
          act->setEnabled(false);
      }
    }
    else {
      action[ACT_MDL_MOV]->setDisabled(true);
    }
  }
  else {
    action[ACT_MDL_MOV]->setVisible(false);
  }

  action[ACT_MDL_DUP]->setEnabled(singleModelSelected);
  action[ACT_MDL_RTR]->setEnabled(singleModelSelected);
  action[ACT_MDL_WIZ]->setEnabled(singleModelSelected);
  action[ACT_MDL_DFT]->setEnabled(singleModelSelected && getCurrentModel() != (int)radioData.generalSettings.currModelIndex);
  action[ACT_MDL_PRT]->setEnabled(singleModelSelected);
  action[ACT_MDL_SIM]->setEnabled(singleModelSelected);
}

void MdiChild::retranslateUi()
{
  action[ACT_GEN_EDT]->setText(tr("Edit Radio Settings"));
  action[ACT_GEN_CPY]->setText(tr("Copy Radio Settings"));
  action[ACT_GEN_PST]->setText(tr("Paste Radio Settings"));
  action[ACT_GEN_SIM]->setText(tr("Simulate Radio"));

  action[ACT_CAT_ADD]->setText(tr("Add Category"));
  action[ACT_CAT_ADD]->setIconText(tr("Category"));
  //action[ACT_CAT_EDT]->setText(tr("Rename Category"));

  action[ACT_MDL_ADD]->setText(tr("Add Model"));
  action[ACT_MDL_ADD]->setIconText(tr("Model"));
  action[ACT_MDL_RTR]->setText(tr("Restore from Backup"));
  action[ACT_MDL_WIZ]->setText(tr("Model Wizard"));
  action[ACT_MDL_DFT]->setText(tr("Set as Default"));
  action[ACT_MDL_PRT]->setText(tr("Print Model"));
  action[ACT_MDL_SIM]->setText(tr("Simulate Model"));
  action[ACT_MDL_DUP]->setText(tr("Duplicate Model"));

  action[ACT_MDL_MOV]->setText(tr("Move to Category"));

  categoriesToolbar->setWindowTitle(tr("Show Category Actions Toolbar"));
  radioToolbar->setWindowTitle(tr("Show Radio Actions Toolbar"));
  modelsToolbar->setWindowTitle(tr("Show Model Actions Toolbar"));
}

QList<QAction *> MdiChild::getGeneralActions()
{
  QList<QAction *> actGrp;
  actGrp.append(getAction(ACT_GEN_SIM));
  actGrp.append(getAction(ACT_GEN_EDT));
  actGrp.append(getAction(ACT_GEN_CPY));
  actGrp.append(getAction(ACT_GEN_PST));
  return actGrp;
}

QList<QAction *> MdiChild::getEditActions(bool incCatNew)
{
  QList<QAction *> actGrp;
  if (incCatNew) {
    actGrp.append(getAction(ACT_CAT_ADD));
    actGrp.append(getAction(ACT_CAT_SEP));
  }
  actGrp.append(action[ACT_MDL_ADD]);
  QAction * sep2 = new QAction(this);
  sep2->setSeparator(true);
  actGrp.append(sep2);
  actGrp.append(getAction(ACT_ITM_EDT));
  actGrp.append(getAction(ACT_ITM_DEL));
  actGrp.append(getAction(ACT_MDL_CUT));
  actGrp.append(getAction(ACT_MDL_CPY));
  actGrp.append(getAction(ACT_MDL_PST));
  actGrp.append(getAction(ACT_MDL_INS));
  actGrp.append(getAction(ACT_MDL_DUP));
  actGrp.append(getAction(ACT_MDL_MOV));
  return actGrp;
}

QList<QAction *> MdiChild::getModelActions()
{
  QList<QAction *> actGrp;
  actGrp.append(getAction(ACT_MDL_RTR));
  actGrp.append(getAction(ACT_MDL_WIZ));
  actGrp.append(getAction(ACT_MDL_DFT));
  actGrp.append(getAction(ACT_MDL_PRT));
  actGrp.append(getAction(ACT_MDL_SIM));
  return actGrp;
}

//QList<QAction *> MdiChild::getCategoryActions()
//{
//  QList<QAction *> actGrp;
//  actGrp.append(getAction(ACT_CAT_ADD));
//  actGrp.append(getAction(ACT_CAT_EDT));
//  actGrp.append(getAction(ACT_CAT_DEL));
//  actGrp.append(getAction(ACT_CAT_SEP));
//  return actGrp;
//}

QAction * MdiChild::getAction(const MdiChild::Actions type)
{
  if (type < ACT_ENUM_END)
    return action[type];
  else
    return NULL;
}

void MdiChild::showModelsListContextMenu(const QPoint & pos)
{
  QModelIndex modelIndex = ui->modelsList->indexAt(pos);
  QMenu contextMenu;

  updateNavigation();

  if (firmware->getCapability(Capability::HasModelCategories)) {
    contextMenu.addAction(action[ACT_CAT_ADD]);
    if(modelsListModel->isCategoryType(modelIndex)) {
      contextMenu.addAction(action[ACT_ITM_EDT]);
      contextMenu.addAction(action[ACT_ITM_DEL]);
    }
    contextMenu.addSeparator();
  }

  if (modelsListModel->isModelType(modelIndex)) {
    contextMenu.addActions(getEditActions());
    if (countSelectedModels() == 1) {
      contextMenu.addSeparator();
      contextMenu.addActions(getModelActions());
    }
  }
  else {
    contextMenu.addAction(action[ACT_MDL_ADD]);
    if (hasClipboardData())
      contextMenu.addAction(action[ACT_MDL_PST]);
  }

  if (!contextMenu.isEmpty())
    contextMenu.exec(ui->modelsList->mapToGlobal(pos));
}

void MdiChild::showContextMenu(const QPoint & pos)
{
  QMenu contextMenu;
  if (firmware->getCapability(Capability::HasModelCategories))
    contextMenu.addAction(categoriesToolbar->toggleViewAction());
  contextMenu.addAction(modelsToolbar->toggleViewAction());
  contextMenu.addAction(radioToolbar->toggleViewAction());
  if (!contextMenu.isEmpty())
    contextMenu.exec(mapToGlobal(pos));
}

void MdiChild::adjustToolbarLayout()
{
  if (size().width() > ui->topToolbarLayout->sizeHint().width() + ui->botToolbarLayout->sizeHint().width() + 30) {
    ui->botToolbarLayout->removeWidget(modelsToolbar);
    ui->topToolbarLayout->insertWidget(1, modelsToolbar);
  }
  else {
    ui->topToolbarLayout->removeWidget(modelsToolbar);
    ui->botToolbarLayout->insertWidget(0, modelsToolbar);
  }
}

/*
 * Data model
*/

void MdiChild::initModelsList()
{
  Board::Type board = firmware->getBoard();

  if (modelsListModel)
    delete modelsListModel;

  modelsListModel = new TreeModel(&radioData, this);
  connect(modelsListModel, &TreeModel::modelsDropped, this, &MdiChild::pasteModelData);
  connect(modelsListModel, &TreeModel::modelsRemoved, this, &MdiChild::deleteModels);
  connect(modelsListModel, &TreeModel::refreshRequested, this, &MdiChild::refresh);
  connect(modelsListModel, &QAbstractItemModel::dataChanged, this, &MdiChild::onDataChanged);

  ui->modelsList->setModel(modelsListModel);
  if (IS_FAMILY_HORUS_OR_T16(board)) {
    ui->modelsList->setIndentation(20);
    // ui->modelsList->resetIndentation(); // introduced in next Qt versions ...
  }
  else {
    ui->modelsList->setIndentation(0);
  }
  refresh();

  if (firmware->getCapability(Capability::HasModelCategories)) {
    ui->modelsList->header()->resizeSection(0, ui->modelsList->header()->sectionSize(0) * 2);   // pad out categories and model names
  }
  else {
    ui->modelsList->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);           // minimise Index
    ui->modelsList->header()->resizeSection(1, ui->modelsList->header()->sectionSize(1) * 1.5); // pad out model names
  }
}

void MdiChild::refresh()
{
  bool expand = true; // TODO: restore user-preferred state
  clearCutList();
  modelsListModel->refresh();
  if (expand)
    ui->modelsList->expandAll();
  if (lastSelectedModel > -1) {
    setSelectedModel(lastSelectedModel);
    lastSelectedModel = -1;
  }
  updateNavigation();
  updateTitle();
}

void MdiChild::onItemActivated(const QModelIndex index)
{
  if (modelsListModel->isModelType(index)) {
    int mIdx = modelsListModel->getModelIndex(index);
    if (mIdx < 0 || mIdx >= (int)radioData.models.size())
      return;
    if (radioData.models[mIdx].isEmpty())
      newModel(mIdx);
    else
      openModelEditWindow(mIdx);
  }
  else if (modelsListModel->isCategoryType(index)) {
    ui->modelsList->edit(index);
  }
}

void MdiChild::onItemSelected(const QModelIndex &)
{
  updateNavigation();
}

void MdiChild::onCurrentItemChanged(const QModelIndex &, const QModelIndex &)
{
  updateNavigation();
}

void MdiChild::onDataChanged(const QModelIndex & index)
{
  if (!modelsListModel->isCategoryType(index))
    return;

  int categoryIndex = modelsListModel->getCategoryIndex(index);
  if (categoryIndex < 0 || categoryIndex >= (int)radioData.categories.size()) {
    return;
  }
  strcpy(radioData.categories[categoryIndex].name, modelsListModel->data(index, 0).toString().left(sizeof(CategoryData::name)-1).toStdString().c_str());

  setWindowModified(true);
  emit modified();
}

/*
 * Get info from data model
*/

QModelIndex MdiChild::getCurrentIndex() const
{
  return ui->modelsList->selectionModel()->currentIndex();
}

int MdiChild::getCurrentCategory() const
{
  return modelsListModel->getCategoryIndex(getCurrentIndex());
}

int MdiChild::countSelectedCats() const
{
  int ret = 0;

  foreach (QModelIndex index, ui->modelsList->selectionModel()->selectedRows()) {
    if (index.isValid() && modelsListModel->isCategoryType(index))
      ++ret;
  }
  return ret;
}

bool MdiChild::hasSelectedCat()
{
  return modelsListModel->isCategoryType(getCurrentIndex());
}

QVector<int> MdiChild::getSelectedCategories() const
{
  QVector<int> cats;
  foreach (QModelIndex index, ui->modelsList->selectionModel()->selectedRows()) {
    if (index.isValid() && modelsListModel->isCategoryType(index))
      cats.append(modelsListModel->getCategoryIndex(index));
  }
  return cats;
}

int MdiChild::getCurrentModel() const
{
  return modelsListModel->getModelIndex(getCurrentIndex());
}

int MdiChild::countSelectedModels() const
{
  int ret = 0;

  foreach (QModelIndex index, ui->modelsList->selectionModel()->selectedRows()) {
    if (index.isValid() && modelsListModel->isModelType(index) && !radioData.models.at(modelsListModel->getModelIndex(index)).isEmpty())
      ++ret;
  }
  return ret;
}

bool MdiChild::hasSelectedModel()
{
  return modelsListModel->isModelType(getCurrentIndex());
}

bool MdiChild::setSelectedModel(const int modelIndex)
{
  QModelIndex idx = modelsListModel->getIndexForModel(modelIndex);
  if (idx.isValid()) {
    ui->modelsList->scrollTo(idx);
    ui->modelsList->setCurrentIndex(idx);
    return true;
  }
  return false;
}

QVector<int> MdiChild::getSelectedModels() const
{
  QVector<int> models;
  foreach (QModelIndex index, ui->modelsList->selectionModel()->selectedRows()) {
    if (index.isValid() && modelsListModel->isModelType(index))
      models.append(modelsListModel->getModelIndex(index));
  }
  return models;
}

/*
 * Misc. internal event handlers
*/

void MdiChild::updateTitle()
{
  QString title =  "[*]" + userFriendlyCurrentFile();  // + " (" + firmware->getName() + QString(")");
  int availableEEpromSize = modelsListModel->getAvailableEEpromSize();
  if (availableEEpromSize >= 0) {
    title += QString(" - %1 ").arg(availableEEpromSize) + tr("free bytes");
  }
  QFileInfo fi(curFile);
  if (!isUntitled && !fi.isWritable()) {
    title += QString(" (%1)").arg(tr("read only"));
  }
  setWindowTitle(title);
}

void MdiChild::setModified()
{
  refresh();
  setWindowModified(true);
  emit modified();
}

void MdiChild::onFirmwareChanged()
{
  Firmware * previous = firmware;
  firmware = getCurrentFirmware();
  //qDebug() << "onFirmwareChanged" << previous->getName() << "=>" << firmware->getName();
  if (!Boards::isBoardCompatible(previous->getBoard(), firmware->getBoard())) {
    if (!convertStorage(previous->getBoard(), firmware->getBoard())) {
      closeFile(true);
      return;
    }
    setModified();
  }
}

/*
 * Categories CRUD
*/

void MdiChild::categoryAdd()
{
  /*: Translators do NOT use accent for this, this is the default category name on Horus. */
  CategoryData category(qPrintable(tr("New category")));
  radioData.categories.push_back(category);
  setModified();
  QModelIndex idx = modelsListModel->getIndexForCategory(radioData.categories.size()-1);
  if (idx.isValid()) {
    ui->modelsList->scrollTo(idx);
    ui->modelsList->setCurrentIndex(idx);
    ui->modelsList->edit(idx);
  }
}

// NOTE: this does not refresh the data/view, need to call setModified() at some point afterwards.
bool MdiChild::deleteCategory(int categoryIndex, QString * error)
{
  if (categoryIndex < 0)
    categoryIndex = modelsListModel->getCategoryIndex(getCurrentIndex());
  if (categoryIndex < 0 || categoryIndex >= (int)radioData.categories.size()) {
    if (error)
      *error = tr("Category index out of range.");
    return false;
  }
  if (radioData.categories.size() <= 1) {
    if (error)
      *error = tr("Cannot delete the last category.");
    return false;
  }
  if (countUsedModels(categoryIndex)) {
    if (error)
      *error = tr("This category is not empty!");
    return false;
  }

  radioData.categories.erase(radioData.categories.begin() + categoryIndex);

  for (unsigned i=0; i < radioData.models.size(); ++i) {
    ModelData & model = radioData.models.at(i);
    if (model.used && model.category > categoryIndex) {
      --radioData.models[i].category;
    }
  }

  return true;
}

void MdiChild::deleteSelectedCats()
{
  bool modified = false;
  QString error;
  QVector<int> cats = getSelectedCategories();
  std::sort(cats.begin(), cats.end());
  std::reverse(cats.begin(), cats.end());
  foreach (const int cat, cats) {
    error.clear();
    if (deleteCategory(cat, &error))
      modified = true;
    else
      showWarning(error);
  }
  if (modified)
    setModified();
}

/*
 * Models CRUD
*/

void MdiChild::checkAndInitModel(int row)
{
  if (row < (int)radioData.models.size() && radioData.models[row].isEmpty()) {
    radioData.models[row].setDefaultValues(row, radioData.generalSettings);
    setModified();
  }
}

void MdiChild::findNewDefaultModel(const unsigned startAt)
{
  for (unsigned i = startAt; i < radioData.models.size(); ++i) {
    if (!radioData.models[i].isEmpty()) {
      radioData.setCurrentModel(i);
      return;
    }
  }
  if (startAt > 0)
    findNewDefaultModel(0);  // restart search from beginning
  else
    radioData.setCurrentModel(0);
}

// NOTE: insertModelRows() does not update the TreeModel, only modifies radioData.models[] array by inserting row(s) of blank model(s).
//  TreeModel::refresh() needs to be called at some point afterwards to sync the data.
// This invalidates any model indices stored previously.
bool MdiChild::insertModelRows(int atModelIdx, int count)
{
  const unsigned maxModels = firmware->getCapability(Models);
  if (atModelIdx < 0)
    return false;

  for (int i=0; i < count; ++i) {
    //qDebug() << atRow << atRow.row() + i << modelIdx + i << maxModels << radioData.models.size();
    if (maxModels > 0 && radioData.models.size() >= maxModels) {
      // trim the array, unless the last model slot is being used.
      if (radioData.models[maxModels-1].isEmpty()) {
        radioData.models.pop_back();
      }
      else {
        showWarning(tr("Cannot insert model, last model in list would be deleted."));
        return false;  // TODO: perhaps something more elegant...
      }
    }
    // add a placeholder model
    radioData.models.insert(radioData.models.begin() + atModelIdx + i, ModelData());
    // adjust current model index if needed
    if ((int)radioData.generalSettings.currModelIndex >= atModelIdx + i)
      findNewDefaultModel(radioData.generalSettings.currModelIndex + 1);
  }
  return true;
}

// Finds the first empty slot and inserts the model into it. In case of category-style models, will append to end of list.
// Return -1 if no slot was found, otherwise new array index.
//  TreeModel::refresh() needs to be called at some point afterwards to sync the data.
int MdiChild::modelAppend(const ModelData model)
{
  int newIdx = -1;
  int trySlot = 0;
  // try to find the next empty slot
  for ( ; trySlot < (int)radioData.models.size(); ++trySlot) {
    if (radioData.models[trySlot].isEmpty()) {
      newIdx = trySlot;
      radioData.models[newIdx] = model;
      break;
    }
  }
  // if no empty slots then check if we can append it
  if (newIdx < 0 && (firmware->getCapability(Models) == 0 || trySlot < firmware->getCapability(Models) - 1)) {
    radioData.models.push_back(model);
    newIdx = radioData.models.size() - 1;
  }
  return newIdx;
}

int MdiChild::newModel(int modelIndex, int categoryIndex)
{
  if (modelIndex < 0)
    modelIndex = modelAppend(ModelData());

  if (modelIndex < 0 || modelIndex >= (int)radioData.models.size()) {
    showWarning(tr("Cannot add model, could not find an available model slot."));
    return -1;
  }

  if (categoryIndex < 0)
    categoryIndex = modelsListModel->getCategoryIndex(getCurrentIndex());

  bool isNewModel = radioData.models[modelIndex].isEmpty();
  checkAndInitModel(modelIndex);
  if (isNewModel && firmware->getCapability(Capability::HasModelCategories) && categoryIndex > -1) {
    radioData.models[modelIndex].category = categoryIndex;
    strcpy(radioData.models[modelIndex].filename, radioData.getNextModelFilename().toStdString().c_str());
    /*: Translators: do NOT use accents here, this is a default model name. */
    strcpy(radioData.models[modelIndex].name, qPrintable(tr("New model")));  // TODO: Why not just use existing default model name?
  }
  // Only set the default model if we just added the first one.
  if (countUsedModels() == 1) {
    radioData.setCurrentModel(modelIndex);
  }
  setModified();
  setSelectedModel(modelIndex);
  //qDebug() << modelIndex << categoryIndex << isNewModel;

  if (isNewModel && g.newModelAction() == AppData::MODEL_ACT_WIZARD)
    openModelWizard(modelIndex);
  else if (g.newModelAction() == AppData::MODEL_ACT_EDITOR)
    openModelEditWindow(modelIndex);

  return modelIndex;
}


// NOTE: deleteModelss() does not update the TreeModel, only modifies radioData.models[] array by clearing the model data.
// If (removeModelSlotsWhenDeleting == true) then removes array rows entirely (and pads w/blank model at the end if needed).
//  TreeModel::refresh() needs to be called at some point afterwards to sync the data
// We delete using stored indexes because actual indexes may change during inserts/deletes.
//   Obviously this only works before the stored indexes get updated in TreeModel::refresh().
unsigned MdiChild::deleteModels(const QVector<int> modelIndices)
{
  unsigned deletes = 0;
  int idx;

  for (int i = (int)radioData.models.size() - 1; i > -1; --i) {
    idx = radioData.models.at(i).modelIndex;
    if (idx > -1 && modelIndices.contains(idx)) {
      radioData.models[i].clear();
      if (g.removeModelSlots() || firmware->getCapability(Models) == 0) {
        radioData.models.erase(radioData.models.begin() + i);
        // append padding rows at the end if needed
        if (firmware->getCapability(Models) > 0)
          insertModelRows(radioData.models.size(), 1);
      }
      ++deletes;
      // adjust current model index if needed
      if ((int)radioData.generalSettings.currModelIndex >= idx)
        findNewDefaultModel(qMax((int)radioData.generalSettings.currModelIndex - 1, 0));

    }
    //qDebug() << "i:" << i << "modelIndex:" << idx << "deletes:" << deletes;
  }

  if (deletes)
    setModified();

  return deletes;
}

bool MdiChild::deleteModel(const int modelIndex)
{
  QVector<int> list = QVector<int>() << modelIndex;
  if (deleteModels(list) == 1)
    return true;
  else
    return false;
}

void MdiChild::deleteSelectedModels()
{
  deleteModels(getSelectedModels());
}

void MdiChild::moveModelsToCategory(const QVector<int> models, const int toCategoryId)
{
  if (toCategoryId < 0 || !models.size())
    return;

  bool modified = false;
  //QVector<int> models = getSelectedModels();
  foreach(const int model, models) {
    if (model < 0 || model >= (int)radioData.models.size())
      continue;

    if (radioData.models[model].category != toCategoryId) {
      radioData.models[model].category = toCategoryId;
      modified = true;
    }
  }
  if (modified)
    setModified();
}

void MdiChild::moveSelectedModelsToCat(const int toCategoryId)
{
  moveModelsToCategory(getSelectedModels(), toCategoryId);
}

unsigned MdiChild::countUsedModels(const int categoryId)
{
  unsigned count = 0;
  for (unsigned i=0; i < radioData.models.size(); ++i) {
    ModelData & model = radioData.models.at(i);
    if (!model.isEmpty() && (categoryId < 0 || model.category == categoryId))
      ++count;
  }
  return count;
}

void MdiChild::pasteModelData(const QMimeData * mimeData, const QModelIndex row, bool insert, bool move)
{
  QVector<ModelData> modelsList;
  if (!TreeModel::decodeMimeData(mimeData, &modelsList))
    return;

  bool modified = false;
  int modelIdx = modelsListModel->getModelIndex(row);
  int categoryIdx = modelsListModel->getCategoryIndex(row);
  unsigned inserts = 0;
  QVector<int> deletesList;

  // Force DnD moves from other file windows to be copy actions because we don't want to delete our models.
  bool hasOwnData = modelsListModel->hasOwnMimeData(mimeData);
  move = (move && hasOwnData);

  //qDebug().nospace() << "row: " << row << "; ins: " << insert << "; mv: " << move << "; row modelIdx: " << modelIdx << "; row categoryIdx: " << categoryIdx << "; removeSlots: " << removeModelSlotsWhenDeleting;

  // Model data
  for (int i=0; i < modelsList.size(); ++i) {
    int origMdlIdx = hasOwnData ? modelsList.at(i).modelIndex : -1;               // where is the modeul in *our* current array?
    bool doMove = (origMdlIdx > -1 && origMdlIdx < (int)radioData.models.size() && (move || cutModels.contains(origMdlIdx)));  // DnD-moved or clipboard cut
    bool ok = true;

    if (modelIdx == -1 || (!insert && modelIdx >= (int)radioData.models.size())) {
      // This handles pasting onto a category label or pasting past the end
      // of a category when pasting multiple models.
      modelIdx = modelAppend(modelsList[i]);
      if (modelIdx < 0) {
        ok = false;
        showWarning(tr("Cannot paste model, out of available model slots."));
      }
    }
    else if (insert) {
      ok = insertModelRows(modelIdx, 1);
      if (ok) {
        radioData.models[modelIdx] = modelsList[i];
        ++inserts;
      }
    }
    else {  // pasting on top of a slot
      if (!radioData.models[modelIdx].isEmpty() && !deletesList.contains(modelIdx))
        ok = askQuestion(tr("You are replacing an existing model, are you sure?")) == QMessageBox::Yes;
      if (ok)
        radioData.models[modelIdx] = modelsList[i];
    }

    if (ok) {
      // We don't want to create an index value conflict so use an invalid one (it will get updated after we're done here)
      //   this is esp. important because otherwise we may delete this model during a move operation (eg. after a cut)
      radioData.models[modelIdx].modelIndex = -modelIdx;
      // Set the destination category, so a user can copy/paste across categories.
      if (categoryIdx > -1)
        radioData.models[modelIdx].category = categoryIdx;
      strcpy(radioData.models[modelIdx].filename, radioData.getNextModelFilename().toStdString().c_str());
      lastSelectedModel = modelIdx;  // after refresh the last pasted model will be selected
      modified = true;
      if (doMove) {
        deletesList.append(origMdlIdx);
        removeModelFromCutList(origMdlIdx);
      }
    }
    //qDebug().nospace() << "i: " << i << "; modelIdx:" << modelIdx << "; origMdlIdx: " << origMdlIdx << "; doMove: " << doMove << "; inserts:" << inserts << "; deletes: " << deletesList;

    ++modelIdx;
  }

  if (deletesList.size()) {
    deleteModels(deletesList);
  }
  if (modified) {
    setModified();
  }
}

/*
 * General settings CRUD
*/

void MdiChild::pasteGeneralData(const QMimeData * mimeData)
{
  GeneralSettings gs;
  bool hasGenSettings = false;

  if (!TreeModel::decodeMimeData(mimeData, NULL, &gs, &hasGenSettings))
    return;

  if (hasGenSettings && askQuestion(tr("Do you want to overwrite radio general settings?")) == QMessageBox::Yes) {
    radioData.generalSettings = gs;
    setModified();
  }
}

void MdiChild::generalEdit()
{
  GeneralEdit * t = new GeneralEdit(this, radioData, firmware);
  connect(t, &GeneralEdit::modified, this, &MdiChild::setModified);
  t->show();
}

void MdiChild::copyGeneralSettings()
{
  QMimeData * mimeData = modelsListModel->getGeneralMimeData();
  modelsListModel->getHeaderMimeData(mimeData);
  QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void MdiChild::pasteGeneralSettings()
{
  if (hasClipboardData(1)) {
    pasteGeneralData(QApplication::clipboard()->mimeData());
  }
}

/*
 * Action targets
*/

void MdiChild::copy()
{
  QMimeData * mimeData = modelsListModel->getModelsMimeData(ui->modelsList->selectionModel()->selectedRows());
  modelsListModel->getHeaderMimeData(mimeData);
  QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
  clearCutList();  // clear the list by default, populate afterwards, eg. in cut().
}

void MdiChild::cut()
{
  copy();
  cutModels = getSelectedModels();
  modelsListModel->markItemsForCut(ui->modelsList->selectionModel()->selectedIndexes());
}

void MdiChild::removeModelFromCutList(const int modelIndex)
{
  int idx = cutModels.indexOf(modelIndex);
  if (idx > -1) {
    cutModels.remove(idx);
    modelsListModel->markItemForCut(modelsListModel->getIndexForModel(modelIndex), false);
  }
}

void MdiChild::clearCutList()
{
  foreach (const int id, cutModels) {
    removeModelFromCutList(id);
  }
}

// type = 0 for models (default), 1 for general radio data
bool MdiChild::hasClipboardData(const quint8 type) const
{
  if (!type) {
    return modelsListModel->hasModelsMimeData(QApplication::clipboard()->mimeData());
  }
  else {
    return modelsListModel->hasGenralMimeData(QApplication::clipboard()->mimeData());
  }
}

void MdiChild::paste()
{
  if (hasClipboardData()) {
    pasteModelData(QApplication::clipboard()->mimeData(), getCurrentIndex());
  }
}

void MdiChild::insert()
{
  if (hasClipboardData()) {
    pasteModelData(QApplication::clipboard()->mimeData(), getCurrentIndex(), true);
  }
}

void MdiChild::edit()
{
  onItemActivated(getCurrentIndex());
}

void MdiChild::confirmDelete()
{
  if (hasSelectedModel()) {
    if (!countSelectedModels() || askQuestion(tr("Delete %n selected model(s)?", 0, countSelectedModels())) == QMessageBox::Yes) {
      deleteSelectedModels();
    }
  }
  else if (hasSelectedCat()) {
    if (askQuestion(tr("Delete %n selected category(ies)?", 0, countSelectedCats())) == QMessageBox::Yes) {
      deleteSelectedCats();
    }
  }
}

void MdiChild::modelAdd()
{
  int modelIdx = -1;
  // add to currently selected empty slot?
  if (modelsListModel->isModelType(getCurrentIndex())) {
    int mIdx = modelsListModel->getModelIndex(getCurrentIndex());
    if (mIdx > -1 && mIdx < (int)radioData.models.size() && radioData.models[mIdx].isEmpty()) {
      modelIdx = mIdx;
    }
  }
  newModel(modelIdx);
}

void MdiChild::modelDuplicate()
{
  int srcModelIndex = getCurrentModel();
  if (srcModelIndex < 0) {
    return;
  }

  int newIdx = modelAppend(ModelData(radioData.models[srcModelIndex]));
  if (newIdx > -1) {
    newModel(newIdx);
  }
  else {
    showWarning(tr("Cannot duplicate model, could not find an available model slot."));
  }
}

void MdiChild::onModelMoveToCategory()
{
  if (!sender()) {
    return;
  }
  bool ok = false;
  int toCatId = sender()->property("categoryId").toInt(&ok);
  if (ok && toCatId >= 0) {
    moveSelectedModelsToCat(toCatId);
  }
}

void MdiChild::modelEdit()
{
  openModelEditWindow(getCurrentModel());
}

void MdiChild::wizardEdit()
{
  openModelWizard(getCurrentModel());
}

void MdiChild::openModelWizard(int row)
{
  if (row < 0 && (row = getCurrentModel()) < 0)
    return;

  WizardDialog * wizard = new WizardDialog(radioData.generalSettings, row+1, radioData.models[row], this);
  int res = wizard->exec();
  if (res == QDialog::Accepted && wizard->mix.complete /*TODO rather test the exec() result?*/) {
    radioData.models[row] = wizard->mix;
    radioData.fixModelFilenames();
    setModified();
    setSelectedModel(row);
  }
}

void MdiChild::openModelEditWindow(int row)
{
  if (row < 0 && (row = getCurrentModel()) < 0)
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  checkAndInitModel(row);
  ModelData & model = radioData.models[row];
  gStopwatch.restart();
  gStopwatch.report("ModelEdit creation");
  ModelEdit * t = new ModelEdit(this, radioData, (row), firmware);
  gStopwatch.report("ModelEdit created");
  t->setWindowTitle(tr("Editing model %1: ").arg(row+1) + QString(model.name) + QString("   (%1)").arg(userFriendlyCurrentFile()));
  connect(t, &ModelEdit::modified, this, &MdiChild::setModified);
  gStopwatch.report("STARTING MODEL EDIT");
  t->show();
  QApplication::restoreOverrideCursor();
  gStopwatch.report("ModelEdit shown");

}

void MdiChild::print(int model, const QString & filename)
{
  // TODO
  PrintDialog * pd = NULL;

  if (model>=0 && !filename.isEmpty()) {
    pd = new PrintDialog(this, firmware, radioData.generalSettings, radioData.models[model], filename);
  }
  else {
    pd = new PrintDialog(this, firmware, radioData.generalSettings, radioData.models[getCurrentModel()]);
  }

  if (pd) {
    pd->setAttribute(Qt::WA_DeleteOnClose, true);
    pd->show();
  }
}

void MdiChild::setDefault()
{
  int row = getCurrentModel();
  if (!radioData.models[row].isEmpty() && radioData.generalSettings.currModelIndex != (unsigned)row) {
    radioData.setCurrentModel(row);
  }
}

void MdiChild::radioSimulate()
{
  startSimulation(this, radioData, -1);
}

void MdiChild::modelSimulate()
{
  startSimulation(this, radioData, getCurrentModel());
}

void MdiChild::newFile(bool createDefaults)
{
  static int sequenceNumber = 1;
  isUntitled = true;
  curFile = QString("document%1.etx").arg(sequenceNumber++);
  updateTitle();

  if (createDefaults && firmware->getCapability(Capability::HasModelCategories)) {
    categoryAdd();
  }
}

bool MdiChild::loadFile(const QString & filename, bool resetCurrentFile)
{
  Storage storage(filename);
  if (!storage.load(radioData)) {
    QMessageBox::critical(this, CPN_STR_TTL_ERROR, storage.error());
    return false;
  }

  QString warning = storage.warning();
  if (!warning.isEmpty()) {
    QMessageBox::warning(this, CPN_STR_TTL_WARNING, warning);
  }

  if (resetCurrentFile) {
    setCurrentFile(filename);
  }

  //  For etx files this will never be true as any conversion occurs when parsing file
  if (!Boards::isBoardCompatible(storage.getBoard(), getCurrentBoard())) {
    if (!convertStorage(storage.getBoard(), getCurrentBoard(), true))
      return false;
    setModified();
  }
  else {
    refresh();
  }

  return true;
}

bool MdiChild::save()
{
  QFileInfo fi(curFile);
  if (isUntitled || !fi.isWritable() || fi.suffix().toLower() != "etx") {
    return saveAs(true);
  }
  else {
    return saveFile(curFile);
  }
}

bool MdiChild::saveAs(bool isNew)
{
  forceNewFilename();
  QFileInfo fi(curFile);
#ifdef __APPLE__
  QString filter;
#else
  QString filter(ETX_FILES_FILTER % YML_FILES_FILTER);
#endif

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), g.eepromDir() + "/" + fi.fileName(), filter);
  if (fileName.isEmpty())
    return false;
  g.eepromDir( QFileInfo(fileName).dir().absolutePath() );

  return saveFile(fileName, true);
}

bool MdiChild::saveFile(const QString & filename, bool setCurrent)
{
  radioData.fixModelFilenames();
  Storage storage(filename);
  bool result = storage.write(radioData);
  if (!result) {
    return false;
  }

  if (setCurrent) {
    setCurrentFile(filename);
  }

  return true;
}

void MdiChild::closeFile(bool force)
{
  forceCloseFlag = force;
  if (parentWindow)
    parentWindow->close();
  else
    this->close();
}

bool MdiChild::maybeSave()
{
  if (isWindowModified()) {
    int ret = askQuestion(tr("%1 has been modified.\nDo you want to save your changes?").arg(userFriendlyCurrentFile()),
                          (QMessageBox::Save | QMessageBox::Discard | (forceCloseFlag ? QMessageBox::NoButton : QMessageBox::Cancel)),
                          (forceCloseFlag ? QMessageBox::Save : QMessageBox::Cancel));

    if (ret == QMessageBox::Save)
      return save();
    else if (ret == QMessageBox::Discard)
      return true;
    else
      return false;
  }
  return true;
}

QString MdiChild::currentFile() const
{
  return curFile;
}

QString MdiChild::userFriendlyCurrentFile() const
{
  return QFileInfo(curFile).fileName();
}

void MdiChild::setCurrentFile(const QString & fileName)
{
  curFile = QFileInfo(fileName).canonicalFilePath();
  isUntitled = false;
  setWindowModified(false);
  updateTitle();

  QStringList files = g.recentFiles();
  files.removeAll(curFile);
  files.prepend(curFile);
  while (files.size() > g.historySize())
    files.removeLast();
  g.recentFiles(files);
}

void MdiChild::forceNewFilename(const QString & suffix, const QString & ext)
{
  curFile.replace(QRegExp("\\.(eepe|bin|hex|otx|etx)$"), suffix + "." + ext);
}

bool MdiChild::convertStorage(Board::Type from, Board::Type to, bool newFile)
{
  QMessageBox::StandardButtons btns;
  QMessageBox::StandardButton dfltBtn;

  if (from == Board::BOARD_X10 && to == Board::BOARD_JUMPER_T16) {
    if (displayT16ImportWarning() == false)
      return false;
  }

  QString q = tr("<p><b>Currently selected radio type (%1) is not compatible with file %3 (from %2), models and settings need to be converted.</b></p>").arg(Boards::getBoardName(to)).arg(Boards::getBoardName(from)).arg(userFriendlyCurrentFile());
  if (newFile) {
    q.append(tr("Do you wish to continue with the conversion?"));
    btns = (QMessageBox::Yes | QMessageBox::No);
    dfltBtn = QMessageBox::Yes;
  }
  else{
    q.append(tr("Choose <i>Apply</i> to convert the file, or <i>Close</i> to close it without conversion."));
    btns = (QMessageBox::Apply | QMessageBox::Close);
    dfltBtn = QMessageBox::Apply;
  }
  if (askQuestion(q, btns, dfltBtn) != dfltBtn)
    return false;

  RadioDataConversionState cstate(from, to, &radioData);
  if (!cstate.convert())
    return false;
  forceNewFilename("_converted");
  initModelsList();
  isUntitled = true;

  if (cstate.hasLogEntries(RadioDataConversionState::EVT_INF)) {
    auto * msgBox = new QDialog(Q_NULLPTR, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

    auto * tv = new ExportableTableView(msgBox);
    tv->setSortingEnabled(true);
    tv->verticalHeader()->hide();
    tv->setModel(cstate.getLogModel(RadioDataConversionState::EVT_INF, tv));
    tv->resizeColumnsToContents();
    tv->resizeRowsToContents();

    auto * btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);

    auto * lo = new QVBoxLayout(msgBox);
    lo->addWidget(new QLabel(tr("<b>The conversion generated some important messages, please review them below.</b>")));
    lo->addWidget(tv);
    lo->addWidget(btnBox);

    connect(btnBox, &QDialogButtonBox::accepted, msgBox, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, msgBox, &QDialog::reject);

    msgBox->setWindowTitle(tr("Companion :: Conversion Result for %1").arg(curFile));
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->show();  // modeless
  }

  return true;
}

void MdiChild::showWarning(const QString & msg)
{
  if (!msg.isEmpty())
    QMessageBox::warning(this, CPN_STR_APP_NAME, msg);
}

int MdiChild::askQuestion(const QString & msg, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
  return QMessageBox::question(this, CPN_STR_APP_NAME, msg, buttons, defaultButton);
}

void MdiChild::writeSettings()  // write to Tx
{
  if (g.confirmWriteModelsAndSettings()) {
    QMessageBox msgbox;
    msgbox.setText(tr("You are about to overwrite ALL models on the Radio."));
    msgbox.setInformativeText(tr("Do you want to continue?"));
    msgbox.setIcon(QMessageBox::Icon::Question);
    msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::Abort);
    msgbox.setDefaultButton(QMessageBox::Abort);

    QCheckBox *cb = new QCheckBox(tr("Don't show this message again"));
    msgbox.setCheckBox(cb);
    connect(cb, &QCheckBox::stateChanged, [=](const int &state){ g.confirmWriteModelsAndSettings(!state); });

    if (msgbox.exec() == QMessageBox::Abort)
      return;
  }

  Board::Type board = getCurrentBoard();

  if (Boards::getCapability(board, Board::HasSDCard)) {
    QString radioPath = findMassstoragePath("RADIO", true);
    qDebug() << "Searching for SD card, found" << radioPath;
    if (radioPath.isEmpty()) {
      qDebug() << "Radio SD card not found";
      QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Unable to find radio SD card!"));
      return;
    }
    if (saveFile(radioPath, false)) {
      QMessageBox::information(this, CPN_STR_TTL_INFO, tr("Saved models and settings to radio"));
    }
    else {
      qDebug() << "MdiChild::writeEeprom(): saveFile error";
      QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Error saving models and settings to radio!"));
    }
  }
  else {
    QString tempFile = generateProcessUniqueTempFileName("temp.bin");
    saveFile(tempFile, false);
    if (!QFileInfo(tempFile).exists()) {
      QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Cannot write temporary file!"));
      return;
    }
    FlashEEpromDialog * cd = new FlashEEpromDialog(this, tempFile);
    cd->exec();
  }
}

bool MdiChild::loadBackup()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open backup Models and Settings file"), g.eepromDir(), EEPROM_FILES_FILTER);
  if (fileName.isEmpty())
    return false;
  QFile file(fileName);

  if (!file.exists()) {
    QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Unable to find file %1!").arg(fileName));
    return false;
  }

  // TODO int index = getCurrentModel();

  int eeprom_size = file.size();
  if (!file.open(QFile::ReadOnly)) {  //reading binary file   - TODO HEX support
    QMessageBox::critical(this, CPN_STR_TTL_ERROR,
                          tr("Error opening file %1:\n%2.")
                          .arg(fileName)
                          .arg(file.errorString()));
    return false;
  }
  QByteArray eeprom(eeprom_size, 0);
  long result = file.read((char*)eeprom.data(), eeprom_size);
  file.close();

  if (result != eeprom_size) {
    QMessageBox::critical(this, CPN_STR_TTL_ERROR,
                          tr("Error reading file %1:\n%2.")
                          .arg(fileName)
                          .arg(file.errorString()));

    return false;
  }

#if 0
  std::bitset<NUM_ERRORS> errorsEeprom((unsigned long long)LoadBackup(radioData, (uint8_t *)eeprom.data(), eeprom_size, index));
  if (!errorsEeprom.test(ALL_OK)) {
    EEPROMInterface::showEepromErrors(this, CPN_STR_TTL_ERROR, tr("Invalid binary backup File %1").arg(fileName), (errorsEeprom).to_ulong());
    return false;
  }
  if (errorsEeprom.test(HAS_WARNINGS)) {
    EEPROMInterface::showEepromWarnings(this, CPN_STR_TTL_WARNING, errorsEeprom.to_ulong());
  }

  refresh(true);
  return true;
#else
  return false;
#endif
}
