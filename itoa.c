#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
int main()
{
	int a = 12315;
	char index [100];
	sprintf(index,"%d",a);
	FILE* fp =NULL;
    
    fp = fopen(index,"a+");
	//printf("%s",index);
	fprintf(fp,"%d ",515);
	fseek(fp,0,SEEK_SET);
	fscanf(fp,"%d",&a);
	printf("%d",a);
	fclose(fp);
return 1;
	
}
