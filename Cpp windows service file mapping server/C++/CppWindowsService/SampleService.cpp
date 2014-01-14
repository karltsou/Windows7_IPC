/****************************** Module Header ******************************\
* Module Name:  SampleService.cpp
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

#pragma region Includes
#include "SampleService.h"
#include "ThreadPool.h"
#include <Windows.h>
#pragma endregion

CSampleService::CSampleService(PWSTR pszServiceName, 
                               BOOL fCanStop, 
                               BOOL fCanShutdown, 
                               BOOL fCanPauseContinue)
: CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
{
    m_fStopping = FALSE;

    // Create a manual-reset event that is not signaled at first to indicate 
    // the stopped signal of the service.
    m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hStoppedEvent == NULL)
    {
        throw GetLastError();
    }
}


CSampleService::~CSampleService(void)
{
    if (m_hStoppedEvent)
    {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
    }
}


//
//   FUNCTION: CSampleService::OnStart(DWORD, LPWSTR *)
//
//   PURPOSE: The function is executed when a Start command is sent to the 
//   service by the SCM or when the operating system starts (for a service 
//   that starts automatically). It specifies actions to take when the 
//   service starts. In this code sample, OnStart logs a service-start 
//   message to the Application log, and queues the main service function for 
//   execution in a thread pool worker thread.
//
//   PARAMETERS:
//   * dwArgc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
//   NOTE: A service application is designed to be long running. Therefore, 
//   it usually polls or monitors something in the system. The monitoring is 
//   set up in the OnStart method. However, OnStart does not actually do the 
//   monitoring. The OnStart method must return to the operating system after 
//   the service's operation has begun. It must not loop forever or block. To 
//   set up a simple monitoring mechanism, one general solution is to create 
//   a timer in OnStart. The timer would then raise events in your code 
//   periodically, at which time your service could do its monitoring. The 
//   other solution is to spawn a new thread to perform the main service 
//   functions, which is demonstrated in this code sample.
//
void CSampleService::OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
    // Log a service start message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStart", 
        EVENTLOG_INFORMATION_TYPE);

    // Queue the main service function for execution in a worker thread.
    CThreadPool::QueueUserWorkItem(&CSampleService::ServiceWorkerThread, this);
}

//#define FILE_MAPPING_KERNELDRIVER
//#if defined(FILE_MAPPING_KERNELDRIVER)
//#define FULL_MAP_KERNELDRIVER_NAME       L"Global\\UserKernelSharedSection"
//#endif
//
//   FUNCTION: CSampleService::ServiceWorkerThread(void)
//
//   PURPOSE: The method performs the main function of the service. It runs 
//   on a thread pool worker thread.
//
void CSampleService::ServiceWorkerThread(void)
{
	// Log a service message to the ServiceWorkerThread.
    WriteEventLogMsg(L"ServiceWorkerThread is started");

	HANDLE hMapFile = NULL;
    PVOID pInOutView = NULL;

    SECURITY_ATTRIBUTES SecAttr, *pSec = 0;
    SECURITY_DESCRIPTOR SecDesc;
    if (InitializeSecurityDescriptor(&SecDesc, SECURITY_DESCRIPTOR_REVISION) &&
        SetSecurityDescriptorDacl(&SecDesc, TRUE, (PACL)0, FALSE))
    {
      SecAttr.nLength = sizeof(SecAttr);
      SecAttr.lpSecurityDescriptor = &SecDesc;
      SecAttr.bInheritHandle = TRUE;
      pSec = &SecAttr;
    }

    // Create the file mapping object.
    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,   // Use paging file - shared memory
        pSec,                   // Default security attributes
        PAGE_READWRITE,         // Allow read and write access
        0,                      // High-order DWORD of file mapping max size
        MAP_SIZE,               // Low-order DWORD of file mapping max size
        FULL_MAP_NAME           // Name of the file mapping object
        );
    
	if (hMapFile == NULL) 
        goto Cleanup;

    WriteEventLogMsg(L"The file mapping is created");

    // Map a output view of the file mapping into the address space of the current 
    // process.
    pInOutView = MapViewOfFile(
        hMapFile,               // Handle of the map object
        FILE_MAP_ALL_ACCESS,    // Read Write access
        0,                      // High-order DWORD of the file offset 
        OUT_VIEW_OFFSET,        // Low-order DWORD of the file offset 
        VIEW_SIZE               // The number of bytes to map to view
        );
    
	if (pInOutView == NULL)
        goto Cleanup;

    WriteEventLogMsg(L"The file view is mapped");

	// Prepare a message to be written to the view.
    PWSTR pszMessage = MESSAGE;
    DWORD cbMessage = (wcslen(pszMessage) + 1) * sizeof(*pszMessage);

    // Write the message to the file-mapping view.
    memcpy_s(pInOutView, VIEW_SIZE, pszMessage, cbMessage);

	// Periodically check if the service is stopping.
    while (!m_fStopping)
    {
        ::Sleep(2000);  // Simulate some lengthy operations.
		
    }

    WriteEventLogMsg(L"ServiceWorkerThread is terminated");
	WriteEventLogMsg((PWSTR)pInOutView);

Cleanup:
    WriteEventLogMsg(L"The file view is unmapped");
    if (hMapFile)
    {
        if (pInOutView)
        {
            // Unmap the file view.
            UnmapViewOfFile(pInOutView);
            pInOutView = NULL;
        }

        CloseHandle(hMapFile);
        hMapFile = NULL;
    }


	// Signal the stopped event.
    SetEvent(m_hStoppedEvent);
}


//
//   FUNCTION: CSampleService::OnStop(void)
//
//   PURPOSE: The function is executed when a Stop command is sent to the 
//   service by SCM. It specifies actions to take when a service stops 
//   running. In this code sample, OnStop logs a service-stop message to the 
//   Application log, and waits for the finish of the main service function.
//
//   COMMENTS:
//   Be sure to periodically call ReportServiceStatus() with 
//   SERVICE_STOP_PENDING if the procedure is going to take long time. 
//
void CSampleService::OnStop()
{
    // Log a service stop message to the Application log.
    WriteEventLogEntry(L"CppWindowsService in OnStop", 
        EVENTLOG_INFORMATION_TYPE);

    // Indicate that the service is stopping and wait for the finish of the 
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}