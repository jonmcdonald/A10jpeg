/*
 * HEADER
 *    east_util.h - utilities for use within the EAST system
 *
 * COPYRIGHT
 *    Copyright (c) MENTOR GRAPHICS CORPORATION 1995 All Rights Reserved 
 *
 * WRITTEN BY:
 *    many, Dave Clemans
 *
 * DESCRIPTION
 *    This module provides a simple hash table and heap system to support
 *    creating and using EAST trees.
 */     

#ifndef INCLUDED_EAST_UTIL
#define INCLUDED_EAST_UTIL

#include <stddef.h>

#if defined(vco_ixl)
/* prevent "declaration of `malloc(size_t)' throws different exceptions" */
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#if defined(vco_ixn)
#define strcasecmp stricmp
#define strncasecmp _strnicmp
#endif

struct east_heap_memblock
{
   long                    size;
   east_heap_memblock      *next;
};

class east_heap
   // A simple "heap", or pool-based memory allocator
{
public:
   void *operator new(size_t size)
      { return malloc(size); }
   void operator delete(void *addr)
      { free(addr); }
   east_heap(long chunk = 32*1024);
   ~east_heap();

   void *allocate(long size);
   void deallocate(void *addr);

   char *alloc_str(const char str[]);
   char *alloc_str(const char str1[], const char str2[]);

   static east_heap *find_heap(void *addr);

private:

   void *getmem(long size);
   void freemem(void *addr,long size);

   east_heap_memblock *arena;
   east_heap_memblock *arenaTail;
   east_heap_memblock *pool;
   long poolSize;
   long chunkSize;
   long mallocTotal;
   long mallocHighWater;

   east_heap *next;
   east_heap *prev;
};

class east_tree_symtab {
public:

    void * operator new(size_t size, east_heap *heap_p)
      { return heap_p->allocate(size); }
    void * operator new(size_t size) 
      { (void)size; fprintf(stderr,"Illegal use of east_tree_symtab default new operator.\n"); return (void *)1; }
    void operator delete(void *addr)
      { east_heap *heap_p = east_heap::find_heap(addr); heap_p->deallocate(addr); }

    east_tree_symtab(int size,east_heap *heap = NULL);
    ~east_tree_symtab();
    void *lookup(char *);
    void *add(char*, void *);
    void change(char *,void *);

private:
    int hash(char*);
    east_heap *d_heap;
    int d_created_heap;
    int d_size;
    struct east_tree_hashlink **d_store;
};

struct east_tree_hashlink {
    char *d_name;
    void *d_data;
    struct east_tree_hashlink *d_next_p;
};

#endif /* INCLUDED_EAST_UTIL */
