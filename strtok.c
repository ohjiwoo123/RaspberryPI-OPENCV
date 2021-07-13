#include <stdio.h>
#include <string.h>

int main(void){
    char str[] = "Kim.aviPark.aviLee.aviChoi.aviSeo.avi";
    char *ptr = strtok(str, ".avi");
    
    while(ptr!=NULL)
    {
        printf("%s\n",ptr);
        ptr = strtok(NULL,".avi");
    }
    return 0;
}