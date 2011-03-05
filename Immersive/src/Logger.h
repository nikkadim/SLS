/*
 Filename:		Logger.h
 Author:		Nikolaus Karpinsky
 Date Created:	09/20/10
 Last Edited:	09/20/10
 
 Revision Log:
 09/20/10 - Nik Karpinsky - Original creation.
 */

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <iostream>
#include <string>

using namespace std;

class Logger
{
public:
	static void logError(const string &message);
	static void logDebug(const string &message);
};

#endif	// _LOGGER_H_