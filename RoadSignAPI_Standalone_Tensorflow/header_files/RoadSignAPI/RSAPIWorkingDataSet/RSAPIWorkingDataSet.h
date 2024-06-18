/*
 * RSAPIWorkingDataSet.h
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_ROADSIGNAPI_RSAPIWORKINGDATASET_RSAPIWORKINGDATASET_H_
#define HEADER_FILES_ROADSIGNAPI_RSAPIWORKINGDATASET_RSAPIWORKINGDATASET_H_

#include <opencv2/opencv.hpp>

#include "FilterManagementLibrary/PipeSystem/PipeWorkingDataSet.h"
#include "RoadSignAPI/DetectedSignDescriptor.h"
#include "RoadSignAPI/DetectedSignCombination.h"

namespace RoadSignAPI
{
	/**
	 * Contains all variables which are needed to (efficiently!) detect and
	 * classifiy roadsigns from an OpenCV image Matrix.
	 * The detected and classified signs will also be stored here as a result.
	 */
	struct RSAPIWorkingDataSet :
			public FilterManagementLibrary::PipeSystem::PipeWorkingDataSet
	{
			/**
			 * This is the image which was feed into the API using
			 * {see RoadSignAPI}'s feedImage(...) function
			 * Needs to be 8UC3 BGR Image because {link TensorflowOpenCVUtils}
			 * expects that.
			 */
			cv::Mat originalBGRImage;

			/**
			 * This will be initialized by {@link SSDLiteRoadSignDetector}.
			 * It creates a scaled cv::Mat from the originalBGRImage
			 * (in 8UC3 format, too) which matches the input size of
			 * the neuronal network model used for
			 * the {@link SSDLiteRoadSignDetector} using bicubic interpolation.
			 */
			cv::Mat detectorScaledBGRImage;

			/**
			 * Describes the signs that were detected by the
			 * {@link SSDLiteRoadSignDetector}.
			 * See {@link DetectedSignDescriptor} for a more detailed
			 * description.
			 */
			std::vector<DetectedSignDescriptor> detectedSigns;

			/**
			 * This vector will be filled by the
			 * {@link DetectionBasedImageSlicer}. It uses the detectedSigns
			 * vector to crop the corresponding signs from the originalBgrImage.
			 * {@link MobilenetV2RoadSignClassificator} will scale them to
			 * the size it needs using bicubic interpolation.
			 */
			std::vector<cv::Mat> cutOutImages;

			/**
			 * Height of the originalBGRImage.
			 */
			int originalImageHeight;

			/**
			 * Width of the originalBGRImage
			 */
			int originalImageWidth;

			/**
			 * Contains the ID of entries in detectedSigns
			 * which were successfully classified by the classifier Filter
			 * and are not in the class of unimportant signs.
			*/
			std::vector<int> classifierApprovedSigns;

			/**
			 * The ClassifiedSignsGrouper tries to group all successfully
			 * classified signs on an (estimated) pole (which means they
			 * are located vertically above each other) into an
			 * DetectedSignCombination. The result is stored in this vector.
			 */
			std::vector<DetectedSignCombination> detectedSignCombinations;

	};
}


#endif /* HEADER_FILES_ROADSIGNAPI_RSAPIWORKINGDATASET_RSAPIWORKINGDATASET_H_ */
