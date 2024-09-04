#include "simulateduiwidget.h"
#include "ui_simulateduiwidgetJumperTPROS.h"
#include "eeprominterface.h"

// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

SimulatedUIWidgetJumperTPROS::SimulatedUIWidgetJumperTPROS(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetJumperTPROS)
{
  RadioUiAction * act;

  ui->setupUi(this);

  act = new RadioUiAction(KEY_MENU, QList<int>() << Qt::Key_PageUp << Qt::Key_Up, SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_MENU_ICN);
  addRadioWidget(ui->leftbuttons->addArea(QRect(85, 85, 70, 70), "JumperTPROS/menu.png", act));

  act = new RadioUiAction(KEY_PAGEDN, QList<int>() << Qt::Key_PageDown << Qt::Key_Down, SIMU_STR_HLP_KEYS_GO_DN, SIMU_STR_HLP_ACT_PAGE);
  addRadioWidget(ui->leftbuttons->addArea(QRect(53, 160, 70, 70), "JumperTPROS/page.png", act));

  act = new RadioUiAction(KEY_EXIT, QList<int>() << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->leftbuttons->addArea(QRect(20, 230, 70, 70), "JumperTPROS/exit.png", act));

  m_mouseMidClickAction = new RadioUiAction(KEY_ENTER, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN);
  addRadioWidget(ui->rightbuttons->addArea(QRect(10, 96, 200, 220), "JumperTPROS/right-ent.png", m_mouseMidClickAction));

  //addRadioWidget(ui->leftbuttons->addArea(QRect(10, 65, 70, 50), "JumperTPROS/left_scrnshot.png", m_screenshotAction));

  m_backlightColors << QColor(215, 243, 255);  // X7 Blue
  m_backlightColors << QColor(166,247,159);
  m_backlightColors << QColor(247,159,166);
  m_backlightColors << QColor(255,195,151);
  m_backlightColors << QColor(247,242,159);

  if (getCurrentBoard() == Board::BOARD_JUMPER_TPROV2) {
    ui->lcd->setBgDefaultColor(QColor(0, 0, 0));
    ui->lcd->setFgDefaultColor(QColor(255, 255, 255));
  }

  setLcd(ui->lcd);

  QString css = "#radioUiWidget {"
                "background-color: rgb(167, 167, 167);"
                "}";

  QTimer * tim = new QTimer(this);
  tim->setSingleShot(true);
  connect(tim, &QTimer::timeout, [this, css]() {
      emit customStyleRequest(css);
  });
  tim->start(100);
}

SimulatedUIWidgetJumperTPROS::~SimulatedUIWidgetJumperTPROS()
{
  delete ui;
}
