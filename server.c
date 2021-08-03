#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "connection.h"
int
main(int argc, char *argv[])
{
    struct sockaddr_un name;
    int down_flag = 0;
    int ret;
    int connection_socket;
    int data_socket;
    int result;
    char buffer[BUFFER_SIZE];
    /* Create local socket. */
    connection_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (connection_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    /*
     * For portability clear the whole structure, since some
     * implementations have additional (nonstandard) fields in
     * the structure.
     */
    memset(&name, 0, sizeof(name));
    /* Bind socket to socket name. */
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);
    /* search: bind address already in use "sys/socket.h"
       https://stackoverflow.com/questions/49604087/what-do-i-change-in-this-code-to-not-get-bind-address-already-in-use-perror/49604176#49604176
       https://stackoverflow.com/questions/17451971/getting-address-already-in-use-error-using-unix-socket
    */
    unlink(SOCKET_NAME);
    ret = bind(connection_socket, (const struct sockaddr *) &name,
               sizeof(name));
    if (ret == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    /*
     * Prepare for accepting connections. The backlog size is set
     * to 20. So while one request is being processed other requests
     * can be waiting.
     */
    ret = listen(connection_socket, 20);
    if (ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    /* This is the main loop for handling connections. */
    for (;;) {
        /* Wait for incoming connection. */
        data_socket = accept(connection_socket, NULL, NULL);
        if (data_socket == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        result = 0;
        for (;;) {
            /* Wait for next data packet. */
            ret = read(data_socket, buffer, sizeof(buffer));
            if (ret == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            puts(buffer);
            /* Ensure buffer is 0-terminated. */
            buffer[sizeof(buffer) - 1] = 0;
            /* Handle commands. */
            if (!strncmp(buffer, "DOWN", sizeof(buffer))) {
                down_flag = 1;
                break;
            }
            if (!strncmp(buffer, "END", sizeof(buffer))) {
                break;
            }
            /* Add received summand. */
            result += atoi(buffer);
        }
        /* Send result. */
        sprintf(buffer, "%d", result);
        ret = write(data_socket, buffer, sizeof(buffer));
        if (ret == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        /* Close socket. */
        close(data_socket);
        /* Quit on DOWN command. */
        if (down_flag) {
            break;
        }
    }
    close(connection_socket);
    /* Unlink the socket. */
    unlink(SOCKET_NAME);
    exit(EXIT_SUCCESS);
}