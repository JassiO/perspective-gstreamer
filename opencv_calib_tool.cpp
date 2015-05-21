#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <iostream>
#include <fstream>
#include <limits>
#include <numeric>
#include <unistd.h>
using namespace cv;
using namespace std;
 
// We need 4 corresponding 2D points(x,y) to calculate homography.
vector<Point2f> left_image;      // Stores 4 points(x,y) of the logo image. Here the four points are 4 corners of image.
vector<Point2f> right_image;    // stores 4 points that the user clicks(mouse left click) in the main image.
 
// Image containers for main and logo image
Mat imageMain;

int element_number = 0;
 
// Function to add main image and transformed logo image and show final output.
// Icon image replaces the pixels of main image in this implementation.
void showFinal(Mat src1,Mat src2)
{
 
    Mat gray,gray_inv,src1final,src2final;
    cvtColor(src2,gray,CV_BGR2GRAY);
    threshold(gray,gray,0,255,CV_THRESH_BINARY);
    //adaptiveThreshold(gray,gray,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,5,4);
    bitwise_not ( gray, gray_inv );
    src1.copyTo(src1final,gray_inv);
    src2.copyTo(src2final,gray);
    Mat finalImage = src1final+src2final;
    namedWindow( "output", WINDOW_AUTOSIZE );
    //resize(finalImage, finalImage, cvSize(854, 480));
    imshow("output",finalImage);
    cvWaitKey(0);
 
}
 
// Here we get four points from the user with left mouse clicks.
// On 5th click we output the overlayed image.
void on_mouse( int e, int x, int y, int d, void *ptr )
{
    if (e == EVENT_LBUTTONDOWN )
    {
        if(right_image.size() < 4 )
        {
 
            right_image.push_back(Point2f(float(x),float(y)));
            cout << x << " "<< y <<endl;
        }
        else
        {
            //destroyWindow("Display window");

            cout << " Calculating Homography " <<endl;
            // Deactivate callback
            cv::setMouseCallback("Display window", NULL, NULL);
            // once we get 4 corresponding points in both images calculate homography matrix
            Mat H = findHomography(  right_image,left_image,0 );
            
            int cols = H.cols, rows = H.rows;

            for(int i = 0; i < rows; i++)
            {
                const double* Mi = H.ptr<double>(i);
                for(int j = 0; j < cols; j++) {
                    std::cout << "m[" << element_number << "] = " << Mi[j] << ";" << std::endl;
                }
            }
            
            H = H.inv();

            // write homography in file

            std::ofstream H_out ("homography.txt");

            for(int i = 0; i < rows; i++)
            {
                const double* Mi = H.ptr<double>(i);
                for(int j = 0; j < cols; j++) {
                   // Mi[j] = Mi[j] / H[3][3];
                    H_out  << Mi[j] << std::endl;
                    element_number += 1;
                }
            }

            H_out.close();

            Mat logoWarped;
            
            H = H.inv();
            // Warp the logo image to change its perspective
            warpPerspective(imageMain,logoWarped,H,imageMain.size() );
            showFinal(imageMain,logoWarped);
 
        }
 
    }
}

void captureImage(){
    CvCapture* capture = 0;
    Mat frame;

    capture = cvCaptureFromCAM(0);
    if(!capture) {
        std::cout << "Error: No device detected!" << std::endl;
    }

    if(capture) {
        
        IplImage* iplImg = cvQueryFrame(capture);
        frame = iplImg;
        std::cout << "Hallo" << std::endl;
        imshow("test", frame);
    }
    
    
}
 
 
int main( int argc, char** argv )
{
//  We need tow argumemts. "Main image" and "logo image"
    if( argc != 2)
    {
        cout <<" Usage: ./opencv_calib_tool your_calibration_image.jpg" << endl;
        return -1;
    }
 
    captureImage();
// Load images from arguments passed.
    imageMain = imread(argv[1], CV_LOAD_IMAGE_COLOR);
    //imageLogo = imread(argv[2], CV_LOAD_IMAGE_COLOR);
// Push the 4 corners of the logo image as the 4 points for correspondence to calculate homography.
    left_image.push_back(Point2f(float(0),float(0)));
    left_image.push_back(Point2f(float(0),float(1080)));
    left_image.push_back(Point2f(float(1920),float(1080))); //854, 480
    left_image.push_back(Point2f(float(1920),float(0)));
 
 
 
    namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
    //resize(imageMain, imageMain, cvSize(854, 480));
    imshow( "Display window", imageMain );
 	
 
    setMouseCallback("Display window",on_mouse, NULL );
 
 
//  Press "Escape button" to exit
    while(1)
    {
        int key=cvWaitKey(10);
        if(key==27) break;
    }
 
 
    return 0;
}