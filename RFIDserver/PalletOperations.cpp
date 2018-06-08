/*
 *
 * PalletNumber is the place or "position" where the pallet is physically located
 * Server 1, channel 1 = pallet 0
 * Server 1, channel 2 = pallet 1
 * Server 2, channel 1 = pallet 4
 * And so on.
 *
 */

#include <string>
#include <iostream>
#include <fstream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "PalletOperations.h"
#include "RFIDserver.h"

std::string filenames[SERVER_NUM*4];

std::string ReadUID(int PalletNumber)
{
	std::string UID, filename = filenames[PalletNumber];
	std::ifstream InputStream(filename);

	if(InputStream.is_open())
	{
		std::getline(InputStream, UID);
	}
	else
	{
		std::cout << "Error reading file " << filename << std::endl;
	}
	InputStream.close();
	return UID;
}

std::string ReadUserData(int PalletNumber, int start, int length)
{
	std::string userdata, filename = filenames[PalletNumber];
	std::ifstream InputStream(filename);

	if(InputStream.is_open())
	{
		//Discard first line
		std::getline(InputStream, userdata);
		//Read user data from second line
		std::getline(InputStream, userdata);
	}
	else
	{
		std::cout << "Error reading file " << filename << std::endl;
	}
	InputStream.close();
	return userdata.substr(start,length);
}

void WriteUserData(int PalletNumber, int start, int length, std::string NewData)
{
	std::string OldData, UID, filename = filenames[PalletNumber];
	std::ifstream InputStream(filename);

	//file can't be edited straight away
	//Read data, edit, and write file again
	if(InputStream.is_open())
	{
		std::getline(InputStream, UID);
		std::getline(InputStream, OldData);
		InputStream.close();
	}
	else
	{
		std::cout << "Error: can't read file " << filename << std::endl;
	}
	std::ofstream OutputStream(filename);
	if(OutputStream.is_open())
	{
		if((start+length) <= TAG_CAPACITY)
			OutputStream << UID << std::endl << OldData.replace(start, length, NewData) << std::endl;
		else
			std::cout << "Error: data too large for tag " << std::endl;
		OutputStream.close();
	}
	else
	{
		std::cout << "Error: can't edit file " << filename << std::endl;
	}
}

/*
 * Get pallet number from server-channel info
 * Pallet number starting at 0
 * Server number starting at 0
 * Channel number starting at 1
 * Now it's a linear relationship but a map could also be used
 */
int LocatePallet(int Channel, int Server)
{
	return(Server*CHANNELS+(Channel-1)); //No estoy seguro de si en tu programa los canales empiezan por 1, revisar.
}

void CreateDefaultPallet(int PalletNumber)
{
	/* Creating new pallets from scratch:
	 *
	 * Filename will be "VIRTUALPALLET_" plus pallet number plus txt extension
	 * i.e. VIRTUALPALLET_05.txt
	 *
	 * UID will be "VIRTUALPALLET_" plus pallet number
	 * i.e. VIRTUALPALLET_05
	 *
	 * Data will be all zero's
	 */
	std::string UID = "VIRTUALPALLET_" + (boost::format("%02u") % PalletNumber).str();
	std::string filename = UID + ".txt";
	std::ofstream OutputStream(filename);

	if(OutputStream.is_open())
	{
		std::cout << "Creating pallet " + filename << std::endl;
		//First line is UID
		OutputStream << UID << std::endl;
		//Blank data in second line
		for(int i=0; i<TAG_CAPACITY; i++)
			OutputStream << 0x01;
		OutputStream << std::endl;

		filenames[PalletNumber] = filename;
	}
	else
	{
		std::cout << "Error: can't create file " << filename << std::endl;
	}
	OutputStream.close();
}

void InsertPallet(std::string filename, int PalletNumber)
{
	filenames[PalletNumber] = filename;
}

bool PalletFileExists(std::string filename)
{
	std::ifstream FileToCheck(filename); //85

	//Try to read file, will return wether file exists or not
	return FileToCheck.good();
}

bool PrintPallets(void)
{
	for(int i=0; i<SERVER_NUM*4; i++)
	{
		std::cout << "Pallet at position " << std::to_string(i) << ":  ";
		std::cout << filenames[i] << ", UID=" << ReadUID(i) << std::endl;
	}
}

/*
 * When program is started, default pallets will be inserted. Missing files will be created
 */
void InitPallets(void)
{
	for(int i=0; i<SERVER_NUM*CHANNELS; i++)
	{
		std::string DefaultFilename = "VIRTUALPALLET_" + (boost::format("%02u") % i).str() + ".txt";
		if(!PalletFileExists(DefaultFilename))
		{
			CreateDefaultPallet(i);
		}
		else
		{
			InsertPallet(DefaultFilename, i);
		}
	}
}

void PalletEditMenu(void)
{
	std::string PalletToMove;
	int PositionToMove;
	do{
		std::cout << "Replace pallet at position: \n>";
		std::cin >> PositionToMove;
		std::cout << "With pallet called [filename]: \n>";
		std::cin >> PalletToMove;
	}while(!PalletFileExists(PalletToMove) || PositionToMove<0 || PositionToMove>(SERVER_NUM*CHANNELS));

		InsertPallet(PalletToMove, PositionToMove);
		std::cout << "Pallet changed. Current layout:" << std::endl;
		PrintPallets();
}
