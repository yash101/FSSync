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
#include <unistd.h>

#include <string.h>
#include <dirent.h>
#include <stdio.h>

int Watcher::FileWatcher::initialize()
{
  setup.inotify_file_descriptor = inotify_init();
  return 0;
}

int Watcher::FileWatcher::begin_watching()
{
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
      if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..") && S_ISDIR(st.st_mode))
      {
        add_directory(dir->d_name);
      }
    }

    closedir(d);
  }

  return 0;
}

void Watcher::FileWatcher::watch()
{
  printf("Hi!");
  watch_();
}

void Watcher::FileWatcher::watch_()
{
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
  }
}

#endif	// ifdef __linux__
