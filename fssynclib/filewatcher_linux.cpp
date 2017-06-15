/*

Author: Devyash Lodha
File: filewatcher_linux.cpp
Description:
    Linux implementation of file system watching. Uses inotify for speed and efficiency.
    If a file or directory is added or changed, a notification will be sent. Otherwise,
    would have to poll to detect changes

*/

#ifdef __linux__
#include "filewatcher.h"
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <vector>
#include <map>

#define MAKE_BLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK)
#define MAKE_UNBLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

int Watcher::FileWatcher::initialize()
{
  setup.inotify_file_descriptor = inotify_init();
  MAKE_BLOCK(setup.inotify_file_descriptor);
  return 0;
}

#define WATCH_ATTR IN_ATTRIB | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_MOVE_SELF | IN_MOVE | IN_UNMOUNT | IN_EXCL_UNLINK

#define ATTRIB_TO_STRING(attrib, str) \
  if(attrib & IN_ATTRIB) str += " IN_ATTRIB ";\
  if(attrib & IN_CREATE) str += " IN_CREATE ";\
  if(attrib & IN_DELETE) str += " IN_DELETE ";\
  if(attrib & IN_DELETE_SELF) str += " IN_DELETE_SELF ";\
  if(attrib & IN_MODIFY) str += " IN_MODIFY ";\
  if(attrib & IN_MOVE_SELF) str += " IN_MOVE_SELF ";\
  if(attrib & IN_MOVE) str += " IN_MOVE ";\
  if(attrib & IN_MOVED_TO) str += " IN_MOVED_TO ";\
  if((attrib & IN_MOVED_FROM) == IN_MOVED_FROM) str += " IN_MOVED_FROM ";\
  if(attrib & IN_UNMOUNT) str += " IN_UNMOUND ";\
  if(attrib & IN_EXCL_UNLINK) str += " IN_EXCL_UNLINK "

int Watcher::FileWatcher::begin_watching()
{
  if(folder.back() == '/' || folder.back() == '\\')
    folder.pop_back();
  Watcher::WatchedItem item;
  item.wd = inotify_add_watch(setup.inotify_file_descriptor, folder.c_str(), WATCH_ATTR);
  item.location = folder;

#ifdef DEBUG
  printf("Adding directory, '%s'\n", folder.c_str());
#endif

  if(item.wd < 0)
  {
    int err = errno;
    printf("[%s](%d) Failed to add root directory, '%s'\n", strerror(err), err, folder.c_str());
    return -1;
  }

  setup.watching[item.wd] = item;

  add_directory(folder.c_str());
  return 0;
}

int Watcher::FileWatcher::add_directory(const char* directory)
{
  //Open the directory, `const char* directory`
  DIR* d;
  struct dirent* dir;

  d = opendir(directory);

  if(d)
  {
    while((dir = readdir(d)) != NULL)
    {
      //Full path to file/directory
      std::string fpath = directory + std::string("/") + dir->d_name;

      //Used to get file/folder information
      struct stat st;
      stat(fpath.c_str(), &st);

      //If the file? is not named '.' or '..' and the file? is a directory
      if(strcmp(fpath.c_str(), ".") && strcmp(fpath.c_str(), "..") && S_ISDIR(st.st_mode) != 0)
      {

        //Print debug information to the command line
#ifdef DEBUG
        printf("[INFO] [%d:%s]: Adding directory, INODE=%lu, name=%s, path=%s\n", __LINE__, __FILE__, dir->d_ino, dir->d_name, fpath.c_str());
#endif
        //Stores the watch information
        Watcher::WatchedItem item;
        item.location = fpath;
        item.wd = inotify_add_watch(setup.inotify_file_descriptor, fpath.c_str(), WATCH_ATTR);

        if(item.wd < 0)
        {
          int err = errno;
          printf("[ERROR] [%d:%s]: Failed creating inotify watch; ERRNO: [%d:%s]\n", __LINE__, __FILE__, err, strerror(err));
          continue;
        }

        setup.watching[item.wd] = item;

#ifdef DEBUG
        printf("[INFO] [%d:%s]: Successfully added directory, %s, to the inotify event queue\n", __LINE__, __FILE__, fpath.c_str());
#endif

        add_directory(fpath.c_str());

      }
#ifdef DEBUG
      else
      {
        printf("[INFO] [%d:%s]: Skipped adding: INODE=%lu, name=%s, path=%s because it is not a directory.\n", __LINE__, __FILE__, dir->d_ino, dir->d_name, fpath.c_str());
      }
#endif
    }

    closedir(d);
  }


  return 0;
}

static void prune(std::vector<Watcher::InotifyEvent>& events, decltype(Watcher::WatcherSetup::watching)& watching)
{
  //Map of pointers to each event
  std::map<decltype(Watcher::InotifyEvent::cookie), std::vector<Watcher::InotifyEvent*> > mp;
  for(size_t i = 0; i < events.size(); i++)
  {
    mp[events[i].cookie].push_back(&events[i]);
  }

  for(std::map<decltype(Watcher::InotifyEvent::cookie), std::vector<Watcher::InotifyEvent*> >::const_iterator it = mp.begin(); it != mp.end(); ++it)
  {
    std::string evt = "";
    ATTRIB_TO_STRING(it->second.front()->mask, evt);
    printf("Processing events for: [%s] (wd=%d, loc=%s/%s)\n", it->second.front()->name.c_str(), it->second.front()->wd, watching[it->second.front()->wd].location.c_str(), it->second.front()->name.c_str());
#define evts it->second
    for(size_t i = 0; i < evts.size(); i++)
    {
      std::string s = "";
      ATTRIB_TO_STRING(evts[i]->mask, s);
      printf("\tEvent: [%s], cookie: [%d], path: [%s]\n", s.c_str(), evts[i]->cookie, evts[i]->name.c_str());
    }
#undef evts
  }
}

void Watcher::FileWatcher::watch()
{
  int dbg = 0;
#define debug printf("REACHED %d (FILE: %s), (LINE: %d)\n", dbg, __FILE__, __LINE__)

  debug;
  char buffer[8192] __attribute__((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event* event;
  char* pointer;

  std::vector<Watcher::InotifyEvent> events;

  while(true)
  {
    ssize_t ret = read(setup.inotify_file_descriptor, buffer, 8192 * sizeof(char));

    if(ret < 0)
    {
      int err = errno;
      if(err == EAGAIN || err == EINTR || err == EWOULDBLOCK)
      {
        continue;
      }
      else
      {
        printf("[%s](%d) read() failed! Line %d\n", strerror(err), err, __LINE__);
      }
    }

    for(pointer = buffer; pointer < buffer + ret; pointer += sizeof(struct inotify_event) + event->len)
    {
      event = (const struct inotify_event*) pointer;
      Watcher::InotifyEvent evt;
      evt.cookie = event->cookie;
      evt.mask = event->mask;
      evt.wd = event->wd;
      evt.name = std::string(event->name, (size_t) event->len);
      events.push_back(std::move(evt));
    }
    prune(events, setup.watching);
    events.clear();
  }
}

#endif	// ifdef __linux__
