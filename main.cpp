#include <iostream>
#include <algorithm>
#include <getopt.h>
#include "frame.h"

using namespace std;

void usage(char *argv[]);

int main(int argc, char *argv[]) {
  int opt;
  long skip = 0;
  string func("format");
  while ((opt = getopt(argc, argv, "hf:s:")) != -1) {
    switch(opt) {
      case 'f':
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
      case 's':
        if (optarg) {
          skip = strtol(optarg, nullptr, 10);
        }
        break;
      default:
        usage(argv);
        break;
    }
  }

  Frame frame(skip);
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

  exit(0);
}