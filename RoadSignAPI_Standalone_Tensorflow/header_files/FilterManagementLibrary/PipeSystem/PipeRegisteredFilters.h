/*
 * PipeRegisteredFilters.h
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_PIPEREGISTEREDFILTERS_H_
#define HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_PIPEREGISTEREDFILTERS_H_


#include "FilterManagementLibrary/PipeSystem/ProcessingPipeline.h"
#include "FilterManagementLibrary/PipeSystem/PipeFilter.h"

#include <vector>

namespace FilterManagementLibrary
{
	namespace PipeSystem
	{
		/**
		* @brief Template struct
		*
		* In from this derived structs, the user should organize it's Filters.
		* We provide this to be able store the ID's of the registered
		* Filters and get them from within any Filter which belongs
		* to the corresponding pipe.
		* Refer to ProcessingPipeline for more context; when it's used and how.
	    */

		struct PipeRegisteredFilters
		{

		};
	}
}
#endif /* HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_PIPEREGISTEREDFILTERS_H_ */
