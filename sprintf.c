#include <stdio.h>
#include <string.h>

int i = 10;
char buf[100];


int main(){
sprintf(buf, "int i =%d입니다." , i);
printf("%s\n",buf);
}
