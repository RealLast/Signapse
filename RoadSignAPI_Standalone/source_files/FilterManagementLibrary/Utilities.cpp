/*
 * Utilities.cpp
 *
 *  Created on: 19.05.2018
 *      Author: oracion
 */

#include "FilterManagementLibrary/Utilities.h"

#include <sys/stat.h>

bool FilterManagementLibrary::Utilities::fileExists
	(const std::string& name)
{
	struct stat buffer;
	return (stat (name.c_str(), &buffer) == 0);
}


