/*
 * PipeFilter.cpp
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */


#include "FilterManagementLibrary/PipeSystem/PipeFilter.h"

/**
 * @brief Standard constructor of PipeFilter
 *
 * Initialises all class member variables to their default values.
 */
FilterManagementLibrary::PipeSystem::PipeFilter::PipeFilter() :
			pipeRegisteredFilters(nullptr),
			pipeWorkingDataSet(nullptr),
			filterID(-1),
			markedProcessingFinished(false),
			markedNextDesiredFilter(false),
			nextDesiredFilter(-1)
{
}

/**
 * @brief Function which is used by ProcessingPipeline to hand in some variables.
 *
 * When a filter is registered to a Pipeline, it needs to know on which
 * objects the pipe is working (PipeWorkingDataSet) and which filters are
 * registered to it (PipeRegisteredFilters).
 * This is done through this function, which is called by ProcessingPipeline in it's registerFilter(...) function.
 * It is not intended that this function is called anywhere outside of ProcessingPipeline.
 *
 * @param int filterID will be the ID used to identify the filter
 * @param PipeRegisteredFilters* pipeRegisteredFilters pointer to the
 * struct containing the ID's of all filters registed to the pipe
 * @param PipeWorkingDataSet* pipeWorkingDataSet pointer to the dataSet
 * the pipe is working on
 * @return void
 */
void FilterManagementLibrary::PipeSystem::PipeFilter::setCredentials(int filterID,
						PipeRegisteredFilters* pipeRegisteredFilters,
						PipeWorkingDataSet* pipeWorkingDataSet)
{
	this->filterID = filterID;
	this->pipeRegisteredFilters = pipeRegisteredFilters;
	this->pipeWorkingDataSet = pipeWorkingDataSet;

	/*
	 Or, if you like it minimalistic:
	(this->filterID, this->pipeRegisteredFilters, this->pipeWorkingDataSet)
	= (filterID, pipeRegisteredFilters, pipeWorkingDataSet);
	*/
}

/**
 * @brief Function for derived filters to indicate all processing is done.
 *
 * Whenever a Filter returns true in it's process() function, it needs to
 * inform the ProcessingPipeline about what to do next (e.g. which filter needs
 * to be invoked next). However if a Filter decides that all processing is
 * finsished and no following filter shall do further calculations on /
 * modifications to the PipeWorkingDataSet, the filter shall indicate
 * that the processing is finished (successfully, because it would
 * return true !)
 *
 * @return void
 */
void FilterManagementLibrary::PipeSystem::PipeFilter::
			indicateProcessingFinished()
{
	this->markedProcessingFinished = true;
}

/*
 * @brief Function to indicate which filter shall be invoked next
 *
 * Whenever a Filter returns true in it's process() function, it needs to
 * inform the ProcessingPipeline about what to do next (is the processing
 * finished, or is there a next filter which shall be invoked ?).
 * The indication for the latter will be done through this funtion.
 *
 * @param const int filterID ID of the filter which shall be invoked next
 * @return void
 */
void FilterManagementLibrary::PipeSystem::PipeFilter::
			invokeNext(const int filterID)
{
	this->markedNextDesiredFilter = true;
	this->nextDesiredFilter = filterID;
}

/**
 * @brief Function to reset basic variables of the filter, called by the pipe.
 *
 * This function will be called by the pipe on the
 * beginning of the processCurrentDataSet() function.
 * It is intended to only reset thoe variables used for inter-pipe-information
 * flow, so it only affects whether the processing is marked finished
 * and the filter which is marked next.
 * Reset of filter-specific variables will be done in the Filter's process
 * function. This is NO virtual function and won't be inherited!
 *
 * @return void
 */
void FilterManagementLibrary::PipeSystem::PipeFilter::reset()
{
	this->markedNextDesiredFilter = false;
	this->markedProcessingFinished = false;
	this->nextDesiredFilter = -1;
}


/**
 * @brief Returns the ID of the filter.
 *
 * @return int filterID the ID of the filter.
 */
int FilterManagementLibrary::PipeSystem::PipeFilter::getFilterID() const
{
	return this->filterID;
}

/**
 * @brief Returns the ID of the filter which shall be invoked next.
 *
 * Whenever a Filter returns true in it's process() function, it needs to
 * inform the ProcessingPipeline about what to do next (is the processing
 * finished, or is there a next filter which shall be invoked ?).
 * For the latter, the ID of the next filter will be set by
 * invokeNextFilter(...) and can be derived by the ProcessingPipeline
 * using this getter-function.
 *
 * @return int ID of the filter which shall be invoked next.
 */

int FilterManagementLibrary::PipeSystem::PipeFilter::
						getNextDesiredFilter() const
{
	return this->nextDesiredFilter;
}

/**
 * @brief Returns whether a filter which shall be invoked next has been marked.
 *
 * Whenever a Filter returns true in it's process() function, it needs to
 * inform the ProcessingPipeline about what to do next (is the processing
 * finished, or is there a next filter which shall be invoked ?).
 * For the latter, the ProcessingPipe can check if a next filter is set
 * using this function.
 *
 * @return bool true if a next filter has been marked, else otherwise
 */
bool FilterManagementLibrary::PipeSystem::PipeFilter::
						hasMarkedNextDesiredFilter() const
{
	return this->markedNextDesiredFilter;
}

/**
 * @brief Returns whether the processing on the current DataSet is finished
 *
 * Whenever a Filter returns true in it's process() function, it needs to
 * inform the ProcessingPipeline about what to do next (is the processing
 * finished, or is there a next filter which shall be invoked ?).
 * Regarding the first, the ProcessingPipeline can check that using
 * this function.
 *
 * @return bool true if the processing is marked finished, else otherwise
 */
bool FilterManagementLibrary::PipeSystem::PipeFilter::
					    hasMarkedProcessingFinished() const
{
	return this->markedProcessingFinished;
}

/**
 * @brief Standard destructor of PipeFilter. Has no functionality yet.
 */
FilterManagementLibrary::PipeSystem::PipeFilter::~PipeFilter()
{

}
