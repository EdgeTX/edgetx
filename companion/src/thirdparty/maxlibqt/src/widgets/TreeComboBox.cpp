/*
	TreeComboBox

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

#include "TreeComboBox.h"

#include <QAccessible>
#include <QGuiApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QStack>
#include <QStandardItemModel>

/*
   TreeComboItemDelegate
*/

// static
bool TreeComboItemDelegate::isSeparator(const QModelIndex &index) {
	return index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator");
}

// static
bool TreeComboItemDelegate::isParent(const QModelIndex &index) {
	return index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("parent");
}

// static
void TreeComboItemDelegate::setSeparator(QAbstractItemModel *model, const QModelIndex &index) {
	model->setData(index, QString::fromLatin1("separator"), Qt::AccessibleDescriptionRole);
	if (QStandardItemModel *m = qobject_cast<QStandardItemModel*>(model))
		if (QStandardItem *item = m->itemFromIndex(index))
			item->setFlags(item->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled));
}

// static
void TreeComboItemDelegate::setParent(QAbstractItemModel *model, const QModelIndex &index, const bool selectable) {
	model->setData(index, QString::fromLatin1("parent"), Qt::AccessibleDescriptionRole);
	if (!selectable) {
		if (QStandardItemModel *m = qobject_cast<QStandardItemModel*>(model))
			if (QStandardItem * item = m->itemFromIndex(index))
				item->setFlags(item->flags() & ~(Qt::ItemIsSelectable));
	}
}

void TreeComboItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (isSeparator(index)) {
		QPen pen = painter->pen();
		pen.setColor(option.palette.color(QPalette::Active, QPalette::Dark));
		pen.setWidthF(1.5);
		painter->save();
		painter->setPen(pen);
		painter->setRenderHint(QPainter::HighQualityAntialiasing);
		painter->drawLine(option.rect.left(), option.rect.center().y(), option.rect.right(), option.rect.center().y());
		painter->restore();
		return;
	}
	else if (isParent(index)) {
		QStyleOptionViewItem opt = option;
		opt.state |= QStyle::State_Enabled;  // make sure it looks enabled even if it isn't
		opt.font.setBold(true);
		opt.font.setItalic(true);
		QStyledItemDelegate::paint(painter, opt, index);
		return;
	}

	QStyledItemDelegate::paint(painter, option, index);
}

QSize TreeComboItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(index.data(Qt::AccessibleDescriptionRole).toString() == QLatin1String("separator"))
		return QSize(0, 5);
	return QStyledItemDelegate::sizeHint(option, index);
}


/*
   TreeComboBox
*/

TreeComboBox::TreeComboBox(QWidget *parent):
  QComboBox(parent),
  m_view(NULL),
  m_nextDataId(0),
  m_autoData(true),
  m_skipNextHide(false)
{
	setView();
	setItemDelegate(new TreeComboItemDelegate(this));
	setSizeAdjustPolicy(AdjustToContents);
	setMaxVisibleItems(30);
}

void TreeComboBox::setView(QAbstractItemView *itemView)
{
	if (view()) {
		view()->removeEventFilter(this);
		view()->viewport()->removeEventFilter(this);
		disconnect(view(), 0, this, 0);
	}

	QTreeView * treeView = NULL;
	if (!itemView || !(treeView = qobject_cast<QTreeView *>(itemView))) {
		if (!m_view)
			m_view = new TreeComboBoxView;
		if (itemView && !treeView)
			qWarning() << "TreeComboBox: Only QTreeView and derived classes are allowed, using default view instead.";
	}
	else {
		m_view = treeView;
	}

	m_view->setItemsExpandable(true);
	m_view->setAllColumnsShowFocus(true);
	m_view->setHeaderHidden(true);
	m_view->setWordWrap(false);
	m_view->setIndentation(m_view->indentation() * 0.5);
	m_view->setSelectionBehavior(QTreeView::SelectRows);
	m_view->setSizeAdjustPolicy(QAbstractItemView::AdjustToContents);

	QComboBox::setView(m_view);
	view()->installEventFilter(this);
	view()->viewport()->installEventFilter(this);
	connect(view(), &QTreeView::expanded, this, &TreeComboBox::adjustPopupWidth);
	connect(view(), &QTreeView::expanded, this, &TreeComboBox::adjustPopupHeight);
	connect(view(), &QTreeView::collapsed, this, &TreeComboBox::adjustPopupHeight);
}

void TreeComboBox::setModel(QAbstractItemModel *model)
{
	if (this->model() == model)
		return;
	if (this->model())
		disconnect(this->model(), 0, this, 0);

	QComboBox::setModel(model);
	reloadModel();

	connect(model, &QAbstractItemModel::dataChanged, this, &TreeComboBox::reloadModel);
	connect(model, &QAbstractItemModel::rowsInserted, this, &TreeComboBox::reloadModel);
	connect(model, &QAbstractItemModel::rowsRemoved, this, &TreeComboBox::reloadModel);
	connect(model, &QAbstractItemModel::modelReset, this, &TreeComboBox::reloadModel);
}

QModelIndex TreeComboBox::insertItem(int index, const QMap<int, QVariant> &values, const QModelIndex &parentIndex, const bool reload)
{
	QModelIndex ret;
	QModelIndex root = parentIndex;
	if (!root.isValid())
		root = view()->rootIndex();

	index = qBound(0, index, model()->rowCount(root));

	bool blocked = model()->signalsBlocked();
	if (!reload)
		model()->blockSignals(true);

	if (QStandardItemModel *m = qobject_cast<QStandardItemModel*>(model())) {
		QStandardItem * item = new QStandardItem();
		QMapIterator<int, QVariant> i(values);
		while (i.hasNext()) {
			i.next();
			item->setData(i.value(), i.key());
		}
		if (root.isValid() && m->itemFromIndex(root))
			m->itemFromIndex(root)->insertRow(index, item);
		else
			m->insertRow(index, item);
		ret = item->index();
	}
	else if (model()->insertRows(index, 1, root)) {
		ret = model()->index(index, modelColumn(), root);
		if (!values.isEmpty())
			model()->setItemData(ret, values);
	}
	model()->blockSignals(blocked);

	return ret;
}

QModelIndex TreeComboBox::insertItem(int index, const QIcon &icon, const QString &text, const QModelIndex &parentIndex, const QVariant &userData, const bool reload)
{
	QMap<int, QVariant> values;
	if (!text.isNull())
		values.insert(Qt::EditRole, text);
	if (!icon.isNull())
		values.insert(Qt::DecorationRole, icon);
	if (userData.isValid())
		values.insert(Qt::UserRole, userData);
	else if (autoData())
		values.insert(Qt::UserRole, m_nextDataId++);

	return insertItem(index, values, parentIndex, reload);
}

void TreeComboBox::insertItems(int index, const QStringList &texts, const QModelIndex &parentIndex)
{
	for (const QString &text : texts)
		insertItem(index++, QIcon(), text, parentIndex, QVariant(), (text == texts.last()));
}

QModelIndex TreeComboBox::insertParentItem(int index, const QIcon &icon, const QString &text, const bool selectable, const QModelIndex &parentIndex, const QVariant &userData)
{
	bool oldAuto = autoData();
	m_autoData = selectable;
	QModelIndex idx = insertItem(index, icon, text, parentIndex, userData);
	m_autoData = oldAuto;
	TreeComboItemDelegate::setParent(model(), idx, selectable);
	return idx;
}

void TreeComboBox::clear()
{
	setModel(new QStandardItemModel(0, 1, this));
}

int TreeComboBox::currentIndex() const
{
	if (m_rowMap.contains(view()->currentIndex()))
		return m_rowMap.value(view()->currentIndex());

	return -1;
}

void TreeComboBox::setCurrentIndex(int index)
{
	setCurrentIndex(m_indexMap.value(index, QModelIndex()));
}

void TreeComboBox::setCurrentIndex(const QModelIndex &index)
{
	//qDebug() << index << m_currentIndex;
	if (!index.isValid() || !(index.flags() & Qt::ItemIsSelectable)) {
		QComboBox::setCurrentIndex(-1);
		m_currentIndex = QPersistentModelIndex();
		return;
	}
	const QModelIndex & root = view()->rootIndex();  // save actual root
	const bool blocked = blockSignals(true);         // we handle all signals
	setRootModelIndex(model()->parent(index));       // set dummy root so that row() sets correct item under parent (if any)
	QComboBox::setCurrentIndex(index.row());
	setRootModelIndex(root);                         // reset so that full tree is visible again
	blockSignals(blocked);

	// set view index after QComboBox::setCurrentIndex call
	view()->setCurrentIndex(index);

	if (m_currentIndex == index)
		return;

	m_currentIndex = QPersistentModelIndex(index);
	// We handle all signals ourselves because QComboBox doesn't always detect change between parent rows (eg. parentA.row0 and parentB.row0)
	const QString text = model()->data(index).toString();
	const QVariant currData = currentData();
	emit currentIndexChanged(m_rowMap.value(index));
	emit currentIndexChanged(text);
	emit currentModelIndexChanged(index);
	if (!isEditable())
		emit currentTextChanged(text);
	if (currData.isValid())
		emit currentDataChanged(currData);
#ifndef QT_NO_ACCESSIBILITY
	QAccessibleValueChangeEvent event(this, text);
	QAccessible::updateAccessibility(&event);
#endif
}

int TreeComboBox::setCurrentData(const QVariant &data, const int defaultIdx, int role, Qt::MatchFlags flags)
{
	int idx = findData(data, role, flags);
	if (idx == -1)
		idx = defaultIdx;
	setCurrentIndex(idx);
	return idx;
}

QVariant TreeComboBox::currentData(int role) const
{
	return model()->data(view()->currentIndex(), role);
}

QVariant TreeComboBox::itemData(int index, int role) const
{
	if (m_indexMap.contains(index))
		return model()->data(m_indexMap.value(index), role);

	return QVariant();
}

QIcon TreeComboBox::itemIcon(int index) const
{
	QVariant decoration = itemData(index, Qt::DecorationRole);
	if (decoration.type() == QVariant::Pixmap)
		return QIcon(qvariant_cast<QPixmap>(decoration));
	else if (decoration.type() == QVariant::Icon)
		return qvariant_cast<QIcon>(decoration);
	else
		return QIcon();
}

void TreeComboBox::setItemData(const QModelIndex & index, const QVariant & value, int role)
{
	if (index.isValid())
		model()->setData(index, value, role);
}

int TreeComboBox::findData(const QVariant &data, int role, Qt::MatchFlags flags) const
{
	const QModelIndex & mi = model()->index(0, modelColumn(), view()->rootIndex());
	QModelIndexList result = model()->match(mi, role, data, -1, (flags | Qt::MatchRecursive));
	while (!result.isEmpty()) {
		const QModelIndex i = result.takeFirst();
		if ((i.flags() & Qt::ItemIsSelectable) && m_rowMap.contains(i))
			return m_rowMap.value(i);
	}

	return -1;
}

QModelIndex TreeComboBox::indexAbove(QModelIndex index)
{
	do {
		index = view()->indexAbove(index);
		if (model()->hasChildren(index) && !view()->isExpanded(index)) {
			view()->setExpanded(index, true);
			index = lastIndex(index);
		}
	} while (index.isValid() && !(model()->flags(index) & Qt::ItemIsSelectable));
	return index;
}

QModelIndex TreeComboBox::indexBelow(QModelIndex index)
{
	do {
		if (model()->hasChildren(index))
			view()->setExpanded(index, true);
		index = view()->indexBelow(index);
	} while (index.isValid() && !(model()->flags(index) & Qt::ItemIsSelectable));
	return index;
}

QModelIndex TreeComboBox::lastIndex(const QModelIndex &index)
{
	if (index.isValid() && !view()->isExpanded(index))
		return index;
	int rows = view()->model()->rowCount(index);
	if (rows == 0)
		return index;
	return lastIndex(view()->model()->index(rows - 1, modelColumn(), index));
}

void TreeComboBox::keyboardSearchString(const QString &text)
{
	view()->keyboardSearch(text);
	QModelIndex index = view()->currentIndex();
	if (index.isValid() && !(model()->flags(index) & Qt::ItemIsSelectable))
		index = indexBelow(index);
	//qDebug() << index;
	if (index.isValid())
		setCurrentIndex(index);
}

void TreeComboBox::wheelEvent(QWheelEvent *event)
{
	QModelIndex index = m_view->currentIndex();
	if (event->delta() > 0)
		index = indexAbove(index);
	else if (event->delta() < 0)
		index = indexBelow(index);

	event->accept();
	if (!index.isValid())
		return;

	setCurrentIndex(index);
	emit activated(m_rowMap.value(index));
}

void TreeComboBox::keyPressEvent(QKeyEvent *event)
{
	if (isEditable() || (event->modifiers() & Qt::ControlModifier)) {
		QComboBox::keyPressEvent(event);
		return; // pass to line edit
	}

	QModelIndex index = m_view->currentIndex();
	switch (event->key()) {
		case Qt::Key_Up:
		case Qt::Key_PageUp:
			index = indexAbove(index);
			break;
		case Qt::Key_Down:
		case Qt::Key_PageDown:
			if (event->modifiers() & Qt::AltModifier) {
				showPopup();
				return;
			}
			index = indexBelow(index);
			break;
		case Qt::Key_Home:
			index = m_view->model()->index(0, modelColumn());
			if (index.isValid() && !(model()->flags(index) & Qt::ItemIsSelectable))
				index = indexBelow(index);
			break;
		case Qt::Key_End:
			index = lastIndex(m_view->rootIndex());
			if (index.isValid() && !(model()->flags(index) & Qt::ItemIsSelectable))
				index = indexAbove(index);
			break;

		case Qt::Key_F4:
		case Qt::Key_Space:
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Select:
		case Qt::Key_Left:
		case Qt::Key_Right:
		case Qt::Key_Back:
			QComboBox::keyPressEvent(event);
			return;

		default:
			if (!event->text().isEmpty())
				keyboardSearchString(event->text());
			else
				event->ignore();
			return;
	}
	if (index.isValid()) {
		setCurrentIndex(index);
		event->accept();
	}
}

bool TreeComboBox::eventFilter(QObject *object, QEvent *event)
{
	if (object == view()->viewport()) {
		if (event->type() == QEvent::MouseButtonRelease) {
			// mouse release filter to keep combo box open when expanding/collapsing groups
			m_skipNextHide = false;
			QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
			const QModelIndex index = view()->indexAt(mouseEvent->pos());
			if (!index.isValid())
				return false;\
			if (!view()->visualRect(index).contains(mouseEvent->pos()) || !(index.flags() & Qt::ItemIsSelectable)) {
				// do not hide popup if clicking on expander icon or a non-selectable item
				m_skipNextHide = true;
				if (model()->hasChildren(index) && view()->visualRect(index).contains(mouseEvent->pos())) {
					// toggle expanded state if clicking on parent label
					view()->setExpanded(index, !view()->isExpanded(index));
				}
				return true;
			}
		}
		else if (event->type() == QEvent::MouseMove) {
			// prevent QComboBox filter which auto-selects any item under the cursor
			return true;
		}
	}
	else if (object == view()) {
		if (event->type() == QEvent::ShortcutOverride) {
			// override handling of keyboard selection events within tree view
			QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
			const QModelIndex & index = view()->currentIndex();

			switch (keyEvent->key()) {
				case Qt::Key_Enter:
				case Qt::Key_Return:
				case Qt::Key_Select:
					if (!index.isValid())
						break;
					if (model()->hasChildren(index) && !(index.flags() & Qt::ItemIsSelectable)) {
						m_skipNextHide = true;
						view()->setExpanded(index, !view()->isExpanded(index));
					}
					else {
						m_skipNextHide = false;
						setCurrentIndex(index);
						hidePopup();
					}
					return true;

				default:
					break;
			}
		}
	}
	return false;
}

void TreeComboBox::showPopup() {
	ensureCurrentExpanded();
	adjustPopupWidth();
	QComboBox::showPopup();
}

void TreeComboBox::hidePopup()
{
	if (m_skipNextHide)
		m_skipNextHide = false;
	else
		QComboBox::hidePopup();
}

int TreeComboBox::buildMap(QModelIndex parent, int row)
{
	if (!row) {
		m_rowMap.clear();
		m_indexMap.clear();
	}
	int rows = model()->rowCount(parent);
	for (int r=0; r < rows; ++r) {
		QPersistentModelIndex index = model()->index(r, 0, parent);
		if (index.flags() & Qt::ItemIsSelectable) {
			m_rowMap.insert(index, row);
			m_indexMap.insert(row, index);
			++row;
		}
		if (model()->hasChildren(index))
			row = buildMap(index, row);
	}
	return row;
}

void TreeComboBox::reloadModel()
{
	buildMap();
	m_nextDataId = m_rowMap.size();

	// simplify the tree view for flat models, or if only one parent item
	QModelIndexList parents;
	int rows = model()->rowCount();
	for (int r=0; r < rows; ++r) {
		QModelIndex index = model()->index(r, modelColumn());
		if (model()->hasChildren(index)) {
			parents << index;
			if (parents.size() > 1)
				break;
		}
	}
	if (!parents.size()) {
		// if no parents then remove space for arrows
		view()->setRootIsDecorated(false);
	}
	else if (parents.size() == 1 && parents.first() == model()->index(0, modelColumn())) {
		// if only one parent then set it as root
		view()->setRootIndex(parents.first());
	}
	else {
		view()->setRootIndex(model()->index(-1, modelColumn()));
		view()->setRootIsDecorated(true);
	}

	// set minium width based on tree view width
	ensureCurrentExpanded();
	adjustPopupWidth();
	setMinimumWidth(view()->minimumWidth() + 16);
}

void TreeComboBox::ensureCurrentExpanded()
{
	QModelIndex p = model()->parent(view()->currentIndex());
	if (p.isValid() && !view()->isExpanded(p)) {
		bool blocked = view()->blockSignals(true);
		view()->setExpanded(p, true);
		view()->blockSignals(blocked);
	}
}

bool TreeComboBox::usePopup(QStyleOptionComboBox *option)
{
	QStyleOptionComboBox opt;
	initStyleOption(&opt);
	if (option)
		*option = opt;
	return style()->styleHint(QStyle::SH_ComboBox_Popup, &opt, this);
}

void TreeComboBox::adjustPopupWidth()
{
	if (qobject_cast<TreeComboBoxView *>(view())) {
		qobject_cast<TreeComboBoxView *>(view())->adjustWidth(topLevelWidget()->geometry().width());
	}
	else {
		view()->resizeColumnToContents(0);
		view()->setMaximumWidth(topLevelWidget()->geometry().width());
		view()->setMinimumWidth(view()->minimumSizeHint().width() + view()->verticalScrollBar()->sizeHint().width() + view()->indentation());
	}
}

void TreeComboBox::adjustPopupHeight()
{
	QStyleOptionComboBox opt;
	const bool usePopup = this->usePopup(&opt);
	if (!(opt.state & QStyle::State_On))
		return;   // popup is hidden

	const bool boundToScreen = !window()->testAttribute(Qt::WA_DontShowOnScreen);
	const QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
	const int viewH = view()->sizeHint().height();
	QWidget * container = view()->parentWidget();
	QRect listRect = container->geometry();  // QComboBox sets geometry on it's private container to control the selector view size

	// set desired adjustment (delta) size
	int adjH = viewH - listRect.size().height();
	//qDebug() << "listRect" << listRect << "adjH" << adjH << "viewH" << viewH << "screen" << screen << "popup" << usePopup << "bound" << boundToScreen;
	if (!adjH)
		return;

	const int algnTop = parentWidget()->mapToGlobal(frameGeometry().topLeft()).y();
	const int algnBot = parentWidget()->mapToGlobal(frameGeometry().bottomLeft()).y();
	const int aboveHeight = algnTop - screen.y();
	const int belowHeight = screen.bottom() - algnBot;

	if (usePopup)
		adjH += 15;  // otherwise scroll buttons cover up list items (better way?)
	else
		adjH += 3;   // for scroll margin (otherwise vert. scrollbar appears when not needed)

	// resize the tree viewport
	listRect.adjust(0, 0, 0, adjH);
	//qDebug() << "listRect" << listRect << "adjH" << adjH << "algnTop" << algnTop << "aboveH" << aboveHeight  << "algnBot" << algnBot << "belowH" << belowHeight;

	// takes into account the container size restraints
	listRect.setSize(listRect.size().expandedTo(container->minimumSize()).boundedTo(container->maximumSize()));
	// make sure the widget fits and visible on screen vertically
	if (usePopup) {
		// Clamp the listRect height and vertical position so we don't expand outside the available screen geometry.
		const int height = !boundToScreen ? listRect.height() : qMin(listRect.height(), screen.height());
		listRect.setHeight(height);
		if (listRect.bottom() < algnTop)
			listRect.moveBottom(algnTop);  // don't leave a short list hanging above the selector widget
		if (boundToScreen) {
			// make sure full list is visible
			if (listRect.top() < screen.top())
				listRect.moveTop(screen.top());
			if (listRect.bottom() > screen.bottom())
				listRect.moveBottom(screen.bottom());
		}
	}
	else {
		// constrain to maximum visible items size
		QRect visReg = view()->visualRect(view()->currentIndex());
		// get the height of one item in the view (better way?)
		int maxH = (visReg.isValid() ? visReg.size().height() : view()->fontMetrics().boundingRect('A').height()) * maxVisibleItems();
		listRect.setHeight(qMin(listRect.height(), maxH));

		if (!boundToScreen || listRect.height() > aboveHeight) {
			// move under selector widget
			if (boundToScreen && belowHeight < listRect.height())
				listRect.setHeight(belowHeight);
			listRect.moveTop(algnBot);
		}
		else if (listRect.height() > belowHeight || listRect.bottom() < algnTop || (listRect.bottom() > algnTop && listRect.top() < algnTop)) {
			// move above selector widget
			if (aboveHeight < listRect.height())
				listRect.setHeight(aboveHeight);
			listRect.moveBottom(algnTop);
		}
	}
	//qDebug() << "listRect" << listRect;

	container->setGeometry(listRect);
}
