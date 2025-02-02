#include  <string.h>
#include  <unistd.h>
#include  <sys/types.h>
#include <pthread.h>

int main()
{
    pid_t pid1,pid2;
    int status;

    pid1 = fork();
    if (pid1 == 0){
        printf("I am child1, my pid is %d\n",getpid());
        sleep(10);
        printf("child1 is done\n");
        exit(0);
    }else if(pid1 > 0){
        pid2 = fork();
        if(pid2 == 0){
            printf("I am child2, my pid is %d\n",getpid());
            sleep(10);
            printf("child2 is done\n");
            exit(0);
        }else if(pid2 > 0){
            printf("I am parent, my pid is %d\n",getpid());
            //printf("using wait to wait any child processes\n");
            //wait(&status);
            printf("using waitpid to wait child1\n");
            pid_t wpid = waitpid(pid1, &status, 0);
            printf("waitpid wait pid: %d\n",wpid);
        }else{
            printf("fork 2 error\n");
        }
    }else{
        printf("fork 1 error\n");
    }
}