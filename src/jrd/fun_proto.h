/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		fun_proto.h
 *	DESCRIPTION:	Prototype header file for fun.cpp
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

#ifndef JRD_FUN_PROTO_H
#define JRD_FUN_PROTO_H

class Jrd::CompilerScratch;

void		FUN_evaluate(Jrd::fun*, Jrd::jrd_nod*, Jrd::impure_value*);
void		FUN_fini(Jrd::thread_db*);
void		FUN_init(void);
Jrd::fun*	FUN_lookup_function(const TEXT*, bool ShowAccessError);
Jrd::fun*	FUN_resolve(Jrd::CompilerScratch*, Jrd::fun*, Jrd::jrd_nod*);

#endif // JRD_FUN_PROTO_H

