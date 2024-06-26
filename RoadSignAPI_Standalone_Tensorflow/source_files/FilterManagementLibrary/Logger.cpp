/*
 * Logger.cpp
 *
 *  Created on: 28.05.2018
 *      Author: Patrick Langer
 */

#include "FilterManagementLibrary/Logger.h"

#include <sstream>
#include <stdarg.h>
#include <cstdio>
#include <iostream>
#include <ctime>

#include <string>
#include <stdexcept>
std::string FilterManagementLibrary::Logger::logTag = "FilterManagementLibrary";

#ifdef __ANDROID__
	#include <android/log.h>
	#define  LOG_TAG    "testjni"
	#define  LOGCAT(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#endif

/**
 * @brief prints a new line to the console
 *
 * Takes a format c-string and a variable amount of parameters
 * and formats an string accordingly.
 * This string will be printed prefixed with the current time
 * and the specified logTag.
 *
 * @param const char* format c-string containing the string which
 * shall be printed. May contain conversion specifiers like %d, %f etc.
 *
 * @param ... variable of amount of parameters used to format the
 * conversion specifiers.
 */
void FilterManagementLibrary::Logger::printfln(const char *format, ...)
{
	va_list vargs;

	va_start(vargs, format);


    char buffer[500] = "";
	std::vsprintf(buffer, format, vargs);


	va_end(vargs);
	std::stringstream ss;
	std::string timeString;
	FilterManagementLibrary::Logger::getTimeString(&timeString);

	ss << "[" << FilterManagementLibrary::Logger::logTag << " | "
			<< timeString << "] " << buffer;

	#ifdef __ANDROID__

		LOGCAT(ss.str().c_str(), __LINE__);
	#else
		std::cout << ss.str().c_str() << "\n";
	#endif

}

/**
 * @brief In-class helper function to get a string containing the current time.
 *
 * Class intern helper function. It provides a string containing
 * the current time as follows: hh:mm::ss
 * hh being the hour
 * mm being the minute
 * ss being the second
 *
 * @param std::string *timeStr pointer to a string to wich the
 * time string will be written to.
 *
 * @return void.
 */
void FilterManagementLibrary::Logger::getTimeString(std::string *timeStr)
{
	time_t t = time(0);
	struct tm * now = localtime(&t);

	std::stringstream ss;

	if (now->tm_hour < 10)
		ss << "0" << now->tm_hour << ":";
	else
		ss << now->tm_hour << ":";

	if (now->tm_min < 10)
		ss << "0" << now->tm_min << ":";
	else
		ss << now->tm_min << ":";

	if (now->tm_sec < 10)
		ss << "0" << now->tm_sec;
	else
		ss << now->tm_sec;

	*timeStr = ss.str();
}

/**
 * @brief sets the logTag of the logger
 *
 * The logTag is a prefix which will be printed when using printfln
 * after the time string.
 *
 * @param std::string logTag string containing the logTag
 *
 * @return void
 */
void FilterManagementLibrary::Logger::setLogTag(std::string logTag)
{
	FilterManagementLibrary::Logger::logTag = logTag;
}

