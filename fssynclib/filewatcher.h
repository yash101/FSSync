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
      static const ActionType CREATED = 1 << 1;
      static const ActionType DELETED = 1 << 2;
      static const ActionType MODIFIED = 1 << 3;
      static const ActionType ATTRIBUTES_MODIFIED = 1 << 4;
    };

    class DirectoryActions
    {
    public:
      static const ActionType CREATED = 1 << 5;
      static const ActionType DELETED = 1 << 6;
      static const ActionType MODIFIED = 1 << 7;
      static const ActionType ATTRIBUTES_MODIFIED = 1 << 8;
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
