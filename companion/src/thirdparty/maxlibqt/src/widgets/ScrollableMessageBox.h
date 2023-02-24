/*
	ScrollableMessageBox
	https://github.com/mpaperno/maxLibQt

	COPYRIGHT: (c)2017 Maxim Paperno; All Right Reserved.
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

#ifndef SCROLLABLEESSAGEBOX_H
#define SCROLLABLEESSAGEBOX_H

#include <QDialog>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QStyle>
#include <QTextEdit>

/**
	\class ScrollableMessageBox
	\version 1.0.0

	\brief A simple message box with a large scrollable area (a \c QTextEdit ) for detailed text (including HTML formatting).

	This is a basic message box dialog for presenting an amount of text which would not comfortably fit in a regular popup message
	dialog.  It addresses the deficiencies found in \c QMessageBox with "informative text" option, namely its small and fixed
	size and lack of text formatting options (no rich text or even font control).

	Basic usage is like \c QMessageBox.  You can set the dialog title, an optional short text message to show at the top, and
	the larger text block to show as the "details."  There is only one "OK" button to dismiss the dialog.

	When first shown, the dialog will try to esablish a reasonable size based on the contents of the QTextEdit.  The dialog is also
	fully resizable.

	Developers have direct access to the \c QTextEdit used for displaying the text, the \c QLineEdit used for the short text message,
	and the \c QDialogButtonBox with the default "Ok" button.  There are a couple convenience functions for setting the font style
	and word wrap on the \c QTextEdit.  By default a variable-width font is used with word-wrap on.

*/
class ScrollableMessageBox : public QDialog
{
		Q_OBJECT

		class TextEdit : public QTextEdit
		{
			public:
				TextEdit(QWidget *parent=0) : QTextEdit(parent) {}
				QSize sizeHint() const Q_DECL_OVERRIDE;
		};

	public:
		explicit ScrollableMessageBox(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::Dialog);
		ScrollableMessageBox(const QString &title, const QString &text = QString(), const QString &details = QString(), QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::Dialog);

		//! The QTextEdit which holds the detailed text
		QTextEdit * textEdit()         const { return m_textEdit; }
		//! The QLabel which holds the text
		QLabel * textLabel()           const { return m_textLabel; }
		//! The dialog's button box
		QDialogButtonBox * buttonBox() const { return m_btnBox; }

	public slots:
		void setText(const QString &text = QString());
		void setDetailedText(const QString &details = QString(), Qt::TextFormat format = Qt::AutoText);
		void setFontFixedWidth(bool fixed = true);
		void setWordWrap(bool wrap = true);

	protected:
		void init(const QString &title = QString(), const QString &text = QString(), const QString &details = QString());

		QLabel * m_textLabel;
		TextEdit * m_textEdit;
		QDialogButtonBox * m_btnBox;
};

#endif // SCROLLABLEESSAGEBOX_H
