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


static void on_mouse( int event, int x, int y, int flags, void* param )
{
    q.mouseClick( event, x, y, flags, param );
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    cv::Mat img = cv::imread("1.jpg", 1);

    const string winName = "image";
    namedWindow( winName, WINDOW_AUTOSIZE );
    setMouseCallback( winName, on_mouse, 0 );

    q.setImageAndWinName(img, winName );

    q.showImage();

    for(;;)
    {
        char c = (char)waitKey(0);
        switch( c )
        {
        case '\x1b':
            cout << "Exiting ..." << endl;
            goto exit_main;
        case 'r':
            cout << endl;
            q.reset();
            q.showImage();
            break;
        case 'n':
            int iterCount = q.getIterCount();
            cout << "<" << iterCount << "... ";
            int newIterCount = q.nextIter();
            if( newIterCount > iterCount )
            {
                q.showImage();
                cout << iterCount << ">" << endl;
            }
            else
                cout << "rect must be determined>" << endl;
            break;
        }
    }

    exit_main:
        destroyWindow( winName );

    return a.exec();
}
