/*
	ImageViewer
	https://github.com/mpaperno/maxLibQt

	COPYRIGHT: (c)2019 Maxim Paperno; All Rights Reserved.
	Contact: http://www.WorldDesign.com/contact

	LICENSE:

	Commercial License Usage
	Licensees holding valid commercial licenses may use this file in
	accordance with the terms contained in a written agreement between
	you and the copyright holder.

	GNU General Public License Usage
	Alternatively, this file may be used under the terms of the GNU
	General Public License as published by the Free Software Foundation,
	either version 3 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	A copy of the GNU General Public License is available at <http://www.gnu.org/licenses/>.
*/

#include "ImageViewer.h"
#include "GraphicsImageView.h"
#include "ImageGrid.h"
#include <OverlayStackLayout>

#include <QApplication>
#include <QAction>
#include <QActionGroup>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QMetaEnum>
#include <QtMath>
#include <QScreen>
#include <QTimer>
#include <QToolButton>

QT_USE_NAMESPACE

ImageViewer::ImageViewer(const QString &appCssFile, const QString &imagePath, QWidget *p) :
  QWidget(p),
  appCssFile(appCssFile),
  msgBox(new Infobox(this))
{
	// Use a stack layout as top-level. This will have two pages with their own toolbars,
	// and a floating menu button for general program options.
	stackLo = new OverlayStackLayout(this);
//! [1]
	// We will use the stack layout for switching pages as well as floating a menu button and info text overlay.
	stackLo->setStackingMode(OverlayStackLayout::StackOne);
	// Keep main floating menu button in the SE corner away from bottom/right edge of window.
	stackLo->setContentsMargins(0,0,15,15);
//! [1]

	// set CSS before creating widgets
	loadStylesheet();
	// First page contains the image list view and a toolbar which is only visible on this page.
	setupListViewPage();
	// Second page is the full-sized image view, and its own toolbar which is only visible on this page.
	setupImageViewPage();
	// create the floating options menu toolbar/button
	setupOptionsMenu();
	// setup the message box overlay
	msgBox->setAttribute(Qt::WA_AlwaysStackOnTop);
	stackLo->addWidget(msgBox, Qt::AlignHCenter | Qt::AlignBottom, QPoint(0, -35));

	// Very convenient when working with app-wide CSS to reload the style w/out re-starting whole app
	// (though clearly only useful when css is not embedded in resource file).
	QAction *reloadCss = new QAction("Reload stylesheet.", this);
	reloadCss->setShortcut(QKeySequence(Qt::ALT | Qt::Key_R));
	addAction(reloadCss);
	connect(reloadCss, &QAction::triggered, this, &ImageViewer::loadStylesheet);

	setPaletteColors();
	stackLo->setCurrentIndex(0);
	resize(QSize(1200, 800).boundedTo(QGuiApplication::primaryScreen()->availableSize()));
	show();

	if (imagePath.isEmpty()) {
		selectFolder();
		return;
	}

	const QFileInfo fi(imagePath);
	if (fi.isFile()) {
		setImagesPath(fi.path());
		loadImage(fi.filePath());
	}
	else {
		setImagesPath(imagePath);
	}
}

void ImageViewer::setupOptionsMenu()
{
//! [3]
	// main app "floating" menu toolbar/button with common options
	Toolbar *mainToolbar = new Toolbar(this);
//! [3]
	mainToolbar->setTheme(2);

	QMenu *mainMenu = mainToolbar->makeMenuButton(mainToolbar->addButton("⋮", "Toggle the main menu for general program options.", QKeySequence(Qt::CTRL | Qt::Key_Period)));
	connect(mainMenu, &QMenu::aboutToShow, this, [mainMenu]() { mainMenu->menuAction()->setText("⋯"); });
	connect(mainMenu, &QMenu::aboutToHide, this, [mainMenu]() { mainMenu->menuAction()->setText("⋮"); });

	// background color and theme toggles
	mainMenu->addAction("Use light background.", this, &ImageViewer::setPaletteColors)->setCheckable(true);
	mainMenu->addAction("Alternate toolbar theme.", this, &ImageViewer::setTheme)->setCheckable(true);

	// Sub menu for image info overlay duration
	QMenu *imgInfoMenu = new QMenu("Image information", mainToolbar);
	mainMenu->addMenu(imgInfoMenu);
	QActionGroup *imgInfoGrp = new QActionGroup(imgInfoMenu);
	for (int i = -1; i <= 20001; i += (i < 0 ? 1 : 2000))
		mainToolbar->addGroupAction(i < 0 ? tr("Do not show") : !i ? tr("Show always") : tr("Show for %1 sec.").arg(i / 1000), imgInfoGrp, i, (i == imgInfoTimeout));
	imgInfoMenu->addActions(imgInfoGrp->actions());
	connect(imgInfoMenu, &QMenu::triggered, this, [this](QAction *act) {
		imgInfoTimeout = act->data().toInt();
		if (imgInfoTimeout < 0)
			msgBox->showMessage();
		else if (listView->currentFileIndex() > -1)
			showImageInfo(listView->currentImage());
	});

	// Sub menu for choosing toolbar position (window edge)
	QMenu *tbEdgeMenu = new QMenu("Toolbar position", mainToolbar);
	mainMenu->addMenu(tbEdgeMenu);
	QActionGroup *edgeGrp = new QActionGroup(tbEdgeMenu);
	for (uint i = Qt::TopEdge; i <= Qt::BottomEdge; i = i << 1)
		mainToolbar->addGroupAction(QString(QMetaEnum::fromType<Qt::Edge>().valueToKey(int(i))).remove(QLatin1String("Edge")), edgeGrp, i, i == Qt::TopEdge);
	tbEdgeMenu->addActions(edgeGrp->actions());
	connect(tbEdgeMenu, &QMenu::triggered, this, [this](QAction *act) {
		emit toolbarEdgeChanged(Qt::Edge(act->data().toUInt()));
		updateTheme();
	});

//! [4]
	// Keep toolbar on top of all other widgets, regardless of "currentIndex()" page.
	mainToolbar->setAttribute(Qt::WA_AlwaysStackOnTop);
	// Add menu button to the overlay layout in the bottom right corner
	stackLo->addWidget(mainToolbar, Qt::AlignRight | Qt::AlignBottom);
//! [4]
	mainToolbar->show();
}

// Create a new page widget container with a OverlayStackLayout holding \a main widget and a Toolbar.
Toolbar *ImageViewer::createPage(QWidget *main)
{
//! [2]
	// Create a page widget which will be shown full-size underneath the main toolbar.
	QWidget *w = new QWidget(this);
	w->setAttribute(Qt::WA_LayoutOnEntireRect);  // ignore main stack layout margins
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);  // stretch to fit
	stackLo->addWidget(w);  // add to main stack view
//! [2]
	Toolbar *toolbar = new Toolbar(w);  // floating toolbar
	toolbar->setObjectName(main->objectName() % "_toolbar");  // for CSS target
	// overlay layout for the container, toolbar will "float" on top of main widget
	OverlayStackLayout *lo = new OverlayStackLayout(w);
	lo->setContentsMargins(0,0,0,0);
	lo->addWidget(main);
	lo->addWidget(toolbar, Qt::AlignTop | Qt::AlignHCenter);
	lo->setCurrentIndex(1);  // stack toolbar on top

	connect(this, &ImageViewer::themeChanged, toolbar, &Toolbar::setTheme);
	connect(this, &ImageViewer::toolbarEdgeChanged, toolbar, &Toolbar::setEdge);
	connect(toolbar, &Toolbar::alignmentChanged, lo, &OverlayStackLayout::setSenderAlignment);
	return toolbar;
}

void ImageViewer::setupListViewPage()
{
	listView = new ImageGrid(this);  // images list
	// show full image page when item is selected from list view
	connect(listView, &ImageGrid::imageSelected, this, &ImageViewer::loadImage);
	connect(listView, &ImageGrid::currentImageChanged, this, &ImageViewer::showImageInfo);
	connect(listView, &ImageGrid::countChanged, this, &ImageViewer::showFolderInfo);

	Toolbar *toolbar = createPage(listView);
	// left expander to keep buttons centered when horizontal
	toolbar->addHSpacer();

	// Folder selector
	connect(toolbar->addButton("Select", tr("Select image folder (%1)"), QKeySequence::Open)->defaultAction(), &QAction::triggered, this, &ImageViewer::selectFolder);

	toolbar->addSeparator();

	// Sorting controls
	QMenu *sortMenu = toolbar->addMenuButton("Name", "Select sorting method.");
	QActionGroup *sortGrp = new QActionGroup(sortMenu);
	sortMenu->addAction(toolbar->addGroupAction("Name", sortGrp, QDir::Name, true));
	sortMenu->addAction(toolbar->addGroupAction("Date", sortGrp, QDir::Time));
	sortMenu->addAction(toolbar->addGroupAction("Size", sortGrp, QDir::Size));
	sortMenu->addAction(toolbar->addGroupAction("Type", sortGrp, QDir::Type));
	QAction *sortDir = toolbar->addButton("⬇", "Sort direction")->defaultAction();
	sortDir->setCheckable(true);

	auto setSort = [this, sortGrp, sortMenu, sortDir]() {
		QDir::SortFlags f = QDir::SortFlag(sortGrp->checkedAction()->data().toInt());
		if (sortDir->isChecked())
			f |= QDir::Reversed;
		sortMenu->setTitle(sortGrp->checkedAction()->text());
		sortDir->setText(sortDir->isChecked() ? "⬆" : "⬇");
		listView->sortBy(f);
	};
	connect(sortGrp, &QActionGroup::triggered, listView, [setSort](QAction *) { setSort(); });
	connect(sortDir, &QAction::toggled, listView, [setSort](bool) { setSort(); });

	toolbar->addSeparator();

	// Layout options buttons
	const QVector<QToolButton *> viewOpts = toolbar->addButtonSet({"◫", "⇔"}, {"Select number of columns.", "Select preview image size."});

	QMenu *colsMenu = toolbar->makeMenuButton(viewOpts.at(0), "No. of Columns");
	QActionGroup *colsGrp = new QActionGroup(toolbar);
	connect(colsGrp, &QActionGroup::triggered, listView, [this](QAction *act) {
		listView->setColumns(act->data().toUInt());
	});
	for (int i=0; i < 10; ++i)
		colsMenu->addAction(toolbar->addGroupAction((i ? QString::number(i) : "Auto Columns"), colsGrp, i, !i));
	QAction *colsSelAct = viewOpts.at(0)->defaultAction();

	// Image preview size menu
	QMenu *sizeMenu = toolbar->makeMenuButton(viewOpts.at(1), "Image Size");
	QActionGroup *sizeGrp = new QActionGroup(toolbar);
	connect(sizeGrp, &QActionGroup::triggered, listView, [this, colsSelAct](QAction *act) {
		const QSize sz(act->data().toSize());
		listView->setImageSize(sz);
		colsSelAct->setEnabled(sz.isEmpty());
	});
	for (int i=0; i < 2561; i = i + (i < 512 ? 64 : i < 1280 ? 128 : 256))
		sizeMenu->addAction(toolbar->addGroupAction((i ? QString::number(i) : "Auto Size"), sizeGrp, (i ? QSize(i, qCeil(i * 0.75)) : QSize()), !i));

	// right expander to keep buttons center/top aligned
	toolbar->addSpacer();
}

void ImageViewer::setupImageViewPage()
{
	imageView = new GraphicsImageView(this);
	Toolbar *toolbar = createPage(imageView);
	// left expander to keep buttons centered when horizontal
	toolbar->addHSpacer();

	// page stack "pop" action
	connect(toolbar->addButton("⟸", "Back to list view (%1)", QKeySequence(Qt::Key_Backspace))->defaultAction(), &QAction::triggered, [this]() {
		stackLo->setCurrentIndex(0);
	});
	// previous/next image navigation buttons
	const QVector<QToolButton *> navBtns = toolbar->addButtonSet(
		{"⤆", "⤇"}, {"Previous image (%1)", "Next image (%1)"},
		{QKeySequence(Qt::Key_Left), QKeySequence(Qt::Key_Right)}
	);
	connect(navBtns.at(0)->defaultAction(), &QAction::triggered, listView, &ImageGrid::prevImage);
	connect(navBtns.at(1)->defaultAction(), &QAction::triggered, listView, &ImageGrid::nextImage);

	toolbar->addSeparator();

	// Scaling mode menu
	QMenu *sclMenu = toolbar->addMenuButton("Mode", "Select image scaling mode.");
	QActionGroup *sclGrp = new QActionGroup(toolbar);
	connect(sclGrp, &QActionGroup::triggered, imageView, [this](QAction *act) {
		imageView->setImageScalingMode(Qt::AspectRatioMode(act->data().toInt()));
	});
	const Qt::AspectRatioMode curMode = imageView->imageScalingMode();
	sclMenu->addAction(toolbar->addGroupAction("Keep Aspect Ratio", sclGrp, Qt::KeepAspectRatio, (curMode == Qt::KeepAspectRatio)));
	sclMenu->addAction(toolbar->addGroupAction("Ignore Aspect Ratio", sclGrp, Qt::IgnoreAspectRatio, (curMode == Qt::IgnoreAspectRatio)));
	sclMenu->addAction(toolbar->addGroupAction("Keep Ratio by Expanding", sclGrp, Qt::KeepAspectRatioByExpanding, (curMode == Qt::KeepAspectRatioByExpanding)));

	// rotate buttons
	const QVector<QToolButton *> rotBtns = toolbar->addButtonSet(
		{"⤿", "⤾"}, {"Rotate counter-clockwise (%1)", "Rotate clockwise (%1)"},
		{QKeySequence(Qt::ALT | Qt::Key_Left), QKeySequence(Qt::ALT | Qt::Key_Right)}
	);
	connect(rotBtns.at(0)->defaultAction(), &QAction::triggered, imageView, &GraphicsImageView::rotateCCw);
	connect(rotBtns.at(1)->defaultAction(), &QAction::triggered, imageView, &GraphicsImageView::rotateCw);

	// zoom buttons
	const QVector<QToolButton *> zoomBtns = toolbar->addButtonSet(
		{"-", "=", "+"},
		{"Zoom out (%1)", "Reset Zoom and Position (%1)", "Zoom in (%1)"},
		{QKeySequence(Qt::CTRL | Qt::Key_Minus), QKeySequence(Qt::CTRL | Qt::Key_0), QKeySequence(Qt::CTRL | Qt::Key_Equal)}
	);
	zoomBtns.at(0)->setAutoRepeat(true);
	zoomBtns.at(2)->setAutoRepeat(true);
	connect(zoomBtns.at(0)->defaultAction(), &QAction::triggered, imageView, &GraphicsImageView::zoomOut);
	connect(zoomBtns.at(1)->defaultAction(), &QAction::triggered, imageView, &GraphicsImageView::zoomReset);
	connect(zoomBtns.at(2)->defaultAction(), &QAction::triggered, imageView, &GraphicsImageView::zoomIn);

	// right expander to keep buttons center/top aligned
	toolbar->addSpacer();
}

void ImageViewer::setImagesPath(const QString &path) const
{
	if (!path.isEmpty())
		listView->setCurrentPath(path);
}

void ImageViewer::loadImage(const QString &img) const
{
	if (img.isEmpty())
		return;
	if (!QFile::exists(img)) {
		msgBox->showMessage(tr("File not found: %1").arg(img), 8000);
		return;
	}
	imageView->setImageFile(img);
	stackLo->setCurrentIndex(1);
}

void ImageViewer::showFolderInfo(int count) const
{
	if (count)
		msgBox->showMessage(tr("Loaded %n image(s)", "", count));
	else
		msgBox->showMessage(tr("This folder contains no images"), 0);
}

void ImageViewer::showImageInfo(const QString &img) const
{
	if (imgInfoTimeout < 0 || img.isEmpty())
		return;
	const QFileInfo fi(img);
	if (!fi.exists())
		return;
	const QString info(
		QStringLiteral(
	    "<div style='font-size: 17px; font-weight: bold; white-space: nowrap;'>%1</div>"
	    "<table cellspacing=0 cellpadding=2 style='font-size: 14px; white-space: nowrap;'>"
	    "<tr>"
				"<td style='font-size: 15px; padding-right: 6px;'>(%6/%7)</td>"
				"<td>Type: %5 &nbsp; Size: %4 KB</td>"
	    "</tr>"
	    "<tr><td>Created: </td><td>%2</td></tr>"
	    "<tr><td>Modified: </td><td>%3</td></tr>"
	    "</table>"
		)
		.arg(fi.baseName(),
	       fi.created().toString(Qt::TextDate),
				 fi.lastModified().toString(Qt::TextDate),
	       QString::number(fi.size() / 1024), fi.suffix())
		.arg(listView->currentFileIndex() + 1)
		.arg(listView->count())
	);
	msgBox->showMessage(info, imgInfoTimeout);
}

void ImageViewer::selectFolder() const
{
	setImagesPath(QFileDialog::getExistingDirectory(const_cast<ImageViewer*>(this), "Select images folder", listView->currentPath(), QFileDialog::ReadOnly));
}

void ImageViewer::setAppCssFile(const QString &filename)
{
	if (appCssFile != filename) {
		appCssFile = filename;
		loadStylesheet();
	}
}

void ImageViewer::setTheme(bool trans)
{
	emit themeChanged(int(trans));
	updateTheme();
}

void ImageViewer::updateTheme() const
{
	// reload css for property selectors to take effect
	qApp->setStyleSheet(qApp->styleSheet());
	// reloading css can cause palette changes
	updateViewPalette();
}

void ImageViewer::loadStylesheet() const
{
	if (appCssFile.isEmpty())
		return;
	QFile cssFile(appCssFile);
	if (!cssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "Could not load style sheet file from" << appCssFile;
		return;
	}
	const QByteArray appStyle = cssFile.readAll();
	if (!appStyle.isEmpty())
		qApp->setStyleSheet(QString(appStyle));
	cssFile.close();
}

void ImageViewer::setPaletteColors(bool light)
{
	QPalette pal(QApplication::palette());
	pal.setBrush(QPalette::Highlight, QColor(QStringLiteral("#4985E5")));
	pal.setBrush(QPalette::Shadow, QColor(light ? QStringLiteral("#E3EAEA") : QStringLiteral("#001616")));
	QApplication::setPalette(pal);
	updateViewPalette();
}

void ImageViewer::updateViewPalette() const
{
	const QPalette pal(QApplication::palette());
	listView->setPalette(pal);
	listView->viewport()->setPalette(pal);
	imageView->setBackgroundBrush(pal.brush(QPalette::Shadow));
}


//
// Infobox
//

#include <QGraphicsEffect>
#include <QPropertyAnimation>
#include <QTimerEvent>

Infobox::Infobox(QWidget *p) :
  QLabel(p),
  m_opacity(new QGraphicsOpacityEffect(this)),
  m_fade(new QPropertyAnimation(m_opacity, QByteArrayLiteral("opacity"), this))
{
	setAttribute(Qt::WA_TransparentForMouseEvents);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	setFrameStyle(NoFrame);
	setTextFormat(Qt::RichText);
	setWordWrap(true);
	setGraphicsEffect(m_opacity);
	connect(m_opacity, &QGraphicsOpacityEffect::opacityChanged, this, [this](qreal op) {
		setHidden(qFuzzyIsNull(op));
		if (isHidden())
			setText(QString());
	});
	m_opacity->setOpacity(0.0);
}

qreal Infobox::opacity() const { return m_opacity->opacity(); }
void Infobox::setOpacity(qreal opacity) { m_opacity->setOpacity(opacity); }

void Infobox::showMessage(const QString &msg, int timeout)
{
	m_msgTim.stop();
	if (msg.isEmpty()) {
		fadeOut(timeout && !text().isEmpty() ? -1 : 0);
		return;
	}
	setText(msg);
	fadeIn(400);
	if (timeout > 0)
		m_msgTim.start(timeout, this);
}

void Infobox::fade(bool out, int duration)
{
	if (m_fade->state() != QAbstractAnimation::Stopped)
		m_fade->stop();

	if (!duration) {
		setOpacity(out ? 0.0 : 1.0);
		return;
	}
	if (duration < 0)
		duration = 2000;
	m_fade->setDuration(duration);
	m_fade->setStartValue(out ? 1.0 : 0.01);
	m_fade->setEndValue(out ? 0.0 : 1.0);
	show();
	m_fade->start();
}

void Infobox::timerEvent(QTimerEvent *e)
{
	if (e->timerId() == m_msgTim.timerId()) {
		m_msgTim.stop();
		fadeOut();
	}
}


//
// Toolbar
//

Toolbar::Toolbar(QWidget *p) :
  QFrame(p)
{
	m_lo = new QBoxLayout(QBoxLayout::LeftToRight, this);
	m_lo->setContentsMargins(0,0,0,0);
	setBackgroundRole(QPalette::Button);
	setFrameStyle(NoFrame);
	setFocusPolicy(Qt::NoFocus);
}

QToolButton *Toolbar::addButton(QAction *act, QLayout *lo, Qt::Alignment a, const QKeySequence &sc)
{
	addAction(act);
	if (!sc.isEmpty())
		act->setShortcut(sc);
	QToolButton *tb = new QToolButton(this);
	tb->setDefaultAction(act);
	tb->setAutoRaise(true);
	if (!lo)
		lo = m_lo;
	lo->addWidget(tb);
	lo->setAlignment(tb, a);
	return tb;
}

QToolButton *Toolbar::addButton(const QString &ttl, const QString &tip, QLayout *lo, Qt::Alignment a, const QKeySequence &sc)
{
	QAction *act = new QAction(ttl, this);
	if (!sc.isEmpty() && tip.contains(QLatin1String("%1")))
		act->setToolTip(tip.arg(sc.toString(QKeySequence::NativeText)));
	else
		act->setToolTip(tip);
	return addButton(act, lo, a, sc);
}

QVector<QToolButton *> Toolbar::addButtonSet(const QStringList &txts, const QStringList &tts, const QVector<QKeySequence> &scuts)
{
	QVector<QToolButton *> ret;
	QHBoxLayout *lo = new QHBoxLayout;
	lo->setSpacing(0);
	lo->setSizeConstraint(QLayout::SetFixedSize);
	for (int i=0, last = txts.count() - 1; i <= last; ++i) {
		QToolButton *btn = addButton(txts.at(i), tts.value(i), lo, (!i ? Qt::AlignRight : i == last ? Qt::AlignLeft : Qt::AlignCenter), scuts.value(i));
		btn->setProperty("align", !i ? "left" : i == last ? "right" : "middle");
		btn->setStyleSheet(QStringLiteral("QToolButton { max-width: %1%; }").arg(50 / (last + 1)));
		ret.append(btn);
	}
	m_lo->addItem(lo);
	m_lo->setAlignment(lo, Qt::AlignCenter);
	return ret;
}

QAction *Toolbar::addGroupAction(const QString &ttl, QActionGroup *grp, const QVariant &data, bool chkd)
{
	QAction *act = new QAction(ttl, this);
	act->setData(data);
	act->setCheckable(true);
	act->setChecked(chkd);
	addAction(act);
	grp->addAction(act);
	return act;
}

QWidget *Toolbar::addSeparator() const
{
	QWidget *sp = new QWidget(const_cast<Toolbar*>(this));
	sp->setObjectName(QStringLiteral("tb_separator"));
	sp->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	m_lo->addWidget(sp);
	return sp;
}

QSpacerItem *Toolbar::addSpacer(int w, int h, QSizePolicy::Policy hPol, QSizePolicy::Policy vPol) const
{
	QSpacerItem *si = new QSpacerItem(w, h, hPol, vPol);
	m_lo->addItem(si);
	return si;
}

QMenu *Toolbar::makeMenuButton(QToolButton *tb, const QString &name) const
{
	QMenu *menu = new QMenu(name, const_cast<Toolbar*>(this));
	tb->defaultAction()->setMenu(menu);
	tb->setPopupMode(QToolButton::InstantPopup);
	return menu;
}

QMenu *Toolbar::addMenuButton(const QString &ttl, const QString &tip, QLayout *lo, Qt::Alignment a)
{
	return makeMenuButton(addButton(ttl, tip, lo, a), ttl);
}

void Toolbar::setOrientation(Qt::Orientation o)
{
	if (m_o == o)
		return;
	setEdge((o == Qt::Horizontal ? (m_edge == Qt::RightEdge ? Qt::BottomEdge : Qt::TopEdge) : (m_edge == Qt::BottomEdge ? Qt::RightEdge : Qt::LeftEdge)));
}

void Toolbar::setAlignment(int a)
{
	if (m_align != Qt::Alignment(a)) {
		m_align = Qt::Alignment(a);
		emit alignmentChanged(m_align);
	}
}

void Toolbar::setEdge(Qt::Edge edge)
{
	if (m_edge == edge)
		return;
	m_edge = edge;
	const Qt::Orientation o4e = ((edge == Qt::LeftEdge || edge == Qt::RightEdge) ? Qt::Vertical : Qt::Horizontal);
	if (o4e != m_o) {
		m_o = o4e;
		m_lo->setDirection(m_o == Qt::Vertical ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
		emit orientationChanged(o4e);
	}
}

void Toolbar::setTheme(int idx)
{
	if (idx == m_theme)
		return;
	m_theme = idx;
	emit themeChanged();
}

#include "moc_ImageViewer.cpp"
