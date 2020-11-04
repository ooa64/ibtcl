# ibtcl

Interbase/Firebird extension for Tcl.

Recompiled and tested 04.11.2020 on Windows 10 and RHEL 7.4
                                           
Changed 04.11.2015, bugfix:

        - Fixed SQLDIALECT usage in the ib_open.
        - Support for FLOAT and INT64 db fields.
        - Makefile updated to build shared lib.

Changed 24.10.2015, version 0.1.1:

        - Optional database role and encoding for the ib_open call.
        - Uses UTF-8 conversions.
        - Defines SQLDIALECT in cmd.c (3 by default).
        - Defines TCLDATEFORMAT in cmd.c (string datetime instead of tcltime by default).
        - Can be compiled on Windows, MSVC makefile included.

Original from:

http://ftp.uni-hannover.de/tcl/mirror/ftp.procplace.com/sorted/packages-8.0/databases/ibtcl/1.0/ibtcl-01.tar.gz

Copyright (C) 1998 Oleg Checkulaev


