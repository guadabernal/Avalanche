#include <unistd.h>
#include "comm_utils.h"
#include "opencv2/opencv.hpp"
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdio.h>

using namespace std;
using namespace cv;

void readValues(){
  std::ifstream valuesInFile("output3/values15.txt", ios::binary);
  ArduinoValues st1;
  while(!valuesInFile.eof()) {
      valuesInFile.read((char*)&st1, sizeof(st1));
      printf("frame = %d, ml = %d mr = %d\n", st1.frameNumber, st1.leftMotor, st1.rightMotor);
  }
}

void view(){}

void record(const std::string& connect, const std::string& file, const std::string& outputval){
  std::cout<< file <<std::endl;
  SerialComm arduino;
  arduino.openPort(connect);
  using namespace cv;
  VideoCapture cap(1);

  int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
  int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
  VideoWriter video(file, CV_FOURCC('M', 'J', 'P', 'G'), 30,
                    Size(frame_width, frame_height), true);
  std::ofstream valuesFile(outputval.c_str(), ios::binary);

  int16_t v[40];
  bool done = false;
  double t0 = millis();
  int frameNumber = 0;
  while (!done) {
    Mat frame;
    cap >> frame;
    double t1 = millis();
    if (t1 - t0 > 30) {
      video.write(frame);
      int n = arduino.getData((char *)v, 80);
      if (n >= 4) {
        if (n % 4 == 0) {
          ArduinoValues st(frameNumber, v[0], v[1]);
          printf("%d - %d %d \n", frameNumber, v[0], v[1]);
          valuesFile.write((char*)&st, sizeof(ArduinoValues));
          valuesFile.flush();
        }
      }
      t0 = t1;
      frameNumber++;
    }
    imshow("camera", frame);
    char c = waitKey(1);
    switch (c) {
    case 27:
      done = true;
      break;
    }
  }
  valuesFile.close();
}

void writeValues(SerialComm& Arduino, short l, short r){
  int16_t v[2] = {l, r};
  Arduino.writeData((char*)v, 2 * sizeof(int16_t));
}


void read(bool writeVideo = false){
  int DELAY_BLUR = 100;
  int MAX_KERNEL_LENGTH = 31;
  //VideoCapture cap("output5/out8.avi");
  VideoCapture cap(1);
  SerialComm arduino;
  if(!arduino.openPort("/dev/ttyACM0") && !arduino.openPort("/dev/ttyACM1")) {
    printf("Arduino not found in ttyACM0 or ttyACM1, check the connextion\n");
    return; 
  }
  usleep(1000000);
  int frame_width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
  int frame_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
  
  
  VideoWriter video;
  if (writeVideo) {
    video.open("output6/out13.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30,
                     Size(frame_width, frame_height), true);
  }
  while(true) {
      vector<Vec4i> lines;
      Mat frame1, frame, norm, blur, thresh, erodee, dil, edge, frame2;
      cap >> frame1;                                                  //    640 X 480
      cvtColor(frame1, frame, CV_BGR2GRAY);
      equalizeHist(frame, norm);
      GaussianBlur( norm, blur, Size( 5, 5 ), 0, 0 );
      //blur = blur(Rect(0,(frame.rows-480), frame.cols, 480)); //_Tp _x, _Tp _y, _Tp _width, _Tp _height
            
      threshold(blur, thresh, 125, 255, THRESH_BINARY);  //95 - 115
      cv::erode(thresh,erodee,Mat());     
      cv::erode(erodee,erodee,Mat());     
      //cv::dilate(erodee,dil,Mat());
      //Canny( erodee, erodee, 200, 300, 3);
      
      /*HoughLinesP( erodee, lines, 1, CV_PI/180, 80, 10, 50);    
      for(int i = 0; i < lines.size(); i++ ) {
        line(frame1, Point(lines[i][0], lines[i][1]),
        Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
      }
      imshow("Video4", frame1 ); */
      
      //edge.convertTo(draw, CV_8U);

      std::vector<Rect> grid;
      //grid.push_back(Rect(Point(100, frame.rows - 200), Point(540, frame.rows - 150)));   //Rectangle bottom
      grid.push_back(Rect(Point(0, frame.rows - 200), Point(frame.cols, frame.rows - 150)));   //Rectangle bottom
      grid.push_back(Rect(Point(160, frame.rows - 300), Point(480, frame.rows - 250)));   //Rectangle top
      grid.push_back(Rect(Point(0, frame.rows - 300), Point(160, frame.rows - 250)));   //Rectangle left top
      grid.push_back(Rect(Point(480, frame.rows - 300), Point(640, frame.rows - 250)));   //Rectangle right top

      imshow("Video4", erodee);     
      Moments m = moments(erodee(grid[0]), false);   //layer bottom
      float x = m.m10 / m.m00;
      float y = m.m01 / m.m00;

      Moments m2 = moments(erodee(grid[1]), false);    //layer top
      float x2 = m2.m10 / m2.m00;
      float y2 = m2.m01 / m2.m00;

      Moments mLeft = moments(erodee(grid[2]), false);   //left
      float xLeft = mLeft.m10 / mLeft.m00;
      float yLeft = mLeft.m01 / mLeft.m00;

      Moments mRight = moments(erodee(grid[3]), false);   //right
      float xRight = mLeft.m10 / mLeft.m00;
      float yRight = mLeft.m01 / mLeft.m00;
      
      float w2 = frame.cols / 2.0f;
      float k = (w2 - x) / w2 * 2;
      int speed = 35;
      int vr = speed;
      int vl = speed;
      if (k >= 0) 
        vl = (1 - fabs(k)) * speed;
      else
        vr = (1 - fabs(k)) * speed;
      printf("w2 = %f x = %f k = %f vl = %d vr = %d\n", w2, x, k, vl, vr);

      writeValues(arduino, vl, vr);  
 

      cv::cvtColor(erodee, frame2, CV_GRAY2BGR);
      frame2 = frame1;
      rectangle(frame2, grid[0], Scalar( 255, 0, 0), 2, 8); //bottom
      rectangle(frame2, grid[1], Scalar( 255, 0, 0), 2, 8); //top
      
      rectangle(frame2, grid[2], Scalar( 0, 0, 255), 2, 8); //left
      rectangle(frame2, grid[3], Scalar( 0, 0, 255), 2, 8); //right
            
      circle(frame2, Point(xLeft, frame.rows - 250 - yLeft), 10, Scalar( 0,0, 255), 3, 8); //left
      circle(frame2, Point(xRight + 480, frame.rows - 250 - yRight), 10, Scalar( 0,0, 255), 3, 8); //right
      
      circle(frame2, Point(x + 100, frame.rows - 150 - y), 10, Scalar( 255, 0, 0), 3, 8); //buttom
      circle(frame2, Point(x2 + 160, frame.rows - 250 - y2), 10, Scalar( 0, 255, 0), 3, 8); //top
      

      if (writeVideo) {   
        video.write(frame2);
      }

      imshow("Video3", frame2 );
      if(waitKey(30) >= 0) {
         break;
      }
  }
}

void printArray(int *p, int n) {
  for (int i = 0; i < n; ++i) 
    printf("%d ", p[i]);
  printf("\n");
}

void convert(){
  Mat frame;
  ArduinoValues st1;
  
  VideoCapture cap("output5/out5.avi");
  std::ifstream valuesInFile("output5/values55.txt", ios::binary);
  
  while(!valuesInFile.eof()) {
      valuesInFile.read((char*)&st1, sizeof(st1));
      cap >> frame;       
      
      imshow("Video0", frame);
      printf("frame = %d, ml = %d mr = %d\n", st1.frameNumber, st1.leftMotor, st1.rightMotor);
      
      if(waitKey(330) >= 0) break;
  } 
}


int main(int argc, char **argv) {
   //for (int i = 0; i < argc; ++i) {
   //  printf("%s\n", argv[i]);
  // }

  //record("/dev/ttyACM0", "output6/out1.avi", "output6/values1.txt");
  read();
  
  //writeValues(100, 100);
  //convert();
}