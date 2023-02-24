/*
	ScrollableMessageBox

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

#include "ScrollableMessageBox.h"


ScrollableMessageBox::ScrollableMessageBox(QWidget *parent, Qt::WindowFlags f) :
  QDialog(parent, f | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
	init();
}

ScrollableMessageBox::ScrollableMessageBox(const QString &title, const QString &text, const QString &details, QWidget *parent, Qt::WindowFlags f) :
  QDialog(parent, f | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
	init(title, text, details);
}

void ScrollableMessageBox::setText(const QString &text)
{
	m_textLabel->setText(text);
}

void ScrollableMessageBox::setDetailedText(const QString &details, Qt::TextFormat format)
{
	if (format == Qt::AutoText)
		m_textEdit->setText(details);
	else if (format == Qt::RichText)
		m_textEdit->setHtml(details);
	else
		m_textEdit->setPlainText(details);
}

void ScrollableMessageBox::setFontFixedWidth(bool fixed)
{
	QFont newFont(m_textEdit->font());
	if (fixed) {
		newFont.setFamily("Courier");
		newFont.setStyleHint(QFont::TypeWriter);
	}
	else {
		newFont.setFamily("Helvetica");
		newFont.setStyleHint(QFont::SansSerif);
	}
#ifdef Q_OS_MACOS
	newFont.setPointSize(13);
	textEdit->setAttribute(Qt::WA_MacNormalSize);
#elif defined Q_OS_WIN
	newFont.setPointSize(10);
#endif
	m_textEdit->setFont(newFont);
}

void ScrollableMessageBox::setWordWrap(bool wrap)
{
	m_textEdit->setWordWrapMode(wrap ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
}

void ScrollableMessageBox::init(const QString & title, const QString & text, const QString & details)
{
	m_textLabel = new QLabel(this);
	m_textLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	m_textLabel->setOpenExternalLinks(true);

	m_textEdit = new TextEdit(this);
	m_textEdit->setReadOnly(true);
	m_textEdit->setTextInteractionFlags(Qt::TextBrowserInteraction | Qt::TextSelectableByKeyboard);

	m_btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);

	connect(m_btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(m_btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	QVBoxLayout * lo = new QVBoxLayout(this);
	lo->setSpacing(8);
	lo->addWidget(m_textLabel);
	lo->addWidget(m_textEdit);
	lo->addWidget(m_btnBox);

	setSizeGripEnabled(true);
	setFontFixedWidth(false);
	setWordWrap(true);

	if (!title.isEmpty())
		setWindowTitle(title);
	if (!text.isEmpty())
		setText(text);
	if (!details.isEmpty())
		setDetailedText(details);
}


/*
	 ScrollableMessageBox::TextEdit
*/

QSize ScrollableMessageBox::TextEdit::sizeHint() const
{
	// stupid trick to get an idea of necessary size to contain all the text, works for html and plain
	QLabel tmp;
	tmp.setFont(font());
	tmp.setText(toHtml());
	return tmp.sizeHint() + QSize(30, 20);
}
