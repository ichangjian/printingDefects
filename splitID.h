#include <opencv2/opencv.hpp>
using namespace cv;
struct Region
{
    std::vector<cv::Mat> image;
    std::vector<cv::Rect> rect;
};
class SplitID
{
private:
    std::vector<std::vector<Region> > word_regions;

    Mat src_image;
    Region card_LU;
    Region card_RU;
    Region card_LRD;

    int flag_front_back;
    cv::Rect picture_rect=Rect(423,79,200,250);
    std::vector<Region> getNoBlankRegions(const Mat &_image, const Rect &_rect);
    void mergeMinorRegion(const Mat &_image, Region &_region);
    int minor_width=20;

public:
    SplitID(int _mode);
    SplitID();
    void setImage(const Mat &_img);
    std::vector<std::vector<Region> > getWordRegions();
    Region getPictureRegion();
    cv::Mat getBlankImage();
    ~SplitID();
};

SplitID::SplitID(int _mode)
{
    flag_front_back = _mode;
}
SplitID::SplitID()
{
}

SplitID::~SplitID()
{
}

void SplitID::setImage(const Mat &_img)
{
    src_image = _img.clone();
    picture_rect=Rect(src_image.cols*0.6,src_image.rows/6,src_image.cols/3,src_image.rows*0.57);

    if (flag_front_back == 1) //front
    {
        card_LU.image.push_back(src_image(Rect(0, 20, picture_rect.x, picture_rect.y + picture_rect.height)).clone());
        card_RU.image.push_back(src_image(picture_rect).clone());
        card_LRD.image.push_back(src_image(Rect(0, picture_rect.y + picture_rect.height, src_image.cols,
                                                src_image.rows - picture_rect.y - picture_rect.height))
                                     .clone());
        card_LU.rect.push_back(Rect(0, 20, picture_rect.x, picture_rect.y + picture_rect.height));
        card_RU.rect.push_back(picture_rect);
        card_LRD.rect.push_back(Rect(0, picture_rect.y + picture_rect.height, src_image.cols,
                                     src_image.rows - picture_rect.y - picture_rect.height));
    }
    rectangle(src_image,picture_rect,Scalar(122));
}

double math_coefficient(int a[], int b[], int N)
{
	double Sum_xy = 0;
	double Sum_x =0;
	double Sum_y = 0;
	double Sum_x2 = 0;
	double Sum_y2 = 0;
	for(int i=0; i<N; i++)
	{
		
		Sum_xy += a[i]*b[i];
		Sum_x += a[i];
		Sum_y += b[i];
		Sum_x2 += a[i]*a[i];
		Sum_y2 += b[i]*b[i];
	}
 
	return (Sum_xy*N-Sum_x*Sum_y)/sqrt((N*Sum_x2-pow(Sum_x,2))*(N*Sum_y2-pow(Sum_y,2)));
}


Region getNoBlankRegion(const Mat &_image, int is_row)
{
    std::vector<float> mean_values;
    Mat image = _image;
    if (is_row == 1)
    {
        for (int i = 0; i < image.rows; i++)
        {
            mean_values.push_back(mean(image.rowRange(i, i + 1))[0]);
        }
    }
    else
    {
        for (int i = 0; i < image.cols; i++)
        {
            mean_values.push_back(mean(image.colRange(i, i + 1))[0]);
        }
    }

    std::vector<int> noblank_indexs;
    for (size_t i = 1; i < mean_values.size(); i++)
    {
        if (mean_values[i - 1] ==255 && mean_values[i] < 255)
        {
            noblank_indexs.push_back(i);
        }
        else if (mean_values[i] ==255 && mean_values[i - 1] < 255)
        {
            noblank_indexs.push_back(i);
        }
    }
    if (is_row == 1)
    {
        Region row_images;
        for (size_t i = 0; i < noblank_indexs.size() - 1; i += 2)
        {
            Rect rect(0, noblank_indexs[i], image.cols, noblank_indexs[i + 1] - noblank_indexs[i]);
            row_images.image.push_back(image(rect));
            row_images.rect.push_back(rect);
        }
        return row_images;
    }
    else
    {
        Region col_images;
        for (size_t i = 0; i < noblank_indexs.size() - 1; i += 2)
        {
            Rect rect(noblank_indexs[i],0, noblank_indexs[i + 1] - noblank_indexs[i], image.rows);
            col_images.image.push_back(image(rect));
            col_images.rect.push_back(rect);
        }
        return col_images;
    }
}

std::vector<Region> SplitID::getNoBlankRegions(const Mat &_image, const Rect &_rect)
{
    std::vector<float> mean_values;
    Mat image = _image;
    imshow("lu", image);
    waitKey();

    Region row_images = getNoBlankRegion(image, 1);
    for (size_t i = 0; i < row_images.image.size(); i++)
    {
        char c[100];
        sprintf(c, "temp/aa_%d.png", i);
        imwrite(c, row_images.image[i]);
        row_images.rect[i].x += _rect.x;
        row_images.rect[i].y += _rect.y;
    }

    std::vector<Region> regions;
    for (size_t i = 0; i < row_images.image.size(); i++)
    {

        Region col_images = getNoBlankRegion(row_images.image[i], 0);
        mergeMinorRegion(row_images.image[i],col_images);
        for (size_t j = 0; j < col_images.image.size(); j++)
        {
            char c[100];
            sprintf(c, "temp/aa_%d_%d.png", i, j);
            imwrite(c, col_images.image[j]);
            col_images.rect[j].x += row_images.rect[i].x;
            col_images.rect[j].y += row_images.rect[i].y;
            rectangle(src_image, col_images.rect[j], Scalar(125), 1);
        }
        regions.push_back(col_images);
    }
    imwrite("temp/aa_lu.png", src_image);
    return regions;
}

void SplitID::mergeMinorRegion(const Mat &_image, Region &_region)
{
    if (_region.image.size() < 2)
    {
        return;
    }

    std::vector<int> minor_indexs;
    for (size_t i = 0; i < _region.rect.size(); i++)
    {
        if (_region.rect[i].width < minor_width)
        {
            minor_indexs.push_back(i);
        }
    }
    if (minor_indexs.size() == 0)
    {
        return;
    }

    std::vector<int> minor_flag(_region.rect.size(), 1);
    //near
    for (size_t i = 0; i < minor_indexs.size(); i++)
    {
        int curr = minor_indexs[i];
        minor_flag[curr] = 2;
        size_t j = i + 1;
        for (; j < minor_indexs.size(); j++)
        {
            if (curr == minor_indexs[j] - 1)
            {
                minor_flag[minor_indexs[j]] = 0;
                minor_flag[minor_indexs[i]] = 3;
                _region.rect[minor_indexs[i]].width = _region.rect[minor_indexs[j]].x - _region.rect[minor_indexs[i]].x +
                                                      _region.rect[minor_indexs[j]].width;
                curr = minor_indexs[j];
            }
            else
            {
                break;
            }
        }
        i = j - 1;
    }

    Region merge_region;
    //begin
    if (minor_indexs[0] == 0)
    {
        if (minor_flag[1] == 1) //0 and 1 is minor
        {
            _region.rect[0].width = _region.rect[1].x - _region.rect[0].x +
                                    _region.rect[1].width;
            minor_flag[0] = 1;
            minor_flag[1] = 0;
        }
    }
    //end
    if ((minor_indexs[minor_indexs.size() - 1] == _region.rect.size() - 1) && (minor_flag[_region.rect.size() - 1] == 2))
    {
        _region.rect[_region.rect.size() - 2].width = _region.rect[_region.rect.size() - 1].x - _region.rect[_region.rect.size() - 2].x +
                                                      _region.rect[_region.rect.size() - 1].width;
        minor_flag[_region.rect.size() - 1] = 0;
    }
    //middle
    for (size_t i = 0; i < _region.rect.size() - 1; i++)
    {
        if (minor_flag[i] == 0)
        {
            continue;
        }
        if (minor_flag[i] == 1 && minor_flag[i + 1] != 2)
        {
            merge_region.image.push_back(_region.image[i]);
            merge_region.rect.push_back(_region.rect[i]);
        }
        else if (minor_flag[i] == 1 && minor_flag[i + 1] == 2)
        {
            _region.rect[i].width = _region.rect[i + 1].x - _region.rect[i].x +
                                    _region.rect[i + 1].width;
            minor_flag[i + 1] = 0;
            merge_region.image.push_back(_image(_region.rect[i]));
            merge_region.rect.push_back(_region.rect[i]);
        }
        else if (minor_flag[i] == 3)
        {
            merge_region.image.push_back(_image(_region.rect[i]));
            merge_region.rect.push_back(_region.rect[i]);
        }
    }
    if (minor_flag[minor_flag.size() - 1] == 1)
    {

        merge_region.image.push_back(_region.image[minor_flag.size() - 1]);
        merge_region.rect.push_back(_region.rect[minor_flag.size() - 1]);
    }
    _region = merge_region;
}

std::vector<std::vector<Region> > SplitID::getWordRegions()
{
    //LU
    getNoBlankRegions(card_LU.image[0],card_LU.rect[0]);
    getNoBlankRegions(card_LRD.image[0],card_LRD.rect[0]);
    //LRD
    std::vector<Region> region;

    return word_regions;
}

class SplitFrontID : public SplitID
{
private:
    Mat image_LU;
    Mat image_RU;
    Mat image_LRD;

public:
    SplitFrontID(/* args */);
    ~SplitFrontID();
};

SplitFrontID::SplitFrontID(/* args */)
{
    getWordRegions();
}

SplitFrontID::~SplitFrontID()
{
}
