#include "MyPicture.h"

void MyPicture::read(const QJsonObject& json)
{
}

void MyPicture::write(QJsonObject& json) const
{
	json["path"] = QString::fromStdString(currPath);
	json["positionX"] = pos().x();
	json["positionY"] = pos().y();
	json["scale"] = scale();
}
