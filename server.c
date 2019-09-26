//server side
//author:  Jason McQueen
//date:  2/12/2019
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h> // defines in_addr structure
#include <sys/socket.h> // for socket creation
#include <netinet/in.h> //contains constants and structures for socket stuff
#include <time.h> //including for random number seeding

#define MAXSIZE 1024 //max size of bits that the client-sent file can be held in

int main(int argc, char const *argv[])
{

//just creating a random port number within 1024-65535 inclusive
int lower = 1024;
int upper = 65535;
int random_number = 0;
srand(time(NULL)); //Seeding with the current time
random_number = rand() % upper;
if(random_number < lower)
{
    random_number += lower;
}

//stage 1 ********************************************************************

    //let's make a server side of the client-server
    //hold the new port number to send
    int random_port[1] = {random_number};




    //first, create a server Socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0){
      printf("ERROR: server TCP socket unable to be created");
      return -1;
    }

    //create the address of the server
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; //make it run on IPV4
    server_address.sin_port = htons(atoi(argv[1])); // make it run on the n_port...atoi converts string to int
    server_address.sin_addr.s_addr = INADDR_ANY; //it is allowed to bind to any address on the machine


    //now bind the socket to the address and port
    int bind_result;
    bind_result = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (bind_result < 0){
      printf("ERROR: Unable to bind to TCP socket (possible fix: Pick different port)");
      return -1;
    }

    //now listen for any connections
    if((listen(server_socket, 80)) < 0){
      printf("ERROR: Unable to find connection");
      return -1;
    }

    //accept the client socket, and hold it
    int client_socket;
    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket < 0){
      printf("ERROR: Unable to reach client TCP socket");
      return -1;
    }

    //have a buffer for the message the client is sending
    char client_message[10];
    recv(client_socket, &client_message, sizeof(client_message), 0);

    //  printf("The client sent: %s\n", client_message);  DEBUG

    //print the port number
    printf("Negotiation has been detected. Please select your special random port %d\n", random_number);

    //send a reply with the new port number for UDP transfer
    send(client_socket, random_port, sizeof(random_port), 0);

    //clear the server_address
    memset(&server_address, 0, sizeof(server_address));
    //close the socket
    close(server_socket);


    //stage 2 ***********************************************************

    //create a buffer to hold the datagram from the client
    char client_udp_message[MAXSIZE];
    char client_udp_buffer[4];
    //create the UPD socket
    int udp_server_socket;
    udp_server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_server_socket < 0){
      printf("ERROR: Unable to create UDP socket");
      return -1;
    }

    //printf("UDP server socket created...\n");
    //set the address of the server and client
    struct sockaddr_in udp_server_address, udp_client_address;
     socklen_t addrlen = sizeof(udp_client_address);
    udp_server_address.sin_family = AF_INET; //make it run on IPV4
    udp_server_address.sin_port = htons(random_number); // make it run on the random port
    udp_server_address.sin_addr.s_addr = INADDR_ANY; //it is allowed to bind to any address on the machine

    //bind it
    if((bind(udp_server_socket, (struct sockaddr*)&udp_server_address, sizeof(udp_server_address))) < 0){
      printf("ERROR: Unable to bind UDP socket");
      return -1;
    }


    //wait for the datagram from the client and grab it once it comes through
    int len;
    int i = 0;
    int a = 0;
    char c;
    char server_recv_message[4] = "Done";
    char end_message_client[4] = "eof";

    //infinite loop
    while(1){
    len = recvfrom(udp_server_socket, client_udp_buffer, 4, 0, (struct sockaddr *)&udp_client_address, &addrlen);

    if((strcmp(client_udp_buffer, end_message_client)) == 0){
      client_udp_message[i] = '\0'; //string doesn't really know what to do with the last character since it is an eof. so set it to null
      //printf("client message udp: %s\n", client_udp_message);  DEBUG
        sendto(udp_server_socket, server_recv_message, 4, 0, (struct sockaddr *)&udp_client_address, addrlen);
        break;
    }
    //printf("string: %s\n", client_udp_buffer);
    while ( a < 4){
    c = client_udp_buffer[a];
    client_udp_message[i] = c;
    //printf("C: %c\n", c); DEBUG
    a++;
    i++;
}
    //capitalize the characters in the udp buffer to send as ack
    for (int d = 0; d < 4; d++){
      client_udp_buffer[d] = toupper(client_udp_buffer[d]);
    }
    //send ack, which is the last characters from the client
    sendto(udp_server_socket, client_udp_buffer, 4, 0, (struct sockaddr *)&udp_client_address, addrlen);
    len = 0;
    a = 0;
}

    //Write the client's text to a txt file named "output.txt"
    FILE* fp;
    //write to the text file
    fp = fopen("output.txt", "w");

    //output the client's message to the new output file.
    char c1;
    int n = 0;
    //if the character is eof, then don't put the "y" eof identifier at the end
    while (c1 != EOF)
    {
    c1 = client_udp_message[n];
    if(c1 == EOF){
      break;
    }
    else{
    putc(c1, fp);
    n++;
  }
}

    //clear client and server address
    memset(&udp_client_address, 0, sizeof(udp_client_address));
    memset(&udp_server_address, 0, sizeof(udp_server_address));
    //close the Socket
    close(udp_server_socket);
    return 0;
}
