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
    fail( "Can't create the needed message queues" );
  signal( SIGINT, sigintHandler );

  int numbers[ argc - 1 ];
  
  for( int i = 1; i < argc; i++ ) {
    int current;
    sscanf( argv[ i ], "%d", &current );
    numbers[ i - 1 ] = current;
  }
  
  // Repeatedly read and process client messages.
  while ( running ) { 
    char buffer[ MESSAGE_LIMIT + 1 ];
    memset( buffer, '\0', MESSAGE_LIMIT + 1 );
    // Receive message from client on serverQueue
    int len = mq_receive( serverQueue, buffer, sizeof( buffer ), NULL );
    printf("TEST");
    // Process message, error checking    
    char keyword[ MESSAGE_LIMIT + 1 ];
    int bufIdx = 0;
    // get the first word from the message stored in buffer
    while( bufIdx < strlen( buffer ) && buffer[ bufIdx ] != ' ' ) {
      keyword[ bufIdx ] = buffer[ bufIdx ];
      bufIdx++;
    }
    keyword[ bufIdx ] = '\0'; // Null terminate string
    
    bool valid = true;
    if( strcmp( keyword, "report" ) == 0 && strcmp( buffer, "report " ) == 0 ) {  // Report case 
      char str[ MESSAGE_LIMIT + 1 ];
      memset( str, '\0', MESSAGE_LIMIT + 1 ); 
      
      for( int i = 0; i < argc - 1; i++ ) {
        char current[ MESSAGE_LIMIT + 1 ];
        sprintf( current, "%d ", numbers[ i ] );
        
        strcat( str, current );
      }
      mq_send( clientQueue, str, strlen( str ), 0 );
      continue; // Do not report success or failure
    }
    else if( strcmp( keyword, "swap" ) == 0 ) {  // Swap case
      int lower;
      int upper;
      // Make sure two numbers are provided
      if( sscanf( buffer, "swap %d %d\n", &lower, &upper ) == 2 ) { 
        if( lower < 0 || upper < 0 || lower >= argc - 1 || upper >= argc - 1 ) {
          // Error case for indices
          valid = false;
        }
        else { // Perform the swap
          int current = numbers[ lower ];
          numbers[ lower ] = numbers[ upper ];
          numbers[ upper ] = current;
        }
      }
      else {
        valid = false;
      }
    }
    else if( strcmp( keyword, "inc" ) == 0 ) {  // Inc case
      int idx;
      if( sscanf( buffer, "inc %d\n", &idx ) == 1 ) {
        if( idx < 0 || idx >= argc - 1 ) { // error case
          valid = false;
        }
        else {
          numbers[ idx ]++;
        }
      }
      else {
        valid = false;
      }
    }
    else if( strcmp( keyword, "dec" ) == 0 ) {  // Dec case
      int idx;
      if( sscanf( buffer, "dec %d\n", &idx ) == 1 ) {
        if( idx < 0 || idx >= argc - 1 ) { // error case
          valid = false;
        }
        else {
          numbers[ idx ]--;
        }
      }
      else {
        valid = false;
      }
    }
    else {  // Error case
      valid = false;   
    }
    // Write message to client on clientQueue
    if( valid ) {
      mq_send( clientQueue, "success", sizeof( "success" ), 0 );
    }
    else {
      mq_send( clientQueue, "error", sizeof( "error" ), 0 );
    }
    
  }
  // Display output
  printf( "\n" );
  for( int i = 0; i < argc - 1; i++ ) {
    printf( "%d ", numbers[ i ] );
  }
  printf( "\n" );
  // Close our two message queues (and delete them).
  mq_close( clientQueue );
  mq_close( serverQueue );

  mq_unlink( SERVER_QUEUE );
  mq_unlink( CLIENT_QUEUE );

  return 0;
}