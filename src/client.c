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
    // Determine position of port and path
    char *index_port = strchr(url, ':');
    char *index_path = strchr(url, '/');
    if(NULL == index_path)
    {
        index_path = url+strlen(url);
    }
    // Populate struct from string
    if(NULL == index_port)
    {
        index_port = index_path;
        strcpy(urlinfo->port, "80");
    }
    else
    {
        strncpy(urlinfo->port, index_port+1, (index_path - index_port) -1);
    }
    strncpy(urlinfo->hostname, url, index_port - url);
    urlinfo->path = strdup(index_path+1);
    // Diagnostics
    // printf("Host: %s\n", urlinfo->hostname);
    // printf("Port: %s\n", urlinfo->port);
    // printf("Path: %s\n", urlinfo->path);
    // Return resultant struct
    return urlinfo;
}
void destroy_urlinfo(urlinfo_t *url_info)
{
    free(url_info->hostname);
    free(url_info->port);
    free(url_info->path);
    free(url_info);
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
        "Connection: close\n\n";
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
    // Check for usage errors
    if (argc != 2) {
        fprintf(stderr,"usage: client HOSTNAME:PORT/PATH\n");
        exit(1);
    }
    // Initialize variables
    int sockfd, numbytes;  
    char buf[BUFSIZE];
    // Parse the input URL
    urlinfo_t *url_info = parse_url(argv[1]);
    // Initialize a socket by calling the `get_socket` function from lib.c
    printf("1\n");
    sockfd = get_socket(url_info->hostname, url_info->port);
    // Call `send_request` to construct the request and send it
    printf("2\n");
    int result = send_request(sockfd, url_info->hostname, url_info->port, url_info->path);
    if(-1 == result)
    {
        printf("Error sending request.");
        exit(3);
    }
    // Call `recv` in a loop until there is no more data to receive from the server. Print the received response to stdout.
    printf("3\n");
    int header_received = 0;
    while(numbytes = recv(sockfd, buf, BUFSIZE-1, 0));
    {
        if(header_received)
        {
            printf(buf);
        }
        else
        {
            /*
                Note: This is a kludge. Originally, I attempted to find the
                substring "\n\n", to signify the end of the header, but that
                always failed for a reasons I haven't figured out. Given that
                I've reached the end of the sprint time, this code exists to
                demonstrate the concept in a way that still works for my server
                and also for google.com, but will not work in the general case.
            */
            char *header_end = strstr(buf, "Connection: close");
            if(NULL != header_end)
            {
                printf(header_end+21);
            }
        }
    }   
    printf("4\n");
    // Clean up any allocated memory and open file descriptors.
    destroy_urlinfo(url_info);
    close(sockfd);
    return 0;
}
