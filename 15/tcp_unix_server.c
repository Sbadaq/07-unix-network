#include <stdio.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#define MAXLINE 1024

void sigchld_handler(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <socket_path>\n", argv[0]);
        exit(1);
    }

    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_un cliaddr, servaddr;

    // Set up SIGCHLD handler to avoid zombie processes
    signal(SIGCHLD, sigchld_handler);

    // Create socket
    if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    // Unlink the socket path if it exists
    unlink(argv[1]);

    // Bind socket
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strncpy(servaddr.sun_path, argv[1], sizeof(servaddr.sun_path) - 1);
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind error");
        exit(1);
    }

    // Listen for connections
    if (listen(listenfd, 5) < 0) {
        perror("listen error");
        exit(1);
    }

    // Main loop
    for (;;) {
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
            perror("accept error");
            continue;
        }

        // Fork a child process
        if ((childpid = fork()) == 0) {
            close(listenfd);
            dup2(connfd, STDIN_FILENO);
            execlp("wc", "wc", "-l", NULL);
            perror("execlp error"); // If execlp fails
            exit(1);
        }
        close(connfd);
    }

    return 0;
}