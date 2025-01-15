#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>  
#include <termios.h>
#include <fcntl.h>  
#include <errno.h>
#include <pthread.h>
#include<stdlib.h>
#include<sys/select.h>
#define FALSE  -1    
#define TRUE   0
#define UART_INPUT_TTL_LEN 8
int fd ;

struct parameter {
    char year[20];       // 年月日
    char time[20];        // 时分秒
    float grade;        // 距离
    int battery;        //电池电量
};

struct parameter data_references = {0};

int UART4_Set(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity) {

    int i;
    int status;
    int speed_arr[] = {B115200, B19200, B9600, B4800, B2400, B1200, B300};
    int name_arr[] = {115200, 19200, 9600, 4800, 2400, 1200, 300};

    struct termios options;
     if (tcgetattr(fd, &options) != 0) {
//        perror("SetupSerial 1");
#ifndef USING_GLOG_PRINT_FILE
        printf("SetupSerial ERROR\n");
#else
        printf("SetupSerial ERROR\n");
#endif
        return (FALSE);
    }

    //设置串口输入波特率和输出波特率      
    for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
        if (speed == name_arr[i]) {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
        }
    }
     //修改控制模式，保证程序不会占用串口    
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据      
    options.c_cflag |= CREAD;

    //设置数据流控制    
    switch (flow_ctrl) {

        case 0 ://不使用流控制    
            options.c_cflag &= ~CRTSCTS;
            break;

        case 1 ://使用硬件流控制    
            options.c_cflag |= CRTSCTS;
            break;
        case 2 ://使用软件流控制     
            options.c_cflag |= IXON | IXOFF | IXANY;
            break;
    }
    //设置数据位    
    //屏蔽其他标志位    
    options.c_cflag &= ~CSIZE;
    switch (databits) {
        case 5:
            options.c_cflag |= CS5;
            break;
        case 6    :
            options.c_cflag |= CS6;
            break;
        case 7    :
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
#ifndef USING_GLOG_PRINT_FILE
        printf("Unsupported data size!\n") ;
#else
        printf("Unsupported data size!\n") ;
#endif
//          fprintf(stderr, "Unsupported data size\n");
            return (FALSE);
    }
        switch (parity) {
        case 'n':
        case 'N': //无奇偶校验位。     
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 'o':
        case 'O'://设置为奇校验        
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;
        case 'e':
        case 'E'://设置为偶校验     
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        case 's':
        case 'S': //设置为空格      
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
            default:
#ifndef USING_GLOG_PRINT_FILE
            printf("Unsupported parity!\n") ;
#else
             printf(""Unsupported parity"!\n") ;
#endif
//            fprintf(stderr, "Unsupported parity\n");
            return (FALSE);
    }
        switch (stopbits) {
            case 1:
                options.c_cflag &= ~CSTOPB;
                break;
            case 2:
                options.c_cflag |= CSTOPB;
                break;
            default:
#ifndef USING_GLOG_PRINT_FILE
        printf("Unsupported stop bits\n") ;
#else
        printf("Unsupported stop bits\n") ;
#endif
//            fprintf(stderr, "Unsupported stop bits\n");
            return (FALSE);
    }

    //修改输出模式，原始数据输出     
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(ICRNL | IGNCR | IXON | IXOFF);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //options.c_lflag &= ~(ISIG | ICANON);    

    //设置等待时间和最小接收字符     
    options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */
    options.c_cc[VMIN] = 0; /* 读取字符的最少个数为1 */

    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读   
    tcflush(fd, TCIFLUSH);

    //激活配置 (将修改后的termios数据设置到串口中）    
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
//        perror("com set error!\n");
#ifndef USING_GLOG_PRINT_FILE
        printf("com set error!\n");
#else
        printf("com set error!\n") ;
#endif
        return (FALSE);
    }
    return (TRUE);
}


int UART4_Init(int fd, int speed, int flow_ctrl, int databits, int stopbits, int parity) {
    int err;
    //设置串口数据帧格式     
    if (UART4_Set(fd, speed, flow_ctrl, databits, stopbits, parity) == FALSE) {
        return FALSE;
    } else {
        return TRUE;
    }
}
int UART4_Recv(int fd, char *rcv_buf, int data_len) {
    int len, fs_sel;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);

    time.tv_sec = 10;
    time.tv_usec = 0;

    len = read(fd, rcv_buf, data_len);
    //printf("len = %d\n", len);
    return len;
}

int hex_to_decimal(const char *hex_string) {
    // 使用 strtol 函数将十六进制字符串转换为十进制整数
    int decimal_value = strtol(hex_string, NULL, 16);
    return decimal_value;
}


void Encode_Data_Analyzing(char *recv_buf)
{
    switch(recv_buf[3])
    {
        case 0x01:
            printf("02 b3 = %02X\n",recv_buf[4]);
            break;
        case 0x03:
            printf("02 b3 = %02X\n",recv_buf[4]);
            break;
        case 0x04:
            printf("02 b3 = %02X\n",recv_buf[4]);
            break;
        default:
            break;
    }

}

void Menu_Data_Analyzing(char *recv_buf)
{
    switch(recv_buf[4])
    {
        case 0x01:
            printf("02 b4 = %02X\n",recv_buf[4]);
            break;

        case 0x02:
            printf("02 b4 = %02X\n",recv_buf[4]);
            break;
        default:
            break;
    }

}
void Rng_Data_Analyzing(char *recv_buf)
{
    switch(recv_buf[5])
    {
        case 0x01://无用，测距按钮，客户这边操作，只是发出来
            printf("02 b5 = %02X\n",recv_buf[5]);
            break;
        default:
            break;
    }

}
void Distance_Analyzing(char *recv_buf)
{
    char result[20] = {0};
    sprintf(result, "%02X%02X", recv_buf[3],recv_buf[4]);
    int Integer_number = hex_to_decimal(result);
    memset(result, 0, 20);
    sprintf(result, "%02X", recv_buf[5]);
    int decimal_fraction = hex_to_decimal(&result);
    memset(result, 0, 20);
    sprintf(result, "%d.%d", Integer_number, decimal_fraction);
    data_references.grade = atof(result);
    printf("测距值: %.1f\n", data_references.grade);
}

void Date_Analyzing(char *recv_buf)
{
    char data[20] = {0};
    sprintf(data, "%02d/%02d/%02d", recv_buf[3],recv_buf[4],recv_buf[5]);
    strcpy(data_references.year, data);
    //printf("日期: %s\n", data_references.year);
}

void Time_Analyzing(char *recv_buf)
{
    char time[20] = {0};
    sprintf(time, "%02d:%02d:%02d", recv_buf[3],recv_buf[4],recv_buf[5]);
    strcpy(data_references.time, time);
    //printf("时间: %s\n", data_references.time);
}
void Battery_Analyzing(char *recv_buf)
{
    char result[20] = {0};
    sprintf(result, "%02X", recv_buf[3]);
    int Integer_number = hex_to_decimal(result);
    data_references.battery = Integer_number;
    //printf("电量: %d%%\n", data_references.battery);

}
void State_judgment(char *recv_buf)
{
    switch(recv_buf[6])
    {
         case 0x01:
            Distance_Analyzing(recv_buf);
            break;

        case 0x02:
            // printf("超程\n");
            data_references.grade= 0;
            // printf("测距值: %d\n", (int)data_references.grade);
            break;
        case 0x03:
            data_references.grade= 0;
            // printf("欠光\n");
            // printf("测距值: %d\n", (int)data_references.grade);
            break;
            default:
            break;


    }


}
void Data_Analyzing(char *recv_buf)
{

    switch(recv_buf[2])
    {
        case 0x01:
            State_judgment(recv_buf);
            break;
        case 0x02:
            Encode_Data_Analyzing(recv_buf);
            Menu_Data_Analyzing(recv_buf);
            Rng_Data_Analyzing(recv_buf);
            break;
        case 0x03:
            Battery_Analyzing(recv_buf);
            break;

        case 0x04:
            Date_Analyzing(recv_buf);
            break;
            
        case 0x05:
            Time_Analyzing(recv_buf);
            break;

        default:
            break;


    }


}
void *receiveData(void) {
    const int buffer_size = UART_INPUT_TTL_LEN;
    char recv_buf[buffer_size];  // 接收缓冲区
    int len;
    //printf("fd=%d\n",fd);
    while (1) {
        // 清空接收缓冲区
        memset(recv_buf, 0, buffer_size);
        // 接收数据
        len = UART4_Recv(fd, recv_buf, buffer_size);
        // 打印每次接收到的数据s
        if (len > 0) {
            if(recv_buf[0] != 0xAA && recv_buf[1] != 0x55 &&  recv_buf[6] != 0x00)
            {
                memset(recv_buf, 0, buffer_size);
                continue;
            }
            unsigned char sun = recv_buf[0]+recv_buf[1]+recv_buf[2]+recv_buf[3]+recv_buf[4]+recv_buf[5]+recv_buf[6];
            // printf("sun = %02X\n",sun);
            // printf("B7 = %02X\n",recv_buf[7]);
            if(sun != recv_buf[7])
            {
                memset(recv_buf, 0, buffer_size);
                continue;
            }
            printf("Raw data received:"); 
            for (int i = 0; i < len; ++i) {
                printf("%02X ", (unsigned char)recv_buf[i]);
            }
            printf("\n");
            Data_Analyzing(recv_buf);
            memset(recv_buf, 0, buffer_size);
        } 
        else {
            sleep(1);
            }
        

    }
}
int open_port(void)
{
    fd = open("/dev/ttyAMA4", O_RDWR|O_NOCTTY|O_NDELAY);
    if (-1 == fd)
    {
        perror("Open:");
        return(-1);
    }
    /*恢复串口为阻塞状态*/
    if(fcntl(fd, F_SETFL, 0) <0)
    {
        perror("fcntl");
        //return -1;
    }

    /*测试是否为终端设备*/
    if(isatty(STDIN_FILENO) == 0)
    {
        perror("isatty");
        return -1;
    }
    return fd;
}
int main()
{
    pthread_t tid;
    open_port();
    if (fd < 0)
    {
        perror("Can't Open Serial Port");
    }
    printf("fd= %d \n", fd);
    if (UART4_Init(fd, 115200, 0, 8, 1, 'n') == FALSE) {
        printf("Failed to initialize UART\n");
        return -1;
    }
    int res = pthread_create(&tid,NULL,receiveData,NULL);
    if(res != 0)
    {
        printf("pthread_create fail\n");
        return -1 ;
    }
    sleep(1);
    pthread_join(tid,NULL);
    close(fd);
    return 0;

}

