#pragma once

#include <qgraphicsitem.h>
#include <opencv2/opencv.hpp>
#include <QJsonObject>
#include <concepts>

template <typename T>
concept MyBrightnessType = requires(T& t, double val)
{
	{ t.getGamma() } -> std::same_as<double>;
	{ t.setGamma(val) } -> std::same_as<void>;
};

template <typename T>
concept MyColorableType = MyBrightnessType<T> && requires (T a, double val)
{
	{ a.getGammaRed() } -> std::same_as<double>;
	{ a.getGammaBlue() } -> std::same_as<double>;
	{ a.getGammaGreen() } -> std::same_as<double>;
	{ a.setGammaRed(val) } -> std::same_as<void>;
	{ a.setGammaBlue(val) } -> std::same_as<void>;
	{ a.setGammaGreen(val) } -> std::same_as<void>;
};

template <MyColorableType T>
double MyPicfoo(T& value)
{
	return value.getGamma() + value.getGammaRed() + value.getGammaBlue() + value.getGammaGreen();
}

class MyPicture : public QGraphicsPixmapItem
{
	Q_GADGET;
public:
	MyPicture(QGraphicsItem* parent = nullptr) : QGraphicsPixmapItem(parent), gamma(1.0),
											gammaRed(1.0), gammaGreen(1.0), gammaBlue(1.0) {}
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
	double gammaSecond;
	double gammaRedSecond;
	double gammaGreenSecond;
	double gammaBlueSecond;
};

