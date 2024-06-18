/*
 * DetectionBasedImageSlicer.h
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_ROADSIGNAPI_FILTERS_DETECTIONBASEDIMAGESLICER_H_
#define HEADER_FILES_ROADSIGNAPI_FILTERS_DETECTIONBASEDIMAGESLICER_H_

#include "FilterManagementLibrary/PipeSystem/PipeFilter.h"
#include "RoadSignAPI/RSAPIWorkingDataSet/RSAPIWorkingDataSet.h"

namespace RoadSignAPI
{
	/**
	 * This filter will use the detections the SSDLiteRoadSignDetector provides
	 * to cut the originalBGRImage into smaller pieces, which the
	 * MobilenetV2RoadSignClassificator can use to classify the detections.
	 *
	 * The dataset which was used to train the network models consists
	 * of images where each road sign was labeled (using a box which
	 * enclosed the sign). These boxes were set in such a way that they
	 * perfectly enclose the signs. However to train the classificator,
	 * we expaneded each box in every direction (x, -x, y, -y) by
	 * 10% of the box width / height or at least 5 pixels (depending
	 * on which value is bigger), in order to achieve a greater
	 * independence from the background of the road sign and noise.
	 * Thus the network learned the road sign's class is independent
	 * from it's background.
	 * To achieve optimal results, we have to do the same expansion
	 * here instead of only crop the images from the given detection
	 * before feeding them into the classificator.
	 * We use the same values here, stored in expandPercentage (10%)
	 * and minExpandPixels (5).
	 */
	class DetectionBasedImageSlicer :
			public FilterManagementLibrary::PipeSystem::PipeFilter
	{
		private:

			/**
			 * Just a pointer casted from {@link PipeWorkingDataSet}*
			 * to {@link RSAPIWorkingDataSet}*, so we just don't have to do
			 * the casting every time we need it ;)
			 */
			RSAPIWorkingDataSet* castedWorkingDataSet;


			/**
			 * This is the value we expand each box around a detected
			 * sign in any direction (x, -x, y, -x).
			 * Please refer to the class description for an explanation
			 * why we (have to) do this.
			 */
			float expandPercentage = 0.1;

			/**
			 * This is the minimum amount of pixels we expand each box
			 * around a detected sign in any direction (x, -x, y, -x).
			 * It is used to expand the box, if the value calculated
			 * using expandPercentage is to small: We want to expand
			 * AT LEAST 5 pixels (or more, respectively).
			 * Please refer to the class description for an explanation
			 * why we (have to) do this.
			 */
			int minExpandPixels = 5;

			void expandBox(cv::Point* upperLeft,
					cv::Point* lowerRight);

			// Get's called when the pipe's setup function is called
			// after all the filters have been registered.
			bool initByPipeSetup();

			// The main function of this filter, will be called
			// by the pipe when data shall be processed.
			bool process();

	};
}


#endif /* HEADER_FILES_ROADSIGNAPI_FILTERS_DETECTIONBASEDIMAGESLICER_H_ */
