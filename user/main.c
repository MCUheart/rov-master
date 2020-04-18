
/**
 * @desc: 主程序
 */
#define LOG_TAG "main"

#include "config.h"
#include "init.h"

#include <elog.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <wiringPi.h>

/*
static void signal_handler(int sig)
{
	// ctrl + c程序中断回调
	printf("\n interrupt Key\n");


	exit(0);
}*/

// TODO 是否需要使用fork函数
/*
void *mjpg_streamer_thread(void *arg)
{

	system("sudo chmod +x video.sh"); // 使 video.sh 脚本可执行

	system("./video.sh &"); // 打开视频推流脚本
	return NULL;
}
*/
int main(int argc, char **argv)
{
    pthread_t mjpg_tid;

    system_init();

    /* register signal handler for <CTRL>+C in order to clean up */
    /*if(signal(SIGINT, signal_handler) == SIG_ERR) 
	{
		printf("123\n");
	}*/

    //pthread_create(&mjpg_tid, NULL, mjpg_streamer_thread, NULL);
    //pthread_detach(mjpg_tid);
    while (1)
    {
        sleep(1);
    }
    return 0;
}
