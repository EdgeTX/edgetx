/*
	GroupedItemsProxyModel
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

#ifndef GROUPEDITEMSPROXYMODEL_H
#define GROUPEDITEMSPROXYMODEL_H

#include <QIdentityProxyModel>
#include <QSize>

/**
	\class GroupedItemsProxyModel
	\version 1.0.0

	\brief GroupedItemsProxyModel allows a grouped item presentation of a flat table data model.

	GroupedItemsProxyModel allows a grouped item presentation of a flat table data model. Items can be visually grouped by data in one or more
	columns of the original data. Any number of groupings can be nested. GroupedItemsProxyModel inherits from QIdentityProxyModel and
	proxies all data from the original model unmodified. Only the visual presentation is affected, for example like \e QSortFilterProxyModel.

	Groupings can be managed with addGroup(), insertGroup() and removeGroup(). Each function takes a column number as a parameter. The source
	data is scanned, and a new grouping row is created for each unique value found in the given column. The title (and icon, if any) of the grouping
	row is taken from the source data (\c Qt::DisplayRole and \c Qt::DecorationRole respectively). By default, items are grouped based on
	their \c Qt::EditRole but you can change this with setGroupMatchRole().

	When grouping is active, a new first column is (optionally) added which can be used to sort the groupings. By default the groups are shown
	in the order in which they were found in the source data. The extra column can be disabled with setGroupColumnVisible(). The title of the column
	can be set with setGroupHeaderTitle().

	This model can be cascased with other models. For example set this model as the source for a \e QSortFilterProxyModel to enable sorting
	the view (including the grouping items as mentioned above).

	This model can only handle <b>flat data models</b> as input (table/list). It will not include any children of the top-level items.

	When no groupings have been added, this acts as a transparent proxy (see \e QIdentityProxyModel). However the limitation of only flat source
	models still applies.

*/
class GroupedItemsProxyModel : public QIdentityProxyModel
{
		Q_OBJECT
		Q_PROPERTY(int groupMatchRole READ groupMatchRole WRITE setGroupMatchRole)
		Q_PROPERTY(bool groupColumnVisible READ groupColumnVisible WRITE setGroupColumnVisible)
		Q_PROPERTY(bool groupColumnIsProxy READ groupColumnIsProxy WRITE setGroupColumnIsProxy)
		Q_PROPERTY(int groupColumnProxySrc READ groupColumnProxySrc WRITE setGroupColumnProxySrc)
		Q_PROPERTY(bool groupRowSelectable READ groupRowSelectable WRITE setGroupRowSelectable)
		Q_PROPERTY(QString groupHeaderTitle READ groupHeaderTitle WRITE setGroupHeaderTitle)

	public:
		enum { SourceRowNumberRole = Qt::UserRole - 5 };

		explicit GroupedItemsProxyModel(QObject *parent = Q_NULLPTR, QAbstractItemModel *sourceModel = Q_NULLPTR, const QVector<int> &groupColumns = QVector<int>());
		~GroupedItemsProxyModel();

		// QAbstractItemModel interface
		void setSourceModel(QAbstractItemModel *newSourceModel) Q_DECL_OVERRIDE;
		QModelIndex index(int row, int column, const QModelIndex &parent) const Q_DECL_OVERRIDE;
		QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
		int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
		int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
		bool hasChildren(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
		QVariant data(const QModelIndex &index, int role = Qt::EditRole) const Q_DECL_OVERRIDE;
		QMap<int, QVariant> itemData(const QModelIndex &index) const Q_DECL_OVERRIDE;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
		Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
		QSize span(const QModelIndex &index) const Q_DECL_OVERRIDE;

		// QAbstractProxyModel interface
		QModelIndex mapToSource(const QModelIndex &proxyIndex) const Q_DECL_OVERRIDE;
		QModelIndex mapFromSource(const QModelIndex &sourceIndex) const Q_DECL_OVERRIDE;

		// GroupedItemProxyModel interface

		/*! \property groupMatchRole Which data role to use for matching group data. Default is \c Qt::EditRole. \sa setGroupMatchRole() */
		inline int groupMatchRole() const { return m_groupMatchRole; }
		/*! \property groupColumnVisible Show the extra grouping column (eg. to allow sorting on it). Default is \c{true}. \sa setGroupColumnVisible() */
		inline int groupColumnVisible() const { return m_groupColumnVisible; }
		/*! \property groupColumnIsProxy Proxy data from source column to the extra grouping column (if shown). This includes display data. Default is \c{false}. \sa setGroupColumnIsProxy() */
		inline int groupColumnIsProxy() const { return m_groupColumnIsProxy; }
		/*! \property groupColumnProxySrc Which column to use for proxy data. A value of \{-1} (default) means to use the grouping column. \sa setGroupColumnProxySrc(), groupColumnIsProxy() */
		inline int groupColumnProxySrc() const { return m_groupColumnProxySrc; }
		/*! \property groupRowSelectable Allow selecting the grouping row. Default is \c{false} \sa setGroupRowSelectable() */
		inline int groupRowSelectable() const { return m_groupRowSelectable; }
		/*! \property groupHeaderTitle Title of grouping column. Default is \c "Group". Set an empty string to hide title. \sa setGroupHeaderTitle() */
		inline QString groupHeaderTitle() const { return m_root->data(Qt::DisplayRole).toString(); }

	public slots:
		/*! Add a new grouping based on \a column. \sa removeGroup() */
		inline void addGroup(int column) { addGroups(QVector<int>() << column); }
		/*! Add multiple groupings based on \a columns. The groups are appended to any existing group(s). \sa clearGroups() */
		virtual void addGroups(const QVector<int> & columns);
		/*! Add multiple groupings based on \a columns. Any existing group(s) are first cleared. \sa clearGroups() */
		virtual void setGroups(const QVector<int> & columns);
		/*! Insert a new grouping at \a index based on \a column. \sa addGroup(), removeGroup() */
		virtual void insertGroup(int index, int column);
		/*! Remove the previously-added \a column grouping. \sa addGroup() */
		virtual void removeGroup(int column);
		/*! Remove the previously-added groupings. \sa addGroup(), addGroups() */
		virtual void clearGroups();
		// properties
		virtual void setGroupMatchRole(int role);
		virtual void setGroupColumnVisible(bool visible) { m_groupColumnVisible = visible; }
		virtual void setGroupColumnIsProxy(bool enable)  { m_groupColumnIsProxy = enable; }
		virtual void setGroupColumnProxySrc(int column) { m_groupColumnProxySrc = column; }
		virtual void setGroupRowSelectable(bool selectable) { m_groupRowSelectable = selectable; }
		virtual void setGroupHeaderTitle(const QString &title, const QString &tooltip = QString());

	protected:
		class GroupedProxyItem;  // forward

		virtual uint extraColumns() const { return (!m_groups.isEmpty() && groupColumnVisible() ? 1 : 0); }
		virtual QModelIndex indexForItem(GroupedProxyItem *item, const int col = 0) const;
		virtual GroupedProxyItem * itemForIndex(const QModelIndex &index, const bool rootDefault = false) const;
		virtual GroupedProxyItem * findGroupItem(const int group, const QVariant &value, GroupedProxyItem *parent = NULL) const;
		virtual QModelIndex sourceIndexForProxy(GroupedProxyItem *item) const;
		virtual bool isProxyColumn(const QModelIndex &index) const { return (index.isValid() && extraColumns() && index.column() == 0 && groupColumnIsProxy()); }
		int totalRowCount(GroupedProxyItem *parent = NULL) const;
		GroupedProxyItem * itemForRow(int row, GroupedProxyItem *parent = NULL) const;

	protected slots:
		virtual GroupedProxyItem * placeSourceRow(const int row);
		virtual void removeItem(GroupedProxyItem *item);
		virtual void removeUnusedGroups(GroupedProxyItem *parent = NULL);
		virtual void reloadSourceModel();
		virtual void modelResetHandler();
		virtual void dataChangedHandler(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
		virtual void rowsInsertedHandler(const QModelIndex &parent, int first, int last);
		virtual void rowsRemovedHandler(const QModelIndex &parent, int first, int last);
		virtual void rowsMovedHandler(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);

	protected:
		GroupedProxyItem * m_root;
		QHash<int, GroupedProxyItem *> m_sourceMap;
		QVector<int> m_groups;
		int m_groupMatchRole;
		int m_groupColumnProxySrc;
		bool m_groupColumnVisible;
		bool m_groupColumnIsProxy;
		bool m_groupRowSelectable;

	private:
		bool setReloadSuspended(bool enable) { bool prev = m_reloadSuspended; m_reloadSuspended = enable; return prev; }
		bool reloadSuspended() const { return m_reloadSuspended; }

		bool m_reloadSuspended;


	protected:
		class GroupedProxyItem
		{
			public:
				explicit GroupedProxyItem(const QModelIndex &srcIndex, const bool isSrcItem = false, GroupedProxyItem *parent = NULL);
				explicit GroupedProxyItem(GroupedProxyItem *parent = NULL);
				virtual ~GroupedProxyItem();

				int row() const;
				QVariant data(int role = Qt::EditRole)    const { return m_itemData.value(role, QVariant()); }
				QMap<int, QVariant> itemData()            const { return m_itemData; }
				int rowCount()                            const { return m_childItems.size(); }
				QVector<GroupedProxyItem *> children()    const { return m_childItems; }
				GroupedProxyItem * child(const int row)   const { return m_childItems.value(row, NULL); }
				int childRow(GroupedProxyItem * child)    const { return m_childItems.indexOf(child); }
				GroupedProxyItem * parent()               const { return m_parentItem; }

				QModelIndex sourceIndex() const { return m_sourceIndex; }
				bool isSourceItem()       const { return m_isSourceItem; }
				bool isGroupItem()        const { return !isSourceItem(); }

				void clear();
				GroupedProxyItem * addChild(const QModelIndex &srcIndex, const bool isSrcItem = true);
				void removeChild(GroupedProxyItem * item);
				void removeChild(const int row) { removeChild(child(row)); }
				bool setData(const QVariant &data, int role = Qt::EditRole);
				bool setItemData(const QMap<int, QVariant> &roles);

			protected:
				GroupedProxyItem * m_parentItem;
				QVector<GroupedProxyItem *> m_childItems;
				QMap<int, QVariant> m_itemData;
				QPersistentModelIndex m_sourceIndex;
				bool m_isSourceItem;
		};  // GroupedProxyItem
		\
};  // GroupedItemsProxyModel

#endif // GROUPEDITEMSPROXYMODEL_H
