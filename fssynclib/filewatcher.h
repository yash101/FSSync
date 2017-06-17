#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include "filewatcher_linux.h"

#include <string>

#define WATCHER_CREATED_DIRECTORY 1
#define WATCHER_CREATED_NON_DIRECTORY 2
#define WATCHER_ATTRIBUTES_MODIFIED 3
#define WATCHER_DELETED 4

namespace Watcher
{
  class Event
  {
  public:
    int action;
    std::string path_a;
    std::string path_b;
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
