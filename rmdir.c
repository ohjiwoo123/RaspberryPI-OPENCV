#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <dirent.h> 
#include <stdio.h> 
int rmdirs(const char *path, int force) { 
    DIR * dir_ptr = NULL; 
    struct dirent *file = NULL; 
    struct stat buf; 
    char filename[1024]; 
    /* 목록을 읽을 디렉토리명으로 DIR *를 return 받습니다. */ 
    if((dir_ptr = opendir(path)) == NULL) { 
        /* path가 디렉토리가 아니라면 삭제하고 종료합니다. */ 
        return unlink(path); 
        } 
        /* 디렉토리의 처음부터 파일 또는 디렉토리명을 순서대로 한개씩 읽습니다. */ 
        while((file = readdir(dir_ptr)) != NULL) { 
            // readdir 읽혀진 파일명 중에 현재 디렉토리를 나타네는 . 도 포함되어 있으므로 // 무한 반복에 빠지지 않으려면 파일명이 . 이면 skip 해야 함 
            if(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) { 
                continue; 
                } 
                sprintf(filename, "%s/%s", path, file->d_name); 
                /* 파일의 속성(파일의 유형, 크기, 생성/변경 시간 등을 얻기 위하여 */ 
                if(lstat(filename, &buf) == -1) { continue; } if(S_ISDIR(buf.st_mode)) { 
                    // 검색된 이름의 속성이 디렉토리이면 
                    /* 검색된 파일이 directory이면 재귀호출로 하위 디렉토리를 다시 검색 */ 
                    if(rmdirs(filename, force) == -1 && !force) { 
                        return -1; 
                        } 
                    } 
                    else if(S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) { 
                        // 일반파일 또는 symbolic link 이면 
                        if(unlink(filename) == -1 && !force) { 
                            return -1; 
                        } 
                    }
            } 
            /* open된 directory 정보를 close 합니다. */ 
            closedir(dir_ptr); 
            return rmdir(path); 
}


// path =  - 삭제할 파일 또는 일괄로 삭제할 디렉토리
// force = - 삭제시 권한 문제 등으로 파일을 삭제할 수 없다는 파일이 있을 때에 어떻게 할 것인지에 대한 옵션입니다. 
// - 파일 또는 directory를 삭제하다가 최초의 오류가 발생하면 멈출것인지, 아니면 오류난 것은 오류난 대로 두고 삭제할 수 있는 것은 모두 삭제합니다. 
// 0이면 : 최초 오류 발생시 중지, 0이 아니면 : 오류가 발생하더라도 나머지는 계속 삭제함
// return = 0 - 정상적으로 path 하위의 모든 파일 및 모든 디렉토리를 삭제하였습니다. -1 - 오류가 발생하였으며, 상세한 오류는 errno에 저장됩니다.

