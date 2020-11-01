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
#include "ibtclInt.h"

static char const rcsid[] = "$Id: ibtcl.c,v 0.3 1998/11/02 07:50:06 coa Exp coa $";

int ib_close_conn( Tcl_Interp *ip, IB_Connection *con );

static void ibtcl_AtExit( ClientData cData ) {
	IB_ClientData *cd = (IB_ClientData *)cData;
	Tcl_HashEntry *hent;
	Tcl_HashSearch hsearch;
	IB_Connection *con;

	while( (hent=Tcl_FirstHashEntry(&(cd->db_hash),&hsearch)) != NULL ) {
		con = (IB_Connection*)Tcl_GetHashValue( hent );
		ib_del_conn_id( cd, con->id );
		ib_close_conn( NULL, con );
	}

	Tcl_DeleteHashTable( &(cd->db_hash) );
	Tcl_DeleteHashTable( &(cd->stmt_hash) );

	Tcl_DeleteExitHandler( ibtcl_AtExit, cData);
}


static void ibtcl_shutdown( ClientData cData, Tcl_Interp *interp ) {
	ibtcl_AtExit( cData );
}



EXTERN int Ibtcl_Init (Tcl_Interp *interp) {
	IB_ClientData *cd;

#ifdef USE_TCL_STUBS
	if (Tcl_InitStubs(interp, "8.1", 0) == 0L) {
		return TCL_ERROR;
	}
#endif

	cd = (IB_ClientData*) ckalloc( sizeof(IB_ClientData) );
	Tcl_InitHashTable( &(cd->db_hash), TCL_STRING_KEYS );
	Tcl_InitHashTable( &(cd->stmt_hash), TCL_STRING_KEYS );
	cd->num_db = 0;
	cd->num_stmt = 0;

	Tcl_CallWhenDeleted( interp, ibtcl_shutdown, (ClientData)cd );
	Tcl_CreateExitHandler( ibtcl_AtExit, (ClientData)cd );

	Tcl_CreateCommand(interp, "ib_test", (Tcl_CmdProc *)do_ib_test, (ClientData)cd, (Tcl_CmdDeleteProc*)NULL);

	Tcl_CreateCommand(interp, "ib_open", (Tcl_CmdProc *)do_ib_open, (ClientData)cd, (Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "ib_close", (Tcl_CmdProc *)do_ib_close, (ClientData)cd, (Tcl_CmdDeleteProc*)NULL);

	Tcl_CreateCommand(interp, "ib_exec", (Tcl_CmdProc *)do_ib_exec, (ClientData)cd, (Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "ib_free_stmt", (Tcl_CmdProc *)do_ib_free_stmt, (ClientData)cd, (Tcl_CmdDeleteProc*)NULL);

	Tcl_CreateCommand(interp, "ib_fetch", (Tcl_CmdProc *)do_ib_fetch, (ClientData)cd, (Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "ib_fetch2proc", (Tcl_CmdProc *)do_ib_fetch2proc, (ClientData)cd, (Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "ib_skip", (Tcl_CmdProc *)do_ib_skip, (ClientData)cd, (Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "ib_fieldname", (Tcl_CmdProc *)do_ib_fieldname, (ClientData)cd, (Tcl_CmdDeleteProc*)NULL);
	Tcl_CreateCommand(interp, "ib_fields", (Tcl_CmdProc *)do_ib_fields, (ClientData)cd, (Tcl_CmdDeleteProc*)NULL);

	Tcl_CreateCommand(interp, "ib_isquery", (Tcl_CmdProc *)do_ib_isquery, (ClientData)cd, (Tcl_CmdDeleteProc*)NULL);

	Tcl_PkgProvide( interp, "ibtcl", "0.1" );
	
	return TCL_OK;
}


EXTERN int Ibtcl_SafeInit( Tcl_Interp *ip ) {
	return Ibtcl_Init( ip );
}
