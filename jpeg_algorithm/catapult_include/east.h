/*
 * HEADER
 *    east.h - define the classes that make up an Extended Abstract Syntax Tree
 *
 * COPYRIGHT
 *    Copyright (c) MENTOR GRAPHICS CORPORATION 1995 All Rights Reserved
 *
 * WRITTEN BY:
 *    Tom Quarles, Steve Lim, Dave Clemans
 *
 * DESCRIPTION
 *    An extended abstract syntax tree gives an in-memory parsed representation
 *    of an HDL program in VHDL, Verilog, etc. to be operated on by
 *    synthesis tools.
 */

#ifndef INCLUDED_EAST
#define INCLUDED_EAST

#include <stdio.h>
#include <stddef.h>

#include <east_util.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef vco_sss
extern int strcasecmp(const char *,const char *);
   // strcasecmp is the new std C library function for case-insensitive comparison
#endif

#ifdef __cplusplus
}
#endif

// MESSAGING SYSTEM
//   Client of AST must implement this function. A simple
// implementation is given in test_east_gen.C
//
void AST_printf(unsigned int severity, char *msg);


//
// A programming "trick" used in the EAST classes is the use made of the
// class "east_predefined_node". To cut down on the size of a final east
// tree, and to specially mark some nodes that synthesis wants to treat
// "specially", sometimes predefined nodes are used in place of a regular
// east node.
//
// This means that the user of the EAST classes MUST be careful about node
// types. The declared type of member data access functions is just the
// normally expected type; in basically ALL!! cases, predefined nodes can
// also appear.  The EAST classes user MUST use functions like
// east_node::node_type() to distinguish between the normal and the exceptional
// case.
//
// EAST class member data access functions are declared in two ways.
// If many different node types could be returned, the function is declared
// to return "east_node *" (east_node is the base class of all EAST classes).
// But if generally exactly one type of node is returned (e.g. east_type),
// and sometimes a predefined node could be returned, then the function is
// declared to return the "usually" expected type.
//
// Of course, this convention leaves room for error; the user might forget
// to handle one particular node type, or might forget to handle predefined
// nodes, or ...
//

//
// Bump up this number ANY time a change is made that would cause a
// a saved EAST file to no longer be compatible.
//
const int EAST_VERSION = 0;

// the following enumeration is ordered and DEFINES the values placed in
// the binary file - any changes to it require changes to the binary format!
// compatibility MUST be maintained!
//
// Maintain this in parallel with the ASCII array in east_read.C
//
enum east_node_type {
  EOT,
  LAN, // language
  FIL, // filename
  LIN, // line number
  PDF, // east_predef_node
  ABK, // east_always_block
  AGG, // east_aggregate
  ARR, // east_array
  ATN, // east_attrib_name
  ATS, // east_attrib_spec
  ATT, // east_attrib
  BGN, // east_begin_block
  BLK, // east_block
  BOP, // east_binary_op
  CDW, // east_conditional_waveform
  CLL, // east_call
  CON, // east_constant
  CSA, // east_case_stmt_alt
  CSE, // east_case_stmt
  CVN, // east_conversion
  DSU, // east_design_unit
  DVA, // east_delayed_assign_stmt
  ELE, // east_record_elem_decl
  EXI, // east_exit_stmt
  FOR, // east_for_loop
  GEN, // east_generic
  IBK, // east_initial_block
  IFS, // east_if
  INS, // east_instance
  IXN, // east_indexed_name
  LIT, // east_literal
  LST, // east_list
  MAP, // east_map
  NEX, // east_next_stmt
  PCS, // east_process_stmt
  PIO, // east_port
  PKG, // east_package
  PRG, // east_pragma
  QLF, // east_qual_expr
  REC, // east_record_def
  RET, // east_return
  RGE, // east_range
  SGN, // east_signal_assign
  SIG, // east_signal_decl
  SLN, // east_sel_name
  SUB, // east_subprog
  TRI, // east_trinary_op
  TYP, // east_type
  UOP, // east_unary_op
  VAR, // east_var_decl
  VFO, // east_vlog_for_loop
  VGN, // east_var_assign
  WAI, // east_wait_stmt
  WHI  // east_while_loop
};

// forward declarations to allow cross-references without having to try to
// do a sort...
class east_node;

class east_aggregate;
class east_always_block;
class east_array;
class east_attrib;
class east_attrib_name;
class east_attrib_spec;
class east_begin_block;
class east_binary_op;
class east_block;
class east_call;
class east_case_stmt;
class east_case_stmt_alt;
class east_conditional_waveform;
class east_constant;
class east_conversion;
class east_delayed_assign_stmt;
class east_design_unit;
class east_exit_stmt;
class east_for_loop;
class east_generic;
class east_if;
class east_indexed_name;
class east_initial_block;
class east_instance;
class east_list;
class east_literal;
class east_map;
class east_next_stmt;
class east_package;
class east_port;
class east_pragma;
class east_predef_node;
class east_process_stmt;
class east_qual_expr;
class east_range;
class east_record_def;
class east_record_elem_decl;
class east_return;
class east_sel_name;
class east_signal_assign;
class east_signal_decl;
class east_subprog;
class east_trinary_op;
class east_type;
class east_unary_op;
class east_var_assign;
class east_var_decl;
class east_vlog_for_loop;
class east_wait_stmt;
class east_while_loop;

class east_virt {
public:
  virtual ~east_virt() {}
  inline void exec(east_node *node);

  virtual void vexec(east_node *vn);
  virtual void vexec(east_aggregate *vn);
  virtual void vexec(east_always_block *vn);
  virtual void vexec(east_array *vn);
  virtual void vexec(east_attrib *vn);
  virtual void vexec(east_attrib_name *vn);
  virtual void vexec(east_attrib_spec *vn);
  virtual void vexec(east_begin_block *vn);
  virtual void vexec(east_binary_op *vn);
  virtual void vexec(east_block *vn);
  virtual void vexec(east_call *vn);
  virtual void vexec(east_case_stmt *vn);
  virtual void vexec(east_case_stmt_alt *vn);
  virtual void vexec(east_conditional_waveform *vn);
  virtual void vexec(east_constant *vn);
  virtual void vexec(east_conversion *vn);
  virtual void vexec(east_delayed_assign_stmt *vn);
  virtual void vexec(east_design_unit *vn);
  virtual void vexec(east_exit_stmt *vn);
  virtual void vexec(east_for_loop *vn);
  virtual void vexec(east_generic *vn);
  virtual void vexec(east_if *vn);
  virtual void vexec(east_indexed_name *vn);
  virtual void vexec(east_initial_block *vn);
  virtual void vexec(east_instance *vn);
  virtual void vexec(east_list *vn);
  virtual void vexec(east_literal *vn);
  virtual void vexec(east_map *vn);
  virtual void vexec(east_next_stmt *vn);
  virtual void vexec(east_package *vn);
  virtual void vexec(east_port *vn);
  virtual void vexec(east_pragma *vn);
  virtual void vexec(east_predef_node *vn);
  virtual void vexec(east_process_stmt *vn);
  virtual void vexec(east_qual_expr *vn);
  virtual void vexec(east_range *vn);
  virtual void vexec(east_record_def *vn);
  virtual void vexec(east_record_elem_decl *vn);
  virtual void vexec(east_return *vn);
  virtual void vexec(east_sel_name *vn);
  virtual void vexec(east_signal_assign *vn);
  virtual void vexec(east_signal_decl *vn);
  virtual void vexec(east_subprog *vn);
  virtual void vexec(east_trinary_op *vn);
  virtual void vexec(east_type *vn);
  virtual void vexec(east_unary_op *vn);
  virtual void vexec(east_var_assign *vn);
  virtual void vexec(east_var_decl *vn);
  virtual void vexec(east_vlog_for_loop *vn);
  virtual void vexec(east_wait_stmt *vn);
  virtual void vexec(east_while_loop *vn);
};

class east_tree {
  friend class east_node;
  public:
    void * operator new(size_t size, east_heap *heap_p)
      { return heap_p->allocate(size); }
    void * operator new(size_t size)
      { (void)size; fprintf(stderr,"Bad use of default east_tree op new\n"); return (void *)1; }
    void operator delete(void *addr)
      { (void)addr; /* memory effectively freed by destructor*/ }

    enum source_lang_type {
      UNKNOWN,
      VHDL,
      VERILOG
    };

    void print();
    void write(const char *);
    void clear_ref();

    const char *lang(); // returns the source language type as a string
    void lang(source_lang_type l) { d_lang = l; }
    void filename(const char *);  // set function
    const char *filename() { return d_filename; }

    void lineno(int lineno) { d_lineno = lineno; }
    int lineno() const { return d_lineno; }

    void error_count(short count) { d_error_count = count; }
    short error_count() const { return d_error_count; }

    east_tree_symtab *symtab() { return d_symtab_p; }
    east_node *find_predef_node(int i);
    east_node *find_predef_node(const char *name);
    void install_predef_nodes();
    east_design_unit *root_design_unit() { return d_dsu_ptr; }
    int issue_node_id() { return d_count++; }
    void set_design_unit(east_design_unit *);

    east_heap *heap() { return d_heap; }
    FILE *iostream() { return d_iostream; }

    static east_tree *read(char*);
    east_tree(east_heap *heap);
    ~east_tree();

  private:
    east_design_unit *d_dsu_ptr;
    int d_count;
    east_heap *d_heap;
    east_predef_node **d_predef_nodes;
    source_lang_type d_lang;
    FILE *d_iostream;
    char *d_filename;
    int d_lineno;
    east_tree_symtab *d_symtab_p;
    short d_error_count;
};

class east_node {
  // base class from which all other east tree member nodes are derived.
  protected:
    int d_node_id;
    unsigned int d_nodeid_valid;
    east_node *d_parent;
  public:
    void * operator new(size_t size, east_tree * tree_p)
      { return tree_p->heap()->allocate(size); }
    void * operator new(size_t size)
      { (void) size; fprintf(stderr,"Bad call of east_node::operator new\n"); return (void *)1; }
    void operator delete(void *addr)
      { east_heap *heap_p = east_heap::find_heap(addr); heap_p->deallocate(addr); }
    virtual east_node_type node_type() const =0;
    virtual void clear_ref();
    virtual void print(east_tree *tree);
    virtual int node_ID(east_tree *tree);
    virtual void virt(east_virt *virt);
    int lineno() const { return d_lineno; }
    const char *source_filename() { return d_filename; }

    // NOTE!!!
    // The "user data" value (if any) is NOT!!! persistent.
    // It WILL be clobbered by any node_ID() or clear_ref() calls!!!
    // The method east_tree::print calls node_ID() and clear_ref()!!!!
    //
    // Depends on integers and pointers being the same size... e.g.
    // not really right yet for DEC Alpha. However, we don't support
    // the DEC Alpha yet.
    //
    void user_data(void *data) { d_nodeid_valid = (unsigned int)data; }
    void *user_data() const { return (void *)d_nodeid_valid; }

    // NOTE!!!
    // A NULL parent should be expected and considered valid, meaning
    // that this object doesn't know its parent.
    //
    // The parent field is NOT set until the object is attached to
    // another object. After it's set, the parent field tries to be a
    // backward pointer to the logical owner of this object; for example,
    // a local declaration in a function should be owned by the function.
    // If you need to find the owning "container" object, such as a
    // package or a design unit, successively follow parent pointers until
    // you reach it.
    //
    // A parent of an east_predef_node is meaningless.
    //
    void parent(east_node *obj) { d_parent = obj; }
    east_node *parent() { return d_parent; }

  protected:
    east_node(east_tree *tree);
    east_node(east_tree *tree, int predef_id);
    virtual ~east_node();
  private:
    const char *d_filename;
    int d_lineno;
};

class east_predef_node : public east_node {
  public:
    static const char *const namelist[];
    enum predef_type {
         PDF_NULL=0,
         PDF_ALL,
         PDF_OTHERS,
         PDF_OPEN,
         PDF_STANDARD,
         PDF_BOOLEAN,
         PDF_BIT,
         PDF_CHARACTER,
         PDF_SEVERITY_LEVEL,
         PDF_INTEGER,
         PDF_REAL,
         PDF_TIME,
         PDF_NOW,
         PDF_NATURAL,
         PDF_POSITIVE,
         PDF_STRING,
         PDF_BIT_VECTOR,
         PDF_TICK_BASE,
         PDF_TICK_LEFT,
         PDF_TICK_RIGHT,
         PDF_TICK_HIGH,
         PDF_TICK_LOW,
         PDF_TICK_POS,
         PDF_TICK_VAL,
         PDF_TICK_SUCC,
         PDF_TICK_PRED,
         PDF_TICK_LEFTOF,
         PDF_TICK_RIGHTOF,
         PDF_TICK_RANGE,
         PDF_TICK_REVERSE_RANGE,
         PDF_TICK_LENGTH,
         PDF_TICK_DELAYED,
         PDF_TICK_STABLE,
         PDF_TICK_QUIET,
         PDF_TICK_TRANSACTION,
         PDF_TICK_EVENT,
         PDF_TICK_ACTIVE,
         PDF_TICK_LAST_EVENT,
         PDF_TICK_LAST_ACTIVE,
         PDF_TICK_LAST_VALUE,
         PDF_TICK_FOREIGN,
         PDF_STD_LOGIC_1164,
         PDF_STD_ULOGIC,
         PDF_STD_ULOGIC_VECTOR,
         PDF_RESOLVED,
         PDF_STD_LOGIC,
         PDF_STD_LOGIC_VECTOR,
         PDF_X01,
         PDF_X01Z,
         PDF_UX01,
         PDF_UX01Z,
         PDF_TO_BIT,
         PDF_TO_BITVECTOR,
         PDF_TO_STDULOGIC,
         PDF_TO_STDLOGICVECTOR,
         PDF_TO_STDULOGICVECTOR,
         PDF_TO_X01,
         PDF_TO_X01Z,
         PDF_TO_UX01,
         PDF_RISING_EDGE,
         PDF_FALLING_EDGE,
         PDF_IS_X,
         PDF_STD_LOGIC_ARITH,
         PDF_SIGNED,
         PDF_UNSIGNED,
         PDF_STD_ULOGIC_WIRED_OR,
         PDF_STD_ULOGIC_WIRED_AND,
         PDF_TO_INTEGER,
         PDF_CONV_INTEGER,
         PDF_TO_UNSIGNED,
         PDF_CONV_UNSIGNED,
         PDF_TO_SIGNED,
         PDF_CONV_SIGNED,
         PDF_ZERO_EXTEND,
         PDF_SIGN_EXTEND,
         PDF_EQ,
         PDF_NE,
         PDF_LT,
         PDF_GT,
         PDF_LE,
         PDF_GE,
         PDF_STD_SYNTHESIS_UTILS,
         PDF_STD_MATCH,
         PDF_CHAR_0,
         PDF_CHAR_1,
         PDF_TRUE,
         PDF_FALSE,
         PDF_INT_ZERO,
         PDF_HIER_GROUP,
         PDF_IGNORE_PROCESS,
         PDF_RESOURCE_GROUP,
         PDF_OPS,
         PDF_ADD_OPS,
         PDF_MAY_MERGE_WITH,
         PDF_DONT_MERGE_WITH,
         PDF_DONT_USE,
         PDF_IMPLEMENTATION,
         PDF_MAP_TO_MODULE,
         PDF_MAX_NUM_RESOURCES,
         PDF_MAX_MUX_SIZE,
         PDF_RESOLUTION_METHOD,
         PDF_MAP_TO_BUILTIN,
         PDF_MAP_TO_ENTITY,
         PDF_FSM_ENCODING_SCHEME,
         PDF_LATCHING_SCHEME,
         PDF_ALLOW_COMB_FEEDBACK,
         PDF_TARGET_TECHNOLOGY,
         PDF_VOLTAGE,
         PDF_PROCESS_COND,
         PDF_MAX_INPUT_CAP,
         PDF_MAX_OUTPUT_CAP,
         PDF_MAX_AREA,
         PDF_RESOURCE,
         PDF_FSM_ENCODING_BINARY,
         PDF_FSM_ENCODING_GRAY,
         PDF_FSM_ENCODING_ONEHOT,
         PDF_FSM_ENCODING_NOVA,
         PDF_FSM_ENCODING_MUSTANG,
         PDF_FSM_ENCODING_SPECTRAL,
         PDF_FSM_ENCODING_FAULT_TOLERANT,
         PDF_TO_STDLOGIC,
         PDF_AND_REDUCE,
         PDF_NAND_REDUCE,
         PDF_OR_REDUCE,
         PDF_NOR_REDUCE,
         PDF_XOR_REDUCE,
         PDF_XNOR_REDUCE,
         PDF_QSIM_LOGIC,
         PDF_QSIM_STATE,
         PDF_QSIM_STATE_VECTOR,
         PDF_QSIM_WIRED_X,
         PDF_QSIM_WIRED_OR,
         PDF_QSIM_WIRED_AND,
         PDF_QSIM_STATE_RESOLVED_X,
         PDF_QSIM_STATE_RESOLVED_OR,
         PDF_QSIM_STATE_RESOLVED_AND,
         PDF_QSIM_VALUE,
         PDF_QSIM_VALUE_VECTOR,
         PDF_BIT_WIRED_OR,
         PDF_BIT_RESOLVED_OR,
         PDF_BIT_RESOLVED_OR_VECTOR,
         PDF_BIT_WIRED_AND,
         PDF_BIT_RESOLVED_AND,
         PDF_BIT_RESOLVED_AND_VECTOR,
         PDF_INTBITSIZE,
         PDF_TO_QSIM_STATE,
         PDF_FN_AND,
         PDF_FN_OR,
         PDF_FN_NAND,
         PDF_FN_NOR,
         PDF_FN_XOR,
         PDF_FN_XNOR,
         PDF_QSIM_STATE_RESOLVED_X_VECTOR,
         PDF_QSIM_STATE_RESOLVED_OR_VECTOR,
         PDF_QSIM_STATE_RESOLVED_AND_VECTOR,
         PDF_VLOG_AND_GATE,
         PDF_VLOG_NAND_GATE,
         PDF_VLOG_NOR_GATE,
         PDF_VLOG_OR_GATE,
         PDF_VLOG_XNOR_GATE,
         PDF_VLOG_XOR_GATE,
         PDF_VLOG_BUF_GATE,
         PDF_VLOG_NOT_GATE,
         PDF_VLOG_BUFIF0_GATE,
         PDF_VLOG_BUFIF1_GATE,
         PDF_VLOG_NOTIF0_GATE,
         PDF_VLOG_NOTIF1_GATE,
         PDF_DONT_INLINE,
         PDF_VLOG_PULLUP_GATE,
         PDF_VLOG_PULLDOWN_GATE,
         PDF_VLOG_FULL_CASE,
         PDF_VLOG_PARALLEL_CASE,
         PDF_EXTERNAL_MEMORY,
         PDF_PACKING_MODE,
         PDF_SYNTHESIS_RETURN,
         REAL_MAX_PREDEF_NODE_PLUS_ONE
    };
    // next line is to make things easier if we ever have to add predefined
    // nodes in the future - decrement the magic constant 1000 (int east.C)
    // by the number
    // of predefined nodes which are added and it won't break any existing
    // EASTs which are out there - they can still be read in.  We can detect
    // an EAST from a newer version of the writer by any nodes between the
    // enum value and the static const int value.
    static const int MAX_PREDEF_NODE_PLUS_ONE;
    const char *const type_name() const { return namelist[d_type]; }
    const predef_type pdf_type() const { return d_type; }
    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual int node_ID(east_tree *tree);
    virtual void virt(east_virt *virt);

    east_predef_node(east_tree *tree,
                     predef_type type);
    virtual ~east_predef_node();

  private:
    predef_type d_type;

};

class east_aggregate : public east_node {
  public:
    east_list *agg_list() const { return d_asn_list_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_aggregate(east_tree*tree,east_list *asn_list);
    virtual ~east_aggregate();
  private:
    east_list *d_asn_list_p; // list of aggregate expression nodes
};

class east_always_block : public east_node {
  public:
    const char *name() const { return d_label_p; }
    east_list *sensitivity_list() const { return d_sens_list_p; }
    east_list *locals() const { return d_decl_list_p; }
    east_list *body() const { return d_stmt_list_p; }
    east_node *pragma() const { return d_pragmas_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_always_block(east_tree*tree,const char *label, east_list *sens, east_list *decl, east_list*stmt, east_node*pragmas);
    virtual ~east_always_block();
  private:
    char *d_label_p;
    east_list *d_sens_list_p;
    east_list *d_decl_list_p;
    east_list *d_stmt_list_p;
    east_node *d_pragmas_p;
};

class east_array : public east_node {
  public:
    east_type *array_base_type() const { return d_basetype; }
    east_node *array_index_constraint() const { return d_index_constraint; }
    east_type *array_element_type() const { return d_elemtype; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_array(east_tree *tree,
               east_type *basetype,
               east_node *index_constraint,
               east_type *elemtype);
    virtual ~east_array();

  private:
    east_type *d_basetype;
    east_node *d_index_constraint;
    east_type *d_elemtype;
};

class east_attrib_name : public east_node {
  public:
    east_node *get_attrib_name_prefix() const { return d_prefix; }
    east_node *attrib_name_prefix() const { return d_prefix; }
    east_attrib *get_attrib() const { return d_attrib; }
    east_attrib *attrib() const { return d_attrib; }
    east_node *get_parameter() const { return d_parameter; }
    east_node *parameter() const { return d_parameter; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_attrib_name(east_tree *tree, east_node *prefix, east_attrib*attrib, east_node *parameter);
    virtual ~east_attrib_name();
  private:
    east_node *d_prefix;
    east_attrib *d_attrib;
    east_node *d_parameter;
};

class east_attrib : public east_node {
  public:
    const char *attrib_name() const { return d_name; }
    east_type *attrib_type() const { return d_type_p; }
    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_attrib(east_tree*tree,const char *name, east_type*type);
    virtual ~east_attrib();
  private:
    char *d_name;
    east_type *d_type_p;
};

class east_attrib_spec : public east_node {
  public:
    east_attrib *decl() const { return d_decl_p; }
    east_node *object() const { return d_object_p; }
    east_node *value() const { return d_value_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_attrib_spec(east_tree*tree,east_attrib *decl, east_node *object, east_node *value);
    virtual ~east_attrib_spec();
  private:
    east_attrib *d_decl_p;
    east_node *d_object_p;
    east_node *d_value_p;
};

class east_begin_block : public east_node {
  public:
    char *label() const {return d_label_p; }
    east_list *decls() const {return d_decls_p; }
    east_list *stmts() const {return d_stmts_p; }
    east_node *pragma() const { return d_pragmas_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_begin_block(east_tree*tree,
      char *label,
      east_list *declarations,
      east_list *statements,
      east_node *pragmas);
    virtual ~east_begin_block();
  private:
    char *d_label_p;
    east_list *d_decls_p;
    east_list *d_stmts_p;
    east_node *d_pragmas_p;
};

class east_block : public east_node {
  public:
    const char *label() const { return d_label; }
    east_node *guard_expr() const { return d_guard_expr_p; }
    east_list *generics() const { return d_gen_list_p; }
    east_list *generic_map() const { return d_gen_map_p; }
    east_list *port_list() const { return d_port_list_p; }
    east_list *port_map() const { return d_port_map_p; }
    east_list *local_decls() const { return d_local_decls_p; }
    east_list *concur_stmts() const { return d_concur_stmts_p; }
    east_node *pragma() const { return d_pragmas_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_block(east_tree*tree,
               const char*label,
               east_node *guard_expr,
               east_list *gen_list,
               east_list *gen_map,
               east_list *port_list,
               east_list *port_map,
               east_list *local_decls,
               east_list *stmts,
               east_node *pragmas);
    virtual ~east_block();
  private:
    char *d_label;
    east_node *d_guard_expr_p;
    east_list *d_gen_list_p;
    east_list *d_gen_map_p;
    east_list *d_port_list_p;
    east_list *d_port_map_p;
    east_list *d_local_decls_p;
    east_list *d_concur_stmts_p;
    east_node *d_pragmas_p;
};

class east_binary_op : public east_node {
  public:
    enum binary_ops {
      AND=1,
      OR,
      XOR,
      NAND,
      NOR,
      EQ,
      NE,
      LT,
      LE,
      GT,
      GE,
      ADD,
      SUB,
      CONC,
      MUL,
      DIV,
      MOD,
      REM,
      EXP,
      XNOR,
      SLL,
      SRL,
      SLA,
      SRA,
      ROL,
      ROR
    };

    binary_ops opcode() const { return d_op; }
    east_node *left_operand() const { return d_left_p; }
    east_node *right_operand() const { return d_right_p; }
    east_node *pragma() const { return d_pragma_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_binary_op(east_tree*tree,binary_ops, east_node *left, east_node *right, east_node *pragma);
    virtual ~east_binary_op();
  private:
    binary_ops d_op;
    east_node *d_left_p;
    east_node *d_right_p;
    east_node *d_pragma_p;
};

class east_conditional_waveform : public east_node {
  public:
    east_node *condition() const { return d_condition_p; }
    east_node *true_wf() const { return d_true_wf_p; }
    east_node *false_wf() const { return d_false_wf_p; }
    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_conditional_waveform(east_tree *tree,
                              east_node *cond,
                              east_node *true_wf,
                              east_node *false_wf);
    virtual ~east_conditional_waveform();
  private:
    east_node *d_condition_p;
    east_node *d_true_wf_p;
    east_node *d_false_wf_p;
};

class east_call : public east_node {
  public:
    const char *label() const { return d_label_p; }
    east_subprog *subprog() const { return d_subprog_p; }
    east_list *params() const { return d_parms_p; }
    east_node *pragma() const { return d_pragmas_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_call(east_tree*tree,const char *label, east_subprog *subprog, east_list *parms, east_node *pragmas);
    virtual ~east_call();
  private:
    char *d_label_p;
    east_subprog *d_subprog_p;
    east_list *d_parms_p;
    east_node *d_pragmas_p;
};

class east_constant : public east_node {
  public:
    enum const_type {
      CONSTANT,
      PARAMETER,
      ITERATOR
    };
    const char *get_name() const { return d_name; }
    const char *name() const { return d_name; }
    east_type  *get_type() const { return d_type_p; }
    east_type  *type() const { return d_type_p; }
    east_node  *get_value() const { return d_value_p; }
    east_node  *value() const { return d_value_p; }
    const_type get_const_type() const { return d_con_type; }
    const_type con_type() const { return d_con_type; }

    east_node *pragma() const { return d_pragmas_p; }

    void set_value(east_node *value_p);

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_constant(east_tree*tree,const char *name, east_type *type,
      east_node *initial, const_type con_type, east_node *pragmas=0);
    virtual ~east_constant();
  private:
    char *d_name;
    east_type *d_type_p;
    east_node *d_value_p;
    const_type d_con_type;
    east_node *d_pragmas_p;
};

class east_conversion : public east_node {
  public:
    const char *name() const { return d_name_p; }
    east_node * operand() const { return d_operand_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_conversion(east_tree*tree, const char *name, east_node *operand);
    virtual ~east_conversion();
  private:
    char *d_name_p;
    east_node *d_operand_p;
};

class east_case_stmt_alt : public east_node {
  public:
    east_list *choices() const { return d_choices_p; }
    east_list *selected() const { return d_stmts_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_case_stmt_alt(east_tree *tree, east_list *choices, east_list *stmts);
    virtual ~east_case_stmt_alt();
  private:
    east_list *d_choices_p;
    east_list *d_stmts_p;
};

class east_case_stmt : public east_node {
  public:
    enum case_type {
      NORMAL,           // for VHDL case stmts
      VLOG_CASE,        // for Verilog case stmts
      VLOG_CASEX,       // for Verilog casex stmts
      VLOG_CASEZ        // for Verilog casez stmts
    };
    const char *label() const { return d_label_p; }
    east_node *select_expr() const { return d_expr_p; }
    case_type type() { return d_type; }
    east_list *alternatives() const { return d_alts_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_case_stmt(east_tree *tree,
                   const char *label,
                   case_type type,
                   east_node *expr,
                   east_list *alternatives);
    virtual ~east_case_stmt();
  private:
    char *d_label_p;
    east_node *d_expr_p;
    east_list *d_alts_p;
    case_type d_type;
};

class east_delayed_assign_stmt : public east_node {
  public:
    east_node *assign_stmt() const { return d_assign_stmt_p; }
    east_node *delay_count() const { return d_delay_count_p; }
    east_node *delay_event() const { return d_delay_event_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_delayed_assign_stmt(east_tree *tree,
      east_node *stmt,
      east_node *count,
      east_node *events);
    virtual ~east_delayed_assign_stmt();
  private:
    east_node *d_assign_stmt_p;
    east_node *d_delay_count_p;
    east_node *d_delay_event_p;
};

class east_design_unit : public east_node {
  public:
    const char *library() const { return d_lib_p; }
    const char *entity() const { return d_ent_p; }
    const char *arch() const { return d_arch_p; }
    int profile_id() const { return d_profile; }
    east_list *generics() const { return d_generics_p; }
    east_list *ports() const { return d_ports_p; }
    east_list *signals() const { return d_local_sigs_p; }
    east_list *packages() const { return d_packages_p; }
    east_list *concur_stmts() const { return d_concur_stmts_p; }
    east_node *pragma() const { return d_pragmas_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_design_unit(east_tree*tree,
                     const char *library,
                     const char *entity,
                     const char *architecture,
                     int profile,
                     east_list *generics,
                     east_list *ports,
                     east_list *signals,
                     east_list *packages,
                     east_list *stmts,
                     east_node *pragmas);
    virtual ~east_design_unit();
  private:
    char *d_lib_p;
    char *d_ent_p;
    char *d_arch_p;
    int d_profile;
    east_list *d_generics_p;
    east_list *d_ports_p;
    east_list *d_local_sigs_p;
    east_list *d_packages_p;
    east_list *d_concur_stmts_p;
    east_node *d_pragmas_p;
};

class east_record_elem_decl : public east_node {
  public:
    const char *name() const { return d_name_p; }
    east_type *type() const { return d_type_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_record_elem_decl(east_tree*tree,const char *name, east_type *type);
    virtual ~east_record_elem_decl();
  private:
    char *d_name_p;
    east_type *d_type_p;
};

class east_exit_stmt : public east_node {
  public:
    const char *get_label() const { return d_label_p; }
    const char *label() const { return d_label_p; }
    const char *get_loop_label() const { return d_loop_label_p; }
    const char *loop_label() const { return d_loop_label_p; }
    east_node  *get_condition() const { return d_condition_p; }
    east_node  *condition() const { return d_condition_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_exit_stmt(east_tree *tree,
                   const char *label,
                   const char *loop_label,
                   east_node *cond);
    virtual ~east_exit_stmt();
  private:
    char *d_label_p;
    char *d_loop_label_p;
    east_node *d_condition_p;
};

class east_range : public east_node {
  public:
    enum dir {
      UP,
      DOWN,
      UNKNOWN
    };
    dir direction() const { return d_direction; }
    east_node *left_bound() const { return d_left_p; }
    east_node *right_bound() const { return d_right_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_range(east_tree*tree,dir ascending, east_node *left, east_node*right);
    virtual ~east_range();
  private:
    dir d_direction;
    east_node *d_left_p;
    east_node *d_right_p;
};

class east_for_loop : public east_node {
  public:
    const char *label() const { return d_label_p; }
    east_node *index() const { return d_index_p; }
    east_list *loop_body() const { return d_stmts_p; }
    east_node *range() const { return d_range_p; }
    east_node *pragma() const { return d_pragma_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_for_loop(east_tree *tree,
                  const char *label,
                  east_node *index,
                  east_node *range,
                  east_list *stmts,
                  east_node *pragma);
    virtual ~east_for_loop();
  private:
    char *d_label_p;
    east_node *d_index_p;
    east_node *d_range_p;
    east_list *d_stmts_p;
    east_node *d_pragma_p;
};

class east_vlog_for_loop : public east_node {
  public:
    const char *label() const { return d_label_p; }
    east_node *init() const { return d_init_p; }
    east_node *limit() const { return d_limit_p; }
    east_node *incr() const { return d_incr_p; }
    east_list *loop_body() const { return d_stmts_p; }
    east_node *pragma() const { return d_pragma_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_vlog_for_loop(east_tree *tree,
                  const char *label,
                  east_node *init,
                  east_node *limit,
                  east_node *incr,
                  east_list *stmts,
                  east_node *pragma);
    virtual ~east_vlog_for_loop();
  private:
    char *d_label_p;
    east_node *d_init_p;
    east_node *d_limit_p;
    east_node *d_incr_p;
    east_list *d_stmts_p;
    east_node *d_pragma_p;
};

class east_generic : public east_node {
  public:
    const char *name() const { return d_name_p; }
    east_type *type() const { return d_type_p; }
    east_node *default_expr() const { return d_expr_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_generic(east_tree*tree,const char *name, east_type *type, east_node *init);
    virtual ~east_generic();
  private:
    char *d_name_p;
    east_type *d_type_p;
    east_node *d_expr_p;
};

class east_if : public east_node {
  public:
    const char *label() const { return d_label_p; }
    east_node *condition() const { return d_cond_p; }
    east_list *then_branch() const { return d_then_p; }
    east_list *else_branch() const { return d_else_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_if(east_tree*tree,const char *label,east_node *d_cond_p, east_list *then_p, east_list *else_p);
    virtual ~east_if();
  private:
    char *d_label_p;
    east_node *d_cond_p;
    east_list *d_then_p;
    east_list *d_else_p;
};

class east_initial_block : public east_node {
  public:
    const char *name() const { return d_label_p; }
    east_list *body() const { return d_stmt_list_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_initial_block(east_tree*tree,const char *label,east_list*stmt);
    virtual ~east_initial_block();
  private:
    char *d_label_p;
    east_list *d_stmt_list_p;
};

class east_instance : public east_node {
  public:
    const char *label() const { return d_name_p; }
    const char *formalname() const { return d_dsu_name_p; }
    east_design_unit *bound_unit() const { return d_des_unit_p; }
    east_list *generic_map() const { return d_generic_map_p; }
    east_list *port_map() const { return d_port_map_p; }
    east_node *pragma() const { return d_pragmas_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_instance(east_tree *tree,
         const char *name,
         const char *fname,
         east_design_unit *des_unit,
         east_list *gmp,
         east_list *pmp,
         east_node *pragmas);
    virtual ~east_instance();
  private:
    char *d_name_p;
    char *d_dsu_name_p;
    east_design_unit *d_des_unit_p;
    east_list *d_generic_map_p;
    east_list *d_port_map_p;
    east_node *d_pragmas_p;
};

class east_indexed_name : public east_node {
  public:
    east_node *prefix() const { return d_name_p; }
    east_node *index() const { return d_index_p; }
    east_node *pragma() const { return d_pragmas_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_indexed_name(east_tree*tree,east_node *name, east_node *index, east_node *pragmas);
    virtual ~east_indexed_name();
  private:
    east_node *d_name_p;
    east_node *d_index_p;
    east_node *d_pragmas_p;
};

class east_literal : public east_node {
  public:
    enum lit_type {
      NUMERIC=1,     // d_lit is numeric value
      CHR,           // d_lit is a string containing the character
      STR,           // d_lit is string
      BIT_STR,       // d_lit is a string containing the ascii bit pattern
      PHYSICAL       // d_lit is a string containing the physical literal
                     //    (for example, "10 ns")
    };
    lit_type type() const { return d_type; }
    const char * value() const { return d_lit_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void virt(east_virt *virt);

    east_literal(east_tree*tree,lit_type type,const char *lit);
    virtual ~east_literal();
  private:
    lit_type d_type;
    char * d_lit_p;

};

class east_list : public east_node {
  public:
    int count() const { return d_count; }
    east_node **list_data() const { return d_data_p; }

    void grow_list_data(east_tree *tree,east_node *node);

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_list(east_tree*tree, int count, east_node**data);
    virtual ~east_list();
  private:
    int d_count;
    east_node **d_data_p;
};

class east_map : public east_node {
  public:
    east_node *left() const { return d_left_p; }
    east_node *right() const { return d_right_p; }
    int identical() const {return ((d_left_p == d_right_p));}

    // for internal use within libmti2east.a(east_expand)
    int internal() const {return d_internal;}
    void set_internal(int internal) {d_internal = internal;}

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_map(east_tree*tree, east_node*left, east_node *right);
    virtual ~east_map();
  private:
    east_node *d_left_p;
    east_node *d_right_p;
    int d_internal;
};

class east_next_stmt : public east_node {
  public:
    const char *label() const { return d_label_p; }
    const char *loop() const { return d_loop_p; }
    const char *get_loop_label() const { return d_loop_p; }
    east_node *condition() const { return d_condition_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_next_stmt(east_tree*tree,const char *label, const char *loop_label, east_node *cond);
    virtual ~east_next_stmt();
  private:
    char *d_label_p;
    char *d_loop_p;
    east_node *d_condition_p;
};

class east_package : public east_node {
  public:
    const char *libname() const { return d_libname_p; }
    const char *name() const { return d_name_p; }
    east_list *decls() const { return d_decls_p; }
    east_list *pkgs() const { return d_pkgs_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_package(east_tree *tree, const char *library, const char *package, east_list *contents, east_list *pkgs);
    virtual ~east_package();

  private:
    char *d_name_p;
    char *d_libname_p;
    east_list *d_decls_p;
    east_list *d_pkgs_p;
};

class east_process_stmt : public east_node {
  public:
    const char *name() const { return d_label_p; }
    east_list *sensitivity_list() const { return d_sens_list_p; }
    east_list *locals() const { return d_decl_list_p; }
    east_list *body() const { return d_stmt_list_p; }
    east_node *pragma() const { return d_pragmas_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_process_stmt(east_tree*tree,const char *label, east_list *sens, east_list *decl, east_list*stmt, east_node*pragmas);
    virtual ~east_process_stmt();
  private:
    char *d_label_p;
    east_list *d_sens_list_p;
    east_list *d_decl_list_p;
    east_list *d_stmt_list_p;
    east_node *d_pragmas_p;
};

class east_port: public east_node {
  public:
    enum modes {
      MODE_IN = 1,
      MODE_OUT ,
      MODE_INOUT ,
      MODE_BUFFER ,
      MODE_LINKAGE
    };
    enum isbus {
      ISBUS_FALSE = 0,
      ISBUS_TRUE
    };
    const char *name() const { return d_name_p; }
    modes port_mode() const { return d_mode; }
    isbus is_bus() const { return d_bus; }
    east_type *type() const { return d_type_p; }
    east_node *init_value() const { return d_expr_p; }
    east_node *pragma() const { return d_pragmas_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_port(east_tree*tree,const char *name, modes mode, isbus bus, east_type *type, east_node *init, east_node *pragmas);
    virtual ~east_port();
  private:
    char *d_name_p;
    modes d_mode;
    isbus d_bus;
    east_type *d_type_p;
    east_node *d_expr_p;
    east_node *d_pragmas_p;
};

class east_pragma : public east_node {
  public:
    enum pragma_kind {
      // Available as in the input file only
      RESOLUTION_METHOD=1,
      LABEL,
      LABEL_APPLIES_TO,
      FULL_CASE,
      PARALLEL_CASE,
      TRANSLATE_OFF,
      TRANSLATE_ON,
      MAP_TO_ENTITY,
      RETURN_PORT_NAME,
      BUILT_IN,
      TEMPLATE,
      FLATTEN,
      BLACK_BOX,
      DONT_TOUCH,
      DISSOLVE,
      MAP_TO_RESOURCE,
      MEMORY_RESOURCE,
      ML_RESOURCE,

      // Has an equivalent directive
      CLOCK_NAME,
      CLOCK_EDGE,
      RESET_NAME,
      RESET_ACTIVE,
      RESET_KIND,
      ENABLE_NAME,
      ENABLE_ACTIVE,
      MAP_TO_OPERATOR,
      UNROLL,
      MAP_TO_MODULE,
      PACKING_MODE,
      WORD_WIDTH,
      VARIABLES,
      IGNORE_PROCESS,
      CLOCK_PERIOD,
      MAXLEN,
      CLOCK_OVERHEAD,
      ITERATIONS,
      EXTERNAL_MEMORY,
      PIPELINE_INIT_INTERVAL,
      PIPELINE_RAMP_UP,
      RESET_CLEARS_ALL_REGS
    };
    pragma_kind kind() const { return d_kind; }
    const char * arg() const { return d_arg_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void virt(east_virt *virt);

    east_pragma(east_tree*tree,pragma_kind type, const char *arg);
    virtual ~east_pragma();
  private:
    pragma_kind d_kind;
    char *d_arg_p;

};

class east_qual_expr : public east_node {
  public:
    east_node *type() const { return d_type_p; }
    east_node *expr() const { return d_expr_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_qual_expr(east_tree*tree, east_node *type, east_node *expr);
    virtual ~east_qual_expr();
  private:
    east_node *d_type_p;
    east_node *d_expr_p;
};

class east_record_def : public east_node {
  public:
    int count() const { return d_count; }
    east_record_elem_decl ** elements() const { return d_members_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_record_def(east_tree*tree,int count, east_record_elem_decl**data);
    virtual ~east_record_def();
  private:
    int d_count;
    east_record_elem_decl **d_members_p;
};

class east_return : public east_node {
  public:
    const char *label() const { return d_label_p; }
    east_node *expr() const { return d_value_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_return(east_tree*tree,const char *label, east_node *expr);
    virtual ~east_return();
  private:
    char *d_label_p;
    east_node *d_value_p;
};

class east_signal_assign: public east_node {
  public:
    const char *get_label()  const { return d_label_p; }
    const char *label()      const { return d_label_p; }
    int         is_guarded() const { return d_guarded; }
    east_node  *get_target() const { return d_target_p; }
    east_node  *target()     const { return d_target_p; }
    east_node  *get_source() const { return d_source_p; }
    east_node  *source()     const { return d_source_p; }
    east_node  *pragma()     const { return d_pragma_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_signal_assign(east_tree *tree,
                       const char *label,
                       int guarded,
                       east_node *target,
                       east_node *source,
                       east_node *pragma);
    virtual ~east_signal_assign();
  private:
    char *d_label_p;
    int d_guarded;
    east_node *d_target_p;
    east_node *d_source_p;
    east_node *d_pragma_p;
};

class east_signal_decl : public east_node {
  public:
    enum sig_modes {
      SIGMODE_NONE=0,
      SIGMODE_IN,
      SIGMODE_OUT,
      SIGMODE_INOUT
    };
    enum sig_kind {
      UNGUARDED=0,
      REGISTER,
      BUS
    };
    const char *name() const { return d_name_p; }
    sig_kind signal_kind() const { return d_kind; }
    east_subprog * resolution_fcn() const { return d_resolution_p; }
    east_type * type() const { return d_type_p; }
    east_node * initial() const { return d_expr_p; }
    sig_modes signal_mode() const { return d_mode; }
    int is_implicit() const { return d_implicit; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_signal_decl(east_tree *tree,
                     const char *name,
                     sig_kind kind,
                     sig_modes mode,
                     east_subprog *resolution,
                     east_type *type,
                     east_node *expr,
                     int implicit);
    virtual ~east_signal_decl();
  private:
    char *d_name_p;
    sig_kind d_kind;
    east_subprog *d_resolution_p;
    east_type *d_type_p;
    east_node *d_expr_p;
    sig_modes d_mode;
    int d_implicit;
};

class east_sel_name : public east_node {
  public:
    east_node *prefix() const { return d_prefix_p; }
    const char *suffix() const { return d_suffix_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_sel_name(east_tree*tree,east_node *prefix, const char *suffix);
    virtual ~east_sel_name();
  private:
    east_node *d_prefix_p;
    char *d_suffix_p;
};

class east_subprog : public east_node {
  public:
    const char *name() const { return d_name_p; }
    east_list *formals() const { return d_formals_p; }
    east_type *return_type() const { return d_return_type_p; }
    east_list *locals() const { return d_locals_p; }
    east_list *body() const { return d_body_p; }
    east_node *pragmas() const { return d_pragmas_p; }

    void set_formals(east_list *formals_p);
    void set_locals(east_list *locals_p);
    void set_subp_body(east_list *body_p);
    void set_pragmas(east_node *pragmas_p);

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_subprog(east_tree*tree,const char *name, east_list *formals, east_type *return_type, east_list *locals, east_list *body, east_node *pragmas);
    virtual ~east_subprog();
  private:
    char *d_name_p;
    east_list *d_formals_p;
    east_type *d_return_type_p;
    east_list *d_locals_p;
    east_list *d_body_p;
    east_node *d_pragmas_p;

};

class east_trinary_op : public east_node {
  public:
    enum trinary_ops {
      QUEST = 1
    };
    trinary_ops opcode() const { return d_opcode; }
    east_node * operand1() const { return d_left_operand_p; }
    east_node * operand2() const { return d_mid_operand_p; }
    east_node * operand3() const { return d_right_operand_p; }
    east_node * pragma() const { return d_pragma_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_trinary_op(
      east_tree *tree,
      trinary_ops opcode,
      east_node *op1,
      east_node *op2,
      east_node *op3,
      east_node *pragmas);
    virtual ~east_trinary_op();
  private:
    trinary_ops d_opcode;
    east_node *d_left_operand_p;
    east_node *d_mid_operand_p;
    east_node *d_right_operand_p;
    east_node *d_pragma_p;
};

class east_type : public east_node {
  public:
    enum types {
      ARRAY = 1,
      ENUMERATION,
      RECORD,
      SCALAR_INT
    };
    types type_kind() const { return d_type; }
    const char * name() const { return d_name_p; }
    east_node * type_def() const { return d_def_node_p; }
    east_subprog *res_func() const { return d_res_func_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_type(east_tree*tree,const char *name);
    east_type(east_tree*tree,const char *name,types type, east_node *def_node);
    east_type(east_tree*tree,const char *name,types type, east_node *def_node, east_subprog *res_func);
    virtual ~east_type();
  private:
    enum types d_type;
    char *d_name_p;
    east_node *d_def_node_p;
    east_subprog *d_res_func_p;
};

class east_unary_op : public east_node {
  public:
    enum unary_ops {
      UPLUS = 1,
      UMINUS,
      UABS,
      UNOT,
      UAND,
      UNAND,
      UOR,
      UNOR,
      UXOR,
      UXNOR,
      UINV
    };
    unary_ops opcode() const { return d_opcode; }
    east_node * operand() const { return d_operand_p; }
    east_node * pragma() const { return d_pragma_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_unary_op(east_tree*tree,unary_ops opcode, east_node*operand, east_node *pragmas);
    virtual ~east_unary_op();
  private:
    unary_ops d_opcode;
    east_node *d_operand_p;
    east_node *d_pragma_p;
};

class east_var_decl : public east_node {
  public:
    enum var_modes {
      VARMODE_NONE=0,
      VARMODE_IN,
      VARMODE_OUT,
      VARMODE_INOUT
    };
    const char *name() const { return d_name_p; }
    east_type *type() const { return d_type_p; }
    east_node *initial_expr() const { return d_initial_p; }
    var_modes var_mode() const { return d_mode; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    void modify_name(const char *name, east_tree *tree);
    east_var_decl(east_tree *tree,
                  const char *name,
                  east_type *sti,
                  east_node *init,
                  var_modes mode);
    virtual ~east_var_decl();
  private:
    char *d_name_p;
    east_type *d_type_p;
    east_node *d_initial_p;
    var_modes d_mode;
};

class east_var_assign : public east_node {
  public:
    const char *label() const { return d_label_p; }
    east_node *target() const { return d_target_p; }
    east_node *source() const { return d_source_expr_p; }
    east_node *pragma() const { return d_pragma_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_var_assign(east_tree*tree,
                    const char *label,
                    east_node *target,
                    east_node *source_expr,
                    east_node *pragma);
    virtual ~east_var_assign();
  private:
    char *d_label_p;
    east_node *d_target_p;
    east_node *d_source_expr_p;
    east_node *d_pragma_p;
};

class east_wait_stmt : public east_node {
  public:
    const char *label() const { return d_label_p; }
    east_list *sensitivity_list() const { return d_sens_list_p; }
    east_node *cond_clause() const { return d_cond_clause_p; }

    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_wait_stmt(east_tree*tree,const char *label, east_list *sens,east_node *cond);
    virtual ~east_wait_stmt();
  private:
    char *d_label_p;
    east_list *d_sens_list_p;
    east_node *d_cond_clause_p;
};

class east_while_loop : public east_node {
  public:
    const char *label() const { return d_label_p; }
    east_node *condition() const { return d_cond_p; }
    east_list *body() const { return d_stmts_p; }
    east_node *pragma() const { return d_pragma_p; }

    void set_label(char *lbl) { d_label_p = lbl; }
    virtual east_node_type node_type() const;
    virtual void print(east_tree *tree);
    virtual void clear_ref();
    virtual void virt(east_virt *virt);

    east_while_loop(east_tree *tree,
                    const char *label,
                    east_node *cond,
                    east_list *stmts,
                    east_node *pragma);
    virtual ~east_while_loop();
  private:
    char *d_label_p;
    east_node *d_cond_p;
    east_list *d_stmts_p;
    east_node *d_pragma_p;
};

void east_virt::exec(east_node *node) { node->virt(this); }

extern int east_nosynth;

#endif /* INCLUDED_EAST */
