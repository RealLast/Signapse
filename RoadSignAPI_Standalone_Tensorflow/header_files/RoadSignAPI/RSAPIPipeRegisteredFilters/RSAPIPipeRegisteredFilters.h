/*
 * RSAPIPipeRegisteredFilters.h
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_ROADSIGNAPI_RSAPIPIPEREGISTEREDFILTERS_RSAPIPIPEREGISTEREDFILTERS_H_
#define HEADER_FILES_ROADSIGNAPI_RSAPIPIPEREGISTEREDFILTERS_RSAPIPIPEREGISTEREDFILTERS_H_

#include "FilterManagementLibrary/PipeSystem/PipeRegisteredFilters.h"

namespace RoadSignAPI
{

	/**
	 * Contains variables which will hold the ID's of the filters of the API.
	 * See {@link PipeWorkingDataSet} for a detailed description of the
	 * the intention of this struct.
	 */
	struct RSAPIPipeRegisteredFilters :
			FilterManagementLibrary::PipeSystem::PipeRegisteredFilters
	{
			int SIGN_DETECTION_FILTER;
			int SIGN_RECOGNITION_FILTER;
			int SIGN_DUPLICATION_DELETER_FILTER;
			int DETECTION_BASED_IMAGE_SLICER_FILTER;
			int CLASSIFIED_SIGNS_GROUPER_FILTER;

	};
}

#endif /* HEADER_FILES_ROADSIGNAPI_RSAPIPIPEREGISTEREDFILTERS_RSAPIPIPEREGISTEREDFILTERS_H_ */
