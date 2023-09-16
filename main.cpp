#include <iostream>
#include <algorithm>
#include <getopt.h>
#include "frame.h"

using namespace std;

void usage(char *argv[]);

int main(int argc, char *argv[]) {
  int opt;
  long skip = 0;
  bool isTrimTplParams = true; // 默认需要删除堆栈中的模板参数
  string func("format");
  while ((opt = getopt(argc, argv, "hf:s:t:")) != -1) {
    switch(opt) {
      case 'f': // 功能
        if (optarg) {
          if (strcmp(optarg, "format") == 0 ||
              strcmp(optarg, "reformat") == 0 ||
              strcmp(optarg, "simplify") == 0) {
            func = optarg;
          } else {
            usage(argv);
          }
        }
        break;
      case 's': // format 功能的 skip 变量值
        if (optarg) {
          skip = strtol(optarg, nullptr, 10);
        }
        break;
      case 't': // 模板参数
        if (optarg) {
          if (strcmp(optarg, "true") == 0) {
            isTrimTplParams = true;
          } else if (strcmp(optarg, "false") == 0) {
            isTrimTplParams = false;
          } else {
            usage(argv);
          }
        }
        break;
      default:
        usage(argv);
        break;
    }
  }

  Frame frame(skip, isTrimTplParams);
  if (func == "reformat") {
    frame.reformat();
  } else if (func == "simplify") {
    frame.simplify();
  } else {
    frame.format();
  }

  return 0;
}

void usage(char *argv[]) {
  cout << argv[0] << endl;
  cout << "  -h Display the help and exit." << endl;
  cout << "  -f The action to do(format, reformat), default: format." << endl;
  cout << "  -s How many lines to skip, must gte 0, default: 0." << endl;
  cout << "  -t Remove template parameters or not(true, false), default: false." << endl;

  exit(0);
}