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
int Account_UI_Login();
void Account_UI_SignIn();
void Main_UI_Menu();
void Friends_UI_Add();
void Chat_UI_Private();
//保存文件的信息
#pragma pack(1)
//结构体: 消息结构体
struct SEND_DATA
{
    char stat;      //状态: 0x1 上线  0x2 下线  0x3 聊天数据 0x4 请求好友 0x5 添加好友 0x6注册用户 0x7登陆请求 0x08 发送文件
    char my_name[100]; //我的昵称
    int your_account; //发送目标的账号
    char data[100]; //发送的实际聊天数据
    int account; //为了方便，不考虑效率的情况下把所有信息汇聚进一个结构体，根据不同的stat复用结构体
    char password[100];
};

int sockfd;
int login_account;
struct SEND_DATA recv_data;
struct SEND_DATA send_data;
int run_flag=1; //运行标志
int chat_flag = 0;
int sign_flag=0;
int login_flag=0;
int friend_RQ;
//创建管道和线程通信
int pipe1[2];
	
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
    time_t   timep; 
    time   (&timep);
    
    //write( *(int*)arg,str, sizeof(str));
    while(1)
    {
        //清空集合
        FD_ZERO(&readfds);
        //添加需要检测的文件描述符
        FD_SET(sockfd,&readfds);
        //检测客户端的IO状态  
        select_cnt=select(sockfd+1,&readfds,NULL,NULL,NULL);
        char str[] = "L";
        
        if(select_cnt>0)
        {
            //读取服务器发送过来的数据
            //printf("进入消息中断.\n");
            r_cnt=read(sockfd,&recv_data,sizeof(struct SEND_DATA));
            //printf("stat:%d\n",recv_data.stat);
	        //printf("my_name:%s\n",recv_data.my_name);
	        //printf("your_account:%d\n",recv_data.your_account);
	        //printf("data:%s",recv_data.data);
	        //printf("account:%d\n",recv_data.account);
	        //printf("password:%s\n",recv_data.password);
            if(r_cnt<=0)  //判断对方是否断开连接
            {
                printf("服务器断开连接.\n");
                break;
            }

            //有用户上线
            if(recv_data.stat==0x1)
            {
                printf("以下好友在线\n");
                printf("%d\n",recv_data.your_account);
            }
            //用户下线
            else if(recv_data.stat==0x2)
            {
                printf("%d 用户下线.\n",recv_data.your_account);
            }
            //用户给我发送的信息
            else if(recv_data.stat==0x3)
            {
                chat_flag = 1;
                //Chat_UI_Private();
                printf("%s",ctime(&timep)); 
                printf("用户%d对你说:%s\n",recv_data.your_account,recv_data.data);
            } 
            //用户请求与我成为好友
            else if(recv_data.stat==0x4)
            {
                printf("%d 用户申请与你成为好友\n",recv_data.your_account);
                printf("是否同意与他成为好友,请输入FriendACC以同意\n");
                friend_RQ = recv_data.your_account;
            }
            //用户同意的信息
            else if(recv_data.stat==0x5)
            {
                printf("%d 用户同意了你的好友请求\n",recv_data.your_account);
            }
            //服务器存储注册数据成功
            else if(recv_data.stat==0x6)
            {
                printf("%s\n",recv_data.data);
                
            }
            //服务器校验登录数据正确
            else if(recv_data.stat==0x7)
            {
                //printf("登陆回传%s",recv_data.data);
                if(strcmp(recv_data.data,"Login")!=0)
        		    printf("%s",recv_data.data);
		        else 
                    write( *(int*)arg, str, sizeof(str));
                
            }
            //有用户给我发了文件，后台接收
            else if (recv_data.stat==0x8)
            {
                printf("%d 用户发送文件给我\n",recv_data.your_account);
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

int Account_UI_Login()
{
    struct SEND_DATA send_data;
    struct SEND_DATA recv_data;
    system("clear");
    int account;
    char buff[100];
    char password[30];
    printf("请输入账户:");
    scanf("%d",&account);
    fflush(stdin);
    printf("请输入密码:");
    scanf("%s",password);
    fflush(stdin);
    send_data.stat = 0x7;
    strcpy(send_data.data,password);
    strcpy(send_data.my_name,password);
    send_data.account = account;
    strcpy(send_data.password,password);
    write(sockfd,&send_data,sizeof(struct SEND_DATA));
	usleep(500);
	read(pipe1[0],buff,100);
	//printf("%d\n",(*buff=='L'));
	//printf("%s",buff);
    if(*buff == 'L')
    	{printf("欢迎登陆\n");
    	sleep(2);
    	login_account = account;
    	return 1;}
    else{
    sleep(2);
    fflush(stdin);
    return 0;
    }
}
void Account_UI_SignIn()
{
    system("clear");
    int account;
    char password[30];
    printf("请输入用户名:");
    scanf("%d",&account);
    fflush(stdin);
    printf("请输入密码:");
    scanf("%s",password);
    fflush(stdin);
    send_data.stat = 0x6;
    send_data.account=account;
    strcpy(send_data.password,password);
    write(sockfd,&send_data,sizeof(struct SEND_DATA));
    sleep(2);
    return;
}
void Main_UI_Menu(){
    char choice;
    do{
        //system("clear");
        //Friends_UI_ShowList();
        //Friends_UI_ShowApply();
        printf( "--------------------------------\n");
        printf( "1.选择好友|2.申请添加好友| \n");
        printf( "--------------------------------\n"
                "功能选择:");
        
        scanf("%c",&choice);
        fflush(stdin);
        switch(choice){
            case '1':
                Chat_UI_Private();
                break;
            case '2':
                Friends_UI_Add();
                break;
            case '3':
                //Friends_UI_Add();
            	printf("chatting");
                return;
                break;    
        }
    }while(choice != '4');
}
void Chat_UI_Private()
{
    time_t   timep; 
    time   (&timep);
    send_data.stat = 0x3;
    char buff[100];
    int friend_account;
    printf("请输入好友账号,以开始聊天:\n输入 -888 开始挂机等好友模式");
    scanf("%d",&friend_account);
    fflush(stdin);
    send_data.account=login_account;
    send_data.your_account = friend_account;
    if(friend_account == -888)
    {
        printf("输入Return以退出\n");
        while(1)
        {
            printf("挂机模式：");
            fgets(buff,100,stdin);
            if((strcmp(buff,"FriendACC\n")==0)||(friend_RQ != 0))
            {
                send_data.stat = 0x5;
                send_data.account=login_account;
                send_data.your_account = friend_RQ;
                write(sockfd,&send_data,sizeof(struct SEND_DATA));
                printf("已同意和%d用户成为好友",friend_RQ);
                sleep(2);
            }
            else if (strcmp(buff,"Return\n")==0)
                break;
        }
    }
    printf("请输入好友账号,以开始聊天:\n输入 -888 开始挂机等好友模式");
    scanf("%d",&friend_account);
    fflush(stdin);
    while(1)
    {
        send_data.stat = 0x3;
        send_data.your_account = friend_account;
        printf("%s",ctime(&timep)); 
        printf("对%d说：",friend_account);
        fgets(send_data.data,100,stdin); //从键盘上读取消息
        if(run_flag==0)break; //与服务器断开连接
        if(write(sockfd,&send_data,sizeof(struct SEND_DATA))<0)
        {
            printf("向服务器发送消息失败.\n");
            break;
        }
        //break;
        if(strcmp(send_data.data,"88\n")==0)
            break;
        if(strcmp(send_data.data,"FriendACC\n")==0)
            {
                send_data.stat = 0x5;
                send_data.account=login_account;
                send_data.your_account = friend_RQ;
                write(sockfd,&send_data,sizeof(struct SEND_DATA));
                printf("已同意和%d用户成为好友",friend_RQ);
                sleep(2);
            } 
    }
}
void Friends_UI_Add()
{
    send_data.stat = 0x4;
    int friend_account;
    system("clear");
    printf("请输入好友账号:\n");
    scanf("%d",&friend_account);
    fflush(stdin);
    send_data.account=login_account;
    send_data.your_account = friend_account;
    write(sockfd,&send_data,sizeof(struct SEND_DATA));
    sleep(2);
    return;
}
void Main_UI_Hello(){
    int choice;
    do{
        system("clear");
        printf(
            "==============================\n"
            " ****欢迎使用Zhichat****\n"
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
                if(Account_UI_Login()) 
                	{return;
                	getchar();}
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
    time_t timep; 
    time (&timep); 
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
	if(pipe(pipe1)<0)
	exit(1);
	int flags1 = fcntl(pipe1[1], F_GETFL, 0);
	flags1 |= O_NONBLOCK;
	fcntl(pipe1[1], F_SETFL, flags1);
	int flags0 = fcntl(pipe1[0], F_GETFL, 0);
	flags0 |= O_NONBLOCK;
	fcntl(pipe1[0], F_SETFL, flags0);
    //创建线程接收消息
    pthread_t pthread_id;
    pthread_create(&pthread_id,NULL,thread_work_func,&pipe1[1]);

    //显示登录界面
    Main_UI_Hello();
    //显示好友添加或选择界面

    send_data.stat=0x1;
    send_data.account = login_account;
    write(sockfd,&send_data,sizeof(struct SEND_DATA));
    usleep(200);
    Main_UI_Menu();

    /*4. 关闭套接字*/
    close(sockfd);
    printf("聊天结束.\n");
    return 0;
}
