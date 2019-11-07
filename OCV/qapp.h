#ifndef QAPP_H
#define QAPP_H

#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace std;
using namespace cv;


const Scalar RED = Scalar(0,0,255);
const Scalar PINK = Scalar(230,130,255);
const Scalar BLUE = Scalar(255,0,0);
const Scalar LIGHTBLUE = Scalar(255,255,160);
const Scalar GREEN = Scalar(0,255,0);

const int BGD_KEY = EVENT_FLAG_CTRLKEY;
const int FGD_KEY = EVENT_FLAG_SHIFTKEY;


class QApp
{
public:

    enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
    static const int radius = 2;
    static const int thickness = -1;


    void mouseClick(int event, int x, int y, int flags, void* );
    int nextIter();
    int getIterCount() const { return iterCount; }
    void showImage() const;
    void reset();
    //void re();
    void setImageAndWinName( const Mat& _image, const string& _winName );

private:
    void setRectInMask();
    void setLblsInMask( int flags, Point p, bool isPr );

    const string* winName;
    const Mat* image;
    Mat mask;
    Mat bgdModel, fgdModel;

    uchar rectState, lblsState, prLblsState;
    bool isInitialized;

    Rect rect;
    vector<Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
    int iterCount;
};





#endif // QAPP_H
