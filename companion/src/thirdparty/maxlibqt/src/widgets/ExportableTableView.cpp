/*
	ExportableTableView
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

#include "ExportableTableView.h"

ExportableTableView::ExportableTableView(QWidget *parent) : QTableView(parent)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	setHtmlStyle(getDefaultHtmlStyle());
	setHtmlTemplate(getDefaultHtmlTemplate());

	QAction * selAll = new QAction(tr("Select All"), this);
	selAll->setShortcut(QKeySequence::SelectAll);
	addAction(selAll);

	QAction * cpyTab = new QAction(tr("Copy selection as TAB-delimited text"), this);
	cpyTab->setShortcut(QKeySequence::Copy);
	cpyTab->setProperty("delim", "\t");
	addAction(cpyTab);

	QAction * cpyCsv = new QAction(tr("Copy selection as comma-delimited text (CSV)"), this);
	cpyCsv->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_C);
	cpyCsv->setProperty("delim", ", ");
	addAction(cpyCsv);

	QAction * cpyPipe = new QAction(tr("Copy selection as pipe-delimited text"), this);
	cpyPipe->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_C);
	cpyPipe->setProperty("delim", " | ");
	addAction(cpyPipe);

	QAction * cpyHtml = new QAction(tr("Copy selection as HTML"), this);
	cpyCsv->setShortcut(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_C);
	cpyHtml->setProperty("delim", "html");
	addAction(cpyHtml);

	QAction * saveFile = new QAction(tr("Save selection to file"), this);
	saveFile->setShortcut(QKeySequence::Save);
	addAction(saveFile);

	connect(selAll, &QAction::triggered, this, &QTableView::selectAll);
	connect(cpyTab, &QAction::triggered, this, &ExportableTableView::copy);
	connect(cpyCsv, &QAction::triggered, this, &ExportableTableView::copy);
	connect(cpyPipe, &QAction::triggered, this, &ExportableTableView::copy);
	connect(cpyHtml, &QAction::triggered, this, &ExportableTableView::copy);
	connect(saveFile, &QAction::triggered, this, &ExportableTableView::save);
	connect(this, &QTableView::customContextMenuRequested, this, &ExportableTableView::onCustomContextMenuRequested);
}

QString ExportableTableView::getDefaultHtmlStyle()
{
	return "th, td {font-family: sans-serif; padding: 3px 15px 3px 3px;} " \
	       "th {text-align: left;}";
}

QString ExportableTableView::getDefaultHtmlTemplate()
{
	return "<!DOCTYPE html>\n<html>\n<head><meta charset='utf-8'/><style>" \
	         "%1" \
	       "</style></head>\n<body>\n" \
	       "<table border=0 cellspacing=2>\n" \
	         "%2" \
	       "</table>\n" \
	       "</body></html>";
}

void ExportableTableView::setHtmlStyle(const QString &value)
{
	m_htmlStyle = value;
}

void ExportableTableView::setHtmlTemplate(const QString &value)
{
	m_htmlTemplate = value;
}

QString ExportableTableView::toPlainText(const QModelIndexList &indexList, const QString &delim) const
{
	QString ret, header;
	const QChar rowDelim = '\n';
	bool firstRow = true;
	for (int i = 0; i < indexList.count(); ++i) {
		const QModelIndex & idx = indexList.at(i);

		if (firstRow)
			header.append(model()->headerData(idx.column(), Qt::Horizontal).toString());

		ret.append(idx.data(Qt::DisplayRole).toString());

		if (i + 1 == indexList.count() || indexList.at(i+1).row() != idx.row()) {
			ret.append(rowDelim);
			if (firstRow && !header.isEmpty())
				header.append(rowDelim);
			firstRow = false;
		}
		else {
			ret.append(delim);
			if (firstRow && !header.isEmpty())
				header.append(delim);
		}
	}
	if (!header.isEmpty())
		ret.prepend(header);

	return ret;
}

QString ExportableTableView::toHtml(const QModelIndexList &indexList) const
{
	QString ret, header, row;
	bool firstRow = true;

	for (int i = 0; i < indexList.count(); ++i) {
		const QModelIndex & idx = indexList.at(i);
		const int algn = (idx.data(Qt::TextAlignmentRole).isValid() ? idx.data(Qt::TextAlignmentRole).toInt() : (Qt::AlignLeft | Qt::AlignVCenter));
		const QString fg = (idx.data(Qt::ForegroundRole).isValid() ? idx.data(Qt::ForegroundRole).value<QBrush>().color().name(QColor::HexRgb) : "initial");
		const QString bg = (idx.data(Qt::BackgroundRole).isValid() ? idx.data(Qt::BackgroundRole).value<QBrush>().color().name(QColor::HexRgb) : "initial");
		const QString ttl = (idx.data(Qt::ToolTipRole).isValid() ? idx.data(Qt::ToolTipRole).toString().replace("\"", "\"\"") : QString());

		QString fnt;
		if (idx.data(Qt::FontRole).isValid()) {
			const QFont font = idx.data(Qt::FontRole).value<QFont>();
			fnt = "font: \"" % font.family() % "\" " % QString::number(font.pointSize()) % "pt;";
		}

		if (firstRow)
			header.append(QString("<th>%1</th>\n").arg(model()->headerData(idx.column(), Qt::Horizontal).toString()));

		row.append("<td style='color: %2; background-color: %3; %4' align='%5' valign='%6' %7>%1</td>\n");  // cell template
		row = row.arg(idx.data(Qt::DisplayRole).toString()).arg(fg).arg(bg).arg(fnt);
		row = row.arg((algn & Qt::AlignRight) ? "right" : (algn & Qt::AlignHCenter) ? "center" : "left");
		row = row.arg((algn & Qt::AlignTop) ? "top" : (algn & Qt::AlignBottom) ? "bottom" : "middle");
		row = row.arg(ttl.isEmpty() ? ttl : QString("title=\"%1\"").arg(ttl));

		if (i + 1 == indexList.count() || indexList.at(i+1).row() != idx.row()) {
			ret.append(QString("<tr>\n%1</tr>\n").arg(row));
			row.clear();
			firstRow = false;
		}
	}

	ret = QString("<tbody>\n%1</tbody>\n").arg(ret);
	if (!header.isEmpty())
		ret.prepend(QString("<thead>\n<tr>\n%1</tr>\n</thead>\n").arg(header));
	ret = m_htmlTemplate.arg(m_htmlStyle).arg(ret);

	return ret;
}

bool ExportableTableView::saveToFile(const QModelIndexList &indexList, const QString &fileName)
{
	static QString lastDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

	if (indexList.isEmpty())
		return false;

	QString fname = fileName;
	if (fname.isEmpty()) {
		QString types = tr("Tab-delimited text") % " (*.tab);;" % tr("Comma-delimited text") % " (*.csv);;" % tr("Pipe-delimited text") % " (*.txt);;" % tr("HTML") % " (*.html)";
		fname = QFileDialog::getSaveFileName(this, tr("Save to file"), lastDir, types);
	}

	if (fname.isEmpty())
		return false;

	QFile file(fname);
	QFileInfo fi(file);
	lastDir = fi.absolutePath();

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	if (fi.suffix() == "html")
		file.write(toHtml(indexList).toUtf8());
	else
		file.write(toPlainText(indexList, (fi.suffix() == "tab" ? "\t" : fi.suffix() == "csv" ? ", " : " | ")).toUtf8());
	file.close();

	return true;
}

QSize ExportableTableView::sizeHint() const
{
	int w = 0;
	for (int i = 0; i < model()->columnCount(); ++i)
		w += sizeHintForColumn(i);
	return QSize(w + verticalScrollBar()->sizeHint().width() + 60, sizeHintForRow(0) * model()->rowCount());
}

void ExportableTableView::copyText(const QString &delim)
{
	QApplication::clipboard()->setText(toPlainText(getSelectedOrAll(), delim));
}

void ExportableTableView::copyHtml()
{
	QMimeData * mdata = new QMimeData;
	mdata->setHtml(toHtml(getSelectedOrAll()));
	QApplication::clipboard()->setMimeData(mdata);
}

void ExportableTableView::copy()
{
	QString delim = "\t";
	if (sender() && sender()->property("delim").isValid())
		delim = sender()->property("delim").toString();
	if (delim == "html")
		copyHtml();
	else
		copyText(delim);
}

void ExportableTableView::save()
{
	saveToFile(getSelectedOrAll());
}

QModelIndexList ExportableTableView::getSelectedOrAll()
{
	QModelIndexList sel = selectionModel()->selectedIndexes();
	if (sel.isEmpty()) {
		selectAll();
		sel = selectionModel()->selectedIndexes();
	}
	return sel;
}

void ExportableTableView::onCustomContextMenuRequested(const QPoint & pos)
{
	QMenu menu;
	menu.addActions(actions());
	menu.exec(mapToGlobal(pos));
}
