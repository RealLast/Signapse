/*
 * ProcessingPipeline.cpp
 *
 *  Created on: 03.06.2018
 *      Author: Patrick Langer
 */


#include "FilterManagementLibrary/PipeSystem/ProcessingPipeline.h"
#include "FilterManagementLibrary/Logger.h"


/**
 * @brief Standard constructor of ProcessingPipeline
 *
 * The ProcessingPipeline needs to know the PipeWorkingDataSet on which
 * all operations of the filters will be performed and the
 * PipeRegisteredFilters struct which will contain the ID's of the filters
 * registered to the pipe. Pointers to these coressponding objects need
 * to be passed to this constructor. They will be passed to all
 * registered filters.
 * Also via manageExternalAllocatedRessources, it can be determined whether
 * the ProcessingPipeline will "take ownership" of all those variables
 * allocated on the heap (PipeWorkingDataSet, PipeRegisteredFilters and
 * all registered Filters themselves), which means it will delete them
 * on destruction.
 * All other class members will be initialised to their defult values.
 *
 * @param PipeWorkingDataSet* a pointer to an object of a struct derived from
 * PipeWorkingDataSet. This is the data set the filters work on and
 * will be passed to all registered filters.
 *
 * @param PipeRegisteredFilters* a pointer to an object of a struct derived from
 * PipeRegisteredFilters. The struct should contain (integer) values to
 * the ID's of all registered Filters. The filters need this for their
 * invokeNext(const int filterID) function to know the ID of the filter
 * they want to invoke next.
 *
 * @param bool manageExternalAllocatedRessources indicate whether the
 * ProcessingPipeline shall take ownership of all variables passed to
 * it which were allocated on the heap, therefore deleting them on destruction.
 */
FilterManagementLibrary::PipeSystem::ProcessingPipeline::ProcessingPipeline(
		PipeWorkingDataSet* workingDataSet,
		PipeRegisteredFilters* pipeRegisteredFiltersHeader,
		bool manageExternallyAllocatedRessources) :
		workingDataSet(workingDataSet),
		pipeRegisteredFiltersHeader(pipeRegisteredFiltersHeader),
		manageExternalAllocatedRessources(manageExternalAllocatedRessources),
		currentFilterID(0), lastError(ErrorType::ERROR_NONE)

{

}

/**
 * @brief Registers a filter onto the ProcessingPipeline.
 *
 * Each filter which shall be included into the ProcessingPipeline needs
 * to be registered to it using this function.
 *
 * @param PipeFilter* filter pointer to the filter to register
 * @param int filterID a pointer to an integer whose value will be changed
 * in this function. It will contain the ID of the newly registered filter
 * afterwards.
 *
 * @return void
 */
void FilterManagementLibrary::PipeSystem::ProcessingPipeline::registerFilter
		(PipeFilter* filter, int* filterID)
{
	*filterID = this->registeredFilters.size();

	filter->setCredentials(*filterID,
			this->pipeRegisteredFiltersHeader, this->workingDataSet);
	this->registeredFilters.push_back(filter);
}

/**oracion
 * @brief Set up the pipe and it's filters.
 *
 * Will iterate over all filters and call initByPipeSetup() on them.
 * On failure, lastError will be set accordingly.
 *
 * @return bool true if all filters return true in their initByPipeSetup()
 * functions, false otherwise.
 */
bool FilterManagementLibrary::PipeSystem::ProcessingPipeline::setup()
{
	Logger::printfln("Setting up pipe");
	for(PipeFilter* filter : this->registeredFilters)
	{
		Logger::printfln("Setting up Filter %d", filter->getFilterID());
		if(filter->initByPipeSetup())
		{
			Logger::printfln("Filter %d set up successfully",
					filter->getFilterID());
		}
		else
		{
			Logger::printfln("Failed to set up Filter %d, aborting",
					filter->getFilterID());
			this->lastError = ErrorType::ERROR_FILTER_SETUP_FAILED;
			return false;
		}
	}

	return true;
}



/**
 * @brief Process the PipeWorkingDataSet.
 *
 * Will iterate over all filters and call their process() functions, which
 * means the DataSet will be processed.
 * Each filter will (should !) indicate either if the processing has been finished,
 * or if (and which) a filter shall be invoked next, as long as their
 * process function returns true.
 * Otherwise, if a filter returns false, this means the current dataset couldn't
 * be processed.
 * Either way, in any case of failure, lastError will be set accordingly.
 *
 * @return bool true if all filters return true in their process()
 * functions AND either indicate which filter to invoke next or that the
 * processing is finished, false otherwise.
 */
bool FilterManagementLibrary::PipeSystem::ProcessingPipeline::
	processCurrentDataSet()
{
	this->processingFinishied = false;
	PipeFilter* currentFilter;

	// First filter is always ID 0!
	currentFilter = this->registeredFilters[0];
	currentFilter->reset();

	bool success = false;

	success = currentFilter->process();

	if(success)
	{
		if(currentFilter->hasMarkedProcessingFinished())
		{
			//FilterManagementLibrary::Logger::
			//				printfln("Filter %d indicated working data "
			//						"has been fully processed successfully!",
			//						this->currentFilterID);
			return true;
		}
	}
	else
		return false;


	while(!this->processingFinishied &&
			currentFilter->hasMarkedNextDesiredFilter())
	{
		// Check if the ID of the next desired Filter
		// is within the range of our List of registered Filters
		if(currentFilter->getNextDesiredFilter() <
				this->registeredFilters.size() &&
				currentFilter->getNextDesiredFilter() > 0)
		{
			this->currentFilterID = currentFilter->getNextDesiredFilter();
			currentFilter = this->registeredFilters[this->currentFilterID];

			// Mandatory!
			currentFilter->reset();

			//FilterManagementLibrary::Logger::
			//	printfln("Invoking Filter %d", this->currentFilterID);

			if(currentFilter->process())
			{
				//FilterManagementLibrary::Logger::
				//	printfln("Filter %d successfully finished it's operations",
				//		this->currentFilterID);
			}
			else
			{
				FilterManagementLibrary::Logger::
					printfln("Processing failed: Filter %d indicated failure",
							this->currentFilterID);
				this->lastError = ProcessingPipeline::ErrorType::
						ERROR_FILTER_INDICATED_FAILURE;
				return false;
			}

			this->processingFinishied =
					currentFilter->hasMarkedProcessingFinished();
		}
		else
		{
			FilterManagementLibrary::Logger::
					printfln("Filter %d wanted to invoke Filter %d\
							 althoguh this ID is not assigned.",
									this->currentFilterID,
									currentFilter->getNextDesiredFilter());
			this->lastError = ProcessingPipeline::ErrorType::
										ERROR_FILTER_INDICATED_FAILURE;
			return false;
		}
	}

	if(!this->processingFinishied &&
			!currentFilter->hasMarkedNextDesiredFilter())
	{
		// currentFilter successfully finished it's calculations,
		// but did neither indicated a next filter nor that the
		// workingDataSet is fully processed.
		// Has to be a logic error !

		FilterManagementLibrary::Logger::
			printfln("Logic fault, Filter %d finished it's operations\
					but neither indicated it the workingDataSet is \
					fully processed, nor which Filter shall\
					be invoked next", this->currentFilterID);
		this->lastError = ProcessingPipeline::ErrorType::ERROR_LOGIC_FAULT;
		return false;
	}
	else
	{
		// Processing has to be finished successfully!
		//FilterManagementLibrary::Logger::
		//	printfln("Filter %d indicated working data\
		//			has been fully processed successfully!",
		//			this->currentFilterID);
		return true;
	}
}


/**
 * @brief Returns an (enum) ID of the last error that happened.
 *
 * @return ProcessingPipeline::ErrorType the enum value of the last error
 * that happened.
 */
FilterManagementLibrary::PipeSystem::ProcessingPipeline::ErrorType
	FilterManagementLibrary::PipeSystem::ProcessingPipeline::getLastError() const
{
	return this->lastError;
}

/**
 * @brief Destructor of ProcessingPipeline.
 *
 * If manageExternalAllocatedRessources was set to true in constructor,
 * all variables on the heap (PipeWorkingDataSet, PipeRegisteredFilters,
 * the filters itselves) will be deleted.
 * Otherwise the destructor does do nothing.
 *
 * @return ProcessingPipeline::ErrorType the enum value of the last error
 * that happened.
 */
FilterManagementLibrary::PipeSystem::ProcessingPipeline::~ProcessingPipeline()
{
	if(this->manageExternalAllocatedRessources)
	{
		for(PipeFilter* filter : this->registeredFilters)
		{
			delete filter;
		}
		delete this->workingDataSet;
		delete this->pipeRegisteredFiltersHeader;
	}
}
