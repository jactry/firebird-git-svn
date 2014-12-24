/*
 *	PROGRAM:	Security data base manager
 *	MODULE:		secur_proto.h
 *	DESCRIPTION:	Prototype header file for security.epp
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
 */

#ifndef UTILITIES_SECUR_PROTO_H
#define UTILITIES_SECUR_PROTO_H

#include "firebird/Interface.h"
#include "../common/classes/ImplementHelper.h"
#include "../common/classes/GetPlugins.h"
#include "../common/classes/array.h"
#include "../common/classes/MetaName.h"

namespace Auth {

class CharField : public Firebird::AutoIface<Firebird::Api::ICharUserFieldImpl<CharField> >
{
public:
	CharField()
		: e(0), s(0), value(*getDefaultMemoryPool())
	{ }

	// ICharUserField implementation
	int entered()
	{
		return e;
	}

	int specified()
	{
		return s;
	}

	void setEntered(Firebird::IStatus*, int newValue)
	{
		e = newValue;
	}

	void setSpecified(int newValue)
	{
		s = newValue;
		if (s)
		{
			value = "";
		}
	}

	const char* get()
	{
		return value.c_str();
	}

	void set(Firebird::IStatus* status, const char* newValue)
	{
		try
		{
			value = newValue ? newValue : "";
		}
		catch (const Firebird::Exception& ex)
		{
			ex.stuffException(status);
		}
	}

	void set(const char* newValue, FB_SIZE_T len)
	{
		value.assign(newValue, len);
	}

	void clear() throw()
	{
		e = s = 0;
		value.erase();		// should not call allocation function - no throw
	}

private:
	int e, s;
	Firebird::string value;
};

class IntField : public Firebird::AutoIface<Firebird::Api::IIntUserFieldImpl<IntField> >
{
public:
	IntField()
		: e(0), s(0), value(0)
	{ }

	// IIntUserField implementation
	int entered()
	{
		return e;
	}

	int specified()
	{
		return s;
	}

	void setEntered(Firebird::IStatus*, int newValue)
	{
		e = newValue;
	}

	void setSpecified(int newValue)
	{
		s = newValue;
		if (s)
		{
			value = 0;
		}
	}

	int get()
	{
		return value;
	}

	void set(Firebird::IStatus*, int newValue)
	{
		value = newValue;
	}

	void clear() throw()
	{
		e = s = 0;
		value = 0;
	}

private:
	int e, s;
	int value;
};

typedef Firebird::Array<UCHAR> AuthenticationBlock;

class UserData : public Firebird::VersionedIface<Firebird::Api::IUserImpl<UserData> >
{
public:
	UserData()
		: op(0), trustedAuth(0), authenticationBlock(*getDefaultMemoryPool())
	{ }

	// IUser implementation
	int operation()
	{
		return op;
	}

	Firebird::ICharUserField* userName()
	{
		return &user;
	}

	Firebird::ICharUserField* password()
	{
		return &pass;
	}

	Firebird::ICharUserField* firstName()
	{
		return &first;
	}

	Firebird::ICharUserField* lastName()
	{
		return &last;
	}

	Firebird::ICharUserField* middleName()
	{
		return &middle;
	}

	Firebird::ICharUserField* comment()
	{
		return &com;
	}

	Firebird::ICharUserField* attributes()
	{
		return &attr;
	}

	Firebird::IIntUserField* admin()
	{
		return &adm;
	}

	Firebird::IIntUserField* active()
	{
		return &act;
	}

	void clear(Firebird::IStatus* status);


	int op, trustedAuth;
	CharField user, pass, first, last, middle, com, attr;
	IntField adm, act;
	CharField database, dba, dbaPassword, role;
	AuthenticationBlock authenticationBlock;

	Firebird::MetaName plugin;

	// deprecated
	CharField group;
	IntField u, g;
};

class StackUserData FB_FINAL : public UserData
{
public:
	void* operator new(size_t, void* memory) throw()
	{
		return memory;
	}
};

class DynamicUserData FB_FINAL : public UserData
{
public:
#ifdef DEBUG_GDS_ALLOC
	void* operator new(size_t size, Firebird::MemoryPool& pool, const char* fileName, int line)
	{
		return pool.allocate(size, fileName, line);
	}
#else	// DEBUG_GDS_ALLOC
	void* operator new(size_t size, Firebird::MemoryPool& pool)
	{
		return pool.allocate(size);
	}
#endif	// DEBUG_GDS_ALLOC
};

class Get : public Firebird::GetPlugins<Firebird::IManagement>
{
public:
	Get(Config* firebirdConf);
	Get(const char* plugName);
};

int setGsecCode(int code, Firebird::IUser* iUser);

} // namespace Auth

#endif // UTILITIES_SECUR_PROTO_H
