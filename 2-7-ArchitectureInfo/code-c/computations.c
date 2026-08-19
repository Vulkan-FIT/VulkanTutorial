

void fmaFloatComputation1(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
	// no parallelism, number of used registers: 3
#define FMA1_10 \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z

#define FMA1_100 \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10

#define FMA1_1000 \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100

	// initial values for the computation
	// (make each variable in the range 0.0 to 0.16383)
	float x = (float)(globalInvocationIdX & 0x3fff) * 0.00001;
	float y = (float)(globalInvocationIdY & 0x3fff) * 0.00001;
	float z = (float)(globalInvocationIdZ & 0x3fff) * 0.00001;

	FMA1_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x == 10.f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*(float*)(globalInvocationIdZ) = y;
	}

}


void fmaDoubleComputation1(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
	// no parallelism, number of used registers: 3
#define FMA1_10 \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z; \
	x = x * y + z

#define FMA1_100 \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10; \
	FMA1_10

#define FMA1_1000 \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100; \
	FMA1_100

	// initial values for the computation
	// (make each variable in the range 0.0 to 0.16383)
	double x = (double)(globalInvocationIdX & 0x3fff) * 0.00001;
	double y = (double)(globalInvocationIdY & 0x3fff) * 0.00001;
	double z = (double)(globalInvocationIdZ & 0x3fff) * 0.00001;

	FMA1_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x == 10.) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*(double*)(globalInvocationIdZ) = y;
	}

}
