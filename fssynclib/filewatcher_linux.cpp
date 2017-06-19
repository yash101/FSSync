/*

Author: Devyash Lodha
File: filewatcher_linux.cpp
Description:
    Linux implementation of file system watching. Uses inotify for speed and efficiency.
    If a file or directory is added or changed, a notification will be sent. Otherwise,
    would have to poll to detect changes
Other Information:
    This file is written for the Linux operating system; it uses inotify for most of its core.

*/

#ifdef __linux__

/*
 * Includes required to make this source file's code tick...
 *
 * Project Includes
 *   "filewatcher.h" - contains the Watcher::FileWatcher class as well as global API for file
 * 	   watching.
 *   "coreutils.h" - core utilities
 *
 * System API Includes
 *   <sys/inotify.h> - contains needed structures and functions to use inotify for high-efficiency
 * 	   file watching
 *   <sys/stat.h> - helps getting file and directory information
 *   <unistd.h> - many useful POSIX and Linux structures, functions and definitions, etc.
 *   <fcndl.h> - file descriptor modification functions
 *   <dirent.h> - browsing through directories and accessing BASIC information
 *
 * C & C++ Standard Library Includes
 *   <string.h> - c-string manipulation functions
 * 	 <stdio.h> - printf, etc.
 *   <vector> - resizeable arrays
 *   <map> - map data structure; allows searching for an object by a key
 */

#include "filewatcher.h"
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <string.h>
#include <stdio.h>
#include <vector>
#include <map>

#include "coreutils.h"

/*
 * Random handy macros
 * MAKE_BLOCK: makes a file descriptor blocking
 * MAKE_UNBLOCK: makes a file descriptor unblocking
 * WATCH_ATTR: the attributes being used in the file watching
 * ATTRIB_TO_STRING: prints in a nice way, which flags are set in a mask; useful for debugging
 */

#define MAKE_BLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK)
#define MAKE_UNBLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

#define WATCH_ATTR IN_ATTRIB | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_MOVE_SELF | IN_MOVE | IN_UNMOUNT | IN_EXCL_UNLINK
#define MAX_ADD_DIRECTORY_RECURSE_LENGTH 20

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

/*
 * This adds the initial folder, 'this->folder' and its contents, recursively
 */

int Watcher::FileWatcher::begin_watching()
{
  //If the file/folder we want to watch ends with '/' or '\', then pop that off; unnecessary and will cause problems
  if(folder.back() == '/' || folder.back() == '\\')
    if(!(folder == "/" || folder == "\\"))
      folder.pop_back();

  //Holds the watch information
  Watcher::WatchedItem item;
  item.wd = inotify_add_watch(setup.inotify_file_descriptor, folder.c_str(), WATCH_ATTR);
  item.location = folder;

  //Check for errors
  if(item.wd < 0)
  {
    int err = errno;
    ERROR("Failed to add root directory, '%s'", strerror(err), err, folder.c_str());
    return -1;
  }
  INFO("Added file or directory, '%s'", folder.c_str());

  //Add this to the map of watches
  setup.watching[item.wd] = item;

  add_directory_recurse_length = 1;
  //Add the contents of the directory
  add_directory(folder.c_str());
  add_directory_recurse_length--;
  return 0;
}

/*
 * Adds a directory to the watch map recursively
 */

int Watcher::FileWatcher::add_directory(const char* directory)
{
#ifdef MAX_ADD_DIRECTORY_RECURSE_LENGTH
  if(add_directory_recurse_length >= MAX_ADD_DIRECTORY_RECURSE_LENGTH)
  {
    ERROR("Too many recursive directories; current depth relative to root: %d", add_directory_recurse_length);
    return -1;
  }
#endif
  //Holds directory information
  DIR* d;
  struct dirent* dir;

  //Open the directory
  d = opendir(directory);

  //Check if the directory was opened successfully
  if(d)
  {
    //Get each file/directory/etc. from the directory; store it into struct dirent* dir
    while((dir = readdir(d)) != NULL)
    {
      //Full path to file/directory
      std::string fpath = directory + std::string("/") + dir->d_name;

      //Get the file/directory/etc. information
      struct stat st;
      lstat(fpath.c_str(), &st);

      //Check if struct stat st points to a directory and its name is not '.' or '..'
      if((S_ISDIR(st.st_mode) != 0 || S_ISLNK(st.st_mode) != 0) && strcmp(dir->d_name, ".") && strcmp(dir->d_name, ".."))
      {
        //Check if we have a link (and whether to allow link following)
        if(!follow_links && S_ISLNK(st.st_mode) != 0)
        {
          INFO("Skipping link (inode: %lu; name: %s; path: %s", dir->d_ino, dir->d_name, fpath.c_str());
          continue;
        }
        INFO("Adding (inode: %lu; name: %s; path: %s)", dir->d_ino, dir->d_name, fpath.c_str());

        //Stores the watch information
        Watcher::WatchedItem item;
        item.location = fpath;
        item.wd = inotify_add_watch(setup.inotify_file_descriptor, fpath.c_str(), WATCH_ATTR);

        //Check for errors
        if(item.wd < 0)
        {
          int err = errno;
          ERROR("Failed to create inotify watch: errno=%d (%s)", err, strerror(err));
          continue;
        }

        //Add the watched item to the map of watched item; the key is the watch descriptor (wd)
        setup.watching[item.wd] = item;

        INFO("Successfully added to inotify event queue: %s", fpath.c_str());

        //Recurse into the directory, pointed by fpath
        add_directory_recurse_length++;
        add_directory(fpath.c_str());
        add_directory_recurse_length--;

        INFO("Back to directory, '%s'", directory);

      }
      else
      {
        INFO("Skipped (inode: %lu; name: %s; path: %s) because not a directory or a link", dir->d_ino, dir->d_name, fpath.c_str());
      }
    }

    //Free the resources
    closedir(d);
  }
  else
  {
    ERROR("Could not open directory, %s", directory);
  }

  return 0;
}

void Watcher::FileWatcher::process_events(std::vector<Watcher::InotifyEvent>& events)
{
  INFO("=============PROCESSING EVENTS==============");

  Watcher::Event event;

  if(events.size() == 0)

  //Events with only one part
  {
    FATAL("Received 0 events; something's weird!");
  }

  else if(events.size() == 1)

  {
    INFO("Received 1 event");

    //If a file or directory was created
    if(events.front().mask & IN_CREATE)
    {
      //Add the directory
      add_directory(std::string(setup.watching[events.front().wd].location + "/" + events.front().name).c_str());
    }
  }

  else if(events.size() == 2)

  {
    INFO("Recieved 2 events");
  }

  else

  {
    ERROR("Received %d events!", events.size());
    printf("Received %lu events!\n", events.size());
    for(size_t i = 0; i < events.size(); i++)
    {
      std::string evt;
      ATTRIB_TO_STRING(events[i].mask, evt);
      printf("\tEvent: cookie=%d, mask=%d, wd=%d, events=%s, name=%s\n", events[i].cookie, events[i].mask, events[i].wd, evt.c_str(), events[i].name.c_str());
    }
  }
}

static void prune(std::vector<Watcher::InotifyEvent>& events, decltype(Watcher::WatcherSetup::watching)& watching, int inotify_fd)
{
  INFO("Recieved %lu events", events.size());
//  INFO("PRUNING");

//Create map of pointers to each event
//  std::map<decltype(Watcher::InotifyEvent::cookie), std::vector<Watcher::InotifyEvent*> > mp;
//  for(size_t i = 0; i < events.size(); i++)
//  {
//    mp[events[i].cookie].push_back(&events[i]);
//  }

//  //Iterate through the map; the map should sort everything by its watch descriptor (wd)
//  for(std::map<decltype(Watcher::InotifyEvent::cookie), std::vector<Watcher::InotifyEvent*> >::const_iterator it = mp.begin(); it != mp.end(); ++it)
//  {

//    //ATTRIB_TO_STRING works on this; loads a string (const char*)
//    std::string evt = "";
//    ATTRIB_TO_STRING(it->second.front()->mask, evt);

//    //Debug
//    INFO("Processing events for: [%s] (wd=%d, loc=%s/%s)", it->second.front()->name.c_str(), it->second.front()->wd, watching[it->second.front()->wd].location.c_str(), it->second.front()->name.c_str());

//    //Save my fingers a bit please :)
//#define evts it->second

//    if(evts.size() == 0)
//    {
//      continue;
//    }

//    //If only one event was received
//    else if(evts.size() == 1)
//    {
//      INFO("Recieved one event");
//      std::string s = "";
//      ATTRIB_TO_STRING(evts.front()->mask, s);
//      INFO("\tEvent: %s for name: '%s', cookie: %d (%x), mask: %d, wd: %d", s.c_str(), evts.front()->name.c_str(), evts.front()->cookie, evts.front()->cookie, evts.front()->mask, evts.front()->wd);

//      //File or directory (or etc.) was created
//      //If it is a directory, add it to the watch list
//      if(evts.front()->mask & IN_CREATE)
//      {
//        //Get the path to the new file
//        //	Use the path to the folder for which evts.front()->wd is defined
//        std::string path = watching[evts.front()->wd].location + std::string("/") + evts.front()->name;

//        //Check if file or directory
//        struct stat st;
//        stat(path.c_str(), &st);

//        Watcher::Event e;
//        e.path_a = path;

//        //Check if file or directory
//        if(S_ISDIR(st.st_mode) != 0)
//        {
//          INFO("Adding directory, %s; stat mode: %d", path.c_str(), st.st_mode);
//          Watcher::WatchedItem item;
//          item.location = path;
//          item.wd = inotify_add_watch(inotify_fd, path.c_str(), WATCH_ATTR);

//          if(item.wd < 0)
//          {
//            int err = errno;
//            ERROR("Failed adding directory, %s, because of error %d (%s)", path.c_str(), err, strerror(err));
//          }

//          e.action = WATCHER_CREATED_DIRECTORY;
//        }
//        else
//        {
//          e.action = WATCHER_CREATED_NON_DIRECTORY;
//        }
//      }


//      else if(evts.front()->mask & IN_ATTRIB)
//      {
//      }


//    }
//    else if(evts.size() == 2)
//    {
//      INFO("Recieved two events");
//      for(size_t i = 0; i < evts.size(); i++)
//      {
//        std::string s = "";
//        ATTRIB_TO_STRING(evts[i]->mask, s);
//        INFO("\tEvent: %s for name: '%s', cookie: %d (%x), mask: %d, wd: %d", s, evts[i]->name, evts[i]->cookie, evts[i]->cookie, evts[i]->mask, evts[i]->wd);
//      }
//    }
//    else
//    {
//      ERROR("Recieved more than two events. Can't decide what to do.");
//      for(size_t i = 0; i < evts.size(); i++)
//      {
//        std::string s = "";
//        ATTRIB_TO_STRING(evts[i]->mask, s);
//        ERROR("\tEvent: %s for name: '%s', cookie: %d (%x), mask: %d, wd: %d", s, evts[i]->name, evts[i]->cookie, evts[i]->cookie, evts[i]->mask, evts[i]->wd);
//      }
//    }

//#undef evts
//  }
}

void Watcher::FileWatcher::watch()
{
  int dbg = 0;
#define debug INFO("DEBUG: REACHED %d", dbg++)

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
//    prune(events, setup.watching, setup.inotify_file_descriptor);
    process_events(events);
    events.clear();
  }
}

int Watcher::FileWatcher::initialize()
{
  setup.inotify_file_descriptor = inotify_init();
  MAKE_BLOCK(setup.inotify_file_descriptor);
  return 0;
}
#endif	// ifdef __linux__
