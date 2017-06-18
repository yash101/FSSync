#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include "filewatcher_linux.h"

#include <string>
#include <vector>

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
    bool follow_links;
    int add_directory_recurse_length;

    int initialize();
    int begin_watching();

    int add_directory(const char* directory);

    void process_events(std::vector<Watcher::InotifyEvent>& events);

  public:
    FileWatcher(std::string folder, bool follow_links);
    FileWatcher();

    void watch();
  };
}

#endif // FILEWATCHER_H
