/*
 * HEADER
 *    mti2east.h - produce EAST from HDL's using the MTI IDL interfaces
 *
 * COPYRIGHT
 *    Copyright (c) MENTOR GRAPHICS CORPORATION 1995 All Rights Reserved 
 *
 * WRITTEN BY:
 *    Dave Clemans
 *
 * DESCRIPTION
 *    Utilities to read the QuickHDL Compiled Library Database
 */     

#ifndef INCLUDED_MTI2EAST_UTIL
#define INCLUDED_MTI2EAST_UTIL

//
// QuickHDL Compiled Library Database API
//    NOTE: this API depends on the format of the
//          compile library database (the "_info" file)
// 

//
// Data structures
//

struct astView
{
   char *name;
   int lineno;
   char *source;
   astView *next;
};
 
struct astCell
{
   char *name;
   int lineno;
   char *source;
   astView *views;
   astCell *next;
};
 
struct astLibrary
{
   char *name;
   astCell *cells;
};
 
//
// ast_get_contents - read in the contents (cells/views) of an MTI library
//
astLibrary *ast_get_contents(
   char                 *libname
   );

//
// ast_freelib - free up a library contents list
//
void ast_freelLib(
   astLibrary           *lib
   );

//
// A front-end to hdl_support::mapToFilenames
//

char **ast_mapToFilenames(
	char     *library_name_p,
	char     *entity_name_p,
	char     *arch_name_p
	);

#endif
