Ryan Schulz
COEN 233
Networks Programming Assignment



Both assignments use port 21234. If this port is occupied, you might need to change it to an open port and recompile both programs.

I used this resource for help setting up socket stuff:
 https://www.cs.rutgers.edu/~pxk/417/notes/sockets/index.html


---------------------------------------------------

- - - - - - - - - ASSIGNMENT 1 - - - - - - - - - - 
---------------------------------------------------

Notes/Assumptions: Server utilizes a counter to verify packet sequence integrity. After 2.5 seconds of not receiving a new packet, it resets that packet counter and displays a message. If you run two different transmissions within 2.5 seconds, it will error out due to the packet counter not resetting.

Please let the packet counter reset between testing different error cases.

I assumed when the client received an error from the server, it would display the respective error and quit. It does not attempt to retransmit the message.



******** TO RUN THE PROGRAM ************


Open a new bash, and cd to the direction of ../SchulzRyan_COEN233_Projects/Assignment1
All cases have been pre-compiled to speed things up.

---To test ack_timer, initially run "./normal_transmission" without the server running.

Now, run "./server". This program should stay running

To test the following cases, open up a new bash in the same directory and then for:
0. Normal transmission, run "./normal_transmission"
1. Case 1: Out of order packet, run "./case1_out_of_order_packet"
2. Case 2: Length mismatch, run "./case2_length_mismatch"
3. Case 3: EOF wrong, run "./case3_EOF_wrong"
4. Case 4: Duplicate packet, run "./case4_duplicate_packet"



******** HOW TO COMPILE ************


If you want to recompile the executables yourself, do the following:

1. cd to the directory ../SchulzRyan_COEN233_Projects/Assignment1/ in bash 
2. In bash, compile server.c by running "gcc server.c -o server".
3. Open up "client.c" in the IDE of your choosing. 
4. Modify the integer variable "demo_transmission_case" on line 74 to the mode of your choosing. See comments in lines 76 to 89 to see available options.
5. Compile the client in bash, by running "gcc client.c -o test"
6. Run "./server" in one bash, and in another bash run "./test"









---------------------------------------------------

- - - - - - - - - ASSIGNMENT 2 - - - - - - - - - - 
---------------------------------------------------

Ensure that the "networkdb.txt" file is in the same directory as the ./network executable. The current .txt file location and ./network should work out of the box.

Due to the payload of the subscriber formatting being unsigned long, the maximum number it can hold is 4,294,967,295. As a result, area codes should not be greater than 428. In the test data, 408 was the area code used for all numbers.




******** TO RUN THE PROGRAM ************


Open a new bash, and cd to the direction of ../SchulzRyan_COEN233_Projects/Assignment2
All cases have been pre-compiled to speed things up.

---To test ack_timer, initially run "./client" without the network server running.


To test the program:

1. Run "./network". This program should stay running
2. Open up a new bash in the same directory and then run "./client".

Here are the characteristics of the data hardcoded into the client.

4087878098 - DENIED - has not paid
4087878099 - ACCESS GRANTED
4082748484 - DENIED - tech mismatch
4083462404 - ACCESS GRANTED
4087874154 - DENIED - not in txt file





******** HOW TO COMPILE ************


If you want to recompile the executables yourself, do the following:

1. cd to the directory ../SchulzRyan_COEN233_Projects/Assignment2/ in bash 
2. In bash, compile network.c by running "gcc network.c -o network".
3. Open up "client.c" in the IDE of your choosing. 
4. Compile the client in bash, by running "gcc client.c -o test"
5. Run "./network" in one bash, and in another bash run "./client"


