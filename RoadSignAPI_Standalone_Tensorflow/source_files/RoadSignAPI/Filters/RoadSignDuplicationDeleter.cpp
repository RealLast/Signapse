/*
 * RoadSignDuplicationDeleter.cpp
 *
 *  Created on: 25.06.2018
 *      Author: oracion
 */

#include "RoadSignAPI/Filters/RoadSignDuplicationDeleter.h"
#include "RoadSignAPI/RSAPIPipeRegisteredFilters/RSAPIPipeRegisteredFilters.h"
#include "FilterManagementLibrary/Logger.h"

#include <algorithm>




/**
 * @brief Initializes the filter.
 *
 * As this filter does not need any specific environment variables,
 * nothing really exciting is happenning here..
 *
 * @return true, always
 */
bool RoadSignAPI::RoadSignDuplicationDeleter::initByPipeSetup()
{
	FilterManagementLibrary::Logger::printfln("RoadSignDuplicationDeleter init");

	this->castedWorkingDataSet =
			(RSAPIWorkingDataSet*) this->pipeWorkingDataSet;


	return true;
}

/** @brief The process function of this filter.
 *
 * Filters duplicates of signs (that were detected multiple times with an
 * offset). For this it iterates over the list of all signs and checks each
 * sign against each other sign and calculates the percentage of the overlap
 * of both rectangles (of the signs). If they overlap by at least
 * minOverlapPercentage (member variable), than the bigger sign is dropped.
 * Why the bigger sign, you may ask? Because it is likely that, if we have one
 * big detected box and one smaller box in it, there are actually two signs
 * inside the box. If that's the case it is likely that also the second smaller
 * sign is detected another time (so we have three detections: two smaller
 * ones, each representing one sign, and one bigger which contains the two
 * signs combined), so those two smaller detections shall be keeped. And if
 * the second smaller sign is not detected, the bigger box would probably be
 * false anyways.
 */
bool RoadSignAPI::RoadSignDuplicationDeleter::process()
{
	this->signsToDelete.clear();

	DetectedSignDescriptor* currentSignFirst;
	DetectedSignDescriptor* currentSignSecond;

	float currentOverlappingPercentage;

	int areaOfFirst;
	int areaOfSecond;

	for(int i = 0; i < this->castedWorkingDataSet->detectedSigns.size(); i++)
	{

		currentSignFirst = &this->castedWorkingDataSet->detectedSigns[i];

		areaOfFirst = this->calculateBoxArea(currentSignFirst);

		for(int j = 0;
				j < this->castedWorkingDataSet->detectedSigns.size(); j++)
		{
			if(!this->isMarkedForDeletion(j) && i != j)
			{
				//FilterManagementLibrary::Logger::printfln("Deleter process 3");
				currentSignSecond = &this->castedWorkingDataSet->detectedSigns[j];

				if(this->signsDoOverlap(currentSignFirst, currentSignSecond))
				{
					//FilterManagementLibrary::Logger::printfln("Overlap detected!");
					currentOverlappingPercentage =
							this->calculateOverlapPercentage(
									currentSignFirst, currentSignSecond);

					//FilterManagementLibrary::Logger::printfln("overlap percentage %f",
					//		currentOverlappingPercentage);

					if(currentOverlappingPercentage >
							this->minOverlapPercentage)
					{
						// mark the bigger box for deletion
						areaOfSecond = this->calculateBoxArea(
								currentSignSecond);

						this->markForDeletion(areaOfFirst > areaOfSecond ?
								i : j);
					}


				}
			}

		}
	}

	this->deleteAllMarkedSignDescriptors();
	this->invokeNext(((RSAPIPipeRegisteredFilters*)
					this->pipeRegisteredFilters)->DETECTION_BASED_IMAGE_SLICER_FILTER);
	return true;
}

/**
 * Marks the ID of the sign for deletion, thus adding it to
 * the signsToDelete (member variable) list.
 */
void RoadSignAPI::RoadSignDuplicationDeleter::markForDeletion(int ID)
{
	this->signsToDelete.push_back(ID);
}


/**
 * Deletes all signs in castedWorkingDataSet->detectedSigns, which were
 * marked for deletion during the process() function. This may change the
 * order of the detectedSigns list, because we want to avoid bigger copy
 * and / or move operations. So the element which shall be deleted is
 * overriden by the last element of the list, and afterwards the list is
 * reduced by one.
 */
void RoadSignAPI::RoadSignDuplicationDeleter::deleteAllMarkedSignDescriptors()
{
	for(int i = 0; i < this->signsToDelete.size(); i++)
	{
		// as the order of elements in the detectedSigns vector
		// is unneccessary, we use a fast deletion method which avoids
		// reallocation and bigger move operations.
		this->castedWorkingDataSet->detectedSigns[this->signsToDelete[i]] =
				this->castedWorkingDataSet->detectedSigns.back();
		this->castedWorkingDataSet->detectedSigns.pop_back();
	}
}

/*
 * @brief Checks if a sign is marked for deletion.
 * A sign is marked for deletion, if markForDeletion(...) was called for it's
 * ID in the process() function.
 *
 * @return true If the ID is contained within signsToDelete (member variable), thus
 * markForDeletion(ID) was called, false otherwise
 *
 */
bool RoadSignAPI::RoadSignDuplicationDeleter::isMarkedForDeletion(int ID) const
{
	return (std::find(this->signsToDelete.begin(),
			this->signsToDelete.end(), ID) != this->signsToDelete.end());

}

/**
 * Checks if the boxes (rectangles) of two @link{DetectedSignDescriptor}s
 * do overlap by doing some basic mathematical operations using the position
 * and dimension of the boxes.
 *
 * @return true, if they do overlap at least by one pixel, false otherwise
 */
bool RoadSignAPI::RoadSignDuplicationDeleter::signsDoOverlap(
		DetectedSignDescriptor* first, DetectedSignDescriptor* second)  const
{
	return (first->upperLeft.x < second->lowerRight.x && first->lowerRight.x > second->upperLeft.x &&
	    first->upperLeft.y < second->lowerRight.y && first->lowerRight.y > second->upperLeft.y);
}

/**
 * Calculates the percentage of which the box (rectangle) of the
 * first @link{DetectedSignDescriptor} does overlap with the second
 * (the order of first and second is relevant ! The percentage is calculated
 * relative of the area of the box of first).
 *
 * @return float Percentage of the overlapping area of the boxes of the two
 * signs relative to the area of the box of first.
 */
float RoadSignAPI::RoadSignDuplicationDeleter::calculateOverlapPercentage(
		DetectedSignDescriptor* first, DetectedSignDescriptor* second)  const
{
	int overlappingArea =
			(std::max(first->upperLeft.x, second->upperLeft.x) -
					std::min(first->lowerRight.x, second->lowerRight.x)) *
					(std::max(first->upperLeft.y, second->upperLeft.y) -
						std::min(first->lowerRight.y, second->lowerRight.y));



	return  ((float) overlappingArea * 1.0f / (float) this->calculateBoxArea(first));
}

/**
 * Calculates the area of the box of a @link{DetectedSignDescriptor}
 *
 * @return int width * height as the area of the box of the signDescriptor.
 */
int RoadSignAPI::RoadSignDuplicationDeleter::calculateBoxArea(
		DetectedSignDescriptor* signDescriptor) const
{
	return (signDescriptor->lowerRight.x -
			signDescriptor->upperLeft.x) *
			(signDescriptor->lowerRight.y -
					signDescriptor->upperLeft.y);

}
