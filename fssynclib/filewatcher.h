#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include "filewatcher_linux.h"

#include <string>

namespace Watcher
{
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

    void watch_();
    void watch();
  };
}

#endif // FILEWATCHER_H