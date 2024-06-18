#include <string>

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "tensorflow/contrib/lite/interpreter.h"

#include "tensorflow/contrib/lite/kernels/register.h"
#include "tensorflow/contrib/lite/model.h"
#include "tensorflow/contrib/lite/optional_debug_tools.h"
#include "tensorflow/contrib/lite/string_util.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <fcntl.h>      // NOLINT(build/include_order)
#include <getopt.h>     // NOLINT(build/include_order)
#include <sys/time.h>   // NOLINT(build/include_order)
#include <sys/types.h>  // NOLINT(build/include_order)
#include <sys/uio.h>    // NOLINT(build/include_order)
#include <unistd.h>     // NOLINT(build/include_order)

#include "FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstanceClassifier.h"
#include "FilterManagementLibrary/TensorflowIntegration/NeuronalNetworkImageProcessSettings.h"

#include "header_files/FilterManagementLibrary/TensorflowIntegration/label_helpers/get_top_n.h"

using namespace std;

using namespace cv;

#include <vector>

#define LOG(x) std::cerr

// Define a pixel
typedef Point3_<uint8_t> Pixel;

double get_us(struct timeval t) { return (t.tv_sec * 1000000 + t.tv_usec); }



bool openCVMatTouint8_tArray(cv::Mat* mat, uint8_t* output, int* width, int* heigth)
{
	*width = mat->size().width;
	*heigth = mat->size().height;
	int channels = 3; // For now only RGB


    int idx = 0;

    for (int r = 0; r < mat->rows; r++)
    {
      // Loop over all columns
      for ( int c = 0; c < mat->cols; c++)
      {
        // Obtain pixel at (r, c)
        Pixel pixel = mat->at<Pixel>(r, c);
        // Apply complicatedTreshold


        output[idx + 2] = (uint8_t) pixel.x;
        output[idx + 1] = (uint8_t) pixel.y;
        output[idx ] = (uint8_t) pixel.z;
    //    printf("%d %d %d\n", tmp[idx + 2], tmp[idx + 1], tmp[idx]);
        idx += 3;
      }

    }

}

using namespace FilterManagementLibrary;


int main() {
	struct timeval start_time, stop_time;
	 gettimeofday(&start_time, NULL);

	 TFIntegration::NeuronalNetworkImageProcessSettings imgProcessSettings;

	 imgProcessSettings.input_mean = 127.5f;
	 imgProcessSettings.input_std = 127.5f;

	 TFIntegration::TensorflowNNInstanceClassifier classifier
	 ("mobilenet_v2_1.4_224.tflite", imgProcessSettings, 4, false);

	 if(!classifier.setupModel())
	 {
		 LOG(FATAL) << "Model could not be loaded" << std::endl;
		 while(true);
	 }
	 printf("1\n");
	 if(!classifier.readLabelsFile("labels.txt"))
	 {
		 LOG(FATAL) << "Labels file could not be loaded" << std::endl;
		 while(true);
	 }

	 printf("2\n");
    Mat image;
       image = imread("tank.jpg", 1);
       int width = image.size().width;
       int heigth = image.size().height;

              	int channels = 3; // For now only RGB
    printf("3\n");
    uint8_t* in = new uint8_t[width * heigth * channels];
    printf("4\n");


    openCVMatTouint8_tArray(&image, in, &width, &heigth);
    printf("5\n");
    if(!classifier.applyImageInputVector(in, width, heigth, channels))
    {
    	LOG(FATAL) << "Could not apply image vector" << std::endl;
    	while(true);
    }

    if(!classifier.runInference())
    {
    	LOG(FATAL) << "inference failed" << std::endl;
    	while(true);
    }

    std::vector<std::string> output;
    if(!classifier.getTopNPredictions(5, 0.0001f, output))
    {
    	LOG(FATAL) << "get top n predictions failed" << std::endl;
    }

    for(std::string prediction : output)
    {
    	std::cout << prediction.c_str() << std::endl;
    }

    namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
       imshow( "Display window", image );                   // Show our image inside it.

       waitKey(0);
   return 0;
}


