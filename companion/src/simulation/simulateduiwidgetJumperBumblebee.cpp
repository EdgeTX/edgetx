#include "simulateduiwidget.h"
#include "ui_simulateduiwidgetJumperBumblebee.h"
#include "eeprominterface.h"

// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

SimulatedUIWidgetJumperBumblebee::SimulatedUIWidgetJumperBumblebee(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetJumperBumblebee)
{
  RadioUiAction * act;

  ui->setupUi(this);

  act = new RadioUiAction(KEY_MENU, QList<int>() << Qt::Key_PageUp << Qt::Key_Up, SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_MENU_ICN);
  addRadioWidget(ui->bottombuttons->addArea(QRect(55, 15, 50, 50), "JumperBumblebee/menu.png", act));

  act = new RadioUiAction(KEY_PAGEDN, QList<int>() << Qt::Key_PageDown << Qt::Key_Down, SIMU_STR_HLP_KEYS_GO_DN, SIMU_STR_HLP_ACT_PAGE);
  addRadioWidget(ui->bottombuttons->addArea(QRect(10, 25, 50, 50), "JumperBumblebee/page.png", act));

  act = new RadioUiAction(KEY_EXIT, QList<int>() << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->bottombuttons->addArea(QRect(25, 80, 50, 50), "JumperBumblebee/exit.png", act));

  m_mouseMidClickAction = new RadioUiAction(KEY_ENTER, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN);
  addRadioWidget(ui->bottombuttons->addArea(QRect(145, 20, 100, 100), "JumperBumblebee/enter.png", m_mouseMidClickAction));

  //addRadioWidget(ui->leftbuttons->addArea(QRect(10, 65, 70, 50), "JumperBumblebee/left_scrnshot.png", m_screenshotAction));

  m_backlightColors << QColor(215, 243, 255);  // X7 Blue
  m_backlightColors << QColor(166,247,159);
  m_backlightColors << QColor(247,159,166);
  m_backlightColors << QColor(255,195,151);
  m_backlightColors << QColor(247,242,159);

  if (getCurrentBoard() == Board::BOARD_JUMPER_BUMBLEBEE) {
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

SimulatedUIWidgetJumperBumblebee::~SimulatedUIWidgetJumperBumblebee()
{
  delete ui;
}
