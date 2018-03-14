/*
 * 	RFID simulator
 *  Author: RBS
 *
 *	Simulates behavior of IFM RFID evaluation unit.
 *
 *	Note: max 24 servers
 */

#include <iostream>
#include <fstream>
#include <pthread.h>
#include <thread>
#include <cstdlib>
#include "PalletOperations.h"
#include "RFIDserver.h"
#include "TCPServer.h"

void *virtual_server(void *Arg)
{
	//receive TCP messages
	OpenServer((void *)Arg);
	pthread_exit(NULL);
}


int main(void)
{
	pthread_t VirtualRFIDServer_thread[SERVER_NUM];
	struct Server_conf config_server[SERVER_NUM];


	//Load or Create default pallets
	InitPallets();

	//Start Servers
	for(int i=0; i<SERVER_NUM; i++)
	{
		//Server configuration
		config_server[i].server_index = i;
		config_server[i].server_port = 34000+i;
		std::cout << "Starting Server " << config_server[i].server_index << " on port " << config_server[i].server_port << "..." << std::endl;
		pthread_create(&VirtualRFIDServer_thread[i], NULL, virtual_server, (void *)&config_server[i]);
	}

	std::string discard;
	std::cout << "Press any key to edit pallet locations..." << std::endl;
	std::cin >> discard;

	while(1)
	{
		PalletEditMenu();
	}
}



