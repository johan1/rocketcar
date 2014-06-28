#ifndef EDITORGRAPHICSVIEW_H
#define EDITORGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QPoint>
class EditorGraphicsView : public QGraphicsView {
	Q_OBJECT

public:
	EditorGraphicsView(QWidget *parent = nullptr) : QGraphicsView(parent) {}

	virtual void mousePressEvent(QMouseEvent * event);

signals:
	void sceneClicked(QPointF position);
};

#endif /* EDITORGRAPHICSVIEW_H */
