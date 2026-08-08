#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>
#include "cpuInfo.h"
#if !defined(NO_MULTITHREADING)
# include <latch>
# include <thread>
#endif

using namespace std;


// constants
constexpr const char* appName = "2-7-ArchitectureInfo-cpp";


// forward declarations
static inline float getCpuTimestampPeriod();
static inline uint64_t getCpuTimestamp();


// global variables
static unsigned numThreads = 1;
static float cpuTimestampPeriod;


// Convert float value to c-string.
//
// It prints float followed by SI suffix, such as K, M, G, m, u, n, etc.
// for kilo, mega, giga, milli, micro, nano,
// It uses precision of three digits, taking form of one of three variants:
// 1.23, 12.3, or 123, followed by space and SI suffix.
// To make it always the same length, third variant appends a space before the number:
// "1.23 K", "12.3 M", or " 123 G"
// Supported range is from "100 a" to "999 E". Bigger values are converted to "+inf   ".
// Lower values including negative numbers are converted to "   0  ".
static auto formatFloatSI(float v)
{
	// return type with implicit conversion to const char*
	struct SmallString {
		array<char,7> buffer;
		operator const char*() const { return buffer.data(); }
	};
	SmallString s;

	// compute significand and exponent
	int exponent = floorf(log10f(v));
	float divisor = expf(float(exponent - 2) * logf(10));  // this computes exp10f(exponent - 2)
	int significand = int(v / divisor + 0.5f);  // value is >=100 and <1000, actually it might be
		// a little out this range because of small floating computation imprecisions; +0.5 makes proper
		// rounding and avoids underflow to 99, but might cause overflow to 1000 (or even 1001?)

	// convert significand to numbers
	char n[4];
	n[3] = significand % 10;
	significand /= 10;
	n[2] = significand % 10;
	significand /= 10;
	n[1] = significand % 10;
	int thousandNumber = significand / 10;  // thousandNumber is 0 or 1; value 1 is present in some extreme cases
	n[0] = thousandNumber;
	exponent += thousandNumber;  // increment exponent if n contains >=1000

	// make exponent ready to index into SI prefix table
	constexpr const array<char,13> siPrefix = {
		'a', 'f', 'p', 'n', 'u', 'm', ' ', 'K', 'M', 'G', 'T', 'P', 'E'
	};
	exponent += 18;  // make zero exponent point on the ' ' in siPrefixes
	if(exponent < 0) {
		s.buffer = { ' ', ' ', ' ', '0', ' ', ' ', 0 };
		return s;
	}
	if(exponent >= 39) {
		s.buffer = { '+', 'i', 'n', 'f', ' ', ' ', 0 };
		return s;
	}

	// create final string
	s.buffer[6] = 0;
	s.buffer[5] = siPrefix[exponent / 3];
	s.buffer[4] = ' ';
	int dotPos = (exponent % 3) + 1;
	if(dotPos == 3)
		s.buffer[0] = ' ';
	else
		s.buffer[dotPos] = '.';
	s.buffer[3] = '0' + n[3 - thousandNumber];
	s.buffer[dotPos==2 ? 1 : 2] = '0' + n[2 - thousandNumber];
	s.buffer[dotPos==3 ? 1 : 0] = '0' + n[1 - thousandNumber];
	return s;
}


template<typename T> static void shaderFmaComputation1(
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
	T x = T(globalInvocationIdX & 0x3fff) * 0.00001;
	T y = T(globalInvocationIdY & 0x3fff) * 0.00001;
	T z = T(globalInvocationIdZ & 0x3fff) * 0.00001;

	FMA1_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x == T(10)) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y;
	}

#if 0
	// initial values of x, y and z
	T x = globalInvocationIdX;
	T y = globalInvocationIdY;
	T z = globalInvocationIdZ;

	FMA1_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x == 0.1f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y;
	}
#endif
}


template<typename T> static void shaderFmaComputation2(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
	// two computations in parallel, number of used registers: 5
#define FMA2_10 \
	x1 = x1 * y1 + z; \
	x2 = x2 * y2 + z; \
	x1 = x1 * y1 + z; \
	x2 = x2 * y2 + z; \
	x1 = x1 * y1 + z; \
	x2 = x2 * y2 + z; \
	x1 = x1 * y1 + z; \
	x2 = x2 * y2 + z; \
	x1 = x1 * y1 + z; \
	x2 = x2 * y2 + z

#define FMA2_100 \
	FMA2_10; \
	FMA2_10; \
	FMA2_10; \
	FMA2_10; \
	FMA2_10; \
	FMA2_10; \
	FMA2_10; \
	FMA2_10; \
	FMA2_10; \
	FMA2_10

#define FMA2_1000 \
	FMA2_100; \
	FMA2_100; \
	FMA2_100; \
	FMA2_100; \
	FMA2_100; \
	FMA2_100; \
	FMA2_100; \
	FMA2_100; \
	FMA2_100; \
	FMA2_100

	// initial values for the computation
	// (make x1, y1 and z in the range 0.0 to 0.16383,
	// and x2 and y2 less than 0.333)
	T x1 = T(globalInvocationIdX & 0x3fff) * 0.00001;
	T y1 = T(globalInvocationIdY & 0x3fff) * 0.00001;
	T z = T(globalInvocationIdZ & 0x3fff) * 0.00001;
	T x2 = x1 + 0.165f;
	T y2 = y1 + 0.165f;

	FMA2_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x1 == T(10) || x2 == T(10)) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y1 + y2;
	}

#if 0
	// initial values for the computation
	T x1 = globalInvocationIdX;
	T y1 = globalInvocationIdY;
	T z = globalInvocationIdZ;
	T x2 = x1 + 0.5f;
	T y2 = y1 + 0.5f;

	FMA2_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x1 == 0.1f || x2 == 0.1f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y1 + y2;
	}
#endif
}


template<typename T> static void shaderFmaComputation3(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
	// three computations in parallel, number of used registers: 7
#define FMA3_9 \
	x1 = x1 * y1 + z; \
	x2 = x2 * y2 + z; \
	x3 = x3 * y3 + z; \
	x1 = x1 * y1 + z; \
	x2 = x2 * y2 + z; \
	x3 = x3 * y3 + z; \
	x1 = x1 * y1 + z; \
	x2 = x2 * y2 + z; \
	x3 = x3 * y3 + z

#define FMA3_99 \
	FMA3_9; \
	FMA3_9; \
	FMA3_9; \
	FMA3_9; \
	FMA3_9; \
	FMA3_9; \
	FMA3_9; \
	FMA3_9; \
	FMA3_9; \
	FMA3_9; \
	FMA3_9

#define FMA3_1000 \
	FMA3_99; \
	FMA3_99; \
	FMA3_99; \
	FMA3_99; \
	FMA3_99; \
	FMA3_99; \
	FMA3_99; \
	FMA3_99; \
	FMA3_99; \
	FMA3_99; \
	FMA3_9; \
	x1 = x1 * y1 + z

	// initial values for the computation
	// (make x1, y1 and z in the range 0.0 to 0.16383,
	// and x2, y2, x3 and y3 less than 0.333)
	T x1 = T(globalInvocationIdX & 0x3fff) * 0.00001;
	T y1 = T(globalInvocationIdY & 0x3fff) * 0.00001;
	T z = T(globalInvocationIdZ & 0x3fff) * 0.00001;
	T x2 = x1 + 0.165f;
	T y2 = y1 + 0.1f;
	T x3 = x1 + 0.1f;
	T y3 = y1 + 0.165f;

	FMA3_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x1 == T(10) || x2 == T(10) || x3 == T(10)) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y1 + y2 + y3;
	}

#if 0
	// initial values for the computation
	T x1 = globalInvocationIdX;
	T y1 = globalInvocationIdY;
	T z = globalInvocationIdZ;
	T x2 = x1 + 0.5f;
	T y2 = y1 + 0.5f;
	T x3 = x1 + 0.25f;
	T y3 = y1 + 0.25f;

	FMA3_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x1 == 0.1f || x2 == 0.1f || x3 == 0.1f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y1 + y2 + y3;
	}
#endif
}


template<typename T> static void shaderMulAddComputation3(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
	// three computations in parallel, number of used registers: 7

#define MulAdd_Prologue \
	x2 *= y2

#define MulAdd_9 \
	x1 *= y1; \
	x2 += z;  \
	x3 *= y3; \
	x1 += z;  \
	x2 *= y2; \
	x3 += z;  \
	x1 *= y1; \
	x2 += z;  \
	x3 *= y3; \
	x1 += z;  \
	x2 *= y2; \
	x3 += z;  \
	x1 *= y1; \
	x2 += z;  \
	x3 *= y3; \
	x1 += z;  \
	x2 *= y2; \
	x3 += z

#define MulAdd_Epilogue \
	x2 += z

#define MulAdd_99 \
	MulAdd_9; \
	MulAdd_9; \
	MulAdd_9; \
	MulAdd_9; \
	MulAdd_9; \
	MulAdd_9; \
	MulAdd_9; \
	MulAdd_9; \
	MulAdd_9; \
	MulAdd_9; \
	MulAdd_9

#define MulAdd_1000 \
	MulAdd_Prologue; \
	MulAdd_99; \
	MulAdd_99; \
	MulAdd_99; \
	MulAdd_99; \
	MulAdd_99; \
	MulAdd_99; \
	MulAdd_99; \
	MulAdd_99; \
	MulAdd_99; \
	MulAdd_99; \
	MulAdd_9; \
	MulAdd_Epilogue

	// initial values for the computation
	// (make x1, y1 and z in the range 0.0 to 0.16383,
	// and x2, y2, x3 and y3 less than 0.333)
	T x1 = T(globalInvocationIdX & 0x3fff) * 0.00001;
	T y1 = T(globalInvocationIdY & 0x3fff) * 0.00001;
	T z = T(globalInvocationIdZ & 0x3fff) * 0.00001;
	T x2 = x1 + 0.165f;
	T y2 = y1 + 0.1f;
	T x3 = x1 + 0.1f;
	T y3 = y1 + 0.165f;

	MulAdd_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x1 == T(10) || x2 == T(10) || x3 == T(10)) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y1 + y2 + y3;
	}

#if 0
	// initial values for the computation
	T x1 = globalInvocationIdX;
	T y1 = globalInvocationIdY;
	T z = globalInvocationIdZ;
	T x2 = x1 + 0.5f;
	T y2 = y1 + 0.5f;
	T x3 = x1 + 0.25f;
	T y3 = y1 + 0.25f;

	MulAdd_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x1 == 0.1f || x2 == 0.1f || x3 == 0.1f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y1 + y2 + y3;
	}
#endif
}


template<typename T> static void shaderFmaComputation4(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
	// four computations in parallel, number of used registers: 9
#define FMA4_8 \
	x1 = x1 * y1 + z; \
	x2 = x2 * y2 + z; \
	x3 = x3 * y3 + z; \
	x4 = x4 * y4 + z; \
	x1 = x1 * y1 + z; \
	x2 = x2 * y2 + z; \
	x3 = x3 * y3 + z; \
	x4 = x4 * y4 + z

#define FMA4_100 \
	FMA4_8; \
	FMA4_8; \
	FMA4_8; \
	FMA4_8; \
	FMA4_8; \
	FMA4_8; \
	FMA4_8; \
	FMA4_8; \
	FMA4_8; \
	FMA4_8; \
	FMA4_8; \
	FMA4_8; \
	x1 = x1 * y1 + z; \
	x2 = x2 * y2 + z; \
	x3 = x3 * y3 + z; \
	x4 = x4 * y4 + z

#define FMA4_1000 \
	FMA4_100; \
	FMA4_100; \
	FMA4_100; \
	FMA4_100; \
	FMA4_100; \
	FMA4_100; \
	FMA4_100; \
	FMA4_100; \
	FMA4_100; \
	FMA4_100

	// initial values for the computation
	// (make x1, y1 and z in the range 0.0 to 0.16383,
	// and x2, y2, x3, y3, x4 and y4 less than 0.333)
	T x1 = T(globalInvocationIdX & 0x3fff) * 0.00001;
	T y1 = T(globalInvocationIdY & 0x3fff) * 0.00001;
	T z = T(globalInvocationIdZ & 0x3fff) * 0.00001;
	T x2 = x1 + 0.165f;
	T y2 = y1 + 0.1f;
	T x3 = x1 + 0.05f;
	T y3 = y1 + 0.165f;
	T x4 = x1 + 0.1f;
	T y4 = y1 + 0.05f;

	FMA4_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x1 == T(10) || x2 == T(10) || x3 == T(10) || x4 == T(10)) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y1 + y2 + y3 + y4;
	}

#if 0
	// initial values for the computation
	T x1 = globalInvocationIdX;
	T y1 = globalInvocationIdY;
	T z = globalInvocationIdZ;
	T x2 = x1 + 0.5f;
	T y2 = y1 + 0.5f;
	T x3 = x1 + 0.25f;
	T y3 = y1 + 0.25f;
	T x4 = x1 + 0.75f;
	T y4 = y1 + 0.75f;

	FMA4_1000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x1 == 0.1f || x2 == 0.1f || x3 == 0.1f || x4 == 0.1f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y1 + y2 + y3 + y4;
	}
#endif

#if 0
	array<T,4> x;
	array<T,4> y;
	array<T,4> z;
	x[0] = T(globalInvocationIdX);
	x[1] = T(globalInvocationIdX) + 0.5f;
	x[2] = T(globalInvocationIdX) + 0.25f;
	x[3] = T(globalInvocationIdX) + 0.75f;
	y[0] = T(globalInvocationIdY);
	y[1] = T(globalInvocationIdY) + 0.5f;
	y[2] = T(globalInvocationIdY) + 0.25f;
	y[3] = T(globalInvocationIdY) + 0.75f;
	z[0] = T(globalInvocationIdZ);
	z[1] = T(globalInvocationIdZ);
	z[2] = T(globalInvocationIdZ);
	z[3] = T(globalInvocationIdZ);
	for(size_t i=0; i<10000; i++) {
		x[0] = x[0] * y[0] + z[0];
		x[1] = x[1] * y[1] + z[1];
		x[2] = x[2] * y[2] + z[2];
		x[3] = x[3] * y[3] + z[3];
	}

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x[0] == 0.1f || x[1] == 0.1f || x[2] == 0.1f || x[3] == 0.1f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y[0] + y[1] + y[2] + y[3];
	}
#endif
}


static void workgroupInvocation(void (*func)(unsigned, unsigned, unsigned),
	unsigned workgroupIdX, unsigned workgroupIdY, unsigned workgroupIdZ)
{
	// call 128 shader invocations
	// each processing 20'000 floating instructions
	for(unsigned y=0; y<4; y++)
		for(unsigned x=0; x<32; x++)
			func(
				workgroupIdX*32 + x,
				workgroupIdY*4 + y,
				workgroupIdZ
			);
}


int main(int argc, char* argv[])
{
	// catch exceptions
	// (vk functions throw if they fail)
	try {

		bool printHelp = false;
		for(int i=1; i<argc; i++) {

			// parse number of threads
			if(argv[i][0] >= '0' && argv[i][0] <= '9') {
				char* endp = nullptr;
				numThreads = strtoul(argv[i], &endp, 10);
				if(numThreads == 0 || endp == argv[i] || ((endp != nullptr) && (*endp != 0)))
					printHelp = true;
				continue;
			}

			// parse options starting with '-'
			if(argv[i][0] == '-') {

				// print help
				if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
					printHelp = true;
					continue;
				}

				printHelp = true;
				continue;
			}

		}

		// print info
		if(printHelp) {
			cout << appName << " prints the performance of your CPU\n"
			        "\n"
			        "Usage: " << appName << " [numThreads]\n"
			        "\n"
			        "numThreads - run test using specified number of threads.\n"
			        "             Print the total performance of all the threads.\n"
			        "             If omitted, single threaded test is used.\n"
			        "\n"
			        "To measure the maximum performance, make sure that this application\n"
			        "is compiled in release mode with optimizations turned on and\n"
			        "with the features your processor supports. For instance,\n"
			        "omission to compile with SSE or AVX support while CPU supports it\n"
			        "would lead to lower performance results as SSE or AVX instructions\n"
			        "will not be used." << endl;
			return 99;
		}

		// only single thread if NO_MULTITHREADING
	#if defined(NO_MULTITHREADING)
		if(numThreads != 1) {
			cout << "Requested " << numThreads << " threads, but the application was compiled\n"
			        "without multithreading support. Terminating." << endl;
			return 99;
		}
	#endif

		// processor info
		cout << "Processor info:" << endl;
		printCpuInfo();

		// perform computation of all workgroups
		auto performTest =
			[&](void (*shaderInvocationFunc)(unsigned, unsigned, unsigned), size_t numWorkgroups) -> float {

				// compute workgroup grid dimensions
				// (avoid any dimension to go over 10000)
				uint32_t workgroupCountX;
				uint32_t workgroupCountY;
				uint32_t workgroupCountZ;
				if(numWorkgroups > 10000 * 10000) {
					workgroupCountZ = 1 + ((numWorkgroups - 1) / (10000 * 10000));
					uint64_t remainder = numWorkgroups / workgroupCountZ;
					workgroupCountY = 1 + ((remainder - 1) / 10000);
					workgroupCountX = remainder / workgroupCountY;
				}
				else {
					if(numWorkgroups == 0)
						numWorkgroups = 1;
					workgroupCountZ = 1;
					workgroupCountY = 1 + ((numWorkgroups - 1) / 10000);
					workgroupCountX = numWorkgroups / workgroupCountY;
				}

				// perform computation
				uint64_t ts1, ts2;
				if(numThreads == 1) {
					ts1 = getCpuTimestamp();
					for(unsigned z=0; z<workgroupCountZ; z++)
						for(unsigned y=0; y<workgroupCountY; y++)
							for(unsigned x=0; x<workgroupCountX; x++)
								workgroupInvocation(shaderInvocationFunc, x, y, z);
					ts2 = getCpuTimestamp();
				}
				else {

				#if !defined(NO_MULTITHREADING)

					latch l1(1);
					latch l2(numThreads);
					auto worker =
						[&](unsigned id) {
							l1.wait();
							unsigned x = id;
							unsigned y = 0;
							unsigned z = 0;
							do {
								while(x >= workgroupCountX) {
									x -= workgroupCountX;
									y++;
									if(y >= workgroupCountY) {
										y -= workgroupCountY;
										z++;
										if(z >= workgroupCountZ)
											goto workerDone;
									}
								}
								workgroupInvocation(shaderInvocationFunc, x, y, z);
								x += numThreads;
							}
							while(true);
						workerDone:
							l2.arrive_and_wait();
						};
					vector<thread> threadList;
					threadList.reserve(numThreads - 1);
					unsigned i = 1;
					while(i < numThreads) {
						threadList.emplace_back(worker, i);
						i++;
					}

					ts1 = getCpuTimestamp();
					l1.count_down();
					worker(0);
					ts2 = getCpuTimestamp();
					for(auto& t : threadList)
						t.join();

				#endif

				}

				// return time as float in seconds
				return float(ts2 - ts1) * cpuTimestampPeriod;

			};

		// record the performance in the list
		auto processResult =
			[](float time, size_t numWorkgroups, vector<float>& performanceList) {
				if(time >= 0.01f) {
					uint64_t numInstructions = uint64_t(2000) * 128 * numWorkgroups;
					float performance = float(numInstructions) / time;
					performanceList.push_back(performance);
				}
			};

		// compute number of workgroups
		// to reach computation time of about 20ms
		auto computeNumWorkgroups =
			[](size_t lastNumWorkgroups, float lastTime) -> size_t
			{
				constexpr float targetTime = 0.02;
				if(lastTime < (targetTime / 10.f)) {
					// multiply numWorkgroups by 10
					return lastNumWorkgroups * 10;
				}
				else {
					// multiply numWorkgroups by ratio
					float ratio = targetTime / lastTime;
					size_t newNumWorkgroups = size_t(lastNumWorkgroups * ratio);
					return (newNumWorkgroups >= 1) ? newNumWorkgroups : 1;
				}
			};

		// run tests
		cout << "Running tests using ";
		if(numThreads == 1)  cout << "1 thread..." << endl;
		else  cout << numThreads << " threads..." << endl;
		constexpr const size_t arraySize = 10;
		array<size_t,arraySize> numWorkgroups = { 1,1,1,1,1, 1,1,1,1,1 };
		array<vector<float>,arraySize> performanceList;
		cpuTimestampPeriod = getCpuTimestampPeriod();
		chrono::time_point startTime = chrono::high_resolution_clock::now();
		do {

			// perform tests
			array<float,arraySize> t;
			t[0] = performTest(shaderFmaComputation1<float>, numWorkgroups[0]);
			t[1] = performTest(shaderFmaComputation2<float>, numWorkgroups[1]);
			t[2] = performTest(shaderFmaComputation3<float>, numWorkgroups[2]);
			t[3] = performTest(shaderFmaComputation4<float>, numWorkgroups[3]);
			t[4] = performTest(shaderMulAddComputation3<float>, numWorkgroups[4]);
			t[5] = performTest(shaderFmaComputation1<double>, numWorkgroups[5]);
			t[6] = performTest(shaderFmaComputation2<double>, numWorkgroups[6]);
			t[7] = performTest(shaderFmaComputation3<double>, numWorkgroups[7]);
			t[8] = performTest(shaderFmaComputation4<double>, numWorkgroups[8]);
			t[9] = performTest(shaderMulAddComputation3<double>, numWorkgroups[9]);
			for(size_t i=0; i<arraySize; i++)
				processResult(t[i], numWorkgroups[i], performanceList[i]);

			// stop measurements after three seconds
			double totalTime = chrono::duration<double>(chrono::high_resolution_clock::now() - startTime).count();
			if(totalTime >= 3.)
				break;

			// compute new numWorkgroups
			for(size_t i=0; i<arraySize; i++)
				numWorkgroups[i] = computeNumWorkgroups(numWorkgroups[i], t[i]);

		} while(true);

		// sort the results
		for(size_t i=0; i<arraySize; i++)
			sort(performanceList[i].begin(), performanceList[i].end());

		// print results
		auto printResult =
			[](const string_view text, bool supported, const vector<float>& performanceList) {
				cout << text;
				if(supported) {
					if(performanceList.empty())
						cout << "measurement error" << endl;
					else {

						// print median
						cout << formatFloatSI(performanceList[performanceList.size()/2]) << "FLOPS";

						// print dispersion using IQR (Interquartile Range);
						// Q1 is the value in 25% and Q3 in 75%
						cout << "  (Q1: " << formatFloatSI(performanceList[performanceList.size()/4]) << "FLOPS,";
						cout << " Q3: " << formatFloatSI(performanceList[performanceList.size()*3/4]) << "FLOPS)";
						cout << endl;
					}
				}
				else
					cout << "not supported" << endl;
			};
		cout << "Float (float32) performance\n";
		printResult("   non-parallel FMA:    ", true, performanceList[0]);
		printResult("   2 parallel FMA:      ", true, performanceList[1]);
		printResult("   3 parallel FMA:      ", true, performanceList[2]);
		printResult("   4 parallel FMA:      ", true, performanceList[3]);
		printResult("   3 parallel Mul+Add:  ", true, performanceList[4]);
		cout << "Double (float64) performance\n";
		printResult("   non-parallel FMA:    ", true, performanceList[5]);
		printResult("   2 parallel FMA:      ", true, performanceList[6]);
		printResult("   3 parallel FMA:      ", true, performanceList[7]);
		printResult("   4 parallel FMA:      ", true, performanceList[8]);
		printResult("   3 parallel Mul+Add:  ", true, performanceList[9]);

	// catch exceptions
	} catch(exception& e) {
		cout << "Failed because of exception: " << e.what() << endl;
	} catch(...) {
		cout << "Failed because of unspecified exception." << endl;
	}

	return 0;
}


#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN  // exclude rarely-used services inclusion by windows.h; this speeds up compilation and avoids some compilation problems
# include <windows.h>  // we include windows.h only at the end of file to avoid compilation problems; windows.h define MemoryBarrier, near, far and many other problematic macros
#endif
static inline float getCpuTimestampPeriod()
{
#ifdef _WIN32
	LARGE_INTEGER f;
	QueryPerformanceFrequency(&f);
	return 1.f / f.QuadPart;
#else
	return 1e-9f;  // on Linux, we use clock_gettime()
#endif
}


static inline uint64_t getCpuTimestamp()
{
#ifdef _WIN32
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return uint64_t(counter.QuadPart);
#else
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tv);
	return tv.tv_nsec + tv.tv_sec*1000000000ull;
#endif
}
