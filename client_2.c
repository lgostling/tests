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
  for(int i = 0; i < argc; i++) {
      int l = strlen(argv[i]);
      for(int c = 0; c < l; c++) {
          message[index] = argv[i][c];
          index ++;
      }
      message[index] = ' ';
      index++;
  }

  // Send buffer to server on server queue
  mq_send( serverQueue, message, strlen( message ), 0 );
  // printf( "Sent message %s\n", buffer );
  // Receive output message from client queue (either success or failure)
  char output[ MESSAGE_LIMIT + 1 ];
  // printf( "Receiving output\n" );
  mq_receive( clientQueue, output, sizeof( output ), NULL );
  // Display output to user
  printf( "%s\n", output );
  // Close these copies of the message queue, exit
  mq_close( serverQueue );
  mq_close( clientQueue );
  

  return EXIT_SUCCESS;
}
