/*
 * Filter.h
 *
 *  Created on: 30.05.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_PIPEFILTER_H_
#define HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_PIPEFILTER_H_

#include "FilterManagementLibrary/PipeSystem/PipeWorkingDataSet.h"
#include "FilterManagementLibrary/PipeSystem/ProcessingPipeline.h"
#include "FilterManagementLibrary/PipeSystem/PipeRegisteredFilters.h"
class ProcessingPipeline;
struct PipeRegisteredFilters;

//! Everything the FilterManagementLibrary provides
namespace FilterManagementLibrary
{
	//! Everything that has to do with the pipes and filters part of the library
	namespace PipeSystem
	{
		/**
		 * @brief Base class were all filters are derived from
		 *
		 * A filter in terms of the pipes and filters architecture is a single
		 * processing step.
		 * Originally, each filter has a data input and a data output.
		 * In our case, however, the data input and output is mapped to a
		 * single object (the data object, see PipeWorkingDataSet) allocated,
		 * on the heap, on which all operations are performed.
		 * In each processing step, the filter transforms
		 * the specific part of the data object that affects him.
		 * For this, each filter implements a function process() which
		 * defines the (conversion) functionality of the filter.
		 *
		 * During each procession, parts of the data object may be added,
		 * edited or removed completely.
		 * The type of conversion is determined by each filter.
		 * As each filter knows the structure of the data object at
		 * compile time, compatibility between each filters of a pipe is
		 * guaranteed! This means, each filter get's at least those
		 * variables of the data object it expects.
		 */
		class PipeFilter
		{
			friend class ProcessingPipeline;

			private:
				int filterID;

				// Should only be called by ProcessingPipeline!
				void setCredentials(int filterID,
						PipeRegisteredFilters* pipeRegisteredFilters,
						PipeWorkingDataSet* pipeWorkingDataSet);

				/**
				 * This is a hint for the ProcessingPipeline;
				 * any filter can indicate which filter should be invoked next.
				 * For this, PipeFilter provides invokeNext()
				 * function, whose result will be stored in this variable.
				 * A getter function is provided so that the ProcessingPipeline
				 * can query the ID of the next desired filter (if any).
				 */
				int nextDesiredFilter = -1;

				/**
				 * Set to true when the invokeNext() function has been called.
				 * A getter function is provided so that the ProcessingPipeline
				 * can query if a next filter has been marked.
				 */
				bool markedNextDesiredFilter = false;

				/**
				 * This is a hint for the ProcessingPipeline;
				 * any filter can indicate that the processing of the
				 * pipeWorkingDataSet has is finished and no more other
				 * filters of the pipe shall be called.
				 * A getter function is provided so that the ProcessingPipeline
				 * can query if a next filter has been marked.
				 */
				bool markedProcessingFinished = false;

			protected:
			 	/**
			 	 * Pointer to a derivate of PipeWorkingDataSet, being the
				 * dataSet the filters of the pipe will work on.
				 * Will be passed to all filters using their
				 * setCredentials(...) function.
				 */
				PipeWorkingDataSet* pipeWorkingDataSet;


				/**
				 * Pointer to a derivate of PipeRegisteredFilters,
				 * being a container for all ID's of the filters registered
				 * to the pipe, mapped to a unique name.
				 * Via this all filters know the ID's of every other filter
				 * registered to the pipe.
				 * The pointer will be passed to all filters using their
				 * setCredentials(...) function.
				 */
				PipeRegisteredFilters* pipeRegisteredFilters;

				/**
				 * Get's called when the pipe's setup function is called
				 * after all the filters have been registered.
				 */
				virtual bool initByPipeSetup() = 0;

				/*
				 * The main function of this filter, will be called
				 * by the pipe when data shall be processed.
				 */

				virtual bool process() = 0;

				void indicateProcessingFinished();

				void invokeNext(const int filterID);

				void reset();



			public:
				PipeFilter();

				int getFilterID() const;
				int getNextDesiredFilter() const;
				bool hasMarkedNextDesiredFilter() const;
				bool hasMarkedProcessingFinished() const;





				virtual ~PipeFilter();
		};
	}
}


#endif /* HEADER_FILES_FILTERMANAGEMENTLIBRARY_PIPESYSTEM_PIPEFILTER_H_ */
