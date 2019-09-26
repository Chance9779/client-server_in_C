//client side
//Author:  Jason McQueen
//Date:  2/12/2019
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define MAXSIZE 1024

int main(int argc, char const *argv[])
{

  // grab the FILE
  FILE *fp;

  //character to hold input from file
  char c;

  fp = fopen (argv[2], "r");

  //printf("argv = %s", argv[1]);  DEBUG
  //stage 1, get the random socket from the server

    //first, create a Socket, and error if it isn't made correctly
    int network_socket;
    network_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (network_socket < 0){
      printf("ERROR: wasn't able to create TCP socket");
      return -1;
    }

    //fill in server information
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; //IPV4
    server_address.sin_port = htons(atoi(argv[1])); //command line input as the port number to use
    server_address.sin_addr.s_addr = INADDR_ANY; //any address

    //connect to the socket and issue an error if it failed
    int connection_status = connect(network_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (connection_status < 0){
      printf("ERROR: didn't connect to the server");
      return -1;
    }

    //send a message to start negotiation
    char client_to_server[10] = "259";
    send(network_socket, client_to_server, sizeof(client_to_server), 0);

    //once the message is sent, grab the port number the server sent
    int new_port[1];
    recv(network_socket, &new_port, sizeof(int), 0);
    printf("Port number server sent: %d\n", new_port[0]);

    close(network_socket);

    //stage 2

    char server_buffer[MAXSIZE];
    char send_to_server[4];
    //create the udp Socket
    int udp_client_socket;
    udp_client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_client_socket < 0){
      printf("ERROR: Unable to create UDP socket");
      return -1;
    }

    //use this as the holder of our remote address, so we can use bidirectional comms
    struct sockaddr_in serv_recv_address;
  	socklen_t addrlen = sizeof(serv_recv_address);

    //fill in server information
    struct sockaddr_in udp_server_address;
    udp_server_address.sin_family = AF_INET;
    udp_server_address.sin_port = htons(new_port[0]);
    udp_server_address.sin_addr.s_addr = INADDR_ANY;

    //send our datagram in 4 bit intervals
    //set up our counter for the four bytes
    int i = 0;
    int count = 0;

    int len; //length of server ack
    char end_message[4] = "eof"; //ending message to signify end of file to the server

    //infinite loop for packet transfer
    while (1){

      //make the four bit packet out of the file input
      while (i < 4){
        c = fgetc(fp);
        //printf("c, i: %c, %d\n", c, i); DEBUG
      send_to_server[i] = c;
      i++;
    }
      //printf("string: %s\n", send_to_server);  DEBUG

      //send the packet to the server
      sendto(udp_client_socket, send_to_server, 4, 0, (struct sockaddr *)&udp_server_address, sizeof(udp_server_address));
      //grab the server ack
      len = recvfrom(udp_client_socket, server_buffer, 4, 0, (struct sockaddr *)&serv_recv_address, &addrlen);

      //print the server ack
      server_buffer[len] = 0;
     //printf("Server: %s\n", server_buffer);  DEBUG

      //reset our integers
      len = 0;
      i = 0;

      // if we get to the end of the file, send an end message
  if (c == EOF){
    //printf("got to eof");  DEBUG
    sendto(udp_client_socket, end_message, 4, 0, (struct sockaddr *)&udp_server_address, sizeof(udp_server_address));
    //end the infinite while loop
    break;
  }
}

  //clear address memory
memset(&server_address, 0, sizeof(server_address));
memset(&udp_server_address, 0, sizeof(udp_server_address));

  //close the socket and end the program
   close(udp_client_socket);
    return 0;
}
