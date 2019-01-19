#include <iostream>
#include<opencv2/opencv.hpp>
#include       <time.h>  
#include<string.h>
#include<sstream>
#include"1.h"
using namespace std;
using namespace cv;

int main()
{ 
	char name;
	cout << "input the name of video" << endl;
	//cin >> name;
	videoProgress();
	waitKey(0);
}


