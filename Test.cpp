// OpenCV를 사용하여 동영상 녹화하기
// API설명 참조 사이트 : https://opencvlib.weebly.com/
// 오지우 과제 제출 
// 멀티프로세스, 멀티스레드 구현 x 
// 폴더 생성 및 용량 표시 삭제 구현까지 로그 구현 완료. 

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>   // O_WRONLY
#include <unistd.h>  // write() ,read()
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h> 
#include <errno.h> 
#include <time.h>
#include <sys/time.h>

using namespace cv;
using namespace std;

#define VIDEO_WINDOW_NAME "record"
#define TIME_FILENAME 0
#define FOLDER_NAME   1
#define LOG_TIME      2
#define MAX_LIST 50 

char fileName[30];
char tBUF[100];
char fBUF[100];
char buf[BUFSIZ];
char rmdirName[100];
char dirName[100];

// 현재경로를 가리키는 path

const char *path = "/home/pi/blackbox"; 

const char *MMOUNT = "/proc/mounts";

/* ".", ".." 은 빼고 나머지 파일명 출력하는 필터 함수 */
static int filter(const struct dirent *dirent)
{
    if((dirent->d_name)[0]!='2'){
        return 0;
    }
    else
        return 1;
}

void getTime(int ret_type)
{
    time_t UTCtime;
    struct tm *tm;
    // 사용자 문자열로 시간정보를 저장하기 위한 문자열 버퍼

    // 커널에서 시간 정보를 읽어서
    // UTCtime변수에 넣어준다.
    time(&UTCtime); // UTC 현재 시간 읽어오기
    //printf("time : %u\n", (unsigned)UTCtime); // UTC 현재 시간 출력

    tm = localtime(&UTCtime);
    //printf("asctime : %s", asctime(tm)); // 현재의 시간을 tm 구조체를 이용해서 출력

    // 1st : 우리가 만들 문자열 저장할 버퍼
    // 2nd : 버퍼 사이즈
    // 3rd : %a : 간단한 요일, %m :월, %e : 일, %H : 24시, %M :분, %S :초, %Y :년
    //strftime(buf,sizeof(buf),"%a %m %e %H:%M:%S %Y", tm); // 사용자 정의 문자열 지정
    if (ret_type==TIME_FILENAME)
        strftime(tBUF,sizeof(tBUF),"%Y%m%d%H%M%S.avi", tm);
    else if(ret_type==FOLDER_NAME)
        strftime(fBUF,sizeof(fBUF),"%Y%m%d%H", tm);
    else if(ret_type==LOG_TIME)
        strftime(tBUF,sizeof(tBUF),"[%Y-%m-%d %H:%M:%S]", tm);
    //printf("strftime: %s\n",buf);
}

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
         if(lstat(filename, &buf) == -1) { 
             continue; 
             } 
        if(S_ISDIR(buf.st_mode)) { 
            // 검색된 이름의 속성이 디렉토리이면 
            /* 검색된 파일이 directory이면 재귀호출로 하위 디렉토리를 다시 검색 */ 
            if(rmdirs(filename, force) == -1 && !force) { 
                return -1; 
                } 
             } else if(S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) { 
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

struct f_size
{
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


int AvailableMemory() {
    int sizeRatio;
    MOUNTP *MP;
    if ((MP=dfopen()) == NULL)
    {
        perror("error");
        return 1;
    }

    // while(1)
    // {
    while(dfget(MP))
    {
        printf("Available Memory = %5f\n", MP->size.ratio);
    }
    sizeRatio = MP->size.ratio;
    printf("sizeRatio=%d\n",sizeRatio);
    sleep(1);
    return sizeRatio;
    // }
}




long long searchOldFolder(void) 
{ 
    char rmdirName[100];
    struct dirent **namelist; 
    int count; 
    int idx; 
    long long min;
    long long num[MAX_LIST];

    // 1st : 내가 탐색하고자 하는 폴더
    // 2nd : namelist를 받아올 구조체 주소값
    // 3rd : filter
    // 4th : 알파벳 정렬
    // scandir()함수에서 namelist 메모리를 malloc
    if((count = scandir(path, &namelist, *filter, alphasort)) == -1) 
    { 
        // fprintf(stderr, "%s Directory Scan Error: %s\n", path, strerror(errno)); 
        return 1; 
    } 
    // printf("count=%d\n",count);    
    
    for(idx=0;idx<count;idx++)
    {
        num[idx] = atoll(namelist[idx]->d_name);
        
    }

    min = num[0];     //min 초기화

    for(idx = 0;idx<count;idx++)
    {
        if(num[idx] < min ) //num[idx]가 min보다 작다면
            min = num[idx]; //min 에는 num[idx]의 값이 들어감
    }

    // 건별 데이터 메모리 해제 
    for(idx = 0; idx < count; idx++) 
    { 
        free(namelist[idx]); 
    } 
    
    // namelist에 대한 메모리 해제 
    free(namelist); 

    return min;
}

int main(int, char**)
{
    // 1. 장치 작동 확인하기 
    system("lsusb");
    printf("\n");
    // 1. VideoCapture("동영상파일의 경로")
    //    VideoCapture(0)
    VideoCapture cap;
    VideoWriter writer;

    int deviceID = 0;
    int apiID = cv::CAP_V4L2;
    int exitFlag = 0;
    int MaxFrame = 1440;
    int frameCount;
    int fd;
    int WRByte;
    char buff[200];
    char filePath[100];
    char dirname[40];
    Mat frame;


    // 로그파일을 기록하기 위해 파일열기
    fd = open("/home/pi/blackbox/data/blackbox.log",O_WRONLY | O_CREAT | O_TRUNC, 0644);
    getTime(LOG_TIME);
    sprintf(buff, "%s blackbox log파일 저장을 시작합니다.\n",tBUF);
    printf("%s",buff);
    WRByte = write(fd, buff, strlen(buff));

    // STEP 1. 카메라 장치 열기 
    cap.open(deviceID, apiID);

    if (!cap.isOpened()) {
        perror("ERROR! Unable to open camera\n");
        return -1;
    }

    //  라즈베리파이 카메라의 해상도를 1280X720으로 변경 
    //cap.set(CAP_PROP_FRAME_WIDTH, 320);
    //cap.set(CAP_PROP_FRAME_HEIGHT, 240);
    cap.set(CAP_PROP_FPS,24);
    // Video Recording
    //  현재 카메라에서 초당 몇 프레임으로 출력하고 있는가?
    float videoFPS = cap.get(CAP_PROP_FPS);
    int videoWidth = cap.get(CAP_PROP_FRAME_WIDTH);
    int videoHeight = cap.get(CAP_PROP_FRAME_HEIGHT);

    printf("videoFPS=%f\n",videoFPS);
    printf("width=%d, height=%d\n",videoWidth, videoHeight);

    // 1st : 저장하고자 하는 파일명
    // 2nd : 코덱을 지정
    // 3rd : FPS
    // 4th : ImageSize,
    // 5th : isColor=True
    while(1)
    {
        // 시간정보를 읽어와서 파일명을 생성
        // 전역변수 fileName에 저장
        getTime(TIME_FILENAME);
        getTime(FOLDER_NAME);
        printf("FILENAME:%s\n",tBUF);
        // 2. 현재 시간으로 된 폴더를 생성한다. 
        
    
        int makeFolder;
        makeFolder=mkdir(fBUF,0700);
        sprintf(filePath, "/home/pi/blackbox/%s/%s",fBUF,tBUF);
        // 카메라에서 매 프레임마다 이미지 읽기 
        writer.open(filePath, VideoWriter::fourcc('D','I','V','X'),
        videoFPS, Size(videoWidth, videoHeight), true);
        getTime(LOG_TIME);
        if(makeFolder==0){
            sprintf(buff,"%s %s명으로 폴더가 생성되었습니다.\n",tBUF,fBUF);
            WRByte=write(fd,buff,strlen(buff));
        }
        sprintf(buff,"%s %s 명으로 녹화를 시작합니다.\n",tBUF,filePath);
        WRByte=write(fd,buff,strlen(buff));
        if(!writer.isOpened()){
            perror("Cant write video");
            return -1;
        }

        frameCount =0;
        namedWindow(VIDEO_WINDOW_NAME);
        
    // 3. 녹화를 시작하기 전에 디스크 용량을 확인한다. 
    // 용량이 부족하면 가장 오래된 폴더를 삭제한다. 
        if(AvailableMemory()<60)
        {
            sprintf(rmdirName,"%lld",searchOldFolder());
            printf("용량이 부족하여 가장 오래된 폴더 %s를 삭제합니다",rmdirName);
            rmdirs(rmdirName,1);
        }
        

        while(frameCount<MaxFrame)
        {
            // 카메라에서 매 프레임마다 이미지 읽기
            cap.read(frame);
            frameCount++;
            // check if we succeeded
            if (frame.empty()) {
                perror("ERROR! blank frame grabbed\n");
                break;
            }

            // 읽어온 한 장의 프레임을  writer에 쓰기
            writer << frame; // test.avi
            imshow(VIDEO_WINDOW_NAME, frame);

            // ESC=>27 'ESC' 키가 입력되면 종료 
            if(waitKey(10)==27)
            {
                printf("Stop video record\n");
                exitFlag = 1;
                break;
            }

        }
        writer.release();
        if(exitFlag==1)
            break;
    }
    cap.release();
    close(fd);
    destroyWindow(VIDEO_WINDOW_NAME);

    return 0;
}
