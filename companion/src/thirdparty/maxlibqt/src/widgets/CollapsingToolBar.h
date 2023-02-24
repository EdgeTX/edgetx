/*
	CollapsingToolBar
	https://github.com/mpaperno/maxLibQt

	COPYRIGHT: (c)2019 Maxim Paperno; All Right Reserved.
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

#ifndef COLLAPSINGTOOLBAR_H_
#define COLLAPSINGTOOLBAR_H_

#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QActionEvent>
#include <QStyle>

/*!
  \brief The CollapsingToolBar will change the \c QToolButton::toolButtonStyle of all added \c QToolButtons (via \c QAction or directly)
  depending on the total available width. It was originally written as an answer to a [StackOverflow question][1], and is not very sophisticated.
  Here is the usage example/test given in the SO question (find a suitable image file to use for an icon).

  \code
  class MainWindow : public QMainWindow
  {
      Q_OBJECT
    public:
      MainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
      {
        QToolBar* tb = new CollapsingToolBar(this);
        tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);

        QIcon icon = QIcon(QPixmap(info_xpm));  // need an image here
        for (int i=0; i < 6; ++i)
          tb->addAction(icon, QStringLiteral("Action %1").arg(i));

        addToolBar(tb);
        show();

        // Adding another action after show() may collapse all the actions if the new toolbar preferred width doesn't fit the window.
        // Only an issue if the toolbar size hint was what determined the window width to begin with.
        //tb->addAction(icon, QStringLiteral("Action After"));

        // Test setting button style after showing (comment out the one above)
        //tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

        // Test changing icon size after showing.
        //tb->setIconSize(QSize(48, 48));

        // Try this too...
        //tb->setMovable(false);
      }
  };

  int main(int argc, char *argv[])
  {
    QApplication app(argc, argv);
    //QApplication::setStyle("Fusion");
    //QApplication::setStyle("windows");
    MainWindow w;
    return app.exec();
  }
  \endcode

  [1]: https://stackoverflow.com/questions/57913277/changing-toolbuttonstyle-of-qtoolbuttons-dynamically-depending-upon-the-size-of/57918115#57918115
*/
class CollapsingToolBar : public QToolBar
{
    Q_OBJECT
  public:
		explicit CollapsingToolBar(QWidget *parent = nullptr) : CollapsingToolBar(QString(), parent) {}

		explicit CollapsingToolBar(const QString &title, QWidget *parent = nullptr) :
		  QToolBar(title, parent)
		{
      initSizes();
      // If icon sizes change we need to recalculate all the size hints, but we need to wait until the buttons have adjusted themselves, so we queue the update.
			connect(this, &QToolBar::iconSizeChanged, [this](const QSize &) {
        QMetaObject::invokeMethod(this, "recalcExpandedSize", Qt::QueuedConnection);
      });
      // The drag handle can mess up our sizing, update preferred size if it changes.
      connect(this, &QToolBar::movableChanged, [this](bool movable) {
        const int handleSz = style()->pixelMetric(QStyle::PM_ToolBarHandleExtent, nullptr, this);;
        m_expandedSize = (movable ? m_expandedSize + handleSz : m_expandedSize - handleSz);
        adjustForSize();
      });
		}

	protected:

		// Monitor action events to keep track of required size.
		void actionEvent(QActionEvent *e) override
		{
			QToolBar::actionEvent(e);

			int width = 0;
			switch (e->type())
			{
				case QEvent::ActionAdded:
					// Personal pet-peeve... optionally set buttons with menus to have instant popups instead of splits with the main button doing nothing.
					//if (QToolButton *tb = qobject_cast<QToolButton *>(widgetForAction(e->action())))
					//	tb->setPopupMode(QToolButton::InstantPopup);
					//Q_FALLTHROUGH;
				case QEvent::ActionChanged:
					width = widthForAction(e->action());
					if (width <= 0)
						return;

					if (e->type() == QEvent::ActionAdded || !m_actionWidths.contains(e->action()))
            m_expandedSize += width + m_spacing;
          else
            m_expandedSize = m_expandedSize - m_actionWidths.value(e->action()) + width;
          m_actionWidths.insert(e->action(), width);
					break;

				case QEvent::ActionRemoved:
					if (!m_actionWidths.contains(e->action()))
						return;
					width = m_actionWidths.value(e->action());
					m_expandedSize -= width + m_spacing;
					m_actionWidths.remove(e->action());
					break;

        default:
          return;
			}
			adjustForSize();
		}

		bool event(QEvent *e) override
    {
			// Watch for style change
      if (e->type() == QEvent::StyleChange)
        recalcExpandedSize();
      return QToolBar::event(e);
    }

		void resizeEvent(QResizeEvent *e) override
		{
			adjustForSize();
			QToolBar::resizeEvent(e);
		}

	private slots:
		// Here we do the actual switching of tool button style based on available width.
		void adjustForSize()
		{
			int availableWidth = contentsRect().width();
			if (!isVisible() || m_expandedSize <= 0 || availableWidth <= 0)
				return;

			switch (toolButtonStyle()) {
				case Qt::ToolButtonIconOnly:
					if (availableWidth > m_expandedSize)
						setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
					break;

				case Qt::ToolButtonTextBesideIcon:
					if (availableWidth <= m_expandedSize)
						setToolButtonStyle(Qt::ToolButtonIconOnly);
					break;

				default:
					break;
			}
		}

		// Loops over all previously-added actions and re-calculates new size (eg. after icon size change)
		void recalcExpandedSize()
		{
			if (m_actionWidths.isEmpty())
				return;
      initSizes();
			int width = 0;
			QHash<QAction *, int>::iterator it = m_actionWidths.begin();
			for ( ; it != m_actionWidths.end(); ++it) {
				width = widthForAction(it.key());
				if (width <= 0)
					continue;
				m_expandedSize += width + m_spacing;
				it.value() = width;
			}
			adjustForSize();
		}

	private:
    void initSizes()
    {
      // Preload some sizes based on style settings.
			// This is the spacing between items
      m_spacing = style()->pixelMetric(QStyle::PM_ToolBarItemSpacing, nullptr, this);
			// Size of a separator
      m_separatorWidth = style()->pixelMetric(QStyle::PM_ToolBarSeparatorExtent, nullptr, this);
			// The layout margins (we can't even get the private QToolBarLayout via layout() so we figure it out like it does)
      m_expandedSize = (style()->pixelMetric(QStyle::PM_ToolBarItemMargin, nullptr, this) + style()->pixelMetric(QStyle::PM_ToolBarFrameWidth, nullptr, this)) * 2;
			// And the size of the drag handle if we have one
      if (isMovable())
				m_expandedSize += style()->pixelMetric(QStyle::PM_ToolBarHandleExtent, nullptr, this);
    }

    int widthForAction(QAction *action) const
		{
			// Try to find how wide the action representation (widget/separator) is.
			if (action->isSeparator())
				return m_separatorWidth;

			if (QToolButton *tb = qobject_cast<QToolButton *>(widgetForAction(action))) {
				const Qt::ToolButtonStyle oldStyle = tb->toolButtonStyle();
				// force the widest size
				tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
				const int width = tb->sizeHint().width();
				tb->setToolButtonStyle(oldStyle);
				return width;
			}

			if (const QWidget *w = widgetForAction(action))
				return w->sizeHint().width();

			return 0;
		}

		int m_expandedSize = -1;   // The maximum size we need with all buttons expanded and allowing for margins/etc
		int m_spacing = 0;         // Layout spacing between items
		int m_separatorWidth = 0;  // Width of separators
		QHash<QAction *, int> m_actionWidths;  // Use this to track action additions/removals/changes
};

#endif  // COLLAPSINGTOOLBAR_H_
