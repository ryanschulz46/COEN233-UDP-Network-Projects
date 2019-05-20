/*
 Ryan Schulz

 Assignment 2 Phone Client
 
 Used this resource for help setting up socket stuff:
 https://www.cs.rutgers.edu/~pxk/417/notes/sockets/index.html
 
If port 21234 is used, you might have to change it to a local port that is open on your device
 
 Phone numbers must start with an areacode of 428 or less due to limitations of the unsigned long formatting for the subscriber payload
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <poll.h>

#define DBSIZE 5 //number of phone numbers to check
#define START_ID 0xFFFF // packet start ID
#define END_ID 0xFFFF // packet end ID
#define CLIENT_ID 0x52 //'R' client ID
#define CLIENT_MAX_ID 0xFF // max size of client id
#define LENGTH_MAX 0xFF //max length

#define ACCREQ 0xFFF8 //request access
#define NOPAY 0xFFF9 // account not paid
#define NOEXIST 0xFFFA // account doesnt exist
#define ACCOK 0xFFFB // access granted

#define G2 0x02 // 2G
#define G3 0x03 // 3G
#define G4 0x04 //4G LTE
#define G5 0x05 //5G next gen


#define SERVICE_PORT 21234    /* hard-coded port number */
#define BUFLEN 2048
#define MSGS 5    /* number of messages to send */

//structure of packets
typedef struct data_packet {
    short start_id;
    unsigned char client_id;
    short packet_type; //ACCOK, ACCREQ, NOPAY, or NOEXIST
    char seg_num;
    unsigned char length;
    char tech;
    unsigned long subscriber_num;
    short end_id;
} data_packet;


/*function prototype*/
int inet_aton(const char *cp, struct in_addr *inp);
int close(int fd);



int main(void)
{
    struct sockaddr_in myaddr, remaddr;
    int fd, i, send_attempt, poll_result, slen=sizeof(remaddr);
    char buf[BUFLEN];    /* message buffer */
    int recvlen;        /* # bytes in acknowledgement message */
    char *server = "127.0.0.1";    /* change this to use a different server */
    
    /*
     create a socket
     */
    
    if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
        printf("Client: socket error\n");
    
    /*
     bind it to all local addresses and selected port number
     */
    
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);
    
    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("Client: bind failed");
        return 0;
    }
    
    /*
     now define remaddr, the address to whom we want to send messages
     For convenience, the host address is expressed as a numeric IP address
     that we will convert to a binary format via inet_aton
     */
    
    memset((char *) &remaddr, 0, sizeof(remaddr));
    remaddr.sin_family = AF_INET;
    remaddr.sin_port = htons(SERVICE_PORT);
    if (inet_aton(server, &remaddr.sin_addr)==0) {
        fprintf(stderr, "Client: inet_aton() failed\n");
        exit(1);
    }
    
    
    /*
     setup pollfd for ack_timer
     */
    struct pollfd pfdsock;
    pfdsock.fd = fd;
    pfdsock.events = POLLIN;
    
    
    /*
     Inialize data to check network
     */
    unsigned long numbers[DBSIZE];
    numbers[0] = 4087878098; //02 //NOT PAID
    numbers[1] = 4087878099; //03 // ACCESS GRANTED
    numbers[2] = 4082748484; //04 // NO EXIST: TECH MISMATCH
    numbers[3] = 4083462404; //05 //ACCESS GRANTED
    numbers[4] = 4087874154; //02 // NO EXIST
    
    char tech[DBSIZE];
    tech[0]= 0x02;
    tech[1]= 0x03;
    tech[2]= 0x04;
    tech[3]= 0x05;
    tech[4]= 0x02;
    
    data_packet data_array[DBSIZE];
    for (i=0; i < DBSIZE; i++){
        data_array[i].start_id = START_ID;
        data_array[i].client_id= CLIENT_ID;
        data_array[i].packet_type = ACCREQ;
        data_array[i].seg_num = i;
        data_array[i].tech = tech[i];
        data_array[i].subscriber_num = numbers[i];
        data_array[i].length = sizeof(data_array[i].tech) + sizeof(data_array[i].client_id);
        data_array[i].end_id = END_ID;
    }

    
    /*
     create receive and return variables
     */
    data_packet response;
    data_packet data;
    i = 0;
    
    printf("\n*************************************************************\n");
    printf("\nClient: Socket created. Beginning transmission\n\n");
    printf("*************************************************************\n\n");
    
    /*
     This loop sends the messages and awaits for feedback from server
     */
    while (i < DBSIZE) {
        data = data_array[i];
        printf("Client: Sending access request for number %lu to %s port %d. Attempt 1 out of 3\n", data.subscriber_num, server, SERVICE_PORT);
        
        if (sendto(fd, &data, sizeof(data_packet), 0, (struct sockaddr *)&remaddr, slen)==-1) {
            perror("Client: Sendto error:\n");
            exit(1);
        }
        
        send_attempt = 1;
        //ack_timer implementation
        while(send_attempt < 3){
            //poll for 3 seconds, and resend up to 2 times
            poll_result = poll(&pfdsock,1,3000);
            
            if(poll_result == 0) {
                //timeout, must resend
                send_attempt++;
                printf("\nClient: No response on access request for %lu.\nRetransmitting. Attempt %d out of 3\n", data.subscriber_num, send_attempt);
                if (sendto(fd, &data, sizeof(data_packet), 0, (struct sockaddr *)&remaddr, slen)==-1) {
                    perror("Client: Sendto error:\n");
                    exit(1);
                }
                
                
            }
            else if(poll_result == -1) {
                //error
                perror("Client: Poll error:\n");
                return poll_result;
                
            }
            else {
                
                /* now receive an response from the server */
                recvlen = recvfrom(fd, &response, sizeof(data_packet), 0, (struct sockaddr *)&remaddr, &slen);
                if(recvlen == -1){
                    perror("Client: Receive error:\n");
                    exit(1);
                }
                
                //checks to make sure the start of the packet is correct -just for debugging purposes
                if (response.start_id != (short)START_ID){
                    printf("Client: Invalid response. Was expecting 0xFFFF for start_id, but received %x\n", response.start_id);
                    return -1;
                }
                
                if(response.packet_type == (short)ACCOK){ //ACK received!
                    printf("Network: %lu has been granted access to the network\nACCESS GRANTED\n\n\n", response.subscriber_num);
                    break;
    
                }
                else if(response.packet_type == (short)NOPAY){ //ACK received!
                    printf("Network: %lu has not paid.\nDENIED\n\n\n", response.subscriber_num);
                    break;
                    
                }
                else if(response.packet_type == (short)NOEXIST && response.tech == 0x00){
                    printf("Network: %lu is not a recognized account on this network.\nDENIED\n\n\n", response.subscriber_num);
                    break;
                }
                else if(response.packet_type == (short)NOEXIST){ //ACK received!
                    printf("Network: %lu has a tech other than network acknowledges.\nDENIED\n\n\n", response.subscriber_num);
                    break;
                    
                }
                else{ //ACK nor REJ received. Corrupt packet? debugging purposes.
                    printf("Client: Unknown response \n");
                    return -1;
                }
            }
        }
        
        //
        if(send_attempt >= 3){ //no need to check for REJ packet as REJ packets would have returned
            break; //time out error, break out of transmission loop
        }
        i++; //increment to next packet
        
    }
    
    close(fd);
    if(send_attempt < 3){
        printf("Client: All finished with no errors\n\n");
        return 0;
    }
    else{ //time out error
        printf("Client: Access request for subscriber %lu was not acknowledged after 3 attempts.\nServer does not respond.\nExiting. . . \n\n",data.subscriber_num);
        return -1;
    }
}
