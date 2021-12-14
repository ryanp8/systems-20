#include "pipe_networking.h"


/*=========================
  server_handshake
  args: int * to_client
  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.
  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  int from_client = 0;

  int res = mkfifo(WKP, 0664);
  if (res == -1) printf("error %d: %s\n", errno, strerror(errno));
  else printf("created wkp\n");

  int fd = open(WKP, O_RDONLY);
  if (fd == -1) printf("error %d: %s\n", errno, strerror(errno));
  else printf("server connected to wkp\n");

  char handshake_buffer[HANDSHAKE_BUFFER_SIZE];
  read(fd, handshake_buffer, sizeof(handshake_buffer));
  printf("received handshake: [%s]\n", handshake_buffer);
  from_client = fd;
  remove("./"WKP);
  printf("removed "WKP"\n");
  close(fd);

  fd = open(handshake_buffer, O_WRONLY);
  if (fd == -1) printf("error %d: %s\n", errno, strerror(errno));
  else printf("server connected to %s\n", handshake_buffer);
  *to_client = fd;
  
  char syn_ack[HANDSHAKE_BUFFER_SIZE] = "private";
  write(fd, syn_ack, sizeof(syn_ack));
  printf("sent SYN_ACK('%s') to client on %s\n", syn_ack, handshake_buffer);
  close(fd);

  fd = open(handshake_buffer, O_RDONLY);
  char ack[HANDSHAKE_BUFFER_SIZE];
  read(fd, ack, sizeof(ack));
  printf("received [%s]\n", ack);
  close(fd);

  return from_client;
}


/*=========================
  client_handshake
  args: int * to_server
  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.
  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  int from_server = 0;
  char private_buf[HANDSHAKE_BUFFER_SIZE];
  sprintf(private_buf, "%d", getpid());

  int res = mkfifo(private_buf, 0664);
  if (res == -1) printf("error %d: %s\n", errno, strerror(errno));
  else printf("client created private %s pipe\n", private_buf);

  int fd = open(WKP, O_WRONLY);
  if (fd == -1) printf("error %d: %s\n", errno, strerror(errno));
  else printf("client connected to wkp\n");

  write(fd, private_buf, sizeof(private_buf));
  *to_server = fd;
  printf("sent [%s] to server\n", private_buf);
  close(fd);

  char handshake_buffer[HANDSHAKE_BUFFER_SIZE];
  fd = open(private_buf, O_RDONLY);
  if (fd == -1) printf("error %d: %s\n", errno, strerror(errno));
  else printf("client connected %s\n", private_buf);
  read(fd, handshake_buffer, sizeof(handshake_buffer));
  printf("received [%s]\n", handshake_buffer);
  from_server = fd;
  close(fd);

  fd = open(private_buf, O_WRONLY);
  write(fd, ACK, strlen(ACK));
  printf("writing ["ACK"] to server\n");
  close(fd);
  
  remove(private_buf);
  printf("removed %s\n", private_buf);

  
  return from_server;
}
