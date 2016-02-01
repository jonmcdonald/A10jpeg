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
 *    A recursive tree-walk package that takes an HDL source (VHDL or Verilog)
 *    and using the MTI IDL interface, produces the synthesis specific
 *    EAST (expanded abstract syntax tree) representation of the source.
 */     

#ifndef INCLUDED_MTI2EAST
#define INCLUDED_MTI2EAST

#ifndef INCLUDED_EAST
#include <east.h>
#endif

#ifndef _MGCERR_H
#include <mgcerr.h>
#endif

//
// A package of routines to read pre-compiled VHDL source, using the
// MTI IDL interface, and produce a synthesis EAST tree.
//
// NORMALLY SHOULD NOT BE CALLED EXTERNALLY!!! use compile_mti2east instead.
//
east_tree *vhdl_mti2east(
   char              **options,
   mgcerrContextIdT  *status
   );
   // specify options to control VHDL compilation

// end of vhdl mti2east 
void vhdl_stop (
   );

//
// A package of routines to read Verilog source, using the MTI IDL interface,
// and produce a synthesis EAST tree.
//
east_tree *vlog_mti2east(
   char              **options,
   mgcerrContextIdT  *status
   );
   // specify files and options to control Verilog compilation

// end of vlog mti2east 
void vlog_stop (
   );

//
// A frontend to the previous language specific routines, that figures
// out what language is needed; does any needed preparation (like calling
// the MTI command qvhcom); and ends up calling vhdl_mti2east, vlog_mti2east,
// etc.
//
east_tree *compile_mti2east(
   const char              *const *options,
   mgcerrContextIdT  *status
   );
   // Convert some HDL source into an EAST tree

// mainly for Verilog to stop and deallocate heap memories.
void compile_stop (
  );

//
// This routine is called whenever the mti2east functions are trying
// to decide whether to expand inline a procedure or function.
// It defaults to always letting the expansion happen; the user can
// replace the routine and do whatever is desired.
//
int expand_this(
   char              *name
   );
   // Should this procedure/function be expanded?

//
// An "EAST" node marker routine; traverse nodes below a given statement
// list and mark all of them with a "given" type by setting its user_data
// field to point to itself.  It is assumed that something external has
// appropriately initialized the set of nodes.
//
void mark_east_nodes(
   east_list            *slist,
   east_node_type       ntype
   );
   // "mark" referenced nodes of a given type

bool subp_parameter(
   east_node *decl
   );

#endif
