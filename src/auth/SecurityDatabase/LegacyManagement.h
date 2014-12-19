/*
 *	PROGRAM:		Firebird authentication
 *	MODULE:			LegacyManagement.h
 *	DESCRIPTION:	Performs legacy actions with security database
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
 *  Copyright (c) 2010 Alex Peshkov <peshkoff at mail.ru>
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
 */

#ifndef AUTH_LEGACY_MANAGEMENT_H
#define AUTH_LEGACY_MANAGEMENT_H

#include "../common/classes/ImplementHelper.h"
#include "firebird/Interface.h"


namespace Auth {

class SecurityDatabaseManagement FB_FINAL : public Firebird::StdPlugin<Firebird::Api::IManagementImpl<SecurityDatabaseManagement> >
{
public:
	explicit SecurityDatabaseManagement(Firebird::IPluginConfig* par);

	// IManagement implementation
	void start(Firebird::IStatus* status, Firebird::ILogonInfo* logonInfo);
	int execute(Firebird::IStatus* status, Firebird::IUser* user, Firebird::IListUsers* callback);
	void commit(Firebird::IStatus* status);
	void rollback(Firebird::IStatus* status);

	int release();

private:
	Firebird::RefPtr<Firebird::IFirebirdConf> config;
	FB_API_HANDLE database, transaction;
};

} // namespace Auth

#endif // AUTH_LEGACY_MANAGEMENT_H
