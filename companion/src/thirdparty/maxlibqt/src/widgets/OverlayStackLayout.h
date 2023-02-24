/*
	OverlayStackLayout
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

#ifndef OVERLAYSTACKLAYOUT_H
#define OVERLAYSTACKLAYOUT_H

#include <QStackedLayout>

/*!
	\brief The OverlayStackLayout re-implements a \c QStackedLayout with additional features
	to allow stacks with "floating" overlay widgets, such as toolbars, buttons, messages, etc.,
	while still allowing interaction with exposed areas of the widget(s) underneath.

	The functionality is similar to \c QStackedLayout::StackAll mode, but instead of forcing all
	child widgets to be the same size (as \c QStackedLayout does), this version respects the size
	hints of each widget. It also respects the widget alignment settings (like most other `QLayout`s),
	and adds a way to fine-tune the position with a `positionOffset` property which can be set on any widget.
	These are set with the standard \c QLayout::setAlignment() and custom \c OverlayStackLayout::setOffset()
	methods respectively. A few \c addWidget() and \c insertWidget() overloads are also provided to set
	these properties at insertion time.

	So for example one could have a "main" widget with \c QSizePolicy::Expanding flags in both directions
	which will take up the full size of the layout. Then add a toolbar with fixed/minimum size policy
	and `Qt::AlignTop | Qt::AlignHCenter` alignment which will "float" on top of the main widget and
	keep itself centered in the available width. Only the actual toolbar area would capture mouse events,
	so interaction with the main area widget is still possible. The toolbar could be spaced away from the
	top of the window using the \c positionOffset property with a positive `y` value.

	\code
		QWidget *w = new QWidget();  // Container widget for stack layout
		// overlay layout for the container, toolbar will "float" on top of main widget
		OverlayStackLayout *lo = new OverlayStackLayout(w);
		lo->setContentsMargins(0,0,0,0);    // Clear the default margins
		QWidget *main = new QWidget(w);     // A content widget
		main->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);  // stretch to fit
		Toolbar *toolbar = new Toolbar(w);  // A floating toolbar
		// Add the main widget
		lo->addWidget(main);
		// Add toolbar widget with center/top alignment and 20px vertical offset
		lo->addWidget(toolbar, Qt::AlignTop | Qt::AlignHCenter, QPoint(0, 20));
		lo->setCurrentIndex(1);  // stack toolbar on top
	\endcode

	\c OverlayStackLayout also respects the \c Qt::WA_LayoutOnEntireRect widget attribute. Widgets with
	this attribute set will always be laid out ignoring any \c contentsMargins() set on this layout (that is,
	using the full available \c geometry() size, vs. \c contentsRect() size). This allows some widgets to
	be spaced away from the edges, while others cover the full available area. Using the example above,
	the toolbar could be spaced away from the top by setting a positive top content margin on the layout,
	and setting the \c Qt::WA_LayoutOnEntireRect attribute on the main expanding widget.

	By default the layout operates in \c QStackedLayout::StackAll mode, meaning no widgets are
	hidden when changing the \c currentIndex, only the stack order changes. However, the
	\c QStackedLayout::StackOne mode can also be used, for example to switch between multiple pages like a
	"typical" stack. In this mode the \c Qt::WA_LayoutOnEntireRect widget attribute could be set
	to ensure one or more widgets are always stacked on top of others.

	Here is another example showing the above concepts in action.

	```
	MyWidget() : QWidget()
	{
		OverlayStackLayout *stackLo = new OverlayStackLayout(this);
	```
	\snippet imageviewer/ImageViewer.cpp 1
	\snippet imageviewer/ImageViewer.cpp 2
	```
	  // ... create more page widgets...
	```
	\snippet imageviewer/ImageViewer.cpp 3
	\snippet imageviewer/ImageViewer.cpp 4
	```
	}
	```

	Note that for non-interactive overlays (eg. messages/information), one can set the
	\c Qt::WA_TransparentForMouseEvents widget attribute to avoid interference with any underlying
	widgets altogether.

	A complete example application demonstrating the different uses of \c OverlayStackLayout is available
	in the \e /examples/imageviewer folder of this project.

	\sa QStackedLayout
*/
class OverlayStackLayout : public QStackedLayout
{
		Q_OBJECT
	public:
		//! Constructs a new \c OverlayStackLayout with the optional \a parent widget.
		//! If \a parent is specified, this layout will install itself on the parent widget.
		explicit OverlayStackLayout(QWidget *parent = nullptr);
		//! Constructs a new \c OverlayStackLayout and inserts it into the given \a parentLayout.
		explicit OverlayStackLayout(QLayout *parentLayout);

		//! Insert \a widget into the stack at \a index position with specified \a alignment.
		//! \sa QStackedLayout::insertWidget()
		int insertWidget(int index, QWidget *widget, Qt::Alignment alignment);
		//! Insert \a widget into the stack at \a index position with specified \a alignment and position \a offset coordinates.
		//! \sa QStackedLayout::insertWidget()
		int insertWidget(int index, QWidget *widget, Qt::Alignment alignment, const QPoint &offset);
		//! Inherited from \c QStackedLayout::insertWidget(int, QWidget *)
		using QStackedLayout::insertWidget;
		//! Add \a widget to the stack with specified \a alignment. \sa QStackedLayout::addWidget()
		int addWidget(QWidget *widget, Qt::Alignment alignment);
		//! Add \a widget to the stack with specified \a alignment and position \a offset coordinates.
		//! \sa QStackedLayout::addWidget()
		int addWidget(QWidget *widget, Qt::Alignment alignment, const QPoint &offset);
		//! Inherited from \c QStackedLayout::addWidget(QWidget *)
		using QStackedLayout::addWidget;

		void setGeometry(const QRect &geo) override;

	public slots:
		//! Set the layout position offset coordinates for given \a widget.
		//! The offset is in absolute pixels and is applied after alignment positioning.
		//! \note Note that you can also simply assign a property named "positionOffset"
		//! with a \c QPoint type value to any widget and it will be respected by this layout.
		void setOffset(QWidget *widget, const QPoint &offset) const;
		//! Convenience slot to set the layout position offset on a signal from a \c QWidget::sender().
		//! This has no effect if the sender widget is not currently in this layout.  \sa setOffset()
		void setSenderOffset(const QPoint &offset) const;
		//! Convenience slot to set the layout alignment on a signal from a \c QWidget::sender().
		//! This has no effect if the sender widget is not currently in this layout.  \sa setAlignment()
		void setSenderAlignment(Qt::Alignment align);

		//! Re-implemented (shadowing) \c QStackedLayout::setStackingMode() to ensure proper child visibility.
		//! The default stacking mode for \c OverlayStackLayout is \c QStackedLayout::StackAll.
		//! \note Setting the stacking mode to \c QStackedLayout::StackOne when `currentIndex() > 0` will
		//! hide (`setVisible(false)`) all widgets except the current one. Conversely, setting the mode to \c StackAll
		//! will set \e all widgets to be visible. This is due to how \c QStackedLayout::setStackingMode() operates.
		//! In general it is recommended to set the desired stacking mode \e before adding widgets.
		void setStackingMode(StackingMode mode);

	private slots:
		void doLayout() const;

	private:
		Q_DISABLE_COPY(OverlayStackLayout)
};

inline
int OverlayStackLayout::addWidget(QWidget *widget, Qt::Alignment alignment)
{
	return insertWidget(count(), widget, alignment);
}

inline
int OverlayStackLayout::addWidget(QWidget *widget, Qt::Alignment alignment, const QPoint &offset)
{
	return insertWidget(count(), widget, alignment, offset);
}

#endif  // OVERLAYSTACKLAYOUT_H
