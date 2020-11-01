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

#include <stdio.h>
#include "ibtcl.h"

static char const rcsid[] = "$Id: ibtclsh.c,v 0.1 1998/10/01 09:06:10 coa Exp $";

main( int argc, char **argv ) {
	Tcl_Main( argc, argv, ibtcl_init );
}
