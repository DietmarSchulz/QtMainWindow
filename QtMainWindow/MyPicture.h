#pragma once

#include <qgraphicsitem.h>
#include <opencv2/opencv.hpp>
#include <QJsonObject>

class MyPicture : public QGraphicsPixmapItem
{
	Q_GADGET;
public:
	MyPicture(std::string path, QPixmap pixmap, QGraphicsItem* parent = nullptr) : QGraphicsPixmapItem(pixmap, parent), currPath(path) {}

	void read(const QJsonObject& json);
	void write(QJsonObject& json) const;
private:
	std::string currPath;
};

