#include "common.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( EXIT_FAILURE );
}

// Flag for telling the server to stop running because of a sigint.
// This is safer than trying to print in the signal handler.
static int running = 1;

/** Sig int handler method, used for ctrl-c */
void sigintHandler( int sig_num ) {
  running = 0;
}

char* numberString(int num, int *values) {
    char s[MESSAGE_LIMIT + 1];
    int index = 0;
    for(int i = 0; i < num; i++) {
        index += sprintf(&s[index], "%d ", values[i]);
    }
    return s;
}

int main( int argc, char *argv[] ) {

  // Remove both queues, in case, last time, this program terminated
  // abnormally with some queued messages still queued.
  mq_unlink( SERVER_QUEUE );
  mq_unlink( CLIENT_QUEUE );

  // Prepare structure indicating maximum queue and message sizes.
  struct mq_attr attr;
  attr.mq_flags = 0;
  attr.mq_maxmsg = 1;
  attr.mq_msgsize = MESSAGE_LIMIT;

  // Make both the server and client message queues.
  mqd_t serverQueue = mq_open( SERVER_QUEUE, O_RDONLY | O_CREAT, 0600, &attr );
  mqd_t clientQueue = mq_open( CLIENT_QUEUE, O_WRONLY | O_CREAT, 0600, &attr );
  if ( serverQueue == -1 || clientQueue == -1 )
    fail( "Failed to create message queues" );
  signal( SIGINT, sigintHandler );

  int values[argc - 1];
  
  for(int i = 0; i < argc - 1; i++) {
    values[i] = atoi(argv[i + 1]);
  }
  
  while ( running ) { 
    char buffer[ MESSAGE_LIMIT + 1 ];
    memset( buffer, '\0', MESSAGE_LIMIT + 1 );

    // recieve command
    int len = mq_receive( serverQueue, buffer, sizeof( buffer ), NULL );
    printf("recieved : %d\n", len);

    if( len > 0 ) {
        char command[3][7];
        memcpy(command[0], '\0', 7);
        memcpy(command[1], '\0', 7);
        memcpy(command[2], '\0', 7);

        int index = 0;
        int element = 0;
        for(int i = 0; i < len - 1; i++) {
            if(buffer[i] == ' ') {
                element++;
                index = 0;
            }
            command[element][index] = buffer[i];
        }
        char responce = "success";
        if(strcmp("report", command[0]) == 0) {
            responce = numberString(argc - 1, &values);
        }
        else if(strcmp("swap", command[0]) == 0){
            int a = atoi(command[1]);
            int b = atoi(command[2]);
            swap(&values, a, b);
        } else if(strcmp("inc", command[0]) == 0) {
            int a = atoi(command[1]);
            values[a]++;
        } else if(strcmp("dec", command[0]) == 0) {
            int a = atoi(command[1]);
            values[a]--;
        } else {
            responce = "error";
        }
      mq_send( clientQueue, responce, sizeof( responce ), 0 );
    }
  }

  printf("\n%s\n", numberString(argc - 1, &values));

  // Close our two message queues (and delete them).
  mq_close( clientQueue );
  mq_close( serverQueue );

  mq_unlink( SERVER_QUEUE );
  mq_unlink( CLIENT_QUEUE );

  return 0;
}