/*
 * Utilities.cpp
 *
 *  Created on: 19.05.2018
 *      Author: Patrick Langer
 */

#include "FilterManagementLibrary/Utilities.h"

#include <sys/stat.h>

/**
 * @brief Check if a file with the given path exists.
 *
 * @param const std::string& name string containing the path of the file
 *
 * @return true if the file exists, false if the file does not exist or
 * if we have insufficient permission to read it.
 */
bool FilterManagementLibrary::Utilities::fileExists
	(const std::string& name)
{
	struct stat buffer;
	return (stat (name.c_str(), &buffer) == 0);
}


