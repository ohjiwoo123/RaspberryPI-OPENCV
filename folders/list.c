#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <error.h>
  
int main()
{
    DIR *dir;
    struct dirent *ent;
    dir = opendir ("./");
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
}