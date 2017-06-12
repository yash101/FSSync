#include "filewatcher.h"

Watcher::FileWatcher::FileWatcher(std::string folder)
{
  this->folder = folder;
  initialize();
  begin_watching();
}

Watcher::FileWatcher::FileWatcher()
{
  initialize();
}
