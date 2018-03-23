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
#include<iostream>                        // standard library
#include<time.h>


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

// Detects and recognizes the object from the cascade file
void detection(Mat frame)
{
    std::vector<Rect> ports; // Creates a vector for license ports
    Mat gray, blurPlate;
    int midPortX = 0;
    int midPortY = 0;
    float distance;
    

    cvtColor(frame, gray, COLOR_BGR2GRAY); // converts current frame to grayscale
    //equalizeHist(gray, gray); // equalizes the histogram of the grayscale image

    // detects object of different sizes and returns as rectangles
    port_cascade.detectMultiScale(gray, ports, 1.1, 5);

    // Iterates through all detected objects
    for( size_t i = 0; i< ports.size(); i++ ) 
    {
        // creates the rectangle box in the frame
        rectangle(frame, ports[i], Scalar( 0, 0, 255 ), 3, 1 );

        midPortX = ports[i].x + (ports[i].width / 2);
        //Pixels
        //putText(frame, "X Dist: " + SSTR(int(midPortX-WIDTH/2)), Point(0,30), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,140,255), 2);
        // MM
        putText(frame, "X Dist: " + SSTR(float(int(midPortX-WIDTH/2)*0.65)) + " mm", Point(0,30), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,140,255), 2);
        line(frame, Point(WIDTH/2,HEIGHT/2), Point(midPortX, HEIGHT/2), Scalar( 0, 255, 0), 3);

        midPortY = ports[i].y + (ports[i].height / 2);
        //Pixels
        //putText(frame, "Y Dist: " + SSTR(int(-midPortY+HEIGHT/2)), Point(0,60), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,140,255), 2);
        // MM
        putText(frame, "Y Dist: " + SSTR(float(int(-midPortY+HEIGHT/2)*0.65)) + " mm", Point(0,60), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,140,255), 2);
        line(frame, Point(WIDTH/2,HEIGHT/2), Point(WIDTH/2, midPortY), Scalar( 0, 255, 0), 3);

        // Hyptonuse
        line(frame, Point(WIDTH/2,HEIGHT/2), Point(midPortX, midPortY), Scalar( 255, 255, 0), 3);
        //cout << "x = " << ports[i].width << ": y = " << ports[i].height << endl;

        // Center
        Point center(midPortX, midPortY);
        circle( frame, center, 3, Scalar(0,0,255), -1, 8, 0 );

        // Gets Distance
        distance = float(REFYPP / float(ports[i].height)) * REFZMM;
        putText(frame, "Z Dist: " + SSTR(distance) + " mm", Point(0,90), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,140,255), 2);
        
    }

    imshow("Port", frame); // Display the current frame
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

    VideoCapture cap;
    cap.open(DEV);     // opens a live video on the selected video device
    
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

    double fps = cap.get(CV_CAP_PROP_FPS); // gets frames per second

    Size size( // gets size of video capture device
        cap.get(CV_CAP_PROP_FRAME_WIDTH),
        cap.get(CV_CAP_PROP_FRAME_HEIGHT)
    );

    //VideoWriter writer;
    //writer.open("port_detector.avi", CV_FOURCC('M','J','P','G'), fps, size);

    Mat gray;
    char quitProg; // handles user input for exiting the program
    Point center(cvRound(WIDTH/2), cvRound(HEIGHT/2));

	int num_frames = 120;
	time_t start, end;
	time(&start);
    for(int i = 0; i < num_frames; i++) // loops through the video until it ends or the user exits
    {
        cap >> frame; // stores the frame from the video 
        if(frame.empty()) break; // if no frame received, exit program


        circle( frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
        detection(frame); // detects objects in the current frame

        //writer.write(frame);
        cout << "FRAMES: " << cap.get(CV_CAP_PROP_FPS) << endl;

        // If user enters a 'q' or ESC key, then exit the video
        quitProg = waitKey(1);
        if(quitProg == 'q' || quitProg == 27)
            break;
    }
    time(&end);
    
    double seconds = difftime(end, start);
    cout << "Time taken : " << seconds << " seconds" << endl;
    fps = num_frames / seconds;
    cout << "Estimated frames per second : " << fps << endl;

    cap.release();       // closes the video file or capturing device
    destroyAllWindows(); // destroy current windows

    return 0;
}
