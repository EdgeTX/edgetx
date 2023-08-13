#include "simulateduiwidget.h"
#include "ui_simulateduiwidgetX9LITE.h"

// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

SimulatedUIWidgetX9LITE::SimulatedUIWidgetX9LITE(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetX9LITE)
{
  RadioUiAction * act;

  ui->setupUi(this);

  // add actions in order of appearance on the help menu

  act = new RadioUiAction(0, QList<int>() << Qt::Key_PageUp << Qt::Key_Up, SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_MENU_ICN);
  addRadioWidget(ui->leftbuttons->addArea(QRect(80, 20, 40, 35), "X9LITE/left_menu.png", act));

  act = new RadioUiAction(3, QList<int>() << Qt::Key_PageDown << Qt::Key_Down, SIMU_STR_HLP_KEYS_GO_DN, SIMU_STR_HLP_ACT_PAGE);
  addRadioWidget(ui->leftbuttons->addArea(QRect(80, 65, 40, 35), "X9LITE/left_page.png", act));

  act = new RadioUiAction(1, QList<int>() << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->leftbuttons->addArea(QRect(80, 130, 40, 35), "X9LITE/left_exit.png", act));

  m_scrollUpAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Minus << Qt::Key_Equal << Qt::Key_Left, SIMU_STR_HLP_KEYS_GO_LFT, SIMU_STR_HLP_ACT_ROT_LFT);
  m_scrollDnAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Plus << Qt::Key_Right, SIMU_STR_HLP_KEYS_GO_RGT, SIMU_STR_HLP_ACT_ROT_RGT);
  connectScrollActions();

  m_mouseMidClickAction = new RadioUiAction(2, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN);
  addRadioWidget(ui->rightbuttons->addArea(polyArc(88, 92, 33), "X9LITE/right_ent.png", m_mouseMidClickAction));

  addRadioWidget(ui->leftbuttons->addArea(QRect(30, 154, 30, 30), "X9LITE/left_scrnshot.png", m_screenshotAction));

  m_backlightColors << QColor(215, 243, 255);  // X9LITE Blue
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

SimulatedUIWidgetX9LITE::~SimulatedUIWidgetX9LITE()
{
  delete ui;
}

