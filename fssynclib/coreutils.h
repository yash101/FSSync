#ifndef COREUTILS_H
#define COREUTILS_H

#if defined(DEBUG) || !defined(HIDE_ERRORS) || !defined(HIDE_WARNINGS)
#include <stdio.h>
#include <string>
#endif

#ifdef DEBUG
#define INFO(fmt, args ...)  printf(("[ INFO] (%d:%s): " + std::string(fmt) + "\n").c_str(), __LINE__, __FILE__, ## args)
#define WARN(fmt, args ...) printf(("[ WARN] (%d:%s): " + std::string(fmt) + "\n").c_str(), __LINE__, __FILE__, ## args)
#define ERROR(fmt, args ...) printf(("[ERROR] (%d:%s): " + std::string(fmt) + "\n").c_str(), __LINE__, __FILE__, ## args)
#define FATAL(fmt, args ...) printf(("[FATAL] (%d:%s): " + std::string(fmt) + "\n").c_str(), __LINE__, __FILE__, ## args)
#else
#define INFO(fmt, args ...) {}
#define WARN(fmt, args ...) {}
#define ERROR(fmt, args ...) {}
#define FATAL(fmt, args ...) {}
#endif


#endif // COREUTILS_H
