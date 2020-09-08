#include "common.h"
#include <mqueue.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( EXIT_FAILURE );
}

int main( int argc, char *argv[] ) {
    mqd_t server_queue = mq_open(SERVER_QUEUE, O_WRONLY);
    mqd_t client_queue = mq_open(CLIENT_QUEUE, O_RDONLY);
    
    char buffer[ MESSAGE_LIMIT  + 1];
    memset(buffer, '\0', MESSAGE_LIMIT + 1);
    for (int i = 0; i < 5;i++) {
      buffer[i] = 'a';
    }
    mq_send( server_queue, buffer, sizeof(buffer), 0 );
    
    int len = mq_receive( client_queue, buffer, sizeof( buffer ), NULL );
    printf("%s\n", buffer);
    mq_close( client_queue );
    mq_close( server_queue );

    mq_unlink( SERVER_QUEUE );
    mq_unlink( CLIENT_QUEUE );

    return EXIT_SUCCESS;

  if ( client_queue == -1 )
    fail( "Can't open message queue" );
}
 
 