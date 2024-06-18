/*
 * TensorflowOpenCVUtils.cpp
 *
 *  Created on: 04.06.2018
 *      Author: Patrick Langer
 */

#include "FilterManagementLibrary/TensorflowOpenCVUtils.h"
#include <stdint.h>
typedef cv::Point3_<uint8_t> Pixel;

/**
 * @brief Applies the raw data of a openCV mat to an Tensorflow input tensor.
 *
 * Uses OpenCV's forEach<...>() function to *fastly* iterate over all pixels
 * of the OpenCV Mat and copy them to the input tensor.
 * As forEach<...>() uses multi threading, this method is *WAY* faster
 * (5 - 10 times in my tests) than naive per pixel copying.
 *
 * If you use this, make sure your CV Mat is in the right format, e.g. 8UC3.
 * Also make sure mat and inputTensor have the same dimension!!
 * We don't do any safety checks here, because we want to be *fast*
 * (although it's not called "fast" because it lacks safety, I want to notice).
 *
 * You may check {@link TFNNBasedPipeFilter} class description for how to use this
 * function easily in a {@link TFNNBasedPipeFilter} (it's a one liner basically!).
 *
 * @param cv::Mat *mat pointer to the OpenCV matrix from which the data
 * will be copied from
 *
 * @param tensorflow::Tensor *inputTensor Tensorflow Tensor where the
 * data will be copied to.
 */
void FilterManagementLibrary::TensorflowOpenCVUtils::
	fastApplyCVMatOnInputTensorUInt8(cv::Mat *mat,
			tensorflow::Tensor *inputTensor)
{

	auto inputTensorMapped = inputTensor->tensor<uint8_t, 4>();



	// This is about 5 - 10 times faster (4 : 35 ms in my tests) than
	// using mat->at(...)
	mat->forEach<Pixel>
	(
		[&](Pixel &pixel, const int *position) -> void
		{
			// Be aware of OpenCV BGR Order!
			inputTensorMapped(0, position[0], position[1], 0) = pixel.z;
			inputTensorMapped(0, position[0], position[1], 1) = pixel.y;
			inputTensorMapped(0, position[0], position[1], 2) = pixel.x;
		}
	);
}


/**
 * @brief Applies the raw data of a openCV mat to an Tensorflow input tensor.
 *
 * Does the same as fastApplyCVMatOnInputTensorUInt8, but additionally
 * converts the uint8_t values to their corresponding float values
 * using a mean and std value.
 *
 * If you use this, make sure your CV Mat is in the right format, e.g.
 * 8UC3. We could use a 32FC3 CV Float Mat for this, but we may wan't to
 * apply our own std and mean values (don't know maybe we don't want to
 * use 127.5f or vice versa), that's why we expect a normal uint based
 * BGR Matrix).
 * Also make sure mat and inputTensor have the same dimension!!
 * We don't do any safety checks here, because we want to be *fast*
 *
 * You may check {@link TFNNBasedPipeFilter} class description for how to use this
 * function easily in a {@link TFNNBasedPipeFilter} (it's a one liner basically!).
 *
 * @param cv::Mat *mat pointer to the OpenCV matrix from which the data
 * will be copied from
 *
 * @param tensorflow::Tensor *inputTensor Tensorflow Tensor where the
 * data will be copied to.
 */
void FilterManagementLibrary::TensorflowOpenCVUtils::
	fastApplyCVMatOnInputTensorFloat(cv::Mat *mat,
			tensorflow::Tensor *inputTensor,
			float mean, float std)
{

	auto inputTensorMapped = inputTensor->tensor<float, 4>();


	// This is about 5 - 10 times faster (4 : 35 ms in my tests) than
	// using mat->at(...)

	mat->forEach<Pixel>
	(
		[&](Pixel &pixel, const int *position) -> void
		{
			// Be aware of OpenCV BGR Order!
			inputTensorMapped(0, position[0], position[1], 0) =
					(static_cast<float>(pixel.z) - mean) / std;
			inputTensorMapped(0, position[0], position[1], 1) =
							(static_cast<float>(pixel.y) - mean) / std;
			inputTensorMapped(0, position[0], position[1], 2) =
							(static_cast<float>(pixel.x) - mean) / std;
		}
	);
}


