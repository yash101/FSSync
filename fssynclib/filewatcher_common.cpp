#include "filewatcher.h"

Watcher::FileWatcher::FileWatcher(std::string folder, bool follow_links)
{
  this->folder = folder;
  this->follow_links = follow_links;
  initialize();
  begin_watching();
}

Watcher::FileWatcher::FileWatcher()
{
  initialize();
}
