
#include <opencv2/opencv.hpp>
#include <iostream>
#include <sys/time.h>   // NOLINT(build/include_order)
#include "RoadSignAPI/RoadSignAPI.h"
#include "RoadSignAPI/DetectedSignDescriptor.h"
#include "RoadSignAPI/DetectedSignCombination.h"
#include "FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstanceClassifier.h"
#include "FilterManagementLibrary/TensorflowOpenCVUtils.h"
#include <sstream>
using namespace cv;

// Define a pixel
typedef Point3_<uint8_t> Pixel;

double get_us(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }

bool readLabelsFile(std::string path, std::vector<std::string>* output)
{
	std::ifstream file(path);
	if (!file)
	{
	    return false;
	}




	output->clear();
	std::string line;
	while (std::getline(file, line))
	{
		output->push_back(line);
	}


	const int padding = 16;
	while (output->size() % padding)
	{
		output->emplace_back();
	}
	return true;
}
std::string type2str(int type) {
  std::string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}
int main()
{


	std::vector<std::string> detectorLabels;


	if(!readLabelsFile("labels.pbtxt", &detectorLabels))
	{
		std::cout << "Loading labels failed" << std::endl;
		return 0;
	}

	std::vector<std::string> classifierLabels;
	if(!readLabelsFile("labels_classifier.pbtxt", &classifierLabels))
		{
			std::cout << "Loading labels classifier failed" << std::endl;
			return 0;
		}

	srand (time(NULL));
	RoadSignAPI::RoadSignAPI::staticInit(2);


	// Video to process
	VideoCapture cap("20180608_161025.mp4");

	  if(!cap.isOpened()){
	    std::cout << "Error opening video stream or file" << std::endl;
	    return -1;
	  }

	  Mat frame;
	  int outWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
	  int outHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
	  double FPS = 10;

	  const int scaledWidth = 1280;
	  const int scaledHeigth = 720;
	  cv::Mat scaledImage(cv::Size(scaledWidth, scaledHeigth), CV_8UC3);

	  // Output file (writer)
	  cv::VideoWriter out("output.mp4", cv::VideoWriter::fourcc('h', '2', '6', '4'), FPS, cv::Size(scaledWidth, scaledHeigth));




	  std::stringstream ss;


	  int skipFrames = 0 ;

	  while(skipFrames > 0)
	  {
		  cap>>frame;
		  skipFrames--;
	  }


	  while(1){

		  ss.str("");
	    // Capture frame-by-frame
		for(int i = 0; i < 10; i++)
		{
			cap>>frame;
			if (frame.empty())
				break;
		}
	    cap >> frame;

	    std::cout << type2str(frame.type()).c_str() << std::endl;

	    // If the frame is empty, break immediately
	    if (frame.empty())
	      break;


	    RoadSignAPI::RoadSignAPI::staticFeedImage(frame);


	    const std::vector<RoadSignAPI::DetectedSignCombination>* const detectedSignCombinations =
	    		RoadSignAPI::RoadSignAPI::staticGetDetectedSignCombinations();





	    // Get all detected sign combinations
	    for(int i = 0; i < detectedSignCombinations->size(); i++)
	    {

	    	const RoadSignAPI::DetectedSignCombination signCombination = detectedSignCombinations->at(i);


	    	int minX = 100000;
	    	int minY = 100000;
	    	int maxX = 0;
	    	int maxY = 0;

	    	// Get all signs in the current combination
	    	for(RoadSignAPI::DetectedSignDescriptor signDescriptor : *signCombination.getSignsInCombination())
	    	{
	    	    // Get a box which contains ALL of the signs of the combination
	    		if(signDescriptor.upperLeft.x < minX)
	    			minX = signDescriptor.upperLeft.x;

	    		if(signDescriptor.upperLeft.y < minY)
	    			minY = signDescriptor.upperLeft.y;

	    		if(signDescriptor.lowerRight.x > maxX)
	    			maxX = signDescriptor.lowerRight.x;

	    		if(signDescriptor.lowerRight.y > maxY)
	    			maxY = signDescriptor.lowerRight.y;


	    		int poleXLeft = signCombination.getGestimatedPolePositionX() - (frame.cols * 0.025f);
	    		int poleXRight = signCombination.getGestimatedPolePositionX() + (frame.cols * 0.025f);

	    		if(poleXLeft < 0)
	    			poleXLeft = 0;

	    		if(poleXRight > frame.cols)
	    			poleXRight = frame.cols;

	    		cv::rectangle(frame, cv::Point(poleXLeft, 0), cv::Point(poleXRight, 700), cv::Scalar(0, 255, 255), 3);
	    		cv::rectangle(frame, signDescriptor.upperLeft, signDescriptor.lowerRight, cv::Scalar(0, 255, 0), 3);
	    		ss.str("");
	    		ss << classifierLabels[signDescriptor.classifierApprovedClassID] << ": " << signDescriptor.classifierConfidence;
	    		cv::putText(frame,
	    		ss.str().c_str(),
	    		cv::Point(signDescriptor.upperLeft.x,signDescriptor.upperLeft.y - 20), // Coordinates
	    		cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
	    		1.5, // Scale. 2.0 = 2x bigger
	    		cv::Scalar(255,255,255), // BGR Color
	    		3);
	    	}
	    	cv::rectangle(frame, cv::Point(minX, minY), cv::Point(maxX, maxY), cv::Scalar(0, 0, 255), 3);
	    }



	   	 cv::resize(frame,
	   	 	    				frame,
	   	 	    				cv::Size(scaledWidth, scaledHeigth),
	   	 	    				0.0, 0.0, cv::INTER_CUBIC );
	   	// Display the resulting frame
	   		   	    imshow( "Frame", frame );
	    out << frame;



	    // Press  ESC on keyboard to exit
	    char c=(char)waitKey(100);
	    if(c==27)
	      break;
	    else
	    {
	    	while(c == 32)
	    	{
	    		c = (char) waitKey(50);
	    	}
	    }
	    printf("m6\n");
	  }

	  // When everything done, release the video capture object
	  cap.release();

	  // Closes all the frames
	  destroyAllWindows();

}
