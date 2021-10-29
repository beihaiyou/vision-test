#include<opencv2/highgui/highgui.hpp>
#include<opencv2/highgui/highgui_c.h>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>

#include "armor_find.h"

using namespace cv;
using namespace std; 

#define REDENEMY 2
#define BLUEENEMY 0


int main()
{
	void ipp(cv::Mat &InputImage, cv::Mat &OutputImage);
	
	VideoCapture capture("/home/liangzh/桌面/opencv 素材/5armor1.mkv");
    
	while (1)
	{

        Mat frame;
        Mat drawing;
		capture >> frame;
		
        ipp(frame, drawing);
        imshow("box", drawing);
		waitKey(5);


	}
	return 0;
}