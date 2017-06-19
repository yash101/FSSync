#include <iostream>
#include <fssynclib.h>
#include <filewatcher.h>

using namespace std;

int main(int argc, char *argv[])
{
  Watcher::FileWatcher f("/home/yash", true);
  f.watch();
  cout << "Hello World!" << endl;
  return 0;
}
