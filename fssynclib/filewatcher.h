#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include "filewatcher_linux.h"

#include <string>

namespace Watcher
{
  typedef int ActionType;
  class Event
  {
  public:
    ActionType action;
    std::string path_a;
    std::string path_b;

    class FileActions
    {
    public:
      static const ActionType CREATED = 1;
      static const ActionType DELETED = 2;
      static const ActionType MODIFIED = 3;
      static const ActionType ATTRIBUTES_MODIFIED = 4;
    };

    class DirectoryActions
    {
    public:
      static const ActionType CREATED = 101;
      static const ActionType DELETED = 102;
      static const ActionType MODIFIED = 103;
      static const ActionType ATTRIBUTES_MODIFIED = 104;
    };
  };

  class FileWatcher
  {
  private:
    WatcherSetup setup;
    std::string folder;

    int initialize();
    int begin_watching();

    int add_directory(const char* directory);

    bool follow_links;

  public:
    FileWatcher(std::string folder, bool follow_links);
    FileWatcher();

    void watch();
  };
}

#endif // FILEWATCHER_H
