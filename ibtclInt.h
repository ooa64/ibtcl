#ifndef __IBTCLINT_H_COA__
#define __IBTCLINT_H_COA__
/*
* ibtcl
*
* Copyright (C) 1998 Oleg Checkulaev
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*/
/*
$Id: ibtclInt.h,v 0.3 1998/10/09 11:33:12 coa Exp $
*/

#include <ibase.h>
#include "ibtcl.h"


typedef struct {
	Tcl_HashTable db_hash;
	Tcl_HashTable stmt_hash;
	int num_db;
	int num_stmt;
} IB_ClientData;

typedef struct {
	char id[32];
	isc_db_handle dbh;
	isc_tr_handle trh;
	Tcl_HashTable stmt_hash;
	Tcl_Encoding enc;
	Tcl_DString enc_buf;
} IB_Connection;

typedef struct {
	char id[32];
	int f_maxlen; /* max length of fields */
	isc_stmt_handle stmth;
	IB_Connection *con;
	XSQLDA *xd;
} IB_Statement;


/* ibtcl functions themselves */
#ifdef DEBUG
int do_ib_test( ClientData, Tcl_Interp*, int, char** );
#endif
int do_ib_open( ClientData, Tcl_Interp*, int, char** );
int do_ib_close( ClientData, Tcl_Interp*, int, char** );
int do_ib_exec( ClientData, Tcl_Interp*, int, char** );
int do_ib_free_stmt( ClientData, Tcl_Interp*, int, char** );
int do_ib_fetch( ClientData, Tcl_Interp*, int, char** );
int do_ib_fetch2proc( ClientData, Tcl_Interp*, int, char** );
int do_ib_skip( ClientData, Tcl_Interp*, int, char** );
int do_ib_fieldname( ClientData, Tcl_Interp*, int, char** );
int do_ib_fields( ClientData, Tcl_Interp*, int, char** );
int do_ib_isquery( ClientData, Tcl_Interp*, int, char** );

/* Id manipulation functions */
void ib_set_conn_id( IB_ClientData*, IB_Connection* );
IB_Connection* ib_get_conn_id( IB_ClientData*, char* );
void ib_del_conn_id( IB_ClientData*, char* );
void ib_set_stmt_id( IB_ClientData*, IB_Connection*, IB_Statement* );
IB_Statement* ib_get_stmt_id( IB_ClientData*, char* );
void id_del_stmt_id( IB_ClientData, char );

#endif
