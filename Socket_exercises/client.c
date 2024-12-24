#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

// Server and client socket
#define SERVER_SOCKET_PATH "./server_socket"
#define CLIENT_SOCKET_PATH "./client_socket"

int socket_fd;                    // Socket file descriptor
struct sockaddr_un server_addr;   // Server socket address
struct sockaddr_un client_addr;   // Client socket address
pthread_t receive_thread;         // Thread for receiving messages
pthread_t send_thread;            // Thread for sending messages
int running = 1;                  // Flag to control the client's execution

// Function to receive message from server
void receive_messages(void *arg) {
  (void *)&arg;
  char recv_buffer[256];
  socklen_t len;
  ssize_t recv_num;

  // Set thread cancel state and type
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  while (running) {
    len = sizeof(struct sockaddr_un);
    // Receive a response from the server
    recv_num = recvfrom(socket_fd, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&client_addr, &len);
    if (recv_num > 0) {
      recv_buffer[recv_num] = '\0'; // Null-terminate the received message
      printf("\n* Server: %s\n", recv_buffer);
    } else if (recv_num == -1) {
      perror("recvfrom() error\n");
    }
  }
  pthread_exit(NULL);
}

void send_messages(void *arg) {
  (void *)&arg;
  socklen_t len;
  ssize_t sent_num;
  char message[256];

  while (running) {
    // Input string
    printf("\nWrite message here ('exit' to quit): ");
    if (fgets(message, sizeof(message), stdin) == NULL) {
      perror("fgets() error\n");
      break;
    }

    // Remove \n character from input
    size_t input_len = strlen(message);
    if (input_len > 0 && message[input_len - 1] == '\n') {
      message[input_len - 1] = '\0';
    }

    // Check for exit condition
    if (strcmp(message, "exit") == 0) {
      printf("Exiting...\n");
      running = 0;  // Update status flag
      pthread_cancel(receive_thread); // Cancel receive message thread
      break;
    }

    len = sizeof(struct sockaddr_un);
    // Send a message to the server
    sent_num = sendto(socket_fd, message, strlen(message), 0, (struct sockaddr *)&server_addr, len);
    if (sent_num == -1) {
      perror("sendto() error\n");
    } else {
      printf("Client: Sent %s\n", message);
    }
    // Timeout between 2 message is 1 seconds
    sleep(1);
  }
  pthread_exit(NULL);
}

int main() {
  // Remove any existing socket file to avoid conflicts
  remove(CLIENT_SOCKET_PATH);

  // Create a socket
  socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (socket_fd == -1) {
    perror("socket() error");
    return 1;
  }

  // Configure client address
  memset(&client_addr, 0, sizeof(struct sockaddr_un));
  client_addr.sun_family = AF_UNIX;
  strncpy(client_addr.sun_path, CLIENT_SOCKET_PATH, sizeof(client_addr.sun_path) - 1);

  // Bind the client socket
  if (bind(socket_fd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr_un)) == -1) {
    perror("bind() error");
    close(socket_fd);
    return 1;
  }

  // Configure server address
  memset(&server_addr, 0, sizeof(struct sockaddr_un));
  server_addr.sun_family = AF_UNIX;
  strncpy(server_addr.sun_path, SERVER_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

  // Create threads
  if (pthread_create(&receive_thread, NULL, (void *)receive_messages, NULL) != 0) {
    perror("pthread_create() error for receive thread");
    close(socket_fd);
    return 1;
  }

  if (pthread_create(&send_thread, NULL, (void *)send_messages, NULL) != 0) {
    perror("pthread_create() error for command thread");
    close(socket_fd);
    return 1;
  }

  // Wait for threads to finish
  pthread_join(send_thread, NULL);
  pthread_join(receive_thread, NULL);

  close(socket_fd);
  remove(CLIENT_SOCKET_PATH);

  return 0;
}
