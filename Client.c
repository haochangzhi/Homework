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

void Main_UI_Hello();
void Account_UI_Login();
void Account_UI_SignIn();
void Main_UI_Menu();

//保存文件的信息
#pragma pack(1)
//结构体: 消息结构体
struct SEND_DATA
{
    char stat;      //状态: 0x1 上线  0x2 下线  0x3 聊天数据 0x4 请求好友 0x5 添加好友 0x6注册用户 0x7登陆请求 0x08 发送文件
    char my_name[100]; //我的昵称
    char your_name[100]; //发送目标的昵称
    char data[100]; //发送的实际聊天数据
    char account[100]; //为了方便，不考虑效率的情况下把所有信息汇聚进一个结构体，根据不同的stat复用结构体
    char password[100];
};

int sockfd;
struct SEND_DATA recv_data;
struct SEND_DATA send_data;
int run_flag=1; //运行标志
int sign_flag=0;
int login_flag=0;

char name [30];
/*
线程工作函数
*/
void *thread_work_func(void *arg)
{
     //3.2 接收数据
    int select_cnt=0;
    int r_cnt;
    char choice;
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
            //用户给我发送的信息
            else if(recv_data.stat==0x3)
            {
                printf("%s:%s\n",recv_data.your_name,recv_data.data);
            } 
            //用户请求与我成为好友
            else if(recv_data.stat==0x4)
            {
                printf("%s 用户申请与你成为好友\n",recv_data.your_name);
                fflush(stdin);
                printf("是否同意与他成为好友，No=0 or Yes=1\n");
                scanf("%c",&choice);
                //与用户成为好友，给服务器发送0x5，服务器收到以后就会建立与用户的联系
                if(choice == '1')
                {
                    send_data.stat = 0x5;
                    strcpy(send_data.my_name,name);
                    strcpy(send_data.your_name,recv_data.your_name);
                    write(sockfd,&send_data,sizeof(struct SEND_DATA));
                } 
            }
            //用户同意的信息
            else if(recv_data.stat==0x5)
            {
                printf("%s 用户同意了你的好友请求\n",recv_data.your_name);
            }
            //服务器存储注册数据成功
            else if(recv_data.stat==0x6)
            {
                sign_flag = 1;
            }
            //服务器校验登录数据正确
            else if(recv_data.stat==0x7)
            {
                login_flag = 1;
            }
            //有用户给我发了文件，后台接收
            else if (recv_data.stat==0x8)
            {
                printf("%s 用户发送文件给我\n",recv_data.your_name);
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

void Account_UI_Login()
{
    system("clear");
    char password[30];
    printf("请输入用户名:");
    scanf("%s",name);
    fflush(stdin);
    printf("请输入密码:");
    scanf("%s",password);
    fflush(stdin);
    send_data.stat = 0x7;
    strcpy(send_data.my_name,name);
    strcpy(send_data.password,password);
    write(sockfd,&send_data,sizeof(struct SEND_DATA));
    while(1)
    {
        if(login_flag)
            break;
    }
    //告诉服务器我上线了
    send_data.stat=0x1; //上线
    strcpy(send_data.my_name,name); //昵称
    write(sockfd,&send_data,sizeof(struct SEND_DATA));
    return;
}
void Account_UI_SignIn()
{
    system("clear");
    char name[30] , password[30];
    printf("请输入用户名:");
    scanf("%s",name);
    fflush(stdin);
    printf("请输入密码:");
    scanf("%s",password);
    fflush(stdin);
    send_data.stat = 0x6;
    strcpy(send_data.my_name,name);
    strcpy(send_data.password,password);
    write(sockfd,&send_data,sizeof(struct SEND_DATA));
    while(1)
    {
        if(sign_flag)
            break;
    }
    return;
}
void Main_UI_Menu(){
    char choice;
    do{
        system("clear");
        //Friends_UI_ShowList();
        //Friends_UI_ShowApply();
        printf( "--------------------------------\n");
        printf( "1.选择好友|2.添加好友\n");
        printf( "--------------------------------\n"
                "功能选择:");
        
        scanf("%c",&choice);
        if(choice == '\n') continue;
        fflush(stdin);
        switch(choice){
            case '1':
                //Chat_UI_Private();
                break;
            case '2':
                //Friends_UI_Add();
                break;
        }
    }while(choice != '4');
}

void Main_UI_Hello(){
    int choice;
    do{
        system("clear");
        printf(
            "==============================\n"
            " ****欢迎使用葫芦娃聊天室****\n"
            "==============================\n");
        printf(
                "功能选项:\n"
                "\t1.登录\n"
                "\t2.注册\n"
                "请输入功能序号:"
               );
        scanf("%d" , &choice);
        fflush(stdin);
        switch(choice){
            case 1:
                Account_UI_Login(); //进入登陆UI
                return;
                break;
            case 2:
                Account_UI_SignIn();  //进入注册UI
                break;
            case 3:
                return;
                break;
            default:
                break;
        }
    }while(1);
}

/*
TCP客户端的创建步骤:
1. 创建socket套接字
2. 连接服务器
3. 进行通信
*/
int main(int argc,char **argv)
{
    char my_name[100]; //我的昵称
     if(argc!=3)
    {
        printf("参数: ./tcp_client <IP地址> <端口号> \n");
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

    //创建线程接收消息
    pthread_t pthread_id;
    pthread_create(&pthread_id,NULL,thread_work_func,NULL);

    //显示登录界面
    Main_UI_Hello();
    
    //显示好友添加或选择界面
    Main_UI_Menu();

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
