#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "csapp.h"
#include "logger.h"
#include "packetUtils.h"

sem_t *gLogMutex;

// Arguments that we will pass to the Pthread_create() method (passing arguments to a thread).
// We need to use this values inside of the thread() method.
struct threadArgu {
    int connfdp;
    int portno;
};

// Prototypes
char **str_split(char *a_str, const char a_delim);
void doProcess(int connfd, int portno);
static inline void getServerTime(serverTimeData *srvTmData);
void *thread(void *vargp);

/**
 * Thread-Based Concurrent Server
 */
int main(int argc, char *argv[]) {
    char* portno;
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    struct threadArgu *args;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    // The port number on which the server will listen for connections is passed in as an argument.
    portno = argv[1];

    listenfd = Open_listenfd(portno);

    // show the menu
    logger_menu();

    // Initialize and open a named semaphore.
    // LOG_SEM_NAME: a name
    // O_CREAT: a flag that control the operation of the call.
    // O_CREAT: the semaphore is created if it does not already exist.
    // 0644: the mode, specifies the permissions to be placed on the new semaphore.
    // 1: specifies the initial value for the new semaphore.
    if (SEM_FAILED == (gLogMutex = sem_open(LOG_SEM_NAME, O_CREAT, 0644, 1))) {
        perror("semaphore initialization failed");
        exit(1);
    }

    // Makes the server capable of handling a number of simultaneous connections, each in its own thread.
    while (1) {
        // Connection
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int)); // Malloc of connected descriptor necessary to avoid deadly race
        *connfdp = Accept(listenfd, (SA *) &clientaddr, &clientlen);

        // Thread arguments
        args = calloc(1, sizeof(struct threadArgu));
        args->connfdp = *connfdp;
        args->portno = atoi(portno);

        // Create thread
        pthread_t tid;
        Pthread_create(
                &tid, // The actual thread object that contains pthread id
                NULL, // Attributes to apply to this thread
                thread, // The function this thread executes
                args // Arguments to pass to thread function above
                );
        //Pthread_join(tid, NULL);
    }
}

/**
 * Thread routine
 */
void *thread(void *vargp) {
    struct threadArgu *args = (struct threadArgu*) vargp;
    int connfdp = args->connfdp;
    int portno = args->portno;
    printf("connfdp: %d\n", connfdp);
    printf("portno: %d\n", portno);

    /* Run thread in "detached mode".
       This tells the pthreads library that no other thread is going to
       join() this thread. This means that, once this thread terminates,
       its resources can be safely freed (instead of keeping them around
       so they can be collected by another thread join()-ing this thread) */
    Pthread_detach(pthread_self());

    doProcess(connfdp, portno); // Starts the work that this thread will do
    Close(connfdp); // Important!
    free(args); // Free storage allocated
    pthread_exit(NULL);
}

/**
 * Starts the logic to read the messages and work with the bytes
 * @param connfd The connection file descriptor
 * @param portNo The port number of the connection
 */
void doProcess(int connfd, int portNo) {
    // Variables recommended by RIO
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    // Our custom variables
    int packetNumber;
    size_t numberOfBytes;
    unsigned char digitToXor;
    PacketCatalog *pc;
    PacketNumVal *tmpPnv;
    serverTimeData serverTimeInfo;

    // Logic provided by RIO
    Rio_readinitb(&rio, connfd);
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        // get the current time -well formated
        getServerTime(&serverTimeInfo);

        // Split the String to know what message we have received
        char **strReceived = str_split(buf, ',');
        char *header = *(strReceived + 0);

        if (strcmp(header, "$Start") == 0) {

            // get the filename
            char *fileName = *(strReceived + 1);
            // strcspn: calculates the length of the number of characters before
            // the 1st occurrence of character present in both the string.
            fileName[strcspn(fileName, "\r\n")] = '\0'; // The \o mark the end of the string.

            printServerLog("PORT: %d | TIME: %s | $Start,%s\n",
                           portNo, serverTimeInfo.frmtBuf, fileName);

            // build the packets container
            pc = PacketCatalog_new(fileName);

        } else if (strcmp(header, "$Packet") == 0) {

            packetNumber = atoi(*(strReceived + 1));
            numberOfBytes = atoi(*(strReceived + 2));

            if (NULL != pc) {
                // add the received packet in the packets container
                PacketCatalog_add(pc, packetNumber, &numberOfBytes);

                // get a ref to the placed packet
                tmpPnv = pc->pairs[pc->length - 1];

                if (NULL != tmpPnv) {
                    if (NULL != tmpPnv->pd.data) {
                        // It's a Packet, start to read the bytes in the ref we got from the container
                        Rio_readnb(&rio, tmpPnv->pd.data, numberOfBytes);

                        printServerLog("PORT: %d | TIME: %s | $Packet,%d,%zu bytes\n",
                                       portNo, serverTimeInfo.frmtBuf, packetNumber, numberOfBytes);
                    }
                }
            } else {
                perror("ERROR on creating packets storage");
                exit(1);
            }


        } else if (strcmp(header, "$End") == 0) {

            // store the byte that will use to decode the packet
            char *bitToXor = *(strReceived + 1);
            bitToXor[strcspn(bitToXor, "\r\n")] = '\0';
            sscanf(bitToXor, "0x%hhx", &digitToXor);

            printServerLog("PORT: %d | TIME: %s | $End,%s\n",
                           portNo, serverTimeInfo.frmtBuf, bitToXor);

            return;
        }
    }

    // decode the packets
    PacketCatalog_xorData(pc, digitToXor);

    // assemble the package to build the image
    PacketCatalog_convertToImage(pc);

    // free the container used to store the packets
    PacketCatalog_free(pc);
}

/**
 * Read the current time on the server
 * @param srvTmData a struct used to store the retreived time
 */
static inline void getServerTime(serverTimeData *srvTmData) {
    // get brut time
    time(&srvTmData->brutTime);

    // convert to local time
    srvTmData->timeInfo = localtime(&srvTmData->brutTime);

    // format
    strftime(srvTmData->frmtBuf, MAX_FRMT_TIME, "%I:%M %p", srvTmData->timeInfo);
}

/**
 * Split a String by comma.
 * Found on Google.
 */
char **str_split(char *a_str, const char a_delim) {
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char *) * count);

    if (result) {
        size_t idx = 0;
        char *token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
