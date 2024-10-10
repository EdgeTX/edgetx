#include "simulateduiwidget.h"
#include "ui_simulateduiwidgetJumperT12max.h"

// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

SimulatedUIWidgetJumperT12max::SimulatedUIWidgetJumperT12max(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetJumperT12max)
{
  RadioUiAction * act;

  ui->setupUi(this);

  act = new RadioUiAction(KEY_MENU, QList<int>() << Qt::Key_PageUp << Qt::Key_Up, SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_MENU_ICN);
  addRadioWidget(ui->leftbuttons->addArea(QRect(17, 45, 90, 30), "JumperT12max/menu.png", act));

  act = new RadioUiAction(KEY_PAGEDN, QList<int>() << Qt::Key_PageDown << Qt::Key_Down, SIMU_STR_HLP_KEYS_GO_DN, SIMU_STR_HLP_ACT_PAGE);
  addRadioWidget(ui->leftbuttons->addArea(QRect(17, 107, 90, 30), "JumperT12max/page.png", act));

  act = new RadioUiAction(KEY_EXIT, QList<int>() << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->leftbuttons->addArea(QRect(17, 180, 90, 30), "JumperT12max/exit.png", act));

  m_mouseMidClickAction = new RadioUiAction(KEY_ENTER, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN);
  addRadioWidget(ui->rightbuttons->addArea(QRect(130, 65, 70, 130), "JumperT12max/right-ent.png", m_mouseMidClickAction));

  //addRadioWidget(ui->leftbuttons->addArea(QRect(10, 65, 70, 50), "JumperT12max/left_scrnshot.png", m_screenshotAction));

  m_backlightColors << QColor(215, 243, 255);  // X7 Blue
  m_backlightColors << QColor(166,247,159);
  m_backlightColors << QColor(247,159,166);
  m_backlightColors << QColor(255,195,151);
  m_backlightColors << QColor(247,242,159);

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

SimulatedUIWidgetJumperT12max::~SimulatedUIWidgetJumperT12max()
{
  delete ui;
}
