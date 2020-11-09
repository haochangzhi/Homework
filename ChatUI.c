
#include<stdio.h>
#include<string.h>
#include<unistd.h>

extern int gl_uid;
extern friends_t * FriendsList;
extern group_t * GroupList;
extern private_msg_t* PriMsgList;
extern group_msg_t* GroMsgList;
group_t *curGroup = NULL;
void Chat_UI_Private(){
    friends_t *curFriend = NULL;
    char msg[1000];
    char fname[30];
    friends_t * f;
    while(1){
        printf("请输入好友用户名:");
        scanf("%s",fname);
        fflush();
        List_ForEach(FriendsList ,f){
            //printf("f->name = %s\n" ,f->name);
            if(strcmp(f->name ,fname) == 0){
                curFriend = f;
            }
        }
        if(curFriend == NULL ){
            printf("%s 不是你的好友." , fname);
            getchar();
            return;
        }else{
            break;
        }
    }
    int this_msg_num;
    private_msg_t * m;
    do{
        system("clear");
        printf( "---------------------------------------\n"
                " -                私聊               -\n"
                "---------------------------------------\n");
        this_msg_num = 0;
        List_ForEach(PriMsgList ,m){
            if(m -> from_uid == curFriend -> uid || m -> from_uid == gl_uid){
                this_msg_num ++;
            }
        }

        List_ForEach(PriMsgList ,m){
            if( m-> from_uid == curFriend -> uid ){
                if(this_msg_num > 10){
                    List_FreeNode(PriMsgList ,m ,private_msg_t);
                    this_msg_num --;
                }else{
                    printf("\t\e[31m%s\e[0m ",m->time);
                    printf("%s\n",m -> name);
                    printf("\t  \e[1m%s\e[0m\n",m -> msg);

                }
            }else if(m -> from_uid == gl_uid){
                if(this_msg_num > 10){
                    List_FreeNode(PriMsgList ,m ,private_msg_t);
                    this_msg_num --;
                }else{
                    printf("\t\e[32m%s\e[0m ",m->time);
                    printf("我\n");
                    printf("\t  \e[1m%s\e[0m\n",m -> msg);
                }
            }
        }
        printf( "---------------------------------------\n"
                "功能: /r  :  返回上一级\n"
                "      /f  :  发送文件\n"
                "      /m  :  聊天记录\n"
                "      回车:  发送/刷新消息\n"
                "---------------------------------------\n");
        printf("消息/功能:");
        sgets(msg ,1000);
        if(*msg == '\0') continue;
        else if(strcmp(msg,"/r") == 0) {
            curFriend -> NewMsgNum = 0;
            return;   
        }else if(strcmp(msg,"/f") == 0){
            if(curFriend -> is_online == 0){
                printf("当前好友不在线,无法发送文件\n");
                getchar();
                continue;
            }
            char filename[100];
            while(1){
                printf("请输入文件路径:");
                sgets(filename ,100);
                if(*filename == '\0') {
                    break;
                }
                if(Chat_Srv_SendFile(filename ,curFriend -> uid)){
                   printf("文件发送成功");
                }
                getchar();
                break;
            }
        }
        else if(strcmp(msg,"/m") == 0) {
            Chat_Srv_GetPrivateRec(curFriend -> uid);
            getchar();
        }else{
            Chat_Srv_SendPrivate(curFriend -> uid,msg);
        }
    }while(1);
}


