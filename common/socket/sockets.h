#ifndef MIDTERM_TICTACTOE_SOCKETS_H
#define MIDTERM_TICTACTOE_SOCKETS_H

// Send an int to the specified socket
bool SendInt(int socket, int number);

// Blocks until it receives an int
bool ReceiveInt(int socket, int * number);

// Send a string to the specified socket
bool SendString(int socket, char * message);

// Blocks until it receives an string
bool ReceiveString(int socket, char * message);

#endif //MIDTERM_TICTACTOE_SOCKETS_H
