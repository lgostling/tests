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

    char message[] = "test message";
    mq_send( server_queue, message, strlen( message ), 0 );



    mq_close( clientQueue );
    mq_close( serverQueue );

    mq_unlink( SERVER_QUEUE );
    mq_unlink( CLIENT_QUEUE );

    return EXIT_SUCCESS;

  if ( client_queue == -1 )
    fail( "Can't open message queue" );
}
 
 