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

// function for ending on ctrl+c
void sigintHandler( int sig_num ) {
  running = 0;
}

char* numberString(int num, int *values) {
    char s[MESSAGE_LIMIT + 1];
    memset(s, '\0', MESSAGE_LIMIT + 1);
    int index = 0;
    for(int i = ; i < num; i++) {
        char n[] = itoa(values[i]);
        int len = strlen(n);
        for(int c = 0; c < len; c++) {
            s[index] = n[c];
            index++;
        }
        s[index] = ' ';
        index++;
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
  
  bool invalid = false;
  // error check inputs
  if(argc > 10 || argv < 1) {
      invalid = true;
  }

  if(!invalid) { 
    for(int i = 0; i < argc - 1; i++) {
        for(int c = 0; c < strlen( argv[i + 1] ); c++) {
            if(argv[i + 1][c] < '0' || argv[i + 1][c] > '9') {
                invalid = true;
            }
        }
    }
  }

  if(invalid) {
      printf("Invalid list of values\n");
      return EXIT_FAILURE;
  }

  for(int i = 0; i < argc - 1; i++) {
    values[i] = atoi(argv[i + 1]);
  }
  
  while ( running ) { 
    char buffer[ MESSAGE_LIMIT + 1 ];
    memset( buffer, '\0', MESSAGE_LIMIT + 1 );

    // recieve command
    int len = mq_receive( serverQueue, buffer, sizeof( buffer ), NULL );

    if( len > 0 ) {
        char command[8];
        memset(command, '\0', 8);
        int index = 1;
        int element = 0;
        for(int i = 0; i < len - 1; i++) {
            if(buffer[i] == ' ') {
                break;
            }
            command[i] = buffer[i];
            index++;
        }

        // report
        if(strcmp("report", command) == 0) {
            char* responce = numberString(argc - 1, &values);
            fprintf(stderr, "%s\n", responce);
            mq_send( clientQueue, responce, strlen(responce), 0 );
            continue;
        }
        
        if(len <= index || buffer[index] > '9' || buffer[index] < '0') {
            mq_send( clientQueue, "error", sizeof( "error" ), 0 ); 
        }
        char indexA[] = "\0\0";
        indexA[0] = buffer[index];
        index = index + 2;
        int a  = atoi(indexA);

        // increment
        if(strcmp("inc", command) == 0) {
            values[a]++;
            mq_send( clientQueue, "success", sizeof( "success" ), 0 );
            continue;
        }
        // decrement
        if(strcmp("dec", command) == 0) {
            values[a]--;
            mq_send( clientQueue, "success", sizeof( "success" ), 0 );
            continue;
        }

        if(len <= index || buffer[index] > '9' || buffer[index] < '0') {
            mq_send( clientQueue, "error", sizeof( "error" ), 0 ); 
        }
        char indexB[] = "\0\0";
        indexB[0] = buffer[index];
        int b  = atoi(indexB);

        //swap
        if(strcmp("swap", command) == 0){
            int temp = values[a];
            values[a] = values[b];
            values[b] = temp;
            mq_send( clientQueue, "success", sizeof( "success" ), 0 );
            continue;
        }

        mq_send( clientQueue, "error", sizeof( "error" ), 0 );  
    }
  }

  printf("\n%s\n", numberString(argc - 1, &values));

  // Close our two message queues (and delete them).
  mq_close( clientQueue );
  mq_close( serverQueue );

  mq_unlink( SERVER_QUEUE );
  mq_unlink( CLIENT_QUEUE );

  return EXIT_SUCCESS;
}