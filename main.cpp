#include <opencv2/opencv.hpp>
#include <iostream>
#include "locMark.hpp"
#include "splitID.h"
using namespace std;
using namespace cv;
void mark();
void tmm();
void split();
int main()
{
    split();
    tmm();
    mark();
    return 1;
    Mat image = imread("a.jpg", 0);
    Mat image2 = imread("b.jpg", 0);

    namedWindow("a", WINDOW_NORMAL);
    vector<Point2f> ppf, qpf(4);
    ppf.push_back(Point2f(1347, 1066));
    ppf.push_back(Point2f(1112, 1066));
    ppf.push_back(Point2f(1112, 1030));
    ppf.push_back(Point2f(1347, 1030));
    // ppf.push_back(Point2f(1112, 2486));
    // ppf.push_back(Point2f(1112, 2450));
    // ppf.push_back(Point2f(1347, 2450));
    // ppf.push_back(Point2f(1347, 2486));
    Mat cut = image(Rect(1200, 1150, 800, 200));
    vector<KeyPoint> kp;
    FAST(cut, kp, 50);
    vector<Point2f> pf;
    for (size_t i = 0; i < kp.size(); i++)
    {
        circle(cut, kp[i].pt, 5, Scalar(255));
        pf.push_back(Point2f(1200, 1150) + kp[i].pt);
    }
    RotatedRect rr = minAreaRect(pf);
    rr.points(qpf.data());
    rectangle(image, rr.boundingRect(), Scalar(0), 4);
    /*
    cut = image(Rect(1200, 2750, 800, 200));
    GaussianBlur(cut, cut, Size(7, 7), 1);
    FAST(cut, kp, 50);
    // vector<Point2f> pf;
    pf.clear();
    for (size_t i = 0; i < kp.size(); i++)
    {
        circle(cut, kp[i].pt, 5, Scalar(255));
        pf.push_back(Point2f(1200, 2750) + kp[i].pt);
    }
    rr = minAreaRect(pf);
    rr.points(qpf.data() + 4);
    rectangle(image, rr.boundingRect(), Scalar(0), 4);
    */
    for (size_t i = 0; i < qpf.size(); i++)
    {
        circle(image, qpf[i], 5 + i * 2, Scalar(155), -1);
        circle(image, ppf[i], 5 + i * 2, Scalar(155), -1);
    }
    imwrite("tp.jpg", image);
    imshow("a", image);
    waitKey();

    Mat H = findHomography(ppf, qpf);
    Mat wp;
    warpPerspective(image2, wp, H, image.size());
    // RotatedRect rr = minAreaRect(pf);
    // rectangle(cut,rr.boundingRect(),Scalar(0),4);
    // namedWindow("a",WINDOW_NORMAL);
    imshow("a", (image + wp) / 2);
    waitKey();

    return 0;
}

void split()
{
    Mat image=imread("idcard-s.png",0);
    
    SplitID spid(1);
    // spid.setImage((image(Rect(2,2,image.cols-4,image.rows-4))>250)*255);
    spid.setImage((image(Rect(2,2,image.cols-4,image.rows-4))));
    spid.getWordRegions();
    return;
}
void tmm()
{
    Mat idcard=imread("idcard.png",0);
    Mat name=imread("idcard-name.png",0);
    Mat out;
    matchTemplate(idcard,name,out,TM_CCOEFF_NORMED);


    double a,b;
    cv::Point p1,p2;
    
    cv::minMaxLoc(out,&a,&b,&p1,&p2);
    circle(out,p1,5,Scalar(122.0/255),-1);
    circle(out,p2,5,Scalar(122.0/255),-1);
    cout<<p1<<"\t"<<p2<<"\n";

    imshow("out",out);
    imwrite("tmm.png",out*255);
    waitKey();
}
void mark()
{
    Mat image1 = imread("20190612181530.jpg", 0);
    Mat image2 = imread("2.png", 0);
    Size imageSize=image1.size();
    
    // Mat cameraMatrix=(Mat_<double>(3,3)<<3.2606318303148678e+03, 0., 1.9902589726153024e+03, 0.,3.2544394559854172e+03, 1.4792963284506968e+03, 0., 0., 1.);
    // Mat distCoeffs=(Mat_<double>(5,1)<<3.3748983542710012e-02, -8.1017183509675891e-02,-3.3142558829948004e-03, -1.5452809687960208e-03, 0.);
    Mat cameraMatrix=(Mat_<double>(3,3)<<3.2439474859364700e+03, 0., 2.0088010945219005e+03, 0.,3.2379690414176052e+03, 1.4863769408813732e+03, 0., 0., 1.);
    Mat distCoeffs=(Mat_<double>(5,1)<<1.3950461445044790e-01, -8.5094151077102653e-01,-2.6431370928322010e-03, 8.7808751132873724e-05,1.5867377592585166e+00);
    
    Mat view, rview, map1, map2;
    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
                            getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
                            imageSize, CV_16SC2, map1, map2);

    // remap(image1, image1, map1, map2, INTER_LINEAR);
    // imwrite("image1.jpg",image1);
    LocMark lm1(700, 10), lm2(500, 10);
    vector<Point2f> pts1, pts2;
    lm1.getMarkPosition(image1, pts1);
    lm2.getMarkPosition(image2, pts2);
    vector<Point2f> chess1, chess2, tp1, tp2;
    tp1.push_back(Point2f(0, 0));
    tp1.push_back(Point2f(image1.cols, 0));
    tp1.push_back(Point2f(image1.cols, image1.rows));
    tp1.push_back(Point2f(0, image1.rows));
    tp2.push_back(Point2f(0, 0));
    tp2.push_back(Point2f(image2.cols, 0));
    tp2.push_back(Point2f(image2.cols, image2.rows));
    tp2.push_back(Point2f(0, image2.rows));
    // undistortPoints(pts1,pts1,cameraMatrix,distCoeffs,cv::noArray(), cameraMatrix);

    
    for (size_t i = 0; i < 4; i++)
    {
        int mindist = 9999999;
        int minindex = 0;
        for (size_t j = 0; j < 4; j++)
        {
            double dist = abs(pts1[j].x - tp1[i].x) + abs(pts1[j].y - tp1[i].y);
            if (dist < mindist)
            {
                mindist = dist;
                minindex = j;
            }
        }
        chess1.push_back(pts1[minindex]);
    }

    for (size_t i = 0; i < 4; i++)
    {
        int mindist = 9999999;
        int minindex = 0;
        for (size_t j = 0; j < 4; j++)
        {
            double dist = abs(pts2[j].x - tp2[i].x) + abs(pts2[j].y - tp2[i].y);
            if (dist < mindist)
            {
                mindist = dist;
                minindex = j;
            }
        }
        chess2.push_back(pts2[minindex]);
    }

    vector<uchar> status;
    Mat H = findHomography(chess1, chess2, CV_RANSAC, 1, status, 1000, 0.999);
    // remap(image1, image1, map1, map2, INTER_LINEAR);
    // image1=imread("image1.jpg");
    Mat wp;
    warpPerspective(image1, wp, H, image1.size());
    imwrite("scen.png", wp);
}
