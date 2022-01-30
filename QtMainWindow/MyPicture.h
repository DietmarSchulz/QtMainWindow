#pragma once

#include <qgraphicsitem.h>
#include <opencv2/opencv.hpp>
#include <QJsonObject>

class MyPicture : public QGraphicsPixmapItem
{
	Q_GADGET;
public:
	MyPicture(QGraphicsItem* parent = nullptr) : QGraphicsPixmapItem(parent), gamma(1.0),
											gammaRed(1.0), gammaGreen(1.0), gammaBlue(1.0) {}
	MyPicture(std::string path, QPixmap pixmap, QGraphicsItem* parent = nullptr) : QGraphicsPixmapItem(pixmap, parent), currPath(path), gamma(1.0),
																		gammaRed(1.0), gammaGreen(1.0), gammaBlue(1.0) {}

	void read(const QJsonObject& json);
	void write(QJsonObject& json) const;
	double getGamma()
	{
		return gamma;
	}
	void setGamma(double val)
	{
		gamma = val;
	}
	double getGammaRed()
	{
		return gammaRed;
	}
	void setGammaRed(double val)
	{
		gammaRed = val;
	}
	double getGammaGreen()
	{
		return gammaGreen;
	}
	void setGammaGreen(double val)
	{
		gammaGreen = val;
	}
	double getGammaBlue()
	{
		return gammaBlue;
	}
	void setGammaBlue(double val)
	{
		gammaBlue = val;
	}
	std::string getCurrPath()
	{
		return currPath;
	}
private:
	std::string currPath;
	double gamma;
	double gammaRed;
	double gammaGreen;
	double gammaBlue;
};

