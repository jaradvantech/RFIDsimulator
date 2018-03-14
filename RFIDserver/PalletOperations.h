/*
 * StorageOperations.h
 *
 *
 */

#ifndef PALLETOPERATIONS_H_
#define PALLETOPERATIONS_H_

#include <string>

std::string ReadUID(int);
std::string ReadUserData(int, int, int);
void WriteUserData(int, int, int, std::string);
int LocatePallet(int, int);
void CreateDefaultPallet(int);
void InsertPallet(std::string, int);
bool PalletFileExists(std::string);
bool PrintPallets(void);
void InitPallets(void);
void PalletEditMenu(void);

#endif
