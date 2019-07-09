#include <opencv2/opencv.hpp>
using namespace cv;

class SplitID
{
private:
    std::vector<Mat> name;
    std::vector<Mat> sex;
    std::vector<Mat> born;
    std::vector<Mat> address;
    std::vector<Mat> id;
    Mat srcImage;
public:
    SplitID(/* args */);
    void setImage(Mat _img);
    std::vector<Mat> getRegionName();
    std::vector<Mat> getRegionSex();
    std::vector<Mat> getRegionBorn();
    std::vector<Mat> getRegionAddress();
    std::vector<Mat> getRegionId();
    ~SplitID();
};

SplitID::SplitID(/* args */)
{
}

SplitID::~SplitID()
{
}
