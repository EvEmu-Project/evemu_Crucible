/*
  Copyright (c) 1999 Rafal Wojtczuk <nergal@avet.com.pl>. All rights reserved.
  See the file COPYING for license details.
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "tcp.h"
#include "util.h"
#include "nids.h"

void
nids_no_mem(char *func)
{
  fprintf(stderr, "Out of memory in %s.\n", func);
  exit(1);
}

char *
test_malloc(int x)
{
  char *ret = malloc(x);
  
  if (!ret)
    nids_params.no_mem("test_malloc");

  return ret;
}

inline int
before(u_int seq1, u_int seq2)
{
  return ((int)(seq1 - seq2) < 0);
}

inline int
after(u_int seq1, u_int seq2)
{
  return ((int)(seq2 - seq1) < 0);
}

void
register_callback(struct proc_node **procs, void (*x))
{
  struct proc_node *ipp;

  for (ipp = *procs; ipp; ipp = ipp->next)
    if (x == ipp->item)
      return;
  ipp = mknew(struct proc_node);
  ipp->item = x;
  ipp->next = *procs;
  *procs = ipp;
}

void
unregister_callback(struct proc_node **procs, void (*x))
{
  struct proc_node *ipp;
  struct proc_node *ipp_prev = 0;

  for (ipp = *procs; ipp; ipp = ipp->next) {
    if (x == ipp->item) {
      if (ipp_prev)
	ipp_prev->next = ipp->next;
      else
	*procs = ipp->next;
      free(ipp);
      return;
    }
    ipp_prev = ipp;
  }
}
