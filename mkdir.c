#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>

const char *path = "/home/jwoh/blackbox/data"; 
char dirname;
char buf ;

int main(){

    mkdir(path, 0755);

}