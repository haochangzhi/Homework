#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
/* According to POSIX.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>

int sockfd;  //服务器的套接字

/*
设置网卡的IP地址
$ sudo ifconfig eth0 192.168.1.23  

服务器创建流程:
1. 创建socket套接字(文件描述符)---类型open函数一样
2. 绑定端口号(创建服务器:提供端口号和IP地址)
3. 设置监听的客户端数量(设置待处理的队列最大缓存待连接的客户端数量)
4. 等待客户端连接(被动--阻塞): 多线程和多进程方式并发处理客户端连接。
5. 实现通信(客户端连接成功)
*/
void *thread_work_func(void *arg);
void signal_work_func(int sig);

//最大的线程数量
#define MAX_THREAD_CNT 100
//保存文件的信息
#pragma pack(1)
//保存线程的信息
struct THREAD_INFO
{
    pthread_t thread_id; //存放所有线程ID
    FILE *fp; //打开的文件
    int fd; //客户端套接字
};

int Thread_GetIndex(struct THREAD_INFO *thread_id);
void Thread_ClearIndex(struct THREAD_INFO *thread_id,pthread_t id);
int Thread_GetThreadID_Index(struct THREAD_INFO *thread,pthread_t id);
int thread_run_flag=1; //线程运行标志
struct THREAD_INFO thread_info[MAX_THREAD_CNT];

//定义结构体,保存连接上服务器的所有客户端
#pragma pack(1)
struct Client_FD
{
    int fd;
    int client_account;
    struct Client_FD *next;
};
struct Client_FD *list_head=NULL; //定义链表头
struct Client_FD * LIST_HeadInit(struct Client_FD *list_head);
void List_AddNode(struct Client_FD *list_head,int fd);
void ListDelNode(struct Client_FD *list_head,int fd);

//定义互斥锁
pthread_mutex_t mutex_lock;

//实现的函数
void User_Online(int account,struct Client_FD *list_head,int fd);
void User_Offline(int account,struct Client_FD * list_head,int fd);
void Message_Deliver(struct Client_FD * list_head,int my_account,int your_account,char * data);
void Friend_Request(int my_account,int your_account);
void Friend_Accept(int my_account,int your_account);
void Sign_In(int my_account,char * password, int fd);
void User_Login(int my_account,char * password, int fd);
int Get_FD_from_Account(int account);
void Send_Online_Message(int fd,int account);
void Send_Offline_Message(int fd,int account);

//结构体: 消息结构体
struct SEND_DATA
{
    char stat;      //状态: 0x1 上线  0x2 下线  0x3 聊天数据 0x4 请求好友 0x5 添加好友 0x6注册用户 0x7登陆请求 0x08 发送文件
    char my_name[100]; //我的昵称
    int your_account; //发送目标的账号
    char data[100]; //发送的实际聊天数据
    int account; //为了方便，不考虑效率的情况下把所有信息汇聚进一个结构体，根据不同的stat复用结构体
    char password[100];
};//转发消息

int main(int argc,char **argv)
{
    if(argc!=2)
    {
        printf("参数: ./tcp_server <端口号>\n");
        return 0;
    }
    //注册需要捕获的信号
    signal(SIGINT,signal_work_func);
    //忽略 SIGPIPE 信号--方式服务器向无效的套接字写数据导致进程退出
    signal(SIGPIPE,SIG_IGN);

    //初始化互斥锁
    pthread_mutex_init(&mutex_lock,NULL);
    //初始化链表头
    list_head=LIST_HeadInit(list_head);
    
    /*1. 创建socket套接字*/
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        printf("服务器:套接字创建失败.\n");
        return 0;
    }

    int on = 1;
    //设置端口号可以复用
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /*2. 绑定端口号*/
    struct sockaddr_in addr;
    addr.sin_family=AF_INET; //IPV4
    addr.sin_port=htons(atoi(argv[1])); //65535
    //addr.sin_addr.s_addr=inet_addr("192.168.2.16");
    addr.sin_addr.s_addr=INADDR_ANY; //本地所有IP地址 "0.0.0.0"
    if(bind(sockfd,(const struct sockaddr *)&addr,sizeof(struct sockaddr)))
    {
        printf("服务器:端口号绑定失败.\n");
        return 0;
    }
    /*3. 设置等待连接的客户端数量*/
    if(listen(sockfd,100))
    {
        printf("设置等待连接的客户端数量识别.\n");
        return 0;
    }

    /*4. 等待客户端连接(被动--阻塞)*/
    struct sockaddr_in client_addr;
    socklen_t addrlen;
    int *client_sockfd=NULL; //客户端的套接字
    int index;
    while(1)
    {
        addrlen=sizeof(struct sockaddr_in);
        client_sockfd=malloc(sizeof(int));
        *client_sockfd=accept(sockfd,(struct sockaddr *)&client_addr,&addrlen);
        if(*client_sockfd<0)
        {
            printf("服务器:处理客户端的连接失败.\n");
            break;
        }
        printf("连接上的客户端IP地址:%s\n",inet_ntoa(client_addr.sin_addr));
        printf("连接上的客户端端口:%d\n",ntohs(client_addr.sin_port));
        /*5. 创建子线程与客户端之间实现数据通信*/
        index=Thread_GetIndex(thread_info);//得到空闲的下标
        if(index==-1)
        {
            close(*client_sockfd);
        }
        else
        {
            if(pthread_create(&thread_info[index].thread_id,NULL,thread_work_func,client_sockfd))
            {
                printf("子线程创建失败.\n");
                break;
            }
            //设置分离属性
            pthread_detach(thread_info[index].thread_id);
        }
    }

    /*6. 关闭套接字*/
    signal_work_func(0);
    return 0;
}


//清理线程资源
void clear_resource_thread(void *arg)
{
    struct THREAD_INFO *thread_p=(struct THREAD_INFO*)arg;
    close(thread_p->fd);
    printf("%lu 线程资源清理成功.\n",thread_p->thread_id);
}

/*
函数功能: 线程工作函数
*/
void *thread_work_func(void *arg)
{
    int client_fd=*(int*)arg; //取出客户端套接字
    free(arg);
    struct SEND_DATA recdata; 
    int select_cnt=0;
    fd_set readfds;
    struct timeval timeout;
    int r_cnt;
    //保存客户端套接字描述符
    List_AddNode(list_head,client_fd);
    //注册清理函数
    //3种:  pthread_exit 、pthread_cleanup_pop(1); 被其他线程杀死(取消执行)
    int index;
    index=Thread_GetThreadID_Index(thread_info,pthread_self());
    thread_info[index].fd=client_fd;
    pthread_cleanup_push(clear_resource_thread,&thread_info[index]);

    //实现与客户端通信
    while(thread_run_flag)
    {   
        r_cnt=read(client_fd,&recdata,sizeof(struct SEND_DATA));
        //strcpy(list_head->name,recdata.my_name);
        Data_interrupt(client_fd,list_head,&recdata); //参数：1.进入消息中断的客户端的文件标识符 2.记录有全部客户端标识符的链表头 3.收到的消息
        if(r_cnt<=0)  //判断对方是否断开连接
        {
           	//sendata.stat=0x2; //下线
            //Client_SendData(client_fd,list_head,&sendata); //转发下线消息
            User_Offline(recdata.account,list_head,client_fd);
            ListDelNode(list_head,client_fd); //删除当前的套接字
            printf("服务器提示: 客户端断开连接.\n");
            break;
        }
    }
    //配套的清理函数
    pthread_cleanup_pop(1);
}

/*
函数功能: 信号处理函数
*/
void signal_work_func(int sig)
{
    thread_run_flag=0; //终止线程的执行
    int i=0;
    printf("正在清理线程资源.\n");
    for(i=0;i<MAX_THREAD_CNT;i++)
    {
        if(thread_info[i].thread_id!=0)
        {
            pthread_cancel(thread_info[i].thread_id);  //必须遇到线程取消点才会结束
            //如何设置取消点?
            //pthread_testcancel(); //判断是否需要结束本身
        }
    }
    sleep(2); //等待线程退出
    printf("服务器正常结束.\n");
    close(sockfd);
    exit(0);
}

/*
函数功能: 获取数组的下标索引
规定: 0表示无效  >0表示有效ID
返回值: -1表示空间满了 正常返回空闲下标值
*/
int Thread_GetIndex(struct THREAD_INFO *thread)
{
    int i=0;
    for(i=0;i<MAX_THREAD_CNT;i++)
    {
        if(thread[i].thread_id==0)return i;
    }
    return -1; 
}

//清除数组里无效的线程的ID
void Thread_ClearIndex(struct THREAD_INFO *thread,pthread_t id)
{
    int i=0;
    for(i=0;i<MAX_THREAD_CNT;i++)
    {
        if(thread[i].thread_id==id)thread[i].thread_id=0;
    }
}

//获取指定线程ID的下标索引
int Thread_GetThreadID_Index(struct THREAD_INFO *thread,pthread_t id)
{
    int i=0;
    for(i=0;i<MAX_THREAD_CNT;i++)
    {
        if(thread[i].thread_id==id)
        {
            return i;
        }
    }
    return -1;
}

//初始化链表头
struct Client_FD * LIST_HeadInit(struct Client_FD *list_head)
{
    if(list_head==NULL)
    {
        list_head=malloc(sizeof(struct Client_FD));
        list_head->next=NULL;
    }
    return list_head;
}

//添加节点
void List_AddNode(struct Client_FD *list_head,int fd)
{
    struct Client_FD *p=list_head;
    struct Client_FD *new_p;
    pthread_mutex_lock(&mutex_lock);
    while(p->next)
    {
        p=p->next;
    }
    new_p=malloc(sizeof(struct Client_FD));
    new_p->next=NULL;
    new_p->fd=fd;
    p->next=new_p;

    pthread_mutex_unlock(&mutex_lock);
}

//删除节点
void ListDelNode(struct Client_FD *list_head,int fd)
{
    struct Client_FD *p=list_head;
    struct Client_FD *tmp;
    pthread_mutex_lock(&mutex_lock);
    while(p->next)
    {
        tmp=p;
        p=p->next;
        if(p->fd==fd)
        {
            tmp->next=p->next;
            free(p);
            break;
        }
    }
    pthread_mutex_unlock(&mutex_lock);
}

/*
函数功能: 向在线的所有客户端发送消息

struct SEND_DATA
{
    char stat;      //状态: 0x1 上线  0x2 下线  0x3 聊天数据 0x4 请求好友 0x5 添加好友 0x6注册用户 0x7登陆请求 0x08 发送文件
    char my_name[100]; //我的昵称
    int your_account; //发送目标的账号
    char data[100]; //发送的实际聊天数据
    int account; //为了方便，不考虑效率的情况下把所有信息汇聚进一个结构体，根据不同的stat复用结构体
    char password[100];
};//转发消息
*/

void Data_interrupt(int client_fd,struct Client_FD *list_head,struct SEND_DATA *recdata)
{
    struct Client_FD *p=list_head;
    struct SEND_DATA sendata;
    pthread_mutex_lock(&mutex_lock);
    
    printf("收到来自文件标识符%d的消息，进入消息中断\n",client_fd);
    printf("stat:%d\n",recdata->stat);
	printf("my_name:%s\n",recdata->my_name);
	printf("your_account:%d\n",recdata->your_account);
	printf("data:%s",recdata->data);
	printf("account:%d\n",recdata->account);
	printf("password:%s\n",recdata->password);
	//write(client_fd,recdata,sizeof(struct SEND_DATA)); //回传调试
    switch(recdata->stat)
    {
        case 1: User_Online(recdata->account,list_head,client_fd);//将上线用户的name与对应的文件标示符绑定到一个数组里
        break;//case1执行的动作：1.绑定账号与fd，更新名单数组 2.将上线通知转发给在线好友 3.发送该用户的好友列表与在线情况
        case 2: User_Offline(recdata->account,list_head,client_fd);
        break; //case2执行的操作：1.将下线账号和对应fd从数组中删除 2.将下线通知转发给好友
        case 3: Message_Deliver(list_head,recdata->account,recdata->your_account,recdata->data);
        break; //case3执行的操作： 0. 读取文件确定双方是否是好友 1.找到信息收发两方的文件标示符 2.如果找不到，即your_name不在线，返回错误语句 3.否则将消息1对1转发
        case 4: Friend_Request(recdata->account,recdata->your_account); 
        break; //case4执行的操作： 0. 读取文件确定双方是否是好友 1.找到好友添加双方的文件标示符 2.如果找不到，即your_name不在线，返回错误语句 3.否则将好友请求消息发送给your_name
        case 5: Friend_Accept(recdata->account,recdata->your_account);
        break; //case5执行的操作 0.读取文件确定双方是否是好友 1.找到好友添加双方的文件标示符 2.如果能找到，则发送好友成立通知 3.改变文件 
        case 6: Sign_In(recdata->account,recdata->password,client_fd);
        break; //case6执行的操作 0.读取账户密码文件，确定是否存在要注册用户 1.如果不存在，则在文件最后加上注册用户的name和password 2.返回注册成功信息
        case 7: User_Login(recdata->account,recdata->password,client_fd);
        break; //case7执行的操作 0.读取账户密码文件，确定是否存在登录用户 1.如果存在且密码正确，返回登录成功 2.如果存在但密码不正确，返回密码错误 3.如果不存在，返回无此用户
        default: printf("收到无效信息/n");
        break;
    }
    pthread_mutex_unlock(&mutex_lock);
    return;
}
void User_Online(int account,struct Client_FD *list_head,int fd)
{
    struct SEND_DATA sendata;
    struct Client_FD * p = list_head;
    struct Client_FD * q = list_head;
    while(p->fd!=fd)
    {
        p = p->next;
    }
    p->client_account = account;
    
    sendata.stat = 0x1;

    int temp_fd;
    int temp_account;
    int i=0;

    char filename[100];
    sprintf(filename,"%d",account);

    FILE* fp =NULL;
    fp = fopen(filename,"a+");

    while(i<100)
    {
        fscanf(fp,"%d",&temp_account);
        printf("从文件读取到登陆用户的第%d个好友:%d\n",i,temp_account);
        printf("遍历链表结果：");
        q = list_head;
        while(q->next!=NULL)
        {
            q=q->next;
            if(q->client_account == temp_account)
                {
                    sendata.your_account = temp_account;
                    write(fd,&sendata,sizeof(struct SEND_DATA));
                }
            printf("%d->",q->client_account);
            //q=q->next;
        }
        printf("\n");
        i++;
        if(feof(fp))
            break;
    }
    fclose(fp);
    return;
}
void User_Offline(int account,struct Client_FD * list_head,int fd)
{
    struct SEND_DATA sendata;
    struct Client_FD * p = list_head;
    struct Client_FD * q = list_head;
    while(p->fd!=fd)
    {
        p = p->next;
    }
    p->client_account = account;
    
    sendata.stat = 0x2;

    int temp_fd;
    int temp_account;
    int i=0;

    char filename[100];
    sprintf(filename,"%d",account);

    FILE* fp =NULL;
    fp = fopen(filename,"a+");

    while(i<100)
    {
        fscanf(fp,"%d",&temp_account);
        printf("从文件读取到登陆用户的第%d个好友:%d\n",i,temp_account);
        printf("遍历链表结果：");
        q = list_head;
        while(q->next!=NULL)
        {
            q=q->next;
            if(q->client_account == temp_account)
                {
                    sendata.your_account = temp_account;
                    write(q->fd,&sendata,sizeof(struct SEND_DATA));
                }
            printf("%d->",q->client_account);
            //q=q->next;
        }
        printf("\n");
        i++;
        if(feof(fp))
            break;
    }
    fclose(fp);
    return;
}
void Message_Deliver(struct Client_FD * list_head,int my_account,int your_account,char * data)
{
    struct SEND_DATA sendata;
    sendata.stat = 0x3;
    strcpy(sendata.data,data);

    struct Client_FD * p = list_head;
    while(p->next!=NULL)
    {
        p=p->next;
        if(p->client_account == your_account)
            {
                sendata.your_account = my_account;
                write(p->fd,&sendata,sizeof(struct SEND_DATA));
            }
        printf("%d->",p->client_account);
        //p=p->next;
    }
}
void Friend_Request(int my_account,int your_account)
{
    
    return;
}

void Friend_Accept(int my_account,int your_account)
{
    struct SEND_DATA sendata; 
    sendata.stat = 0x5;
    int account=-1;
    char filename[100];
    sprintf(filename,"%d",my_account);
    FILE* fp =NULL;
    fp = fopen(filename,"a+");
    while(account != your_account)
    {
	    fscanf(fp,"%d",&account);
	    if(feof(fp))
	    	{
                fprintf(fp,"%d\n",your_account);
                break;
            }
    }
    fclose(fp);
    return;
}
void Sign_In(int my_account,char * password, int fd)
{
    FILE* fp =NULL;
    struct SEND_DATA sendata; 
    sendata.stat = 0x6;

	fp = fopen("./acc_pass.txt","a+");
	fprintf(fp,"%d ",my_account);
	fprintf(fp,"%s\n",password);
	fclose(fp);
	strcpy(sendata.data,"Sign in accept\n");
    write(fd,&sendata,sizeof(struct SEND_DATA));
    return;
}
void User_Login(int my_account,char * password, int fd)
{
    FILE* fp =NULL;
    char buff[255];
    int account = -1;
    struct SEND_DATA sendata; 
    sendata.stat = 0x7;

    //fseek(fp,0,SEEK_SET);
    fp = fopen("./acc_pass.txt","a+");
    while(account != my_account)
    {
	    fscanf(fp,"%d",&account);
	    fscanf(fp,"%s",buff);
	    if(feof(fp)){
	    	strcpy(sendata.data,"No that account\n");
	    	write(fd,&sendata,sizeof(struct SEND_DATA));
	    	fclose(fp);
	    	return;
    		}
    }
    if(!strcmp(password,buff))
    {
        strcpy(sendata.data,"Login");
    }
    else
    {
        strcpy(sendata.data,"Wrong password");
    }
    fclose(fp);
    write(fd,&sendata,sizeof(struct SEND_DATA));
    return;
}

int Get_FD_from_Account(int account)
{
    int fd=0;
    while(0)
    {
        fd++;
        if(fd>=100)
            return 0;
    }
    return fd;
}

void Send_Online_Message(int fd,int account)
{
    struct SEND_DATA sendata; 
    sendata.stat = 0x1;
    sendata.your_account = account;
    write(fd,&sendata,sizeof(struct SEND_DATA));
    return;
}
void Send_Offline_Message(int fd,int account)
{
    struct SEND_DATA sendata; 
    sendata.stat = 0x2;
    sendata.your_account = account;
    write(fd,&sendata,sizeof(struct SEND_DATA));
    return;
}
