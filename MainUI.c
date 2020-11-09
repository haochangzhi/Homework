#include<stdio.h>
extern char name [30];
extern int sockfd;
extern struct SEND_DATA send_data;
extern int sign_flag=0;
extern int login_flag=0;


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
        Friends_UI_ShowList();
        Friends_UI_ShowApply();
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
