/*
  BuddyLabel
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

#ifndef BUDDYLABEL_H
#define BUDDYLABEL_H

#include <QLabel>
#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QMouseEvent>
#include <QLineEdit>

/*!
	\brief The BuddyLabel class is a QLabel with enhanced "buddy" capabilities.

	It overrides the \c QLabel::setBuddy() method and, besides the usual shortcut handling provided by \c QLabel,
	it adds mouse click handling and mirroring of the buddy's tool tip text.

	Mouse clicks are connected to the \c QWidget::setFocus slot. For \c QCheckBox it also connects to the \c click() slot so the box can be (un)checked by clicking on the label.
	Mouse double-clicks are connected to \c QLineEdit::selectAll() on widgets which either are or have a \c QLineEdit (like \c QAbstractSpinBox and editable \c QComboBox).
	Custom connections could be added by connecting to the \c clicked() and/or \c doubleClicked() signals, or inheriting and overriding the \c connectBuddy() virtual method.
*/
class BuddyLabel : public QLabel
{
		Q_OBJECT
	public:
		using QLabel::QLabel;

	public slots:
		//! Overrides the \c QLabel::setBuddy() method, which isn't virtual. Calls the base class implementation as well, so the shortcut mechanism still works.
		void setBuddy(QWidget *buddy)
		{
			if (this->buddy()) {
				this->buddy()->removeEventFilter(this);
				disconnect(this->buddy());
				disconnectBuddy(this->buddy());
			}

			QLabel::setBuddy(buddy);

			if (!buddy)
				return;

			setToolTip(buddy->toolTip());
			buddy->installEventFilter(this);
			connectBuddy(buddy);
		}

	signals:
		//! Emitted when label is clicked with left mouse button (or something emulating one).
		void clicked();
		//! Emitted when label is double-clicked with left mouse button (or something emulating one).
		void doubleClicked();

	protected:
		//! Override this method for custom connections.
		virtual void connectBuddy(QWidget *buddy)
		{
			// Single clicks
			connect(this, &BuddyLabel::clicked, buddy, QOverload<>::of(&QWidget::setFocus));
			if (QCheckBox *cb = qobject_cast<QCheckBox*>(buddy))
				connect(this, &BuddyLabel::clicked, cb, &QCheckBox::click);

			// Double clicks
			if (QLineEdit *le = qobject_cast<QLineEdit*>(buddy))
				connect(this, &BuddyLabel::doubleClicked, le, &QLineEdit::selectAll);
			else if (QAbstractSpinBox *sb = qobject_cast<QAbstractSpinBox*>(buddy))
				connect(this, &BuddyLabel::doubleClicked, sb, &QAbstractSpinBox::selectAll);
			else if (QComboBox *cb = qobject_cast<QComboBox*>(buddy))
				if (cb->isEditable() && cb->lineEdit())
					connect(this, &BuddyLabel::doubleClicked, cb->lineEdit(), &QLineEdit::selectAll);
		}

		//! Hook for custom disconnections. We already disconnect ourselves from all slots in \a buddy in the main handler.
		virtual void disconnectBuddy(QWidget *buddy) { Q_UNUSED(buddy) }

		//! The filter monitors for tool tip changes on the buddy
		bool eventFilter(QObject *obj, QEvent *ev)
		{
			if (ev->type() == QEvent::ToolTipChange && buddy() && obj == buddy())
				setToolTip(buddy()->toolTip());
			return false;
		}

		void mousePressEvent(QMouseEvent *ev)
		{
			if (ev->button() == Qt::LeftButton) {
				m_pressed = true;
				ev->accept();
			}
			QLabel::mousePressEvent(ev);
		}

		void mouseReleaseEvent(QMouseEvent *ev)
		{
			if (m_pressed && rect().contains(ev->pos()))
				emit clicked();
			m_pressed = false;
			QLabel::mouseReleaseEvent(ev);
		}

		void mouseDoubleClickEvent(QMouseEvent *ev)
		{
			if (ev->button() == Qt::LeftButton && rect().contains(ev->pos()))
				emit doubleClicked();
			QLabel::mouseDoubleClickEvent(ev);
		}

	private:
		bool m_pressed = false;
		Q_DISABLE_COPY(BuddyLabel)
};

#endif // BUDDYLABEL_H
