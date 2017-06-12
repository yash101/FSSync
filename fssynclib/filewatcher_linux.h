#ifdef __linux__
#ifndef FILEWATCHER_LINUX_H
#define FILEWATCHER_LINUX_H

/*
 * FILENAME: /filewatcher_linux.h
 * AUTHOR: Devyash Lodha
 * DESCRIPTION: contains header definitions for linux-based inode watching (inotify)
 * 				In Linux, the system call, `inotify` is used to watch directories (and child files)
 * 				for changes. This is efficient, compared to polling.
 */
namespace Watcher
{
  class WatcherSetup
  {
  public:
    int inotify_file_descriptor;

    inline WatcherSetup()
    {
      inotify_file_descriptor = 0;
    }
  };
}

#endif // FILEWATCHER_LINUX_H
#endif // __linux__
