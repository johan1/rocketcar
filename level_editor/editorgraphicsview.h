#ifndef EDITORGRAPHICSVIEW_H
#define EDITORGRAPHICSVIEW_H

#include "common.h"

#include <QGraphicsView>
#include <QPoint>
class EditorGraphicsView : public QGraphicsView {
	Q_OBJECT

public:
	EditorGraphicsView(QWidget *parent = nullptr) :
			QGraphicsView(parent), scaleFactor(1.0f), currentGameObject(nullptr) {}

	void setCurrentGameObject(GameObject const* gameObject);

private:
	float scaleFactor;
	GameObject const* currentGameObject;
	QPixmap cursorPixmap;

	void updateCursor();
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);

signals:
	void sceneClicked(QMouseEvent* event, QPointF position);
    void sceneMoved(QMouseEvent* event, QPointF position);
};

#endif /* EDITORGRAPHICSVIEW_H */
