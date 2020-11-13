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

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ibtclInt.h"

typedef struct {
	short len;
	char data[1];
} VarChar;

static char const rcsid[] = "$Id: cmd.c,v 0.5 1999/01/06 07:27:11 coa Exp coa $";

void ib_del_stmt_id( IB_ClientData *cd, char *id );

static int coercetypes( XSQLDA* );
static void ib_getval( XSQLVAR *var, char *buf );

#define SQLDIALECT 3
#define TCLDATEFORMAT 0
#define ERR_BUFFSIZE 512	
#define MAX_DPB_SIZE 1024

#define ENC_LENGTH( _con_ ) ( Tcl_DStringLength( &( ( _con_ )->enc_buf ) ) )
#define ENC_VALUE( _con_ ) ( Tcl_DStringValue( &( ( _con_ )->enc_buf ) ) )

#ifndef ISC_INT64_FORMAT
#if (defined(_MSC_VER) && defined(WIN32))
#define	ISC_INT64_FORMAT	"I64"
#else
#define	ISC_INT64_FORMAT	"ll"
#endif
#endif


static char * ib_text2tcl( IB_Connection * con, char * s ) {
	if( con ) {
		Tcl_DStringFree( &(con->enc_buf) );
		Tcl_ExternalToUtfDString( con->enc, s, -1, &(con->enc_buf) );
		return Tcl_DStringValue( &(con->enc_buf) );
	} else {
		return s;
	}
};


static char * ib_tcl2text( IB_Connection * con, char * s ) {
	if( con ) {
		Tcl_DStringFree( &(con->enc_buf) );
		Tcl_UtfToExternalDString( con->enc, s, -1, &(con->enc_buf) );
		return Tcl_DStringValue( &(con->enc_buf) );
	} else {
		return s;
	}
};


static int ib_init_encoding( Tcl_Interp* ip, IB_Connection * con, char * e ) {
	if( con ) {
		if( e && *e ) {
			con->enc = Tcl_GetEncoding( ip, e );
			if( !con->enc ) {
				return TCL_ERROR;
			}
		} else {
			con->enc = NULL;
		}
		Tcl_DStringInit( &(con->enc_buf) );
	}
	return TCL_OK;
}


static void ib_free_encoding( IB_Connection * con ) {
	if( con ) {
		Tcl_DStringFree( &(con->enc_buf) );
		if( con->enc ) {
			Tcl_FreeEncoding( con->enc );
			con->enc = NULL;
		}
	}
}


static void ib_append_dpb( IB_Connection *con, char * dpb, short * dpblen, char t, char * s ) {
	int l = strlen( s );
	int is_text = 1; /* username, password, etc */
	if( is_text && con ) {
		ib_tcl2text( con, s );
		l = ENC_LENGTH( con );
		s = ENC_VALUE( con );
	}
	if( s && l < 256 && ( ( * dpblen ) + 2 + l ) < MAX_DPB_SIZE ) {
		dpb[ (* dpblen )++ ] = t;
		dpb[ (* dpblen )++ ] = l;
		char i = 0;
		while ( i < l ) {
			dpb[ ( * dpblen )++ ] = s[ i++ ];
		}
	}
}


/* ib_int64str( buf, (ISC_INT64 *) var->sqldata, var->sqlscale ) */
static char * ib_int64str( char* buf, ISC_INT64* d, short dscale ) {
	if( dscale < 0 ) {
		ISC_INT64 tens = 1;
		short i;
		for( i = 0; i > dscale; i-- )
			tens *= 10;
		if( *d >= 0 )
			sprintf( buf, "%" ISC_INT64_FORMAT "d.%0*" ISC_INT64_FORMAT "d",
				(ISC_INT64) (*d / tens), -dscale, (ISC_INT64) (*d % tens) );
		else if( (*d / tens) != 0 )
			sprintf( buf, "%" ISC_INT64_FORMAT "d.%0*" ISC_INT64_FORMAT "d",
				(ISC_INT64) (*d / tens), -dscale, (ISC_INT64) -(*d % tens) );
		else
			sprintf( buf, "%s.%0*" ISC_INT64_FORMAT "d",
				"-0", -dscale, (ISC_INT64) -(*d % tens) );
	} else if( dscale ) {
		sprintf( buf, "%" ISC_INT64_FORMAT "d%0*d", (ISC_INT64) *d, dscale, 0 );
	} else {
		sprintf( buf , "%" ISC_INT64_FORMAT "d", (ISC_INT64) *d );
	}
	return buf;
}


static int ib_err( Tcl_Interp* ip, long **p ) {
	char err[ERR_BUFFSIZE];
	Tcl_DString ds;

	Tcl_DStringInit( &ds );
	fb_interpret( err, ERR_BUFFSIZE, p );
	Tcl_DStringAppend( &ds, err, -1 );

	while( fb_interpret( err, ERR_BUFFSIZE, p ) ) {
		Tcl_DStringAppend( &ds, "\n- ", -1 );
		Tcl_DStringAppend( &ds, err, -1 );
	}

	Tcl_DStringResult( ip, &ds );
	Tcl_DStringFree( &ds );

	return TCL_ERROR;
}


int ib_close_conn( Tcl_Interp *ip, IB_Connection *con ) {
	long stat[20], *statp = stat;

	if( con->trh ) {
		if( isc_commit_transaction( stat, &con->trh ) ) {
			if( ip != NULL ) {
				return ib_err( ip, &statp );
			}
		}
	}

	if( isc_detach_database( stat, &con->dbh ) ) {
		if( ip != NULL ) {
			return ib_err( ip, &statp );
		}
	}

	return TCL_OK;
}



void ib_close_stmt( IB_Statement *s ) {
	long stat[20], *statp = stat;
	int i;

	if( s->xd != NULL ) {
		for( i=0 ; i<s->xd->sqld ; ++i ) {
			XSQLVAR *var = &(s->xd->sqlvar[i]);
			ckfree( var->sqldata );
			if( var->sqltype & 1 ) {
				ckfree( (char *)var->sqlind );
			}
		}
		ckfree( (char *)s->xd );
		s->xd = NULL;
	}

	if( s->stmth != 0L ) {
		isc_dsql_free_statement( stat, &s->stmth, DSQL_drop );
		s->stmth = 0L;
	}

	return;
}



/*
ib_open database user password
	- opens connection to server
	- returns connection id
*/
int do_ib_open( ClientData cData, Tcl_Interp* ip, int argc, char** argv ) {
	IB_ClientData* cd = (IB_ClientData*) cData;
	IB_Connection *con;
	char *db, *usr, *pass, *role, *enc, *set, *dpb;
	long stat[20], *statp = stat;
	isc_db_handle dbh;
	isc_tr_handle trh;
	short l;

	if( argc < 4 || argc > 7 ) {
		Tcl_AppendResult( ip, "ib_open: Wrong # of arguments\n",
				"ib_open database user password ?role? ?encoding? ?charset?", NULL );
		return TCL_ERROR;
	}

	con = (IB_Connection*) ckalloc( sizeof(IB_Connection) );

	db = argv[1]; usr = argv[2]; pass = argv[3];
	role = argc > 4 ? argv[4] : NULL; 
	enc = argc > 5 ? argv[5] : NULL; 
	set = argc > 6 ? argv[6] : NULL; 

	if( enc ) {
		if( ib_init_encoding( ip, con, enc ) != TCL_OK ) {
			isc_commit_transaction( stat, &con->trh );
			ckfree( (char *)con );
			return TCL_ERROR;
		}
	} else {
		ib_init_encoding( ip, con, NULL );
	}

	/* From InterBase API
	l = 4;
	con->dpb = t  = (char*)ckalloc( l*sizeof(char) );
	*t++ = isc_dpb_version1;
	*t++ = isc_dpb_num_buffers;
	*t++ = 1;
	*t++ = (char)90;
	*/

	l = 0;
	dpb = ckalloc( MAX_DPB_SIZE ); /* recommended limit */

	dpb[l++] = isc_dpb_version1;

	dpb[l++] = isc_dpb_sql_dialect;
	dpb[l++] = ( char )1;
	dpb[l++] = ( char )SQLDIALECT;

	dpb[l++] = isc_dpb_utf8_filename;
	dpb[l++] = ( char )1;
	dpb[l++] = ( char )1;
   
	ib_append_dpb( con, dpb, &l, isc_dpb_user_name, usr );
	ib_append_dpb( con, dpb, &l, isc_dpb_password, pass );
	if ( role ) {
		ib_append_dpb( con, dpb, &l, isc_dpb_sql_role_name, role );
	}
	if( set ) {
		ib_append_dpb( NULL, dpb, &l, isc_dpb_lc_ctype, set );
		ib_append_dpb( NULL, dpb, &l, isc_dpb_lc_messages, set );
	}

	dbh = 0L;
	isc_attach_database( stat, strlen(db), db, &dbh, l, dpb );
	ckfree( (char *)dpb );
	if( stat[0]==1 && stat[1] ) {
		ib_free_encoding( con );
		ckfree( (char *)con );
		return ib_err( ip, &statp );
	}

	trh = 0L;
	if( isc_start_transaction( stat, &trh, 1, &dbh, 0, NULL ) ) {
		ib_free_encoding( con );
		ckfree( (char *)con );
		return ib_err( ip, &statp );
	}

	con->dbh = dbh;
	con->trh = trh;
	sprintf( con->id, "ibc%d", cd->num_db++ );
	ib_set_conn_id( cd, con );

	Tcl_AppendResult( ip, con->id, NULL );
	return TCL_OK;
}


/*
ib_close db_handler
	- closes connection to database
	- returns nothing in success
*/
int do_ib_close( ClientData cData, Tcl_Interp* ip, int argc, char** argv ) {
	IB_ClientData *cd = (IB_ClientData*) cData;
	IB_Connection *con;
	int res;

	if( argc != 2 ) {
		Tcl_AppendResult( ip, "ib_close: Wrong # of arguments\n",
				"ib_open db_handler", NULL );
		return TCL_ERROR;
	}

	con = ib_get_conn_id( cd, argv[1] );
	if( con == NULL ) {
		Tcl_AppendResult( ip, 
			"ib_close: invalid database handler", NULL );
		return TCL_ERROR;
	}

	ib_free_encoding( con );

	ib_del_conn_id( cd, argv[1] );
	res = ib_close_conn( ip, con );
	if( res != TCL_OK ) return res;

	ckfree( (char *)con );

	return TCL_OK;
}



/*
ib_exec db_handler statement
	- executes statement
	- returns statement handler
*/
int do_ib_exec( ClientData cData, Tcl_Interp* ip, int argc, char** argv ) {
	IB_ClientData *cd = (IB_ClientData*) cData;
	IB_Connection *con;
	IB_Statement *st;
	XSQLDA *xd;
	isc_stmt_handle stmth;
	long stat[20], *statp = stat;
	int i;

	if( argc != 3 ) {
		Tcl_AppendResult( ip, "ib_exec: Wrong # of arguments\n",
				"ib_exec db_handler statement", NULL );
		return TCL_ERROR;
	}

	con = ib_get_conn_id( cd, argv[1] );
	if( con == NULL ) {
		Tcl_AppendResult( ip, 
			"ib_exec: invalid database handler", NULL );
		return TCL_ERROR;
	}

	if( con->trh == 0 ) {
	/* latest statement was commit, is not it ? */
		if( isc_start_transaction( stat, &(con->trh), 1, &(con->dbh), 0, NULL ) ) {
			return ib_err( ip, &statp );
		}
	}
	
	stmth = 0L;
	if( isc_dsql_allocate_statement( stat, &(con->dbh), &stmth ) ) {
		return ib_err( ip, &statp );
	}

	xd = (XSQLDA*) ckalloc( XSQLDA_LENGTH(10) );
	memset( xd, 0x0, XSQLDA_LENGTH(10) );
	xd->version = SQLDA_VERSION1;
	xd->sqln = 10;

	if( isc_dsql_prepare( stat, &(con->trh), &stmth, 0, ib_tcl2text( con, argv[2] ), SQLDIALECT, xd ) ) {
		return ib_err( ip, &statp );
	}

	if( xd->sqld > xd->sqln ) {
		int n = xd->sqld;
		
		ckfree( (char *)xd );
		xd = (XSQLDA*) ckalloc( XSQLDA_LENGTH(n) );
		memset( xd, 0x0, XSQLDA_LENGTH(n) );
		
		xd->sqln = n;
		xd->version = SQLDA_VERSION1;
		isc_dsql_describe( stat, &stmth, SQLDIALECT, xd );
	}
	

	if( isc_dsql_execute( stat, &(con->trh), &stmth, SQLDIALECT, NULL )  ) {
		return ib_err( ip, &statp );
	}

	if( xd->sqld == 0 ) { /* It's not a select statement */
		ckfree( (char *)xd );
		if( isc_dsql_free_statement( stat, &stmth, DSQL_drop ) ) {
			return ib_err( ip, &statp );
		}
		Tcl_AppendResult( ip, "ok", NULL );
		return TCL_OK;
	}

	if( coercetypes( xd ) ) {
		for( i=0 ; i<xd->sqld ; ++i ) {
			if( xd->sqlvar[i].sqldata != NULL ) ckfree( (char *)xd->sqlvar[i].sqldata );
			if( xd->sqlvar[i].sqlind != NULL ) ckfree( (char *)xd->sqlvar[i].sqlind );
		}
		ckfree( (char *)xd );
		(void) isc_dsql_free_statement( stat, &stmth, DSQL_drop );

		Tcl_AppendResult( ip, 
			"ib_exec: can't handle data type statement", NULL );
		return TCL_ERROR;
	}

	st = (IB_Statement*)ckalloc( sizeof(IB_Statement) );
	st->con = con;
	st->stmth = stmth;
	st->xd = xd;

	st->f_maxlen = 80;
	for( i=0 ; i<xd->sqld ; ++i ) {
		if( xd->sqlvar[i].sqllen > st->f_maxlen ) {
			st->f_maxlen = xd->sqlvar[i].sqllen;
		}
		xd->sqlvar[i].aliasname[ xd->sqlvar[i].aliasname_length ] = '\0';
	}

	sprintf( st->id, "ibs%d", cd->num_stmt++ );
	ib_set_stmt_id( cd, con, st );

	Tcl_AppendResult( ip, st->id, NULL );
	return TCL_OK;
}



/*
ib_isquery stmt_handler
	- checks is statement has data
	- returns 1 if statement is query and 0 else
*/
int do_ib_isquery( ClientData cData, Tcl_Interp* ip, int argc, char** argv ) {
	IB_ClientData *cd = (IB_ClientData*) cData;
	IB_Statement *st;

	if( argc != 2 ) {
		Tcl_AppendResult( ip, "ib_isquery: Wrong # of arguments\n",
				"ib_isquery stmt_handler", NULL );
		return TCL_ERROR;
	}

	if( strcmp( argv[1], "ok" ) == 0 ) {
		Tcl_AppendResult( ip, "0", NULL );
		return TCL_OK;
	}

	st = ib_get_stmt_id( cd, argv[1] );
	if( st == NULL ) {
		Tcl_AppendResult( ip, 
			"ib_isquery: invalid statement handler", NULL );
		return TCL_ERROR;
	}

	Tcl_AppendResult( ip, "1", NULL );
	return TCL_OK;
}



/*
ib_free stmt_handler
	- closes statement handle
	- returns nothing in success
*/
int do_ib_free_stmt( ClientData cData, Tcl_Interp* ip, int argc, char** argv ) {
	IB_ClientData *cd = (IB_ClientData*) cData;
	IB_Statement *st;

	if( argc != 2 ) {
		Tcl_AppendResult( ip, "ib_free: Wrong # of arguments\n",
				"ib_free stmt_handler", NULL );
		return TCL_ERROR;
	}

	st = ib_get_stmt_id( cd, argv[1] );
	if( st == NULL ) {
		Tcl_AppendResult( ip, 
			"ib_free: invalid statement handler", NULL );
		return TCL_ERROR;
	}

	ib_del_stmt_id( cd, argv[1] );
	ib_close_stmt( st );
	ckfree( (char *)st );

	return TCL_OK;
}


/*
ib_fetch [-name] [-n rows] stmt_handle varname
	- fetches all or atmost rows tuples from statement handle
	  into the specificied variable. Former value of variable
	  will be lost. Result array is indexed by numbers i.e.
	  (0,0) or by field name and tuple number i.e. (VAL,0) if 
	  -name option used.
	- returns nothing in success
*/
int do_ib_fetch( ClientData cData, Tcl_Interp* ip, int argc, char** argv ) {
	IB_ClientData *cd = (IB_ClientData*) cData;
	IB_Statement *st;
	long fstat, stat[20], *statp = stat;
	char *sp, *vp, *buf, vbuf[80];
	int i, j, num, byname;

	byname = 0; num = 0;
	i = 1;
	while( i<argc && argv[i][0] == '-' ) {
		char *p = argv[i];
		if( strcmp( p, "-name" ) == 0 ) {
			byname = 1;
			++i;
			continue;
		}
		if( strcmp( p, "-n" ) == 0 ) {
			if( i >= argc-1 ) {
				Tcl_AppendResult( ip, 
					"ib_fetch: -n must followed by number", NULL );
				return TCL_ERROR;
			}
			
			num = atoi( argv[i+1] );
			i += 2;
			continue;
		}

		Tcl_AppendResult( ip,
			"ib_fetch: unknown option", NULL );

		return TCL_ERROR;
	}

	if( i >= argc-1 ) {
		Tcl_AppendResult( ip, "ib_fetch: Wrong # of arguments\n",
			"Usage: ib_fetch [-name] [-n tuples] stmt_handler varname", NULL );

		return TCL_ERROR;
	}

	sp = argv[i++];
	vp = argv[i];

	st = ib_get_stmt_id( cd, sp );
	if( st == NULL ) {
		Tcl_AppendResult( ip, 
			"ib_fetch: invalid statement handler", NULL );
		return TCL_ERROR;
	}

	buf = (char*)alloca( sizeof(char)*(st->f_maxlen+2) );
	if( buf == NULL ) {
		Tcl_AppendResult( ip,
			"ib_fetch: no memory", NULL );
		return TCL_ERROR;
	}

	Tcl_UnsetVar( ip, vp, 0 );
	i = 0;
	while( (fstat=isc_dsql_fetch( stat, &st->stmth, SQLDIALECT, st->xd)) == 0 ) {

		for( j=0 ; j<st->xd->sqld ; ++j ) {
			ib_getval( &(st->xd->sqlvar[j]), buf );
			
			if( byname ) {
				sprintf( vbuf, "%s(%s,%d)", vp, ib_text2tcl( st->con, st->xd->sqlvar[j].aliasname ), i );
			} else {
				sprintf( vbuf, "%s(%d,%d)", vp, i, j );
			}

			if( Tcl_SetVar( ip, vbuf, ib_text2tcl( st->con, buf ), TCL_LEAVE_ERR_MSG ) == NULL ) {
				return TCL_ERROR;
			}
		}

		++i;
		if( num>0 && i>=num ) break;
	}

	sprintf( vbuf, "%s(cols)", vp ); sprintf( buf, "%d", st->xd->sqld );
	if( Tcl_SetVar( ip, vbuf, buf, TCL_LEAVE_ERR_MSG ) == NULL ) {
		return TCL_ERROR;
	}
	
	sprintf( vbuf, "%s(rows)", vp ); sprintf( buf, "%d", i );
	if( Tcl_SetVar( ip, vbuf, buf, TCL_LEAVE_ERR_MSG ) == NULL ) {
		return TCL_ERROR;
	}

	if( fstat!=100 && fstat!=0 ) {
		return ib_err( ip, &statp );
	}

	return TCL_OK;
}



/*
ib_fetch2proc [-name] [-n rows] stmt_handler varname proc
	- fetches all or atmost rows tuples from statement handle.
	  For each of tuple procedure will be executed. Tuple
	  is stored in varname, indexed by field names if -name
	  option supplied or bye field number.
	- returns nothing in success
*/
int do_ib_fetch2proc( ClientData cData, Tcl_Interp* ip, int argc, char** argv ) {
	IB_ClientData *cd = (IB_ClientData*) cData;
	IB_Statement *st;
	long fstat, stat[20], *statp = stat;
	char *sp, *vp, *pp, *buf, vbuf[80];
	int i, j, num, byname;

	byname = 0; num = 0;
	i = 1;
	while( i<argc && argv[i][0] == '-' ) {
		char *p = argv[i];
		if( strcmp( p, "-name" ) == 0 ) {
			byname = 1;
			++i;
			continue;
		}
		if( strcmp( p, "-n" ) == 0 ) {
			if( i >= argc-1 ) {
				Tcl_AppendResult( ip, 
					"ib_fetch2proc: -n must followed by number", NULL );
				return TCL_ERROR;
			}
			
			num = atoi( argv[i+1] );
			i += 2;
			continue;
		}

		Tcl_AppendResult( ip,
			"ib_fetch2proc: unknown option", NULL );

		return TCL_ERROR;
	}

	if( i >= argc-2 ) {
		Tcl_AppendResult( ip, "ib_fetch: Wrong # of arguments\n",
			"Usage: ib_fetch2proc [-name] [-n tuples] stmt_handler varname proc", NULL );

		return TCL_ERROR;
	}

	sp = argv[i++];
	vp = argv[i++];
	pp = argv[i];

	st = ib_get_stmt_id( cd, sp );
	if( st == NULL ) {
		Tcl_AppendResult( ip, 
			"ib_fetch2proc: invalid statement handler", NULL );
		return TCL_ERROR;
	}

	buf = (char*)alloca( sizeof(char)*(st->f_maxlen+2) );
	if( buf == NULL ) {
		Tcl_AppendResult( ip,
			"ib_fetch2proc: no memory", NULL );
		return TCL_ERROR;
	}

	Tcl_UnsetVar( ip, vp, 0 );
	i = 0;
	while( (fstat=isc_dsql_fetch( stat, &st->stmth, SQLDIALECT, st->xd)) == 0 ) {
		for( j=0 ; j<st->xd->sqld ; ++j ) {
			if( byname ) {
				sprintf( vbuf, "%s(%s)", vp, ib_text2tcl( st->con,st->xd->sqlvar[j].aliasname ));
			} else {
				sprintf( vbuf, "%s(%d)", vp, j );
			}
			
			ib_getval( &(st->xd->sqlvar[j]), buf );

			if( Tcl_SetVar( ip, vbuf, ib_text2tcl( st->con, buf ), TCL_LEAVE_ERR_MSG ) == NULL ) {
				return TCL_ERROR;
			}
		}

		if( Tcl_Eval( ip, pp ) != TCL_OK ) {
			return TCL_ERROR;
		}
		++i;
		if( num>0 && i>=num ) break;
	}

	Tcl_UnsetVar( ip, vp, 0 );

	if( fstat!=100 && fstat!=0 ) {
		return ib_err( ip, &statp );
	}

	return TCL_OK;
}



/*
ib_skip rows stmt_handler
	- skips rows tuples 
	- returns nothing in success
*/
int do_ib_skip( ClientData cData, Tcl_Interp* ip, int argc, char** argv ) {
	IB_ClientData *cd = (IB_ClientData*) cData;
	IB_Statement *st;
	long fstat, stat[20], *statp = stat;
	int i, num;

	if( argc!=3 ) {
		Tcl_AppendResult( ip, "ib_skip: Wrong # of arguments\n",
				"ib_skip rows stmt_handler", NULL );
		return TCL_ERROR;
	}

	num = atoi( argv[1] );
	if( num<0 ) num = 0;

	st = ib_get_stmt_id( cd, argv[2] );
	if( st == NULL ) {
		Tcl_AppendResult( ip, 
			"ib_skip: invalid statement handler", NULL );
		return TCL_ERROR;
	}

	for( i=0 ; i<num ; ++i ) {
		if( (fstat=isc_dsql_fetch( stat, &st->stmth, SQLDIALECT, st->xd)) != 0 ) {
			if( fstat!=100 ) {
				return ib_err( ip, &statp );
			}
		}
	}

	return TCL_OK;
}



/*
ib_fieldname stmt_handler field_number
	- returns field's name
*/
int do_ib_fieldname( ClientData cData, Tcl_Interp* ip, int argc, char** argv ) {
	IB_ClientData *cd = (IB_ClientData*) cData;
	IB_Statement *st;
	int col;

	if( argc != 3 ) {
		Tcl_AppendResult( ip, "ib_fieldname: Wrong # of arguments\n",
				"ib_fieldname stmt_handler col_number", NULL );
		return TCL_ERROR;
	}

	st = ib_get_stmt_id( cd, argv[1] );
	if( st == NULL ) {
		Tcl_AppendResult( ip, 
			"ib_fieldname: invalid statement handler", NULL );
		return TCL_ERROR;
	}

	col = atoi( argv[2] );
	if( col<0 || col>=st->xd->sqld ) {
		Tcl_AppendResult( ip, 
			"ib_fieldname: invalid column number", NULL );
		return TCL_ERROR;
	}

	Tcl_AppendResult( ip, ib_text2tcl( st->con, st->xd->sqlvar[col].aliasname ), NULL );

	return TCL_OK;
}



/*
ib_fields stmt_handler
	- returns number of field in reply
*/
int do_ib_fields( ClientData cData, Tcl_Interp* ip, int argc, char** argv ) {
	IB_ClientData *cd = (IB_ClientData*) cData;
	IB_Statement *st;
	char buf[80];

	if( argc != 2 ) {
		Tcl_AppendResult( ip, "ib_fields: Wrong # of arguments\n",
				"ib_fields stmt_handler", NULL );
		return TCL_ERROR;
	}

	st = ib_get_stmt_id( cd, argv[1] );
	if( st == NULL ) {
		Tcl_AppendResult( ip, 
			"ib_fields: invalid statement handler", NULL );
		return TCL_ERROR;
	}

	sprintf( buf, "%d", st->xd->sqld );
	Tcl_AppendResult( ip, buf, NULL );
	return TCL_OK;
}



#ifdef DEBUG
/*
ib_test []
	- test routing
*/
int do_ib_test( ClientData cData, Tcl_Interp* ip, int argc, char** argv ) {
/*
	int c, i;
	char *p, **a;

	if( argc!=2 ) {
		Tcl_AppendResult( ip, "usage: ib_test listname", NULL );
		return TCL_ERROR;
	}

	p = (char *)Tcl_GetVar( ip, argv[1], TCL_LEAVE_ERR_MSG );
	if( p == NULL ) {
		return TCL_ERROR;	
	}

	if( Tcl_SplitList( ip, p, &c, &a ) != TCL_OK ) {
		return TCL_ERROR;
	}

	printf( "There are %d items in the list\n", c );
	for( i=0 ; i<c ; ++i ) {
		printf( "%s\n", a[i] );
	}

	free( (char*)a );
*/
	return TCL_OK;
}
#endif


/*
	Static staff begins here
*/

static void ib_getval( XSQLVAR *var, char *buf ) {

	if( var->sqltype & 1 ) {
		if( var->sqlind[0] == -1 ) {
			buf[0] = '\0';
			return ;
		}
	}

	switch( var->sqltype & ~1 ) {
			case SQL_VARYING: {
				VarChar *d;
				d = (VarChar*) var->sqldata;
				d->data[d->len] = '\0';
				strcpy( buf, d->data );
				return;
			}
			
			case SQL_TEXT:
				var->sqldata[ var->sqllen ] = '\0';
				strcpy( buf, var->sqldata );
				return;

			case SQL_SHORT: {
				short *d = (short*)var->sqldata;
				sprintf( buf, "%d", *d );
				return;
			}
			
			case SQL_FLOAT: {
				float *d = (float*)var->sqldata;
				sprintf( buf, "%f", *d );
				return;
			}
			
			case SQL_LONG: {
				long *d = (long*)var->sqldata;
				sprintf( buf, "%d", *d );
				return;
			}
			
			case SQL_DOUBLE: {
				double *d = (double*)var->sqldata;
				sprintf( buf, "%f", *d );
				return;
			}

			case SQL_D_FLOAT: {
				double *d = (double*)var->sqldata;
				sprintf( buf, "%f", *d );
				return;
			}

			case SQL_DATE: {
				struct tm ctm;
#if TCLDATEFORMAT
				time_t sec;
				isc_decode_date( (ISC_QUAD*)var->sqldata, &ctm );
				ctm.tm_isdst = 0;
				sec = mktime( &ctm );
				sprintf( buf, "%ld", sec );
#else
				isc_decode_date( (ISC_QUAD*)var->sqldata, &ctm );
				sprintf( buf, "%04d-%02d-%02d %02d:%02d:%02d", ctm.tm_year+1900, ctm.tm_mon+1, ctm.tm_mday, ctm.tm_hour, ctm.tm_min, ctm.tm_sec );
#endif
				return;
			}

			case SQL_TYPE_DATE: {
				struct tm ctm;
#if TCLDATEFORMAT
				time_t sec;
				isc_decode_sql_date( (ISC_DATE*)var->sqldata, &ctm );
				ctm.tm_isdst = 0;
				sec = mktime( &ctm );
				sprintf( buf, "%ld", sec );
#else
				isc_decode_sql_date( (ISC_DATE*)var->sqldata, &ctm );
				sprintf( buf, "%04.4d-%02.2d-%02.2d", ctm.tm_year+1900, ctm.tm_mon+1, ctm.tm_mday );
#endif
				return;
			}

			case SQL_TYPE_TIME: {
				struct tm ctm;
#if TCLDATEFORMAT
				time_t sec;
				isc_decode_sql_time( (ISC_TIME*)var->sqldata, &ctm );
				ctm.tm_isdst = 0;
				ctm.tm_year = 70;
				ctm.tm_mon = 0;
				ctm.tm_mday = 1;
				sec = mktime( &ctm );
				sprintf( buf, "%ld", sec );
#else
				isc_decode_sql_time( (ISC_TIME*)var->sqldata, &ctm );
				sprintf( buf, "%02.2d:%02.2d:%02.2d", ctm.tm_hour, ctm.tm_min, ctm.tm_sec );
#endif
				return;
			}

			case SQL_INT64: {
				ib_int64str( buf, (ISC_INT64*)var->sqldata, var->sqlscale );
				return;
			}
			
			case SQL_QUAD:
				strcpy( buf, "Unsupported type QUAD" );
				return;

			default:
				strcpy( buf, "Unsupported type" );
				return;
	}

	return;
}



static int coercetypes( XSQLDA* da ) {
	XSQLVAR* var;
	int i;
	short dtype;
		
	for( i=0, var = da->sqlvar ; i<da->sqld ; ++i, ++var ) {
		dtype = (var->sqltype & ~1);
		switch( dtype ) {
			case SQL_VARYING:
				var->sqldata = (char*)ckalloc( sizeof(char)*(var->sqllen*2 + 2) );
				break;
			case SQL_TEXT:
				var->sqldata = (char*)ckalloc( sizeof(char)*var->sqllen*2 );
				break;
			case SQL_SHORT:
				var->sqldata = (char*)ckalloc( sizeof(short)*1 );
				break;
			case SQL_FLOAT:
				var->sqldata = (char*)ckalloc( sizeof(float)*1 );
				break;
			case SQL_LONG:
				var->sqldata = (char*)ckalloc( sizeof(long)*1 );
				break;
			case SQL_DOUBLE:
				var->sqldata = (char*)ckalloc( sizeof(double)*1 );
				break;
			case SQL_D_FLOAT:
				var->sqldata = (char*)ckalloc( sizeof(double)*1 );
				break;
			case SQL_DATE:
				var->sqldata = (char*)ckalloc( sizeof(ISC_QUAD)*1 );
				break;
			case SQL_TYPE_DATE:
				var->sqldata = (char*)ckalloc( sizeof(ISC_DATE)*1 );
				break;
			case SQL_TYPE_TIME:
				var->sqldata = (char*)ckalloc( sizeof(ISC_TIME)*1 );
				break;
			case SQL_INT64:
				var->sqldata = (char*)ckalloc( sizeof(ISC_INT64)*1 );
				break;
			case SQL_QUAD:
				return 1;
			default:
				return 1;
		}

		if( var->sqltype & 1 ) {
			var->sqlind = (short*) ckalloc( sizeof(short) );
		}
	}

	return 0;
}

