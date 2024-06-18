
#include <opencv2/opencv.hpp>
#include <iostream>
#include <sys/time.h>   // NOLINT(build/include_order)
#include "FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstance.h"
#include "FilterManagementLibrary/TensorflowIntegration/TensorflowNNInstanceClassifier.h"
#include "FilterManagementLibrary/Logger.h"
#include "header_files/FilterManagementLibrary/TensorflowIntegration/TensorflowNNModelDescription.h"
using namespace cv;

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

bool openCVMatTouint8_t3DArray(cv::Mat* mat, uint8_t*** output, int* width, int* heigth)
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

        output[r][c][0] = (uint8_t) pixel.z;
        output[r][c][1] = (uint8_t) pixel.y;
        output[r][c][2] = (uint8_t) pixel.x;

      }

    }

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

static void GetTopN(
    const Eigen::TensorMap<Eigen::Tensor<float, 1, Eigen::RowMajor>,
                           Eigen::Aligned>& prediction,
    const int num_results, const float threshold,
    std::vector<std::pair<float, int> >* top_results) {
  // Will contain top N results in ascending order.
  std::priority_queue<std::pair<float, int>,
                      std::vector<std::pair<float, int> >,
                      std::greater<std::pair<float, int> > >
      top_result_pq;

  const int count = prediction.size();
  for (int i = 0; i < count; ++i) {
    const float value = prediction(i);

    // Only add it if it beats the threshold and has a chance at being in
    // the top N.
    if (value < threshold) {
      continue;
    }

    top_result_pq.push(std::pair<float, int>(value, i));

    // If at capacity, kick the smallest value out.
    if (top_result_pq.size() > num_results) {
      top_result_pq.pop();
    }
  }

  // Copy to output vector and reverse into descending order.
  while (!top_result_pq.empty()) {
    top_results->push_back(top_result_pq.top());
    top_result_pq.pop();
  }
  std::reverse(top_results->begin(), top_results->end());
}

void testNet()
{
	std::string input_name = "input:0";
	std::string output_name = "output:0";



	  int g_tensorflow_input_size;  // The image size for the model input.
	  int g_image_mean;             // The image mean.
	  float g_image_std;            // The scale value for the input image.
	  std::unique_ptr<std::string> g_input_name;
	  std::unique_ptr<std::string> g_output_name;


 g_tensorflow_input_size = 224;
	  g_image_mean = 127;
	  g_image_std = 1;
	  g_input_name.reset(&input_name);
	  g_output_name.reset(&output_name);
	  std::unique_ptr<tensorflow::Session> session;

	  std::string model = "tensorflow_inception_graph.pb";

	  std::cout << "Loading TensorFlow." << std::endl;

	  std::cout << "Making new SessionOptions." << std::endl;
	  tensorflow::SessionOptions options;
	  tensorflow::ConfigProto& config = options.config;
	  std::cout << "Got config, " << config.device_count_size() << " devices" << std::endl;

	  session.reset(tensorflow::NewSession(options));
	  std::cout << "Session created." << std::endl;

	  tensorflow::GraphDef tensorflow_graph;
	  std::cout << "Graph created." << std::endl;

	  tensorflow::Status status = tensorflow::ReadBinaryProto(tensorflow::Env::Default(),
	  				model.c_str(), &tensorflow_graph);


	  std::cout << "Creating session.";
	  tensorflow::Status s = session->Create(tensorflow_graph);
	  if (!s.ok()) {
		  std::cout << "Could not create TensorFlow Graph: " << s << std::endl;
	  }

	  // Clear the proto to save memory space.
	  tensorflow_graph.Clear();
	  std::cout << "TensorFlow graph loaded from: " << model.c_str() << std::endl;


	    // Create input tensor
	    tensorflow::Tensor input_tensor(
	        tensorflow::DT_FLOAT,
	        tensorflow::TensorShape(
	            {1, g_tensorflow_input_size, g_tensorflow_input_size, 3}));





	    cv::Mat image = cv::imread("grace_hopper.bmp");

	    		// get pointer to memory for that Tensor

	    		Mat scaledImage(g_tensorflow_input_size, g_tensorflow_input_size, CV_32FC3);
	    		cv::resize(image, scaledImage, cv::Size(g_tensorflow_input_size, g_tensorflow_input_size), 0.0, 0.0, INTER_CUBIC );
	    	/*	int i = 0;



	    		cv::cvtColor(scaledImage, scaledImage, cv::COLOR_BGR2RGB);


	    		// get pointer to memory for that Tensor
	    		float *p = tfInstance.getInputTensor()->flat<float>().data();
	    		// create a "fake" cv::Mat from it
	    		cv::Mat cameraImg(settings.inputHeigth, settings.inputWidth, CV_32FC3, p);
	    		//cv::cvtColor(cameraImg, cameraImg, cv::COLOR_BGR2RGB);

	    		scaledImage.convertTo(cameraImg, CV_32FC3);*/

	    		uint8_t*** vector = new uint8_t**[scaledImage.rows];

	    		for(int i = 0; i < scaledImage.rows; i++)
	    		{
	    			vector[i] = new uint8_t*[scaledImage.cols];
	    			for(int j = 0; j < scaledImage.cols; j++)
	    			{
	    				vector[i][j] = new uint8_t[3];
	    			}
	    		}

	    		int width = 0;
	    		int heigth = 0;
	    		int channels = 0;

	    		openCVMatTouint8_t3DArray(&scaledImage, vector, &heigth, &width);

	    		 auto input_tensor_mapped = input_tensor.tensor<float, 4>();
	    				for (int i = 0; i < heigth; ++i)
	    				{
	    				    for (int j = 0; j < width; ++j)
	    				    {
	    				    	for(int k = 0; k < 2; k++)
	    				    	{
	    				    		input_tensor_mapped(0, i, j, k) =
	    				    				     (static_cast<float>(vector[i][j][k]) -
	    				    				        1) /
	    											127;

	    				    	}
	    				    }
	    				}

	    std::vector<std::pair<std::string, tensorflow::Tensor> > input_tensors(
	        {{*g_input_name, input_tensor}});

	    std::cout << "Start computing." << std::endl;
	    std::vector<tensorflow::Tensor> output_tensors;
	    std::vector<std::string> output_names({*g_output_name});


	    int64 start_time, end_time;


	      tensorflow::RunOptions run_options;
	      run_options.set_trace_level(tensorflow::RunOptions::FULL_TRACE);
	      tensorflow::RunMetadata run_metadata;

	      s = session->Run(input_tensors, output_names, {}, &output_tensors);



	    if (!s.ok()) {
	    	std::cout << "Error during inference: " << s;
	    }
	    else
	    {
	    	 std::cout << "Reading from layer " << output_names[0] << std::endl;


	    		    tensorflow::Tensor* output = &output_tensors[0];
	    		      const int kNumResults = 5;
	    		      const float kThreshold = 0.1f;
	    		      std::vector<std::pair<float, int> > top_results;
	    		      printf("1\n");
	    		      GetTopN(output->flat<float>(), kNumResults, kThreshold, &top_results);
	    		      printf("2\n");
	    		      std::stringstream ss;
	    		      printf("3\n");
	    		      ss.precision(3);
	    		      for (const auto& result : top_results) {
	    		        const float confidence = result.first;
	    		        const int index = result.second;

	    		        ss << index << " " << confidence << " ";



	    		        ss << "\n";
	    		    }
	    		      std::cout << "4\n";
	    		     std::cout << ss.str().c_str() << std::endl;
	    }



}

int main()
{

	int x = 0;
	float y = 1;
	std::string z = "";

	(x, y, z) = (1, 2.0f, "Hallo");
	std::cout << x << " " << y << " " << z.c_str() << "\n";
while(true);
	/*FilterManagementLibrary::TFIntegration::NeuronalNetworkImageProcessSettings settings;

	settings.inputHeigth = 224;
	settings.inputWidth = 224;
	settings.input_floating = true;
	settings.input_mean = 127.0f;
	settings.input_std = 1.0f;

	settings.inputLayerNameStr = "input:0";
	settings.outputLayerNameStr = "output:0";
	FilterManagementLibrary::TFIntegration::TensorflowNNInstanceClassifier* tfInstance = new
			FilterManagementLibrary::TFIntegration::TensorflowNNInstanceClassifier("tensorflow_inception_graph.pb", settings, 1);*/

    FilterManagementLibrary::TFIntegration::TensorflowNNModelDescription settings;

    settings.inputHeigth = 300;
    settings.inputWidth = 300;
    settings.input_floating = false;
    settings.input_mean = 127.5f;
    settings.input_std = 1.0f;

    settings.inputLayerNameStr = "image_tensor";
    settings.outputLayerNames.push_back("detection_scores");
    settings.modelFile = "frozen_inference_ssdlite";
    FilterManagementLibrary::TFIntegration::TensorflowNNInstance tfInstance(settings, 1);


	struct timeval start_time, stop_time, single_start_time, single_stop_time;

	if(!tfInstance.setupModel())
		{
			switch(tfInstance.getLastError())
			{
				case FilterManagementLibrary::TFIntegration::TensorflowNNInstance::ErrorType::ERROR_INVALID_MODEL_FILE:
				{
					std::cout << "Model file not found\n";
				}
				break;
				case FilterManagementLibrary::TFIntegration::TensorflowNNInstance::ErrorType::ERROR_COULD_NOT_LOAD_MODEL:
				{
					std::cout << "Model could not be loaded (invalid format or missing permissions)\n";
				}
				break;
				case FilterManagementLibrary::TFIntegration::TensorflowNNInstance::ErrorType::ERROR_COULD_NOT_ADD_GRAPH_TO_SESSION:
				{
					std::cout << "Could not add graph to session\n";
				}
				break;
				case FilterManagementLibrary::TFIntegration::TensorflowNNInstance::ErrorType::ERROR_FAILED_TO_CONSTRUCT_NEW_SESSION:
				{
					std::cout << "Failed to construct new session\n";
				}
				break;
			}
			while(true);
		}
	/*
	if(!tfInstance.readLabelsFile("imagenet_comp_graph_label_strings.txt"))
	{
		std::cout << "Failed to load labels file.\n";
		while(true);
	}

	std::cout << "Labels loaded\n";
*/
	/*uint8_t*** vector = new uint8_t**[settings.inputHeigth];

	for(int i = 0; i < settings.inputHeigth; i++)
	{
		vector[i] = new uint8_t*[settings.inputWidth];
		for(int j = 0; j < settings.inputWidth; j++)
		{
			vector[i][j] = new uint8_t[3];
		}
	}


	if(!tfInstance.applyImageInputVector(vector, settings.inputHeigth, settings.inputWidth, 3))
	{
		std::cout << "Input vector could not be applied\n";
		while(true);
	}*/



		cv::Mat image = cv::imread("grace_hopper.bmp");
		gettimeofday(&start_time, NULL);
		// get pointer to memory for that Tensor

		Mat scaledImage(settings.inputHeigth, settings.inputWidth, CV_32FC3);
		cv::resize(image, scaledImage, cv::Size(settings.inputHeigth, settings.inputWidth), 0.0, 0.0, INTER_CUBIC );
	/*	int i = 0;



		cv::cvtColor(scaledImage, scaledImage, cv::COLOR_BGR2RGB);


		// get pointer to memory for that Tensor
		float *p = tfInstance.getInputTensor()->flat<float>().data();
		// create a "fake" cv::Mat from it
		cv::Mat cameraImg(settings.inputHeigth, settings.inputWidth, CV_32FC3, p);
		//cv::cvtColor(cameraImg, cameraImg, cv::COLOR_BGR2RGB);

		scaledImage.convertTo(cameraImg, CV_32FC3);*/

		uint8_t*** vector = new uint8_t**[scaledImage.rows];

		for(int i = 0; i < scaledImage.rows; i++)
		{
			vector[i] = new uint8_t*[scaledImage.cols];
			for(int j = 0; j < scaledImage.cols; j++)
			{
				vector[i][j] = new uint8_t[3];
			}
		}

		int width = 0;
		int heigth = 0;
		int channels = 0;

		openCVMatTouint8_t3DArray(&scaledImage, vector, &heigth, &width);



		if(!tfInstance.applyImageInputVector(vector, settings.inputHeigth, settings.inputWidth, 3))
		{
			std::cout << "Input vector could not be applied\n";
			while(true);
		}


		 gettimeofday(&stop_time, NULL);
		 std::stringstream ss;
		 ss << "Time: " << (get_us(stop_time) - get_us(start_time)) / (1000.0) << std::endl;
		 if(!tfInstance.runInference())
		 {
			std::cout << "Inference failed\n";
			while(true);
		 }


		 gettimeofday(&start_time, NULL);
	    std::stringstream tmp;
	    int runs = 1;
	    for(int i = 0; i < runs; i++)
	    {
	        gettimeofday(&single_start_time, NULL);
	        if(!tfInstance.runInference())
	        {
	            FilterManagementLibrary::Logger::printfln("Inference failed\n");
	            while(true);
	        }
	        gettimeofday(&single_stop_time, NULL);
	        tmp.str("");
	        tmp << "Iter " << i << " " << (get_us(single_stop_time) - get_us(single_start_time)) / (1000.0) << "\n";
	        FilterManagementLibrary::Logger::printfln(tmp.str().c_str());
	    }
	    gettimeofday(&stop_time, NULL);

	    ss << "Time: " << (get_us(stop_time) - get_us(start_time)) / (1000.0 * runs) << std::endl;


	std::cout << "Time: " << (get_us(stop_time) - get_us(start_time)) / 1000.0 << std::endl;


	std::cout << "Inference success omg you don't believe it\n";










	//tfInstance.applyImageInputVector();



	std::cout << type2str(image.type()).c_str() << std::endl;


/*

	std::vector<std::string> outputList;
	if(!tfInstance.getTopNPredictionsStringVector(5, 0.1f, &outputList))
	{
		std::cout << "getTopN failed..\n";
		while(true);
	}
	else
	{
		std::cout << "size: " << outputList.size() << std::endl;
		for(int i = 0; i < outputList.size(); i++)
		{
			std::cout << outputList[i].c_str() << "\n";
		}
	}

	for(int i = 0; i < settings.inputHeigth; i++)
	{
		for(int j = 0; j < settings.inputWidth; j++)
		{
			delete[] vector[i][j];
		}
		delete[] vector[i];
	}
*/
	delete[] vector;



	while(true);

	cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
	cv::imshow( "Display window", scaledImage );
	cv::waitKey(0);
}
