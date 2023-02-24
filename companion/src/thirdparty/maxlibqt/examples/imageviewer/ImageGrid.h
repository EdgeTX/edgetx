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

#ifndef IMAGEGRID_H
#define IMAGEGRID_H

#include <QFileSystemModel>
#include <QHelpEvent>
#include <QListView>
#include <QModelIndex>
#include <QPainter>
#include <QPixmap>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

/*!
  \ingroup examples_imageviewer
*/
class ImageGridDelegate : public QStyledItemDelegate
{
		Q_OBJECT
	public:
		using QStyledItemDelegate::QStyledItemDelegate;

		bool showTooltips = true;  // respond to tooltip help event
		qreal frameWidth = 2.0;    // selection frame size

		void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const override;
		QPixmap pixmap(const QModelIndex &idx, const QSize &size) const;
		bool helpEvent(QHelpEvent *e, QAbstractItemView *v, const QStyleOptionViewItem &, const QModelIndex &idx) override;
		QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &) const override { return opt.decorationSize; }

	protected:
		// we don't use this, but provide an optimized version in case this gets called internally in superclass
		void initStyleOption(QStyleOptionViewItem *o, const QModelIndex &idx) const override { o->index = idx; }
	private:
		Q_DISABLE_COPY(ImageGridDelegate)
};


/*!
  \ingroup examples_imageviewer

  \c ImageGrid is a \c QListView using a custom \c ImageGridDelegate to present a grid of "previews" of images in a given folder.
	The number of columns used to show the images, or the image preview sizes can set explicitly, or both can auto-adjust based
	on the available space (this is the default setting).

  Note that the global \c QPixmapCache is used to store image previews by the \c ImageGridDelegate class. It should be suitably "large."
*/
class ImageGrid : public QListView
{
		Q_OBJECT
		Q_PROPERTY(int count READ count NOTIFY countChanged)
		Q_PROPERTY(QString currentPath READ currentPath WRITE setCurrentPath NOTIFY currentPathChanged)
		Q_PROPERTY(QString currentImage READ currentImage WRITE setCurrentImage NOTIFY currentImageChanged)
		Q_PROPERTY(int currentFileIndex READ currentFileIndex WRITE setCurrentFile NOTIFY currentFileIndexChanged)
		Q_PROPERTY(QDir::SortFlags sortFlags READ sortFlags WRITE sortBy NOTIFY sortChanged)
		Q_PROPERTY(QSize imageSize READ imageSize WRITE setImageSize NOTIFY imageSizeChanged)
		Q_PROPERTY(int columns READ columns WRITE setColumns)

	public:
		explicit ImageGrid(QWidget *p = nullptr);

		int count() const;
		QString currentPath() const;
		QString currentImage() const;
		int currentFileIndex() const;
		QDir::SortFlags sortFlags() const;
		QSize imageSize() const;
		int columns() const;

	public slots:
		void setCurrentPath(const QString &path);
		void setCurrentImage(const QString &imageFile);
		void setCurrentFile(int index);
		void nextImage() { selectImage(findNextImage(false)); }
		void prevImage() { selectImage(findNextImage(true)); }
		void sortBy(QDir::SortFlags flags);
		void setColumns(uint columns = 0);
		void setImageSize(const QSize &size = QSize());

	signals:
		void countChanged(int count);
		void currentPathChanged(const QString &path) const;
		void imageSelected(const QString &filename) const;
		void currentImageChanged(const QString &filename) const;
		void currentFileIndexChanged(int index) const;
		void sortChanged(QDir::SortFlags flags) const;
		void imageSizeChanged(const QSize &size) const;

	protected:
		void resizeEvent(QResizeEvent *e) override;

	private slots:
		void selectImage(const QModelIndex &idx);
		void updateLayout(bool force = false);
		void resetModel();

	private:
		uint columnsForWidth(int w) const;
		QModelIndex findNextImage(bool prevoius = false);

		QFileSystemModel *m_model = nullptr;
		uint m_columns = 0;
		QDir::SortFlags m_sortBy = QDir::Name;
		QSize m_icnSize;

		Q_DISABLE_COPY(ImageGrid)
};

inline
int ImageGrid::count() const { return m_model ? m_model->rowCount(rootIndex()) : 0; }

inline
QString ImageGrid::currentPath() const { return m_model ? m_model->rootPath() : QString(); }

inline
QString ImageGrid::currentImage() const {
	return currentIndex().isValid() ?
	      currentIndex().data(QFileSystemModel::FilePathRole).toString() :
	      QString();
}

inline
int ImageGrid::currentFileIndex() const { return currentIndex().row(); }

inline
QDir::SortFlags ImageGrid::sortFlags() const { return m_sortBy; }

inline
QSize ImageGrid::imageSize() const { return m_icnSize; }

inline
int ImageGrid::columns() const { return m_columns; }

inline
void ImageGrid::resizeEvent(QResizeEvent *e)
{
	QListView::resizeEvent(e);
	updateLayout();
}

#endif // IMAGEGRID_H
