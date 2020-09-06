 #include "common.h"
 #include <mqueue.h>

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

int main( int argc, char *argv[] ) {
    mqd_t server_queue = mq_open(SERVER_QUEUE, O_WRONLY);
    mqd_t client_queue = mq_open(CLIENT_QUEUE, O_RDONLY);

    char message[] = "test message";
    mq_send( server_queue, message, strlen( message ), 0 );

  if ( client_queue == -1 )
    fail( "Can't open message queue" );
}
 
 