# UNIX Socket Communication (Client-Server)

This project demonstrates a chat program based on the client-server model. Both run on the same computer. 

## Table of Contents
0. [Overview](#overview)
1. [How It Works](#how-it-works)
2. [How to Use](#how-to-use)
---

## Overview
This program uses **datagram (SOCK_DGRAM)** UNIX domain sockets for local communication. The client sends messages to the server, and the server responds back. Both processes run on the same computer and communicate through socket files.
---

## How It Works

### Communication Flow:
1. The **server** creates a socket and binds it to a specific path (e.g., `./server_socket`).
2. The **client** creates its own socket and binds to its path (e.g., `./client_socket`).
3. The **client** sends a message to the server via the server's socket path.
4. The **server** receives the message, processes it and sends the response to the client's socket.
5. The **client** receives the response and displays it.

### Threaded Design:
- **Client**:
  - **Send Thread**: Reads user input and sends messages to the server.
  - **Receive Thread**: Waits for messages from the server and displays them.

- **Server**:
  - **Command handle**: Reads user input command (`exit`)
  - **Receive Thread**: Waits for messages from the client and displays them then responds to client.
- The threads run concurrently, ensuring smooth operation of both sending and receiving messages.
---

## How to use
0. build using `make`
1. Run server first.  `./server`
2. Run client.        `./client`
3. Try typing message then hit Enter to send.
4. Server received the message and responds to client.
5. Type `exit` to exit program.
