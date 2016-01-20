//--------------------------------------------------------------------------------------
//
//
// Copyright 2014 ADVANCED MICRO DEVICES, INC.  All Rights Reserved.
//
// AMD is granting you permission to use this software and documentation (if
// any) (collectively, the "Materials") pursuant to the terms and conditions
// of the Software License Agreement included with the Materials.  If you do
// not have a copy of the Software License Agreement, contact your AMD
// representative for a copy.
// You agree that you will not reverse engineer or decompile the Materials,
// in whole or in part, except as allowed by applicable law.
//
// WARRANTY DISCLAIMER: THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND.  AMD DISCLAIMS ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
// INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE, NON-INFRINGEMENT, THAT THE SOFTWARE
// WILL RUN UNINTERRUPTED OR ERROR-FREE OR WARRANTIES ARISING FROM CUSTOM OF
// TRADE OR COURSE OF USAGE.  THE ENTIRE RISK ASSOCIATED WITH THE USE OF THE
// SOFTWARE IS ASSUMED BY YOU.
// Some jurisdictions do not allow the exclusion of implied warranties, so
// the above exclusion may not apply to You.
//
// LIMITATION OF LIABILITY AND INDEMNIFICATION:  AMD AND ITS LICENSORS WILL
// NOT, UNDER ANY CIRCUMSTANCES BE LIABLE TO YOU FOR ANY PUNITIVE, DIRECT,
// INCIDENTAL, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING FROM USE OF
// THE SOFTWARE OR THIS AGREEMENT EVEN IF AMD AND ITS LICENSORS HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
// In no event shall AMD's total liability to You for all damages, losses,
// and causes of action (whether in contract, tort (including negligence) or
// otherwise) exceed the amount of $100 USD.  You agree to defend, indemnify
// and hold harmless AMD and its licensors, and any of their directors,
// officers, employees, affiliates or agents from and against any and all
// loss, damage, liability and other expenses (including reasonable attorneys'
// fees), resulting from Your use of the Software or violation of the terms and
// conditions of this Agreement. 
//
// U.S. GOVERNMENT RESTRICTED RIGHTS: The Materials are provided with "RESTRICTED
// RIGHTS." Use, duplication, or disclosure by the Government is subject to the
// restrictions as set forth in FAR 52.227-14 and DFAR252.227-7013, et seq., or
// its successor.  Use of the Materials by the Government constitutes
// acknowledgement of AMD's proprietary rights in them.
//
// EXPORT RESTRICTIONS: The Materials may be subject to export restrictions as
// stated in the Software License Agreement.
//
//--------------------------------------------------------------------------------------

#include "Thread.h"

#if defined( WIN32 )
//----------------------------------------------------------------------------------------------
//  Windows implementation
//----------------------------------------------------------------------------------------------

Thread::Thread() : m_bRunning(false), m_hThread(NULL), m_uiThreadId(0)
{
}

Thread::~Thread()
{
    if (m_bRunning)
    {
        join();
    }
}

bool Thread::create(THREAD_PROC pThreadFunc, void *pArg)
{
    if (pThreadFunc == NULL || m_hThread != NULL)
    {
        return false;
    }

    m_hThread = CreateThread(NULL, 0, pThreadFunc, pArg, 0, &((DWORD)m_uiThreadId));

    if (m_hThread == NULL)
    {
        return false;
    }

    m_bRunning = true;

    return true;
}


void Thread::join()
{
    if (m_hThread)
    {
        WaitForSingleObject(m_hThread, INFINITE);
    }

    m_bRunning = false;
}




Semaphore::Semaphore() : m_Semaphore(NULL), m_uiMax(0)
{
}

Semaphore::~Semaphore()
{
    if (m_Semaphore)
    {
        destroy();
    }
}


bool Semaphore::create(unsigned int uiInitial, unsigned int uiMax)
{
    if (m_Semaphore != NULL)
    {
        return NULL;
    }

    m_Semaphore = CreateSemaphore(NULL, uiInitial, uiMax, NULL);

    if (m_Semaphore == NULL)
    {
        return false;
    }

    m_uiMax = uiMax;

    return true;
}


void Semaphore::destroy()
{
    if (m_Semaphore)
    {
        ReleaseSemaphore(m_Semaphore, 1, 0);
        CloseHandle(m_Semaphore);

        m_Semaphore = NULL;
    }
}


void Semaphore::release()
{
    if (m_Semaphore)
    {
        ReleaseSemaphore(m_Semaphore, 1, 0);
    }
}

bool Semaphore::waitForObject(unsigned long uiTimeOut)
{
    if (!m_Semaphore)
    {
        return false;
    }

    DWORD dwStatus = WaitForSingleObject(m_Semaphore, uiTimeOut);

    if (dwStatus != WAIT_OBJECT_0)
    {
        return false;
    }

    return true;
}


#elif defined ( LINUX )

//----------------------------------------------------------------------------------------------
//  Linux implementation of Thread
//----------------------------------------------------------------------------------------------
#include <pthread.h>

Thread::Thread() : m_bRunning(false), m_uiThreadId(0)
{
}

Thread::~Thread()
{
    if (m_bRunning)
    {
        join();
    }
}

bool Thread::create(THREAD_PROC pThreadFunc, void *pArg)
{
    if (pThreadFunc == NULL)
    {
        return false;
    }

    int res = pthread_create(&m_uiThreadId, NULL, pThreadFunc, pArg);

    if (res != 0)
    {
        return false;
    }

    m_bRunning = true;

    return true;
}

void Thread::join()
{
    pthread_join(m_uiThreadId, NULL);

    m_bRunning = false;
}



Semaphore::Semaphore() : m_uiMax(0)
{
}


Semaphore::~Semaphore() 
{

}

bool Semaphore::create(unsigned int uiInitial, unsigned int uiMax)
{
    if (pthread_mutex_init(&m_Mutex, NULL) != 0)
    {
        return false;
    }

    if (pthread_cond_init(&m_Condition, NULL))
    {
        return false;
    }

    m_SemCount = uiInitial;
    m_uiMax    = uiMax;

    
    return true;
}


void Semaphore::destroy()
{
    pthread_mutex_destroy(&m_Mutex);
    pthread_cond_destroy(&m_Condition);
}

void Semaphore::release()
{
    pthread_mutex_lock(&m_Mutex);

    if (m_SemCount < m_uiMax)
    {
        m_SemCount++;
    }
    
    pthread_cond_signal(&m_Condition);
    pthread_mutex_unlock(&m_Mutex);
}

bool Semaphore::waitForObject(unsigned long uiTimeOut)
{
    pthread_mutex_lock(&m_Mutex);

    while (m_SemCount < 1)
    {
        pthread_cond_wait(&m_Condition, &m_Mutex);
    }

    m_SemCount--;
    pthread_mutex_unlock(&m_Mutex);
    
    return true;
}

#endif
