#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include "filewatcher_linux.h"

#include <string>

#define WATCHER_CREATE_FILE 1
#define WATCHER_DELETE_FILE 2
#define WATCHER_CREATE_DIRECTORY 3
#define WATCHER_DELETE_DIRECTORY 4
#define FILE_MODIFIED 5

namespace Watcher
{
  class Event
  {
  public:
    int action;
    std::string a;
    std::string b;
  };

  class FileWatcher
  {
  private:
    WatcherSetup setup;
    std::string folder;

    int initialize();
    int begin_watching();

    int add_directory(const char* directory);

  public:
    FileWatcher(std::string folder);
    FileWatcher();

    void watch();
  };
}

#endif // FILEWATCHER_H
