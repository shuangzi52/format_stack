/**
 * Created by 操盛春 on 2023/9/13.
 */

#include <iostream>
#include <libgen.h>
#include <fstream>
#include <cctype>
#include "frame.h"

Frame::Frame(long skip) {
  if (skip >= 0) {
    skip_ = skip;
  }
}

void Frame::initConfigs() {
  char *dir = dirname((char *)__FILE__);

  string path(dir);
  path += '/';
  path += configFile_;

  ifstream ifs(path, ios::in);
  if (!ifs.good()) {
    cerr << "file[" << path << "] is not exists" << endl;
    return ;
  }

  bool isItem;
  string configItem;
  string line;
  string::size_type endPos;
  while (getline(ifs, line)) {
    line = trimSpace(line);
    if (line.empty()) { // 空行，忽略
      continue;
    }
    if (line[0] == ';') { // 注释，忽略
      continue;
    }

    isItem = (*line.begin() == '[' && *(line.end() - 1) == ']');
    if (isItem) {
      if (line == "[project_root]" || line == "[namespace]") {
        configItem = line;
      } else {
        configItem = "";
      }
    } else if (configItem == "[project_root]") {
      replace(line.begin(), line.end(), '\\', '/');
      if (*(line.end() - 1) != '/') {
        line += '/';
      }

      projectRoots_.push_back(line);
    } else if (configItem == "[namespace]") {
      endPos = line.length() - 1;
      if (line[endPos] != ':') {
        line += ':';
      }
      if (line[endPos - 1] != ':') {
        line += ':';
      }

      namespaces_.push_back(line);
    }
  }

  ifs.close();

  // 对 namespaces 进行倒序排序，让子命名空间先替换
  sort(namespaces_.rbegin(), namespaces_.rend());

  /* vector<string>::const_iterator iter;
  cout << "****** project roots ******" << endl;
  for (iter = projectRoots_.begin(); iter != projectRoots_.end(); iter++) {
    cout << *iter << endl;
  }
  cout << endl;

  cout << "****** namespaces ******" << endl;
  for (iter = namespaces_.begin(); iter != namespaces_.end(); iter++) {
    cout << *iter << endl;
  }
  cout << endl; */
}

vector<string> Frame::readFromFile() {
  initConfigs();

  vector<string> stackReverse;
  vector<string> stack;

  char *dir = dirname((char *)__FILE__);

  string path(dir);
  path += '/';
  path += formatFile_;

  ifstream ifs(path, ios::in);
  if (!ifs.good()) {
    cerr << "file[" << path << "] is not exists" << endl;
    return stack;
  }

  string line;
  int skipped = 0;
  while (getline(ifs, line)) {
    stackReverse.push_back(line);
  }

  ifs.close();

  vector<string>::const_reverse_iterator it;
  for (it = stackReverse.crbegin(); it != stackReverse.crend(); it++) {
    if (skipped < skip_) {
      skipped++;
      continue;
    }

    line = *it;
    line = trimSpace(line);
    line = formatPath(line);
    line = formatInvoke(line);
    stack.push_back(line);
  }

  return stack;
}

void Frame::format() {
  vector<string> stack = readFromFile();
  if (stack.empty()) {
    cout << "file [" << formatFile_ << "] is empty" << endl;
  } else {
    show(stack);
  }
}

void Frame::reformat() {
  char *dir = dirname((char *)__FILE__);

  string path(dir);
  path += '/';
  path += reformatFile_;

  ifstream ifs(path, ios::in);
  if (!ifs.good()) {
    cerr << "file[" << path << "] is not exists" << endl;
    return ;
  }

  int level = 1;
  string prefix;
  string line;
  string::size_type startPos, levelIdx, markIdx;
  while (getline(ifs, line)) {
    line = trimSpace(line);
    startPos = line.find_first_of('>');

    prefix = "";
    for (levelIdx = 0; levelIdx < level; levelIdx++) {
      markIdx = levelIdx % markCount_;
      cout << marks_[markIdx] << " ";
    }
    cout << line.substr(startPos) << endl;

    level++;
  }

  ifs.close();
}

string Frame::trimSpace(string content) {
  if (content.empty()) {
    return content;
  }

  string::size_type len = content.length();
  string::size_type leftSpace = 0;
  string::size_type rightSpace = 0;
  for (string::size_type i = 0; i < len; i++) {
    if (isspace(content.at(i))) {
      leftSpace++;
    } else {
      break;
    }
  }

  for (string::size_type i = len - 1; i >= 0; i--) {
    if (isspace(content.at(i))) {
      rightSpace++;
    } else {
      break;
    }
  }

  return content.substr(leftSpace, len - leftSpace - rightSpace);
}

/**
 * 去掉堆栈路径中的括号
 * 例如：
 *   parse_sql(THD*, Parser_state*, Object_creation_ctx*) (/opt/data/workspace_c/mysql8/sql/sql_parse.cc:7085)
 * 去掉路径两边的括号之后得到：
 *   parse_sql(THD*, Parser_state*, Object_creation_ctx*) /opt/data/workspace_c/mysql8/sql/sql_parse.cc:7085
 * @param content
 * @return
 */
string Frame::formatPath(string content) {
  if (content.empty()) {
    return content;
  }

  vector<string>::const_iterator it;
  string::size_type pos;
  for (it = projectRoots_.begin(); it != projectRoots_.end(); it++) {
    pos = content.find(*it);
    if (pos != string::npos) {
      content = content.replace(pos, it->length(), "");
    }
  }

  string::size_type endPos = content.size() - 1;
  if (content.at(endPos) != ')') {
    return content;
  }

  char letter;
  string::size_type midPos = -1;
  for (string::size_type i = endPos; i >= 0; i--) {
    letter = content.at(i);
    if (isspace(letter)) {
      break;
    } else if (letter == '(') {
      midPos = i;
    }
  }

  if (midPos == -1) {
    return content;
  }

  string newContent;
  newContent.append(content.substr(0, midPos));
  newContent.append(content.substr(midPos + 1, endPos - midPos - 1));

  return newContent;
}

string Frame::formatInvoke(string content) {
  if (content.empty()) {
    return content;
  }

  char letter;
  string newContent;

  bool isInAngleBracket = false;
  bool isInRoundBracket = false;

  vector<char> angleBrackets;
  vector<char>roundBrackets;

  string::size_type angleStart = string::npos;
  string::size_type angleEnd = string::npos;
  string::size_type roundStart = string::npos;
  string::size_type roundEnd = string::npos;

  string::size_type len = content.length();
  bool isError = false;
  for (string::size_type i = 0; i < len; i++) {
    letter = content.at(i);

    // 把模板方法 < 和 > 之间的所有内容替换为 ...
    if (isInAngleBracket) {
      if (letter == '<') {
        angleBrackets.push_back(letter);
      } else if (letter == '>') {
        if (*(angleBrackets.end() - 1) == '<') {
          angleBrackets.pop_back();
        } else {
          isError = true;
          break;
        }

        if (angleBrackets.empty()) {
          angleEnd = i;
          isInAngleBracket = false;

          newContent.append(content.substr(0, angleStart + 1));
          newContent.append("...>");
        }
      }

      continue;
    }

    if (isInRoundBracket) {
      if (letter == ')') {
        roundEnd = i;
      }

      continue;
    }

    // isInAngleBracket、isInParentheses 都为 false
    if (letter == '<') {
      angleStart = i;
      angleBrackets.push_back(letter);
      isInAngleBracket = true;
    } else if (letter == '(') {
      roundStart = i;
      roundBrackets.push_back(letter);
      isInRoundBracket = true;
    }
  }

  // 把函数、方法中 ( 和 )之间的内容替换为空
  if (angleEnd == string::npos) {
    newContent.append(content.substr(0, roundStart + 1));
    newContent.append(content.substr(roundEnd, len - roundEnd));
  } else {
    newContent.append(content.substr(angleEnd + 1, roundStart - angleEnd));
    newContent.append(content.substr(roundEnd, len - roundEnd));
  }

  if (isError) {
    cerr << content << ", error" << endl;
    return content;
  }

  // 循环把 namespaces 中指定的表空间名替换为空
  vector<string>::const_iterator it;
  string::size_type pos;
  for (it = namespaces_.begin(); it != namespaces_.end(); it++) {
    pos = newContent.find(*it);
    if (pos != string::npos) {
      newContent = newContent.replace(pos, it->length(), "");
    }
  }

  return newContent;
}

void Frame::show(vector<string> &frames) {
  if (frames.empty()) {
    return ;
  }

  int level = 1;
  int levelIdx, markIdx;
  vector<string>::const_iterator it;
  for (it = frames.cbegin(); it != frames.cend(); it++) {
    for (levelIdx = 0; levelIdx < level; levelIdx ++) {
      markIdx = levelIdx % markCount_;
      cout << marks_[markIdx] << " ";
    }

    level++;

    cout << "> ";
    cout << *it << endl;
  }
}