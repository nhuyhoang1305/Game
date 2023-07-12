//
// Created by devon on 5/7/15.
//

#include <unistd.h>
#include "sockets.h"
#include <iostream>

/**
 * Send out an int to the specified player
 *
 * @param socket    The socket to send the int to
 * @param number    The integer to send out
 *
 * @return  True if the socket is still open, false if the socket was disconnected
 */
bool SendInt(int socket, int number)
{
    char * data = (char*)&number;
    size_t left = sizeof(number);
    ssize_t rc;

    while (left)
    {
        rc = write(socket, data + sizeof(number) - left, left);
        if(rc <= 0) return false;
        left -= rc;
    }

    return true;
}

/**
 * Blocks until it receives an int
 *
 * @param socket    The socket to send the int to
 * @param number    The integer that was received
 *
 * @return  True if the socket is still open, false if the socket was disconnected
 */
bool ReceiveInt(int socket, int * number)
{
    int ret;
    char *data = (char*)&ret;
    size_t left = sizeof(*number);
    ssize_t rc;

    while (left) {
        rc = read(socket, data + sizeof(ret) - left, left);
        if (rc <= 0) return false;
        left -= rc;
    }

    *number = ret;
    return true;
}

/**
 * Send out an string to the specified player
 *
 * @param socket    The socket to send the string to
 * @param message    The string to send out
 *
 * @return  True if the socket is still open, false if the socket was disconnected
 */
bool SendString(int socket, char * message)
{
    char buffer[1024];
    int i = 0;
    for (i = 0; message[i] != '\0' && int(message[i]) != 127; ++i)
    {
        buffer[i] = message[i];
    }
    buffer[i] = '\0';

    if (write(socket, buffer, sizeof(buffer)) < 0)
    {
        return false;
    }
    
    return true;
}

/**
 * Blocks until it receives a string
 *
 * @param socket    The socket to send the string to
 * @param message    The string that was received
 *
 * @return  True if the socket is still open, false if the socket was disconnected
 */
bool ReceiveString(int socket, char * message)
{
    char ret[1024];
    ssize_t rc;

    rc = read(socket, ret, sizeof(ret));

    if (rc <= 0) return false;
    int i;
    for (i = 0; ret[i] != '\0'; ++i)
    {
        message[i] = ret[i];
    }
    message[i] = '\0';

    return true;
}