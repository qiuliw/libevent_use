#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>  // for inet_pton()
#include <iostream>

int server_port = 20010;
char *server_ip = "127.0.0.1";

int main() {
    // 1. 创建 socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    // 2. 设置服务器地址
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);

    // 使用 inet_pton() 将 IP 地址字符串转换为二进制格式
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        exit(1);
    }

    // 3. 连接服务器
    int ret = connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));
    if (ret < 0) {
        perror("connect");
        close(sock);
        exit(1);
    }
    std::cout << "Connected to server " << server_ip << ":" << server_port << std::endl;

    char buf[128];

    // 4. 循环发送和接收数据
    while (true) {
        printf("请输入数据（输入 'quit' 退出）：");
        fgets(buf, sizeof(buf), stdin);

        // 检查是否输入 'quit'
        if (strncmp(buf, "quit", 4) == 0) {
            std::cout << "Exiting..." << std::endl;
            break;
        }

        // 发送数据
        int bytes_sent = write(sock, buf, strlen(buf));
        if (bytes_sent < 0) {
            perror("write");
            break;
        }
    }

    // 5. 关闭 socket
    close(sock);
    return 0;
}