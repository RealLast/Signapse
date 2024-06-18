/*
 * DetectionBasedImageSlicer.cpp
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */


#include "RoadSignAPI/Filters/DetectionBasedImageSlicer.h"
#include "RoadSignAPI/RSAPIPipeRegisteredFilters/RSAPIPipeRegisteredFilters.h"
#include "FilterManagementLibrary/Logger.h"

/**
 * @brief Initializes the filter.
 *
 * As this filter does not need any specific environment variables,
 * nothing really exciting is happenning here..
 *
 * @return true, always
 */
bool RoadSignAPI::DetectionBasedImageSlicer::initByPipeSetup()
{
	this->castedWorkingDataSet =
			(RSAPIWorkingDataSet*) this->pipeWorkingDataSet;

	FilterManagementLibrary::Logger::printfln("DetectionBasedImageSlicer init");

	return true;
}

/** @brief The process function of this filter.
 *
 * This function will be called by the ProcessingPipeline.
 * Here the road signs detected by SSDRoadSignDetector will
 * be cut out into smaller images (which only contain the sign),
 * which will then be fed into the MobilenetV2RoadSignClassificator.
 * To be fast, we assume that the coordinates calculated by the
 * previous filter are correct!
 *
 * @return true, always
 */
bool RoadSignAPI::DetectionBasedImageSlicer::process()
{
	this->castedWorkingDataSet->cutOutImages.clear();


	// Store coordinates mapped to original image
	cv::Point2i originalCoordinates;

	// The "cropping function" uses box width and height
	// over absolute coordinates.
	cv::Point2i boxDimensions;
	for(DetectedSignDescriptor detectedSignDescriptor :
			this->castedWorkingDataSet->detectedSigns)
	{
		cv::Point adjustedUpperLeft = detectedSignDescriptor.upperLeft;
		cv::Point adjustedLowerRight = detectedSignDescriptor.lowerRight;

		this->expandBox(&adjustedUpperLeft,
				&adjustedLowerRight);


		boxDimensions.x = adjustedLowerRight.x - adjustedUpperLeft.x;
		boxDimensions.y = adjustedLowerRight.y - adjustedUpperLeft.y;

		originalCoordinates.x =
				adjustedUpperLeft.x;

		originalCoordinates.y =
				adjustedUpperLeft.y;



		cv::Mat croppedMat = cv::Mat(this->castedWorkingDataSet->originalBGRImage,
				cv::Rect(originalCoordinates.x, originalCoordinates.y,
						boxDimensions.x, boxDimensions.y));
		this->castedWorkingDataSet->cutOutImages.push_back(croppedMat);
	}
	this->invokeNext(((RSAPIPipeRegisteredFilters*)
					this->pipeRegisteredFilters)->SIGN_RECOGNITION_FILTER);
	return true;
}

/**
 * Expands the given coordinates of the given box (rectangle)
 * by expandPercentage (see member variables) in each direction,
 * if the amount of pixels to expand is bigger than minExpandPixels
 * (see member variables), otherwise minExpandPixels will be used
 * as amount of pixels to expand. If we exceed the border of
 * the originalBGRImage (see RSAPIWorkingDataSet) at any direction
 * (e.g. coordinates < 0 or > width or height), use set the
 * highest / lowest possible value (so concerning the example,
 * 0 or width / height).
 *
 * @param cv::Point *upperLeft Pointer to the upperLeft coordinates
 * of the box which shall be expanded. We use copies from the
 * corresponding values of the RSAPIWorkingDataSet, because
 * we do not want to expand the original, close-fitting boxes.
 *
 * @param cv::Point *lowerRight Pointer to the lowerRight coordinates
 * of the box which shall be expanded. We use copies from the
 * corresponding values of the RSAPIWorkingDataSet, because
 * we do not want to expand the original, close-fitting boxes.
 *
 */
void RoadSignAPI::DetectionBasedImageSlicer::
		expandBox(cv::Point* upperLeft, cv::Point* lowerRight)
{
	cv::Point calculatedBoxDimensions;

	calculatedBoxDimensions.x = lowerRight->x - upperLeft->x;

	calculatedBoxDimensions.y = lowerRight->y -upperLeft->y;

	int imageWidth = this->castedWorkingDataSet->originalImageWidth;

	int imageHeight = this->castedWorkingDataSet->originalImageHeight;

	int expandPixelAmountX = calculatedBoxDimensions.x * this->expandPercentage;

	if(expandPixelAmountX < this->minExpandPixels)
		expandPixelAmountX = this->minExpandPixels;


	int expandPixelAmountY = calculatedBoxDimensions.x * this->expandPercentage;

	if(expandPixelAmountY < this->minExpandPixels)
		expandPixelAmountY = this->minExpandPixels;

	upperLeft->x = upperLeft->x - expandPixelAmountX > 0 ?
			upperLeft->x - expandPixelAmountX : 0;

	upperLeft->y = upperLeft->y - expandPixelAmountY > 0 ?
			upperLeft->y - expandPixelAmountY : 0;

	lowerRight->x =
			lowerRight->x + expandPixelAmountX < imageWidth ?
					lowerRight->x + expandPixelAmountX : imageWidth;

	lowerRight->y =
				lowerRight->y + expandPixelAmountY < imageHeight ?
					lowerRight->y + expandPixelAmountY : imageHeight;
}

