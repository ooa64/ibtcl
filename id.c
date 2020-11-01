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

#include <stdlib.h>
#include <string.h>
#include "ibtclInt.h"

static char const rcsid[] = "$Id: id.c,v 0.2 1998/10/01 14:18:49 coa Exp $";

void ib_set_conn_id( IB_ClientData *cd, IB_Connection *connid ) {
	Tcl_HashEntry *hent;
	int hnew;

	Tcl_InitHashTable( &(connid->stmt_hash), TCL_STRING_KEYS );

	hent = Tcl_CreateHashEntry( &(cd->db_hash), connid->id, &hnew );
	Tcl_SetHashValue( hent, (ClientData)connid );
}



IB_Connection* ib_get_conn_id( IB_ClientData *cd, char *id) {
	Tcl_HashEntry *hent;

	hent = Tcl_FindHashEntry( &(cd->db_hash), id );
	if( hent == NULL ) {
		return (IB_Connection*)NULL;
	}

	return (IB_Connection*) Tcl_GetHashValue( hent );
}



void ib_del_conn_id( IB_ClientData *cd, char *id ) {
	Tcl_HashEntry *hent, *hent2, *hent3;
	Tcl_HashSearch hsearch;
	IB_Connection *connid;
	IB_Statement *sid;

	hent = Tcl_FindHashEntry( &(cd->db_hash), id );
	if(hent == NULL) {
		return;
	}

	connid = (IB_Connection*) Tcl_GetHashValue( hent );

	hent2 = Tcl_FirstHashEntry( &(connid->stmt_hash), &hsearch );
	while( hent2 != NULL ) {
		sid = (IB_Statement*) Tcl_GetHashValue( hent2 );
		ib_close_stmt( sid );
		hent3 = Tcl_FindHashEntry( &(cd->stmt_hash), sid->id );
		if( hent3 != NULL ) {
			Tcl_DeleteHashEntry( hent3 );
		}
		ckfree( sid );
		hent2 = Tcl_NextHashEntry( &hsearch );
	}

	Tcl_DeleteHashTable( &(connid->stmt_hash) );
	Tcl_DeleteHashEntry( hent );

	return;
}



void ib_set_stmt_id( IB_ClientData *cd, IB_Connection *con, IB_Statement *st ) {
	Tcl_HashEntry *hent;
	int hnew;


	hent = Tcl_CreateHashEntry( &(cd->stmt_hash), st->id, &hnew);
	Tcl_SetHashValue( hent, (ClientData)st );

	if(con != NULL) {
		hent = Tcl_CreateHashEntry( &(con->stmt_hash), st->id, &hnew );
		Tcl_SetHashValue( hent, (ClientData)st );
	}
}



IB_Statement* ib_get_stmt_id( IB_ClientData *cd, char *id) {
	Tcl_HashEntry *hent;

	hent = Tcl_FindHashEntry( &(cd->stmt_hash), id );
	if( hent == NULL ) {
		return (IB_Statement*)NULL;
	}

	return (IB_Statement*) Tcl_GetHashValue( hent );
}



void ib_del_stmt_id( IB_ClientData *cd, char *id ) {
	Tcl_HashEntry	*hent;
	Tcl_HashEntry	*hent2;
	IB_Statement	*stmt;

	hent = Tcl_FindHashEntry( &(cd->stmt_hash), id );
	if( hent==NULL ) {
		return;
	}

	stmt = (IB_Statement*)Tcl_GetHashValue( hent );
	if( stmt->con != NULL ) {
		hent2 = Tcl_FindHashEntry( &(stmt->con->stmt_hash), id );
		if( hent2 != NULL ) {
			Tcl_DeleteHashEntry( hent2 );
		}
	}

	Tcl_DeleteHashEntry( hent );

	return;
}
