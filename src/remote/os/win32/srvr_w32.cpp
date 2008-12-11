/*************  history ************
*
*       COMPONENT: REMOTE       MODULE: SRVR_W32.CPP
*       generated by Marion V2.5     2/6/90
*       from dev              db        on 26-JAN-1996
*****************************************************************
*
*       20927   klaus   26-JAN-1996
*       Call ICS_enter at start
*
*       20858   klaus   17-JAN-1996
*       Get rid of extraneous header file
*
*       20841   klaus   12-JAN-1996
*       Add interprocess comm under remote component
*
*       20804   RCURRY  9-JAN-1996
*       Change priority for NP threads to normal
*
*       20768   klaus   20-DEC-1995
*       More xnet driver work
*
*       20729   klaus   8-DEC-1995
*       Begin adding xnet protocol
*
*       20716   jmayer  6-DEC-1995
*       Update to not show NamedPipes as supported on Win95.
*
*       20690   jmayer  4-DEC-1995
*       Change to start the IPC protocol when running as a service.
*
*       20682   jmayer  3-DEC-1995
*       Update to write to logfile and display msg box as a non-service.
*
*       20373   RCURRY  24-OCT-1995
*       Fix bug with license checking
*
*       20359   RMIDEKE 23-OCT-1995
*       add a semicollin
*
*       20356   rcurry  23-OCT-1995
*       Add more license file checking
*
*       20350   RCURRY  20-OCT-1995
*       add license file checking for remote protocols
*
*       20281   RCURRY  13-OCT-1995
*       fix multi thread scheduler problem
*
*       20198   RCURRY  27-SEP-1995
*       Make Windows95 and Windows NT have the same defaults
*
 * The contents of this file are subject to the Interbase Public
 * License Version 1.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy
 * of the License at http://www.Inprise.com/IPL.html
 *
 * Software distributed under the License is distributed on an
 * "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express
 * or implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code was created by Inprise Corporation
 * and its predecessors. Portions created by Inprise Corporation are
 * Copyright (C) Inprise Corporation.
 *
 * All Rights Reserved.
 * Contributor(s): ______________________________________.
 *
 * 2001.11.20: Claudio Valderrama: Honor -b in SS for high priority.
 *
*/


/*
 *      PROGRAM:        JRD Remote Server
 *      MODULE:         srvr_w32.cpp
 *      DESCRIPTION:    Windows NT remote server.
 *
 * copyright (c) 1993, 1996 by Borland International
 */

#include "firebird.h"
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../remote/remote.h"
#include "gen/iberror.h"
#include "../jrd/license.h"
#include "../jrd/ThreadStart.h"
#include "../utilities/install/install_nt.h"
#include "../remote/os/win32/cntl_proto.h"
#include "../remote/inet_proto.h"
#include "../remote/serve_proto.h"
#include "../remote/os/win32/window_proto.h"
#include "../remote/os/win32/wnet_proto.h"
#include "../remote/os/win32/window.rh"
#include "../remote/xnet_proto.h"
#include "../jrd/gds_proto.h"
#include "../jrd/isc_proto.h"
#include "../jrd/jrd_proto.h"
#include "../jrd/os/isc_i_proto.h"
#include "../jrd/isc_s_proto.h"
#include "../jrd/file_params.h"
#include "../jrd/thread_proto.h"
#include "../common/config/config.h"
#include "../common/utils_proto.h"
#include "../../../common/classes/semaphore.h"


static THREAD_ENTRY_DECLARE inet_connect_wait_thread(THREAD_ENTRY_PARAM);
static THREAD_ENTRY_DECLARE wnet_connect_wait_thread(THREAD_ENTRY_PARAM);
static THREAD_ENTRY_DECLARE xnet_connect_wait_thread(THREAD_ENTRY_PARAM);
static THREAD_ENTRY_DECLARE start_connections_thread(THREAD_ENTRY_PARAM);
static THREAD_ENTRY_DECLARE process_connection_thread(THREAD_ENTRY_PARAM);
static HANDLE parse_args(LPCSTR, USHORT*);
static void service_connection(rem_port*);
static int wait_threads(const int reason, const int mask, void* arg);

static HINSTANCE hInst;

static TEXT protocol_inet[128];
static TEXT protocol_wnet[128];
static TEXT instance[MAXPATHLEN];
static USHORT server_flag;
static bool server_shutdown = false;

class ThreadCounter
{
public:
	ThreadCounter() 
	{
		++m_count;
	}

	~ThreadCounter() 	
	{
		--m_count;
		m_semaphore.release();
	}

	static bool wait() 
	{
		while (m_count.value() > 0)
			if (!m_semaphore.tryEnter(10))
				break; 
		
		return (m_count.value() == 0);
	}

private:
	static Firebird::AtomicCounter m_count;
	static Firebird::Semaphore m_semaphore;
};

Firebird::AtomicCounter ThreadCounter::m_count;
Firebird::Semaphore ThreadCounter::m_semaphore;


int WINAPI WinMain(HINSTANCE	hThisInst,
				   HINSTANCE	hPrevInst,
				   LPSTR		lpszArgs,
				   int			nWndMode)
{
/**************************************
 *
 *      W i n M a i n
 *
 **************************************
 *
 * Functional description
 *      Run the server with NT named
 *      pipes and/or TCP/IP sockets.
 *
 **************************************/
	hInst = hThisInst;

	// We want server to crash without waiting for feedback from the user
	try
	{
		if (!Config::getBugcheckAbort())
			SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
	}
	catch (Firebird::fatal_exception& e)
	{
		MessageBox(NULL, e.what(), "Firebird server failure",
			MB_OK | MB_ICONHAND | MB_SYSTEMMODAL  | MB_DEFAULT_DESKTOP_ONLY);
		return STARTUP_ERROR; // see /jrd/common.h
	}
	catch (Firebird::status_exception& e)
	{
		TEXT buffer[BUFFER_LARGE];
		const ISC_STATUS* vector = e.value();
		if (! (vector && fb_interpret(buffer, sizeof(buffer), &vector)))
		{
			strcpy(buffer, "Unknown internal failure");
		}

		MessageBox(NULL, buffer, "Firebird server failure",
			MB_OK | MB_ICONHAND | MB_SYSTEMMODAL  | MB_DEFAULT_DESKTOP_ONLY);
		return STARTUP_ERROR; // see /jrd/common.h
	}


#ifdef SUPERSERVER
	server_flag = SRVR_multi_client;
#else
	server_flag = 0;
#endif

#ifdef SUPERSERVER
	SetProcessAffinityMask(GetCurrentProcess(), static_cast<DWORD>(Config::getCpuAffinityMask()));
#endif

	protocol_inet[0] = 0;
	protocol_wnet[0] = 0;

	strcpy(instance, FB_DEFAULT_INSTANCE);

	HANDLE connection_handle = parse_args(lpszArgs, &server_flag);

#ifdef SUPERSERVER
	// get priority class from the config file
	int priority = Config::getProcessPriorityLevel();

	// override it, if necessary
	if (server_flag & SRVR_high_priority) {
		priority = 1;
	}

	// set priority class
	if (priority > 0) {
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	}
	else if (priority < 0) {
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
	}
#endif

	TEXT mutex_name[MAXPATHLEN];
	fb_utils::_snprintf(mutex_name, sizeof(mutex_name), SERVER_MUTEX, instance);
	fb_utils::prefix_kernel_object_name(mutex_name, sizeof(mutex_name));
	CreateMutex(ISC_get_security_desc(), FALSE, mutex_name);

	// Initialize the service

	ISC_signal_init();
	ISC_enter();

	int nReturnValue = 0;
	ISC_STATUS_ARRAY status_vector;
	fb_utils::init_status(status_vector);

	fb_shutdown_callback(0, wait_threads, fb_shut_finish, NULL);

	if (connection_handle != INVALID_HANDLE_VALUE)
	{
		rem_port* port = 0;

		if (server_flag & SRVR_inet)
		{
			port = INET_reconnect(connection_handle, status_vector);

			if (port)
			{
				SRVR_multi_thread(port, server_flag);
				port = NULL;
			}
		}
		else if (server_flag & SRVR_wnet)
			port = WNET_reconnect(connection_handle, status_vector);
		else if (server_flag & SRVR_xnet)
			port = XNET_reconnect((ULONG) connection_handle, status_vector);

		if (port) {
			service_connection(port);
		}
		else if (status_vector[1])
			gds__log_status(0, status_vector);

		fb_shutdown(0, fb_shutrsn_no_connection);
	}
	else if (!(server_flag & SRVR_non_service))
	{
		Firebird::string service_name;
		service_name.printf(REMOTE_SERVICE, instance);

		CNTL_init(start_connections_thread, instance);

		const SERVICE_TABLE_ENTRY service_table[] =
		{
			{const_cast<char*>(service_name.c_str()), CNTL_main_thread},
			{NULL, NULL}
		};

		// BRS There is a error in MinGW (3.1.0) headers
		// the parameter of StartServiceCtrlDispatcher is declared const in msvc headers
#if defined(MINGW)
		if (!StartServiceCtrlDispatcher(const_cast<SERVICE_TABLE_ENTRY*>(service_table))) {
#else
		if (!StartServiceCtrlDispatcher(service_table)) {
#endif
			if (GetLastError() != ERROR_CALL_NOT_IMPLEMENTED) {
				CNTL_shutdown_service("StartServiceCtrlDispatcher failed");
			}
			server_flag |= SRVR_non_service;
		}
	}
	else
	{
		start_connections_thread(0);
		nReturnValue = WINDOW_main(hThisInst, nWndMode, server_flag);
	}

#ifdef DEBUG_GDS_ALLOC
/* In Debug mode - this will report all server-side memory leaks
 * due to remote access
 */
	//gds_alloc_report(0, __FILE__, __LINE__);
	char name[MAXPATHLEN];
	gds__prefix(name, "memdebug.log");
	FILE* file = fopen(name, "w+t");
	if (file) {
	  fprintf(file, "Global memory pool allocated objects\n");
	  getDefaultMemoryPool()->print_contents(file);
	  fclose(file);
	}
#endif

	return nReturnValue;
}


THREAD_ENTRY_DECLARE process_connection_thread(THREAD_ENTRY_PARAM arg)
{
/**************************************
 *
 *      p r o c e s s _ c o n n e c t i o n _ t h r e a d
 *
 **************************************
 *
 * Functional description
 *
 **************************************/
	ThreadCounter counter;

	service_connection((rem_port*)arg);
	return 0;
}


static THREAD_ENTRY_DECLARE inet_connect_wait_thread(THREAD_ENTRY_PARAM)
{
/**************************************
 *
 *      i n e t _ c o n n e c t _ w a i t _ t h r e a d
 *
 **************************************
 *
 * Functional description
 *
 **************************************/
	ThreadCounter counter;

	ISC_STATUS_ARRAY status_vector;
	while (!server_shutdown)
	{
		fb_utils::init_status(status_vector);
		rem_port* port = INET_connect(protocol_inet, NULL, status_vector, server_flag, 0);

		if (!port) {
			if (status_vector[1]) {
				gds__log_status(0, status_vector);
			}
			break;
		}
		if (server_flag & SRVR_multi_client) {
			SRVR_multi_thread(port, server_flag);
			break;
		}
		else if (gds__thread_start(process_connection_thread, port, THREAD_medium, 0, 0))
		{
			gds__log("INET: can't start worker thread, connection terminated");
			port->disconnect(NULL, NULL);
		}
	}
	return 0;
}


static THREAD_ENTRY_DECLARE wnet_connect_wait_thread(THREAD_ENTRY_PARAM)
{
/**************************************
 *
 *      w n e t _ c o n n e c t _ w a i t _ t h r e a d
 *
 **************************************
 *
 * Functional description
 *
 **************************************/
	ThreadCounter counter;

	ISC_STATUS_ARRAY status_vector;
	while (!server_shutdown)
	{
		fb_utils::init_status(status_vector);
		rem_port* port = WNET_connect(protocol_wnet, NULL, status_vector, server_flag);

		if (!port)
		{
			const ISC_STATUS err = status_vector[1];
			if (err)
			{
				if (err == isc_net_server_shutdown)
					break;
				gds__log_status(0, status_vector);
			}
		}
		else if (gds__thread_start(process_connection_thread, port, THREAD_medium, 0, 0))
		{
			gds__log("WNET: can't start worker thread, connection terminated");
			port->disconnect(NULL, NULL);
		}
	}

	return 0;
}


static THREAD_ENTRY_DECLARE xnet_connect_wait_thread(THREAD_ENTRY_PARAM)
{
/**************************************
 *
 *      x n e t _ c o n n e c t _ w a i t _ t h r e a d
 *
 **************************************
 *
 * Functional description
 *   Starts xnet server side interprocess thread
 *
 **************************************/
	ThreadCounter counter;

	ISC_STATUS_ARRAY status_vector;
	while (!server_shutdown)
	{
		fb_utils::init_status(status_vector);
		rem_port* port = XNET_connect(NULL, NULL, status_vector, server_flag);

		if (!port)
		{
			const ISC_STATUS err = status_vector[1];
			if (err)
			{
				if (err == isc_net_server_shutdown)
					break;
				gds__log_status(0, status_vector);
			}
		}
		else if (gds__thread_start(process_connection_thread, port, THREAD_medium, 0, 0))
		{
			gds__log("XNET: can't start worker thread, connection terminated");
			port->disconnect(NULL, NULL);
		}
	}

	return 0;
}


static void service_connection(rem_port* port)
{
/**************************************
 *
 *      s e r v i c e _ c o n n e c t i o n
 *
 **************************************
 *
 * Functional description
 *
 **************************************/

	SRVR_main(port, server_flag & ~SRVR_multi_client);
}


static THREAD_ENTRY_DECLARE start_connections_thread(THREAD_ENTRY_PARAM)
{
/**************************************
 *
 *      s t a r t _ c o n n e c t i o n s _ t h r e a d
 *
 **************************************
 *
 * Functional description
 *
 **************************************/
	ThreadCounter counter;

	if (server_flag & SRVR_inet) {
		gds__thread_start(inet_connect_wait_thread, 0, THREAD_medium, 0, 0);
	}
	if (server_flag & SRVR_wnet) {
		gds__thread_start(wnet_connect_wait_thread, 0, THREAD_medium, 0, 0);
	}
	if (server_flag & SRVR_xnet) {
		gds__thread_start(xnet_connect_wait_thread, 0, THREAD_medium, 0, 0);
	}
	return 0;
}


static HANDLE parse_args(LPCSTR lpszArgs, USHORT* pserver_flag)
{
/**************************************
 *
 *      p a r s e _ a r g s
 *
 **************************************
 *
 * Functional description
 *      WinMain gives us a stupid command string, not
 *      a cool argv.  Parse through the string and
 *      set the options.
 * Returns
 *      a connection handle if one was passed in,
 *      INVALID_HANDLE_VALUE otherwise.
 *
 **************************************/
	bool delimited = false;

	HANDLE connection_handle = INVALID_HANDLE_VALUE;

	const TEXT* p = lpszArgs;
	while (*p)
	{
		TEXT c;
		if (*p++ == '-')
		{
			while ((*p) && (c = *p++) && (c != ' '))
			{
				switch (UPPER(c))
				{
				case 'A':
					*pserver_flag |= SRVR_non_service;
					break;

				case 'B':
					*pserver_flag |= SRVR_high_priority;
					break;

				case 'D':
					*pserver_flag |= (SRVR_debug | SRVR_non_service);
					break;

#ifndef SUPERSERVER
				case 'H':
					while (*p && *p == ' ')
						p++;
					if (*p) {
						TEXT buffer[32];
						char* pp = buffer;
						while (*p && *p != ' ' && (pp - buffer < sizeof(buffer) - 1))
							*pp++ = *p++;
						*pp++ = '\0';
						connection_handle = (HANDLE) atol(buffer);
					}
					break;
#endif

				case 'I':
					*pserver_flag |= SRVR_inet;
					break;

				case 'M':
					*pserver_flag |= SRVR_multi_client;
					break;

				case 'N':
					*pserver_flag |= SRVR_no_icon;
					break;

				case 'P':	// Specify a port or named pipe other than the default
					while (*p && *p == ' ')
						p++;
					if (*p) {
						// Assumed the buffer size for both protocols may differ
						// in the future, hence I did generic code.
						char* pi = protocol_inet;
						const char* piend = protocol_inet + sizeof(protocol_inet) - 1;
						char* pw = protocol_wnet;
						const char* pwend = protocol_wnet + sizeof(protocol_wnet) - 1;

						*pi++ = '/';
						*pw++ = '\\';
						*pw++ = '\\';
						*pw++ = '.';
						*pw++ = '@';
						while (*p && *p != ' ')
						{
							if (pi < piend)
								*pi++ = *p;
							if (pw < pwend)
								*pw++ = *p++;
						}
						*pi++ = '\0';
						*pw++ = '\0';
					}
					break;

				case 'R':
					*pserver_flag &= ~SRVR_high_priority;
					break;

				case 'S':
					delimited = false;
					while (*p && *p == ' ')
						p++;
					if (*p && *p == '"') {
						p++;
						delimited = true;
					}
					if (delimited) {
						char* pi = instance;
						const char* pend = instance + sizeof(instance) - 1;
						while (*p && *p != '"' && pi < pend) {
							*pi++ = *p++;
						}
						*pi++ = '\0';
						if (*p && *p == '"')
							p++;
					}
					else {
						if (*p && *p != '-') {
							char* pi = instance;
							const char* pend = instance + sizeof(instance) - 1;
							while (*p && *p != ' ' && pi < pend) {
								*pi++ = *p++;
							}
							*pi++ = '\0';
						}
					}
					break;

				case 'W':
					*pserver_flag |= SRVR_wnet;
					break;

				case 'X':
					*pserver_flag |= SRVR_xnet;
					break;

				case 'Z':
					// CVC: printf doesn't work because we don't have a console attached.
					//printf("Firebird remote server version %s\n",  FB_VERSION);
					MessageBox(NULL, FB_VERSION, "Firebird server version",
						MB_OK | MB_ICONINFORMATION | MB_TOPMOST | MB_DEFAULT_DESKTOP_ONLY);
					exit(FINI_OK);

				default:
					/* In case of something unrecognized, just
					 * continue, since we have already taken it off
					 * of p. */
					break;
				}
			}
		}
	}

	if ((*pserver_flag & (SRVR_inet | SRVR_wnet | SRVR_xnet)) == 0) {
		*pserver_flag |= SRVR_wnet;
		*pserver_flag |= SRVR_inet;
		*pserver_flag |= SRVR_xnet;
	}

	return connection_handle;
}

static int wait_threads(const int, const int, void*)
{
	server_shutdown = true;

	if (!ThreadCounter::wait()) {
		gds__log("Timeout expired during remote server shutdown");
	}

	return FB_SUCCESS;
}
