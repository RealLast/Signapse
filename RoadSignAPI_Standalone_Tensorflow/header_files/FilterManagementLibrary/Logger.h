/*
 * Logger.h
 *
 *  Created on: 28.05.2018
 *      Author: Patrick Langer
 */

#ifndef HEADER_FILES_LOGGER_H_
#define HEADER_FILES_LOGGER_H_

#include <string>

namespace FilterManagementLibrary
{
	/**
	 * @brief  Logger providing Desktop Linux <-> Android platform independent logging
	 *
	 * While compiling, it's checked if the library is built for Android
	 * or any other architecture. The logger class provides platform indepent
	 * logging by using preprocessor directives to only compile those
	 * functions that are needed on the given architecture.
	 * On Linux for example, the logger prints to stdout.
	 * On Android however, the logging library is used and the logger
	 * prints to the logcat output console.
	 */
	class Logger
	{
		private:
				/**
				 * A tag to display in every print.
				 * By default it is set to "FilterManagementLibrary".
				 * With this, every use of printfln(...) would look like this:
				 * [00:38:12 - FilterManagementLibrary]: Logger print test.
				 * Notice that every print is prefixed with the current time
				 * for convenience.
				 */
				static std::string logTag;

				static void getTimeString(std::string *timeStr);
		public:
			static void printfln(const char *format, ...);
			static void setLogTag(std::string logTag);
	};
}


#endif /* HEADER_FILES_LOGGER_H_ */
