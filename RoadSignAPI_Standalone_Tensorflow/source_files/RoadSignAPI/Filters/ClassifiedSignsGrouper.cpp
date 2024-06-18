/*
 * ClassifiedSignsGrouper.cpp
 *
 *  Created on: 21.06.2018
 *      Author: oracion
 */


#include "RoadSignAPI/Filters/ClassifiedSignsGrouper.h"


/**
 * @brief Initializes the filter.
 *
 * As this filter does not need any specific environment variables,
 * nothing really exciting is happenning here..
 *
 * @return true, always
 */
bool RoadSignAPI::ClassifiedSignsGrouper::initByPipeSetup()
{
	FilterManagementLibrary::Logger::printfln("ClassifiedSignsGrouper init");

	this->castedWorkingDataSet =
			(RSAPIWorkingDataSet*) this->pipeWorkingDataSet;


	return true;
}

/** @brief The process function of this filter.
 *
 * Iterates through all signs that were successfully classified and
 * whose IDs were thus added to the classifierApprovedSigns list
 * of the @link{RSAPIWorkingDataSet}. It calculates an estimated
 * Position of the pole of a sign (by using the middle of the
 * X coordinates of the edges of the box in which a sign is contained)
 * and adding a threshold to it defined in the local member variable
 * horizontalRangePercentage. which should not be to big!
 * For the first sign in the list, a new @link{DetectedSignCombination}
 * will be created. For all other signs, they are checked against all
 * existing signCombinations (so for the second sign, there is exactly one!).
 * If their X coordinates match the estimated pole area of a
 * @link{DetectedSignCombination), than they are added to it (one sign
 * is only added to one combination!). If no matching combination were found,
 * a new one is created.
 *
 */
bool RoadSignAPI::ClassifiedSignsGrouper::process()
{
	// Store current image width in a local copy
	this->imageWidth = this->castedWorkingDataSet->originalImageWidth;

	// first is the left boundary, second is the right boundary
	// of the box in (from y = 0 : imageHeight) in which the pole
	// of the signCombination is expected.
	std::pair<int, int> detectedSignCombinationEstimatedPoleArea;

	int currentGroupEstimatedPolePositionX;

	int currentSignHorizontalMiddle = 0;

	int poleAreaHorizontalRangeInEachDirection =
			this->imageWidth * this->horizontalRangePercentage;


	DetectedSignDescriptor* currentDetectedSignDescriptor;

	bool signAddedToExistingCombination = false;
	DetectedSignCombination* detectedSignCombination;
	for(int classifiedSignID : this->castedWorkingDataSet->classifierApprovedSigns)
	{

		signAddedToExistingCombination = false;

		currentDetectedSignDescriptor =
				&this->castedWorkingDataSet->detectedSigns[classifiedSignID];

		currentSignHorizontalMiddle =
				(currentDetectedSignDescriptor->lowerRight.x +
				currentDetectedSignDescriptor->upperLeft.x) / 2;


		// Iterate over all already created sign combinations and check
		// if the current sign fits into the combination because it's
		// estimated pole position is contained within the
		// detectedSignCombinationEstimatedPoleArea of the current combination.
		if(this->castedWorkingDataSet->detectedSignCombinations.size() > 0)
		{
			for(int j = 0; j <
				this->castedWorkingDataSet->detectedSignCombinations.size(); j++)
			{
				// Calculate the estimated pole area of the sign combination.

				detectedSignCombination = &this->castedWorkingDataSet->detectedSignCombinations[j];

				// First is left boundary

				detectedSignCombinationEstimatedPoleArea.first =
						detectedSignCombination->getGestimatedPolePositionX()
						- poleAreaHorizontalRangeInEachDirection;

				if(detectedSignCombinationEstimatedPoleArea.first < 0)
					detectedSignCombinationEstimatedPoleArea.first = 0;

				// Second is right boundary
				detectedSignCombinationEstimatedPoleArea.second =
						detectedSignCombination->getGestimatedPolePositionX()
						+ poleAreaHorizontalRangeInEachDirection;

				if(detectedSignCombinationEstimatedPoleArea.second
							> this->imageWidth)
								detectedSignCombinationEstimatedPoleArea.second
									= this->imageWidth;

				if(detectedSignCombinationEstimatedPoleArea.second > this->imageWidth)
					detectedSignCombinationEstimatedPoleArea.second = this->imageWidth;




				// Check if the current sign fits into the combination.


				if(currentSignHorizontalMiddle >=
						detectedSignCombinationEstimatedPoleArea.first &&
						currentSignHorizontalMiddle <=
						detectedSignCombinationEstimatedPoleArea.second)
				{
					signAddedToExistingCombination = true;

					// This will also recalculate the estimatedPolePositionX
					// of the combination.
					detectedSignCombination->addDetectedSign(
							*currentDetectedSignDescriptor);
					break;
				}
			}
		}

		if(!signAddedToExistingCombination)
		{
			// In this case we create a new combination and add the sign to it.
			DetectedSignCombination newDetectedSignCombination;
			newDetectedSignCombination.addDetectedSign(*currentDetectedSignDescriptor);
			this->castedWorkingDataSet->detectedSignCombinations.
								push_back(newDetectedSignCombination);
		}
	}

	this->indicateProcessingFinished();
	return true;
}
