/*
	GroupedItemsProxyModel

	COPYRIGHT: (c)2017 Maxim Paperno; All Right Reserved.
	Contact: http://www.WorldDesign.com/contact

	LICENSE:

	Commercial License Usage
	Licensees holding valid commercial licenses may use this file in
	accordance with the terms contained in a written agreement between
	you and Maxim Paperno/World Design Group.

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

#include "GroupedItemsProxyModel.h"

#include <QDebug>
#include <QQueue>

/**
	 GroupedProxyItem
 */

GroupedItemsProxyModel::GroupedProxyItem::GroupedProxyItem(const QModelIndex &srcIndex, const bool isSrcItem, GroupedProxyItem *parent) :
  m_parentItem(parent),
  m_sourceIndex(srcIndex),
  m_isSourceItem(isSrcItem)
{
}

GroupedItemsProxyModel::GroupedProxyItem::GroupedProxyItem(GroupedProxyItem *parent) :
  GroupedProxyItem(QModelIndex(), false, parent)
{
}

GroupedItemsProxyModel::GroupedProxyItem::~GroupedProxyItem()
{
	clear();
}

void GroupedItemsProxyModel::GroupedProxyItem::clear()
{
	qDeleteAll(m_childItems);
	m_childItems.clear();
}

int GroupedItemsProxyModel::GroupedProxyItem::row() const
{
	if (parent())
		return parent()->childRow(const_cast<GroupedProxyItem *>(this));

	return -1;
}

GroupedItemsProxyModel::GroupedProxyItem * GroupedItemsProxyModel::GroupedProxyItem::addChild(const QModelIndex &srcIndex, const bool isSrcItem)
{
	GroupedProxyItem * item = new GroupedProxyItem(srcIndex, isSrcItem, const_cast<GroupedProxyItem *>(this));
	m_childItems.append(item);
	return item;
}

void GroupedItemsProxyModel::GroupedProxyItem::removeChild(GroupedProxyItem *item)
{
	if (item) {
		m_childItems.removeAll(item);
		delete item;
	}
}

bool GroupedItemsProxyModel::GroupedProxyItem::setData(const QVariant &data, int role)
{
	m_itemData.insert(role, data);
	return true;
}

bool GroupedItemsProxyModel::GroupedProxyItem::setItemData(const QMap<int, QVariant> &roles)
{
	bool b = true;
	for (QMap<int, QVariant>::ConstIterator it = roles.begin(); it != roles.end(); ++it)
		b = b && setData(it.value(), it.key());
	return b;
}


/**
	 GroupedItemProxyModel
 */

GroupedItemsProxyModel::GroupedItemsProxyModel(QObject *parent, QAbstractItemModel *sourceModel, const QVector<int> &groupColumns) :
  QIdentityProxyModel(parent),
  m_root(new GroupedProxyItem())
{
	m_root->setData(-1, Qt::EditRole);

	setGroupMatchRole(Qt::EditRole);
	setGroupColumnVisible(true);
	setGroupColumnIsProxy(false);
	setGroupColumnProxySrc(-1);
	setGroupRowSelectable(false);
	setGroupHeaderTitle(tr("Grouping"), tr("This column shows item group information."));

	if (sourceModel) {
		setReloadSuspended(!groupColumns.isEmpty());
		setSourceModel(sourceModel);
		setReloadSuspended(false);
		if (!groupColumns.isEmpty())
			setGroups(groupColumns);
		else
			reloadSourceModel();
	}
}

GroupedItemsProxyModel::~GroupedItemsProxyModel()
{
	delete m_root;
}

void GroupedItemsProxyModel::setSourceModel(QAbstractItemModel *newSourceModel)
{
	if (sourceModel()) {
		disconnect(sourceModel(), 0, this, 0);
	}

	QIdentityProxyModel::setSourceModel(newSourceModel);
	reloadSourceModel();

	if (sourceModel()) {
		connect(sourceModel(), &QAbstractItemModel::modelReset, this, &GroupedItemsProxyModel::modelResetHandler);
		connect(sourceModel(), &QAbstractItemModel::dataChanged, this, &GroupedItemsProxyModel::dataChangedHandler);
		connect(sourceModel(), &QAbstractItemModel::rowsInserted, this, &GroupedItemsProxyModel::rowsInsertedHandler);
		connect(sourceModel(), &QAbstractItemModel::rowsRemoved, this, &GroupedItemsProxyModel::rowsRemovedHandler);
		connect(sourceModel(), &QAbstractItemModel::rowsMoved, this, &GroupedItemsProxyModel::rowsMovedHandler);
	}
}

QModelIndex GroupedItemsProxyModel::index(int row, int column, const QModelIndex &parent) const
{
	if (GroupedProxyItem * childItem = itemForIndex(parent, true)->child(row))
		return indexForItem(childItem, column);

	return QModelIndex();
}

QModelIndex GroupedItemsProxyModel::parent(const QModelIndex &child) const
{
	GroupedProxyItem * childItem = itemForIndex(child);
	if (!childItem)
		return QModelIndex();

	GroupedProxyItem * parentItem = childItem->parent();
	if (!parentItem || parentItem == m_root)
		return QModelIndex();

	return indexForItem(parentItem);
}

int GroupedItemsProxyModel::rowCount(const QModelIndex &parent) const
{
	return itemForIndex(parent, true)->rowCount();
}

int GroupedItemsProxyModel::columnCount(const QModelIndex &parent) const
{
	return sourceModel()->columnCount(parent) + extraColumns();
}

bool GroupedItemsProxyModel::hasChildren(const QModelIndex &parent) const
{
	return (rowCount(parent) > 0);
}

QVariant GroupedItemsProxyModel::data(const QModelIndex &index, int role) const
{
	GroupedProxyItem * item = itemForIndex(index);
	if (!item)
		return QVariant();

	if (role == SourceRowNumberRole)
		return headerData(index.row(), Qt::Vertical, Qt::DisplayRole);

	if (item->isGroupItem() && index.column() == 0)
		return sourceModel()->data(item->sourceIndex(), role);

	if (isProxyColumn(index))
		return sourceModel()->data(sourceIndexForProxy(item), role);

	return sourceModel()->data(mapToSource(index), role);
}

QMap<int, QVariant> GroupedItemsProxyModel::itemData(const QModelIndex &index) const
{
	GroupedProxyItem * item = itemForIndex(index);
	if (!item)
		return QMap<int, QVariant>();

	if (item->isGroupItem() && index.column() == 0)
		return sourceModel()->itemData(item->sourceIndex());

	if (isProxyColumn(index))
		return sourceModel()->itemData(sourceIndexForProxy(item));

	return sourceModel()->itemData(mapToSource(index));
}

QVariant GroupedItemsProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal) {
		if (extraColumns() && section == 0 && (role == Qt::DisplayRole || role == Qt::ToolTipRole))
			return m_root->data(role);

		return sourceModel()->headerData(section - extraColumns(), orientation, role);
	}
	if (orientation == Qt::Vertical) {
		GroupedProxyItem * item = itemForRow(section);
		if (item  && item->isSourceItem())
			return sourceModel()->headerData(section, orientation, role);
	}
	return QVariant();
}

Qt::ItemFlags GroupedItemsProxyModel::flags(const QModelIndex &index) const
{
	GroupedProxyItem * item = itemForIndex(index);
	if (!item)
		return Qt::NoItemFlags;

	if (item->isGroupItem()) {
		if (groupRowSelectable())
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		else
			return Qt::ItemIsEnabled;
	}

	if (isProxyColumn(index))
		return (sourceModel()->flags(sourceIndexForProxy(item)) & ~Qt::ItemIsEditable);

	return sourceModel()->flags(mapToSource(index));
}

QSize GroupedItemsProxyModel::span(const QModelIndex & index) const
{
	GroupedProxyItem * item = itemForIndex(index);
	if (!item)
		return QSize();

	if (item->isSourceItem())
		return sourceModel()->span(mapToSource(index));

	return QSize(columnCount(), 1);
}

QModelIndex GroupedItemsProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
	GroupedProxyItem * item = itemForIndex(proxyIndex, false);
	if (item && item->isSourceItem()) {
		//qDebug() << proxyIndex << item->sourceIndex << sourceModel()->index(item->sourceRow(), proxyIndex.column() - extraColumns());
		return sourceModel()->index(item->sourceIndex().row(), proxyIndex.column() - extraColumns());
	}

	return QModelIndex();
}

QModelIndex GroupedItemsProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
	GroupedProxyItem * item = NULL;
	if (m_sourceMap.contains(sourceIndex.row()) && (item = m_sourceMap.value(sourceIndex.row()))) {
		//qDebug() << sourceIndex << indexForItem(item, sourceIndex.column() + extraColumns());
		return indexForItem(item, sourceIndex.column() + extraColumns());
	}

	return QModelIndex();
}

void GroupedItemsProxyModel::addGroups(const QVector<int> &columns)
{
	if (columns.isEmpty())
		return;
	bool prev = setReloadSuspended(true);
	for (int i : columns)
		insertGroup(m_groups.size(), i);
	setReloadSuspended(prev);
	reloadSourceModel();
}

void GroupedItemsProxyModel::setGroups(const QVector<int> &columns)
{
	if (columns.isEmpty()) {
		clearGroups();
		return;
	}
	bool prev = setReloadSuspended(true);
	clearGroups();
	addGroups(columns);
	setReloadSuspended(prev);
	reloadSourceModel();
}

void GroupedItemsProxyModel::insertGroup(int index, int column)
{
	index = qBound(0, index, m_groups.size());
	if (m_groups.indexOf(column) == -1) {
		m_groups.insert(index, column);
		reloadSourceModel();
	}
}

void GroupedItemsProxyModel::removeGroup(int column)
{
	if (m_groups.removeAll(column))
		reloadSourceModel();
}

void GroupedItemsProxyModel::clearGroups()
{
	if (m_groups.isEmpty())
		return;
	bool prev = setReloadSuspended(true);
	for (int i : m_groups)
		removeGroup(i);
	setReloadSuspended(prev);
	reloadSourceModel();
}

void GroupedItemsProxyModel::setGroupMatchRole(int role)
{
	if (m_groupMatchRole != role) {
		m_groupMatchRole = role;
		if (sourceModel() && !m_groups.isEmpty())
			reloadSourceModel();
	}
}

void GroupedItemsProxyModel::setGroupHeaderTitle(const QString &title, const QString &tooltip) {
	m_root->setData(title, Qt::DisplayRole);
	if (!tooltip.isEmpty())
		m_root->setData(tooltip, Qt::ToolTipRole);
	else
		m_root->setData(title, Qt::ToolTipRole);
	if (extraColumns())
		emit headerDataChanged(Qt::Horizontal, 0, 0);
}


//
// protected methods
//

QModelIndex GroupedItemsProxyModel::indexForItem(GroupedProxyItem *item, const int col) const
{
	if (!item || item->row() < 0 || col < 0)
		return QModelIndex();

	return createIndex(item->row(), col, item);
}

GroupedItemsProxyModel::GroupedProxyItem * GroupedItemsProxyModel::itemForIndex(const QModelIndex &index, const bool rootDefault) const
{
	GroupedProxyItem * item;
	if (index.isValid() && (item = static_cast<GroupedProxyItem *>(index.internalPointer())))
		return item;
	if (rootDefault)
		return m_root;
	else
		return NULL;
}

GroupedItemsProxyModel::GroupedProxyItem * GroupedItemsProxyModel::findGroupItem(const int group, const QVariant &value, GroupedProxyItem *parent) const
{
	if (!parent)
		parent = m_root;
	foreach (GroupedProxyItem * item, parent->children()) {
		if (!item)
			continue;
		if (!item->isSourceItem() && item->data(Qt::UserRole).toInt() == group && item->data(Qt::EditRole) == value)
			return item;
		else if (item->rowCount() && (item = findGroupItem(group, value, item)))
			return item;
	}
	return NULL;
}

QModelIndex GroupedItemsProxyModel::sourceIndexForProxy(GroupedItemsProxyModel::GroupedProxyItem *item) const
{
	QModelIndex srcIdx = item->sourceIndex();
	if (groupColumnProxySrc() > -1)
		srcIdx = sourceModel()->index(srcIdx.row(), groupColumnProxySrc(), srcIdx.parent());
	return srcIdx;
}

int GroupedItemsProxyModel::totalRowCount(GroupedProxyItem *parent) const
{
	if (!parent)
		parent = m_root;
	int count = 0;
	foreach (GroupedProxyItem * item, parent->children()) {
		++count;
		if (item->isGroupItem())
			count += totalRowCount(item);
	}
	return count;
}

GroupedItemsProxyModel::GroupedProxyItem * GroupedItemsProxyModel::itemForRow(int row, GroupedProxyItem *parent) const
{
	GroupedProxyItem * ret = NULL;
	if (!parent)
		parent = m_root;
	QQueue<GroupedProxyItem *> q;
	q.enqueue(parent);
	int count = 0;
	while (!q.isEmpty()) {
		GroupedProxyItem * p = q.dequeue();
		foreach (GroupedProxyItem * item, p->children()) {
			if (count++ == row)
				return item;
			if (item->rowCount())
				q.enqueue(item);
		}
	}
	return ret;
}

GroupedItemsProxyModel::GroupedProxyItem * GroupedItemsProxyModel::placeSourceRow(const int row)
{
	GroupedProxyItem * grpParent = m_root;
	QModelIndex sourceIndex = sourceModel()->index(row, 0);;
	for (int col : m_groups) {
		sourceIndex = sourceModel()->index(row, col);
		QVariant val = sourceIndex.data(groupMatchRole());
		GroupedProxyItem * grpItem;
		if (!(grpItem = findGroupItem(col, val, grpParent))) {
			QAbstractItemModel::beginInsertRows(indexForItem(grpParent), grpParent->rowCount(), grpParent->rowCount());
			grpItem = grpParent->addChild(sourceIndex, false);
			grpItem->setData(col, Qt::UserRole);
			grpItem->setData(val, Qt::EditRole);
			QAbstractItemModel::endInsertRows();
		}
		grpParent = grpItem;
	}
	QAbstractItemModel::beginInsertRows(indexForItem(grpParent), grpParent->rowCount(), grpParent->rowCount());
	GroupedProxyItem * newItem = grpParent->addChild(sourceIndex);
	m_sourceMap.insert(row, newItem);
	QAbstractItemModel::endInsertRows();

	return newItem;
}

void GroupedItemsProxyModel::removeItem(GroupedProxyItem *item)
{
	if (!item || !item->parent())
		return;

	GroupedProxyItem * parent = item->parent();
	QAbstractItemModel::beginRemoveRows(indexForItem(parent), item->row(), item->row());
	if (item->isSourceItem())
		m_sourceMap.remove(item->sourceIndex().row());
	parent->removeChild(item);
	QAbstractItemModel::endRemoveRows();
}

void GroupedItemsProxyModel::removeUnusedGroups(GroupedProxyItem *parent)
{
	if (!parent)
		parent = m_root;
	foreach (GroupedProxyItem * item, parent->children()) {
		if (item->isSourceItem())
			continue;
		if (item->rowCount())
			removeUnusedGroups(item);
		if (!item->rowCount())
			removeItem(item);
	}
}

void GroupedItemsProxyModel::reloadSourceModel()
{
	if (reloadSuspended())
		return;
	beginResetModel();
	m_root->clear();
	m_sourceMap.clear();
	for (int row = 0, e = sourceModel()->rowCount(); row < e; ++row)
		placeSourceRow(row);
	endResetModel();
}

void GroupedItemsProxyModel::modelResetHandler()
{
	reloadSourceModel();
}

void GroupedItemsProxyModel::dataChangedHandler(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
	//qDebug() << topLeft << bottomRight << roles;
	if (!topLeft.isValid() || !bottomRight.isValid() || m_groups.isEmpty() || (!roles.isEmpty() && !roles.contains(groupMatchRole())))
		return;

	const QModelIndex & srcParent = topLeft.parent();
	int endRow = qMin(bottomRight.row(), sourceModel()->rowCount());
	int startCol = qMax(topLeft.column(), 0);
	int endCol = qMin(bottomRight.column(), sourceModel()->columnCount() - 1);
	bool modified = false;
	for (int row = topLeft.row(); row <= endRow; ++row) {
		for (int col : m_groups) {
			if (startCol < col || endCol > col)
				continue;  // not a column we care about

			const QModelIndex & srcIdx = sourceModel()->index(row, col, srcParent);
			GroupedProxyItem * currItem = itemForIndex(mapFromSource(srcIdx), false);
			if (!currItem) {
				// source model is out of sync (shouldn't happen)
				reloadSourceModel();
				return;
			}
			GroupedProxyItem * currParent = currItem->parent();
			GroupedProxyItem * newParent = findGroupItem(col, srcIdx.data(groupMatchRole()));
			if (newParent && currParent == newParent)
				continue;  // parent group hasn't changed

			// grouping value has changed
			removeItem(currItem);
			placeSourceRow(row);
			modified = true;
			break;
		}
	}
	if (modified)
		removeUnusedGroups();
}

void GroupedItemsProxyModel::rowsInsertedHandler(const QModelIndex &parent, int first, int last)
{
	Q_UNUSED(parent)
	for (int row = first; row <= last; ++row)
		placeSourceRow(row);
}

void GroupedItemsProxyModel::rowsRemovedHandler(const QModelIndex &parent, int first, int last)
{
	for (int row = first; row <= last; ++row) {
		GroupedProxyItem * currItem = m_sourceMap.value(row, NULL);
		if (!currItem || !currItem->parent()) {
			// source model is out of sync (shouldn't happen)
			reloadSourceModel();
			return;
		}
		removeItem(currItem);
	}
	removeUnusedGroups();
}

void GroupedItemsProxyModel::rowsMovedHandler(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row)
{
	rowsRemovedHandler(parent, start, end);
	rowsInsertedHandler(destination.parent(), row, row + (end - start));
}
