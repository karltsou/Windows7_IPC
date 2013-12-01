/****************************** Module Header ******************************\
* Module Name:  SampleService.h
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
* 
* Provides a sample service class that derives from the service base class - 
* CServiceBase. The sample service logs the service start and stop 
* information to the Application event log, and shows how to run the main 
* function of the service in a thread pool worker thread.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once

#include "ServiceBase.h"

// In terminal services: The name can have a "Global\" or "Local\"  prefix 
// to explicitly create the object in the global or session namespace. The 
// remainder of the name can contain any character except the backslash 
// character (\). For more information, see:
// http://msdn.microsoft.com/en-us/library/aa366537.aspx
#define MAP_PREFIX          L"Local\\"
#define MAP_NAME            L"SampleMap"
#define FULL_MAP_NAME       MAP_PREFIX MAP_NAME

// Max size of the file mapping object.
#define MAP_SIZE            65536

// File offset where the view is to begin.
#define OUT_VIEW_OFFSET     0
#define IN_VIEW_OFFSET      1024

// The number of bytes of a file mapping to map to the view. All bytes of the 
// view must be within the maximum size of the file mapping object (MAP_SIZE). 
// If VIEW_SIZE is 0, the mapping extends from the offset (VIEW_OFFSET) to  
// the end of the file mapping.
#define VIEW_SIZE           1024

// Unicode string message to be written to the mapped view. Its size in byte 
// must be less than the view size (VIEW_SIZE).
#define MESSAGE             L"Message from the server process."

class CSampleService : public CServiceBase
{
public:

    CSampleService(PWSTR pszServiceName, 
        BOOL fCanStop = TRUE, 
        BOOL fCanShutdown = TRUE, 
        BOOL fCanPauseContinue = FALSE);
    virtual ~CSampleService(void);

protected:

    virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
    virtual void OnStop();

    void ServiceWorkerThread(void);

private:

    BOOL m_fStopping;
    HANDLE m_hStoppedEvent;
};