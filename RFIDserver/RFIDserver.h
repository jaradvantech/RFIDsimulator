#ifndef RFIDSERVER_H_
#define RFIDSERVER_H_

#define TAG_CAPACITY	112
#define UID_LENGTH		8       //8 Bytes (16 hex characters)
#define SERVER_NUM 		3
#define START_PORT 		34000
#define CHANNELS 		4
#define TRUE   			1
#define FALSE  			0

struct Server_conf{
	int server_index;
	int server_port;
	bool failSafe;
	std::string separator;
};


#endif
