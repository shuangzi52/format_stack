/**
 * Created by 操盛春 on 2023/9/13.
 */

#ifndef FORMAT_STACK_FRAME_H
#define FORMAT_STACK_FRAME_H

#include <vector>

using namespace std;

class Frame {
public:
    explicit Frame(long skip);
    void format();
    void reformat();
    void simplify();

private:
    char marks_[5];
    int markCount_;
    constexpr static char *formatFile_ = (char *)"format.txt";
    constexpr static char *reformatFile_ = (char *)"reformat.txt";
    constexpr static char *simplifyFile_ = (char *)"simplify.txt";
    constexpr static char *configFile_ = (char *)"config.ini";


    long skip_;
    vector<string> projectRoots_;
    vector<string> namespaces_;

private:
    vector<string> readFromFile();
    void show(vector<string> &frames);
    string trimSpace(string content);
    string formatPath(string content);
    string formatInvoke(string content);
    void initConfigs();
};


#endif //FORMAT_STACK_FRAME_H
