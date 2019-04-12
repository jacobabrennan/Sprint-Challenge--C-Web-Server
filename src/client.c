#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "lib.h"

#define BUFSIZE 4096 // max number of bytes we can get at once

/**
 * Struct to hold all three pieces of a URL
 */
typedef struct urlinfo_t {
  char *hostname;
  char *port;
  char *path;
} urlinfo_t;

/**
 * Tokenize the given URL into hostname, path, and port.
 *
 * url: The input URL to parse.
 *
 * Store hostname, path, and port in a urlinfo_t struct and return the struct.
*/
urlinfo_t *parse_url(char *url)
{
    // Allocate Memory
    urlinfo_t *urlinfo = malloc(sizeof(urlinfo_t));
    urlinfo->hostname = malloc(1024);
    urlinfo->port = malloc(16);
    urlinfo->path = malloc(1024);
    // Strip the protocol
    char *protocol_signifier = strstr(url, "://");
    if(NULL != protocol_signifier)
    {
        url = protocol_signifier + 3;
    }
    // Determine position of port and path, handle errors
    char *index_port = strchr(url, ':');
    char *index_path = strchr(index_port, '/');
    if(NULL == index_port || NULL == index_path)
    {
        printf("Urls must include both a port and a path");
        exit(1);
    }
    // Populate struct from string
    strncpy(urlinfo->hostname, url, index_port - url);
    strncpy(urlinfo->port, index_port+1, (index_path - index_port) -1);
    urlinfo->path = strdup(index_path+1);
    // Return resultant struct
    return urlinfo;
}

/**
 * Constructs and sends an HTTP request
 *
 * fd:       The file descriptor of the connection.
 * hostname: The hostname string.
 * port:     The port string.
 * path:     The path string.
 *
 * Return the value from the send() function.
*/
int send_request(int fd, char *hostname, char *port, char *path)
{
    // Initialize variables
    const int max_request_size = 16384;
    char request[max_request_size];
    int rv;
    // Construct Request
    char *HTTP_GET_FORMAT =
        "GET /%s HTTP/1.1\n"
        "Host: %s:%s\n"
        "Connection: close\n";
    rv = sprintf(request, HTTP_GET_FORMAT, path, hostname, port);
    // Handle write errors
    if(rv < 0)
    {
        printf("A problem was encountered writing to the buffer.\n");
        exit(2);
    }
    // Send request
    return (int) send(fd, request, rv, 0);
}

int main(int argc, char *argv[])
{  
    int sockfd, numbytes;  
    char buf[BUFSIZE];

    if (argc != 2) {
        fprintf(stderr,"usage: client HOSTNAME:PORT/PATH\n");
        exit(1);
    }

    // Parse the input URL
    urlinfo_t *url_info = parse_url(argv[1]);
    // Initialize a socket by calling the `get_socket` function from lib.c
    int socket_id = get_socket(url_info->hostname, url_info->port);
    // Call `send_request` to construct the request and send it
    send_request(socket_id, url_info->hostname, url_info->port, url_info->path);
    // Call `recv` in a loop until there is no more data to receive from the server. Print the received response to stdout.
    // Clean up any allocated memory and open file descriptors.

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    return 0;
}
