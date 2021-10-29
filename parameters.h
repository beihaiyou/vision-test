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
}




