#include "MyGraphicsView.h"
#include <qt5/QtCore/qdebug.h>

void MyGraphicsView::keyPressEvent(QKeyEvent* event)
{
	QGraphicsView::keyPressEvent(event);
}

void MyGraphicsView::keyReleaseEvent(QKeyEvent* event)
{
	QGraphicsView::keyReleaseEvent(event);
}

void MyGraphicsView::mouseDoubleClickEvent(QMouseEvent* event)
{
	QGraphicsView::mouseDoubleClickEvent(event);
}

void MyGraphicsView::mousePressEvent(QMouseEvent* event)
{
	QGraphicsView::mousePressEvent(event);
}

void MyGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);
}

void MyGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
	QGraphicsView::mouseReleaseEvent(event);
}

void MyGraphicsView::wheelEvent(QWheelEvent* event)
{
	QGraphicsView::wheelEvent(event);
}
