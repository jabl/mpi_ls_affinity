/*
Copyright (c) 2013 Janne Blomqvist

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <hwloc.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef MPI
#include <mpi.h>
#endif

static hwloc_topology_t topo;

static void print_cpu_bind(FILE* f, char* name, int rank, int threadid)
{
	char* s;
	hwloc_bitmap_t cpuset = hwloc_bitmap_alloc();

	/* get the current thread CPU location */
	hwloc_get_cpubind(topo, cpuset, HWLOC_CPUBIND_THREAD);
	hwloc_bitmap_list_asprintf(&s, cpuset);
	hwloc_bitmap_free(cpuset);
	fprintf(f, "On host %s, MPI rank %d thread %d bound to PU(s) %s\n", 
		name, rank, threadid, s);
	free(s);
}

int main(int argc, char **argv)
{
	int rank = 0;
	char name[HOST_NAME_MAX];

#ifdef MPI
	int provided;
	MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#ifdef _OPENMP
	if (provided == MPI_THREAD_SINGLE && rank == 0)
		fprintf(stderr, "WARNING: Only MPI_THREAD_SINGLE available,"
			" continuing anyway.\n");
#endif
#endif

	hwloc_topology_init(&topo);
	hwloc_topology_load(topo);

	if (gethostname(name, HOST_NAME_MAX)) {
		perror("gethostname failed");
		abort();
	}

	#pragma omp parallel
	{
#ifdef _OPENMP
		int threadid = omp_get_thread_num();
#else
		int threadid = 0;
#endif
		print_cpu_bind(stdout, name, rank, threadid);
	}

#ifdef MPI
	MPI_Finalize();
#endif
	return 0;
}
