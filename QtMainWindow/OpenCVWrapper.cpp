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
