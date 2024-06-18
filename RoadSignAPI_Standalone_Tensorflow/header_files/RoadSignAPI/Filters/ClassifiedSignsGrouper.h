/*
 * ClassifiedSignGrouper.h
 *
 *  Created on: 21.06.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_ROADSIGNAPI_FILTERS_CLASSIFIEDSIGNSGROUPER_H_
#define HEADER_FILES_ROADSIGNAPI_FILTERS_CLASSIFIEDSIGNSGROUPER_H_

#include "FilterManagementLibrary/PipeSystem/PipeFilter.h"
#include "RoadSignAPI/DetectedSignCombination.h"
#include "RoadSignAPI/RSAPIWorkingDataSet/RSAPIWorkingDataSet.h"
#include "RoadSignAPI/RSAPIPipeRegisteredFilters/RSAPIPipeRegisteredFilters.h"
#include "FilterManagementLibrary/Logger.h"


namespace RoadSignAPI
{
	/**
	 * For the user, it is nice to know which signs are mounted onto the
	 * same pole, so they are logically grouped. This has a direct effect
	 * on the scope of some signs (i.e. signs indicating danger can limit
	 * the validity of speed limits for the duration of a dangerous right turn
	 * etc.). This Filter aims to group those signs into one
	 * @link{DetectedSignCombination}. For this, it estimates where the pole
	 * of a sign is located (for more, please refer to the process() function
	 * and to @link{DetectedSignCombination}.
	 */
	class ClassifiedSignsGrouper :
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
			 * We store a local copy to the originalImageWidth of the
			 * working data set (just for easier access and aesthetically more
			 * pleasing code ;) )
			 */
			int imageWidth;

			/**
			 * We try to find out which signs are on one pole. For this, we
			 * take the middle of the box of the detected sign and add a threshold
			 * in +X and -X direction, to guess where the pole is. This variable
			 * specifies how big this threshold is (i.e. how wide the area is in
			 * which the pole is expected).
			 * The percentage refers to the total width of the originalBGRImage.
			 * If the width for example is 1280, a horizontalPercentageRange of
			 * 10% would mean 128 pixels in EACH direction (which, of course would
			 * be a lot, so we would use something around 1-5%...).
			 * If we would exceed the image boundaries while applying this value,
			 * we of course would adjust it to stay within the boundaries
			 * (we assert that the range always starts where x > 0 and ends
			 * where x < originalBGRImageWidth).
			 */
			float horizontalRangePercentage = 0.025;

			// Get's called when the pipe's setup function is called
			// after all the filters have been registered.
			bool initByPipeSetup();

			// The main function of this filter, will be called
			// by the pipe when data shall be processed.
			bool process();


	};
}




#endif /* HEADER_FILES_ROADSIGNAPI_FILTERS_CLASSIFIEDSIGNSGROUPER_H_ */
