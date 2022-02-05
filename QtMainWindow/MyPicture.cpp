#include "MyPicture.h"
#include "OpenCVWrapper.h"

void MyPicture::read(const QJsonObject& json)
{
	if (json.contains("path") && json["path"].isString()) {
		currPath = json["path"].toString().toStdString();
		cv::Mat picture = cv::imread(currPath);
		if (json.contains("gamma") && json["gamma"].isDouble()) {
			gamma = json["gamma"].toDouble();
			if (gamma != 1.0) {
				picture = OpenCVWrapper::GammaBrightness(picture, gamma);
			}
		}
		if (json.contains("gammaRed") && json["gammaRed"].isDouble() &&
			json.contains("gammaGreen") && json["gammaGreen"].isDouble() &&
			json.contains("gammaBlue") && json["gammaBlue"].isDouble()) {
			gammaRed = json["gammaRed"].toDouble();
			gammaGreen = json["gammaGreen"].toDouble();
			gammaBlue = json["gammaBlue"].toDouble();
			if (gammaRed != 1.0 || gammaGreen != 1.0 || gammaBlue != 1.0) {
				picture = OpenCVWrapper::ScaleRGB(picture, gammaRed, gammaGreen, gammaBlue);
			}
		}
		if (json.contains("secondPath") && json["secondPath"].isString()) {
			secondPath = json["secondPath"].toString().toStdString();
			if (json.contains("alphaAdd") && json["alphaAdd"].isDouble()) {
				alphaAdd = json["alphaAdd"].toDouble();
				cv::Mat secondPicture = cv::imread(secondPath);

				// using brightness gamma and rgb gammas as predecessor
				secondPicture = OpenCVWrapper::GammaBrightness(secondPicture, gammaSecond);
				secondPicture = OpenCVWrapper::ScaleRGB(secondPicture, gammaRedSecond, gammaGreenSecond, gammaBlueSecond);
				picture = OpenCVWrapper::Add(picture, secondPicture, alphaAdd);
			}
		}
		QImage qim = OpenCVWrapper::Mat2QImage(picture);
		setPixmap(QPixmap::fromImage(qim));
		setFlag(QGraphicsItem::ItemIsMovable);
		setFlag(QGraphicsItem::ItemIsSelectable);
	}
	if (json.contains("positionX") && json["positionX"].isDouble() &&
		json.contains("positionY") && json["positionY"].isDouble()) {
		setPos(json["positionX"].toDouble(), json["positionY"].toDouble());
	}
	if (json.contains("scale") && json["scale"].isDouble()) {
		setScale(json["scale"].toDouble());
	}
	if (json.contains("rotation") && json["rotation"].isDouble()) {
		setRotation(json["rotation"].toDouble());
	}
}

void MyPicture::write(QJsonObject& json) const
{
	json["path"] = QString::fromStdString(currPath);
	json["positionX"] = pos().x();
	json["positionY"] = pos().y();
	json["scale"] = scale();
	json["rotation"] = rotation();
	json["gamma"] = gamma;
	json["gammaRed"] = gammaRed;
	json["gammaGreen"] = gammaGreen;
	json["gammaBlue"] = gammaBlue;
	if (!secondPath.empty()) {
		json["secondPath"] = QString::fromStdString(secondPath);
		json["alphaAdd"] = alphaAdd;
	}
}
