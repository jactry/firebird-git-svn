/*
 *	PROGRAM:	JRD Access Method
 *	MODULE:		ext.c
 *	DESCRIPTION:	External file access
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
 * 26-Sept-2001 Paul Beach - Windows External File Directory Config. Parameter 
 *
 * 2001.07.06 Sean Leyne - Code Cleanup, removed "#ifdef READONLY_DATABASE"
 *                         conditionals, as the engine now fully supports
 *                         readonly databases.
 *
 * 2001.08.07 Sean Leyne - Code Cleanup, removed "#ifdef READONLY_DATABASE"
 *                         conditionals, second attempt
 *
 * 2002.10.29 Sean Leyne - Removed obsolete "Netware" port
 *
 */

#include "firebird.h"
#include "../jrd/ib_stdio.h"
#include <errno.h>
#include <string.h>
#include "../jrd/jrd.h"
#include "../jrd/req.h"
#include "../jrd/val.h"
#include "../jrd/exe.h"
#include "../jrd/rse.h"
#include "../jrd/ext.h"
#include "../jrd/tra.h"
#include "gen/codes.h"
#include "../jrd/all_proto.h"
#include "../jrd/err_proto.h"
#include "../jrd/ext_proto.h"
#include "../jrd/gds_proto.h"
#include "../jrd/met_proto.h"
#include "../jrd/mov_proto.h"
#include "../jrd/thd_proto.h"
#include "../jrd/vio_proto.h"
#include "../jrd/dls_proto.h"


extern "C" {


#ifdef WIN_NT
#include <windows.h>
#include <stdlib.h>
#include <io.h>
#define FOPEN_TYPE	"a+b"
#define FOPEN_READ_ONLY	"rb"
#define	SYS_ERR		gds_arg_win32
#endif

#ifndef FOPEN_TYPE
#define FOPEN_TYPE	"a+"
#define FOPEN_READ_ONLY	"rb"
#endif

#ifndef SYS_ERR
#define SYS_ERR		gds_arg_unix
#endif


static void io_error(EXT, TEXT *, STATUS, SLONG);


void EXT_close(RSB rsb)
{
/**************************************
 *
 *	E X T _ c l o s e
 *
 **************************************
 *
 * Functional description
 *	Close a record stream for an external file.
 *
 **************************************/
}


void EXT_erase(RPB * rpb, int *transaction)
{
/**************************************
 *
 *	E X T _ e r a s e
 *
 **************************************
 *
 * Functional description
 *	Update an external file.
 *
 **************************************/

	ERR_post(isc_ext_file_delete, 0);
}


EXT EXT_file(JRD_REL relation, TEXT * file_name, SLONG * description)
{
/**************************************
 *
 *	E X T _ f i l e
 *
 **************************************
 *
 * Functional description
 *	Create a file block for external file access.
 *
 **************************************/
#ifdef WIN_NT
	TEXT ib_file_path[MAXPATHLEN];
	TEXT absolute_file[MAXPATHLEN];
	TEXT drive_buf[_MAX_DRIVE];
	TEXT dir_buf[_MAX_DIR];
	TEXT file_buf[_MAX_FNAME];
	TEXT ext_buf[_MAX_EXT];
	EDLS *dir_list;
	BOOLEAN found_dir;
#endif

	DBB dbb;
	EXT file;

	dbb = GET_DBB;
	CHECK_DBB(dbb);

/* if we already have a external file associated with this relation just
 * return the file structure */
	if (relation->rel_file) {
		EXT_fini(relation);
	}

#ifdef WIN_NT
	/* Default number of file handles stdio.h on Windows is 512, use this 
	call to increase and set to the maximum */
	_setmaxstdio(2048);
#endif

#ifdef WIN_NT
	found_dir = FALSE;	
	dir_list = DLS_get_file_dirs();
		
	while (dir_list && !found_dir)
	{
		strcpy(ib_file_path, dir_list->edls_directory);
		strcat(ib_file_path, "\\");
		_splitpath(file_name, drive_buf, dir_buf, file_buf, ext_buf);
		strcpy(absolute_file, file_buf);
		strcat(absolute_file, ext_buf);
		strncat(ib_file_path, absolute_file, MAXPATHLEN - strlen(ib_file_path) - 1);
			
		if (!_access(ib_file_path, 4))
		{
			relation->rel_file = file =
				FB_NEW_RPT(*dbb->dbb_permanent, strlen(ib_file_path) + 1) ext();
			strcpy(reinterpret_cast<char*>(file->ext_filename), absolute_file);
			found_dir = TRUE;
		}
		dir_list = dir_list->edls_next;
	}

	if (!found_dir)
	{
		relation->rel_file = file =
			FB_NEW_RPT(*dbb->dbb_permanent, strlen(file_name) + 1) ext();
		strcpy(reinterpret_cast<char*>(file->ext_filename), file_name);
	}
#else
	relation->rel_file = file =
		FB_NEW_RPT(*dbb->dbb_permanent, (strlen(file_name) + 1)) ext();
	strcpy(reinterpret_cast<char*>(file->ext_filename), file_name);
#endif

	file->ext_flags = 0;
	file->ext_ifi = (int *) NULL;

/* If the database is updateable, then try opening the external files in
 * RW mode. If the DB is ReadOnly, then open the external files only in
 * ReadOnly mode, thus being consistent.
 */
#ifdef WIN_NT
	if (found_dir)
	{
		if (!(file->ext_ifi = (int*) ib_fopen(ib_file_path, FOPEN_TYPE)))
		{
			/* could not open the file as read write attempt as read only */
    		if (!(file->ext_ifi = (int*) ib_fopen(ib_file_path, FOPEN_READ_ONLY)))
				ERR_post(isc_io_error, 
    					 gds_arg_string, "ib_fopen", 
    					 gds_arg_string,
						 ERR_cstring(reinterpret_cast <
									 char *>(file->ext_filename)),
						 isc_arg_gds, isc_io_open_err, SYS_ERR, errno, 0);
			else 
				file->ext_flags |= EXT_readonly;
		}
	}
	else
	{
		if (!(file->ext_ifi = (int*) ib_fopen(file_name, FOPEN_TYPE)))
		{
			/* could not open the file as read write attempt as read only */
			if (!(file->ext_ifi = (int*) ib_fopen(file_name, FOPEN_READ_ONLY)))
				ERR_post(isc_io_error, 
    					 gds_arg_string, "ib_fopen", 
    					 gds_arg_string,
						 ERR_cstring(reinterpret_cast <
									 char *>(file->ext_filename)),
						 isc_arg_gds, isc_io_open_err, SYS_ERR, errno, 0);
			else 
				file->ext_flags |= EXT_readonly;
		}
	}
#else
	if (!(dbb->dbb_flags & DBB_read_only))
		file->ext_ifi = (int *) ib_fopen(file_name, FOPEN_TYPE);
	if (!(file->ext_ifi))
	{
		/* could not open the file as read write attempt as read only */
		if (!(file->ext_ifi = (int *) ib_fopen(file_name, FOPEN_READ_ONLY)))
			ERR_post(isc_io_error,
					 gds_arg_string, "ib_fopen",
					 gds_arg_string,
					 ERR_cstring(reinterpret_cast <
								 char *>(file->ext_filename)),
					isc_arg_gds, isc_io_open_err, SYS_ERR, errno, 0);
		else
			file->ext_flags |= EXT_readonly;
	}
#endif

	return file;
}


void EXT_fini(JRD_REL relation)
{
/**************************************
 *
 *	E X T _ f i n i
 *
 **************************************
 *
 * Functional description
 *	Close the file associated with a relation.
 *
 **************************************/
	EXT file;

	if (relation->rel_file) {
		file = relation->rel_file;
		if (file->ext_ifi)
			ib_fclose((IB_FILE *) file->ext_ifi);
		/* before zeroing out the rel_file we need to deallocate the memory */
		delete relation->rel_file;
		relation->rel_file = 0;
	}
}


int EXT_get(RSB rsb)
{
/**************************************
 *
 *	E X T _ g e t
 *
 **************************************
 *
 * Functional description
 *	Get a record from an external file.
 *
 **************************************/
	TDBB tdbb;
	JRD_REQ request;
	JRD_REL relation;
	EXT file;
	RPB *rpb;
	REC record;
	FMT format;
	LIT literal;
	JRD_FLD field;
	DSC desc;
	SSHORT c, l, offset, i;
	UCHAR *p;
	vec::iterator itr;

	tdbb = GET_THREAD_DATA;

	relation = rsb->rsb_relation;
	file = relation->rel_file;
	request = tdbb->tdbb_request;

	if (request->req_flags & req_abort)
		return FALSE;

	rpb = &request->req_rpb[rsb->rsb_stream];
	record = rpb->rpb_record;
	format = record->rec_format;

	offset = (SSHORT) format->fmt_desc[0].dsc_address;
	p = record->rec_data + offset;
	l = record->rec_length - offset;

	if (file->ext_ifi == 0 ||
		(ib_fseek((IB_FILE *) file->ext_ifi, rpb->rpb_ext_pos, 0) != 0))
		ERR_post(isc_io_error,
				 gds_arg_string, "ib_fseek",
				 gds_arg_string,
				 ERR_cstring(reinterpret_cast < char *>(file->ext_filename)),
				 isc_arg_gds, isc_io_open_err, SYS_ERR, errno, 0);

	while (l--) {
		c = ib_getc((IB_FILE *) file->ext_ifi);
		if (c == EOF)
			return FALSE;
		*p++ = c;
	}
	rpb->rpb_ext_pos = ib_ftell((IB_FILE *) file->ext_ifi);

/* Loop thru fields setting missing fields to either blanks/zeros
   or the missing value */

	fmt::fmt_desc_iterator desc_ptr = format->fmt_desc.begin();

	for (i = 0, itr = relation->rel_fields->begin();
			i < format->fmt_count; ++i, ++itr, ++desc_ptr) {
	    field = (JRD_FLD) (*itr);
		SET_NULL(record, i);
		if (!desc_ptr->dsc_length || !field)
			continue;
		if ( (literal = (LIT) field->fld_missing_value) ) {
			desc = *desc_ptr;
			desc.dsc_address = record->rec_data + (int) desc.dsc_address;
			if (!MOV_compare(&literal->lit_desc, &desc))
				continue;
		}
		CLEAR_NULL(record, i);
	}

	return TRUE;
}


void EXT_modify(RPB * old_rpb, RPB * new_rpb, int *transaction)
{
/**************************************
 *
 *	E X T _ m o d i f y
 *
 **************************************
 *
 * Functional description
 *	Update an external file.
 *
 **************************************/

/* ERR_post (gds_wish_list, gds_arg_interpreted, "EXT_modify: not yet implemented", 0); */
	ERR_post(isc_ext_file_modify, 0);
}


void EXT_open(RSB rsb)
{
/**************************************
 *
 *	E X T _ o p e n
 *
 **************************************
 *
 * Functional description
 *	Open a record stream for an external file.
 *
 **************************************/
	TDBB tdbb;
	JRD_REL relation;
	JRD_REQ request;
	RPB *rpb;
	REC record;
	FMT format;

	tdbb = GET_THREAD_DATA;

	relation = rsb->rsb_relation;
	request = tdbb->tdbb_request;
	rpb = &request->req_rpb[rsb->rsb_stream];

	if (!(record = rpb->rpb_record) || !(format = record->rec_format)) {
		format = MET_current(tdbb, relation);
		VIO_record(tdbb, rpb, format, request->req_pool);
	}

	rpb->rpb_ext_pos = 0;
}


RSB EXT_optimize(register OPT opt, SSHORT stream, JRD_NOD * sort_ptr)
{
/**************************************
 *
 *	E X T _ o p t i m i z e
 *
 **************************************
 *
 * Functional description
 *	Compile and optimize a record selection expression into a
 *	set of record source blocks (rsb's).
 *
 **************************************/
	TDBB tdbb;
	register CSB csb;
	JRD_REL relation;
	RSB rsb_;
/* all these are un refrenced due to the code commented below
JRD_NOD		node, inversion;
register opt::opt_repeat	*tail, *opt_end;
SSHORT		i, size;
*/
	SSHORT size;
	csb_repeat *csb_tail;

	tdbb = GET_THREAD_DATA;

	csb = opt->opt_csb;
	csb_tail = &csb->csb_rpt[stream];
	relation = csb_tail->csb_relation;

/* Time to find inversions.  For each index on the relation
   match all unused booleans against the index looking for upper
   and lower bounds that can be computed by the index.  When
   all unused conjunctions are exhausted, see if there is enough
   information for an index retrieval.  If so, build up and
   inversion component of the boolean. */

/*
inversion = NULL;
opt_end = opt->opt_rpt + opt->opt_count;

if (opt->opt_count)
    for (i = 0; i < csb_tail->csb_indices; i++)
	{
	clear_bounds (opt, idx);
	for (tail = opt->opt_rpt; tail < opt_end; tail++)
	    {
	    node = tail->opt_conjunct;
	    if (!(tail->opt_flags & opt_used) &&
		computable (csb, node, -1))
		match (opt, stream, node, idx);
	    if (node->nod_type == nod_starts)
		compose (&inversion,
			 make_starts (opt, node, stream, idx), nod_bit_and);
	    }
	compose (&inversion, make_index (opt, relation, idx),
		nod_bit_and);
	idx = idx->idx_rpt + idx->idx_count;
	}
*/


	rsb_ = FB_NEW_RPT(*tdbb->tdbb_default,0) Rsb;
	rsb_->rsb_type = rsb_ext_sequential;
	size = sizeof(struct irsb);

	rsb_->rsb_stream = stream;
	rsb_->rsb_relation = relation;
	rsb_->rsb_impure = csb->csb_impure;
	csb->csb_impure += size;

	return rsb_;
}


void EXT_ready(JRD_REL relation)
{
/**************************************
 *
 *	E X T _ r e a d y
 *
 **************************************
 *
 * Functional description
 *	Open an external file.
 *
 **************************************/
}


void EXT_store(RPB * rpb, int *transaction)
{
/**************************************
 *
 *	E X T _ s t o r e
 *
 **************************************
 *
 * Functional description
 *	Update an external file.
 *
 **************************************/
	JRD_REL relation;
	REC record;
	FMT format;
	EXT file;
	JRD_FLD field;
	LIT literal;
	DSC desc;
	UCHAR *p;
	USHORT i, l, offset;

	relation = rpb->rpb_relation;
	file = relation->rel_file;
	record = rpb->rpb_record;
	format = record->rec_format;

/* Loop thru fields setting missing fields to either blanks/zeros
   or the missing value */

/* check if file is read only if read only then
   post error we cannot write to this file */
	if (file->ext_flags & EXT_readonly) {
		DBB dbb;

		dbb = GET_DBB;
		CHECK_DBB(dbb);
		/* Distinguish error message for a ReadOnly database */
		if (dbb->dbb_flags & DBB_read_only)
			ERR_post(isc_read_only_database, 0);
		else
			ERR_post(isc_io_error,
					 gds_arg_string, "insert",
					 gds_arg_string, file->ext_filename,
					 isc_arg_gds, isc_io_write_err,
					 gds_arg_gds, gds_ext_readonly_err, 0);
	}

	vec::iterator field_ptr = relation->rel_fields->begin();
	fmt::fmt_desc_iterator desc_ptr = format->fmt_desc.begin();

	for (i = 0; i < format->fmt_count; i++, field_ptr++, desc_ptr++)
	{
		field = (JRD_FLD)*field_ptr;
		if (field &&
			!field->fld_computation &&
			desc_ptr->dsc_length &&
			TEST_NULL(record, i))
		{
			p = record->rec_data + (int) desc_ptr->dsc_address;
			if ( (literal = (LIT) field->fld_missing_value) ) {
				desc = *desc_ptr;
				desc.dsc_address = p;
				MOV_move(&literal->lit_desc, &desc);
			}
			else {
				l = desc_ptr->dsc_length;
				offset = (desc_ptr->dsc_dtype == dtype_text) ? ' ' : 0;
				do
					*p++ = offset;
				while (--l);
			}
		}
	}

	offset = (USHORT) format->fmt_desc[0].dsc_address;
	p = record->rec_data + offset;
	l = record->rec_length - offset;

	if (file->ext_ifi == 0
		|| (ib_fseek((IB_FILE *) file->ext_ifi, (SLONG) 0, 2) != 0))
		ERR_post(isc_io_error, gds_arg_string, "ib_fseek", gds_arg_string,
				 ERR_cstring(reinterpret_cast < char *>(file->ext_filename)),
				 isc_arg_gds, isc_io_open_err, SYS_ERR, errno, 0);
	for (; l--; ++p)
		ib_putc(*p, (IB_FILE *) file->ext_ifi);
	ib_fflush((IB_FILE *) file->ext_ifi);
}


void EXT_trans_commit(JRD_TRA transaction)
{
/**************************************
 *
 *	E X T _ t r a n s _ c o m m i t
 *
 **************************************
 *
 * Functional description
 *	Checkin at transaction commit time.
 *
 **************************************/
}


void EXT_trans_prepare(JRD_TRA transaction)
{
/**************************************
 *
 *	E X T _ t r a n s _ p r e p a r e
 *
 **************************************
 *
 * Functional description
 *	Checkin at transaction prepare time.
 *
 **************************************/
}


void EXT_trans_rollback(JRD_TRA transaction)
{
/**************************************
 *
 *	E X T _ t r a n s _ r o l l b a c k
 *
 **************************************
 *
 * Functional description
 *	Checkin at transaction rollback time.
 *
 **************************************/
}


void EXT_trans_start(JRD_TRA transaction)
{
/**************************************
 *
 *	E X T _ t r a n s _ s t a r t
 *
 **************************************
 *
 * Functional description
 *	Checkin at start transaction time.
 *
 **************************************/
}


} // extern "C"
