/*
    fat32_dump_valid_block.cpp
    A simple program to view the content of sector in FAT32 disk.
    Used to analyze the FAT32 file format.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA  02110-1301, US
    Copyright (C) 2018-2020 sfyip <yipxxx@gmail.com>
*/
#include "stdafx.h"

//Placed these files in "stdafx.h" to speed up the compile time
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <cstdio>
#include <assert.h>
#include <atlstr.h>

#define SECTOR_SIZE    512

void PrintSector(UINT32 addr, const BYTE* buf) 
{
	assert((SECTOR_SIZE & 15) == 0);

	UINT32 i;
	for (i = 0; i < SECTOR_SIZE; i += 16)
	{
		_tprintf(TEXT("%08X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n"), addr + i,
			buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4], buf[i + 5], buf[i + 6], buf[i + 7],
			buf[i + 8], buf[i + 9], buf[i + 10], buf[i + 11], buf[i + 12], buf[i + 13], buf[i + 14], buf[i + 15]);
	}
}

void PrintLastError() 
{
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
	{
		_tprintf(TEXT("No error"));
		return;
	}

	LPTSTR messageBuffer = NULL;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&messageBuffer, 0, NULL);

	_tprintf(messageBuffer);

	//Free the buffer.
	LocalFree(messageBuffer);
}

int main()
{
	const TCHAR devicePath[] = TEXT("\\\\.\\K:");
	const DWORD addrBegin = 0;
	const DWORD addrEnd = 128*1024*1024;		  //128MB

	BYTE buf[SECTOR_SIZE];
	BYTE zeroFillBuf[SECTOR_SIZE];

	DWORD bytesRead;
	HANDLE device = NULL;

	int exitStatus = EXIT_SUCCESS;

	if ((addrBegin & (SECTOR_SIZE - 1)))
	{
		_tprintf(TEXT("addrBegin not aligned\n"));
		exitStatus = EXIT_FAILURE;
		goto EXIT;
	}

	if ((addrEnd & (SECTOR_SIZE - 1)))
	{
		_tprintf(TEXT("addrEnd not aligned\n"));
		exitStatus = EXIT_FAILURE;
		goto EXIT;
	}

	memset(zeroFillBuf, 0x00, SECTOR_SIZE);

	device = CreateFile(devicePath,    // Drive to open
		GENERIC_READ,                  // Access mode
		FILE_SHARE_READ,               // Share Mode
		NULL,                          // Security Descriptor
		OPEN_EXISTING,                 // How to create
		0,                             // File attributes
		NULL);                         // Handle to template


	if (device == INVALID_HANDLE_VALUE)
	{
		_tprintf(TEXT("Cannot open file\n"));
		PrintLastError();
		exitStatus = EXIT_FAILURE;
		goto EXIT;
	}

	SetFilePointer(device, addrBegin, NULL, FILE_BEGIN);

	DWORD i;
	for (i = addrBegin; i < addrEnd; i += SECTOR_SIZE)
	{
		if (!ReadFile(device, buf, SECTOR_SIZE, &bytesRead, NULL))
		{
			_tprintf(TEXT("Error in reading file\n"));
			PrintLastError();
			exitStatus = EXIT_FAILURE;
			goto EXIT;
		}

		//Print the buf if non-zero
		if (memcmp(buf, zeroFillBuf, SECTOR_SIZE) != 0)
		{
			PrintSector(i, buf);
			_tprintf(TEXT("\n"));

			_getch(); //wait for enter
		}
		
		// Terminate if 'x' is pressed
		if (_kbhit())
		{
			if (_getch() == 'x')
			{
				_tprintf(TEXT("break, addr=%08X\n"), i);
				break;
			}
		}
	}

EXIT:
	CloseHandle(device);
	_getch();
	return exitStatus;
}