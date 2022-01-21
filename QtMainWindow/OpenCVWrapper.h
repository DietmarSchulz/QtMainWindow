#pragma once

#include <opencv2/opencv.hpp>
#include <qt5/QtGui/qimage.h>

class OpenCVWrapper
{
public:
    static QImage Mat2QImage(cv::Mat const& src)
    {
        cv::Mat temp; // make the same cv::Mat
        cvtColor(src, temp, cv::COLOR_BGR2RGB); // cvtColor Makes a copt, that what i need
        QImage dest((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
        dest.bits(); // enforce deep copy, see documentation 
        // of QImage::QImage ( const uchar * data, int width, int height, Format format )
        return dest;
    }

    static cv::Mat QImage2Mat(QImage const& src)
    {
        cv::Mat tmp(src.height(), src.width(), CV_8UC3, (uchar*)src.bits(), src.bytesPerLine());
        cv::Mat result; // deep copy just in case (my lack of knowledge with open cv)
        cvtColor(tmp, result, cv::COLOR_BGR2RGB);
        return result;
    }

    static cv::Mat GammaBrightness(cv::Mat& imageSource, double gamma)
    {
        cv::Mat imageDestination;
        cv::Mat lookUpTable(1, 256, CV_8U);
        uchar* p = lookUpTable.ptr();
        for (int i = 0; i < 256; ++i)
            p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
        cv::LUT(imageSource, lookUpTable, imageDestination);
        return imageDestination;
    }
};

