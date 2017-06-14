/*
 * FILENAME: /filewatcher_linux.h
 * AUTHOR: Devyash Lodha
 * DESCRIPTION: contains header definitions for linux-based inode watching (inotify)
 * 				In Linux, the system call, `inotify` is used to watch directories (and child files)
 * 				for changes. This is efficient, compared to polling.
 */

#ifdef __linux__
#ifndef FILEWATCHER_LINUX_H
#define FILEWATCHER_LINUX_H

#include <stdint.h>
#include <string>
#include <map>

namespace Watcher
{
  class WatchedItem
  {
  public:
    std::string location;
    int wd;
  };

  class WatcherSetup
  {
  public:
    int inotify_file_descriptor;

    inline WatcherSetup()
    {
      inotify_file_descriptor = 0;
    }

    std::map<int, WatchedItem> watching;
  };

  class InotifyEvent
  {
  public:
    uint32_t cookie;
    uint32_t mask;

    int wd;

    std::string name;
  };
}

#endif // FILEWATCHER_LINUX_H
#endif // __linux__
