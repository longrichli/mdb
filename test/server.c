#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 5
#define CONCURRENT_MAX 4    // 最大链接数
#define SERVER_PORT 8181
#define BUFFER_SIZE 1024
#define QUIT_CMD ".quit"

int client_fds[CONCURRENT_MAX];
struct kevent events[6];    //CONCURRENT_MAX +2

int main(int argc, char * argv[])
{
    char input_msg[BUFFER_SIZE];
    char recv_msg[BUFFER_SIZE];

    //本地地址
    struct sockaddr_in server_addr;
    server_addr.sin_len = sizeof(struct sockaddr_in);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(server_addr.sin_zero),8);

//创建socket
    int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock_fd == -1) {
        perror("socket error");
        return 1;
    }
    //绑定socket
    int bind_result = bind(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_result == -1) {
        perror("bind error");
        return 1;
    }
    //listen
    if (listen(server_sock_fd, BACKLOG) == -1) {
        perror("listen error");
        return 1;
    }

    struct timespec timeout = {10,0};
    // 返回新的kqueue描述符
    int kq = kqueue();
    if (kq == -1) {
        perror("创建kqueue出错!\n");
        exit(1);
    }

    printf("server_sock_fd: %d \n", server_sock_fd);
    struct kevent event_change;
    // 设置event_change 结构体 指定为EVFILT_READ读的过滤器, 并把本事件加入该过滤器EV_ADD
    EV_SET(&event_change, STDIN_FILENO, EVFILT_READ, EV_ADD, 0, 0, NULL);
    // 以设置好的kevent结构体 作为过滤器请求调用kevent
    kevent(kq, &event_change, 1, NULL, 0, NULL);
    EV_SET(&event_change, server_sock_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(kq, &event_change, 1, NULL, 0, NULL);

    while(1) {
        int ret = kevent(kq, NULL, 0, events, 10, &timeout);
        if (ret < 0) {
            printf("kevent 出错!\n");
            continue;
        }else if(ret == 0){
            printf("kenvent 超时!\n");
            continue;
        }else{
            //ret > 0 返回事件放在events中
            for (int i = 0; i < ret; i++) {
                struct kevent current_event = events[i];
                // kevent中的ident就是文件描述符
                if (current_event.ident == STDIN_FILENO) {  // 如果是标准输入
                    bzero(input_msg, BUFFER_SIZE);
                    fgets(input_msg, BUFFER_SIZE, stdin);

                    // 判断是否为退出
                    if (strcmp(input_msg, QUIT_CMD) == 0) {
                        exit(0);
                    }
                    // 将服务端的输入还是广播到每个客户端了
                    for (int i = 0; i < CONCURRENT_MAX; i++) {
                        if (client_fds[i] != 0) {
                            send(client_fds[i], input_msg, BUFFER_SIZE, 0);
                        }
                    }
                } else if (current_event.ident == server_sock_fd) { //有新的链接
                    printf("current_event ident:%d \n", (int)current_event.ident);
                    struct sockaddr_in client_address;
                    socklen_t address_len;
                    int client_socket_fd = accept(server_sock_fd, (struct sockaddr *)&client_address, &address_len);
                    printf("client_socket_fd :%d \n", (int)client_socket_fd);
                    if (client_socket_fd > 0) {
                        int index = -1;
                        for(int i = 0; i < CONCURRENT_MAX; i++) {
                            printf("i:%d  client_fd:%d\n",i , client_fds[i]);
                            if (client_fds[i] == 0) {
                                index = i;
                                client_fds[i] = client_socket_fd;
                                break;
                            }
                        }
                        printf("index:%d \n", index);

                        if (index >= 0) {
                            EV_SET(&event_change, client_socket_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                            kevent(kq, &event_change, 1, NULL, 0, NULL);
                            printf("new client fd: %d  %s: %d\n",client_socket_fd, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                        } else {
                            bzero(input_msg, BUFFER_SIZE);
                            strcpy(input_msg, "server has max connection, can not join!\n");
                            send(client_socket_fd, input_msg, BUFFER_SIZE, 0);
                            printf("%s", input_msg);
                        }
                    }
                } else {
                    // 处理某个客户端 发送过来的消息
                    bzero(recv_msg, BUFFER_SIZE);
                    long byte_num = recv((int)current_event.ident, recv_msg, BUFFER_SIZE, 0);
                    if (byte_num > 0) {
                        if (byte_num > BUFFER_SIZE) {
                            byte_num = BUFFER_SIZE;
                        }
                        recv_msg[byte_num] = '\0';
                        printf("client fd:%d msg:%s\n",(int)current_event.ident, recv_msg);
                    } else if(byte_num < 0){
                        printf("from client (fd = %d)recive msg error.\n",(int)current_event.ident);
                    }else{
                        EV_SET(&event_change, current_event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
                        kevent(kq, &event_change, 1, NULL, 0, NULL);
                        close((int)current_event.ident);
                        for (int i = 0; i < CONCURRENT_MAX; i++) {
                            if (client_fds[i] == (int)current_event.ident) {
                                client_fds[i] = 0;
                                break;
                            }
                        }
                        printf("client fd:%d 不玩了 跑路了\n",(int)current_event.ident);
                    }
                }
            }
        }
    }
    return 0;
}