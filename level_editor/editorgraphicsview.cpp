#include "editorgraphicsview.h"

#include <QMouseEvent>

void EditorGraphicsView::mousePressEvent(QMouseEvent *event) {
	QGraphicsView::mousePressEvent(event);
	
	auto& pos = event->pos();
	auto scenePos = this->mapToScene(pos);
	emit sceneClicked(scenePos);
}
