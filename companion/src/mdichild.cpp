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

#include "mdichild.h"
#include "ui_mdichild.h"
#include "modeledit/modeledit.h"
#include "generaledit/generaledit.h"
#include "burnconfigdialog.h"
#include "printdialog.h"
#include "helpers.h"
#include "appdata.h"
#include "wizarddialog.h"
#include "flashfirmwaredialog.h"
#include "storage.h"
#include "radiointerface.h"
#include "radiodataconversionstate.h"
#include "filtereditemmodels.h"
#include "labels.h"

#include <algorithm>
#include <ExportableTableView>

MdiChild::MdiChild(QWidget * parent, QWidget * parentWin, Qt::WindowFlags f):
  QWidget(parent, f),
  ui(new Ui::MdiChild),
  modelsListModel(nullptr),
  labelsListModel(nullptr),
  modelsListProxyModel(nullptr),
  parentWindow(parentWin),
  radioToolbar(nullptr),
  modelsToolbar(nullptr),
  labelsToolbar(nullptr),
  lblLabels(nullptr),
  firmware(getCurrentFirmware()),
  lastSelectedModel(-1),
  isUntitled(true),
  forceCloseFlag(false),
  stateDataVersion(1),
  cboModelSortOrder(nullptr)
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

  ui->lstLabels->setContextMenuPolicy(Qt::CustomContextMenu);

  retranslateUi();

  connect(this, &MdiChild::customContextMenuRequested, this, &MdiChild::showContextMenu);
  connect(ui->modelsList, &QTreeView::activated, this, &MdiChild::onItemActivated);
  connect(ui->modelsList, &QTreeView::customContextMenuRequested, this, &MdiChild::showModelsListContextMenu);
  connect(ui->modelsList, &QTreeView::pressed, this, &MdiChild::onItemSelected);
  connect(ui->modelsList->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MdiChild::onCurrentItemChanged);
  connect(QGuiApplication::clipboard(), &QClipboard::dataChanged, this, &MdiChild::updateNavigation);
  connect(ui->lstLabels, &QTreeView::customContextMenuRequested, this, &MdiChild::showLabelsContextMenu);

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
      stream >> showLabelToolbar >> visMdl >> visGen;
      modelsToolbar->setVisible(visMdl);
      radioToolbar->setVisible(visGen);
    }
  }
}

MdiChild::~MdiChild()
{
  delete modelSortOrderItemModel;
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
         << false
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
  if (slotObj == nullptr)
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
  action.fill(nullptr, ACT_ENUM_END);

  addAct(ACT_GEN_EDT, "edit.png",     SLOT(generalEdit()),          tr("Alt+Shift+E"));
  addAct(ACT_GEN_CPY, "copy.png",     SLOT(copyGeneralSettings()),  tr("Ctrl+Alt+C"));
  addAct(ACT_GEN_PST, "paste.png",    SLOT(pasteGeneralSettings()), tr("Ctrl+Alt+V"));
  addAct(ACT_GEN_SIM, "simulate.png", SLOT(radioSimulate()),        tr("Alt+Shift+S"));

  addAct(ACT_ITM_EDT, "edit.png",  SLOT(edit()),          Qt::Key_Enter);
  addAct(ACT_ITM_DEL, "clear.png", SLOT(confirmDelete()), QKeySequence::Delete);

  addAct(ACT_MDL_ADD, "add.png",    SLOT(modelAdd()),   tr("Alt+A"));
  addAct(ACT_MDL_RTR, "open.png",   SLOT(loadBackup()), tr("Alt+R"));
  addAct(ACT_MDL_WIZ, "wizard.png", SLOT(wizardEdit()), tr("Alt+W"));

  addAct(ACT_LBL_ADD, "add.png",    SLOT(labelAdd()), tr("Alt-L"));
  addAct(ACT_LBL_DEL, "clear.png",  SLOT(labelDelete()), tr("Alt-L"));
  addAct(ACT_LBL_REN, "edit.png",   SLOT(labelRename()), tr("Alt-R"));
  addAct(ACT_LBL_MVU, "moveup.png", SLOT(labelMoveUp()), tr("Alt-+"));
  addAct(ACT_LBL_MVD, "movedown.png", SLOT(labelMoveDown()), tr("Alt--"));

  addAct(ACT_MDL_DFT, "currentmodel.png", SLOT(setDefault()),     tr("Alt+U"));
  addAct(ACT_MDL_PRT, "print.png",        SLOT(print()),          QKeySequence::Print);
  addAct(ACT_MDL_SIM, "simulate.png",     SLOT(modelSimulate()),  tr("Alt+S"));
  addAct(ACT_MDL_DUP, "duplicate.png",    SLOT(modelDuplicate()), QKeySequence::Underline);

  addAct(ACT_MDL_CUT, "cut.png",   SLOT(cut()),           QKeySequence::Cut);
  addAct(ACT_MDL_CPY, "copy.png",  SLOT(copy()),          QKeySequence::Copy);
  addAct(ACT_MDL_PST, "paste.png", SLOT(paste()),         QKeySequence::Paste);
  addAct(ACT_MDL_INS, "list.png",  SLOT(insert()),        QKeySequence::Italic);
  addAct(ACT_MDL_EXP, "save.png",  SLOT(modelExport()),   tr("Ctrl+Alt+S"));

  addAct(ACT_MDL_MOV, "arrow-right.png");
  QMenu * catsMenu = new QMenu(this);
  action[ACT_MDL_MOV]->setMenu(catsMenu);

  // set up the toolbars

  QToolButton * btn;
  QSize tbIcnSz(16, 16);
  QString tbCss = "QToolBar {border: 1px solid palette(midlight);}";

  // Add labels Label to bottom layout
  if(lblLabels)
    lblLabels->deleteLater();
  lblLabels = new QLabel(tr("Labels Management"));
  lblLabels->setStyleSheet("font-weight: bold");
  ui->bottomLayout->addWidget(lblLabels);

  if(labelsToolbar)
    labelsToolbar->deleteLater();
  labelsToolbar = new QToolBar(this);
  labelsToolbar->setObjectName("TB_LABELS");
  labelsToolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  labelsToolbar->setFloatable(false);
  labelsToolbar->setIconSize(tbIcnSz);
  labelsToolbar->setStyleSheet(tbCss);
  labelsToolbar->addAction(getAction(ACT_LBL_ADD));
  labelsToolbar->addAction(getAction(ACT_LBL_DEL));
  labelsToolbar->addAction(getAction(ACT_LBL_REN));
  labelsToolbar->addAction(getAction(ACT_LBL_MVU));
  labelsToolbar->addAction(getAction(ACT_LBL_MVD));
  ui->bottomLayout->addWidget(labelsToolbar);

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

  if (cboModelSortOrder)
    cboModelSortOrder->deleteLater();
  cboModelSortOrder = new QComboBox(this);
  modelSortOrderItemModel = RadioData::modelSortOrderItemModel();
  cboModelSortOrder->setModel(modelSortOrderItemModel);
  connect(cboModelSortOrder, QOverload<int>::of(&QComboBox::currentIndexChanged), [=] (int index) {
    radioData.sortOrder = index;
    setModified();
  });
  action.replace(ACT_GEN_SRT, radioToolbar->addWidget(cboModelSortOrder));

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
  modelsToolbar->addActions(getEditActions());
  modelsToolbar->addSeparator();
  modelsToolbar->addActions(getModelActions());
  if ((btn = qobject_cast<QToolButton *>(modelsToolbar->widgetForAction(action[ACT_MDL_ADD])))) {
    btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  }
  if ((btn = qobject_cast<QToolButton *>(modelsToolbar->widgetForAction(action[ACT_MDL_MOV])))) {
    btn->setPopupMode(QToolButton::InstantPopup);
  }
  ui->botToolbarLayout->addWidget(modelsToolbar);

  connect(radioToolbar, &QToolBar::visibilityChanged, this, &MdiChild::adjustToolbarLayout);
  connect(modelsToolbar, &QToolBar::visibilityChanged, this, &MdiChild::adjustToolbarLayout);
}

void MdiChild::updateNavigation()
{
  const int modelsSelected = countSelectedModels();
  const bool singleModelSelected = (modelsSelected == 1);
  const bool hasModelSlotSelcted = (getCurrentModel() > -1);
  const bool hasLabels = firmware->getCapability(Capability::HasModelLabels);
  const int numOnClipbrd = modelsListModel->countModelsInMimeData(QApplication::clipboard()->mimeData());
  const QString modelsRemvTxt = tr("%n Model(s)", "As in \"Copy 3 Models\" or \"Cut 1 Model\" or \"Delete 3 Models\" action).", modelsSelected);
  const QString modelsAddTxt = tr("%n Model(s)", "As in \"Paste 3 Models\" or \"Insert 1 Model.\"", numOnClipbrd);
  static const QString noSelection = tr("Nothing selected");
  static const QString sp = " ";
  static const QString ns;

  labelsToolbar->setVisible(hasLabels);
  ui->lstLabels->setVisible(hasLabels);
  lblLabels->setVisible(hasLabels);

  action[ACT_GEN_PST]->setEnabled(hasClipboardData(1));
  if (hasLabels) {
    cboModelSortOrder->blockSignals(true);
    cboModelSortOrder->setCurrentIndex(radioData.sortOrder);
    cboModelSortOrder->blockSignals(false);
  }
  action[ACT_GEN_SRT]->setVisible(hasLabels);

  action[ACT_MDL_CUT]->setEnabled(modelsSelected);
  action[ACT_MDL_CUT]->setText(tr("Cut") % (modelsSelected ? sp % modelsRemvTxt : ns));
  action[ACT_MDL_CPY]->setEnabled(modelsSelected);
  action[ACT_MDL_CPY]->setText(tr("Copy") % (modelsSelected ? sp % modelsRemvTxt : ns));
  action[ACT_MDL_PST]->setEnabled(numOnClipbrd);
  action[ACT_MDL_PST]->setText(tr("Paste") % (numOnClipbrd ? sp % modelsAddTxt : ns));
  action[ACT_MDL_INS]->setEnabled(numOnClipbrd && hasModelSlotSelcted);
  action[ACT_MDL_INS]->setText(tr("Insert") % QString(action[ACT_MDL_INS]->isEnabled() ? sp % modelsAddTxt : ns));
  action[ACT_MDL_EXP]->setEnabled(modelsSelected);
  action[ACT_MDL_EXP]->setText(tr("Export") % (modelsSelected ? sp % modelsRemvTxt : ns));
  action[ACT_MDL_MOV]->setVisible(false);
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
  cboModelSortOrder->setToolTip(tr("Radio Models Order"));

  action[ACT_ITM_EDT]->setText(tr("Edit Model"));
  action[ACT_ITM_DEL]->setText(tr("Delete"));

  action[ACT_LBL_ADD]->setText(tr("Add"));
  action[ACT_LBL_DEL]->setText(tr("Delete"));
  action[ACT_LBL_REN]->setText(tr("Rename"));
  action[ACT_LBL_MVU]->setText(tr("Move Up"));
  action[ACT_LBL_MVD]->setText(tr("Move Down"));

  action[ACT_MDL_ADD]->setText(tr("Add Model"));
  action[ACT_MDL_ADD]->setIconText(tr("Model"));
  action[ACT_MDL_EXP]->setText(tr("Export Model"));
  action[ACT_MDL_EXP]->setIconText(tr("Export"));
  action[ACT_MDL_RTR]->setText(tr("Restore from Backup"));
  action[ACT_MDL_WIZ]->setText(tr("Model Wizard"));
  action[ACT_MDL_DFT]->setText(tr("Set as Default"));
  action[ACT_MDL_PRT]->setText(tr("Print Model"));
  action[ACT_MDL_SIM]->setText(tr("Simulate Model"));
  action[ACT_MDL_DUP]->setText(tr("Duplicate Model"));

  radioToolbar->setWindowTitle(tr("Show Radio Actions Toolbar"));
  modelsToolbar->setWindowTitle(tr("Show Model Actions Toolbar"));
  labelsToolbar->setWindowTitle(tr("Show Labels Actions Toolbar"));
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

QList<QAction *> MdiChild::getEditActions()
{
  QList<QAction *> actGrp;
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
  actGrp.append(getAction(ACT_MDL_EXP));
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

QList<QAction *> MdiChild::getLabelsActions()
{
  QList<QAction *> actGrp;
  actGrp.append(getAction(ACT_LBL_ADD));
  actGrp.append(getAction(ACT_LBL_DEL));
  actGrp.append(getAction(ACT_LBL_REN));
  actGrp.append(getAction(ACT_LBL_MVU));
  actGrp.append(getAction(ACT_LBL_MVD));
  return actGrp;
}

QAction * MdiChild::getAction(const MdiChild::Actions type)
{
  if (type < ACT_ENUM_END)
    return action[type];
  else
    return nullptr;
}

void MdiChild::showModelsListContextMenu(const QPoint & pos)
{
  QModelIndex viewIndex = ui->modelsList->indexAt(pos);
  QMenu contextMenu;

  updateNavigation();

  if (modelsListModel->isModelType(getDataIndex(viewIndex))) {
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

void MdiChild::showLabelsContextMenu(const QPoint &pos)
{
  QMenu contextMenu;

  contextMenu.addAction(action[ACT_LBL_ADD]);
  contextMenu.addAction(action[ACT_LBL_DEL]);
  contextMenu.addAction(action[ACT_LBL_REN]);
  contextMenu.addAction(action[ACT_LBL_MVU]);
  contextMenu.addAction(action[ACT_LBL_MVD]);

  if (!contextMenu.isEmpty())
    contextMenu.exec(ui->lstLabels->mapToGlobal(pos));
}

void MdiChild::showContextMenu(const QPoint & pos)
{
  QMenu contextMenu;
  contextMenu.addAction(modelsToolbar->toggleViewAction());
  contextMenu.addAction(radioToolbar->toggleViewAction());
  if(firmware->getCapability(Capability::HasModelLabels))
    contextMenu.addAction(labelsToolbar->toggleViewAction());
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
  if (modelsListProxyModel)
    delete modelsListProxyModel;

  modelsListProxyModel = new ModelsListProxyModel();

  if (modelsListModel)
    delete modelsListModel;

  modelsListModel = new ModelsListModel(&radioData, this);
  connect(modelsListModel, &ModelsListModel::modelsDropped, this, &MdiChild::pasteModelData);
  connect(modelsListModel, &ModelsListModel::modelsRemoved, this, &MdiChild::deleteModels);
  connect(modelsListModel, &ModelsListModel::refreshRequested, this, &MdiChild::refresh);
  connect(modelsListModel, &QAbstractItemModel::dataChanged, this, &MdiChild::onDataChanged);

  modelsListProxyModel->setSourceModel(modelsListModel);

  ui->modelsList->setModel(modelsListProxyModel);
  ui->modelsList->selectionModel()->currentIndex().row();

  // Labels Editor + Model
  if (labelsListModel)
    delete labelsListModel;

  labelsListModel = new LabelsModel(modelsListProxyModel, ui->modelsList->selectionModel(), &radioData, this);
  connect(labelsListModel, &LabelsModel::modelChanged, this, &MdiChild::modelLabelsChanged);
  connect(labelsListModel, &LabelsModel::labelsFault, this, &MdiChild::labelsFault);
  ui->lstLabels->setModel(labelsListModel);
  ui->lstLabels->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->lstLabels->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  ui->lstLabels->setItemDelegate(new LabelEditTextDelegate);

  modelsListProxyModel->setFilter(labelsListModel); // TODO Used for filters when developed

  ui->modelsList->setIndentation(0);

  refresh();
  modelsListProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

  connect(ui->modelsList->header(), &QHeaderView::sectionDoubleClicked, [=] (int logicalIndex) {
    if (ui->modelsList->header()->isSortIndicatorShown()) {
      ui->modelsList->sortByColumn(-1, Qt::AscendingOrder); // turn off sort indicator shown and return proxy model to its unsorted order
      ui->modelsList->setSortingEnabled(false);             // disable clicking a column to sort
    }
    else {
      ui->modelsList->setSortingEnabled(true); // enable sorting by clicking a column
      ui->modelsList->sortByColumn(logicalIndex, Qt::AscendingOrder);
    }
  });

  if (firmware->getCapability(Capability::HasModelLabels)) {
    ui->modelsList->header()->resizeSection(0, ui->modelsList->header()->sectionSize(0) * 1.5); // pad out model names
    ui->modelsList->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);           // minimise rx #
  } else {
    ui->modelsList->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);           // minimise Index
    ui->modelsList->header()->resizeSection(1, ui->modelsList->header()->sectionSize(1) * 1.5); // pad out model names
  }
}

void MdiChild::refresh()
{
  clearCutList();
  modelsListModel->refresh();
  modelsListProxyModel->invalidate(); // force view refresh
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
  QModelIndex srcIdx = getDataIndex(index);
  if (modelsListModel->isModelType(srcIdx)) {
    int mIdx = modelsListModel->getModelIndex(srcIdx);
    if (mIdx < 0 || mIdx >= (int)radioData.models.size())
      return;
    if (radioData.models[mIdx].isEmpty())
      newModel(mIdx);
    else
      openModelEditWindow(mIdx);
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
  return;
}

/*
 * Get info from data model
*/

QModelIndex MdiChild::getCurrentIndex() const
{
  return getDataIndex(ui->modelsList->selectionModel()->currentIndex());
}

int MdiChild::getCurrentModel() const
{
  return modelsListModel->getModelIndex(getCurrentIndex());
}

QModelIndex MdiChild::getDataIndex(QModelIndex viewIndex) const
{
  return modelsListProxyModel->mapToSource(viewIndex);
}

int MdiChild::getDataModel(QModelIndex viewIndex) const
{
  return modelsListModel->getModelIndex(getDataIndex(viewIndex));
}

int MdiChild::countSelectedModels() const
{
  int ret = 0;

  foreach (QModelIndex viewIndex, ui->modelsList->selectionModel()->selectedRows()) {
    QModelIndex index = getDataIndex(viewIndex);
    if (index.isValid() && modelsListModel->isModelType(index) &&
        !radioData.models.at(modelsListModel->getModelIndex(index)).isEmpty())
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
    QModelIndex viewIdx = modelsListProxyModel->mapFromSource(idx);
    ui->modelsList->scrollTo(viewIdx);
    ui->modelsList->setCurrentIndex(viewIdx);
    return true;
  }
  return false;
}

QVector<int> MdiChild::getSelectedModels() const
{
  QVector<int> models;
  foreach (QModelIndex viewIndex, ui->modelsList->selectionModel()->selectedRows()) {
    QModelIndex index = getDataIndex(viewIndex);
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
 * Models CRUD
*/

void MdiChild::checkAndInitModel(int row)
{
  if (row < (int)radioData.models.size() && radioData.models[row].isEmpty()) {
    radioData.models[row].setDefaultValues(row, radioData.generalSettings);
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

// NOTE: insertModelRows() does not update the ModelsListModel, only modifies radioData.models[] array by inserting row(s) of blank model(s).
//  ModelsListModel::refresh() needs to be called at some point afterwards to sync the data.
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

// Finds the first empty slot and inserts the model into it.
// Return -1 if no slot was found, otherwise new array index.
//  ModelsListModel::refresh() needs to be called at some point afterwards to sync the data.
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

int MdiChild::newModel(int modelIndex)
{
  if (modelIndex < 0)
    modelIndex = modelAppend(ModelData());

  if (modelIndex < 0 || modelIndex >= (int)radioData.models.size()) {
    showWarning(tr("Cannot add model, could not find an available model slot."));
    return -1;
  }

  bool isNewModel = radioData.models[modelIndex].isEmpty();
  checkAndInitModel(modelIndex);

  // Only set the default model if we just added the first one.
  if (countUsedModels() == 1) {
    radioData.setCurrentModel(modelIndex);
  }
  setModelModified(modelIndex);
  setSelectedModel(modelIndex);

  if (isNewModel) {
    if (g.newModelAction() == AppData::MODEL_ACT_WIZARD)
      openModelWizard(modelIndex);
    else if (g.newModelAction() == AppData::MODEL_ACT_TEMPLATE)
      openModelTemplate(modelIndex);
    else if (g.newModelAction() == AppData::MODEL_ACT_PROMPT)
      openModelPrompt(modelIndex);
    else if (g.newModelAction() == AppData::MODEL_ACT_EDITOR)
      openModelEditWindow(modelIndex);
  }
  else if (g.newModelAction() == AppData::MODEL_ACT_EDITOR)
    openModelEditWindow(modelIndex);

  radioData.fixModelFilenames();
  return modelIndex;
}


// NOTE: deleteModelss() does not update the ModelsListModel, only modifies radioData.models[] array by clearing the model data.
// If (removeModelSlotsWhenDeleting == true) then removes array rows entirely (and pads w/blank model at the end if needed).
//  ModelsListModel::refresh() needs to be called at some point afterwards to sync the data
// We delete using stored indexes because actual indexes may change during inserts/deletes.
//   Obviously this only works before the stored indexes get updated in ModelsListModel::refresh().
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

unsigned MdiChild::countUsedModels()
{
  unsigned count = 0;
  for (unsigned i=0; i < radioData.models.size(); ++i) {
    ModelData & model = radioData.models.at(i);
    if (!model.isEmpty())
      ++count;
  }
  return count;
}

void MdiChild::pasteModelData(const QMimeData * mimeData, const QModelIndex row, bool insert, bool move)
{
  QVector<ModelData> modelsList;
  if (!ModelsListModel::decodeMimeData(mimeData, &modelsList))
    return;

  bool modified = false;
  int modelIdx = modelsListModel->getModelIndex(row);
  // unsigned inserts = 0;
  QVector<int> deletesList;

  // Force DnD moves from other file windows to be copy actions because we don't want to delete our models.
  bool hasOwnData = modelsListModel->hasOwnMimeData(mimeData);
  move = (move && hasOwnData);

  //qDebug().nospace() << "row: " << row << "; ins: " << insert << "; mv: " << move << "; row modelIdx: " << modelIdx;

  // Model data
  for (int i=0; i < modelsList.size(); ++i) {
    int origMdlIdx = hasOwnData ? modelsList.at(i).modelIndex : -1;               // where is the model in *our* current array?
    bool doMove = (origMdlIdx > -1 && origMdlIdx < (int)radioData.models.size() && (move || cutModels.contains(origMdlIdx)));  // DnD-moved or clipboard cut
    bool ok = true;

    if (modelIdx == -1 || (!insert && modelIdx >= (int)radioData.models.size())) {
      // This handles pasting past the end or when pasting multiple models.
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
        // ++inserts;
      }
    }
    else if (!deletesList.contains(modelIdx)) {
      // pasting on top of a slot
      if (radioData.models[modelIdx].isEmpty()) {
        radioData.models[modelIdx] = modelsList[i];
        ok = true;
      }
      else {
        QMessageBox msgBox;
        msgBox.setWindowTitle(CPN_STR_APP_NAME);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("Model already exists! Do you want to overwrite it or insert into a new slot?"));
        QPushButton *overwriteButton = msgBox.addButton(tr("Overwrite"),QMessageBox::ActionRole);
        QPushButton *insertButton = msgBox.addButton(tr("Insert"),QMessageBox::ActionRole);
        QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);

        msgBox.exec();

        if (msgBox.clickedButton() == overwriteButton) {
          radioData.models[modelIdx] = modelsList[i];
          ok = true;
        }
        else if (msgBox.clickedButton() == insertButton) {
          ok = insertModelRows(modelIdx, 1);
          if (ok) {
            radioData.models[modelIdx] = modelsList[i];
            // ++inserts;
          }
        }
        else if (msgBox.clickedButton() == cancelButton) {
          ok = false;
        }
      }
    }

    if (ok) {
      // We don't want to create an index value conflict so use an invalid one (it will get updated after we're done here)
      //   this is esp. important because otherwise we may delete this model during a move operation (eg. after a cut)
      radioData.models[modelIdx].modelIndex = -modelIdx;
      strcpy(radioData.models[modelIdx].filename, radioData.getNextModelFilename().toStdString().c_str());
      lastSelectedModel = modelIdx;  // after refresh the last pasted model will be selected
      modified = true;
      setModelModified(modelIdx, false);  // avoid unnecessary refreshes
      if (doMove) {
        deletesList.append(origMdlIdx);
        removeModelFromCutList(origMdlIdx);
      }
      radioData.addLabelsFromModels();
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

  if (!ModelsListModel::decodeMimeData(mimeData, nullptr, &gs, &hasGenSettings))
    return;

  if (hasGenSettings && askQuestion(tr("Do you want to overwrite radio general settings?")) == QMessageBox::Yes) {
    radioData.generalSettings = gs;
    setModified();
  }
}

void MdiChild::generalEdit()
{
  if (getModelEditDialogsList()->count() > 0) {
    QMessageBox::information(this, CPN_STR_APP_NAME, tr("Unable to Edit Radio Settings whilst models are open for editing."));
    return;
  }

  GeneralEdit * t = new GeneralEdit(this, radioData, firmware);
  connect(t, &GeneralEdit::modified, this, &MdiChild::setModified);
  connect(t, &GeneralEdit::internalModuleChanged, this, &MdiChild::onInternalModuleChanged);  // passed up from HardwarePanel >> GeneralEdit
  t->exec();
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
  QModelIndexList indexes = modelsListProxyModel->mapSelectionToSource(ui->modelsList->selectionModel()->selection()).indexes();
  QMimeData * mimeData = modelsListModel->getModelsMimeData(indexes);
  modelsListModel->getHeaderMimeData(mimeData);
  QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
  clearCutList();  // clear the list by default, populate afterwards, eg. in cut().
}

void MdiChild::cut()
{
  copy();
  cutModels = getSelectedModels();
  QModelIndexList indexes = modelsListProxyModel->mapSelectionToSource(ui->modelsList->selectionModel()->selection()).indexes();
  modelsListModel->markItemsForCut(indexes);
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
    return modelsListModel->hasGeneralMimeData(QApplication::clipboard()->mimeData());
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
  onItemActivated(ui->modelsList->selectionModel()->currentIndex());
}

void MdiChild::confirmDelete()
{
  if (hasSelectedModel()) {
    if (!countSelectedModels() || askQuestion(tr("Delete %n selected model(s)?", 0, countSelectedModels())) == QMessageBox::Yes) {
      deleteSelectedModels();
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
    setModelModified(row);
    setSelectedModel(row);
  }
}

void MdiChild::openModelEditWindow(int row)
{
  if (row < 0 && (row = getCurrentModel()) < 0)
    return;

  QDialog * med = getModelEditDialog(row);
  if (med) {
    med->activateWindow();
    med->raise();
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  checkAndInitModel(row);
  ModelData & model = radioData.models[row];
  gStopwatch.restart();
  gStopwatch.report("ModelEdit creation");
  ModelEdit * t = new ModelEdit(this, radioData, (row), firmware);
  gStopwatch.report("ModelEdit created");
  t->setWindowTitle(tr("Editing model %1: ").arg(row+1) + QString(model.name) + QString("   (%1)").arg(userFriendlyCurrentFile()));
  connect(t, &ModelEdit::modified, this, &MdiChild::setCurrentModelModified);
  gStopwatch.report("STARTING MODEL EDIT");
  t->show();
  QApplication::restoreOverrideCursor();
  gStopwatch.report("ModelEdit shown");

}

void MdiChild::print(int model, const QString & filename)
{
  // TODO
  PrintDialog * pd = nullptr;

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
    refresh();
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
  modelsListModel->setFilename(curFile);
  radioData.addLabel(tr("Favorites"));
  labelsListModel->buildLabelsList();
}

bool MdiChild::loadFile(const QString & filename, bool resetCurrentFile)
{
  if (getStorageType(filename) == STORAGE_TYPE_YML) {
    newFile(false);
    resetCurrentFile = false;
  }

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

  if (radioData.generalSettings.fix6POSCalibration())
    setModified();

  //  set after successful import
  if (getStorageType(filename) == STORAGE_TYPE_YML)
    setModified();

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
  QString filter(ETX_FILES_FILTER);
  QString fileName;

  do
  {
    fileName = QFileDialog::getSaveFileName(this, tr("Save As"), g.eepromDir() + "/" + fi.fileName(), filter);
    if (fileName.isEmpty())
      return false;
    if (QFileInfo(fileName).suffix().toLower() != "etx")
      QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Invalid file extension!"));
  }
  while (QFileInfo(fileName).suffix().toLower() != "etx");

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

  g.eepromDir(QFileInfo(filename).dir().absolutePath());

  for (int i = 0; i < (int)radioData.models.size(); i++) {
    if (!radioData.models[i].isEmpty())
      radioData.models[i].modelUpdated = false;
  }

  refresh();

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
  modelsListModel->setFilename(curFile);

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
    auto * msgBox = new QDialog(nullptr, Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

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

void MdiChild::writeSettings(StatusDialog * status)  // write to Tx
{
  if (g.confirmWriteModelsAndSettings()) {
    QMessageBox msgbox;
    msgbox.setText(tr("You are about to overwrite ALL models on the Radio."));
    msgbox.setInformativeText(tr("Do you want to continue?"));
    msgbox.setIcon(QMessageBox::Icon::Question);
    msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::Abort);
    msgbox.setDefaultButton(QMessageBox::Abort);

    QCheckBox *cb = new QCheckBox(tr("Do not show this message again"));
    msgbox.setCheckBox(cb);
    connect(cb, &QCheckBox::stateChanged, [=](const int &state){ g.confirmWriteModelsAndSettings(!state); });

    if (msgbox.exec() == QMessageBox::Abort)
      return;
  }

  QString radioPath = findMassstoragePath("RADIO", true);
  qDebug() << "Searching for SD card, found" << radioPath;
  if (radioPath.isEmpty()) {
    qDebug() << "Radio SD card not found";
    QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Unable to find radio SD card!"));
    return;
  }

  if (saveFile(radioPath, false)) {
    status->hide();
    QMessageBox::information(this, CPN_STR_TTL_INFO, tr("Models and settings written to radio"));
  }
  else {
    status->hide();
    QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Error writing models and settings to radio!"));
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

QList<QDialog *> * MdiChild::getModelEditDialogsList()
{
  QList<QDialog *> *ret = new QList<QDialog *>();

  QList<QDialog *> dlgs = findChildren<QDialog *>();

  for (QDialog *dlg : dlgs) {
    ModelEdit * med = dynamic_cast<ModelEdit *>(dlg);
    if (med)
      ret->append(dlg);
  }

  return ret;
}

QDialog * MdiChild::getModelEditDialog(int row)
{
  QList<QDialog *> *dlgs = getModelEditDialogsList();

  for (QDialog *dlg : *dlgs) {
    ModelEdit * med = dynamic_cast<ModelEdit *>(dlg);
    if (med && med->getModelId() == row)
      return med;
  }

  return nullptr;
}

void MdiChild::onInternalModuleChanged()
{
  FilteredItemModel * fim = new FilteredItemModel(ModuleData::protocolItemModel(radioData.generalSettings), 0 + 1/*flag cannot be 0*/);

  int cnt = 0;

  for (unsigned int i = 0; i < radioData.models.size(); i++) {
    ModuleData & module = radioData.models[i].moduleData[0];
    bool found = false;

    for (int j = 0; j < fim->rowCount(); j++) {
      if (fim->data(fim->index(j, 0), AbstractItemModel::IMDR_Id).toInt() == (int)module.protocol) {
        found = true;
        break;
      }
    }

    if (!found) {
      module.clear();
      cnt++;
    }
  }

  if (cnt > 0) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Internal module protocol changed to <b>OFF</b> for %1 models!").arg(cnt));
    setModified();
  }

  delete fim;
}

void MdiChild::openModelTemplate(int row)
{
  if (row < 0 && (row = getCurrentModel()) < 0)
    return;

  QString filename = QFileDialog::getOpenFileName(this, tr("Select a model template file"), QDir::toNativeSeparators(g.profile[g.id()].sdPath() + "/TEMPLATES"), YML_FILES_FILTER);

  if (filename.isEmpty())
    return;

  //  validate like read single model
  //  if okay copy into current model slot

  RadioData data;

  Storage storage(filename);
  if (!storage.load(data)) {
    QMessageBox::critical(this, CPN_STR_TTL_ERROR, storage.error());
    return;
  }

  QString warning = storage.warning();
  if (!warning.isEmpty()) {
    QMessageBox::warning(this, CPN_STR_TTL_WARNING, warning);
  }

  if (radioData.generalSettings.fix6POSCalibration())
    setModified();

  radioData.models[row] = data.models[0];

  //  reset module bindings
  for (int i = 0; i < CPN_MAX_MODULES; i++) {
    radioData.models[row].moduleData[i].modelId = row + 1;
  }

  setModelModified(row);
}

void MdiChild::openModelPrompt(int row)
{
  if (row < 0 && (row = getCurrentModel()) < 0)
    return;

  QMessageBox msgBox;
  msgBox.setWindowTitle(CPN_STR_APP_NAME);
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setText(tr("Add a new model using"));
  QPushButton *defaultsButton = msgBox.addButton(tr("Defaults"),QMessageBox::ActionRole);
  QPushButton *editButton = msgBox.addButton(tr("Edit"),QMessageBox::ActionRole);
  QPushButton *wizardButton = msgBox.addButton(tr("Wizard"),QMessageBox::ActionRole);
  QPushButton *templateButton = msgBox.addButton(tr("Template"),QMessageBox::ActionRole);
  QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);

  msgBox.exec();

  if (msgBox.clickedButton() == cancelButton) {
      if (!deleteModel(row))
        QMessageBox::critical(this, CPN_STR_APP_NAME, tr("Failed to remove temporary model!"));
      return;
  }
  else if (msgBox.clickedButton() == defaultsButton) {
      //  nothing to do here
      return;
  }
  else if (msgBox.clickedButton() == editButton) {
      openModelEditWindow(row);
  }
  else if (msgBox.clickedButton() == wizardButton) {
      openModelWizard(row);
  }
  else if (msgBox.clickedButton() == templateButton) {
      openModelTemplate(row);
  }

  return;
}

void MdiChild::modelExport()
{
  exportSelectedModels();
}

void MdiChild::labelAdd()
{
  labelsListModel->insertRow(0);
  QModelIndex newind = labelsListModel->index(0,0);
  ui->lstLabels->setCurrentIndex(newind);
  ui->lstLabels->edit(newind);
  setWindowModified(true);
}

void MdiChild::labelDelete()
{
  int row = ui->lstLabels->selectionModel()->currentIndex().row();
  if(row < 0) return;
  labelsListModel->removeRow(row);
  setWindowModified(true);
}

void MdiChild::labelRename()
{
  int row = ui->lstLabels->selectionModel()->currentIndex().row();
  if(row < 0) return;
  QModelIndex newind = labelsListModel->index(row,0);
  ui->lstLabels->setCurrentIndex(newind);
  ui->lstLabels->edit(newind);
}

void MdiChild::labelMoveUp()
{
  int row = ui->lstLabels->selectionModel()->currentIndex().row();
  if(row == 0) return;
  radioData.swapLabel(row, row-1);
  labelsListModel->buildLabelsList();
  ui->lstLabels->selectionModel()->setCurrentIndex(labelsListModel->index(row - 1,0), QItemSelectionModel::ClearAndSelect);
}

void MdiChild::labelMoveDown()
{
  int row = ui->lstLabels->selectionModel()->currentIndex().row();
  if(row == labelsListModel->rowCount() -1) return;
  radioData.swapLabel(row, row+1);
  labelsListModel->buildLabelsList();
  ui->lstLabels->selectionModel()->setCurrentIndex(labelsListModel->index(row + 1,0), QItemSelectionModel::ClearAndSelect);
}

void MdiChild::modelLabelsChanged(int index)
{
  setWindowModified(true);
  refresh();
  const QModelIndex idx = modelsListProxyModel->mapFromSource(modelsListModel->getIndexForModel(index));
  ui->modelsList->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect |
                                                                           QItemSelectionModel::Rows);
}

void MdiChild::labelsFault(QString msg)
{
  QMessageBox::warning(this, CPN_STR_TTL_WARNING, msg);
}

unsigned MdiChild::exportModels(const QVector<int> modelIndices)
{
  unsigned saves = 0;

  foreach(const int idx, modelIndices) {
    if (idx < 0 || idx >= (int)radioData.models.size())
      continue;

    const QString path(QDir::toNativeSeparators(g.profile[g.id()].sdPath() + "/TEMPLATES/" + QString(radioData.models[idx].name) + ".yml"));
    qDebug() << path;

    QString filename;

    do
    {
      filename = QFileDialog::getSaveFileName(this, tr("Export model"), path, YML_FILES_FILTER);

      if (filename.isEmpty())
        return false;

      if (QFileInfo(filename).suffix().toLower() != "yml")
        QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Invalid file extension!"));
    }
    while (QFileInfo(filename).suffix().toLower() != "yml");

    Storage storage(filename);

    if (!storage.writeModel(radioData, idx)) {
      QMessageBox::critical(this, CPN_STR_TTL_ERROR, storage.error());
      return false;
    }

    ++saves;
  }
 return saves;
}

bool MdiChild::exportModel(const int modelIndex)
{
  QVector<int> list = QVector<int>() << modelIndex;

  if (exportModels(list) == 1)
    return true;
  else
    return false;
}

void MdiChild::exportSelectedModels()
{
  exportModels(getSelectedModels());
}

void MdiChild::setCurrentModelModified()
{
  setModelModified(getCurrentModel());
}

void MdiChild::setModelModified(const int modelIndex, bool cascade)
{
  if (modelIndex >= 0 && modelIndex < (int)radioData.models.size()) {
    radioData.models[modelIndex].modelUpdated = true;
    if (cascade)
      setModified();
  }
}
