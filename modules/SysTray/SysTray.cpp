/**************************************************************************
*   Copyright (C) 2004-2007 by Michael Medin <michael@medin.name>         *
*                                                                         *
*   This code is part of NSClient++ - http://trac.nakednuns.org/nscp      *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "stdafx.h"
#include "SysTray.h"
#include "TrayIcon.h"
#include <ServiceCmd.h>
//#include <config.h>
#include <Winwlx.h>
#include <sysinfo.h>

SysTray gSysTray;

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	NSCModuleWrapper::wrapDllMain(hModule, ul_reason_for_call);
    return TRUE;
}

SysTray::SysTray() : icon(_T("SysTray")) {}
SysTray::~SysTray() {}


void SysTray::show() {
	icon.createThread();
}

bool SysTray::loadModule(NSCAPI::moduleLoadMode mode) {
	if (mode == NSCAPI::normalStart) {
		if (SETTINGS_GET_BOOL(settings_def::SHARED_SESSION)) {
			NSC_LOG_ERROR(_T("You have enabled shared session, systray module will not load..."));
			return true;
		}
		try {
			if ((serviceControll::GetServiceType(SZSERVICENAME)&SERVICE_INTERACTIVE_PROCESS)!=SERVICE_INTERACTIVE_PROCESS) {
				NSC_LOG_ERROR(_T("SysTray is not installed (or it cannot interact with the desktop) SysTray won't be loaded. Run ") SZAPPNAME _T(" SysTray install to change this."));
				return true;
			}
		} catch (serviceControll::SCException e) {
			NSC_LOG_ERROR(_T("SysTray is not installed (or it cannot interact with the desktop) SysTray won't be loaded. Run ") SZAPPNAME _T(" SysTray install to change this."));
			return true;
		}
		show();
	}
	return true;
}
void SysTray::hide() {
	if (!icon.exitThread(20000)) {
		std::wcout << _T("MAJOR ERROR: Could not unload thread...") << std::endl;
		NSC_LOG_ERROR(_T("Could not exit the thread, memory leak and potential corruption may be the result..."));
	}
}
bool SysTray::unloadModule() {
	if (systemInfo::isBelowXP(systemInfo::getOSVersion())) {
		hide();
	}
	hide();
	return true;
}

int SysTray::commandLineExec(const TCHAR* command,const unsigned int argLen,TCHAR** args) {
	if (_wcsicmp(command, _T("install")) == 0) {
		try {
			serviceControll::ModifyServiceType(SZSERVICENAME, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS);
			std::wcout << _T(MODULE_NAME) << _T(" is now able to run as the SERVICE_INTERACTIVE_PROCESS flag has been set.") << std::endl;
		} catch (const serviceControll::SCException& e) {
			std::wcerr << _T("Could not modify service: ") << e.error_ << std::endl;
			return -1;
		} catch (...) {
			std::wcerr << _T("Could not modify service: Unknown error!") << std::endl;
			return -1;
		}
	} else if (_wcsicmp(command, _T("uninstall")) == 0) {
		try {
			serviceControll::ModifyServiceType(SZSERVICENAME, SERVICE_WIN32_OWN_PROCESS);
			std::wcout << _T(" is now not able to run as the SERVICE_INTERACTIVE_PROCESS flag has been reset.") << std::endl;
		} catch (const serviceControll::SCException& e) {
			std::wcerr << _T("Could not modify service: ") << e.error_ << std::endl;
			return -1;
		} catch (...) {
			std::wcerr << _T("Could not modify service: Unknown error!") << std::endl;
			return -1;
		}
	} else {
		std::wcerr << _T("Undefined command, usage: install or uninstall") << std::endl;
		return -1;
	}
	return 0;
}

bool SysTray::hasCommandHandler() {
	return false;
}
bool SysTray::hasMessageHandler() {
	return true;
}

void SysTray::setLogWindow(HWND hWnd) { 
	MutexLock lock(logLock);
	if (lock.hasMutex()) {
		hLogWnd = hWnd; 
	}
}

void SysTray::handleMessage(int msgType, TCHAR* file, int line, TCHAR* message) {
	log_entry record(msgType, file, line, message);
	HWND hWnd = NULL;
	{
		MutexLock lock(logLock);
		if (lock.hasMutex()) {
			log.push_back(record);
			if (log.size() > 50)
				log.pop_front();
			hWnd = hLogWnd;
		} else {
			NSC_LOG_ERROR_STD(_T("Failed to get mutex in logger, message discarded"));
		}
	}
	if (hWnd) {
		SendMessage(hWnd, WM_USER+1, reinterpret_cast<WPARAM>(&record), NULL);
	}
}
SysTray::log_type SysTray::getLog() {
	log_type ret;
	for (log_type::const_iterator cit = log.begin(); cit != log.end(); ++cit)
		ret.push_back(*cit);
	return ret;
}

extern void ShowIcon() {
	gSysTray.show();
}
extern void HideIcon() {
	gSysTray.hide();
}
NSC_WRAPPERS_MAIN_DEF(gSysTray);
NSC_WRAPPERS_HANDLE_MSG_DEF(gSysTray);
NSC_WRAPPERS_IGNORE_CMD_DEF();
NSC_WRAPPERS_CLI_DEF(gSysTray);

