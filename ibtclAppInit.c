#include <tcl.h>

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

EXTERN int Ibtcl_Init _ANSI_ARGS_ ((Tcl_Interp *));

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

int Ibtcl_AppInit(Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.1", 0) == 0L) {
        return TCL_ERROR;
    }
#endif

    if (Tcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    if (Ibtcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    Tcl_StaticPackage(interp, "ibtcl", Ibtcl_Init, NULL);
    Tcl_SetVar(interp, "tcl_rcFileName", "~/ibtclshrc.tcl", TCL_GLOBAL_ONLY);
    return TCL_OK;
}
