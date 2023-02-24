/*
	GraphicsImageView
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

#ifndef GRAPHICSIMAGEVIEW_H
#define GRAPHICSIMAGEVIEW_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>

/*!
  \ingroup examples_imageviewer
	A custom \c QGraphicsView for displaying an image scaled to fit into the full available viewport/scene geometry.
	It will automatically resize the \c QGraphicsScene and rescale the contained image whenever the size of this widget
	changes.  It also has a few extra features for "zooming" and rotating the image using \c QGraphicsItem transformations,
	and a setting for how the image ratio is preserved while scaling.
*/
class GraphicsImageView : public QGraphicsView
{
		Q_OBJECT
		Q_PROPERTY(QString imageFile READ imageFile WRITE setImageFile NOTIFY imageChanged USER true)
		Q_PROPERTY(qreal imageScale READ imageScale WRITE setImageScale NOTIFY imageScaleChanged)
		Q_PROPERTY(qreal imageRotation READ imageRotation WRITE setImageRotation NOTIFY imageRotationChanged)
		Q_PROPERTY(Qt::AspectRatioMode imageScalingMode READ imageScalingMode WRITE setImageScalingMode NOTIFY imageScalingModeChanged)

	public:
    explicit GraphicsImageView(QWidget *p = nullptr);

		QString imageFile() const { return m_imageFile; }
    Qt::AspectRatioMode imageScalingMode() const { return m_scaleMode; }
    qreal imageScale() const  { return m_item ? m_item->scale() : 0.0; }
    qreal imageRotation() const { return m_item ? m_item->rotation() : 0.0; }

  public slots:
		void setImageFile(const QString &imageFile);
		void setImageScalingMode(int mode);

    void setImageScale(qreal scale) const;
    void zoomImage(int steps) const { setImageScale(imageScale() + steps * 0.1); }
    void zoomIn()    const { zoomImage(1); }
    void zoomOut()   const { zoomImage(-1); }
    void zoomReset() const { setImageScale(1.0); }

    void setImageRotation(qreal degrees) const;
    void rotateCw() const  { setImageRotation(imageRotation() + 90.0); }
    void rotateCCw() const { setImageRotation(imageRotation() - 90.0); }
    void rotationReset() const { setImageRotation(0); }

	signals:
		void imageChanged() const;
		void imageScaleChanged(qreal) const;
		void imageRotationChanged(qreal) const;
		void imageScalingModeChanged(Qt::AspectRatioMode) const;

  protected slots:
    void loadImage(const QSize &size);

	protected:
		void resizeEvent(QResizeEvent *e) override;

	private:
		bool createItem();
		void removeItem();

    Qt::AspectRatioMode m_scaleMode = Qt::KeepAspectRatio;
    QString m_imageFile;
		QGraphicsPixmapItem *m_item = nullptr;

		Q_DISABLE_COPY(GraphicsImageView)
};

#endif // GRAPHICSIMAGEVIEW_H
