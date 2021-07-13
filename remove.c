/**********************************************************************
* ex_remove.c                                                         *
* exmple source – delete file and directory                           *
**********************************************************************/
#include <linux/limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
void Remove(const char *path);
int main(int argc, char **argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s <path name>\n", argv[0]);
        return 1;
    }
 
    Remove(argv[1]);
    return 0;
}
 
void Remove(const char *path)
{
    struct stat statbuf;
    if(stat(path, &statbuf)<0)
    {
        perror("stat error");
        exit(1);
    }
 
    if(S_ISDIR(statbuf.st_mode))
    {
        DIR *dir = opendir(path);
        struct dirent *de=NULL;
 
 
        while((de = readdir(dir))!=NULL)
        {
            if(strcmp(de->d_name,".")==0)
            {
                continue;
            }
            if(strcmp(de->d_name,"..")==0)
            {
                continue;
            }
            char npath[PATH_MAX];
            sprintf(npath,"%s/%s",path, de->d_name);
            Remove(npath);
 
        }
        closedir(dir);
        rmdir(path);
    }
    else
    {
        unlink(path);
    }
}