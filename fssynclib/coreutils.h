#ifndef COREUTILS_H
#define COREUTILS_H

#if defined(DEBUG) || !defined(HIDE_ERRORS) || !defined(HIDE_WARNINGS)
#include <stdio.h>
#include <string>
#endif

#define DISABLE_INFO

#ifdef DEBUG
#define INFO(fmt, args ...)  printf(("\033[22;32m[ INFO] (%d:%s): " + std::string(fmt) + "\n").c_str(), __LINE__, __FILE__, ## args)
#define WARN(fmt, args ...) printf(("\033[01;31m[ WARN] (%d:%s): " + std::string(fmt) + "\n").c_str(), __LINE__, __FILE__, ## args)
#define ERROR(fmt, args ...) printf(("\033[22;31m[ERROR] (%d:%s): " + std::string(fmt) + "\n").c_str(), __LINE__, __FILE__, ## args)
#define FATAL(fmt, args ...) printf(("\033[22;31m[FATAL] (%d:%s): " + std::string(fmt) + "\n").c_str(), __LINE__, __FILE__, ## args)
#else
#define INFO(fmt, args ...) {}
#define WARN(fmt, args ...) {}
#define ERROR(fmt, args ...) {}
#define FATAL(fmt, args ...) {}
#endif

#ifdef DISABLE_INFO
#define INFO(fmt, args ...) {}
#endif

#endif // COREUTILS_H
