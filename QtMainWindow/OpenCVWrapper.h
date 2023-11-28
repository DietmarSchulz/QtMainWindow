#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/line_descriptor.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <opencv2/ximgproc.hpp>
#include <QtGui/qimage.h>
#include <qfiledialog.h>

class OpenCVWrapper
{
public:
    OpenCVWrapper();

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

    static cv::Mat ScaleRGB(cv::Mat& imageSource, double gammaRed, double gammaGreen, double gammaBlue)
    {
        std::vector<cv::Mat> bgr;
        cv::split(imageSource, bgr);
        cv::Mat imageDestination;
        cv::Mat redlookUpTable(1, 256, CV_8U);
        uchar* redp = redlookUpTable.ptr();
        cv::Mat greenlookUpTable(1, 256, CV_8U);
        uchar* greenp = greenlookUpTable.ptr();
        cv::Mat bluelookUpTable(1, 256, CV_8U);
        uchar* bluep = bluelookUpTable.ptr();        
        for (int i = 0; i < 256; ++i)
            bluep[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gammaBlue) * 255.0);
        LUT(bgr[0], bluelookUpTable, bgr[0]);
        for (int i = 0; i < 256; ++i)
            greenp[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gammaGreen) * 255.0);
        LUT(bgr[1], greenlookUpTable, bgr[1]);
        for (int i = 0; i < 256; ++i)
            redp[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gammaRed) * 255.0);
        LUT(bgr[2], redlookUpTable, bgr[2]);
        merge(bgr, imageDestination);
        return imageDestination;
    }

    static cv::Mat Add(cv::Mat img1, cv::Mat img2, double alpha)
    {
        cv::Mat dst;
        auto beta = (1.0 - alpha);
        addWeighted(img1, alpha, img2, beta, 0.0, dst);
        return dst;
    }

    static cv::Mat ColorHistEqualization(cv::Mat image);

    cv::Mat Sobel(cv::Mat orgImg);


    static std::string saveSubPicture(std::string&& pic)
    {
        if (pic.empty())
            return pic; // Nothing in, nothing out
        /// Read the image
        cv::Mat src = cv::imread(pic);

        /// Show the image
        std::string window_image{ "Original" };
        std::string result_image{ "POI Image" };
        cv::namedWindow(window_image, cv::WINDOW_NORMAL);
        cv::imshow(window_image, src);
        cv::namedWindow(result_image, cv::WINDOW_NORMAL);
        cv::moveWindow(result_image, 0, 0);

        cv::Rect roi;
        while (roi.empty()) {
            roi = cv::selectROI(window_image, src);
        }
        auto subPicture = src(roi).clone();
        imshow(result_image, subPicture);
        std::string saveAsPath = QFileDialog::getSaveFileName(nullptr, "Save as:", QString::fromStdString(pic), "All picture Files (*.jpg *.png *.tiff)").toStdString();
        if (!saveAsPath.empty()) {
            auto success = imwrite(saveAsPath, subPicture);
            if (!success) {
                std::cout << "Error writing the file\n";
            }
        }
        cv::waitKey();
        cv::destroyAllWindows();
        return saveAsPath;
    }

private:
    cv::Ptr<cv::ximgproc::StructuredEdgeDetection> pDollar;
};

