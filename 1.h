#pragma once
#include <iostream>
#include<opencv2/opencv.hpp>
#include       <time.h>  
#include<string.h>
#include<sstream>
using namespace std;
using namespace cv;

#define maxRange 10000  //�������߿��ⷶΧ
#define minRange 6000  //������С�߿��ⷶΧ
#define BlueThreshold 55
#define GreenThreshold 55
#define RedThreshold 30
#define Blue 145
#define Green 145
#define Red 225
//��Ϊ�����ļ���û����ʾ��ɫ�߿���ʱֻ���˺�ɫ��ʶ�𣬵���ʵ�������Ƶ����������ֵ������ɫʶ��
Point2f circlarCore;
Point2f rectangleCore;
int x = 1;
ofstream mycout("test.txt");
class Circle {
public:
	Point2f circleCore;
	bool isClockwise;
};
void colorReduce(Mat & inputImg, Mat &outputImg, int div)//��Ϊ�������256ɫ��ͼƬ��ͨ���������������ɫ�����������Ժ�����
{
	outputImg = inputImg.clone();
	int row = outputImg.rows; //����
	int col = outputImg.cols*outputImg.channels();  //����*ͨ����=ÿһ��Ԫ�صĸ���
	//��������
	for (int i = 0; i < row; i++)
	{
		uchar *data = outputImg.ptr<uchar>(i);
		for (int j = 0; j < col; j++)
		{
			data[j] = data[j] / div * div + div / 2;//�������ظ���
			//�����Թ���ͼƬ�ķ������ҷ����߿򲿷ֵ�rgbֵ�����ڣ�R>200��B>100��G>100+������ɫ�����ǣ�200+��200+��200+�������������������ڼ�ʮ���ҡ���ϣ���Դ�������ͼ��

		}
	}

}
//Blue  Green Red
Vec3b target = Vec3b(Blue, Green, Red);//Ŀ����ɫ�����ڹ���������(135, 135, 242)���������Ҫ����ʵ�ʵ�����ͷ�����İɡ�
//�������ɫ����Ҫ��Ūһ��target1;
bool getDistance(const Vec3b& color)
{
	//��Ϊ����ÿ32����һ��bgr����ֵ����Ϊ33���Խ���Χ���ܵķ�������������ڣ�Ҳ������ʵ������ͷ��ɢ�����⡣
	return abs(color[2] - target[2]) < RedThreshold&&abs(color[1] - target[1]) < GreenThreshold&&abs(color[0] - target[0]) < BlueThreshold;
}

Point2f calculaterectangleCore(Mat src)//��������ͼƬҪ�����װ�װ�����ĵ�
{
	Mat outputImg;
	colorReduce(src, outputImg, 16);
	Mat resultImg(outputImg.size(), CV_8U, Scalar(0)); //��ʼ��һ�ź�ԭͼ�ȴ�Ķ�ֵ��ͼ��װ�غ�ɫ�������
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
	threshold(resultImg, resultImg, 100, 255, THRESH_TRIANGLE);//���
	//�˲����˵�������ȥ��ָʾ�ȵ�С�ƹ����������
	medianBlur(resultImg, resultImg, 3);
	//��Ҫ�����װ�װ����ɫҪ���Ѿ������װ�װ���������Ծ����˲�֮���Ѿ��������װ�װ����һ���ֳɷֵĲ�ȱ���ⲻ�ù��������Ի���Ժ����ʶ������������
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
			// ע��Scala�д洢˳�� BGR
			line(src, points4[i], points4[(i + 1) % 4], Scalar(255, 0, 0), 5);
		cout << contourArea(contour) << endl;
		if (contourArea(contour) > minRange&& contourArea(contour) < maxRange)
		{
			RotatedRect rotated_rect = minAreaRect(contour);
			rotated_rect.points(points4);
			float x = rotated_rect.size.height / rotated_rect.size.width;
			if (x<2.1&&x>0.48) {
				rectangleCore = rotated_rect.center;
				for (int i = 0; i < 4; i++)
					// ע��Scala�д洢˳�� BGR
					line(src, points4[i], points4[(i + 1) % 4], Scalar(0, 255, 0), 5);
			}
		}
	}
	circle(src, rectangleCore, 5, Scalar(0,0,255));
	string b = "abc";
	string c = ".png";
	stringstream ss;
	ss << b << x << c;
	//imshow(ss.str(), src);
	//imwrite(ss.str(), src);
	x++;
	cout << "��������" << rectangleCore << endl;
	mycout<< "��������" << rectangleCore << endl;
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
	//Ϊ�˷����д������temp = A1*B2 - A2*B1
	temp = A1 * B2 - A2 * B1;
	//����һ��Բ�����ݵĽṹ�����CD
	Point2f CD;
	//�ж������Ƿ���
	if (temp == 0) {
		//�����򽫵�һ����pt1��ΪԲ��
		CD.x = pt1.x;
		CD.y = pt1.y;
	}
	else {
		//�����������Բ�ģ�
		//center.x = (C1*B2 - C2*B1) / A1*B2 - A2*B1;
		//center.y = (A1*C2 - A2*C1) / A1*B2 - A2*B1;
		CD.x = (C1*B2 - C2 * B1) / temp;
		CD.y = (A1*C2 - A2 * C1) / temp;
	}
	Mat src4 = src3.clone();
	circle(src4, CD, 5, Scalar(0, 255, 0));
	imshow("abc4", src4);
	cout << "Բ������Ϊ" << CD << endl;
	circle1.circleCore = CD;
	double ans = (pt2.x - pt1.x)*(pt3.y - pt1.y) - (pt2.y - pt1.y)*(pt3.x - pt1.x);//��ʾ����AB��AC�Ĳ���Ľ�� 
	if (ans < 0)
	{
		circle1.isClockwise = 0;
		cout << "��ʱ��" << endl;
	}
	if (ans > 0)
	{
		circle1.isClockwise = 1;
		cout << "˳ʱ��" << endl;
	}
	if (ans == 0)

		cout << "����" << endl;

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
		imshow("��ȡ��Ƶ", frame);
		waitKey(5);
	}
}

