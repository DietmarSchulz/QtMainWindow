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

	Ptr<StructuredEdgeDetection> pDollar =
		createStructuredEdgeDetection("models/model.yml");
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
	return res;
}
