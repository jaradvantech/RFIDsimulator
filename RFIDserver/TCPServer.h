/*
 * TCPServerLibrary.h
 *
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <string>
#define MAXPACKETSIZE 4096

void * OpenServer(void *Arg);
bool RFIDStringToBool(std::string);
std::string RFIDBoolToString(bool);

#endif /* TCPSERVER_H_ */
