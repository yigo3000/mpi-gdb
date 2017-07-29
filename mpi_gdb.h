/*
Copyright (c) 2017 Adam Dodd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef MPI_GDB_H
#define MPI_GDB_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include <openmpi/mpi.h>

#ifndef __linux
#error Unsupported architecture
#endif

#ifndef NDEBUG
#define MPI_GDB_INIT(x) mpigdb_init_func(x)
#else
#define initMPIGDB(x) {}
#endif

#ifdef __cplusplus
extern "C" {
#endif

void mpigdb_init_func()
{
   int mpiInit;
   int mpiRank;
   int pid;
   int forkRes;
   int colSize;
   char arg0[24];
   char title[64];
   char gdbCmd[64];
   char geometry[32];
   
   MPI_Initialized(&mpiInit);
   
   if (!mpiInit)
   {
      fprintf(stderr, "MPI has not been initialized\n");
      abort();
   }
   
   MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
   
   colSize = 3;
   pid = getpid();
   forkRes = fork();
   
   sprintf(arg0, "mpi %.02d: xterm", mpiRank);
   sprintf(title, "Rank %.02d", mpiRank);
   sprintf(gdbCmd, "gdb --pid %d -ex \"finish\" -ex \"finish\" "
           "-ex \"set var start = 1\" -ex \"finish\"", pid);
   /* Automatically unwind the stack back to the user's code (may not work on
    * some archs) */
   
   int column = (mpiRank / colSize) * 506;
   int row = (mpiRank % colSize) * 344 + 25;
   /* These vars make the xterms appear neatly on screen */
   
   sprintf(geometry, "80x24+%d+%d", column, row);
   
   if (forkRes == 0)
   {
      if ((execlp("xterm", arg0, "-geometry", geometry, "-title", title,
                  "-e", gdbCmd, NULL)) == -1)
      {
         perror("execlp");
         MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
      }
   }
   else if (forkRes == -1)
   {
      perror("fork");
      MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
   }
   
   int start = 0;
   
   while (start == 0)
      sleep(1);
}

#ifdef __cplusplus
}
#endif

#endif
