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

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QBoxLayout>
#include <QFrame>
#include <QVariant>

class OverlayStackLayout;
class GraphicsImageView;
class ImageGrid;
QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QMenu;
class QSpacerItem;
class QToolButton;
QT_END_NAMESPACE

//! \ingroup examples_imageviewer
class Toolbar : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(Qt::Edge edge READ edge WRITE setEdge NOTIFY edgeChanged)
    Q_PROPERTY(int alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
    Q_PROPERTY(int theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
		Q_PROPERTY(int hSizePolicy READ hSizePolicy WRITE setHSizePolicy)
		Q_PROPERTY(int vSizePolicy READ vSizePolicy WRITE setVSizePolicy)

  public:
    explicit Toolbar(QWidget *p = nullptr);

    int theme() const { return m_theme; }
    int spacing() const { return m_lo->spacing(); }
    Qt::Edge edge() const { return m_edge; }
    Qt::Orientation orientation() const { return m_o; }
		int alignment() const { return m_align; }
		int hSizePolicy() const { return sizePolicy().horizontalPolicy(); }
		int vSizePolicy() const { return sizePolicy().verticalPolicy(); }

    QToolButton *addButton(QAction *act, QLayout *lo, Qt::Alignment a = Qt::AlignCenter, const QKeySequence &sc = QKeySequence());
    QToolButton *addButton(const QString &ttl, const QString &tip, QLayout *lo, Qt::Alignment a, const QKeySequence &sc = QKeySequence());
		QToolButton *addButton(const QString &ttl, const QString &tip, const QKeySequence &sc = QKeySequence()) { return addButton(ttl, tip, m_lo, Qt::AlignCenter, sc); }
		QMenu *addMenuButton(const QString &ttl, const QString &tip = QString(), QLayout *lo = nullptr, Qt::Alignment a = Qt::AlignCenter);
		QMenu *makeMenuButton(QToolButton *tb, const QString &name = QString()) const;

    QVector<QToolButton *> addButtonSet(const QStringList &txts, const QStringList &tts, const QVector<QKeySequence> &scuts = QVector<QKeySequence>());
    QAction *addGroupAction(const QString &ttl, QActionGroup *grp, const QVariant &data = QVariant(), bool chkd = false);
    QWidget *addSeparator() const;
    QSpacerItem *addSpacer(int w = 0, int h = 0, QSizePolicy::Policy hPol = QSizePolicy::Expanding, QSizePolicy::Policy vPol = QSizePolicy::Expanding) const;
    QSpacerItem *addHSpacer() { return addSpacer(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored); }
    QSpacerItem *addVSpacer() { return addSpacer(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding); }

  public slots:
    void setOrientation(Qt::Orientation o);
		void setAlignment(int a);
    void setVertical(bool on) { setOrientation(on ? Qt::Vertical : Qt::Horizontal); }
    void setHorizontal(bool on) { setVertical(!on); }
    void setEdge(Qt::Edge edge);
    void setTheme(int idx);
    void toggleTheme(bool on) { setTheme(int(on)); }
    void setSpacing(int spacing) { m_lo->setSpacing(spacing); }
		void setHSizePolicy(int pol) { QSizePolicy p = sizePolicy(); p.setHorizontalPolicy(QSizePolicy::Policy(pol)); setSizePolicy(p); }
		void setVSizePolicy(int pol) { QSizePolicy p = sizePolicy(); p.setVerticalPolicy(QSizePolicy::Policy(pol)); setSizePolicy(p); }

  signals:
    void orientationChanged(Qt::Orientation o) const;
    void edgeChanged(Qt::Edge edge) const;
    void themeChanged() const;
    void alignmentChanged(Qt::Alignment align) const;

  private:
    Qt::Orientation m_o = Qt::Horizontal;
    Qt::Edge m_edge = Qt::TopEdge;
		Qt::Alignment m_align = Qt::AlignLeft | Qt::AlignTop;
    int m_theme = 0;
    QBoxLayout *m_lo;

    Q_DISABLE_COPY(Toolbar)
};


#include <QLabel>
#include <QBasicTimer>

QT_BEGIN_NAMESPACE
class QGraphicsOpacityEffect;
class QPropertyAnimation;
class QTimerEvent;
QT_END_NAMESPACE

//! \ingroup examples_imageviewer
class Infobox : public QLabel
{
		Q_OBJECT
		Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)

	public:
		explicit Infobox(QWidget *p);
		qreal opacity() const;

	public slots:
		void showMessage(const QString &msg = QString(), int timeout = 5000);
		void setOpacity(qreal opacity);
		void fade(bool out = true, int duration = -1);
		void fadeIn() { fade(false, -1); }
		void fadeIn(int duration) { fade(false, duration); }
		void fadeOut() { fade(true, -1); }
		void fadeOut(int duration) { fade(true, duration); }

	signals:
		void opacityChanged(qreal) const;

	protected:
		void timerEvent(QTimerEvent *e) override;

	private:
		QGraphicsOpacityEffect *m_opacity;
		QPropertyAnimation *m_fade;
		QBasicTimer m_msgTim;
};


//! \ingroup examples_imageviewer
class ImageViewer : public QWidget
{
    Q_OBJECT
  public:
    explicit ImageViewer(const QString &appCssFile, const QString &imagePath = QString(), QWidget *p = nullptr);

  public slots:
    void setImagesPath(const QString &path) const;
		void loadImage(const QString &img) const;
    void setAppCssFile(const QString &filename);

	signals:
		void themeChanged(int theme) const;
		void toolbarEdgeChanged(Qt::Edge edge) const;

	private slots:
		void showFolderInfo(int count) const;
		void showImageInfo(const QString &img) const;
		void selectFolder() const;
		void setTheme(bool trans);
    void updateTheme() const;
    void loadStylesheet() const;
    void setPaletteColors(bool light = false);
    void updateViewPalette() const;

  private:
		void setupOptionsMenu();
		Toolbar *createPage(QWidget *main);
		void setupListViewPage();
		void setupImageViewPage();

		int imgInfoTimeout = 6000;
    QString appCssFile;
		Infobox *msgBox;
    ImageGrid *listView;
    GraphicsImageView *imageView;
    OverlayStackLayout *stackLo;

		Q_DISABLE_COPY(ImageViewer)
};

#endif // IMAGEVIEWER_H
