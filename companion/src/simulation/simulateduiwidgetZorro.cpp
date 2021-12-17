#include "simulateduiwidget.h"
#include "ui_simulateduiwidgetZorro.h"

// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

SimulatedUIWidgetZorro::SimulatedUIWidgetZorro(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetZorro)
{
  RadioUiAction * act;

  ui->setupUi(this);

  // add actions in order of appearance on the help menu

  act = new RadioUiAction(5, QList<int>() << Qt::Key_Up, SIMU_STR_HLP_KEY_UP, SIMU_STR_HLP_ACT_MDL);
  addRadioWidget(ui->rightbuttons->addArea(QRect(65, 35, 90, 45), "Zorro/right-mdl.png", act));

  m_mouseMidClickAction = new RadioUiAction(1, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN);
  addRadioWidget(ui->rightbuttons->addArea(QRect(7, 84, 110, 90), "Zorro/right-ent.png", m_mouseMidClickAction));

  act = new RadioUiAction(4, QList<int>() << Qt::Key_Left, SIMU_STR_HLP_KEY_LFT, SIMU_STR_HLP_ACT_SYS);
  addRadioWidget(ui->leftbuttons->addArea(QRect(52, 39, 70, 50), "Zorro/left-sys.png", act));

  act = new RadioUiAction(6, QList<int>() << Qt::Key_Right, SIMU_STR_HLP_KEY_RGT, SIMU_STR_HLP_ACT_TELE);
  addRadioWidget(ui->rightbuttons->addArea(QRect(75, 180, 80, 35), "Zorro/right-tele.png", act));

  act = new RadioUiAction(3, QList<int>() << Qt::Key_PageDown, SIMU_STR_HLP_KEY_PGDN, SIMU_STR_HLP_ACT_PGDN);
  addRadioWidget(ui->leftbuttons->addArea(QRect(70, 145, 80, 35), "Zorro/left-pagedn.png", act));

  act = new RadioUiAction(2, QList<int>() << Qt::Key_PageUp, SIMU_STR_HLP_KEY_PGUP, SIMU_STR_HLP_ACT_PGUP);
  addRadioWidget(ui->leftbuttons->addArea(QRect(60, 185, 80, 35), "Zorro/left-pageup.png", act));

  act = new RadioUiAction(0, QList<int>() << Qt::Key_Down << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->leftbuttons->addArea(QRect(85, 105, 80, 35), "Zorro/left-rtn.png", act));

  m_scrollUpAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Minus << Qt::Key_Equal << Qt::Key_Left, SIMU_STR_HLP_KEYS_GO_LFT, SIMU_STR_HLP_ACT_ROT_LFT);
  m_scrollDnAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Plus << Qt::Key_Right, SIMU_STR_HLP_KEYS_GO_RGT, SIMU_STR_HLP_ACT_ROT_RGT);
  connectScrollActions();

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

SimulatedUIWidgetZorro::~SimulatedUIWidgetZorro()
{
  delete ui;
}
