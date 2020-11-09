#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

//保存文件的信息
#pragma pack(1)
//结构体: 消息结构体
struct SEND_DATA
{
    char stat;      //状态: 0x1 上线  0x2 下线  0x3 正常数据 0x4 请求好友 0x5 同意好友 0x6注册用户 0x7登陆请求
    char my_name[100]; //我的昵称
    char your_name[100] //发送目标的昵称
    char data[100]; //发送的实际聊天数据
    char account[100]; //为了方便，不考虑效率的情况下把所有信息汇聚进一个结构体，根据不同的stat复用结构体
    char password [100];
};

int sockfd;
struct SEND_DATA recv_data;
struct SEND_DATA send_data;
int run_flag=1; //运行标志

/*
线程工作函数
*/
void *thread_work_func(void *arg)
{
     //3.2 接收数据
    int select_cnt=0;
    int r_cnt;
    fd_set readfds;

    while(1)
    {
        //清空集合
        FD_ZERO(&readfds);
        //添加需要检测的文件描述符
        FD_SET(sockfd,&readfds);
        //检测客户端的IO状态  
        select_cnt=select(sockfd+1,&readfds,NULL,NULL,NULL);
        if(select_cnt>0)
        {
            //读取服务器发送过来的数据
            r_cnt=read(sockfd,&recv_data,sizeof(struct SEND_DATA));
            if(r_cnt<=0)  //判断对方是否断开连接
            {
                printf("服务器断开连接.\n");
                break;
            }

            //有用户上线
            if(recv_data.stat==0x1)
            {
                printf("%s 用户上线.\n",recv_data.your_name);
            }
            //用户下线
            else if(recv_data.stat==0x2)
            {
                printf("%s 用户下线.\n",recv_data.your_name);
            }
            else if(recv_data.stat==0x3)
            {
                printf("%s:%s\n",recv_data.your_name,recv_data.data);
            } 
            else if(recv_data.stat==0x5)
            {
                printf("%s 用户同意了你的好友请求",recv_data.your_name)
            }
            
            
        }
        else
        {
            printf("select 函数出现错误.\n");
            break;   
        }
    }
    run_flag=0; //服务器断开连接
}

/*
TCP客户端的创建步骤:
1. 创建socket套接字
2. 连接服务器
3. 进行通信
*/
int main(int argc,char **argv)
{
     if(argc!=4)
    {
        printf("参数: ./tcp_client <IP地址> <端口号> <用户昵称>\n");
        return 0;
    }
    /*1. 创建socket套接字*/
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        printf("客户端:套接字创建失败.\n");
        return 0;
    }
    /*2. 连接服务器*/
    struct sockaddr_in addr;
    addr.sin_family=AF_INET; //IPV4
    addr.sin_port=htons(atoi(argv[2])); //65535  服务器的端口号
    addr.sin_addr.s_addr=inet_addr(argv[1]); //服务器IP地址
    if(connect(sockfd,(const struct sockaddr *)&addr,sizeof(struct sockaddr)))
    {
        printf("客户端:连接服务器失败.\n");
        return 0;
    }

    //上线提醒
    send_data.stat=0x1; //上线
    strcpy(send_data.my_name,argv[3]); //昵称
    write(sockfd,&send_data,sizeof(struct SEND_DATA));

    //创建线程接收消息
    pthread_t pthread_id;
    pthread_create(&pthread_id,NULL,thread_work_func,NULL);

    //发送消息
    send_data.stat=0x3;
    while(1)
    {
        fgets(send_data.data,100,stdin); //从键盘上读取消息
        if(run_flag==0)break; //与服务器断开连接
        if(write(sockfd,&send_data,sizeof(struct SEND_DATA))<0)
        {
            printf("向服务器发送消息失败.\n");
            break;
        }
    }
    /*4. 关闭套接字*/
    close(sockfd);
    printf("聊天结束.\n");
    return 0;
}
