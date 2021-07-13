// OpenCV를 사용하여 동영상 녹화하기
// API설명 참조 사이트 : https://opencvlib.weebly.com/

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


#define MAX_LIST 50 

char buf[BUFSIZ];


void makefoldername(void)
{
    time_t UTCtime;
    struct tm *tm;
    // 사용자 문자열로 시간정보를 저장하기 위한 문자열 버퍼

    // 커널에서 시간정보를 읽어서
    //  UTC 변수에 넣어준다.
    time(&UTCtime); // UTC 현재시간 읽어오기 
    printf("time : %u\n",(unsigned)UTCtime); // UTC 현재 시간 출력

    tm = localtime(&UTCtime);
    
    // 1st : 우리가 만들 문자열 저장할 버퍼
    // 2nd : 버퍼 사이즈
    // 3rd : %a : 간단한 요일, %m :월, %e : 일, %H : 24시, %M :분, %S :초, %Y :년
    //strftime(buf,sizeof(buf),"%a %m %e %H:%M:%S %Y", tm); // 사용자 정의 문자열 지정
    strftime(buf,sizeof(buf),"%Y%m%d%H", tm); // 사용자 정의 문자열 지정
    //printf("strftime: %s\n",buf);
}

void makefileName(void)
{
    time_t UTCtime;
    struct tm *tm;
    // 사용자 문자열로 시간정보를 저장하기 위한 문자열 버퍼
    char buf[BUFSIZ];
    // 커널에서 시간정보를 읽어서
    //  UTC 변수에 넣어준다.
    time(&UTCtime); // UTC 현재시간 읽어오기 
    printf("time : %u\n",(unsigned)UTCtime); // UTC 현재 시간 출력

    tm = localtime(&UTCtime);
    
    // 1st : 우리가 만들 문자열 저장할 버퍼
    // 2nd : 버퍼 사이즈
    // 3rd : %a : 간단한 요일, %m :월, %e : 일, %H : 24시, %M :분, %S :초, %Y :년
    //strftime(buf,sizeof(buf),"%a %m %e %H:%M:%S %Y", tm); // 사용자 정의 문자열 지정
    strftime(buf,sizeof(buf),"%Y%m%d%H", tm); // 사용자 정의 문자열 지정
    //printf("strftime: %s\n",buf);
}

const char *MMOUNT = "/proc/mounts";

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


int dfclose(MOUNTP *MP)
{
    fclose(MP->fp);
}

using namespace cv;
using namespace std;

#define VIDEO_WINDOW_NAME "record"
char fileName[30];

char tBUF[100];

#define TIME_FILENAME 0
#define FOLDER_NAME   1
#define LOG_TIME      2


// 현재경로를 가리키는 path

const char *path = "/home/jwoh/blackbox/"; 


/* ".", ".." 은 빼고 나머지 파일명 출력하는 필터 함수 */
static int filter(const struct dirent *dirent)
{
  if(!(strcmp(dirent->d_name, ".")) ||
     !(strcmp(dirent->d_name, ".."))){
             
     /* 현재 디렉토리, 이전 디렉토리 표시는 출력안함 */
    }else{
    // printf("   %s() : %s\n", __FUNCTION__, dirent->d_name);
    }
}

long long searchOldFolder(void) 
{ 
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

int OldFolder()
{
    long long result; 
    char folderName[30];
    result = searchOldFolder();
    printf("result=%lld",result);
    return result;
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

    // 2. 현재 시간으로 된 폴더를 생성한다. 
    makefoldername();
    sprintf(dirname, "/home/pi/blackbox/data/%s",buf);
    printf("dirname=%s\n",dirname);
    printf("\n");
    mkdir(dirname,0755);

    // 3. 녹화를 시작하기 전에 디스크 용량을 확인한다. 
    // 용량이 부족하면 가장 오래된 폴더를 삭제한다. 
    if(AvailableMemory()<10)
    {
        remove(searchOldFolder());
    }

    // 로그파일을 기록하기 위해 파일열기
    fd = open("/home/pi/blackbox/blackbox.log",O_WRONLY | O_CREAT | O_TRUNC, 0644);
    getTime(LOG_TIME);
    sprintf(buff, "%s blackbox log파일 저장을 시작합니다.\n",tBUF);
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
        printf("FILENAME:%s\n",tBUF);
        sprintf(filePath, "/home/pi/blackbox/%s",tBUF);
        // D I V X = DivX MPEG-4 코덱으로 압축한다. 
        // writer = 
        writer.open(filePath, VideoWriter::fourcc('D','I','V','X'),
        videoFPS, Size(videoWidth, videoHeight), true);
        getTime(LOG_TIME);
        sprintf(buff, "%s %s 명으로 녹화를 시작합니다.\n",tBUF,filePath);
        // write = 
        WRByte = write(fd, buff, strlen(buff));

        if(AvailableMemory()<10){
            remove("OldFolder()");
        }

        if (!writer.isOpened())
        {
            perror("Can't write video");
            return -1;
        }
        frameCount =0;
        namedWindow(VIDEO_WINDOW_NAME);

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
