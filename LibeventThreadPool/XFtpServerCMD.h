#include "XTask.h"

class XFtpServerCMD : public XTask {

public:
    XFtpServerCMD();
    ~XFtpServerCMD();
    // 初始化任务
    bool Init() override;

};