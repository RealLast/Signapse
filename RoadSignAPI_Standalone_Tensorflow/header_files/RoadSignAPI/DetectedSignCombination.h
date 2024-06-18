/*
 * DetectedSignCombination.h
 *
 *  Created on: 21.06.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_ROADSIGNAPI_DETECTEDSIGNCOMBINATION_H_
#define HEADER_FILES_ROADSIGNAPI_DETECTEDSIGNCOMBINATION_H_

#include "DetectedSignDescriptor.h"

namespace RoadSignAPI
{
	/**
	 * A class combining multiple {@link DetectedSignDescriptor}s into one
	 * object. The idea is that the RoadSignAPI not only detects and classifies
	 * the signs, but also tries to determine which signs form one combination,
	 * i.e. which signs are on one pole (this is done by ClassifiedSigsGrouper
	 * filter, NOT in this class!). All signs on one pole will be stored in
	 * a DetectedSignCombination, which also calculates the estimated X
	 * coordinate in the image, where the pole is expected to be by
	 * calculating the average of the center X coordinate of all signs in
	 * the combination.
	 * Furthermore, the signs will be sorted by their y coordinates in from top
	 * to bottom (top has the lowest y value, as the coordinate origin is at
	 * the top left), what insertion sort is used for (as the signs will be
	 * added to the combination one by one).
	 */
	class DetectedSignCombination
	{
		private:

			std::vector<DetectedSignDescriptor> signs;

			/**
			 * To find out which sign belongs to a group of detected signs
			 * (a sign combination), i.e. signs on one pole, an external
			 * logic may be interested to know where the "middle"
			 * (in x direction) of the signs is. For this, we take all
			 * X positions of the signs in a combination, sum them up and
			 * calculate the average.
			 */
			int estimatedPolePositionX;


			void insertSorted(DetectedSignDescriptor* detectedSign);

		public:

			void addDetectedSign(DetectedSignDescriptor detectedSign);

			int getDetectedSignsAmount() const;

			int getGestimatedPolePositionX() const;

			const std::vector<DetectedSignDescriptor>*
						getSignsInCombination() const;


			DetectedSignCombination();
			~DetectedSignCombination();
	};
}

#endif /* HEADER_FILES_ROADSIGNAPI_DETECTEDSIGNCOMBINATION_H_ */
