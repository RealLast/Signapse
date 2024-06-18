/*
 * TensorflowOpenCVUtils.h
 *
 *  Created on: 04.06.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_ROADSIGNAPI_TENSORFLOWOPENCVUTILS_H_
#define HEADER_FILES_ROADSIGNAPI_TENSORFLOWOPENCVUTILS_H_
#include <tensorflow/core/platform/env.h>
#include <tensorflow/core/public/session.h>

#include <opencv2/opencv.hpp>

namespace FilterManagementLibrary
{
	/**
	 * Some utilities to efficiently use OpenCV with the
	 * Filter management library. Remember YOU DON'T NEED
	 * THIS TO USE THE FILTER MANAGEMENT LIBRARY!!
	 */
	class TensorflowOpenCVUtils
	{
		public:
			// If you use this, make sure your CV Mat is in the right format, e.g.
			// 8UC3.
			// Also make sure mat and inputTensor have the same dimension!!
			// We don't do any safety checks here, because we want to be *fast*
			static void fastApplyCVMatOnInputTensorUInt8(cv::Mat *mat,
					tensorflow::Tensor *inputTensor);

			// If you use this, make sure your CV Mat is in the right format, e.g.
			// 8UC3. We could use a 32FC3 CV Float Mat for this, but we may wan't to
			// apply our own std and mean values (don't know maybe we don't want to
			// use 127.5f or vice versa), that's why we expect a normal uint based
			// BGR Matrix).
			// Also make sure mat and inputTensor have the same dimension!!
			// We don't do any safety checks here, because we want to be *fast*
			static void fastApplyCVMatOnInputTensorFloat(cv::Mat *mat,
								tensorflow::Tensor *inputTensor,
								float mean, float std);
	};
}


#endif /* HEADER_FILES_ROADSIGNAPI_TENSORFLOWOPENCVUTILS_H_ */
