/*
  ActionPushButton
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

#ifndef ACTIONPUSHBUTTON_H
#define ACTIONPUSHBUTTON_H

#include <QPushButton>

class QAction;
class QEvent;

/*!
	\brief The ActionPushButton class is a \c QPushButton which takes a default \c QAction, just like a \c QToolButton can.
	Like \c QToolButton, it will inherit all properties from the default \c QAction, such as text, icon, checkable status & state,
	tool tip, and so on.  The default action can be set with \c setDefaultAction() and retrieved with \c defaultAction().
	The default action can also be set using the dedicated \ref ActionPushButton(QAction *, QWidget *) constructor.

	It also adds a \c triggered(QAction *) signal for all \c QActions added to the button (not just the default one).
*/
class ActionPushButton : public QPushButton
{
		Q_OBJECT
		//! Current default action, if any. Value is `nullptr` if no default action has been set.
		Q_PROPERTY(QAction *defaultAction READ defaultAction WRITE setDefaultAction)

	public:
		//! Inherits base class constructors.
		using QPushButton::QPushButton;
		//! Construct using \a defaultAction as the default action.
		explicit ActionPushButton(QAction *defaultAction, QWidget *parent = nullptr);

		//! Current default action, if any. Returns `nullptr` if no default action has been set.
		inline QAction *defaultAction() const { return m_defaultAction; }

	public slots:
		//! Sets the default action to \a action. The action is added to the widget if it hasn't been already.
		//! To clear the default action, pass a `nullptr` as the \a action. Clearing the default action in this way
		//! does *not* remove it from this widget itself. Use \c QWidget::removeAction() for that instead, which will
		//! also clear the default action on this button (if the action being removed is the current default, of course).
		void setDefaultAction(QAction *action);

	signals:
		//! Signal emitted whenever any \c QAction added to this button (with \c QWidget::addAction() or \c setDefaultAction()) is triggered.
		void triggered(QAction *);

	protected:
		bool event(QEvent *e) override;
		void nextCheckState() override;

	private slots:
		void updateFromAction(QAction *action);
		void onActionTriggered();

	private:
		QAction *m_defaultAction = nullptr;

		Q_DISABLE_COPY(ActionPushButton)

#ifdef DOXYGEN_SHOULD_INCLUDE_THIS
	public:
		//! Inherited from \c QPushButton. \{
		ActionPushButton(QWidget *parent = nullptr);
		ActionPushButton(const QString &text, QWidget *parent = nullptr);
		ActionPushButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
		//! \}
#endif
};

#endif // ACTIONPUSHBUTTON_H
