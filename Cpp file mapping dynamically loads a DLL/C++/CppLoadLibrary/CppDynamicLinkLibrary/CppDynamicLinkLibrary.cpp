/****************************** Module Header ******************************\
Module Name:  CppDynamicLinkLibrary.cpp
Project:      CppDynamicLinkLibrary
Copyright (c) Microsoft Corporation.

Defines the exported data and functions of the DLL application.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include "CppDynamicLinkLibrary.h"
#include <strsafe.h>
#include <Windows.h>

// In terminal services: The name can have a "Global\" or "Local\" prefix 
// to explicitly create the object in the global or session namespace. The 
// remainder of the name can contain any character except the backslash 
// character (\). For more information, see:
// http://msdn.microsoft.com/en-us/library/aa366537.aspx
#define MAP_PREFIX          L"Local\\"
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

// Local Function
static int SharedMappedFileClient();

#pragma region DLLMain
BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
         wprintf(L"DLLMain - DLL_PROCESS_ATTACH\n");
		 SharedMappedFileClient();
	break;
	case DLL_THREAD_ATTACH:
    break;
	case DLL_THREAD_DETACH:
    break;
	case DLL_PROCESS_DETACH:
		 wprintf(L"DLLMain - DLL_PROCESS_DETACH\n");
	break;
	}
	return TRUE;
}
#pragma endregion


#pragma region Global Data

//// An exported/imported global data using a DEF file
//// Initialize it to be 1
//int g_nVal1 = 1;
//
//
//// An exported/imported global data using __declspec(dllexport/dllimport)
//// Initialize it to be 2
//SYMBOL_DECLSPEC int g_nVal2 = 2;

#pragma endregion


#pragma region Ordinary Functions

static int SharedMappedFileClient()
{
	HANDLE hMapFile = NULL;
    PVOID pInOutView = NULL;

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
    PWSTR pszMessage = MESSAGE;
    DWORD cbMessage = (wcslen(pszMessage) + 1) * sizeof(*pszMessage);

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

// An exported/imported cdecl(default) function using a DEF file
//int /*__cdecl*/ GetStringLength1(PCWSTR pszString)
//{
//    return static_cast<int>(wcslen(pszString));
//}


// An exported/imported stdcall function using __declspec(dllexport/dllimport)
//SYMBOL_DECLSPEC int __stdcall GetStringLength2(PCWSTR pszString)
//{
//    return static_cast<int>(wcslen(pszString));
//}

#pragma endregion


#pragma region Callback Function

// An exported/imported stdcall function using a DEF file
// It requires a callback function as one of the arguments
//int __stdcall CompareInts(int a, int b, PFN_COMPARE cmpFunc)
//{
//	// Make the callback to the comparison function
//
//	// If a is greater than b, return a; 
//    // If b is greater than or equal to a, return b.
//    return ((*cmpFunc)(a, b) > 0) ? a : b;
//}

#pragma endregion


#pragma region Class

//// Constructor of the simple C++ class
//CSimpleObject::CSimpleObject(void) : m_fField(0.0f)
//{
//  wprintf(L"CSimpleObject Constructor\n");
//}
//
//
//// Destructor of the simple C++ class
//CSimpleObject::~CSimpleObject(void)
//{
//  wprintf(L"CSimpleObject Destructor\n");
//}
//
//
//float CSimpleObject::get_FloatProperty(void)
//{
//	return this->m_fField;
//}
//
//
//void CSimpleObject::set_FloatProperty(float newVal)
//{
//	this->m_fField = newVal;
//}
//
//
//HRESULT CSimpleObject::ToString(PWSTR pszBuffer, DWORD dwSize)
//{
//    return StringCchPrintf(pszBuffer, dwSize, L"%.2f", this->m_fField);
//}
//
//
//int CSimpleObject::GetStringLength(PCWSTR pszString)
//{
//    return static_cast<int>(wcslen(pszString));
//}

#pragma endregion