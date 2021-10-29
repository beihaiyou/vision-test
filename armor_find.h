#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>


// #define TEST 1 //是否测试

#define BLUE 0
#define RED 1

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#define BLUE 0
#define RED 1

struct Ptz
{
  float x_os;
  float y_os;
  float z_os;
  float pitch_os_a;
  float pitch_os_b;
  float yaw_os;
};

struct Camera
{
  cv::Mat IntrinsicMatrix;
  cv::Mat Distortion;
  std::vector<cv::Point3f> points_object_max;
  std::vector<cv::Point3f> points_object_min;
  std::vector<cv::Point3f> points_pos_max;
  std::vector<cv::Point3f> points_pos_min;
};

class Parameters
{
public:
  Parameters();

  int enemy_color;
  int video;
  float area_max;
  float area_min;
  float angle_max;
  float angle_min;
  float ratio_max;
  float ratio_min;
  float ratio_g;
  float ratio_l;
  float armor_plat_width;
  float armor_plat_height;
  float armor_pos_width;
  float armor_pos_height;
  float weight_max;
  int blue_gray_min;
  int red_gray_min;
  int blue_color_min;
  int red_color_min;

  Ptz ptz;
  Camera camera;


};

Parameters::Parameters()
{
    enemy_color = BLUE;
    blue_gray_min = 127;
    red_gray_min = 110;
    blue_color_min = 100;
    red_color_min = 60;
    area_max = 2000;
    area_min = 0;
    angle_max = 65;
    angle_min = 27;
    ratio_max = 3.9;
    ratio_min = 1.2;
    ratio_g = 30;
    ratio_l = 1.7;
    armor_plat_width = 0.130;
    armor_plat_height = 0.55;
    weight_max = 0.6;

}

Parameters param;

void ipp(cv::Mat &InputImage, cv::Mat &OutputImage)
{
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1,5));
    cv::Mat Gray, Ass;

    std::vector<cv::Mat> channels(3);
    cv::split(InputImage, channels);

    Gray = 0.114 * channels[0] + 0.587* channels[1] + 0.299 * channels[2];//不用opcv函数，直接用权值计算的形式得到灰度图
    cv::blur(Gray, Gray, cv::Size(2, 5));//定义一个尺寸矩阵用于高斯滤波

    if (param.enemy_color)
    {
        Gray = (Gray >= param.red_gray_min);
        OutputImage = ((channels[2] - channels[1]/2 - channels[0]/2)>param.red_color_min);
        Ass = ((channels[0] - channels[1]/2 - channels[2]/2)>param.blue_color_min);
    }
    else
    {
        Gray = (Gray >= param.blue_gray_min);//直接用矩阵来判断每一个矩阵元素是否大于阈值，来做二值化处理，得到灰度阵
        OutputImage = ((channels[0] - channels[1]/2 - channels[2]/2)> param.blue_color_min);//二值化，得到蓝色阵
        Ass = ((channels[2] - channels[1]/2 - channels[0]/2)> param.red_color_min);//未知作用，掩模？
    }

    cv::dilate(OutputImage, OutputImage,element);//腐蚀
    std::vector<std::vector<cv::Point>> contours;//存储findcontours结果，二重向量
    cv::findContours(OutputImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);//最外轮廓，仅保存轮廓的拐点信息
    std::vector<cv::Point> scontour; //装若干点的向量，向量的每一个元素内存储边框的所有边缘点
    //contours是二重向量，auto自动推导contour为一重向量
    for(auto contour : contours)//auto是类型推导，不用在编译时决定类型，而在运行时推导类型，使c++有了类似python的动态类型,for(:)遍历数组的新方法
    {
        scontour.insert(scontour.end(), contour.begin(), contour.end());//向量就是迭代器，insert是针对迭代器的方法，根据输入参数的类型不同，有不同的重载
    }
    cv::Rect rect = cv::boundingRect(scontour);//把scontour的边缘生成最小正方形赋予rect，rect只有左上点和长与宽
    cv::Mat ROI_OUT = OutputImage(rect);//针对Rect的重载，只读取rect区域的元素
    Gray(rect).copyTo(ROI_OUT);//把区域内的灰度图的值盖在
#ifdef TEST//如果宏定义了TEST就执行以下程序
    cv::rectangle(Image0d, rect, cv::Scalar(255, 0, 0), 10);
#endif
    contours.clear();//清空蓝色二值图的轮廓点集
    cv::findContours(Ass, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);//找红色二值图的轮廓
    if(!contours.empty())//如果有轮廓
    {
        scontour.clear();//清空
        for(auto contour : contours)
        {
            scontour.insert(scontour.end(), contour.begin(), contour.end());
        }
        cv::Rect rect_ass = cv::boundingRect(scontour);
        if(rect_ass.area() <rect.area())
        {
            ROI_OUT = OutputImage(rect_ass) > 255;
        }
    }
}

bool judge_h(const float &angle, const cv::Size2f &size)//判断长与宽的大小，标准化旋转矩形
{
    if(-angle > param.angle_max)//angle一定为负，65，27
    {
        return size.width < size.height;
    }
    if(-angle < param.angle_min)
    {
        return size.width > size.height;
    }
    return 0;    
}

bool judge_v(const float &angle, const cv::Size2f &size)//判断长与宽的大小，标准化旋转矩形
{
    if(-angle > param.angle_max)
    {
        return size.width < size.height;
    }
    if(-angle < param.angle_min)
    {
        return size.width > size.height;
    }
    return 0;    
}

float getRatio(cv::RotatedRect &box)//归一化后，计算长宽比
{
    if(-box.angle > param.angle_max)
    {
        return box.size.height / box.size.width;
    }
    if(-box.angle < param.angle_min)
    {
        return box.size.width / box.size.height;
    }
    
}

struct Weight//自行定义了一个权重结构体，用于储存boxs的权重
{
    Weight() = default;
    Weight(size_t i, size_t j, float w, float r) : index1(i), index2(j), weight(w), ratio(r) {}
    size_t index1;
    size_t index2;
    float weight;
    float ratio;
};

//根据处理后的图像找出最优的装甲版，且只保留一个
bool glb(cv::Mat &InputIamge, std::vector<cv::RotatedRect> &boxs, std::vector<Weight> &boxs_weight, bool &cr_mode)//cr_mode用于后续函数，boxs_weight用于储存上一条矩形信息
{
#ifdef DDBUG//如果是DDBUG模式
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "get lampbar:";
#endif
    boxs.clear();
    boxs_weight.clear();//每次用的是来自外部的变量，需要清空原始数据
    std::vector<std::vector<cv::Point>>contours;
    cv::findContours(InputIamge, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::RotatedRect> lampbars;
    cr_mode = 0;

    if(contours.size() > 1)//有两个以上的灯条在进行判断，筛取有效灯条
    {
        float area;

        for(size_t i = 0; i < contours.size(); ++i)
        {
#ifdef DEBUG
            std::cout<< "\n\t" << i + 1 << "  "
                <<"contoursArea  " << cv::contourArea(contour[i]);//输出每个灯条的大小
#endif // DEBUG

            area = cv::contourArea(contours[i]);
            if(area > param.area_min && area < param.area_max)//轮廓面积满足要求
            {
                cv::RotatedRect box = cv::minAreaRect(contours[i]);//求最小矩形
#ifdef DDBUG//
                std::cout << "\tangle  " << box.angle << std::endl;//输出矩形的角度
                circle(InputImage, box.center, 3, cv::Scalar(155), -1, 8, 0);//在原图中画出矩形中心
#endif
                if(!cr_mode && box.center.y > 400)//如果矩形中心低于400,让cr_mode为1
                {
                    cr_mode = 1;
                }
                if(judge_v(box.angle, box.size))//判断是不是一个竖起来的矩形?
                {
                    if (-box.angle > param.angle_max)//如果角度也满足要求，角度修正后，压入栈
                    {
                        box.angle = box.angle + 91; //in order to distinguish -90 and -0, plus 1
                        lampbars.push_back(std::move(box));
                    }
                    else
                    {
                        lampbars.push_back(std::move(box));//转换为右值引用
                    }
                }
            }
        }
    }

    if(!lampbars.empty())//如果有筛取到的灯条
    {
#ifdef DDBUG
        std::cout << "\nget the pre_plat:";
#endif
        bool exist;
        float ratio, ratio_g, ratio_l, weight;

        for(size_t i = 0; i < lampbars.size(); ++i)//size_t一种整型类型，从这个矩形的下一个开始遍历
        {
            exist = 0;
            for (size_t j = i + 1; j < lampbars.size(); ++j)
            {
                ratio = std::max(lampbars[i].size.width, lampbars[i].size.height);//储存这个矩形的长
                ratio_g = std::max(lampbars[j].size.width, lampbars[j].size.height);//储存下一个矩形的长
                ratio_l = std::max(ratio_g, ratio) / std::min(ratio_g, ratio);//二者的比例
                if(ratio_l > param.ratio_l)//如果长的比例不满足要求，放弃这个灯条的矩形
                {
                    continue;
                }
                ratio_g = fabs(lampbars[i].angle - lampbars[j].angle);//旋转角度的绝对差值
#ifdef DDBUG//输出三个比例
                std::cout << "\n\t* angle:  " << lampbars[i].angle << "," << lampbars[j].angle;
                std::cout << "\t\t ratio_g:  " << ratio_g;
                std::cout << "\t\t ratio_l:  " << ratio_l;
#endif
                if(ratio_g < param.ratio_g)//如果角度绝对差值满足，计算出上宽中心点，下宽中心点，并描出一个装甲版的长方形
                {
                    float sin_, cos_, theta;
                    std::vector<cv::Point> v_lampbar;
                    if(lampbars[i].angle > 0)//如果角度大于零
                    {
                        theta = (lampbars[i].angle - 1) * CV_PI / 180;//角度转弧度
                        sin_ = lampbars[i].size.width * sinf(theta) / 2;//返回浮点量的sin
                        cos_ = lampbars[i].size.width * cosf(theta) / 2;
                        v_lampbar.push_back(cv::Point(round(lampbars[i].center.x + cos_), round(lampbars[i].center.y + sin_)));
                        v_lampbar.push_back(cv::Point(round(lampbars[j].center.x - sin_), round(lampbars[j].center.y + cos_)));
                    }
                    else
                    {
                        theta = (90 + lampbars[j].angle) * CV_PI / 180;
                        sin_ = lampbars[j].size.height * sinf(theta) / 2;
                        cos_ = lampbars[j].size.height * cosf(theta) / 2;
                        v_lampbar.push_back(cv::Point(round(lampbars[j].center.x + cos_), round(lampbars[j].center.y + sin_)));
                        v_lampbar.push_back(cv::Point(round(lampbars[j].center.x - cos_), round(lampbars[j].center.y - sin_)));
                    }
                    if (lampbars[j].angle > 0)
                    {
                        theta = (lampbars[j].angle - 1) * CV_PI / 180; //eliminate the plus of 1
                        sin_ = lampbars[j].size.width * sinf(theta) / 2;
                        cos_ = lampbars[j].size.width * cosf(theta) / 2;
                        v_lampbar.push_back(cv::Point(round(lampbars[j].center.x + sin_), round(lampbars[j].center.y - cos_)));
                        v_lampbar.push_back(cv::Point(round(lampbars[j].center.x - sin_), round(lampbars[j].center.y + cos_)));
                    }
                    else
                    {
                        theta = (90 + lampbars[j].angle) * CV_PI / 180;
                        sin_ = lampbars[j].size.height * sinf(theta) / 2;
                        cos_ = lampbars[j].size.height * cosf(theta) / 2;
                        v_lampbar.push_back(cv::Point(round(lampbars[j].center.x + cos_), round(lampbars[j].center.y + sin_)));
                        v_lampbar.push_back(cv::Point(round(lampbars[j].center.x - cos_), round(lampbars[j].center.y - sin_)));
                    }
                    cv::RotatedRect box = cv::minAreaRect(v_lampbar);
                    ratio = getRatio(box);
#ifdef DEBUG
                    std::cout << "\t\tratio:" << ratio;
#endif // DEBUG 
                    if(ratio && ratio > param.ratio_min && ratio < param.ratio_max)
                    {
#ifdef DEBUG
                        draw_rect(InputImage, box, cv::Scalar(155));
                        std::cout << "\tweight: " << (ratio_g / Param.ratio_g) * 0.75 + ((ratio_l - 1) / (Param.ratio_l - 1)) * (1 - 0.75)
                                  << "\t^_^\n";
#endif // DEBUG
                        weight = (ratio_g / param.ratio_g) * 0.8 + ((ratio_l - 1)) / (param.ratio_l - 1) * (1-0.8);//将角度差值参数和长度差值参数归一化后存储在weight中，角度的权重更大一些
                        if(exist)//如果不是第一个，比较，如果角度差值小于上一个，就把装甲版矩形替换为差值更小的，保证每次只留下一个，然后用于后续计算
                        {
                            if(ratio < boxs_weight.back().weight)
                            {
                                boxs.back() = box;
                                boxs_weight.back() =Weight(i, j, weight, ratio);
                            }
                        }
                        else//如果是第一个压入，向量类型都
                        {
                            exist = 1;
                            boxs.push_back(std::move(box));
                            boxs_weight.push_back(Weight(i, j, weight, ratio));
                        }
                    }
                }
            }
        }
#ifdef DDBUG
        std::cout << std::endl;
#endif
    }
    else
    {
        return 0;
    }
    
    return 1;
}