/*
 * DetectedSignDescriptor.h
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_ROADSIGNAPI_DETECTEDSIGNDESCRIPTOR_H_
#define HEADER_FILES_ROADSIGNAPI_DETECTEDSIGNDESCRIPTOR_H_

#include <opencv2/opencv.hpp>

namespace RoadSignAPI
{
	/**
	 * Struct describing position and ID of a roadsign detected
	 * and classified in an image. It also contains the confidence
	 * values of the detector and the classificator.
	 * Mainly used in {@link RSAPIWorkingDataSet}.
	 */
	struct DetectedSignDescriptor
	{
		/**
		 * Upper left position of the detection of the roadsign
		 * (mapped to the coordinates of the original image that
		 * was provied, not to the scaled image the detector uses).
		 */
		cv::Point upperLeft;

		/**
		 * Lower right position of the detection of the roadsign
		 * (mapped to the coordinates of the original image that
		 * was provied, not to the scaled image the detector uses).
		 */
		cv::Point lowerRight;

		/**
		 * Class ID the detection filter predicts.
		 */
		int detectionPredictedClassID;

		/**
		 * Confidence of the detector for this detection.
		 */
		float detectorConfidence;

		/**
		 * Class ID the classifier determined.
		 */
		int classifierApprovedClassID;

		/**
		 * Confidence of the classifier for the classification
		 * of this detection. Will be filled when the DetectedSignDescriptor
		 * is passed to the {@link MobilenetV2RoadSignClassificator}.
		 */
		float classifierConfidence;

	};
}


#endif /* HEADER_FILES_ROADSIGNAPI_DETECTEDSIGNDESCRIPTOR_H_ */
