/*
 * DetectedSignCombination.cpp
 *
 *  Created on: 21.06.2018
 *      Author: oracion
 */

#include "RoadSignAPI/DetectedSignCombination.h"
#include "FilterManagementLibrary/Logger.h"

/**
 * @brief Constructor of DetectedSignCombination.
 *
 * Intializes all member variables and constructs a new object
 * of type DetectedSignCombination. Does not do any other specific tasks.
 */
RoadSignAPI::DetectedSignCombination::DetectedSignCombination() :
			estimatedPolePositionX(0)
{
	this->signs.clear();
}

/**
 * @brief Adds a sign to the list of signs of this combination.
 *
 *Uses insertSorted(...) to add a detected sign to the
* list of signs in the this combination, so that the list is
* sorted by the y values of the signs (from top to bottom)
*/
void RoadSignAPI::DetectedSignCombination::
		addDetectedSign(DetectedSignDescriptor detectedSign)
{

	this->insertSorted(&detectedSign);

	int numSignsOnPole = this->signs.size();
	int estimatedPoleXSum = 0;

	DetectedSignDescriptor* detectedSignDescriptor;
	for(int i = 0; i < this->signs.size(); i++)
	{
		detectedSignDescriptor = &this->signs[i];

		estimatedPoleXSum +=
				(detectedSignDescriptor->lowerRight.x +
				detectedSignDescriptor->upperLeft.x) / 2;
	}


	this->estimatedPolePositionX = estimatedPoleXSum / this->signs.size();
}

/** @brief Inserts a detectedSign at the right position of this combination
*
* Uses insertion sort to add the detected sign to the
* list of signs in the this combination, so that the list is
* sorted by the y values of the signs (from top to bottom).
*/
void RoadSignAPI::DetectedSignCombination::insertSorted(
		DetectedSignDescriptor* detectedSign)
{
	if(this->signs.size() == 0)
	{
		this->signs.push_back(*detectedSign);
	}
	else
	{
		DetectedSignDescriptor* currentSign;

		int currentSignPosYUpperLeft = 0;
		int signToAddPosYUpperLeft = detectedSign->upperLeft.y;

		bool added = false;


		for(int i = 0; i < this->signs.size(); i++)
		{

			currentSign = &this->signs.at(i);
			currentSignPosYUpperLeft = currentSign->upperLeft.y;

			// the array is sorted from top to bottom, so if the (upper) y
			// coordinate of the sign to add is higher as the (upper) y
			// coordinate of the currentSign, while we are iterating
			// from top to bottom, insert the sign at the position of the
			// currentSign.
			if(signToAddPosYUpperLeft < currentSignPosYUpperLeft)
			{

				this->signs.insert(this->signs.begin() + i , *detectedSign);
				added = true;
				break;
			}



		}
		if(!added)
		{
			this->signs.push_back(*detectedSign);
		}
	}
}

/**
 * @brief Returns the amount of signs contained by this combination.
 * @return int Size of the list (= amount of entries in the list) containing
 * all @link{DetectedSignDescriptor}s of this combination.
 */
int RoadSignAPI::DetectedSignCombination::getDetectedSignsAmount() const
{
	return this->signs.size();
}

/**
 * @brief Returns the estimated pole position X coordinate of this sign combination.
 *
 * The estimated pole position X is calculated by forming the average of
 * the center X coordinate (middle of the box in which a sign is contained).
 */
int RoadSignAPI::DetectedSignCombination::getGestimatedPolePositionX() const
{
	return this->estimatedPolePositionX;
}

/**
 * @brief Returns a vector containing all Signs of this combination.
 *
 * All @link{DetectedSignCombination}s which were added to this combination
 * will be added to a list, which can be retrieved using this function.
 * This list is sorted by the y values of the signs from top to bottom.
 */
const std::vector<RoadSignAPI::DetectedSignDescriptor>*
				 RoadSignAPI::DetectedSignCombination::
									getSignsInCombination() const
{
	return &this->signs;
}

/**
 * @brief Standard Destructor of DetectedSignCombination.
 *
 * Standard Destructor of DetectedSignCombination, does not do any specific
 * tasks.
 */
RoadSignAPI::DetectedSignCombination::~DetectedSignCombination()
{

}
