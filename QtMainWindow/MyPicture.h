#pragma once

#include <qgraphicsitem.h>
#include <opencv2/opencv.hpp>
#include <QJsonObject>

class MyPicture : public QGraphicsPixmapItem
{
	Q_GADGET;
public:
	MyPicture(QGraphicsItem* parent = nullptr) : QGraphicsPixmapItem(parent), gamma(1.0),
											gammaRed(1.0), gammaGreen(1.0), gammaBlue(1.0), alphaAdd(0.5) {}
	MyPicture(std::string path, QPixmap pixmap, QGraphicsItem* parent = nullptr) : QGraphicsPixmapItem(pixmap, parent), currPath(path), gamma(1.0),
																		gammaRed(1.0), gammaGreen(1.0), gammaBlue(1.0), alphaAdd(0.5) {}

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
	std::string getCurrPath() const
	{
		return currPath;
	}
	std::string getSecondPath()
	{
		return secondPath;
	}
	void setSecondPath(std::string value)
	{
		secondPath = value;
	}
	double getAlphaAdd()
	{
		return alphaAdd;
	}
	void setAlphaAdd(double val)
	{
		alphaAdd = val;
	}
	double getGammaSecond()
	{
		return gammaSecond;
	}
	void setGammaSecond(double val)
	{
		gamma = val;
	}
	double getGammaRedSecond()
	{
		return gammaRedSecond;
	}
	void setGammaRedSecond(double val)
	{
		gammaRed = val;
	}
	double getGammaGreenSecond()
	{
		return gammaGreenSecond;
	}
	void setGammaGreenSecond(double val)
	{
		gammaGreen = val;
	}
	double getGammaBlueSecond()
	{
		return gammaBlueSecond;
	}
	void setGammaBlueSecond(double val)
	{
		gammaBlue = val;
	}
private:
	std::string currPath;
	std::string secondPath;
	double alphaAdd;
	double gamma;
	double gammaRed;
	double gammaGreen;
	double gammaBlue;
	double gammaSecond = 0.0;
	double gammaRedSecond = 0.0;
	double gammaGreenSecond = 0.0;
	double gammaBlueSecond = 0.0;
};

