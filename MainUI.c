#include<stdio.h>

void Main_UI_Hello(){
    int choice;
    do{
        if(gl_uid > 0){
            Main_UI_Menu(); //登陆成功后，得到登陆用户信息，进入Menu菜单
        }
        system("clear");
        printf(
            "==============================\n"
            " ****欢迎使用葫芦娃聊天室****\n"
            "==============================\n");
        printf(
                "功能选项:\n"
                "\t1.登录\n"
                "\t2.注册\n"
                "\t3.退出\n"
                "请输入功能序号:"
               );
        scanf("%d" , &choice);
        fflush();
        switch(choice){
            case 1:
                //gl_uid = Account_UI_Login(); //进入登陆UI
                break;
            case 2:
                //Account_UI_SignIn();  //进入注册UI
                break;
            case 3:
                return;
                break;
            default:
                break;
        }
    }while(1);
}

void Main_UI_Menu(){
    Friends_Srv_GetList();
    char choice;
    do{
        system("clear");
        Friends_UI_ShowList();
        Friends_UI_ShowApply();
        printf( "--------------------------------\n");
        printf( "1.选择好友|2.处理申请|3.添加好友\n");
        printf( "--------------------------------\n"
                "功能选择:");
        
        scanf("%c",&choice);
        if(choice == '\n') continue;
        fflush();
        switch(choice){
            case '1':
                //Chat_UI_Private();
                break;
            case '2':
                //Friends_UI_Apply();
                break;
            case '3':
                //Friends_UI_Add();
                break;
        }
    }while(choice != '4');
}
