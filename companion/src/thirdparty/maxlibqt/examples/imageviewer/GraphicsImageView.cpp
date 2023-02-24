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

#include "GraphicsImageView.h"
#include <QPixmap>

GraphicsImageView::GraphicsImageView(QWidget *p) :
  QGraphicsView(new QGraphicsScene, p)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setBackgroundRole(QPalette::Shadow);
	setViewportUpdateMode(FullViewportUpdate);
	setOptimizationFlags(DontSavePainterState | DontClipPainter | DontAdjustForAntialiasing);
	setAlignment(Qt::AlignCenter);
	setFrameStyle(QFrame::NoFrame);
	setLineWidth(0);
}

void GraphicsImageView::setImageFile(const QString &imageFile)
{
	if (m_imageFile != imageFile) {
		m_imageFile = imageFile;
		loadImage(viewport()->contentsRect().size());
	}
}

void GraphicsImageView::setImageScalingMode(int mode)
{
	if (m_scaleMode != Qt::AspectRatioMode(mode)) {
		m_scaleMode = Qt::AspectRatioMode(mode);
		if (m_item) {
			rotationReset();
			zoomReset();
			scene()->setSceneRect(viewport()->contentsRect());
			loadImage(viewport()->contentsRect().size());
		}
		emit imageScalingModeChanged(m_scaleMode);
	}
}

void GraphicsImageView::setImageScale(qreal scale) const
{
	if (m_item && scale > 0.0) {
		m_item->setScale(scale);
		if (qFuzzyCompare(scale, 1.0))
			m_item->setPos(0,0);
		emit imageScaleChanged(scale);
	}
}

void GraphicsImageView::setImageRotation(qreal degrees) const
{
	if (!m_item)
		return;
	m_item->setRotation(degrees);
	emit imageRotationChanged(degrees);
}

void GraphicsImageView::loadImage(const QSize &size)
{
	if (!scene())
		return;
	if (m_imageFile.isEmpty()) {
		// remove existing image, if any
		removeItem();
		return;
	}

	// Load image at original size
	QPixmap pm(m_imageFile);
	if (pm.isNull()) {
		// file not found/other error
		removeItem();
		return;
	}
	// Resize the image here.
	pm = pm.scaled(size, m_scaleMode, Qt::SmoothTransformation);
	if (createItem()) {
		m_item->setPixmap(pm);
		m_item->setTransformOriginPoint(m_item->boundingRect().center());
	}
	if (pm.size().width() < size.width() || pm.size().height() < size.height())
		scene()->setSceneRect(QRectF(viewport()->contentsRect().topLeft(), QSizeF(pm.size())));
	emit imageChanged();
}

void GraphicsImageView::resizeEvent(QResizeEvent *e)
{
	QGraphicsView::resizeEvent(e);
	if (!scene())
		return;
	// Set scene size to fill the available viewport size;
	const QRect sceneRect(viewport()->contentsRect());
	scene()->setSceneRect(sceneRect);
	// Keep the root item sized to fill the viewport and scene;
	if (m_item)
		loadImage(sceneRect.size());
}

bool GraphicsImageView::createItem() {
	if (m_item)
		return true;
	if (!m_item && scene()) {
		m_item = new QGraphicsPixmapItem();
		m_item->setFlag(QGraphicsItem::ItemIsMovable);
		m_item->setTransformationMode(Qt::SmoothTransformation);
		m_item->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
		m_item->setCursor(Qt::ClosedHandCursor);
		scene()->addItem(m_item);
		return true;
	}
	return false;
}

void GraphicsImageView::removeItem()
{
	if (m_item) {
		if (scene())
			scene()->removeItem(m_item);
		delete m_item;
		m_item = nullptr;
		emit imageChanged();
	}
}
