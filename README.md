## Run

```
$ gcc server.c -o build/server
$ gcc client.c -o build/client
$ ./build/server
# another terminal
$ ./build/client hello
```

## Links

- https://en.m.wikipedia.org/wiki/Unix_domain_socket#cite_ref-man-unix-sockets_1-0
- connection.h, server.c, client.c from https://man7.org/linux/man-pages/man7/unix.7.html