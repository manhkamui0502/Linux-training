#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

// Server socket
#define SERVER_SOCKET_PATH  "./server_socket"

// Global variables
int socket_fd;                    // File descriptor for the server socket
struct sockaddr_un server_addr;   // Server socket address
struct sockaddr_un client_addr;   // Client socket address
pthread_t receive_thread;         // Thread for receiving messages
pthread_t command_thread;         // Thread for handling commands
int running = 1;                  // Flag to control server execution

// Function to receive messages from the client and send responses
void receive_messages(void *arg) {
  (void *)&arg;
  char recv_buffer[256], res_buffer[256];
  socklen_t len;
  ssize_t recv_num, sent_num;

  // Set thread cancel state and type
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  while (running) {
    len = sizeof(struct sockaddr_un);
    // Receive message from client
    recv_num = recvfrom(socket_fd, recv_buffer, sizeof(recv_buffer) - 1, 0, (struct sockaddr *)&client_addr, &len);
    if (recv_num > 0) {
      recv_buffer[recv_num] = '\0'; // Null-terminate the received message
      printf("\nClient: %s\n", recv_buffer);

      // Prepare and send a response to the client
      snprintf(res_buffer, sizeof(res_buffer), "Received %s", recv_buffer);
      sent_num = sendto(socket_fd, res_buffer, strlen(res_buffer), 0, (struct sockaddr *)&client_addr, len);
      if (sent_num == -1) {
        perror("sendto() error\n");
      } else {
        printf("* Responsed to client *\n");
      }
    } else if (recv_num == -1) {
      perror("recvfrom() error\n");
    }
  }
  pthread_exit(NULL);
}

void handle_commands(void *arg) {
  (void *)&arg;
  char command[256];
  while (running) {
    // Get server operator input
    printf("Enter command ('exit' to quit): ");
    if (fgets(command, sizeof(command), stdin) == NULL) {
      perror("fgets() error");
      continue;
    }

    // Remove \n character from input
    size_t input_len = strlen(command);
    if (input_len > 0 && command[input_len - 1] == '\n') {
      command[input_len - 1] = '\0';
    }

    // Check for exit condition
    if (strcmp(command, "exit") == 0) {
      printf("Exiting server...\n");
      running = 0; // Update status flag
      // Inform the client that the server is shutting down
      sendto(socket_fd, 
             "Server shutting down.", 
             strlen("Server shutting down."), 
             0, 
             (struct sockaddr *)&client_addr, 
             sizeof(struct sockaddr_un));
      pthread_cancel(receive_thread); // Cancel receive message thread
      break;
    } else {
      printf("Unrecognized command: %s\n", command);
    }
  }
  pthread_exit(NULL);
}

int main() {
  // Remove any existing socket file to avoid conflicts
  remove(SERVER_SOCKET_PATH);

  // Create a socket
  socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (socket_fd == -1) {
    perror("socket() error");
    return 1;
  }

  // Configure server address
  memset(&server_addr, 0, sizeof(struct sockaddr_un));
  server_addr.sun_family = AF_UNIX;
  strncpy(server_addr.sun_path, SERVER_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

  // Bind the socket to the server address
  if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
    perror("bind() error");
    close(socket_fd);
    return 1;
  }

  printf("Server started...\n");

  // Create threads
  if (pthread_create(&receive_thread, NULL, (void *)receive_messages, NULL) != 0) {
    perror("pthread_create() error for receive thread");
    close(socket_fd);
    return 1;
  }

  if (pthread_create(&command_thread, NULL, (void *)handle_commands, NULL) != 0) {
    perror("pthread_create() error for command thread");
    close(socket_fd);
    return 1;
  }

  // Wait for threads to finish
  pthread_join(command_thread, NULL);
  pthread_join(receive_thread, NULL);

  // Clean up
  close(socket_fd);
  remove(SERVER_SOCKET_PATH);
  printf("Server shut down.\n");

  return 0;
}
