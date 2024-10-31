#ifndef POLYSTAN_MPI_HPP_
#define POLYSTAN_MPI_HPP_

#ifdef USE_MPI
#include <mpi.h>
#endif

namespace polystan {
namespace mpi {

void initialize() {
#ifdef USE_MPI
  MPI_Init(NULL, NULL);
#endif
}

void finalize() {
#ifdef USE_MPI
  MPI_Finalize();
#endif
}

#ifdef USE_MPI
MPI_Comm dup_comm() {
  MPI_Comm comm;
  MPI_Comm_dup(MPI_COMM_WORLD, &comm);
  return comm;
}

MPI_Comm& get_comm() {
  static MPI_Comm comm = dup_comm();
  return comm;
}

int get_size() {
  int size;
  MPI_Comm_size(get_comm(), &size);
  return size;
}
#endif

void barrier() {
#ifdef USE_MPI
  MPI_Barrier(get_comm());
#endif
}

bool is_rank_zero() {
#ifdef USE_MPI
  int rank;
  MPI_Comm_rank(get_comm(), &rank);
  return rank == 0;
#else
  return true;
#endif
}

}  // end namespace mpi
}  // end namespace polystan

#endif  // POLYSTAN_MPI_HPP_
