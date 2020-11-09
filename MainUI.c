#include<stdio.h>
extern char name [30];
extern int sockfd;
struct SEND_DATA
{
    char stat;      //状态: 0x1 上线  0x2 下线  0x3 聊天数据 0x4 请求好友 0x5 添加好友 0x6注册用户 0x7登陆请求 0x08 发送文件
    char my_name[100]; //我的昵称
    char your_name[100] //发送目标的昵称
    char data[100]; //发送的实际聊天数据
    char account[100]; //为了方便，不考虑效率的情况下把所有信息汇聚进一个结构体，根据不同的stat复用结构体
    char password[100];
};
extern struct SEND_DATA recv_data;
extern struct SEND_DATA send_data;
extern int sign_flag=0;
extern int login_flag=0;

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
        
        scanf("%lc",&choice);
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

