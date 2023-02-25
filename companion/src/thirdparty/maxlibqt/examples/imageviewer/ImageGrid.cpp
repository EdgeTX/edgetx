/*
	ImageGrid
	https://github.com/mpaperno/maxLibQt

	COPYRIGHT: (c)2019 Maxim Paperno; All Rights Reserved.
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

#include "ImageGrid.h"

#include <QApplication>
#include <QDateTime>
#include <QtMath>
#include <QPixmapCache>
#include <QScrollBar>
#include <QStyle>
#include <QToolTip>
#include <QWhatsThis>

ImageGrid::ImageGrid(QWidget *p) :
  QListView(p)
{
	setObjectName("ImageGridView");
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setFrameStyle(QFrame::NoFrame);
	setBackgroundRole(QPalette::Shadow);
	viewport()->setBackgroundRole(QPalette::Shadow);
	viewport()->setMouseTracking(false);  // avoid unnecessary repaints
	setMouseTracking(false);
	setItemDelegate(new ImageGridDelegate(this));
	setEditTriggers(NoEditTriggers);
	setSelectionMode(SingleSelection);
	setSelectionBehavior(SelectRows);
	setViewMode(IconMode);
	setDragDropMode(NoDragDrop);
	setMovement(Static);
	setLayoutMode(Batched);
	setBatchSize(1);
	setSpacing(4);
	setIconSize(QSize(64, 64));  // reset in resizeEvent
	setUniformItemSizes(true);   // delegate always returns decorationSize (icon size) for sizeHint()

	connect(this, &QListView::activated, this, &ImageGrid::selectImage);
	connect(this, &QListView::clicked, this, &ImageGrid::selectImage);
}

void ImageGrid::setCurrentPath(const QString &path)
{
	if (currentPath() == path)
		return;

	resetModel();
	Q_ASSERT(m_model != nullptr);
	m_model->setRootPath(path);
	setRootIndex(m_model->index(path));
	// ensure a layout update on next event loop
	QMetaObject::invokeMethod(this, "updateLayout", Qt::QueuedConnection, Q_ARG(bool, true));
	emit currentPathChanged(path);
}

void ImageGrid::selectImage(const QModelIndex &idx)
{
	QString img;
	if (!idx.isValid() || (img = idx.data(QFileSystemModel::FilePathRole).toString()).isEmpty())
		return;
	if (currentIndex() != idx)
		setCurrentIndex(idx);
	emit imageSelected(img);
}

void ImageGrid::setCurrentImage(const QString &imageFile)
{
	qDebug("%s : %d", qPrintable(imageFile), count());
	if (!count())
		return;
	selectImage(m_model->match(m_model->index(0, 0, rootIndex()), QFileSystemModel::FilePathRole, imageFile, 1, Qt::MatchContains).value(0));
}

void ImageGrid::setCurrentFile(int index)
{
	if (m_model)
		selectImage(m_model->index(index, 0, rootIndex()));
}

void ImageGrid::sortBy(QDir::SortFlags flags)
{
	if (m_sortBy == flags || !m_model)
		return;
	m_sortBy = flags; // | QDir::IgnoreCase;
	const int col = (flags & QDir::Name) ? 0 : (flags & QDir::Size) ? 1 : (flags & QDir::Type) ? 2 : (flags & QDir::Time) ? 3 : 0;
	m_model->sort(col, (flags & QDir::Reversed) ? Qt::DescendingOrder : Qt::AscendingOrder);
	emit sortChanged(flags);
}

void ImageGrid::setColumns(uint columns)
{
	if (m_columns != columns) {
		m_columns = columns;
		updateLayout();
	}
}

void ImageGrid::setImageSize(const QSize &size)
{
	if (m_icnSize == size)
		return;
	m_icnSize = size;
	if (size.isEmpty())
		updateLayout();
	else
		setIconSize(size);
	emit imageSizeChanged(size);
}

uint ImageGrid::columnsForWidth(int w) const  // never returns zero
{
	if (m_columns)
		return m_columns;
	if (!m_model || !rootIndex().isValid())
		return 1;
	return qMax(uint(qMin(qRound(w / 256.0), count())), 1U);
	//return qMax(qRound(w / 256.0), 1);
	//return (w >= 2560 ? 6 : w >= 1920 ? 5 : w >= 1200 ? 4 : w >= 800 ? 3 : w > 400 ? 2 : 1);
}

void ImageGrid::updateLayout(bool force)
{
	if (!m_model || !m_icnSize.isEmpty() || !isVisibleTo(parentWidget()))
		return;
	int w = viewport()->contentsRect().width() -
	        style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr, verticalScrollBar()) -
	        ((width() - contentsRect().width()) * 2);
	const uint cols = columnsForWidth(w);
	w -= (spacing() * (int(cols) - 1));
	const int sz = qRound(w / float(cols));
	// only resize at steps of 12 to avoid excessive redraws
	if (force || qAbs(iconSize().width() - sz) > 12)
		setIconSize(QSize(sz, qCeil(sz * 0.75)));
}

void ImageGrid::resetModel()
{
	static const QStringList filter = {
	  QLatin1String("*.jpg"), QLatin1String("*.jpeg"), QLatin1String("*.png"),
	  QLatin1String("*.gif"), QLatin1String("*.bmp"),  QLatin1String("*.pbm"),
#ifdef QT_SVG_LIB
	  QLatin1String("*.svg")
#endif
	};
	// Qt bug, QFileSystemModel "forgets" filter() settings when root is reset or tree is navigated, so we need a full reset.
	if (m_model)
		m_model->deleteLater();
	m_model = new QFileSystemModel(this);
	m_model->setNameFilters(filter);
	m_model->setFilter(QDir::Files);
	m_model->setReadOnly(true);
	m_model->setNameFilterDisables(false);
	setModel(m_model);
	connect(m_model, &QFileSystemModel::directoryLoaded, this, [this](const QString &path) {
		if (path == currentPath())
			emit countChanged(count());
	});
	connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection &sel, const QItemSelection &) {
		const QModelIndex &idx = sel.indexes().value(0);
		if (!idx.isValid())
			return;
		emit currentFileIndexChanged(idx.row());
		emit currentImageChanged(currentImage());
	});
}

QModelIndex ImageGrid::findNextImage(bool prevoius)
{
	const int n = count();
	if (n < 2)
		return QModelIndex();
	QModelIndex idx;
	bool wrapped = false;
	int i = currentIndex().row();
	while (!idx.isValid() || m_model->isDir(idx)) {
		i = (prevoius ? i - 1 : i + 1);
		if (i < 0 || i >= n) {
			if (wrapped)
				break;
			i = (prevoius ? n - 1 : 0);
			wrapped = true;
		}
		idx = m_model->index(i, 0, rootIndex());
	}
	if (idx.isValid() && m_model->isDir(idx))
		idx = QModelIndex();
	return idx;
}


//
// ImageGridDelegate
//

void ImageGridDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const
{
	if (!idx.isValid() || opt.rect.isEmpty() || opt.decorationSize.isEmpty())
		return;
	const QPixmap pm(pixmap(idx, opt.decorationSize - QSizeF(frameWidth * 2, frameWidth * 2).toSize()));
	if (pm.isNull())
		return;
	QRectF rect = QRectF(opt.rect).adjusted(frameWidth, frameWidth, -frameWidth, -frameWidth);
	QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
	const QRect arect = style->alignedRect(opt.direction, opt.displayAlignment, pm.size(), QRect(QPoint(0,0), rect.size().toSize()));
	rect.moveTopLeft(rect.topLeft() + arect.topLeft());
	//qDebug() << opt.rect << rect << opt.decorationSize << pm.size();
	p->drawPixmap(rect.topLeft(), pm);  // use point position to avoid any painter scaling check
	if (opt.state & QStyle::State_Selected) {
		// draw selection rectangle
		p->setBrush(Qt::NoBrush);
		p->setPen(QPen(opt.palette.brush(QPalette::Highlight), frameWidth, Qt::DashLine, Qt::RoundCap));
		p->drawRect(rect.adjusted(0, 0, arect.x() * -2, arect.y() * -2));
	}
}

QPixmap ImageGridDelegate::pixmap(const QModelIndex &idx, const QSize &size) const
{
	QPixmap pm;
	if (!idx.isValid())
		return pm;
	const QString imgFile = idx.data(QFileSystemModel::FilePathRole).toString();
	if (imgFile.isEmpty() || !QFileInfo(imgFile).exists())
		return pm;
	const QString hash(imgFile.simplified().append(QLatin1String("_")).append(QString::number(quint64(size.width()) << 32 | uint(size.height()))));
	QPixmapCache::find(hash, &pm);
	if (pm.isNull()) {
		//qDebug() << "Cache miss!" << imgFile << sz << hash;
		pm.load(imgFile);
		if (!pm.isNull()) {
			if (pm.size().width() > size.width() || pm.size().height() > size.height())
				pm = pm.scaled(size, Qt::KeepAspectRatio, Qt::FastTransformation);
			QPixmapCache::insert(hash, pm);
		}
	}
	return pm;
}

bool ImageGridDelegate::helpEvent(QHelpEvent *e, QAbstractItemView *v, const QStyleOptionViewItem &, const QModelIndex &idx)
{;
	if (!idx.isValid() || !e || !v || (e->type() == QEvent::ToolTip && !showTooltips))
		return false;
	const QFileInfo fi(idx.data(QFileSystemModel::FilePathRole).toString());
	if (!fi.isFile())
		return false;
	const QString tt(fi.fileName() % "\nC: " % fi.created().toString(Qt::TextDate) % "\nM: " %
	                 fi.lastModified().toString(Qt::TextDate) % "\n" % QString::number(fi.size() / 1024) % " KB");
	switch (e->type()) {
		case QEvent::ToolTip:
			QToolTip::showText(e->globalPos(), tt, v);
			return true;
		case QEvent::WhatsThis:
			QWhatsThis::showText(e->globalPos(), tt, v);
			return true;
		case QEvent::QueryWhatsThis:
			return true;
		default:
			return false;
	}
}

#include "moc_ImageGrid.cpp"
