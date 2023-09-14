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

  marks_[0] = '|';
  marks_[1] = '+';
  marks_[2] = '-';
  marks_[3] = 'x';
  marks_[4] = '=';
  markCount_ = sizeof(marks_) / sizeof(char);
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

  string line;
  vector<string> stack;
  string::size_type angleBracketPos;
  string::size_type lastPos;
  string::size_type prefixTrimLetter = 0;
  bool isNeedReformat = true;
  bool isInMultiComment = false;
  while (getline(ifs, line)) {
    line = trimSpace(line);

    // 空行
    if (line.empty()) {
      stack.push_back(line);
      continue;
    }

    // 多行注释开始
    if (line.substr(0, 2) == "/*") {
      isInMultiComment = true;
      stack.push_back(line);
      continue;
    }

    // 多行注释结束
    lastPos = line.length() - 1;
    if (line[lastPos - 1] == '*' && line[lastPos] == '/') {
      isInMultiComment = false;
      stack.push_back(line);
      continue;
    }

    // 多行注释
    if (isInMultiComment) {
      stack.push_back(line);
      continue;
    }

    if (prefixTrimLetter == 0) {
      if (line.substr(0, 2) == "> ") {
        // 如果堆栈的第 1 行以尖括号加空格（"> "）开头，说明不需要重新格式化
        isNeedReformat = false;
        break;
      } else {
        angleBracketPos = line.find(" > ");
        if (angleBracketPos == string::npos) {
          break;
        } else {
          prefixTrimLetter = angleBracketPos + 1;
        }
      }
    }

    if (!isNeedReformat) {
      return ;
    }

    // 重新格式化
    int idx = 0, markIdx = 0;
    string::const_iterator iter;
    line.erase(0, prefixTrimLetter);
    // line = line.substr(prefixTrimLetter);
    for (iter = line.cbegin(); iter != line.cend(); iter++, idx++) {
      if (*iter == '>' && *(iter + 1) == ' ') {
        break;
      }
      if (*iter == ' ') {
        continue;
      }


      markIdx = idx % markCount_;
      line[idx] = marks_[markIdx];
    }
    stack.push_back(line);
  }

  ifs.close();

  if (stack.empty()) {
    cout << "file[" << reformatFile_ << "] is empty" << endl;
    return;
  }

  vector<string>::const_iterator constIter;
  for (constIter = stack.cbegin(); constIter != stack.cend(); constIter++) {
    cout << *constIter << endl;
  }
}

void Frame::simplify() {
  char *dir = dirname((char *)__FILE__);

  string path(dir);
  path += '/';
  path += simplifyFile_;

  ifstream ifs(path, ios::in);
  if (!ifs.good()) {
    cerr << "file[" << path << "] is not exists" << endl;
    return ;
  }

  string line;
  vector<string> stack;
  while (getline(ifs, line)) {
    line = trimSpace(line);
    stack.push_back(line);
  }
  if ((stack.end() - 1)->find("this is the ending") == string::npos) {
    stack.emplace_back(string("| > *** this is the ending ***"));
  }

  ifs.close();

  vector<string>::size_type angleBracketPos;
  vector<string>::const_iterator iter;
  vector<string>::size_type lineIdx = 0;
  vector<string>::size_type len = stack.size();
  for (iter = stack.cbegin(); iter != stack.cend(); iter++, lineIdx++) {
    bool isMatched = false;

    line = *iter;
    angleBracketPos = line.find('>');
    if (angleBracketPos == string::npos) {
      continue;
    }

    string leftLine;
    vector<string>::size_type idxLeft, idxReplace, endReplaceIdx = 0;
    for (idxLeft = lineIdx + 1; idxLeft < len; idxLeft++) {
      bool isMark = false;
      char letter = stack[idxLeft][angleBracketPos];
      leftLine = stack[idxLeft];
      for (char mark : marks_) {
        if (leftLine[angleBracketPos - 1] == ' ' &&
            letter == mark &&
            leftLine[angleBracketPos + 1] == ' ') {
          isMark = true;
          break;
        }
      }

      if (isMark) {
        continue;
      } else if (letter == '>') {
        isMatched = true;
        break;
      } else {
        endReplaceIdx = idxLeft;
        break;
      }
    }

    if (isMatched) {
      continue;
    }

    if (endReplaceIdx == 0) {
      endReplaceIdx = len - 1;
    }

    for (idxReplace = lineIdx + 1; idxReplace < endReplaceIdx; idxReplace++) {
      stack[idxReplace].replace(angleBracketPos, 1, " ");
    }
  }

  int idxReplace;
  for (lineIdx = 0; lineIdx < len; lineIdx++) {
    idxReplace = 0;
    line = stack[lineIdx];

    string::const_iterator charIter;
    for (charIter = line.cbegin(); charIter != line.cend(); charIter++, idxReplace++) {
      if (*(charIter - 1) == ' ' && *charIter == '>' && *(charIter + 1) == ' ') {
        break;
      }

      for (char mark : marks_) {
        if (*charIter == mark && *charIter != '|') {
          line.replace(idxReplace, 1, "|");
        }
      }
    }
    if (lineIdx < len - 1) {
      cout << line << endl;
    } else {
      idxReplace = 1;
      for (charIter = line.cbegin() + idxReplace; charIter != line.cend(); charIter++, idxReplace++) {
        if (*charIter == '>') {
          break;
        } else {
          line.replace(idxReplace, 1, " ");
        }
      }
      cout << line << endl;
    }
  }
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

  int level = 0;
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