#define _XOPEN_SOURCE 700

#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char * argv[]){

	// required
  MPI_Init(&argc, &argv); 

  int nmb_mpi_proc, mpi_rank;

	// query the size of communicator
	// => number of processes participating in computation
  MPI_Comm_size(MPI_COMM_WORLD, &nmb_mpi_proc);

	// get rank of this specific process
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

  if ( mpi_rank == 0 ) {
    printf( "Number of processes: %d\n", nmb_mpi_proc );
    {
      int msg = 1; int len = 1; int dest_rank = 1; int tag = 1;
      MPI_Send(&msg, len, MPI_INT, dest_rank, tag, MPI_COMM_WORLD);
      printf( "MPI message sent from %d: %d\n", mpi_rank, msg );
    }

    {
      int msg; int max_len = 1; int src_rank = 1; int tag = 1;
      MPI_Recv(&msg, max_len, MPI_INT, src_rank, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf( "MPI message received at %d: %d\n", mpi_rank, msg );
    }
  }
  
  // NOTE: We will print in one of worker processes here. This is only OK,
  // because we know exactly the setup in which we run this example program and
  // can live with interleaved text output, if it happens occassionally.
  else if ( mpi_rank == 1 ) {
    int msg;
    
    {
      int max_len = 1; int src_rank = 0; int tag = 1;
      MPI_Recv(&msg, max_len, MPI_INT, src_rank, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf( "MPI message received at %d: %d\n", mpi_rank, msg );
    }

    ++msg;

    {
      int len = 1; int dest_rank = 0; int tag = 1;
      MPI_Send(&msg, len, MPI_INT, dest_rank, tag, MPI_COMM_WORLD);
      printf( "MPI message sent from %d: %d\n", mpi_rank, msg );
    }


  }
	// equivalent to freeing all resources
  MPI_Finalize(); 


  return 0;
}