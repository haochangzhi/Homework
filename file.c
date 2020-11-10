#include <stdio.h>
#include <string.h>
int main()
{
	FILE* fp =NULL;
	char buff[255];
	int account;
	char password[100];
	//account = 123152;
	strcpy(buff,"zhihaochang1");

	fp = fopen("./acc_pass.txt","a+");
	//fprintf(fp,"%d ",account);
	//fprintf(fp,"%s\n",password);
	//fputs("This is testing for fputs...\n",fp);
	//fseek(fp,0,SEEK_SET);
	fscanf(fp,"%d",&account);
	printf("%d",account);
	fscanf(fp,"%s",password);
	printf("%s",password);
	//fgets(buff,255,(FILE*)fp);
	printf("%d",strcmp(buff,password));
	fclose(fp);
	return 1;
}
