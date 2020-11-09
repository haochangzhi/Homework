#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int Account_UI_SignIn(){
    char name[30] , password[30];
    int sex;
    printf("请输入要注册的用户名:");
    scanf("%s",name);
    fflush();
    printf("请输入密码:");
    scanf("%s",password);
    fflush();
    return Account_Srv_SignIn(name ,sex ,password); //向服务器发送注册请求，同时等待服务器确认
}

int Account_UI_Login(){
    char name[30] , password[30];
    printf("请输入用户名:");
    scanf("%s",name);
    fflush();
    printf("请输入密码:");
    scanf("%s",password);
    fflush();
    return Account_Srv_Login(name , password); //向服务器发送登陆请求，同时等待服务器返回的校验信息
}