/*
 * TCPServer
 *
 */
#define FRAME_LENGTH 9 //probar con 11 si todo peta

#include "TCPServer.h"
#include "PalletOperations.h"
#include "RFIDserver.h"
#include <stdio.h>
#include <iostream>
#include <string.h>   //strlen
#include <string>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <boost/algorithm/string/predicate.hpp>
#include <pthread.h>
#include "math.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

int KeepConnectionOpened = 1;

inline const char * const TCPSBoolToString(bool b) {
	return b ? "1" : "0";
}


void * OpenServer(void *Arg)
{
	pthread_detach((unsigned long)pthread_self());
	printf("despues\n");

	//Get port and server index
	struct Server_conf *thisServerConfig = (struct Server_conf *)Arg;

    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] ,
          max_clients = 30 , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;

    char bufferRead[1025];  //data buffer of 1K
    std::string bufferWrite;  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;

    //a message
    //char *message = "Communication established.\r\n";

    //initialize all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(thisServerConfig->server_port);

    //bind the socket to localhost port
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", thisServerConfig->server_port);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");


    while(KeepConnectionOpened)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" ,
            		new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

            /*send new connection greeting message
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )
            {
                perror("send");
            }*/

            puts("Welcome message sent successfully");

            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n" , i);

                    break;
                }
            }
        }

        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , bufferRead, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }

                else
                {
                    //set the string terminating NULL byte on the end
                	bufferRead[valread]= '\0';

                	//construct string from null-terminated array of chars
                	std::string Answer_command, Host_command, Frame, FramedAnswerCMD, FramedHostCMD(bufferRead);

                	//Command starts after 10 bytes of frame
                	Host_command = FramedHostCMD.substr(10);
                	Frame = FramedHostCMD.substr(0,4); //comprobar indices

                   	//std::cout << "Framed Message: " << FramedHostCMD << std::endl;
                   	//std::cout << "Host command: " << Host_command << std::endl;


                   	//TODO sacar fuera de los ifs las declaraciones
                	if((Host_command.substr(0,2)=="RU"))
                	{
                		//Read tag UID
                		int channel = std::stoi(Host_command.substr(3,2));
    					int pallet = LocatePallet(channel, thisServerConfig->server_index);
                		Answer_command  = "RU";
                		Answer_command += thisServerConfig->separator;
                		Answer_command += (boost::format("%02u") % channel).str();
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "00"; //diagnostic information not implemented yet
                		Answer_command += thisServerConfig->separator;
                		Answer_command += (boost::format("%02u") % UID_LENGTH).str();
                		Answer_command += thisServerConfig->separator;
                		Answer_command += ReadUID(pallet);
                		Answer_command += "\r\n";
					}
                	else if ((Host_command.substr(0,2)=="RD"))
                	{
                		//Read user data
                		int start = std::stoi(Host_command.substr(6,5));
    					int length = std::stoi(Host_command.substr(12,4));
    					int channel = std::stoi(Host_command.substr(3,2));
    					int pallet = LocatePallet(channel, thisServerConfig->server_index);
                		Answer_command  = "RD";
                		Answer_command += thisServerConfig->separator;
                		Answer_command += (boost::format("%02u") % channel).str();
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "00"; //diagnostic information not implemented yet
                		Answer_command += thisServerConfig->separator;
                		Answer_command += (boost::format("%05u") % start).str();
                		Answer_command += thisServerConfig->separator;
                		Answer_command += (boost::format("%04u") % length).str();
                		Answer_command += thisServerConfig->separator;
                		Answer_command += ReadUserData(pallet, start, length);
                		Answer_command += "\r\n";
					}
                	else if ((Host_command.substr(0,2)=="WR"))
                	{
                		int start = std::stoi(Host_command.substr(6,5));
    					int length = std::stoi(Host_command.substr(12,4));
    					int channel = std::stoi(Host_command.substr(3,2));
    					int pallet = LocatePallet(channel, thisServerConfig->server_index);
						std::string data = Host_command.substr(17, length);
						WriteUserData(pallet, start, length, data);

                		Answer_command  = "WR";
                		Answer_command += thisServerConfig->separator;
                		Answer_command += (boost::format("%02u") % channel).str();
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "00"; //diagnostic information not implemented yet
                		Answer_command += thisServerConfig->separator;
                		Answer_command += (boost::format("%05u") % start).str();
                		Answer_command += thisServerConfig->separator;
                		Answer_command += (boost::format("%04u") % length).str();
                		Answer_command += thisServerConfig->separator;
                		Answer_command += data;
                		Answer_command += "\r\n";
					}
                	else if ((Host_command.substr(0,2)=="CU"))
                	{
                		//Configure unit. (Implement functions as required)
                		//Ignore control registers and ticket numbers for now
                		thisServerConfig->failSafe = RFIDStringToBool(Host_command.substr(3,2));
                		thisServerConfig->separator = Host_command.substr(17,1);
                		if(thisServerConfig->separator.compare("#") == 0) thisServerConfig->separator = ""; //Page 65 of the manual

                		Answer_command  = "CU";
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "00";
                		Answer_command += thisServerConfig->separator;
                		Answer_command += RFIDBoolToString(thisServerConfig->failSafe);
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "00"; //Control Register 1
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "00"; //Control Register 2
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "00"; //Ticket number ACK
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "00"; //Reserved
                		Answer_command += thisServerConfig->separator; //Separator ACK
                		Answer_command += "AS"; //ASCII
                		Answer_command += "\r\n";
					}
                	else if ((Host_command.substr(0,2)=="CI"))
                	{
                		//Configure interface. Not needed yet.
                		Answer_command  = "CI";
                		Answer_command += thisServerConfig->separator;
                		Answer_command += Host_command.substr(3,2); //IOChannel
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "00"; //Diagnostic info
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "11"; //Channel mode
                		Answer_command += thisServerConfig->separator;
                		Answer_command += Host_command.substr(9,4); //Data hold time
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "000"; //Length of tag block
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "00"; //Number of blocks on the tag
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "01"; //Overload protection L+
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "01"; //Overcurrent protection Q0
                		Answer_command += thisServerConfig->separator;
                		Answer_command += "00"; //TP/UID
                		Answer_command += "\r\n";
					}
                	else
                	{
                		Answer_command = "Incorrect command\r\n";
					}

                	//FramedAnswerCMD  = (boost::format("%01u") % (thisServerConfig->server_index+1)).str();
                	FramedAnswerCMD += Frame;
                	FramedAnswerCMD += thisServerConfig->separator;
                	FramedAnswerCMD += (boost::format("%04u") % (Answer_command.length()+FRAME_LENGTH)).str();
                	FramedAnswerCMD += thisServerConfig->separator;
                	FramedAnswerCMD += Answer_command;
                	//FramedAnswerCMD += thisServerConfig->separator;

                	if((Host_command.substr(0,2)!="RU"))
                	std::cout << "Answer command: " << Answer_command << std::endl;

                	//std::cout << "Framed Answer command: " << FramedAnswerCMD << std::endl;

                	//Send answer
                    send(sd, FramedAnswerCMD.c_str(), strlen(FramedAnswerCMD.c_str()), 0 );
            		bufferWrite="";
                }
            }
        }
    }
}

bool RFIDStringToBool(std::string inputstring)
{
	return inputstring.compare("00") ? true : false;
}

std::string RFIDBoolToString(bool b)
{
	return b ? "01" : "00";
}
