#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include<string>
#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include<libgen.h>
#include<sys/vfs.h>
#include<errno.h>

using namespace cv;
using namespace std;

#define VIDEO_WINDOW_NAME "record"
char fileName[30];
char tBUF[100];
char fBUF[100];
char rmdirName[100];
#define TIME_FILENAME 0
#define FOLDER_NAME 1
#define LOG_TIME 2
const char *path = "/home/pi/blackbox/";
const char *MMOUNT="/proc/mounts";

static int filter(const struct dirent *dirent){ //scandir filter
    
    if((dirent->d_name)[0]!='2'){
        return 0;
    }
    else
        return 1;
    
}

struct f_size{
    long blocks;
    long avail;
    float ratio;
};

typedef struct _mountinfo 
{
    FILE *fp;                // 파일 스트림 포인터    
    char devname[80];        // 장치 이름
    char mountdir[80];        // 마운트 디렉토리 이름
    char fstype[12];        // 파일 시스템 타입
    struct f_size size;        // 파일 시스템의 총크기/사용율 
} MOUNTP;
MOUNTP *dfopen()
{
    MOUNTP *MP;

    // /proc/mounts 파일을 연다.
    MP = (MOUNTP *)malloc(sizeof(MOUNTP));
    if(!(MP->fp = fopen(MMOUNT, "r")))
    {
        return NULL;
    }
    else
        return MP;
}

MOUNTP *dfget(MOUNTP *MP)
{
    char buf[256];
    char *bname;
    char null[16];
    struct statfs lstatfs;
    struct stat lstat; 
    int is_root = 0;

    // /proc/mounts로 부터 마운트된 파티션의 정보를 얻어온다.
    while(fgets(buf, 255, MP->fp))
    {
        is_root = 0;
        sscanf(buf, "%s%s%s",MP->devname, MP->mountdir, MP->fstype);
        if (strcmp(MP->mountdir,"/") == 0) 
            is_root=1;
        //if (stat(MP->devname, &lstat) == 0 || is_root)
        if(is_root)
        {
            if (strstr(buf, MP->mountdir) && S_ISBLK(lstat.st_mode) || is_root)
            {
                // 파일시스템의 총 할당된 크기와 사용량을 구한다.        
                statfs(MP->mountdir, &lstatfs);
                MP->size.blocks = lstatfs.f_blocks * (lstatfs.f_bsize/1024); 
                MP->size.avail  = lstatfs.f_bavail * (lstatfs.f_bsize/1024); 
                MP->size.ratio  = (MP->size.avail *100) / MP->size.blocks;
                return MP;
            }
            break;
        }
    }
    rewind(MP->fp);
    return NULL;
}

void getTime(int ret_type){
    time_t UTCtime;
    struct tm *tm;
    time(&UTCtime);
    tm=localtime(&UTCtime);

    if(ret_type==TIME_FILENAME)
        strftime(tBUF,sizeof(tBUF),"%Y%m%d%H%M%S.avi",tm);
    if(ret_type==FOLDER_NAME)
        strftime(fBUF,sizeof(fBUF),"%Y%m%d%H",tm);
    if(ret_type==LOG_TIME)
        strftime(tBUF,sizeof(tBUF),"[%Y-%m-%d %H:%M:%S]",tm);
}
int rmdirs(const char *path, int force)
{
    DIR *  dir_ptr      = NULL;
    struct dirent *file = NULL;
    struct stat   buf;
    char   filename[1024];

    /* 목록을 읽을 디렉토리명으로 DIR *를 return 받습니다. */
    if((dir_ptr = opendir(path)) == NULL) {
		/* path가 디렉토리가 아니라면 삭제하고 종료합니다. */
		return unlink(path);
    }

    /* 디렉토리의 처음부터 파일 또는 디렉토리명을 순서대로 한개씩 읽습니다. */
    while((file = readdir(dir_ptr)) != NULL) {
        // readdir 읽혀진 파일명 중에 현재 디렉토리를 나타네는 . 도 포함되어 있으므로 
        // 무한 반복에 빠지지 않으려면 파일명이 . 이면 skip 해야 함
        if(strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
             continue;
        }

        sprintf(filename, "%s/%s", path, file->d_name);

        /* 파일의 속성(파일의 유형, 크기, 생성/변경 시간 등을 얻기 위하여 */
        if(lstat(filename, &buf) == -1) {
            continue;
        }

        if(S_ISDIR(buf.st_mode)) { // 검색된 이름의 속성이 디렉토리이면
            /* 검색된 파일이 directory이면 재귀호출로 하위 디렉토리를 다시 검색 */
            if(rmdirs(filename, force) == -1 && !force) {
                return -1;
            }
        } else if(S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) { // 일반파일 또는 symbolic link 이면
            if(unlink(filename) == -1 && !force) {
                return -1;
            }
        }
    }

    /* open된 directory 정보를 close 합니다. */
    closedir(dir_ptr);
    
    return rmdir(path);
}
void scandirr(void){
    struct dirent **namelist;
    int count;
    int idx;

    if((count=scandir(path,&namelist,*filter,alphasort))==-1){
        exit(1);
    }
    sprintf(rmdirName,"%s",namelist[0]->d_name);
    printf("rmdirName=%s",rmdirName);
    for(idx=0;idx<count;idx++){
        free(namelist[idx]);
    }
    free(namelist);
}
int main(int , char ** ){

    VideoCapture cap;
    VideoWriter writer;
    int deviceId=0;
    int apiId=cv::CAP_V4L2;
    int exitflag =0;
    int Maxframe=1780;
    int framecount;
    int fd;
    int WRByte;
    char buff[200];
    char filepath[100];
    Mat frame;
    MOUNTP *MP;
    if((MP=dfopen())==NULL){
        perror("error");
        return 1;
    }

    fd=open("/home/pi/blackbox/blackbox.log",O_WRONLY | O_CREAT | O_TRUNC,0644);
    getTime(LOG_TIME);
    sprintf(buff,"%s blackbox log파일 저장을 시작합니다\n",tBUF);
    WRByte=write(fd,buff,strlen(buff));
    cap.open(deviceId,apiId);
    if(!cap.isOpened()){
        perror("open() err");
        return -1;
    }
    cap.set(CAP_PROP_FPS,30);
    cap.set(CAP_PROP_FRAME_WIDTH,320);
    cap.set(CAP_PROP_FRAME_HEIGHT,240);

    float videoFPS = cap.get(CAP_PROP_FPS);
    int videoWidth=cap.get(CAP_PROP_FRAME_WIDTH);
    int videoHeight=cap.get(CAP_PROP_FRAME_HEIGHT);

    //인자값 1. 파일명 2. 코덱 지정. 3.FPS , 4 ImageSize, 5.
    
    while(1){
        getTime(FOLDER_NAME);
        getTime(TIME_FILENAME);
        printf("FILENAME:%s\n",tBUF);
        int sig;
        sig=mkdir(fBUF,0700);
        sprintf(filepath,"/home/pi/blackbox/%s/%s",fBUF,tBUF);
        //카메라에서 매 프레임마다 이미지 읽기
        //check if we succeded;
        writer.open(filepath,VideoWriter::fourcc('D','I','V','X'),videoFPS,
        Size(videoWidth,videoHeight),true);
        getTime(LOG_TIME);
        if(sig==0){
            sprintf(buff,"%s %s명으로 폴더가 생성되었습니다.\n",tBUF,fBUF);
            WRByte=write(fd,buff,strlen(buff));
        }
        sprintf(buff,"%s %s 명으로 녹화를 시작합니다.\n",tBUF,filepath);
        WRByte=write(fd,buff,strlen(buff));
        if(!writer.isOpened()){
            perror("Cant write video");
            return -1;
        }
        
        framecount=0;
        namedWindow(VIDEO_WINDOW_NAME);

        while(framecount<Maxframe){
            cap.read(frame);
            framecount++;
            if(frame.empty()){
                perror("ERROR! frame grabbed\n");
                break;
            }
        
            imshow(VIDEO_WINDOW_NAME, frame);
            writer << frame;
            if(waitKey(1000/videoFPS)==27){
                sprintf(buff,"%s 녹화가 종료되었습니다.\n",tBUF);
                WRByte=write(fd,buff,strlen(buff));
                printf("Stop video record\n");
                exitflag=1;
                break;
            }
        }
        
        writer.release();
        if(exitflag==1)
            break;       
        while(dfget(MP)){
            if((MP->size.ratio)<60.0){
                printf(" now ratio=%f\n",MP->size.ratio);
                scandirr();
                rmdirs(rmdirName,1);
                sprintf(buff,"%s %s명의 폴더가 삭제되었습니다.\n",tBUF,rmdirName);
                WRByte=write(fd,buff,strlen(buff));
            }
        }
    }
    
    cap.release();
    close(fd);
    destroyWindow(VIDEO_WINDOW_NAME);
    return 0;
}
