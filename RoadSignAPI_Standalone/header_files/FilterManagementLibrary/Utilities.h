/*
 * Utilities.h
 *
 *  Created on: 19.05.2018
 *      Author: oracion
 */

#ifndef HEADER_FILES_UTILITIES_H_
#define HEADER_FILES_UTILITIES_H_

#include <string>

namespace FilterManagementLibrary
{
	class Utilities
	{
		public:
			static bool fileExists(const std::string& name);
	};
}



#endif /* HEADER_FILES_UTILITIES_H_ */
