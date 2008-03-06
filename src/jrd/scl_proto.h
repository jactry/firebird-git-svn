/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		scl_proto.h
 *	DESCRIPTION:	Prototype header file for scl.epp
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

#ifndef JRD_SCL_PROTO_H
#define JRD_SCL_PROTO_H

#include "../jrd/scl.h"
#include "../common/classes/array.h"

//namespace Jrd {
//	class SecurityClass;
//}

struct dsc;

void SCL_check_access(const Jrd::SecurityClass*, SLONG, const Firebird::MetaName&,
					  const Firebird::MetaName&, Jrd::SecurityClass::flags_t,
					  const TEXT*, const char*);
void SCL_check_access(const Jrd::SecurityClass*, SLONG, const Firebird::MetaName&,
					  const Firebird::MetaName&, Jrd::SecurityClass::flags_t,
					  const TEXT*, const Firebird::MetaName&, const Firebird::MetaName&);
inline void SCL_check_access(const Jrd::SecurityClass* s_class,
							 SLONG view_id,
							 const Firebird::MetaName& trg_name,
							 const Firebird::MetaName& prc_name,
							 Jrd::SecurityClass::flags_t mask,
							 const TEXT* type,
							 const Firebird::string& name)
{
	SCL_check_access(s_class, view_id, trg_name, prc_name, mask, type, name.c_str());
}
inline void SCL_check_access(const Jrd::SecurityClass* s_class,
							 SLONG view_id,
							 const Firebird::MetaName& trg_name,
							 const Firebird::MetaName& prc_name,
							 Jrd::SecurityClass::flags_t mask,
							 const TEXT* type,
							 const Firebird::MetaName& name)
{
	SCL_check_access(s_class, view_id, trg_name, prc_name, mask, type, name.c_str());
}

void SCL_check_index(Jrd::thread_db*, const Firebird::MetaName&, UCHAR, Jrd::SecurityClass::flags_t);
void SCL_check_procedure(const dsc*, Jrd::SecurityClass::flags_t);
void SCL_check_relation(const dsc*, Jrd::SecurityClass::flags_t);
Jrd::SecurityClass* SCL_get_class(Jrd::thread_db*, const TEXT*);
Jrd::SecurityClass::flags_t SCL_get_mask(const TEXT*, const TEXT*);
void SCL_init(bool, const Jrd::UserId& tempId, Jrd::thread_db*);
void SCL_move_priv(UCHAR**, Jrd::SecurityClass::flags_t, Firebird::UCharBuffer&, ULONG*);
Jrd::SecurityClass* SCL_recompute_class(Jrd::thread_db*, const TEXT*);
void SCL_release(Jrd::SecurityClassList*);

#endif // JRD_SCL_PROTO_H

