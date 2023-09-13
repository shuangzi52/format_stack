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
    static void reformat();

private:
    constexpr static char marks_[]  = {'|', '+', '-', 'x', '='};
    constexpr static int markCount_ = sizeof(marks_) / sizeof(char);
    constexpr static char *formatFile_ = (char *)"format.txt";
    constexpr static char *reformatFile_ = (char *)"reformat.txt";
    constexpr static char *configFile_ = (char *)"config.ini";


    long skip_;
    vector<string> projectRoots_;
    vector<string> namespaces_;

private:
    vector<string> readFromFile();
    static void show(vector<string> &frames);
    static string trimSpace(string content);
    string formatPath(string content);
    string formatInvoke(string content);
    void initConfigs();
};


#endif //FORMAT_STACK_FRAME_H
