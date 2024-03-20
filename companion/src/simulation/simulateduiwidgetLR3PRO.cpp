#include "simulateduiwidget.h"
#include "ui_simulateduiwidgetLR3PRO.h"

// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

SimulatedUIWidgetLR3PRO::SimulatedUIWidgetLR3PRO(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetLR3PRO)
{
  RadioUiAction * act;

  ui->setupUi(this);

  act = new RadioUiAction(6, QList<int>() << Qt::Key_Up << Qt::Key_PageUp, SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_UP);
  addRadioWidget(ui->rightbuttons->addArea(QRect(30, 56, 30, 20), "LR3PRO/right_up.png", act));

  act = new RadioUiAction(7, QList<int>() << Qt::Key_Down << Qt::Key_PageDown, SIMU_STR_HLP_KEYS_GO_DN, SIMU_STR_HLP_ACT_DN);
  addRadioWidget(ui->rightbuttons->addArea(QRect(30, 120, 30, 20), "LR3PRO/right_down.png", act));

  act = new RadioUiAction(9, QList<int>() << Qt::Key_Right << Qt::Key_Plus, SIMU_STR_HLP_KEY_RGT % "|" % SIMU_STR_HLP_KEY_PLS, SIMU_STR_HLP_ACT_PLS);
  addRadioWidget(ui->rightbuttons->addArea(QRect(70, 84, 20, 30), "LR3PRO/right_plus.png", act));

  act = new RadioUiAction(8, QList<int>() << Qt::Key_Left << Qt::Key_Minus, SIMU_STR_HLP_KEY_LFT % "|" % SIMU_STR_HLP_KEY_MIN, SIMU_STR_HLP_ACT_MIN);
  addRadioWidget(ui->rightbuttons->addArea(QRect(5, 84, 20, 30), "LR3PRO/right_minus.png", act));

  m_mouseMidClickAction = new RadioUiAction(2, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ENT);
  addRadioWidget(ui->rightbuttons->addArea(QRect(30, 83, 30, 30), "LR3PRO/right_enter.png", m_mouseMidClickAction));

  act = new RadioUiAction(1, QList<int>() << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->leftbuttons->addArea(QRect(58, 98, 30, 30), "LR3PRO/left_escape.png", act));

  //addRadioWidget(ui->leftbuttons->addArea(QRect(10, 65, 70, 50), "LR3PRO/left_scrnshot.png", m_screenshotAction));

  m_backlightColors << QColor(215, 243, 255);  // X7 Blue
  m_backlightColors << QColor(166,247,159);
  m_backlightColors << QColor(247,159,166);
  m_backlightColors << QColor(255,195,151);
  m_backlightColors << QColor(247,242,159);

  ui->lcd->setBgDefaultColor(QColor(0, 0, 0));
  ui->lcd->setFgDefaultColor(QColor(255, 255, 255));

  setLcd(ui->lcd);

  QString css = "#radioUiWidget {"
                "background-color: qlineargradient(spread:reflect, x1:0, y1:0, x2:0, y2:1,"
                "stop:0 rgba(255, 255, 255, 255),"
                "stop:0.757062 rgba(241, 238, 238, 255),"
                "stop:1 rgba(247, 245, 245, 255));"
                "}";

  QTimer * tim = new QTimer(this);
  tim->setSingleShot(true);
  connect(tim, &QTimer::timeout, [this, css]() {
      emit customStyleRequest(css);
  });
  tim->start(100);
}

SimulatedUIWidgetLR3PRO::~SimulatedUIWidgetLR3PRO()
{
  delete ui;
}
