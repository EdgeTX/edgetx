/*
	TreeComboBox
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

#ifndef TREECOMBOBOX_H
#define TREECOMBOBOX_H

#include <QComboBox>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QModelIndex>
#include <QScrollBar>

class QPainter;

/**
	\class TreeComboItemDelegate
	\brief An item rendering delegate to highlight parent items and draw better item separators. Used by TreeComboBox.

	This delegate differs from the default by how it renders separator items and by adding a new formatting style for "parent" items.
	With this delegate, separators are visible on all platforms.  Parent items get bolded and italicisized fonts.
	Parent items are drawn with enabled style even if they're actually disabled.
	It also has convenience static functions for flagging/checking a data model item as a separator or parent.
	The separator type is fully compatible with default QComboBox (it is set the same way).

*/
class TreeComboItemDelegate : public QStyledItemDelegate
{
		Q_OBJECT

	public:
		explicit TreeComboItemDelegate(QObject *parent = Q_NULLPTR) : QStyledItemDelegate(parent) {}

		static bool isSeparator(const QModelIndex &index);
		static bool isParent(const QModelIndex &index);
		static void setSeparator(QAbstractItemModel *model, const QModelIndex &index);
		static void setParent(QAbstractItemModel *model, const QModelIndex &index, const bool selectable = false);

	protected:
		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
		virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
};


/**
	\class TreeComboBox
	\version 1.0.0

	\brief A \c QComboBox control which works with a tree-based data model & view, allowing drill-down selection of items.

	TreeComboBox accepts a nested/tree data model and presents the first column of the data as choices in the dropdown.
	It respects the data model's settings as to which items are selectable or not (typically only the last-level "leaf" elements).

	It works best when unique \c Qt::UserRole data is set on the items and used for further access, but it should also work for
	index-based operations (\c currentIndex() and \c setCurrentIndex().  If your data does not already have unique \c Qt::UserRole
	data, you can have TreeComboBox create it automatically by setting the \p autoData property to \e true.

	Although it is usually simpler to build the data model first and then assign it to the combo box, there are also methods included
	to build the model directly, just as with \c QComboBox.  In addition to the usual methods for adding items, there are new methods
	for adding parent items and specifying parents when adding sub-items.

	It preserves keyboard navigation of items and expanding/collapsing parents.  It tries to present a good user experience when the
	size of the popup selection box changes as ites are expanded/collapsed.

	Unfortunately \c QComboBox is not easy to subclass or modify since most of the class is private.  This is a reasonable attempt,
	and is a bit similar to \c TreeViewComboBox used in \e QtCreator, but geared towards much more general use.

*/
class TreeComboBox : public QComboBox
{
		Q_OBJECT
		Q_PROPERTY(bool autoData READ autoData WRITE setAutoData)

	public:
		explicit TreeComboBox(QWidget * parent = Q_NULLPTR);

		/*! \reimp  Reimplemented from \e QComboBox::setModel()  */
		void setModel(QAbstractItemModel *model);
		/*!  Like \e QComboBox::setView but only accepts QTreeView (and derived) view classes.  */
		void setView(QAbstractItemView *itemView = Q_NULLPTR);  // only QTreeView and derived
		inline QTreeView * view() const { return m_view; }
		inline bool autoData() const    { return m_autoData; }

		/*! \reimp  Reimplemented from \e QComboBox::currentIndex()  */
		int currentIndex() const;
		/*! \reimp  Reimplemented from \e QComboBox::currentData()  */
		QVariant currentData(int role = Qt::UserRole) const;
		/*! \reimp  Reimplemented from \e QComboBox::itemData()  */
		QVariant itemData(int index, int role = Qt::UserRole) const;
		/*! \reimp  Reimplemented from \e QComboBox::itemIcon()  */
		QIcon itemIcon(int index) const;
		inline QString itemText(int index) const { return itemData(index, Qt::DisplayRole).toString(); }

		/*! Sets the data \a role for the item at \a index to the specified \a value. Convenience function vs. accessing the data model directly.  */
		void setItemData(const QModelIndex &index, const QVariant &value, int role = Qt::UserRole);
		void setItemData(int index, const QVariant &value, int role = Qt::UserRole) { setItemData(m_indexMap.value(index, QModelIndex()), value, role); }
		void setItemText(int index, const QString &text)                            { setItemData(index, QVariant::fromValue(text), Qt::EditRole); }
		void setItemIcon(int index, const QIcon &icon)                              { setItemData(index, QVariant::fromValue(icon), Qt::DecorationRole); }

		/*! \reimp  Reimplemented from \e QComboBox::findData()  */
		int findData(const QVariant &data, int role = Qt::UserRole, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive)) const;
		inline int findText(const QString &text,
		                    Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive)) const { return findData(text, Qt::DisplayRole, flags); }

		/*!
			Insert an item at given \a index (row), with optional data \a values, under parent item indicated by \a parentIndex.
			 \param index  Row number at which to put item.
			 \param values  Optional map of data \e role to \e value pairs to add to the item's \e data().
			 \param parentIndex  If not specified, current view rootIndex is used.
			 \param reload  Used to indicate that this is the last insert in a series. When \e false, item model signals are blocked during data changes.
			 \return  Model index of newly inserted item. May be invalid index if insert failed.
		*/
		QModelIndex insertItem(int index, const QMap<int, QVariant> &values = QMap<int, QVariant>(), const QModelIndex &parentIndex = QModelIndex(), const bool reload = true);

		/*!
			 Like \e QComboBox::insertItem but adds ability to specify a parent item. \a index indicates row number in parent.
			 \param index  Row number at which to put item.
			 \param parentIndex  If not specified, current view rootIndex is used.
			 \param reload Used to indicate that this is the last insert in a series. When \e false, item model signals are blocked during data changes.
			 \return  Model index of newly inserted item. May be invalid index if insert failed.
			 \sa insertItem(int, QMap<int, QVariant>, QModelIndex, bool)
		 */
		QModelIndex insertItem(int index, const QIcon & icon, const QString &text, const QModelIndex &parentIndex,  const QVariant &userData = QVariant(), const bool reload = true);

		/*!  Inserts multiple items from a list as children of \a parentIndex. \sa insertItems()  */
		void insertItems(int index, const QStringList &texts, const QModelIndex &parentIndex);

		// in addition to regular QComboBox add/insert, these allow specifying a parent item instead of the default root
		inline QModelIndex addItem(const QString &text, const QModelIndex &parentIndex,
		                           const QVariant &userData = QVariant())                                   { return addItem(QIcon(), text, parentIndex, userData); }
		inline QModelIndex addItem(const QIcon &icon, const QString &text, const QModelIndex &parentIndex,
		                           const QVariant &userData = QVariant())                                   { return insertItem(model()->rowCount(parentIndex), icon, text, parentIndex, userData); }
		inline void addItems(const QStringList &texts, const QModelIndex &parentIndex)                      { insertItems(model()->rowCount(parentIndex), texts, parentIndex); }

		// re-implement QComboBox add/insert interface
		inline void insertItem(int index, const QIcon &icon, const QString &text,
		                       const QVariant &userData = QVariant())                                       { insertItem(index, icon, text, QModelIndex(), userData); }
		inline void addItem(const QString &text, const QVariant &userData = QVariant())                     { insertItem(count(), QIcon(), text, userData); }
		inline void addItem(const QIcon &icon, const QString &text, const QVariant &userData = QVariant())  { insertItem(count(), icon, text, userData); }
		inline void insertItem(int index, const QString &text, const QVariant &userData = QVariant())       { insertItem(index, QIcon(), text, userData); }
		inline void addItems(const QStringList &texts)                                                      { insertItems(count(), texts); }
		inline void insertItems(int index, const QStringList &texts)                                        { insertItems(index, texts, QModelIndex()); }

		// parent items (convenience functions insted of creating own data model)
		/*!
			Inserts an item and marks is as a "parent" type item. By default the parent item is only used to group other items and cannot be selected.
			 \param index  Row number at which to put item.
			 \param parentIndex  If not specified, current view rootIndex is used.
			 \return  Model index of newly inserted item. This index can then be used to add children items. May be invalid index if insert failed.
			 \sa insertItem(), TreeComboItemDelegate::setParent()
		*/
		QModelIndex insertParentItem(int index, const QIcon &icon, const QString &text, const bool selectable = false,
		                             const QModelIndex &parentIndex = QModelIndex(), const QVariant &userData = QVariant());
		inline QModelIndex addParentItem(const QString &text, const bool selectable = false, const QModelIndex &parentIndex = QModelIndex(),
		                                 const QVariant &userData = QVariant())                             { return addParentItem(QIcon(), text, selectable, parentIndex, userData); }
		inline QModelIndex addParentItem(const QIcon &icon, const QString &text, const bool selectable = false, const QModelIndex &parentIndex = QModelIndex(),
		                                 const QVariant &userData = QVariant())                             { return insertParentItem(model()->rowCount(parentIndex), icon, text, selectable, parentIndex, userData); }

		/*!  \reimp  Reimplemented from \e QComboBox::wheelEvent()  */
		virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
		/*! \reimp  Reimplemented from \e QComboBox::keyPressEvent()  */
		virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
		/*! \reimp  Reimplemented from \e QComboBoxPrivateContainer::eventFilter()  */
		virtual bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;
		/*! \reimp  */
		virtual void showPopup() Q_DECL_OVERRIDE;
		/*! \reimp  */
		virtual void hidePopup() Q_DECL_OVERRIDE;

	public slots:
		/*!  Like \e QComboBox::clear() but  <b>does NOT remove any items from the current data model</b>.  Instead, a new, blank, model is created.  */
		void clear();
		/*! \reimp  Reimplemented from \e QComboBox::setCurrentIndex()  */
		void setCurrentIndex(int index);
		/*!  Sets currently selected item to \a index using data model indices.  This is ultimately called by all other methods which may set the current item.  */
		void setCurrentIndex(const QModelIndex &index);
		void setCurrentText(const QString &text) { setCurrentIndex(findText(text)); }

		/*!
			This is a shortcut to using setCurrentIndex(findData()), and also returns the new index (like findData() would).
			If data was not found, will set current item to \a defaultIdx (defalut is -1, an invalid index).
		*/
		int setCurrentData(const QVariant &data, const int defaultIdx = -1, int role = Qt::UserRole, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));
		/*!  \reimp  Reimplemented from \e QComboBoxPrivate::keyboardSearchString()  */
		void keyboardSearchString(const QString &text);
		void setAutoData(bool enable) { m_autoData = enable; }

	signals:
		void currentModelIndexChanged(const QModelIndex &index);
		void currentDataChanged(const QVariant &data);  //! Emitted along with currentIndexChanged() signals and if item has valid Qt::UserRole data.

	protected slots:
		/*!
			We keep a mapping of original tree model data to the "flat" reprenstation which QComboBox understands.
			This allows eg. setCurrentIndex(int) and findData() to work as normal. We exclude any non-selectable items.
		*/
		int buildMap(QModelIndex parent = QModelIndex(), int row = 0);
		void reloadModel();
		/*!  Called before showing popup to make sure the initial view size is correct */
		void ensureCurrentExpanded();
		/*!  Make sure tree view has a reasonable width. */
		void adjustPopupWidth();
		/*!  Resize visible popup height when items are expanded/collapsed in the tree view  */
		void adjustPopupHeight();

	protected:
		/*!  Find first valid item index which if before given \a index.  */
		QModelIndex indexAbove(QModelIndex index);
		/*!  Find first valid item index which if after given \a index.  */
		QModelIndex indexBelow(QModelIndex index);
		/*!  Find last (greatest) valid item index in a set (eg. under a parent), starting at \a index.  */
		QModelIndex lastIndex(const QModelIndex &index);
		/*!  Return true if style options are set to use a "popup" presentation style (typ. true on Linux/OS X if non-editable combo) */
		bool usePopup(QStyleOptionComboBox *option = NULL);

		QTreeView * m_view;
		QMap<QPersistentModelIndex, int> m_rowMap;
		QMap<int, QPersistentModelIndex> m_indexMap;
		QPersistentModelIndex m_currentIndex;
		int m_nextDataId;
		bool m_autoData;
		bool m_skipNextHide;
};


/**
   \class TreeComboBoxView
   \brief QTreeView subclass used to display the combo box popup items. Only needed to compute the column width.
          We could do this from within TreeComboBox, except that sizeHintForColumn() is protected and is apparently the only way to get a reasonable width. :-|
*/

class TreeComboBoxView : public QTreeView
{
	Q_OBJECT
	public:
		inline void adjustWidth(int maxWidth) {
			resizeColumnToContents(0);
			setMaximumWidth(maxWidth);
			setMinimumWidth(qMin(qMax(sizeHintForColumn(0), minimumSizeHint().width()), maxWidth) + verticalScrollBar()->sizeHint().width() + indentation());
		}
};

#endif // TREECOMBOBOX_H
