// Author : Luke Gostling

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

int main( int argc, char *argv[] ) {
  mqd_t serverQueue = mq_open( SERVER_QUEUE, O_WRONLY );
  mqd_t clientQueue = mq_open( CLIENT_QUEUE, O_RDONLY );

  if( serverQueue == -1 || clientQueue == -1 ){
    fail( "Failed to create queues" );
  }

  // create message
  char message[ MESSAGE_LIMIT + 1 ];
  memset(message, '\0', MESSAGE_LIMIT + 1);
  int index = 0;
  for(int i = 1; i < argc; i++) {
    int l = strlen(argv[i]);
    for(int c = 0; c < l; c++) {
      message[index] = argv[i][c];
      index ++;
    }
    message[index] = ' ';
    index++;
  }
  // send message
  mq_send( serverQueue, message, strlen( message ), 0 );
  
  // recieve reply
  char reply[ MESSAGE_LIMIT + 1 ];
  mq_receive( clientQueue, reply, sizeof( reply ), NULL );
  printf( "%s\n", reply );

  // close queues
  mq_close( serverQueue );
  mq_close( clientQueue );
  
  return EXIT_SUCCESS;
}
