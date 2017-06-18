#ifndef FSSYNCLIB_H
#define FSSYNCLIB_H

#define VERSION "0.0.0"

inline static const char* getVersion()
{
  return VERSION;
}

inline static const char* getLastBuildTime()
{
  return __TIME__;
}

inline static const char* getLastBuildData()
{
  return __DATE__;
}

#endif // FSSYNCLIB_H
