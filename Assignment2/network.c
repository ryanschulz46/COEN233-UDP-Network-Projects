/*
 Ryan Schulz
 
 Assignment 2 Network
 
 Used this resource for help setting up socket stuff:
 https://www.cs.rutgers.edu/~pxk/417/notes/sockets/index.html
 
 If port 21234 is used, you might have to change it to a local port that is open on your device
 
 networkdb.txt file must be in the same directory as the compiled executable "network"
 
 Phone numbers must start with an areacode of 428 or less due to limitations of the unsigned long formatting for the subscriber payload
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdbool.h> //allows for boolean

#define PACKETS 5 //number of phone numbers that will be checked against db
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

//node structure for storing text file content
typedef struct node
{
    unsigned long subscriber_num;
    char tech;
    char paid;
    struct node *next;
}NODE;

//linked list
NODE *head = NULL;
NODE *tail = NULL;



/*prototype*/
void insertlist(unsigned long, char, char);
NODE * search(unsigned long);
int readfile(char *);


int main(int argc, char **argv)
{
    struct sockaddr_in myaddr;    // our address
    struct sockaddr_in remaddr;    // remote address
    socklen_t addrlen = sizeof(remaddr);        // length of addresses
    int recvlen; //# bytes received
    int poll_result; //result from poll
    int fd;  // socket created
    data_packet data;
    data_packet response;
    NODE* search_result;
    
    /* Populate the Linked List DB from TXT file*/
    if(readfile("networkdb.txt") != 1){
        printf("Network: Error reading file\n");
        return -1;
    }
    
    
    // create a UDP socket
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Network: socket error:\n");
        return 0;
    }
    
    // set up polling for counter reset to assume a new client connection
    struct pollfd pfdsock;
    pfdsock.fd = fd;
    pfdsock.events = POLLIN;
    
    
    
    // bind the socket to local ip address and a specific port
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(SERVICE_PORT);
    
    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("Network: Bind error:\n");
        return 0;
    }
    
    printf("\n*************************************************************\n");
    printf("\nNetwork: Ready on port %d for client connection!\n\n", SERVICE_PORT);
    printf("*************************************************************\n\n");
    
    // now loop, receiving data, printing any errors, and sending back packet to client
    for (;;) {
        
        /*
         Receive packet from client
         */
        recvlen = recvfrom(fd, &data, sizeof(data_packet), 0, (struct sockaddr *)&remaddr, &addrlen);
        if (recvlen > 0) {
            printf("Network: Received request for: \"%lu\" (%d bytes)\n", data.subscriber_num, recvlen);
        }
        else{
            printf("Network: Receive error!\n");
        }
        
        
        //Initialize common attributes in packet to send to client
        response.start_id = START_ID;
        response.end_id = END_ID;
        response.client_id = CLIENT_ID;
        response.seg_num = data.seg_num;
        response.length = data.length;
        response.subscriber_num = data.subscriber_num;
        
        search_result = search(data.subscriber_num);
        
        if(search_result == NULL){ //not exist at all?
            response.packet_type = NOEXIST;
            response.tech = 0x00;
            printf("Network: Does not exist on network.\nDENIED\n\n");
        }
        else if(search_result->tech != data.tech){ //exist, but no matching tech
            response.tech = search_result->tech;
            response.packet_type = NOEXIST;
            printf("Network: Tech mismatch with network.\nDENIED\n\n");
        }
        else if(search_result->paid == 0){ //user exists but has not paid
            response.tech = search_result->tech;
            response.packet_type = NOPAY;
            printf("Network: User has not paid\nDENIED\n\n");
        }
        else{
            response.tech = search_result->tech; //user is found and everything checks out
            response.packet_type = ACCOK;
            printf("Network: Found account\nACCESS GRANTED\n\n");
        }
       
        
        //transmit err/ack packet
        if (sendto(fd, &response, sizeof(data_packet), 0, (struct sockaddr *)&remaddr, addrlen) < 0){
            perror("Network: Sendto error:\n");
        }
        
    }
    /* server never quits */
}


//takes a string to the file location, opens it, and imports the data into a linked list
int readfile(char *x){
    FILE *fp;
    unsigned long subscriber_num;
    unsigned int tech;
    unsigned int paid;
    
    //checks to make sure file is opened properly
    if ((fp = fopen(x,"r")) == NULL){
        printf("Network: Cant open the file. Make sure it is in the same director as the compiled executable.\n");
        return -1;
    } //pass each line's data into insertlist() function to create of node of entry
    while((fscanf(fp,"%lu %x %x", &subscriber_num, &tech, &paid) >0)){
        insertlist(subscriber_num, (char)tech, (char)paid);
    }
    fclose(fp);
    return 1;
    
}


//takes each line of data from the file, imports it to linked list
void insertlist(unsigned long sub, char tech, char paid) {
    //temp node dynamically created
    NODE *temp=(NODE *)malloc(sizeof(NODE));
    if ((temp= (NODE *)malloc(sizeof(NODE))) ==(NODE *)NULL)
    {
        printf("Network: Temp node memory cannot be allocated\n");
        return;
    }
    
    //each line's data is fed into temp node
    temp->subscriber_num = sub;
    temp->tech = tech;
    temp->paid = paid;
    
    
    //if first node, make temp node head
    if (head == NULL)
    {
        head = temp;
        tail = temp;
        tail->next = NULL;
    }
    else //if not first node, add to end of list
    {
        tail->next = temp;
        tail = temp;
        tail->next = NULL;
    }
    return;
}

//searches link list populated by db
//returns null if nothing is found
NODE * search (unsigned long x)
{
    
    NODE *p=head;
    while (p != NULL)
    {
        if (p->subscriber_num == x)
            return p;
        else
            p=p->next;
    }
    return NULL;
}
