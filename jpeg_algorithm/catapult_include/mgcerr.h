/*
 * mgcerr.h - Header file for MGC standard status object/internationalized
 * string component.
 *
 * Copyright (C) Mentor Graphics Corporation 1995 All Rights Reserved
 *
 * This component provides internationlized string support based on the Unix
 * NLS message facilities or the WIN32 String resource facilities.
 *
 * HISTORY
 *      v2-1_1-1   12/21/95   JRH  Initial version.
 *      v2-1_2-1   02/15/96   JRH  Added mgcerrPopStatus.
 * 
 */

#ifndef _MGCERR_H
#define _MGCERR_H

#include <stdarg.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif

/* Special declarations needed for Win32/NT dynamic loading   */
/* symbol resolution at run-time. Symbols must be "exported". */
#ifndef DllExport
#ifdef _WIN32
#define DllExport   __declspec(dllexport)
#else
#define DllExport
#endif
#endif

/*
 * Defines used to get around the differences between Win32 and UNIX.
 */
#ifdef _WIN32
#define CHAR _TCHAR
#else
#define CHAR char
#endif

/*
 * Defines for various offsets used to deal with differences in base values
 * used by gencat and the Message Compiler.
 */
#ifdef _WIN32
#define MGCERR_PKG_ID_OFFSET 256
#else
#define MGCERR_PKG_ID_OFFSET 1
#endif
#define MGCERR_MSG_NUM_OFFSET 1

/* Macros used to access default status object values. */

/*
 * Expand to default values for msgId, format, and pkgName parameters.
 */
#define MGCERR_FORMAT(pkg, msgId) msgId,\
    pkg##MgcerrDefaults[(msgId & 0xffff)-MGCERR_MSG_NUM_OFFSET].format,\
    pkg##MgcerrDefaults[(msgId & 0xffff)-MGCERR_MSG_NUM_OFFSET].pkgName

/*
 * Expand to default values for msgId, format, pkgName, severity, and flags
 * parameters.
 */
#define MGCERR_STATUS(pkg, msgId) MGCERR_FORMAT(pkg, msgId),\
    pkg##MgcerrDefaults[(msgId & 0xffff)-MGCERR_MSG_NUM_OFFSET].severity,\
    pkg##MgcerrDefaults[(msgId & 0xffff)-MGCERR_MSG_NUM_OFFSET].flags

/*
 * Return pointer to default status object.
 */
#define MGCERR_STATUS_PTR(pkg, msgId) \
    &pkg##MgcerrDefaults[(msgId & 0xffff)-MGCERR_MSG_NUM_OFFSET]

#ifdef __cplusplus
extern "C" {
#endif

/*
 * mgcerrContextIdT is an opaque handle for an error context.  The current
 * interface definintion only supports one context.
 */
typedef void   *mgcerrContextIdT;

/*
 * mgcerrStatusObjectT defines the data strucure used to store default status
 * object values.
 */
typedef struct _mgcerrStatusObjectT {
    unsigned int msgId;
    const CHAR *format;
    const CHAR *pkgName;
    unsigned int severity;
    unsigned int flags;
} mgcerrStatusObjectT;

/* The severities supported by MGCERR. */

#define MGCERR_ERROR 3
#define MGCERR_INFORMATIONAL 1
#define MGCERR_WARNING 2
#define MGCERR_SUCCESS 0

/*
 * The functions in the API provided by the MGCERR library. See the MGCER
 * Functional Specification for detailed descriptions.
 */

/*
 * Add a status object to the specfied context. Returns zero on success
 * and nonzero on failure.
 */
DllExport int
mgcerrAddStatus(
    mgcerrContextIdT context,  /* context object is added to */
    unsigned int     msgId,    /* ID used to create message */
    const CHAR       *format,  /* format string used to create message */
    const CHAR       *pkgName, /* package name assigned to status object */
    unsigned int     severity, /* severity assigned to object */
    unsigned int     flags,    /* flags assigned to object */
    ...                        /* variant argument list */
);

/*
 * Add a status object to the specfied context using a va_list for arguments.
 * Returns zero on success and nonzero on failure.
 */
DllExport int
mgcerrAddStatusVararg(
    mgcerrContextIdT context,  /* context object is added to */
    unsigned int     msgId,    /* ID used to create message */
    const CHAR       *format,  /* format string used to create message */
    const CHAR       *pkgName, /* package name assigned to status object */
    unsigned int     severity, /* severity assigned to object */
    unsigned int     flags,    /* flags assigned to object */
    va_list          args      /* variant argument list */
);

/*
 * Clear the specified context of all status objects.
 */
DllExport void
mgcerrClearContext(
    mgcerrContextIdT context /* context that is cleared */
);

/*
 * Return the context for the currently executing thread. Returns NULL
 * on failure.
 */
DllExport mgcerrContextIdT
mgcerrContextForThread(void);

/*
 * Copy (add) all status objects from one context to another. Returns zero
 * on success and nonzero on failure.
 */
DllExport int
mgcerrCopyContext(
    mgcerrContextIdT destContext, /* context objects are copied to */
    mgcerrContextIdT srcContext   /* context objects are copied from */
);

/*
 * Copy (add) a status object from one context to another. Returns zero
 * on success and nonzero on failure.
 */
DllExport int
mgcerrCopyStatus(
    mgcerrContextIdT destContext, /* context object is copied to */
    mgcerrContextIdT srcContext,  /* context object is copied from */
    unsigned int item             /* index of object copied */
);

/*
 * Create and initialize a context object. Returns NULL on failure.
 */
DllExport mgcerrContextIdT
mgcerrCreateContext(void);

/*
 * Destroy a context object, freeing all it's resources.
 */
DllExport void
mgcerrDestroyContext(
    mgcerrContextIdT context
);

/*
 * Retrieve a formatted, NULL terminated message string. On success, returns
 * the number of byes copied into the buffer, excluding the NULL character.
 * Note that buffer may not contain a NULL terminated string if the number
 * returned is equal to msgSize. On failure, zero is returned. 
 */
DllExport int
mgcerrFormatMessage(
    unsigned int msgId,    /* ID used to create message */
    const CHAR   *format,  /* format string used to create message */
    const CHAR   *pkgName, /* package name assigned to status object */
    unsigned int msgSize,  /* size of buffer message is copied into */
    CHAR         **msg,    /* pointer to buffer message is copied into */
    ...                    /* variant argument list */
);

/*
 * Retrieve a formatted, NULL terminated message string using a va_list for
 * arguments. On success, returns the number of byes copied into the buffer,
 * excluding the NULL character. Note that buffer may not contain a NULL 
 * terminated string if the number returned is equal to msgSize. On failure, 
 * zero is returned. 
 */
DllExport int
mgcerrFormatMessageVararg(
    unsigned int msgId,    /* ID used to create message */
    const CHAR   *format,  /* format string used to create message */
    const CHAR   *pkgName, /* package name assigned to status object */
    unsigned int msgSize,  /* size of buffer message is copied into */
    CHAR         **msg,    /* pointer to buffer message is copied into */
    va_list      args      /* variant argument list */
);

/*
 * Return the number of status objects in the specified context. On failure
 * a negative value is returned.
 */
DllExport int
mgcerrGetCount(
    mgcerrContextIdT context
);

/*
 * Return the flags for the specified status object. On failure a negative value
 * is returned.
 */
DllExport int
mgcerrGetFlags(
    mgcerrContextIdT context, /* context containing status object */
    unsigned int     item     /* index of status object */
);

/*
 * Return the message for the specified status object. The pointer returned is
 * to an area of memory maintained by MGCERR. The pointer remains valid until the
 * context is destroyed. On failure NULL is returned.
 */
DllExport const CHAR*
mgcerrGetMessage(
    mgcerrContextIdT context, /* context containing status object */
    unsigned int     item     /* index of status object */
);

/*
 * Return the ID of the message for the specified status object. On failure a
 * negative values is returned.
 */
DllExport unsigned int
mgcerrGetMessageId(
    mgcerrContextIdT context, /* context containing status object */
    unsigned int     item     /* index of status object */
);

/*
 * Return the package name of the specified status object. The pointer returned
 * is to an area of memory maintained by MGCERR. The pointer remains valid until
 * the context is destroyed. On failure NULL is returned.
 */
DllExport const CHAR*
mgcerrGetPackage(
    mgcerrContextIdT context, /* context containing status object */
    unsigned int     item     /* index of status object */
);

/*
 * Return the severity of the specified status object. On failure a negative
 * values is returned.
 */
DllExport int
mgcerrGetSeverity(
    mgcerrContextIdT context, /* context containing status object */
    unsigned int     item     /* index of status object */
);

/*
 * Return the version number string for MGCERR. The pointer is to an area of
 * memory maintained by MGCERR.
 */
DllExport const CHAR*
mgcerrGetVersion(void);

/*
 * Pop the top status object off the stack for the specfied context. Returns zero
 * on success and nonzero on failure.
 */
DllExport int
mgcerrPopStatus(
    mgcerrContextIdT context
);

/*
 * Print the messages contained in the status objects in a context. Returns zero
 * on success and nonzero on failure.
 */
DllExport int
mgcerrPrintContext(
	FILE*            fp,       /* file messages are to be printed to */
    mgcerrContextIdT context,  /* context containing status objects */
    int              show_tags /* true = preceed message with severity tag */
);

/*
 * Register a package with native external string mechanism. Returns zero
 * on success and nonzero on failure.
 */
DllExport int
mgcerrRegisterPackage(
    const CHAR   *pkgName,  /* symbolic name for package */
    unsigned int pkgId,     /* ID for package */
    const CHAR   *resource, /* UNIX: catalog name, Win32: resource DLL name */
    int info                /* UNIX: pkgId, Win32: language identifier */
);

/*
 * Set the flags of the specfied status object. Returns zero on success and
 * nonzero on failure.
 */
DllExport int
mgcerrSetFlags(
    mgcerrContextIdT context, /* context containing status object */
    unsigned int     item,    /* index of status object */
    unsigned int     flags    /* flags assigned to object */
);

/*
 * Unregister the specfied package, freeing it's associated resources. Returns
 * zero on success and nonzero on failure.
 */
DllExport int
mgcerrUnregisterPackage(
    const CHAR* pkgName
);

#ifdef __cplusplus
}
#endif

#endif /* _MGCERR_H */
