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

#include "ActionPushButton.h"

#include <QAction>
#include <QActionEvent>

ActionPushButton::ActionPushButton(QAction *defaultAction, QWidget *parent) :
  QPushButton(parent)
{
	setDefaultAction(defaultAction);
}

bool ActionPushButton::event(QEvent *e)
{
	switch (e->type()) {
		case QEvent::ActionAdded:
			if (QActionEvent *ae = static_cast<QActionEvent *>(e))
				connect(ae->action(), &QAction::triggered, this, &ActionPushButton::onActionTriggered);
			break;

		case QEvent::ActionRemoved:
			if (QActionEvent *ae = static_cast<QActionEvent *>(e)) {
				ae->action()->disconnect(this);
				if (ae->action() == m_defaultAction)
					setDefaultAction(nullptr);
			}
			break;

		case QEvent::ActionChanged:
			if (QActionEvent *ae = static_cast<QActionEvent *>(e))
				if (ae->action() == m_defaultAction)
					updateFromAction(m_defaultAction);
			break;

		default:
			break;
	}
	return QPushButton::event(e);
}

void ActionPushButton::nextCheckState()
{
	if (!!m_defaultAction)
		m_defaultAction->trigger();
	else
		QPushButton::nextCheckState();
}

void ActionPushButton::updateFromAction(QAction *action)
{
	if (!action)
		return;
	QString buttonText = action->iconText();
	// If iconText() is generated from text(), we need to remove any '&'s so they don't turn into shortcuts
	if (buttonText == action->text())
		buttonText.replace(QLatin1String("&"), QLatin1String(""));
	setText(buttonText);
	setIcon(action->icon());
	setToolTip(action->toolTip());
	setStatusTip(action->statusTip());
	setWhatsThis(action->whatsThis());
	setCheckable(action->isCheckable());
	setChecked(action->isChecked());
	setEnabled(action->isEnabled());
	setVisible(action->isVisible());
	setAutoRepeat(action->autoRepeat());
	if (!testAttribute(Qt::WA_SetFont)) {
		setFont(action->font());
		setAttribute(Qt::WA_SetFont, false);
	}
}

void ActionPushButton::setDefaultAction(QAction *action)
{
	if (m_defaultAction == action)
		return;

	if (!!m_defaultAction && !!m_defaultAction->menu() && m_defaultAction->menu() == menu())
		setMenu(nullptr);

	m_defaultAction = action;
	if (!action)
		return;

	if (!actions().contains(action))
		addAction(action);
	updateFromAction(action);
	if (!!action->menu() && !menu())
		setMenu(action->menu());
}

void ActionPushButton::onActionTriggered()
{
	if (QAction *act = qobject_cast<QAction *>(sender()))
		emit triggered(act);
}
