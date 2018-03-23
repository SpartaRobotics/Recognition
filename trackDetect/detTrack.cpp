/*
Capstone
Capstone Project Code
12/3/2017

Authors: Bruce Nelson
*/

// Inlucde the standard and opencv libraries
#include<opencv2/objdetect/objdetect.hpp> // object-detection
#include<opencv2/highgui/highgui.hpp>     // UI interface
#include<opencv2/imgproc/imgproc.hpp>     // Image processing
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include<iostream>                        // standard library
#include <sys/time.h>

using namespace cv;
using namespace std;

#define WIDTH 640
#define HEIGHT 480
#define DEV 0
#define PORTD 128 // Port diameter in mm
#define REFXMM 130
#define REFYMM 130
#define REFZMM 600 // MM of camera distance
#define REFXPP 200
#define REFYPP 200
#define PMM 0.65 // Pixels per MM


// Convert FPS from integer to ostream to be display on frame
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()

// Creates and store the cascade dataset for the license ports
std::string port_cascade_file = "cascade.xml";
CascadeClassifier port_cascade;

// stores current frame
Mat frame;
Rect2d bbox;
struct timeval begin, end;


void drawCaller(Rect inBox)
{
    int midPortX = 0;
    int midPortY = 0;
    float distance;

    rectangle(frame, inBox, Scalar( 0, 0, 255 ), 3, 1 );

    midPortX = inBox.x + (inBox.width / 2);
    //Pixels
    //putText(frame, "X Dist: " + SSTR(int(midPortX-WIDTH/2)), Point(0,30), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,140,255), 2);
    // MM
    putText(frame, "X Dist: " + SSTR(float(int(midPortX-WIDTH/2)*0.65)) + " mm", Point(0,30), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,140,255), 2);
    line(frame, Point(WIDTH/2,HEIGHT/2), Point(midPortX, HEIGHT/2), Scalar( 0, 255, 0), 3);

    midPortY = inBox.y + (inBox.height / 2);
    //Pixels
    //putText(frame, "Y Dist: " + SSTR(int(-midPortY+HEIGHT/2)), Point(0,60), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,140,255), 2);
    // MM
    putText(frame, "Y Dist: " + SSTR(float(int(-midPortY+HEIGHT/2)*0.65)) + " mm", Point(0,60), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,140,255), 2);
    line(frame, Point(WIDTH/2,HEIGHT/2), Point(WIDTH/2, midPortY), Scalar( 0, 255, 0), 3);

    // Hyptonuse
    line(frame, Point(WIDTH/2,HEIGHT/2), Point(midPortX, midPortY), Scalar( 255, 255, 0), 3);
    //cout << "x = " << inBox.width << ": y = " << inBox.height << endl;

    // Center
    Point center(midPortX, midPortY);
    circle( frame, center, 3, Scalar(0,0,255), -1, 8, 0 );

    // Gets Distance
    distance = float(REFYPP / float(inBox.height)) * REFZMM;
    putText(frame, "Z Dist: " + SSTR(distance) + " mm", Point(0,90), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,140,255), 2);
    
    bbox = Rect2d(inBox.x, inBox.y, inBox.width, inBox.height);
    
    //bbox = Rect2d(midPortX-25, midPortY-25, 50, 50);
    //Rect2d((bbox.x+(bbox.width/2)-25),(bbox.y+(bbox.height/2)-25),50,50);
}

// Detects and recognizes the object from the cascade file
void detection()
{
    std::vector<Rect> ports; // Creates a vector for license ports
    Mat gray, blurPlate;

    cvtColor(frame, gray, COLOR_BGR2GRAY); // converts current frame to grayscale
    //equalizeHist(gray, gray); // equalizes the histogram of the grayscale image

    // detects object of different sizes and returns as rectangles
    port_cascade.detectMultiScale(gray, ports, 1.1, 5);

    for( size_t i = 0; i < ports.size(); i++ ) 
    {
        // creates the rectangle box in the frame
        drawCaller(ports[i]);
    }

    //imshow("Port", frame); // Display the current frame
}


// Main - handles the video processing and capturing of frames
int main()
{
    
    // Loads the cascade file. If not read, then exit program
    if(!port_cascade.load(port_cascade_file))
    {
        std::cout << "Error! Could not load cascade file!" << std::endl;
        return -1;
    }


    // Handles Tracking
    Ptr<Tracker> tracker;
    tracker = TrackerKCF::create();

    // Starts Live Video
    VideoCapture cap;
    cap.open(DEV);
    
    std::string VideoFileName = ""; // used if reading an a video file
    // cap.open("VideoFileName"); // Uncomment to insert video file

    // Opens the video file or feed. If not opened, then exit program
    if(!cap.isOpened())
    {
        std::cout << "ERROR! No video found!" << std::endl;
        return -1;
    }

    // Creates a window for the video
    namedWindow("Port", WINDOW_AUTOSIZE);

    double fps = cap.get(CAP_PROP_FPS); // gets frames per second

    Size size( // gets size of video capture device
        cap.get(CAP_PROP_FRAME_WIDTH),
        cap.get(CAP_PROP_FRAME_HEIGHT)
    );

    VideoWriter writer;
    writer.open("port_detector002.avi", CV_FOURCC('M','J','P','G'), fps, size);

    Mat gray;
    char quitProg; // handles user input for exiting the program
    bool chk = 0;
    double elapsed;

    Point center(cvRound(WIDTH/2), cvRound(HEIGHT/2));

    //cap >> frame;
    //bbox = selectROI("Port", frame);
    //tracker->init(frame, bbox);

    while(1) // loops through the video until it ends or the user exits
    {
        //gettimeofday(&begin, NULL);
        cap >> frame; // stores the frame from the video 
        //gettimeofday(&end, NULL); 
        if(frame.empty()) break; // if no frame received, exit program

        //gettimeofday(&begin, NULL);
        chk = tracker->update(frame,bbox);
        //gettimeofday(&end, NULL); 
       // chk = tracker->update(Rect2d((bbox.x+(bbox.width/2)-25),
       //                     (bbox.y+(bbox.height/2)-25),50,50));
       // cout << chk << endl;

        circle( frame, center, 3, Scalar(0,255,0), -1, 8, 0 );

        if (chk){
            
            //rectangle(frame, Rect2d(bbox.x-50,bbox.y-50, 100,100), Scalar( 0, 255, 0 ), 2, 1 );
            //putText(frame, "Tracking SUCCESS detected", Point(200,120), 
            //FONT_HERSHEY_SIMPLEX, 1.00, Scalar(255,255,0),2);
            
            drawCaller(bbox);
        }
        else{
            //putText(frame, "Tracking failure detected", Point(200,80), 
            //FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,0,255),2);
            
            gettimeofday(&begin, NULL);
            detection();
            tracker = TrackerKCF::create();
            tracker->init(frame, bbox);
            gettimeofday(&end, NULL); 
        }

        //chk = tracker->update(frame,bbox);

        imshow("Port", frame);

        //writer.write(frame);

        // If user enters a 'q' or ESC key, then exit the video
        quitProg = waitKey(30);
        if(quitProg == 'q' || quitProg == 27)
            break;
        else if(quitProg == 'b')
            imwrite("detTrack001.jpg",frame);

        elapsed = (end.tv_sec - begin.tv_sec) + 
			((end.tv_usec - begin.tv_usec)/1000.0);
		cout << elapsed << endl;
    }

    cap.release();       // closes the video file or capturing device
    destroyAllWindows(); // destroy current windows

    return 0;
}