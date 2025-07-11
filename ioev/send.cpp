#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
int main(){
    const char* fifo_path = "fifo.tmp";

    // 1. 尝试直接打开 FIFO（如果已存在）
    int fd = open(fifo_path, O_WRONLY); // 如果另一端没有创建，则打开会阻塞
    if (fd != -1) {
        // FIFO 已存在，直接打开成功
        std::cout << "FIFO already exists, opened successfully." << std::endl;
    } else {
        // 2. 如果打开失败，检查是否是因为 FIFO 不存在（errno == ENOENT）
        if (errno == ENOENT) {
            // FIFO 不存在，尝试创建它
            int ret = mkfifo(fifo_path, 0700);
            if (ret == -1) {
                perror("mkfifo");
                exit(1);
            }
            std::cout << "FIFO created successfully." << std::endl;

            // 3. 创建成功后，再次尝试打开
            fd = open(fifo_path, O_WRONLY);
            if (fd == -1) {
                perror("open");
                exit(1);
            }
            std::cout << "FIFO opened after creation." << std::endl;
        } else {
            // 其他错误（如权限不足等）
            perror("open");
            exit(1);
        }
    }
    while(1){ 
        char buf[32] = {0};
        printf("请输入数据：");
        fgets(buf, sizeof(buf), stdin);
        // 对端如果关闭，默认触发SIGPIPE信号，导致程序退出
        write(fd, buf, sizeof(buf));
    }
}