#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
class LocMark
{
public:
	LocMark();
	LocMark(int markSize, int colorGap);
	~LocMark();
	int getMarkPosition(const cv::Mat& img, cv::Point2f &position);
	int getMarkPosition(const cv::Mat& img, std::vector<cv::Point2f> &position);
	int setMarkParams(int markSize, int colorGap);
private:
	std::vector<cv::Point3f> rankPosition(const std::vector<cv::Point2f> &position);
	std::vector<cv::Point2f> getAcrossCorner(const cv::Point2f& A, const cv::Point2f& B);
	int m_markSize = 150;
	int m_blurSize = 51;
	int m_blackMarkThreshold = 50;
	int m_whiteMarkThreshold = 50;
	int m_imgBorderSize = 500;
	std::vector<cv::Point2f> m_prePositon;
};

#define DEBUG_MARK
LocMark::LocMark()
{
	m_markSize = 150;
	m_blurSize = 51;
	m_blackMarkThreshold = 50;
	m_whiteMarkThreshold = 50;
	m_imgBorderSize = 10;
}

LocMark::LocMark(int markSize,int colorGap)
{
	m_markSize = markSize;
	m_blurSize = m_markSize;
	m_blackMarkThreshold = colorGap;
	m_whiteMarkThreshold = colorGap;
	m_imgBorderSize = 100;
}

LocMark::~LocMark()
{
}
int LocMark::setMarkParams(int markSize, int colorGap)
{
	m_markSize = markSize;
	m_blurSize = m_markSize;
	m_blackMarkThreshold = colorGap;
	m_whiteMarkThreshold = colorGap;
	m_imgBorderSize = 10;
	return 0;
}

std::vector<cv::Point2f> LocMark::getAcrossCorner(const cv::Point2f& A, const cv::Point2f& B)
{	
	/*
		D.white  B.black
			\  /
			 \/
			 /\O
			/  \
		A.black  C.white
	*/
	std::vector<cv::Point2f> CD;
	cv::Point2f C, D, O;
	O = (A + B) / 2;
	cv::Point2f ab = A - B;
	int dst = ((A.x - O.x)*(A.x - O.x) + (A.y - O.y)*(A.y - O.y))*1.4;

	if (abs(ab.x) < abs(ab.y))
	{
		cv::Point2f cd = cv::Point2f(1, -ab.x / ab.y);
		int x = 0, y = 0;
		for (int i = 1; i < m_markSize; i += 2)
		{
			x = i + O.x;
			y = cd.y*i + O.y;
			int dst2 = ((x - O.x)*(x - O.x) + (y - O.y)*(y - O.y));
			if (dst2 > dst)
			{
				break;
			}
		}
		C = cv::Point2f(x-2, cd.y*(x-2-O.x) + O.y);
		for (int i = -1; i > -m_markSize; i -= 2)
		{
			x = i + O.x;
			y = cd.y*i + O.y;
			int dst2 = ((x - O.x)*(x - O.x) + (y - O.y)*(y - O.y));
			if (dst2 > dst)
			{
				break;
			}
		}
		D = cv::Point2f(x+2, cd.y*(x+2-O.x) + O.y);

	}
	else
	{
		cv::Point2f cd = cv::Point2f(-ab.y / ab.x, 1);
		int x = 0, y = 0;
		for (int i = 1; i < m_markSize; i += 2)
		{
			y = i + O.y;
			x = cd.x*i + O.x;
			int dst2 = ((x - O.x)*(x - O.x) + (y - O.y)*(y - O.y));
			if (dst2 > dst)
			{
				break;
			}
		}
		C = cv::Point2f(cd.x*(y-2-O.y) + O.x, y-2);
		for (int i = -1; i > -m_markSize; i -= 2)
		{
			y = i + O.y;
			x = cd.x*i + O.x;
			int dst2 = ((x - O.x)*(x - O.x) + (y - O.y)*(y - O.y));
			if (dst2 > dst)
			{
				break;
			}
		}
		D = cv::Point2f(cd.x*(y+2-O.y) + O.x, y+2);
	}
	CD.push_back(C);
	CD.push_back(D);
	return CD;
}

int LocMark::getMarkPosition(const cv::Mat& img, cv::Point2f &position)
{
	std::vector<cv::Point2f> position2;
	getMarkPosition(img, position2);
	if (position2.size()==1)
	{
		position = position2[0];
	}
	return 0;
}
int LocMark::getMarkPosition(const cv::Mat& img, std::vector<cv::Point2f> &position)
{
	cv::Mat image = img.clone();
	m_blurSize = m_markSize*1.2;
	//blur image for getting the black/white region
	cv::Mat imgBlur;
	cv::blur(img, imgBlur, cv::Size(m_blurSize, m_blurSize));

	cv::Mat imgBlackMark = imgBlur - img > m_blackMarkThreshold;
	cv::Mat imgWhiteMark = img - imgBlur > m_whiteMarkThreshold;

	//use rectangle features to find black Mark (square)
	cv::Mat bw;
	cv::erode(imgBlackMark, bw, cv::Mat(5, 5, CV_8UC1, cv::Scalar(255)));

	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(bw, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

	std::vector<cv::Point2f> centers;
	std::vector<int> centerIndex;
	std::vector<cv::Point2f> centerWH;
	for (size_t i = 0; i < contours.size(); i++)
	{


		cv::RotatedRect rotRect = cv::minAreaRect(contours[i]);
		cv::Point2f corners[4];
		rotRect.points(corners);
		//rectangle too small
		if (rotRect.boundingRect().width < 100 || rotRect.boundingRect().height < 100 ||rotRect.boundingRect().width > 300 || rotRect.boundingRect().height > 300)
		{
			continue;
		}
		//rectangle too far with square
		if (abs(rotRect.boundingRect().height - rotRect.boundingRect().width) > 50)
		{
			continue;
		}
		#ifdef DEBUG_MARK
		char c[10];
		sprintf(c, "%d", i);
		putText(bw, c, contours[i][0], 1, 2, cv::Scalar(125));
#endif // DEBUG_MARK
		//abandon rectangle center in image border
		if (rotRect.center.x<m_imgBorderSize ||
			rotRect.center.x>img.cols - m_imgBorderSize ||
			rotRect.center.y<m_imgBorderSize ||
			rotRect.center.y>img.rows - m_imgBorderSize)
		{
			continue;
		}
/*
		//get the width/height of rectangle
		int width = 0;
		int height = 0;
		int minY = 9999;
		int mini = 0;
		for (size_t i = 0; i < 4; i++)
		{
			if (minY > corners[i].y)
			{
				minY = corners[i].y;
				mini = i;
			}
		}
		if (mini == 1 || rotRect.angle < -45)
		{
			width = corners[mini - 1].y - corners[mini].y;
			height = corners[mini + 1].x - corners[mini].x;
		}
		else
		{
			width = corners[mini].x - corners[mini - 1].x;
			height = corners[mini + 1].y - corners[mini].y;
		}
#ifdef DEBUG_MARK
		std::cout << i << "\t" << height << "\t" << width << "\t" << rotRect.angle << "\t" << mini << std::endl;
#endif // DEBUG_MARK

		//rectangle too small
		if (height < 100 || width < 100)
		{
			continue;
		}
		//rectangle too far with square
		if (abs(height - width) > 50)
		{
			continue;
		}
		*/
		centers.push_back(rotRect.center);
		centerWH.push_back(cv::Point2f(rotRect.boundingRect().width, rotRect.boundingRect().height));
		centerIndex.push_back(i);

	}
	std::vector<cv::Point2f> markCenter;
	int basicDst = m_markSize * 2;
	int tpm_markSize=m_markSize;
	int	tpm_blackMarkThreshold=m_blackMarkThreshold;
	int	tpm_whiteMarkThreshold=m_whiteMarkThreshold;
	for (size_t i = 0; i < centers.size(); i++)
	{
		//get the nearest center
		int minj = 0;
		int minDst = 99999;
		for (size_t j = i + 1; j < centers.size(); j++)
		{
			int dst = abs((centers[i] - centers[j]).x) + abs((centers[i] - centers[j]).y);
			if (dst < minDst)
			{
				minDst = dst;
				minj = j;
			}
		}

		//too far
		if (minDst > basicDst)
		{
#ifdef DEBUG_MARK
			std::cout << "bbbbbbbbbbbbbbbbb\t" << i << "\t" << minj << "\t" << minDst << std::endl;
#endif // DEBUG_MARK
			continue;
		}
		if (centers[i].x<m_imgBorderSize ||
			centers[i].x>img.cols - m_imgBorderSize ||
			centers[i].y<m_imgBorderSize ||
			centers[i].y>img.rows - m_imgBorderSize ||
			centers[minj].x<m_imgBorderSize ||
			centers[minj].x>img.cols - m_imgBorderSize ||
			centers[minj].y<m_imgBorderSize ||
			centers[minj].y>img.rows - m_imgBorderSize)
		{
			continue;
		}
		//get the center of the white region
		std::vector<cv::Point2f> CD = getAcrossCorner(centers[i], centers[minj]);
		if (CD[0].x<m_imgBorderSize ||
			CD[0].x>img.cols - m_imgBorderSize ||
			CD[0].y<m_imgBorderSize ||
			CD[0].y>img.rows - m_imgBorderSize ||
			CD[1].x<m_imgBorderSize ||
			CD[1].x>img.cols - m_imgBorderSize ||
			CD[1].y<m_imgBorderSize ||
			CD[1].y>img.rows - m_imgBorderSize)
		{
			CD.clear();
			CD.push_back(cv::Point2f(centers[minj].x, centers[i].y));
			CD.push_back(cv::Point2f(centers[i].x, centers[minj].y));
		}

#ifdef DEBUG_MARK
		std::cout << centerIndex[i] << "\t" << centerIndex[minj] << "\t" << minDst << std::endl;
		cv::circle(bw, centers[minj], 5, cv::Scalar(200), 2);
		cv::circle(bw, centers[i], 5, cv::Scalar(200), 2);
		cv::circle(bw, CD[0], 5, cv::Scalar(100), 2);
		cv::circle(bw, CD[1], 5, cv::Scalar(100), 2);
#endif // DEBUG_MARK
		//the center must be surronded by the white/black rectangle
		//if (cv::mean(imgWhiteMark(cv::Rect(centers[minj].x - 2, centers[i].y - 2, 5, 5)))[0] == 255 &&
		//	cv::mean(imgWhiteMark(cv::Rect(centers[i].x - 2, centers[minj].y - 2, 5, 5)))[0] == 255 &&
		// if (cv::mean(imgWhiteMark(cv::Rect(CD[0].x - 2, CD[0].y - 2, 5, 5)))[0] == 255 &&
		// 	cv::mean(imgWhiteMark(cv::Rect(CD[1].x - 2, CD[1].y - 2, 5, 5)))[0] == 255 &&
		int width=30;
			if (cv::mean(imgBlackMark(cv::Rect(centers[i].x - width/2, centers[i].y - width/2, width, width)))[0] == 255 &&
			cv::mean(imgBlackMark(cv::Rect(centers[minj].x - width/2, centers[minj].y - width/2, width, width)))[0] == 255)

		{
			//the shape is close
			if (abs(centerWH[i].x + centerWH[i].y - centerWH[minj].x - centerWH[minj].y) < 100)
			{
				markCenter.push_back((centers[i] + centers[minj]) / 2);

				//update parameters
				tpm_markSize += minDst;
				tpm_blackMarkThreshold -= (image.at<uchar>(centers[i].y, centers[i].x) +
					image.at<uchar>(centers[minj].y, centers[minj].x) -
					imgBlur.at<uchar>(centers[i].y, centers[i].x) -
					imgBlur.at<uchar>(centers[minj].y, centers[minj].x)) / 4;
				tpm_whiteMarkThreshold += (image.at<uchar>(CD[0].y, CD[0].x) +
					image.at<uchar>(CD[1].y, CD[1].x) -
					imgBlur.at<uchar>(CD[0].y, CD[0].x) -
					imgBlur.at<uchar>(CD[1].y, CD[1].x)) / 4;

#ifdef DEBUG_MARK
				cv::circle(img, markCenter[markCenter.size() - 1], 10, cv::Scalar(15), 2);
			}

			std::cout << "dddddddddd\t" << centerWH[i] << std::endl;
			std::cout << "dddddddddd\t" << centerWH[minj] << std::endl;
#else
			}
#endif // DEBUG_MARK

		}
	}

#ifdef DEBUG_MARK
	cv::imwrite("bw.jpg",bw);
	cv::imwrite("img.jpg",img);
	cv::imwrite("imgWhiteMark.jpg",imgWhiteMark);
	cv::imwrite("imgBlackMark.jpg",imgBlackMark);
#endif // DEBUG_MARK
	if (markCenter.size()>=1)
	{
		int denominator = markCenter.size() + 1;
		m_markSize =tpm_markSize/ denominator;
		m_blackMarkThreshold =tpm_blackMarkThreshold/ denominator;
		m_whiteMarkThreshold =tpm_whiteMarkThreshold/ denominator;

		cv::Size zeroZone = cv::Size(-1, -1);
		cv::TermCriteria criteria = cv::TermCriteria(
			2,
			40, //maxCount=40  
			0.001);  //epsilon=0.001  
		cv::cornerSubPix(image, markCenter, cv::Size(m_markSize / 4 + 3, m_markSize / 4 + 3), zeroZone, criteria);

#ifdef DEBUG_MARK
		for (size_t i = 0; i < markCenter.size(); i++)
		{
			circle(img, markCenter[i], 4 + i * 4, cv::Scalar(225), 4);
			//circle(img, markCenter[i], 4 , cv::Scalar(225), 2);
		}
#endif // DEBUG_MARK

		//position = markCenter[0];
		position = markCenter;
	}
	else
	{
#ifdef DEBUG_MARK
		std::cout << "aaaaaaaaaaaaaaaaaaaaaaaa\t" << markCenter.size() << std::endl;
#endif // DEBUG_MARK
		position.clear();
	}
	return 0;
	}