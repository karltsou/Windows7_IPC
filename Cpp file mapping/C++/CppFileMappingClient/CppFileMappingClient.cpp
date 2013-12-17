/****************************** Module Header ******************************\
* Module Name:  CppFileMappingClient.cpp
* Project:      CppFileMappingClient
* Copyright (c) Microsoft Corporation.
* 
* File mapping is a mechanism for one-way or duplex inter-process 
* communication among two or more processes in the local machine. To share a 
* file or memory, all of the processes must use the name or the handle of the 
* same file mapping object.
* 
* To share a file, the first process creates or opens a file by using the 
* CreateFile function. Next, it creates a file mapping object by using the 
* CreateFileMapping function, specifying the file handle and a name for the 
* file mapping object. The names of event, semaphore, mutex, waitable timer, 
* job, and file mapping objects share the same name space. Therefore, the 
* CreateFileMapping and OpenFileMapping functions fail if they specify a name
* that is in use by an object of another type.
* 
* To share memory that is not associated with a file, a process must use the 
* CreateFileMapping function and specify INVALID_HANDLE_VALUE as the hFile 
* parameter instead of an existing file handle. The corresponding file  
* mapping object accesses memory backed by the system paging file. You must  
* specify a size greater than zero when you use an hFile of  
* INVALID_HANDLE_VALUE in a call to CreateFileMapping.
* 
* Processes that share files or memory must create file views by using the 
* MapViewOfFile or MapViewOfFileEx function. They must coordinate their  
* access using semaphores, mutexes, events, or some other mutual exclusion 
* technique.
* 
* This VC++ code sample demonstrates opening a file mapping object named 
* "Local\SampleMap" and reading the string written to the file mapping by 
* other process.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma region Includes
#include <stdio.h>
#include <windows.h>
#pragma endregion


// In terminal services: The name can have a "Global\" or "Local\" prefix 
// to explicitly create the object in the global or session namespace. The 
// remainder of the name can contain any character except the backslash 
// character (\). For more information, see:
// http://msdn.microsoft.com/en-us/library/aa366537.aspx
#define MAP_PREFIX          L"Global\\"
#define MAP_NAME            L"SampleMap"
#define FULL_MAP_NAME       MAP_PREFIX MAP_NAME

// File offset where the view is to begin.
#define IN_VIEW_OFFSET         0
#define OUT_VIEW_OFFSET        1024

// The number of bytes of a file mapping to map to the view. All bytes of the 
// view must be within the maximum size of the file mapping object. If 
// VIEW_SIZE is 0, the mapping extends from the offset (VIEW_OFFSET) to the 
// end of the file mapping.
#define VIEW_SIZE           1024

// Unicode string message to be written to the mapped view. Its size in byte 
// must be less than the view size (VIEW_SIZE).
#define MESSAGE             L"Message from the client process."

// I/O declarification
#include <string>
#include <iostream>
using namespace std;
int wmain(int argc, wchar_t* argv[])
{
    HANDLE hMapFile = NULL;
    PVOID pInOutView = NULL;
    CHAR Text1[256];
    WCHAR* Text2;
    string s1;
    // Try to open the named file mapping identified by the map name.
    hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,    // Read Write access
        FALSE,                  // Do not inherit the name
        FULL_MAP_NAME           // File mapping name 
        );
    if (hMapFile == NULL) 
    {
        wprintf(L"OpenFileMapping failed w/err 0x%08lx\n", GetLastError());
        goto Cleanup;
    }
    wprintf(L"The file mapping (%s) is opened\n", FULL_MAP_NAME);

    // Map a input view of the file mapping into the address space of the current 
    // process.
    pInOutView = MapViewOfFile(
        hMapFile,               // Handle of the map object
        FILE_MAP_ALL_ACCESS,    // Read Write access
        0,                      // High-order DWORD of the file offset 
        IN_VIEW_OFFSET,         // Low-order DWORD of the file offset
        VIEW_SIZE               // The number of bytes to map to view
        );
    if (pInOutView == NULL)
    {
        wprintf(L"MapViewOfFile failed w/err 0x%08lx\n", GetLastError()); 
        goto Cleanup;
    }

    wprintf(L"The file view is mapped\n");

    // Read and display the content in view.
    wprintf(L"Read from the file mapping:\n\"%s\"\n", (PWSTR)pInOutView);

	// Prepare a message to be written to the server view.
	PWSTR pszMessage;
    DWORD cbMessage;
	DWORD ssize = 0;
	cout << "Enter a string that stored in share mapping object :";
	getline(cin, s1);
	//cout << "You entered: " << s1;

	while (s1[ssize] != '\0') {
	  Text1[ssize] = s1[ssize];
		ssize++;
	}
	Text1[ssize] = '\0';
	//wprintf(L"number of size is %d\n", ssize);

	Text2 = new WCHAR[ssize];
	// Convert char* string to a wchar_t* string.
    mbstowcs(Text2, Text1, ssize+1);
    pszMessage = Text2;
    cbMessage = cbMessage = (wcslen(pszMessage) + 1) * sizeof(*pszMessage);

	// Write the message to the server view.
    memcpy_s((pInOutView), VIEW_SIZE, pszMessage, cbMessage);

    // Wait to clean up resources and stop the process.
    wprintf(L"Press ENTER to clean up resources and quit");
    getchar();

Cleanup:

    if (hMapFile)
    {
        if (pInOutView)
        {
            // Unmap the file view.
            UnmapViewOfFile(pInOutView);
            pInOutView = NULL;
        }
		
        // Close the file mapping object.
        CloseHandle(hMapFile);
        hMapFile = NULL;
    }

    return 0;
}

