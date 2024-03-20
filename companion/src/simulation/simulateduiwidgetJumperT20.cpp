#include "simulateduiwidget.h"
#include "ui_simulateduiwidgetJumperT20.h"

// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

SimulatedUIWidgetJumperT20::SimulatedUIWidgetJumperT20(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetJumperT20)
{
  RadioUiAction * act;

  ui->setupUi(this);

  act = new RadioUiAction(0, QList<int>() << Qt::Key_PageUp << Qt::Key_Up, SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_MENU_ICN);
  addRadioWidget(ui->leftbuttons->addArea(QRect(0, 30, 51, 28), "JumperT20/left.png", act));

  act = new RadioUiAction(3, QList<int>() << Qt::Key_PageDown << Qt::Key_Down, SIMU_STR_HLP_KEYS_GO_DN, SIMU_STR_HLP_ACT_PAGE);
  addRadioWidget(ui->leftbuttons->addArea(QRect(0, 70, 51, 28), "JumperT20/left.png", act));

  act = new RadioUiAction(1, QList<int>() << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 110, 51, 28), "JumperT20/left.png", act));

  m_mouseMidClickAction = new RadioUiAction(2, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN);
  addRadioWidget(ui->rightbuttons->addArea(QRect(130, 50, 50, 70), "JumperT20/right.png", m_mouseMidClickAction));

  m_backlightColors << QColor(215, 243, 255);  // X7 Blue
  m_backlightColors << QColor(166,247,159);
  m_backlightColors << QColor(247,159,166);
  m_backlightColors << QColor(255,195,151);
  m_backlightColors << QColor(247,242,159);

  ui->lcd->setBgDefaultColor(QColor(0, 0, 0));
  ui->lcd->setFgDefaultColor(QColor(255, 255, 255));

  setLcd(ui->lcd);

  QString css = "#radioUiWidget {"
                "background-color: rgb(200, 200, 200);"
                "}";

  QTimer * tim = new QTimer(this);
  tim->setSingleShot(true);
  connect(tim, &QTimer::timeout, [this, css]() {
      emit customStyleRequest(css);
  });
  tim->start(100);
}

SimulatedUIWidgetJumperT20::~SimulatedUIWidgetJumperT20()
{
  delete ui;
}
