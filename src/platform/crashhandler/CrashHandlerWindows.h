/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERWINDOWS_H
#define ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERWINDOWS_H

#include <windows.h>

#include "platform/Platform.h"
#include "platform/WindowsUtils.h"
#include "platform/crashhandler/CrashHandlerImpl.h"

class CrashHandlerWindows : public CrashHandlerImpl {
	
public:
	
	CrashHandlerWindows();
	~CrashHandlerWindows() override;
	
	bool initialize() override;
	
	bool registerThreadCrashHandlers() override;
	void unregisterThreadCrashHandlers() override;
	
	void registerCrashCallback(CrashHandler::CrashCallback crashCallback);
	void unregisterCrashCallback(CrashHandler::CrashCallback crashCallback);
	
	[[noreturn]] void handleCrash(int crashType, void * crashExtraInfo = 0, int fpeCode = 0);
	
	static CrashHandlerWindows & getInstance();
	
private:
	
	bool registerCrashHandlers() override;
	void unregisterCrashHandlers() override;
	
	void writeCrashDump(PEXCEPTION_POINTERS pointers);
	
	void processCrashInfo() override;
	void processCrashSignal() override;
	void processCrashTrace() override;
	void processCrashDump() override;
	
private:
	
	platform::WideString m_exe;
	platform::WideString m_args;
	
	// Crash handlers to restore
	struct PlatformCrashHandlers * m_pPreviousCrashHandlers;
	static CrashHandlerWindows * m_sInstance;
	
};

#endif // ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERWINDOWS_H
