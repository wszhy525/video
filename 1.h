#pragma once
#include <iostream>
#include<opencv2/opencv.hpp>
#include       <time.h>  
#include<string.h>
#include<sstream>
using namespace std;
using namespace cv;

#define maxRange 9000  //设置最大边框检测范围
#define minRange 600  //设置最小边框检测范围
#define BlueThreshold 55
#define GreenThreshold 55
#define RedThreshold 30
#define Blue 145
#define Green 145
#define Red 225
//因为规则文件中没有显示蓝色边框，暂时只做了红色的识别，等有实物或者视频再来调整阈值加上蓝色识别
Point2f circlarCore;
Point2f rectangleCore;
int x = 1;
ofstream mycout("test.txt");
class Circle {
public:
	Point2f circleCore;
	bool isClockwise;
};
void colorReduce(Mat & inputImg, Mat &outputImg, int div)//因为输入的是256色的图片，通过这个函数减少颜色数量，便于以后区分
{
	outputImg = inputImg.clone();
	int row = outputImg.rows; //行数
	int col = outputImg.cols*outputImg.channels();  //列数*通道数=每一行元素的个数
	//遍历像素
	for (int i = 0; i < row; i++)
	{
		uchar *data = outputImg.ptr<uchar>(i);
		for (int j = 0; j < col; j++)
		{
			data[j] = data[j] / div * div + div / 2;//减少像素个数
			//经过对规则图片的分析，我发现线框部分的rgb值区域在（R>200，B>100，G>100+），白色区域是（200+，200+，200+），而其他杂质区域都在几十左右。我希望以此来处理图像

		}
	}

}
//Blue  Green Red
Vec3b target = Vec3b(Blue, Green, Red);//目标颜色区域，在规则书上是(135, 135, 242)，这个区域还要经过实际的摄像头处理后改吧。
//如果是蓝色，需要再弄一个target1;
bool getDistance(const Vec3b& color)
{
	//因为我是每32区别一次bgr，阈值设置为33可以将周围可能的发光区域包含在内，也适用于实际摄像头的散光问题。
	return abs(color[2] - target[2]) < RedThreshold&&abs(color[1] - target[1]) < GreenThreshold&&abs(color[0] - target[0]) < BlueThreshold;
}

Point2f calculaterectangleCore(Mat src)//计算输入图片要打击的装甲版的中心点
{
	Mat outputImg;
	colorReduce(src, outputImg, 16);
	Mat resultImg(outputImg.size(), CV_8U, Scalar(0)); //初始化一张和原图等大的二值化图来装载红色结果区域。
	Mat_ <cv::Vec3b> ::const_iterator it = outputImg.begin<Vec3b>();
	Mat_ <cv::Vec3b> ::const_iterator itend = outputImg.end<Vec3b>();
	Mat_ <uchar>::iterator itout = resultImg.begin<uchar>();
	for (it; it != itend; ++it, ++itout)
	{
		if (getDistance(*it))
		{
			*itout = 255;
		}
		else
		{
			*itout = 0;
		}
	}
	threshold(resultImg, resultImg, 100, 255, THRESH_TRIANGLE);//大津法
	//滤波过滤掉噪声，去掉指示等的小灯光产生的噪声
	medianBlur(resultImg, resultImg, 3);
	//将要打击的装甲板的颜色要比已经打击的装甲板更亮，所以经过滤波之后已经打击过的装甲板会有一部分成分的残缺，这不用管他，可以还会对后面的识别有所帮助。
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(resultImg, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	Mat draw_img(resultImg.rows, resultImg.cols, CV_8UC1, Scalar(255));
	drawContours(draw_img, contours, -1, Scalar(0), 3);
	Mat srcImage(outputImg.size(), CV_8U, Scalar(0));
	cv::Point2f points4[4];
	for (auto contour : contours)
	{
		RotatedRect rotated_rect = minAreaRect(contour);
		rotated_rect.points(points4);
		rectangleCore = rotated_rect.center;
		for (int i = 0; i < 4; i++)
			// 注意Scala中存储顺序 BGR
			line(src, points4[i], points4[(i + 1) % 4], Scalar(255, 0, 0), 5);
		cout << contourArea(contour) << endl;
		if (contourArea(contour) > minRange&& contourArea(contour) < maxRange)
		{
			RotatedRect rotated_rect = minAreaRect(contour);
			rotated_rect.points(points4);
			for (int i = 0; i < 4; i++)
				// 注意Scala中存储顺序 BGR
				line(src, points4[i], points4[(i + 1) % 4], Scalar(255, 255, 0), 5);
			float y = rotated_rect.size.height / rotated_rect.size.width;
			if (y<1.8&&y>0.56) {
				rectangleCore = rotated_rect.center;
				for (int i = 0; i < 4; i++)
					// 注意Scala中存储顺序 BGR
					line(src, points4[i], points4[(i + 1) % 4], Scalar(0, 255, 0), 5);
				circle(src, rectangleCore, 5, Scalar(0,0,255));
			}
		}
	}
	
	string b = "abc";
	string c = ".png";
	stringstream ss;
	ss << b << x << c;
	//imshow(ss.str(), src);
	//imwrite(ss.str(), src);
	x++;
	cout << "矩形中心" << rectangleCore << endl;
	mycout<< "矩形中心" << rectangleCore << endl;
	mycout.close();
	return rectangleCore;
	
};

Circle circle1;

Circle calculateCirclarCore(Mat src1, Mat src2, Mat src3)
{

	Point2f pt1 = calculaterectangleCore(src1);
	Point2f pt2 = calculaterectangleCore(src2);
	Point2f pt3 = calculaterectangleCore(src3);
	float A1, A2, B1, B2, C1, C2, temp;
	A1 = pt1.x - pt2.x;
	B1 = pt1.y - pt2.y;
	C1 = (pow(pt1.x, 2) - pow(pt2.x, 2) + pow(pt1.y, 2) - pow(pt2.y, 2)) / 2;
	A2 = pt3.x - pt2.x;
	B2 = pt3.y - pt2.y;
	C2 = (pow(pt3.x, 2) - pow(pt2.x, 2) + pow(pt3.y, 2) - pow(pt2.y, 2)) / 2;
	//为了方便编写程序，令temp = A1*B2 - A2*B1
	temp = A1 * B2 - A2 * B1;
	//定义一个圆的数据的结构体对象CD
	Point2f CD;
	//判断三点是否共线
	if (temp == 0) {
		//共线则将第一个点pt1作为圆心
		CD.x = pt1.x;
		CD.y = pt1.y;
	}
	else {
		//不共线则求出圆心：
		//center.x = (C1*B2 - C2*B1) / A1*B2 - A2*B1;
		//center.y = (A1*C2 - A2*C1) / A1*B2 - A2*B1;
		CD.x = (C1*B2 - C2 * B1) / temp;
		CD.y = (A1*C2 - A2 * C1) / temp;
	}
	Mat src4 = src3.clone();
	circle(src4, CD, 5, Scalar(0, 255, 0));
	imshow("abc4", src4);
	cout << "圆心中心为" << CD << endl;
	circle1.circleCore = CD;
	double ans = (pt2.x - pt1.x)*(pt3.y - pt1.y) - (pt2.y - pt1.y)*(pt3.x - pt1.x);//表示向量AB与AC的叉积的结果 
	if (ans < 0)
	{
		circle1.isClockwise = 0;
		cout << "逆时针" << endl;
	}
	if (ans > 0)
	{
		circle1.isClockwise = 1;
		cout << "顺时针" << endl;
	}
	if (ans == 0)

		cout << "共线" << endl;

	return circle1;
}

void videoProgress() {
	VideoCapture capture("2.mp4");
	while (1)
	{
		Mat frame;
		capture >> frame;
		if (frame.empty())
		{
			break;
		}
		calculaterectangleCore(frame);
		imshow("读取视频", frame);
		waitKey(1);
	}
}

