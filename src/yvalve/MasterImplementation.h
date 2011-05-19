/*
 *	PROGRAM:		Firebird interface.
 *	MODULE:			MasterImplementation.h
 *	DESCRIPTION:	Main firebird interface, used to get other interfaces.
 *
 *  The contents of this file are subject to the Initial
 *  Developer's Public License Version 1.0 (the "License");
 *  you may not use this file except in compliance with the
 *  License. You may obtain a copy of the License at
 *  http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl.
 *
 *  Software distributed under the License is distributed AS IS,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *  See the License for the specific language governing rights
 *  and limitations under the License.
 *
 *  The Original Code was created by Alex Peshkov
 *  for the Firebird Open Source RDBMS project.
 *
 *  Copyright (c) 2011 Alex Peshkov <peshkoff at mail.ru>
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
 *
 *
 */

#ifndef YVALVE_MASTER_IMPLEMENTATION_H
#define YVALVE_MASTER_IMPLEMENTATION_H

#include "firebird.h"
#include "Interface.h"
#include "../common/classes/ImplementHelper.h"

namespace Why
{
	extern Firebird::IProvider* dispatcherPtr;

	class MasterImplementation : public Firebird::StackIface<Firebird::IMaster>
	{
	public:
		// IMaster implementation
		Firebird::IStatus* FB_CARG getStatus();
		Firebird::IProvider* FB_CARG getDispatcher();
		Firebird::IPluginManager* FB_CARG getPluginManager();
		int FB_CARG upgradeInterface(Firebird::IInterface* toUpgrade, int desiredVersion, void* missingFunctionClass);
		const char* FB_CARG circularAlloc(const char* s, size_t len, intptr_t thr);
		Firebird::ITimerControl* FB_CARG getTimerControl();
		Firebird::IAttachment* registerAttachment(Firebird::IProvider* provider,
			Firebird::IAttachment* attachment);
		Firebird::ITransaction* registerTransaction(Firebird::IAttachment* attachment,
			Firebird::ITransaction* transaction);
		Firebird::IDtc* FB_CARG getDtc();
	};

	void shutdownTimers();
} // namespace Why

#endif // YVALVE_MASTER_IMPLEMENTATION_H