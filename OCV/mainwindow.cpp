#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/types_c.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace std;
using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // read an image
    cv::Mat image = cv::imread("1.jpg", 1);
    cv::Rect rectangle(50,70,image.cols - 100, image.rows - 80);

    cv::Mat Result, bgmodel, fgmodel;

    cv::grabCut(image,Result, rectangle, bgmodel, fgmodel, 1, cv::GC_INIT_WITH_RECT );

    cv:: compare(Result, cv::GC_PR_FGD, Result, cv::CMP_EQ);

    cv::Mat FG(image.size(), CV_8UC3, cv::Scalar(0,0,0));
    image.copyTo(FG, Result);

    cv:: rectangle(image, rectangle, cv::Scalar(0,0,255), 1);

    cv::namedWindow("My Image");
    // show the image on window
    cv::imshow("My Image", image);

    cv::namedWindow("Segmented Image");
    cv::imshow("Segmented Image", FG);
    cv::waitKey();
    return;
}

MainWindow::~MainWindow()
{
    delete ui;
}
