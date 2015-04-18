#include "editorgraphicsview.h"
#include "config.h"

#include <QMouseEvent>
#include <iostream>

void EditorGraphicsView::mousePressEvent(QMouseEvent *event) {
	QGraphicsView::mousePressEvent(event);
	
	auto& pos = event->pos();
	auto scenePos = this->mapToScene(pos);
	emit sceneClicked(event, scenePos);
}

void EditorGraphicsView::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);
	
	auto& pos = event->pos();
	auto scenePos = this->mapToScene(pos);
	emit sceneMoved(event, scenePos);
}

void EditorGraphicsView::setCurrentGameObject(GameObject const* gameObject) {
	currentGameObject = gameObject;
	cursorPixmap = QPixmap(QString::fromStdString(getImagePath(*gameObject)));
	updateCursor();
}

void EditorGraphicsView::updateCursor() {
	int newWidth = static_cast<float>(cursorPixmap.width()) * scaleFactor;
	int newHeight = static_cast<float>(cursorPixmap.height()) * scaleFactor;
	setCursor(cursorPixmap.scaled(newWidth, newHeight));
}

void EditorGraphicsView::wheelEvent(QWheelEvent *event) {
	int degrees = event->delta()/8;
	float scaleDelta = static_cast<float>(100 + degrees)/100.0f;
	scaleFactor = scaleFactor * scaleDelta;
	std::cout << "Wheel event degrees: " << degrees << ", scaleFactor: " << scaleFactor << std::endl;
	this->scale(scaleDelta, scaleDelta);

	if (currentGameObject) {
		updateCursor();
	}
}
