/*
 * ProcessingPipeline.h
 *
 *  Created on: 30.05.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_PROCESSINGPIPELINE_H_
#define HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_PROCESSINGPIPELINE_H_

#include "FilterManagementLibrary/PipeSystem/PipeFilter.h"
#include "FilterManagementLibrary/PipeSystem/PipeWorkingDataSet.h"
#include "FilterManagementLibrary/PipeSystem/PipeRegisteredFilters.h"
#include <vector>


class PipeFilter;
namespace FilterManagementLibrary
{
	namespace PipeSystem
	{
		/**
		 * @brief Pipeline of the pipes and filters architecture.
		 *
		 * The Pipes-and-Filters-Architecture is a software architectural
		 * pattern usually used for systems processing data or data streams
		 * in a specific way.
		 * A pipe, which can be seen as a sequence of processing operations,
		 * represents the connection between the individual filters
		 * (processing steps).
		 * You may refer to {@link PipeFilter} for a more detailed explanation of the
		 * filters part of the architecture.
		 *
		 * The output of a processing step (filter) can be seen as the input
		 * for the subsequent processing step, although in this implementation
		 * the input and output data is mapped to a single object allocated
		 * on the heap, the {@link PipeWorkingDataSet}.
		 *
		 * As this implementation is based on the
		 * "Tee-and-Joins-Pipes-and-Filters-Architecture", the order in which
		 * the filters are executed can change dynamically.
		 * For now this is implemented as follows:
		 * 	Each filter provides a function process().
		 * 	In this function a filter either has to indicate which filter
		 * 	shall be invoked next, or that it claims to be the last filter
		 * 	of the processing at least for the current iteration, which means
		 * 	the current data set has been fully processed.
		 * 	The filter may decide differently for other input data, however.
		 * 	Strictly speaking, this means every filter has to give a hint to
		 * 	the pipe to allow it to decide what to do next.
		 *  For this, each filter knows the ID's of all registered filters
		 *  on the pipe itself does belong to. These ID's are provided via
		 *  a shared object, a derivate of the PipeRegisteredFilters struct.
         * 
         *  @image latex api_class_filtermanagement.png width=16cm
         *  @image html api_class_filtermanagement.svg
		 */
		class ProcessingPipeline
		{
			public:
				enum ErrorType
				{
					/**
					 * Standard value set on initialization.
					 * Needed because getLastError() shall not return a
					 * random value (which could cause unexpected behavior)
					 */
					ERROR_NONE,

					/**
					 * E.g. if a filter neither marked a next filter nor
					 * indicated the workingDataSet has been fully processed.
					 */
					ERROR_LOGIC_FAULT,

					/**
					 * If a filter returned false in process()
					 */
					ERROR_FILTER_INDICATED_FAILURE,

					/**
					 * If a filter wants to invoke a next filter
					 * with an ID that is not assigned.
					 */
					ERROR_FILTER_ID_NOT_FOUND,

					/**
					 * When we reun initByPipeSetup() on a filter
					 * and it returned false, which means the
					 * filter could not be set up and all filters depending
					 * on it may not be able to perform their tasks.
					 */
					ERROR_FILTER_SETUP_FAILED,

				};


			private:
				/**
				 * ID of the currently active filter.
				 * Each filter ID is unique and has to be positive.
				 * Uniqueness is assured by the pipe itself due to
				 * continuous numerating in registerFilter()
				 */
				int currentFilterID = 0;

				/**
				 * Stores whether the last filter indicated the current
				 * data set has been fully processed.
				 */
				bool processingFinishied = false;


				/**
				 * Indicate whether we shall manage the heap allocated memory
				 * of workingDataSet, pipeRegisteredFiltersHeader and the
				 * themselves, i.e. explicitly delete those externally
				 * allocated objects in our destructor.
				 */
				bool manageExternalAllocatedRessources = true;

				/**
				 * Just a list containing pointers to all filters registered
				 * on this instance of the pipe.
				 */
				std::vector<PipeFilter*> registeredFilters;

				/**
				 * Error code of the last error that happened.
				 * See ProcessingPipeline::ErrorType
				 */
				ErrorType lastError;

			protected:

				/**
				 * Pointer to a derivate of PipeWorkingDataSet, being the
				 * dataSet the filters of the pipe will work on.
				 * Will be passed to all filters using their
				 * setCredentials(...) function.
				 */
				PipeWorkingDataSet* workingDataSet;


				/**
				 * Pointer to a derivate of PipeRegisteredFilters,
				 * being a container for all ID's of the filters registered
				 * to the pipe, mapped to a unique name.
				 * Via this all filters know the ID's of every other filter
				 * registered to the pipe.
				 * The pointer will be passed to all filters using their
				 * setCredentials(...) function.
				 */
				PipeRegisteredFilters* pipeRegisteredFiltersHeader;

			public:
				ProcessingPipeline(PipeWorkingDataSet* workingDataSet,
						PipeRegisteredFilters* pipeRegisteredFiltersHeader,
						bool manageExternalAllocatedRessources = true);

				void registerFilter(PipeFilter* filter, int* filterID);

				ErrorType getLastError() const;

				bool setup();

				bool processCurrentDataSet();

				~ProcessingPipeline();
		};
	}
}



#endif /* HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_PROCESSINGPIPELINE_H_ */
