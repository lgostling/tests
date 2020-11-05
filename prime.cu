// Elapsed Real Time for input-4.txt:
// Type of GPU:

#include <stdio.h>
#include <stdbool.h>
#include <cuda_runtime.h>

// Resizable array for a list of values to test for prime-ness
int *vList;

// Number of values on the list.
int vCount = 0;

// Capacity of the list of values.
int vCap = 0;

// General function to report a failure and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Print out a usage message, then exit.
static void usage() {
  printf( "usage: prime [report]\n" );
  exit( 1 );
}

// Read the list of numbers to test for prime-ness
void readList() {
  // Set up initial list and capacity.
  vCap = 5;
  vList = (int *) malloc( vCap * sizeof( int ) );

  // Keep reading as many values as we can.
  int v;
  while ( scanf( "%d", &v ) == 1 ) {
    // Grow the list if needed.
    if ( vCount >= vCap ) {
      vCap *= 2;
      vList = (int *) realloc( vList, vCap * sizeof( int ) );
    }

    // Store the latest value.
    vList[ vCount++ ] = v;
  }
}

// Check my assigned element of vList to see if it's prime (you'll need to add
// some parameters)
__global__ void checkPrime( int vCount,
                            bool report,
                            int* values,
                            int* results ) {
  int idx = blockDim.x * blockIdx.x + threadIdx.x;

  if ( idx < vCount ) {
    bool prime = true;
    int value = values[idx];
    if(value % 2 == 0) {
      prime = false;
    } else {
      for(int i = 3; i * i <= value; i +=2 ){
        if(value % i == 0){
          prime = false;
          break;
        }
      }
    }
    if(prime){
      fprintf(stdout, "%d\n", value);
      results[idx] = 1;
    } else{
      results[idx] = 0;
    }
  }
}

int main( int argc, char *argv[] ) {
  if ( argc < 1 || argc > 2 )
    usage();

  // If there's an argument, it better be "report"
  bool report = false;
  if ( argc == 2 ) {
    if ( strcmp( argv[ 1 ], "report" ) != 0 )
      usage();
    report = true;
  }

  readList();

  // Add code to allocate memory on the device and copy over the list.
  int *valueList = NULL;
  if ( cudaMalloc((void **)&valueList, vCount * sizeof(int) ) != cudaSuccess )
    fail( "Failed to allocate space for lenght list on device" );

  // Copy the list over to the device.
  if ( cudaMemcpy( valueList, vList, vCount * sizeof(int),
                   cudaMemcpyHostToDevice) != cudaSuccess )
    fail( "Failed to copy list to device" );

  // Add code to allocate space on the device to hold the results.
  int *resultsList = NULL;
  resultsList = (int *) malloc( vCount * sizeof( int ) );
  if ( cudaMalloc((void **)&resultsList, vCount * sizeof(int) ) != cudaSuccess )
    fail( "Failed to allocate space for lenght list on device" );

  // Block and grid dimensions.
  int threadsPerBlock = 100;
  // Round up.
  int blocksPerGrid = ( vCount + threadsPerBlock - 1 ) / threadsPerBlock;

  // Run our kernel on these block/grid dimensions (you'll need to add some parameters)
  checkPrime<<<blocksPerGrid, threadsPerBlock>>>( vCount, report, *valueList, *resultsList );
  if ( cudaGetLastError() != cudaSuccess )
    fail( "Failure in CUDA kernel execution." );

  // Add code to copy results back to the host then add up the total number
  // of primes found.
  
  int* results = malloc(vCount * sizeof(int));

  if ( cudaMemcpy( results, resultsList, vCount * sizeof(int),
                   cudaMemcpyDeviceToHost) != cudaSuccess )
    fail( "Can't copy list from device to host" );
  
  int primeCount = 0;
  for(int i = 0; i < vCount; i++) {
    if(results[i] == 1) {
      primeCount++;
    }
  }

  fprintf(stdout, "Prime count: %d\n", primeCount);

  // Free memory on the device and the host.
  cudaFree(resultsList);
  cudaFree(valueList);
  free( results )
  free( vList );

  cudaDeviceReset();

  return 0;
}
