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

#ifndef EXPORTABLETABLEVIEW_H
#define EXPORTABLETABLEVIEW_H

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMimeData>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTableView>

/**
	\class ExportableTableView
	\version 1.0.1

	\brief The ExportableTableView class provides a regular QTableView but with features to export the data
	as plain text or HTML.

	Any selection of data can be exported.  The horizontal headings, if any, are included in the export.
	The export functions are available to the user via a custom context menu or keyboard shortcuts.
	3 field delimiter choices are available for plain-text export (tab, comma, or pipe).
	Data can be saved to the clipboard or to a file.

	The export functions can also be accessed programmatically via \c toPlainText(), \c toHtml(),
	and \c saveToFile().

	The style sheet for the generated page can be set with \c setHtmlStyle().  The overall HTML
	template can be customized with \c setHtmlTemplate().  The data itself is always
	formatted as a basic HTML table and then inserted into the template at the \c %2 placeholder.

	HTML version tries to preserve many data role attributes of the model items:

	\li \c Qt::FontRole
	\li \c Qt::ForegroundRole
	\li \c Qt::BackgroundRole
	\li \c Qt::TextAlignmentRole
	\li \c Qt::ToolTipRole

	Note that \c Qt::EditRole data is not specifically preserved (unless it already matches \c Qt::DisplayRole ).

 */
class ExportableTableView : public QTableView
{
	Q_OBJECT

	public:
		/*!
			 \brief ExportableTableView
			 \param parent
		 */
		ExportableTableView(QWidget *parent = Q_NULLPTR);

		/*! \brief Get the default style sheet used for HTML export. */
		static QString getDefaultHtmlStyle();
		/*! \brief Get the default overall template used for HTML export. */
		static QString getDefaultHtmlTemplate();
		/*!
			\brief Set the style sheet for HTML export. This goes between the \c "<style></style>" tags of the page.
			\param value  The CSS code.
		 */
		void setHtmlStyle(const QString &value);
		/*!
			\brief Set the overall template for HTML export. The template must have two placeholders: \c %1 for the style and \c %2 for the table.
			\param value  The HTML code.
		 */
		void setHtmlTemplate(const QString &value);

		/*!
			\brief Saves data from the passed model indices to a text string.
			\param indexList  A list of indices to export.
			\param delim  The delimiter to use. Can be multiple characters (eg. \c ", ") Default is a single TAB.
			\return \e QString with the formatted text. If the data has column headings, these are on the first line.
		 */
		QString toPlainText(const QModelIndexList &indexList, const QString &delim = "\t") const;

		/*!
			\brief Saves data from the passed model indices to an HTML-formatted string.
			\param indexList  A list of indices to export.
			\return \e QString with the formatted HTML. If the data has column headings, these are included as table headings.
		 */
		QString toHtml(const QModelIndexList &indexList) const;

		/*!
			Saves data from the passed model indices to a file in text or HTML format. The file extension determines
			the format/text delimiter: \e \.html for HTML, \e \.tab for TAB, \e \.csv for CSV, and anything else
			is pipe-delimited.

			\param indexList  A list of indices to export.
			\param fileName  An optional file name (with path). If none is passed then a file chooser dialog is presented.
			\return true on success, false on failure (no file selected or write error)
		 */
		bool saveToFile(const QModelIndexList &indexList, const QString &fileName = QString());

		/*! \brief Return a list of selected cells. If none are selected then it first selects the whole table. */
		QModelIndexList getSelectedOrAll();

		QSize sizeHint() const Q_DECL_OVERRIDE;

	public slots:
		/*!
			\brief Copies currently selected cell(s) to the clipboard as plain text.
			\param delim  The delimiter to use. Can be multiple characters (eg. ", ") Default is a single TAB.
		 */
		void copyText(const QString &delim = QString("\t"));
		/*! \brief Copies currently selected cell(s) to the clipboard as HTML (with MIME content-type \e "text/html"). */
		void copyHtml();
		/*! \brief Designed for \e QAction connections, it calls \c copyText() or \c copyHtml() based on delimiter specified in \c sender()->property\("delim"\) (use "html" as \p delim for HTML format). */
		void copy();
		/*! \brief Saves currently selected cell(s) to a file of the user's choice and format. */
		void save();

	protected:
		void onCustomContextMenuRequested(const QPoint &pos);

		QString m_htmlTemplate;
		QString m_htmlStyle;
};

#endif // EXPORTABLETABLEVIEW_H
