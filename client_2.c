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

// Fail message
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( EXIT_FAILURE );
}


int main( int argc, char *argv[] ) {
  // Open message queue
  // printf( "Starting\n" );
  mqd_t serverQueue = mq_open( SERVER_QUEUE, O_WRONLY );
  mqd_t clientQueue = mq_open( CLIENT_QUEUE, O_RDONLY );
  // Error checking
  if( serverQueue == -1 || clientQueue == -1 )
    fail( "Can't create the needed message queues\n" );
  
  char buffer[ MESSAGE_LIMIT + 1 ];
  
  // Store message from command line arguments in buffer
  int bufferIdx = 0;
  for( int i = 1; i <= argc - 1; i++ ) {
    int currentLen = strlen( argv[ i ] );
    // Input next command line argument to buffer
    for( int j = 0; j < currentLen; j++ ) {
      buffer[ bufferIdx ] = argv[ i ][ j ];
      bufferIdx++;
    }
    // Add a space to buffer after current argv is input
    buffer[ bufferIdx ] = ' ';
    bufferIdx++;  
  }
  // printf( "Wrote message to buffer %s\n", buffer );
  // Add null terminator to buffer
  buffer[ bufferIdx + 1 ] = '\0'; 
  // Send buffer to server on server queue
  mq_send( serverQueue, buffer, strlen( buffer ), 0 );
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
