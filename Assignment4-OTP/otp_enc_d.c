/* 
 * File:   otp_enc_d.c
 * Author: Corwin Perren
 *
 * Created on November 29, 2016, 7:26 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define PROGRAM_SUCCESS 0
#define PROGRAM_FAILURE 1

#define PORT_MAX 65535
#define PORT_MIN 0

int main(int argc, char** argv) {
    int listen_sfd; //sfd == socked_file_descriptor
    int comms_sfd;
    unsigned long int listen_port_number;    //0 - 65535
    
    socklen_t client_length;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    
    //Check if we have enough arguments, error and exit if not.
    if(argc < 2){
        fprintf(stderr, "No port provided! Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    //Create our listening socket, check if created successfully
    listen_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_sfd < 0){
        fprintf(stderr, "Could not create listening socket! Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    //Get port number, and make sure it's valid
    listen_port_number = atoi(argv[1]);
    if((listen_port_number > PORT_MAX) || (listen_port_number < PORT_MIN)){
        fprintf(stderr, "Port out of range! Please choose a different port! "
                        "Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    //Set up listening parameters
    memset((char *)&server_address, '\0', sizeof(server_address));
    server_address.sin_port = htons(listen_port_number);
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_family = AF_INET;

    //Bind server to settings set above, let system know we're ready
    int bind_result = bind(listen_sfd, (struct sockaddr *)&server_address, \
            sizeof(server_address));
    if(bind_result < 0){
        fprintf(stderr, "Failed to bind listening port! "
                        "Please choose a different port! Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    fprintf(stdout, ":: otp_enc_d :: Bound on port %u.\n", listen_port_number);
    
    
    while(1){
        static int status;
        waitpid(-1, &status, WNOHANG);

        listen(listen_sfd, 5); //Listen to the port

        //At this point, we have a valid queued connection

        socklen_t clilen = sizeof(client_address);
        comms_sfd = accept(listen_sfd, 
        (struct sockaddr *) &client_address,
        &clilen);
        if (comms_sfd < 0){
        }
    }

    close(listen_sfd);
    exit(PROGRAM_SUCCESS);
}

