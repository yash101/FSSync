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

#include "smartptr.h"

int Watcher::FileWatcher::initialize()
{
  setup.inotify_file_descriptor = inotify_init();
  fcntl(setup.inotify_file_descriptor, F_SETFL, fcntl(setup.inotify_file_descriptor, F_GETFL) | O_NONBLOCK);
  return 0;
}

int Watcher::FileWatcher::begin_watching()
{
  if(folder.back() == '/' || folder.back() == '\\')
    folder.pop_back();
  int wd = inotify_add_watch(setup.inotify_file_descriptor, folder.c_str(), IN_ALL_EVENTS);

#ifdef DEBUG
      printf("Adding directory, '%s'\n", folder.c_str());
#endif

  if(wd < 0)
  {
    int err = errno;
    printf("[%s](%d) Failed to add root directory, '%s'\n", strerror(err), err, folder.c_str());
    return -1;
  }

  add_directory(folder.c_str());
  return 0;
}

int Watcher::FileWatcher::add_directory(const char* directory)
{
  DIR* d;
  struct dirent* dir;

  d = opendir(directory);

  if(d)
  {
    while((dir = readdir(d)) != NULL)
    {
#ifdef DEBUG
      printf("Adding directory (INODE: %lu), '%s'\n", dir->d_ino, dir->d_name);
#endif

      int wd = inotify_add_watch(setup.inotify_file_descriptor, dir->d_name, IN_ALL_EVENTS);

      if(wd < 0)
      {
        int err = errno;
        printf("[%s](%d) Failed to add directory (INODE: %lu), '%s'\n", strerror(err), err, dir->d_ino, dir->d_name);
      }

      struct stat st;
      stat(dir->d_name, &st);
      if((strcmp(dir->d_name, "."))
         && (strcmp(dir->d_name, ".."))
         && S_ISDIR(st.st_mode) != 0
      )
      {
        printf("Adding directory, '%s'\n", dir->d_name);
        std::string dirname = directory;
        dirname += '/';
        dirname += dir->d_name;
        add_directory(dirname.c_str());
      }
    }

    closedir(d);
  }


  return 0;
}

void Watcher::FileWatcher::watch()
{
  printf("Hi!\n");
  watch_();
}

#define MAKE_BLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK)
#define MAKE_UNBLOCK(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

void process(std::vector<struct inotify_event>& events)
{
  for(size_t i = 0; i < events.size(); i++)
  {
    printf("EVENT: name: %s; len: %d; cookie: %d; mask: %d", events[i].name, events[i].len, events[i].cookie, events[i].mask);
  }
}

void Watcher::FileWatcher::watch_()
{
  int d = 0;
#define debug printf("Reached line %d; d=%d\n", __LINE__, d++)

  std::vector<struct inotify_event> events;
  events.reserve(32);
  struct inotify_event evt;
  bool simFlag = false;

  MAKE_BLOCK(setup.inotify_file_descriptor);

  while(true)
  {
    ssize_t ret = read(setup.inotify_file_descriptor, &evt, sizeof(decltype(evt)));
    if(ret < 0)
    {
      int err = errno;
      if(err == EAGAIN)
      {
        simFlag = false;
        MAKE_BLOCK(setup.inotify_file_descriptor);
        process(events);
        events.clear();
        continue;
      }
      else break;
    }
    else
    {
      MAKE_UNBLOCK(setup.inotify_file_descriptor);
      simFlag = true;
      events.push_back(evt);
    }
  }









/*
  fd_set sfd;
  FD_ZERO(&sfd);
  FD_SET(setup.inotify_file_descriptor, &sfd);

  while(true)
  {
    fd_set sfd_n = sfd;
    d = 0;
    debug;
//    if(!simFlag)	//We will block just in case
//    {
//      events.clear();
//      debug;
//      ssize_t ret = select(setup.inotify_file_descriptor + 1, &sfd_n, NULL, NULL, NULL);
//      if(ret < 0)
//      {
//        int err = errno;
//        printf("SELECT() ERROR: '%s'(%d): %d\n", strerror(err), err, __LINE__);
//      }
//      printf("SELECT() RETURNED: '%zd'\n", ret);
//      debug;
//    }
    debug;
    ssize_t len = read(setup.inotify_file_descriptor, &evt, sizeof(struct inotify_event));
    debug;
    simFlag = true;
    if(len < 0)
    {
      int err = errno;
      if(err == EAGAIN)
      {
        simFlag = false;
        //process(events)
      }
    }
    else if(len == 0)	//BIG TIME ERROR!
    {
      printf("[FATAL] read returned 0\n");
    }
    else
    {
      events.push_back(evt);
    }
  }*/

/*
  char buffer[128 * sizeof(struct inotify_event)]
      __attribute__((aligned(__alignof__(struct inotify_event))));
  struct inotify_event* ptr;
  int d = 0;
      printf("DEBUG: [%3d]", d++);
  while(true)
  {
    ssize_t len = read(setup.inotify_file_descriptor, buffer, sizeof(buffer));
    if(len < 0)
    {
      int err = errno;
      printf("[%s](%d) Error reading from the inotify file descriptor", strerror(err), err);
      if(err == EAGAIN) continue;
    }

    while(ptr < (struct inotify_event*) buffer + len)
    {
      if(ptr->mask & IN_OPEN)
        printf("IN_OPEN: ");
      else if(ptr->mask & IN_CLOSE_NOWRITE)
        printf("IN_CLOSE_NOWRITE: ");
      else if(ptr->mask & IN_CLOSE_WRITE)
        printf("IN_CLOSE_WRITE: ");
      else
        printf("OTHER_ACTION");

      ptr->name[ptr->len - 1] = '\0';
      printf("%s", ptr->name);


      ptr++;
    }
  }*/
}

#endif	// ifdef __linux__
