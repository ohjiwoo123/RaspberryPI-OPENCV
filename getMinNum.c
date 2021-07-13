#include<stdio.h>
#include<string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <dirent.h>
#include <error.h>
#include <limits.h>

#define MAX_LIST 50 


char FolderList()
{
    DIR *dir;
    struct dirent *ent;
    dir = opendir ("./folders/");
    if (dir != NULL) {
  
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
        printf ("%s\n", ent->d_name);
    }
    closedir (dir);
    } else {
         /* could not open directory */
         perror ("");
        return EXIT_FAILURE;
    }
    char folders;
    folders [ent->d_name];
    printf("folders=%s",folders);
}


long unsigned int searchOldFolder(void)
{
    
    // 숫자를 비교할 배열의 갯수를 MAX_LIST
    long unsigned int num[MAX_LIST] ={FolderList(),NULL};
    int i;
    long unsigned int min;
    min = num[0];     //min 초기화

    // 배열의 들어있는 값을 출력
    for(i=0;i<MAX_LIST;i++)
    {
        if(num[i]==NULL)
            break;
        printf("%d 번째 배열 요소 . . . . . . %d \n",i+1, num[i]);
    }

    for(i = 0;i<MAX_LIST;i++)
    {
        if(num[i]==NULL)
            break;    
        else
            if(num[i] < min ) //num[i]가 min보다 작다면
                min = num[i]; //min 에는 num[i]의 값이 들어감
    }

    return min;

}

int main(void)
{
    long unsigned int result; 
    char folderName[30];
    result = searchOldFolder();
    sprintf(folderName,"%ld",result);
    printf("가장 오래된 폴더는 %s 이다.\n",folderName);
}

