#pragma once

#include <qgraphicsitem.h>
#include <opencv2/opencv.hpp>

class MyPicture : public QGraphicsPixmapItem
{
public:
	MyPicture(std::string path, QPixmap pixmap, QGraphicsItem* parent = nullptr) : QGraphicsPixmapItem(pixmap, parent), currPath(path) {}
private:
	std::string currPath;
};

