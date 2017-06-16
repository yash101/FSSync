#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include "filewatcher_linux.h"

#include <string>

#define WATCHER_ATTRIBUTES_MODIFIED 1
#define WATCHER_DELETED 2

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
