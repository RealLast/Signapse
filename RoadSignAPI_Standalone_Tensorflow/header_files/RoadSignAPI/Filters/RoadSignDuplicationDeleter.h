/*
 * RoadSignDuplicationFilter.h
 *
 *  Created on: 25.06.2018
 *      Author: oracion
 */

#ifndef HEADER_FILES_ROADSIGNAPI_FILTERS_ROADSIGNDUPLICATIONDELETER_H_
#define HEADER_FILES_ROADSIGNAPI_FILTERS_ROADSIGNDUPLICATIONDELETER_H_

#include "FilterManagementLibrary/PipeSystem/PipeFilter.h"
#include "RoadSignAPI/RSAPIWorkingDataSet/RSAPIWorkingDataSet.h"

namespace RoadSignAPI
{
	/**
	 * As it is possible that the detector detects one RoadSign
	 * at about the same position (with a small offset), it is neccessary
	 * to filter those duplicates out, because they would be classified and
	 * added to a {@link DetectedSignCombination} (by ClassifiedSignsGrouper)
	 * twice. This Filter tries to the detect duplicates by checking the
	 * congruency of the rectangles of the detected signs.
	 * If they overlap by a certain percentage (specified by
	 * minOverLapPercentage), the bigger sign will be dropped.
	 * Why the bigger sign? Because it is possible that the detector detects
	 * two signs three times: two times as single signs, and the third time
	 * combined into one sign, which is - obviously - not wanted.
	 */
	class RoadSignDuplicationDeleter :
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
			 * The minimum percentage by which two signs need to overlap to be
			 * considered equal.
			 */
			float minOverlapPercentage = 0.60;

			/**
			 * A local list which is used by markForDeletion(...),
			 * deleteAllMarkedSignDescriptors() and isMarkedForDeletion(...).
			 * It is used to remember which of the signs which were evaluated
			 * being equal in the process() function shall be deleted, because
			 * they are duplicates.
			 */
			std::vector<int> signsToDelete;

			void markForDeletion(int ID);

			void deleteAllMarkedSignDescriptors();

			bool isMarkedForDeletion(int ID) const;

			bool signsDoOverlap(DetectedSignDescriptor* first,
					DetectedSignDescriptor* second) const;

			float calculateOverlapPercentage(DetectedSignDescriptor* first,
					DetectedSignDescriptor* second) const;

			int calculateBoxArea(DetectedSignDescriptor* signDescriptor) const;

			// Get's called when the pipe's setup function is called
			// after all the filters have been registered.
			bool initByPipeSetup();

			// The main function of this filter, will be called
			// by the pipe when data shall be processed.
			bool process();
	};
}



#endif /* HEADER_FILES_ROADSIGNAPI_FILTERS_ROADSIGNDUPLICATIONDELETER_H_ */
