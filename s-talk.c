#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "list.h"

char* myPort;
char* remoteHost;
char* remotePort;
static List* sendList;
static List* printList;
pthread_mutex_t sendMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t printMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sendListEmpty = PTHREAD_COND_INITIALIZER;
pthread_cond_t recListEmpty = PTHREAD_COND_INITIALIZER;

pthread_t readerThread;
pthread_t printerThread;
pthread_t senderThread;
pthread_t receiverThread;

#define BUFSIZE 256

void handleError(const char* message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void sendMessage(const char* msg) {
    pthread_mutex_lock(&sendMutex);
    List_prepend(sendList, strdup(msg));
    pthread_cond_signal(&sendListEmpty);
    pthread_mutex_unlock(&sendMutex);
}

void* reader(void* nothing) {
    char exitCode[] = "!\n";
    while (1) {
        char msg[BUFSIZE];
        int readCounter = read(0, msg, BUFSIZE);
        if (readCounter == -1) {
            handleError("Error reading input try again.");
        }
        msg[readCounter] = '\0';

        // Check if the message is the exit code
        if (strcmp(msg, exitCode) == 0) {
            printf("Exit key inputted, shutting down\n");
            sendMessage(msg);
            break; // Break the loop to exit gracefully
        }

        // Add the message to the send list
        sendMessage(msg);
    }
    pthread_exit(NULL);
}

void sendUDPMessage(const char* msg) {
    int sockfd = -1;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(remoteHost, remotePort, &hints, &servinfo)) != 0) {
        handleError("getaddrinfo");
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        break;
    }

    if (p == NULL) {
        handleError("Failed to create socket");
    }

    freeaddrinfo(servinfo);

    numbytes = sendto(sockfd, msg, strlen(msg), 0, p->ai_addr, p->ai_addrlen);
    if (numbytes == -1) {
        handleError("sendto");
    }

    if (strcmp(msg, "!\n") == 0) {
        close(sockfd);
    }
}

void* UDPsender(void* nothing) {
    while (1) {
        char* msg = NULL;

        pthread_mutex_lock(&sendMutex);
        if (List_count(sendList) == 0) {
            pthread_cond_wait(&sendListEmpty, &sendMutex); // Wait until sendList is not empty
        }
        msg = List_trim(sendList);
        pthread_mutex_unlock(&sendMutex);

        if (msg == NULL) {
            handleError("Failed to retrieve message from sendList function");
        }

        sendUDPMessage(msg);

        if (strcmp(msg, "!\n") == 0) {
            break;
        }
    }
    pthread_exit(NULL);
}

void printMessage(const char* msg) {
    int bytesWritten; 
    bytesWritten = write(1, msg, strlen(msg));
    if (bytesWritten == -1) {
        handleError("write");
    }
}

void* printer(void* nothing) {
    while (1) {
        pthread_mutex_lock(&printMutex);
        while (List_count(printList) == 0) {
            pthread_cond_wait(&recListEmpty, &printMutex); // Wait until printList is not empty
        }
        char* buf = List_trim(printList);
        pthread_mutex_unlock(&printMutex);

        if (buf == NULL) {
            handleError("Failed to retrieve message from printList function");
        }

        printMessage(buf);

        if (strcmp(buf, "!\n") == 0) {
            break; // Exit the loop if the exit code is received
        }
    }
    pthread_exit(NULL);
}

void* UDPreceiver(void* nothing) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int result; 
    int receivedBytes; 
    struct sockaddr_storage their_addr;
    socklen_t addr_len;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((result = getaddrinfo(NULL, myPort, &hints, &servinfo)) != 0) {
        handleError("getaddrinfo");
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        handleError("Failed to bind socket");
    }

    freeaddrinfo(servinfo);

    addr_len = sizeof their_addr;
    while (1) {
        char buf[BUFSIZE];

        receivedBytes = recvfrom(sockfd, buf, BUFSIZE - 1, 0, (struct sockaddr *)&their_addr, &addr_len);
        if (receivedBytes == -1) {
            handleError("Error recieving message");
        }

        buf[receivedBytes] = '\0';

        pthread_mutex_lock(&printMutex);
        List_prepend(printList, strdup(buf)); // Allocate memory for the message
        pthread_cond_signal(&recListEmpty);    // Signal that the printList is not empty
        pthread_mutex_unlock(&printMutex);

        if (strcmp(buf, "!\n") == 0) {
            printf("Connection Terminated by other user.\n");
            pthread_cancel(senderThread);
            pthread_cancel(readerThread);
            pthread_exit(NULL);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Invalid arguments try again.\n");
        return EXIT_FAILURE;
    }

    myPort = argv[1];
    remoteHost = argv[2];
    remotePort = argv[3];

    sendList = List_create();
    printList = List_create();

    pthread_create(&senderThread, NULL, UDPsender, NULL);
    pthread_create(&receiverThread, NULL, UDPreceiver, NULL);
    pthread_create(&printerThread, NULL, printer, NULL);
    pthread_create(&readerThread, NULL, reader, NULL);

    pthread_join(senderThread, NULL);
    pthread_join(receiverThread, NULL);
    pthread_join(printerThread, NULL);
    pthread_join(readerThread, NULL);

    List_free(printList, free);
    List_free(sendList, free);

    return EXIT_SUCCESS;
}
