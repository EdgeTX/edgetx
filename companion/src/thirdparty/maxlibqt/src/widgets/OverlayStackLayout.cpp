/*
	OverlayStackLayout
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

#include "OverlayStackLayout.h"
#include <QVariant>
#include <QWidget>

namespace {
static const char offsetProperty[15] {"positionOffset"};
}

OverlayStackLayout::OverlayStackLayout(QWidget * parent) :
  QStackedLayout(parent)
{
	setStackingMode(StackAll);
}

OverlayStackLayout::OverlayStackLayout(QLayout * parentLayout) :
  QStackedLayout(parentLayout)
{
	setStackingMode(StackAll);
}

int OverlayStackLayout::insertWidget(int index, QWidget *widget, Qt::Alignment alignment)
{
	const int ret = insertWidget(index, widget);
	if (ret > -1)
		setAlignment(widget, alignment);
	return ret;
}

int OverlayStackLayout::insertWidget(int index, QWidget *widget, Qt::Alignment alignment, const QPoint &offset)
{
	const int ret = insertWidget(index, widget, alignment);
	if (ret > -1)
		setOffset(widget, offset);
	return ret;
}

void OverlayStackLayout::setOffset(QWidget *widget, const QPoint &offset) const
{
	if (widget->property(offsetProperty).isValid() && widget->property(offsetProperty).toPoint() == offset)
		return;
	widget->setProperty(offsetProperty, offset);
	doLayout();
}

void OverlayStackLayout::setSenderOffset(const QPoint &offset) const
{
	if (QWidget *w = qobject_cast<QWidget*>(sender()))
		setOffset(w, offset);
}

void OverlayStackLayout::setSenderAlignment(Qt::Alignment align)
{
	if (QWidget *w = qobject_cast<QWidget*>(sender()))
		setAlignment(w, align);
}

void OverlayStackLayout::setStackingMode(QStackedLayout::StackingMode mode)
{
	if (mode == stackingMode())
		return;
	QStackedLayout::setStackingMode(mode);
	// resetting the mode to StackAll messes with our layout
	if (mode == StackAll)
		doLayout();
}

void OverlayStackLayout::setGeometry(const QRect & geo)
{
	if (geo == geometry())
		return;
	QLayout::setGeometry(geo);
	doLayout();
}

void OverlayStackLayout::doLayout() const
{
	const int n = count();
	if (!n)
		return;
	for (int i=0; i < n; ++i) {
		QWidget *w = nullptr;
		if (QLayoutItem *item = itemAt(i))
			w = item->widget();
		if (!w)
			continue;

		// available geometry for widgets, use size based on layout attribute
		const QRect rect = w->testAttribute(Qt::WA_LayoutOnEntireRect) ? geometry() : contentsRect();
		// widget's desired size
		const QSize wSize = w->sizeHint().expandedTo(w->minimumSize()).boundedTo(w->maximumSize());
		QRect wRect(rect.topLeft(), wSize);  // default widget position and size
		QPoint offset(0, 0);   // position offset
		if (w->property(offsetProperty).isValid())
			offset = w->property(offsetProperty).toPoint();

		// expand or constrain to full width?
		if ((w->sizePolicy().expandingDirections() & Qt::Horizontal) || wRect.width() > rect.width() - offset.x())
			wRect.setWidth(rect.width() - offset.x());

		// expand or constrain to full height?
		if ((w->sizePolicy().expandingDirections() & Qt::Vertical) || wRect.height() > rect.height() - offset.y())
			wRect.setHeight(rect.height() - offset.y());

		// Adjust position for alignment
		const Qt::Alignment align = itemAt(i)->alignment();
		if (rect.width() > wRect.width() - offset.x()) {
			switch (align & Qt::AlignHorizontal_Mask) {
				case Qt::AlignHCenter:
					wRect.moveLeft(rect.x() + (rect.width() - wRect.width()) / 2);
					break;
				case Qt::AlignRight:
					wRect.moveRight(rect.right());
					break;
				case Qt::AlignLeft:
				default:
					wRect.moveLeft(rect.left());
					break;
			}
		}
		if (rect.height() > wRect.height() - offset.y()) {
			switch (align & Qt::AlignVertical_Mask) {
				case Qt::AlignVCenter:
					wRect.moveTop(rect.y() + (rect.height() - wRect.height()) / 2);
					break;
				case Qt::AlignBottom:
					wRect.moveBottom(rect.bottom());
					break;
				case Qt::AlignTop:
				default:
					wRect.moveTop(rect.top());
					break;
			}
		}
		// adjust for user-defined offset
		if (!offset.isNull())
			wRect.moveTopLeft(wRect.topLeft() + offset);

		// Set position and size of the widget.
		w->setGeometry(wRect);
		// Honor the stacking attribute
		if (w->testAttribute(Qt::WA_AlwaysStackOnTop))
			w->raise();
	}
}

#include "moc_OverlayStackLayout.cpp"
