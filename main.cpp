#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

int main()
{
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
        circle(image, qpf[i], 5+i*2, Scalar(155),-1);
        circle(image, ppf[i], 5+i*2, Scalar(155),-1);
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