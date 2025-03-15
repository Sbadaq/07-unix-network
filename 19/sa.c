#include <sys/socket.h>
#include <linux/pfkeyv2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

int main() {
    int key_socket = socket(PF_KEY, SOCK_RAW, PF_KEY_V2);
    if (key_socket < 0) {
        perror("socket");
        exit(1);
    }

    struct sadb_msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.sadb_msg_version = PF_KEY_V2;
    msg.sadb_msg_type = SADB_ADD;
    msg.sadb_msg_len = sizeof(msg) / sizeof(uint64_t);
    msg.sadb_msg_seq = 1;
    msg.sadb_msg_pid = getpid();

    sendmsg(key_socket, &msg, 0);
    close(key_socket);
    return 0;
}
