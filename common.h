#ifndef _COMMON_H_
#define _COMMON_H_

// Name for the queue of messages going to the server.
#define SERVER_QUEUE "/unity-id-server-queue"

// Name for the queue of messages going to the current client.
#define CLIENT_QUEUE "/unity-id-client-queue"

// Maximum length for a message in the queue (Long enough to hold any
// request or return any request or server response)
#define MESSAGE_LIMIT 1024

#endif
