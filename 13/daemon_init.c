#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syslog.h>
//把一个普通进程转化为守护进程，通常需要 2次 fork，并创建新会话
void daemonize(const char *path) {
    // 第一次 fork, 让父进程退出,目的是让子进程在后台运行，脱离终端
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // 父进程退出
    }
    //目前为止，子进程已经是会话的领头进程，脱离了终端，但它仍然继承了终端、进程组和会话

    // 第一次fork后，创建新会话，成为会话的领头进程，脱离终端和进程组
    //会话领导者是一个新会话的起点，跟过去的终端没有关系了
    //如果不做这一步，守护进程仍然可以打开终端，并且重新获得终端，此时关闭终端 会使应用程序崩溃（收到SIGHUP信号）
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
    //目前为止，子进程不再有控制终端，但它仍然可以打开终端，并且仍然可以重新获得终端

    // 第二次 fork，让父进程退出，目的是彻底脱离终端，彻底成为一个守护进程
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // 第一次 fork 的子进程退出
    }
    // 至此，守护进程彻底脱离了终端、会话和进程组,成为一个完全独立的进程。也不是会话的领头进程了。  
    // 关闭文件描述符
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // 重定向标准输入、输出、错误到 /dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);

    openlog(path, LOG_PID, 0);
}

int main() {
    static const char logfile_path[] = "/var/log/mydaemon.log";

    daemonize(logfile_path);
    // 守护进程的代码
    while (1) {
        sleep(10);
        // 执行守护进程的任务...
        // 例如，记录日志...
        // 这里使用 syslog 而不是直接写入文件，因为守护进程不应该直接操作文件
        // 但为了演示，我们在这里使用 syslog 模拟写入日志文件的行为
        // 注意：在实际的守护进程中，你应该使用 syslog 或其他日志记录机制来写入系统日志
        // 这里仅为演示，实际守护进程应避免直接写入文件
        // 例如：syslog(LOG_INFO, "Daemon is running\n");
        syslog(LOG_INFO, "Daemon is running\n");
    }
    return 0;
}