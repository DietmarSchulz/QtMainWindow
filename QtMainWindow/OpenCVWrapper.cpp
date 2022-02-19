#include "OpenCVWrapper.h"
#include <qfile.h>
#include <qtemporaryfile.h>
#include <opencv2/bioinspired.hpp>
#include <opencv2/core/utility.hpp>

using namespace std;
using namespace cv;
using namespace cv::ximgproc;
using namespace cv::line_descriptor;

OpenCVWrapper::OpenCVWrapper()
{
    QFile file(":/QtMainWindow/Models/model.yml");
    bool isOpen = file.open(QIODevice::ReadOnly);
    auto content = file.readAll();
    std::string show = content.data();
    QTemporaryFile tempfile;
    if (tempfile.open()) {
        // file.fileName() returns the unique file name
        tempfile.write(show.data());
        tempfile.flush();
        pDollar =
            createStructuredEdgeDetection(tempfile.fileName().toStdString());
    }

}

cv::Mat OpenCVWrapper::ColorHistEqualization(cv::Mat image)
{
    //Convert the image from BGR to YCrCb color space
    Mat hist_equalized_image;
    cvtColor(image, hist_equalized_image, COLOR_BGR2YCrCb);

    //Split the image into 3 channels; Y, Cr and Cb channels respectively and store it in a std::vector
    vector<Mat> vec_channels;
    split(hist_equalized_image, vec_channels);

    //Equalize the histogram of only the Y channel 
    equalizeHist(vec_channels[0], vec_channels[0]);

    //Merge 3 channels in the vector to form the color image in YCrCB color space.
    merge(vec_channels, hist_equalized_image);

    //Convert the histogram equalized image from YCrCb to BGR color space again
    cvtColor(hist_equalized_image, hist_equalized_image, COLOR_YCrCb2BGR);

    //Define the names of windows
    String windowNameOfOriginalImage = "Original Image";
    String windowNameOfHistogramEqualized = "Histogram Equalized Color Image";

    // Create windows with the above names
    namedWindow(windowNameOfOriginalImage, WINDOW_NORMAL);
    namedWindow(windowNameOfHistogramEqualized, WINDOW_NORMAL);

    // Show images inside the created windows.
    imshow(windowNameOfOriginalImage, image);
    imshow(windowNameOfHistogramEqualized, hist_equalized_image);

    // Calc Histogram
    //! [Separate the image in 3 places ( B, G and R )]
    vector<Mat> bgr_planes;
    split(image, bgr_planes);
    //! [Separate the image in 3 places ( B, G and R )]

    //! [Establish the number of bins]
    int histSize = 256;
    //! [Establish the number of bins]

    //! [Set the ranges ( for B,G,R) )]
    float range[] = { 0, 256 }; //the upper boundary is exclusive
    const float* histRange = { range };
    //! [Set the ranges ( for B,G,R) )]

    //! [Set histogram param]
    bool uniform{ true }, accumulate{ false };
    //! [Set histogram param]

    //! [Compute the histograms]
    Mat b_hist, g_hist, r_hist;
    calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
    calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
    calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);
    //! [Compute the histograms]

    //! [Draw the histograms for B, G and R]
    int hist_w{ 512 }, hist_h{ 400 };
    int bin_w = cvRound((double)hist_w / histSize);

    Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
    //! [Draw the histograms for B, G and R]

    //! [Normalize the result to ( 0, histImage.rows )]
    normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
    normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
    normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
    //! [Normalize the result to ( 0, histImage.rows )]

    //! [Draw for each channel]
    for (int i = 1; i < histSize; i++)
    {
        line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
            Point(bin_w * (i), hist_h - cvRound(b_hist.at<float>(i))),
            Scalar(255, 0, 0), 2, 8, 0);
        line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
            Point(bin_w * (i), hist_h - cvRound(g_hist.at<float>(i))),
            Scalar(0, 255, 0), 2, 8, 0);
        line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
            Point(bin_w * (i), hist_h - cvRound(r_hist.at<float>(i))),
            Scalar(0, 0, 255), 2, 8, 0);
    }
    //! [Draw for each channel]

    // Show Histogram
    namedWindow("histo", WINDOW_NORMAL);
    imshow("histo", histImage);

    waitKey(0); // Wait for any keystroke in any one of the windows

    destroyAllWindows(); //Destroy all opened windows
    return hist_equalized_image;
}

cv::Mat OpenCVWrapper::Sobel(cv::Mat orgImg)
{
	string orgWindowName{ "Originabild" };
	namedWindow(orgWindowName, WINDOW_NORMAL);
	moveWindow(orgWindowName, 0, 0);
	imshow(orgWindowName, orgImg);

	string sobelWindowName{ "Sobel" };
	namedWindow(sobelWindowName, WINDOW_NORMAL);
	moveWindow(sobelWindowName, 400, 0);

	string randomWindowName{ "Random Forest" };
	namedWindow(randomWindowName, WINDOW_NORMAL);
	moveWindow(randomWindowName, 800, 0);

	Mat orgFloat;
	orgImg.convertTo(orgFloat, DataType<float>::type, 1 / 255.0);

	Mat edges(orgFloat.size(), orgFloat.type());

	pDollar->detectEdges(orgFloat, edges);

	// computes orientation from edge map
	Mat orientation_map;
	pDollar->computeOrientation(edges, orientation_map);

	// suppress edges
	Mat edge_nms;
	pDollar->edgesNms(edges, orientation_map, edge_nms, 2, 0, 1, true);

	imshow(sobelWindowName, edges);
	imshow(randomWindowName, edge_nms);

	string resultWindowName{ "Original - Random Forest" };
	namedWindow(resultWindowName, WINDOW_NORMAL);
	moveWindow(resultWindowName, 400, 400);

	double minVal, maxVal;
	minMaxLoc(edge_nms, &minVal, &maxVal); //find minimum and maximum intensities
	edge_nms.convertTo(edge_nms, CV_8U, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));

	cvtColor(edge_nms, edge_nms, COLOR_GRAY2BGR);

	edge_nms.convertTo(edge_nms, CV_8UC3);
	resize(edge_nms, edge_nms, size(orgImg));
	Mat res = orgImg - edge_nms;

	imshow(resultWindowName, res);

	auto wait_time = 1000;
	while (getWindowProperty(randomWindowName, WND_PROP_VISIBLE) >= 1) {
		auto keyCode = waitKey(wait_time);
		if (keyCode == 27) { // Wait for ESC key stroke
			destroyAllWindows();
			break;
		}
	}
	destroyAllWindows();
	return edges;
}
