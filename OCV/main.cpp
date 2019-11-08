#include "mainwindow.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "qapp.h"
#include <iostream>
#include <QApplication>

using namespace std;
using namespace cv;

QApp q;
QApp q2;


static void on_mouse( int event, int x, int y, int flags, void* param )
{
    q.mouseClick( event, x, y, flags, param );
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

//    cv::Mat img = cv::imread("1_3.jpg", 1);

//    const string winName = "SELECT THE WATER BODY";
//    namedWindow( winName, WINDOW_AUTOSIZE );
//    setMouseCallback( winName, on_mouse, 0 );

//    q.setImageAndWinName(img, winName );

//    q.showImage();

//    for(;;)
//    {
//        char c = (char)waitKey(0);
//        switch( c )
//        {
//        case '\x1b':
//            cout << "Exiting ..." << endl;
//            q.saveimg1();
//            goto exit_main;
//        case 'r':
//            cout << endl;
//            q.reset();
//            q.showImage();
//            break;
//        case 'n':
//            int iterCount = q.getIterCount();
//            cout << "<" << iterCount << "... ";
//            int newIterCount = q.nextIter();
//            if( newIterCount > iterCount )
//            {
//                q.showImage();
//                cout << iterCount << ">" << endl;
//            }
//            else
//                cout << "rect must be determined>" << endl;
//            break;
//        }
//    }
//    exit_main:
//        destroyWindow( winName );

//    cv::Mat img1 = cv::imread("1_1.jpg", 1);
//    q.setBG(img1, img);


//    cv::waitKey(500);

//    cv::Mat img2 = cv::imread("1_3.jpg", 1);

//    const string winName1 = "SELECT THE WATER BODY";
//    namedWindow( winName1, WINDOW_AUTOSIZE );
//    setMouseCallback( winName1, on_mouse, 0 );

//    q2.setImageAndWinName(img2, winName1 );

//    q2.showImage();

//    for(;;)
//    {
//        char c = (char)waitKey(0);
//        switch( c )
//        {
//        case '\x1b':
//            cout << "Exiting ..." << endl;
//            q2.saveimg2();
//            goto exit_main1;
//        case 'r':
//            cout << endl;
//            q2.reset();
//            q2.showImage();
//            break;
//        case 'n':
//            int iterCount = q2.getIterCount();
//            cout << "<" << iterCount << "... ";
//            int newIterCount = q2.nextIter();
//            if( newIterCount > iterCount )
//            {
//                q2.showImage();
//                cout << iterCount << ">" << endl;
//            }
//            else
//                cout << "rect must be determined>" << endl;
//            break;
//        }
//    }
//    exit_main1:
//        destroyWindow( winName1 );

//    cv::Mat img3 = cv::imread("1_4.jpg", 1);
//    q2.setBG(img3, img2);


    //1_2.png - boat
    //1_3.jpg - bg

    cv::Mat bg = cv::imread("1_3.jpg", -1);
    cv::Mat boat = cv::imread("1_2.png", -1);

    cv::Mat mask;
    vector<cv::Mat> rgbLayer;
    cv::split(boat, rgbLayer);

    if(boat.channels() == 4)
    {
        split(boat,rgbLayer);         // seperate channels
        Mat cs[3] = { rgbLayer[0],rgbLayer[1],rgbLayer[2] };
        merge(cs,3,boat);  // glue together again
        mask = rgbLayer[3];       // png's alpha channel used as mask
    }

    boat.copyTo(bg(cv::Rect(0,0,boat.cols, boat.rows)),mask);


    std::cout<<bg.channels()<<std::endl;

    namedWindow("Final Image",1);

    imshow("Final Image",bg);

    imwrite("final.jpg", bg);

    waitKey(0);

    return a.exec();
}
