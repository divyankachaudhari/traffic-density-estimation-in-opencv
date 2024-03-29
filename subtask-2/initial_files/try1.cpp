#include <opencv2/opencv.hpp>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <regex>

#define pb push_back
#define fast ios_base::sync_with_stdio(false), cin.tie(NULL), cout.tie(NULL);
#define endl "\n"
#define ff first
#define ss second
#define ll long long int

using namespace cv;
using namespace std;

struct userdata {
    Mat im;
    vector<Point2f> points;
};


void mouseHandler(int event, int x, int y, int flags, void* data_ptr) {
    if  (event == 1) {
        // Taking input of the mouse click
        userdata *data = ((userdata *) data_ptr);
        // Using small red dots/circles wherever the mouse clicks for user to know
        circle(data->im, Point(x,y),2,Scalar(0,0,600), 5, cv::LINE_AA);
        // To show the Image with Red dots
        imshow("Image", data->im);
        // Saving the co-ordinates of mouse click in data
        data->points.push_back(Point2f(x,y));

    }
}

int checkImage(bool check) {
  if (check == false) {
      cout << "Mission failed succesfully: could not save the image. Try again?" << endl;
      // wait for any key to be pressed
      cin.get();
      return -1;
  }
  return 0;
}
bool isImageFile(string str) {
  // Regex to check valid image file extension.
  const regex pattern("[^\\s]+(.*?)\\.(jpg|jpeg|png|gif|JPG|JPEG|PNG|GIF)$");
  if (str.empty()){ return false; }
  if(regex_match(str, pattern)){ return true;}
  else { return false;}
}

void cropImage(Mat &croppedImage, Mat &bird_view, vector<pair<int, int>> &mouse_clicks, vector<pair<int, int>> &crop_this, userdata &data, Mat &h) {
  for(int i=0; i<4; i++) {
      mouse_clicks[i].ff = data.points[i].x;
      mouse_clicks[i].ss = data.points[i].y;
      // cout<<mouse_clicks[i].ff<<" "<<mouse_clicks[i].ss<<endl;

      Mat pt1 = (Mat_<double>(3, 1) << mouse_clicks[i].ff, mouse_clicks[i].ss, 1);
      Mat pt2 = h*pt1;
      pt2 /= pt2.at<double>(2);

      // cout<<pt2.at<double>(0)<<" "<<pt2.at<double>(1)<<" "<<pt2.at<double>(2)<<"kb"<<endl;
      crop_this[i].ff = pt2.at<double>(0);
      crop_this[i].ss = pt2.at<double>(1);
  }
  int crop_w = crop_this[2].ff - crop_this[0].ff;
  int crop_h = crop_this[1].ss - crop_this[0].ss;
  Mat ROI(bird_view, Rect(crop_this[0].ff, crop_this[0].ss, crop_w, crop_h));

  ROI.copyTo(croppedImage);
}

void destPoints(vector<Point2f> &pts_dst){
  pts_dst.pb(Point2f(472,52));
  pts_dst.pb(Point2f(472, 830));
  pts_dst.pb(Point2f(800, 830));
  pts_dst.pb(Point2f(800, 52));

}

inline bool isFileExist (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

int checkFile(bool check1, bool check2) {
  if (check1 == false || check2 == false) {
      cout << "Hold up! \nMission failed succesfully: Could not find that image, or maybe that file isn't an image. \nDid you check if that file exists in this directory?" << endl;
      // wait for any key to be pressed
      cin.get();
      return -1;
  }
  return 0;
}


int main(int argc, char** argv) {
//--------------------------------taking user input points to warp perspective

    // cout<<"Enter file name/destination: ";
    // //string name = argv[1];
    // string name;
    // waitKey(0);
    // cin>> name;
    //
    // // Check if file exists; return if it doesn't
    // if(checkFile(isFileExist(name), isImageFile(name)) == -1) {return 0;}
    string name = "bg.jpg";

    Mat im_src = imread(name);
    Size size = im_src.size();
    Mat im_dst = Mat::zeros(size,CV_8UC3);

    // Create vector and add destination points to it
    vector<Point2f> pts_dst;
    destPoints(pts_dst);

    // Set data for mouse event
    Mat im_temp = im_src.clone();
    userdata data;
    data.im = im_temp;

    cout << "Choose the point in anti-clockwise fashion, starting from top-left." << endl;

    // Show image and wait for mouse clicks
    imshow("Image", im_temp);
    setMouseCallback("Image", mouseHandler, &data);
    waitKey(0);

    // Calculate the homography & warp source image to destination
    Mat h = findHomography(data.points, pts_dst);
    warpPerspective(im_src, im_dst, h, size);

//------------------------------------------------------------------------------------------
    VideoCapture vid("trafficsmall.mp4");

    double n = vid.get(CAP_PROP_FRAME_COUNT);
    // print number of frames
    // cout << n;
    if(!vid.isOpened()) {
        cout<<"Error uanble to open video"<<endl;
        return -1;
    }
    int c=1;
    while(1) {
        c += 1;
        c %= 5;
        Mat frame;
        vid >> frame;
        if(frame.empty()) break;

        if(c == 1) {
            // imshow("Frame", frame);
            Mat warpped_frame = Mat::zeros(size,CV_8UC3);  /// f grayscale karna hai abhi
            warpPerspective(frame, warpped_frame, h, size);
            imshow("View corrected", warpped_frame);
        }

        char c = (char)waitKey(25);
        if(c == 27) break;
    }
    vid.release();
    destroyAllWindows();
    return 0;
}
