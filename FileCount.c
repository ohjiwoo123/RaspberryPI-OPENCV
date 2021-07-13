#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>

#include <string.h>
#include <stdlib.h>
#include <error.h>

int main(void)
{
    int    file_count = 0;
    char *dir_path = "./folders/";
    struct dirent *dir_ent;
    DIR  *dp;

    if ((dp = opendir(dir_path)) == NULL)
    {
        fprintf(stderr, "opendir() error!\n");
        exit(EXIT_FAILURE);
    }

    while ((dir_ent = readdir(dp)) != NULL)
    {
        if (strcmp(dir_ent->d_name, ".") == 0 || strcmp(dir_ent->d_name, "..") == 0)
            continue;

        ++file_count;
    }

    printf("File Count = %d\n", file_count);
    closedir(dp);
    exit(EXIT_SUCCESS);
}