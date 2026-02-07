#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

using namespace std;


// constants
constexpr const char* appName = "2-6-ArchitectureInfo-cpp";


// forward declarations
static inline float getCpuTimestampPeriod();
static inline uint64_t getCpuTimestamp();


// global variables
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


template<typename T> static void shaderInvocation1(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
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

#define FMA1_10000 \
	FMA1_1000; \
	FMA1_1000; \
	FMA1_1000; \
	FMA1_1000; \
	FMA1_1000; \
	FMA1_1000; \
	FMA1_1000; \
	FMA1_1000; \
	FMA1_1000; \
	FMA1_1000

	// initial values of x, y and z
	T x = globalInvocationIdX;
	T y = globalInvocationIdY;
	T z = globalInvocationIdZ;

	FMA1_10000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x == 0.1f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y;
	}
}


template<typename T> static void shaderInvocation2(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
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

#define FMA2_10000 \
	FMA2_1000; \
	FMA2_1000; \
	FMA2_1000; \
	FMA2_1000; \
	FMA2_1000; \
	FMA2_1000; \
	FMA2_1000; \
	FMA2_1000; \
	FMA2_1000; \
	FMA2_1000

	// initial values of x, y and z
	T x1 = globalInvocationIdX;
	T y1 = globalInvocationIdY;
	T z = globalInvocationIdZ;
	T x2 = x1 + 0.5f;
	T y2 = y1 + 0.5f;

	FMA2_10000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x1 == 0.1f || x2 == 0.1f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y1 + y2;
	}
}


template<typename T> static void shaderInvocation3(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
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

#define FMA3_999 \
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
	FMA3_9

#define FMA3_10000 \
	FMA3_999; \
	FMA3_999; \
	FMA3_999; \
	FMA3_999; \
	FMA3_999; \
	FMA3_999; \
	FMA3_999; \
	FMA3_999; \
	FMA3_999; \
	FMA3_999; \
	FMA3_9; \
	x1 = x1 * y1 + z

	// initial values of x, y and z
	T x1 = globalInvocationIdX;
	T y1 = globalInvocationIdY;
	T z = globalInvocationIdZ;
	T x2 = x1 + 0.5f;
	T y2 = y1 + 0.5f;
	T x3 = x1 + 0.25f;
	T y3 = y1 + 0.25f;

	FMA3_10000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x1 == 0.1f || x2 == 0.1f || x3 == 0.1f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y1 + y2 + y3;
	}
}


template<typename T> static void shaderInvocation4(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
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

#define FMA4_10000 \
	FMA4_1000; \
	FMA4_1000; \
	FMA4_1000; \
	FMA4_1000; \
	FMA4_1000; \
	FMA4_1000; \
	FMA4_1000; \
	FMA4_1000; \
	FMA4_1000; \
	FMA4_1000

	// initial values of x, y and z
	T x1 = globalInvocationIdX;
	T y1 = globalInvocationIdY;
	T z = globalInvocationIdZ;
	T x2 = x1 + 0.5f;
	T y2 = y1 + 0.5f;
	T x3 = x1 + 0.25f;
	T y3 = y1 + 0.25f;
	T x4 = x1 + 0.75f;
	T y4 = y1 + 0.75f;

	FMA4_10000;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x1 == 0.1f || x2 == 0.1f || x3 == 0.1f || x4 == 0.1f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*reinterpret_cast<T*>(size_t(globalInvocationIdZ)) = y1 + y2 + y3 + y4;
	}
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
			        "Only single-threaded measurement is performed.\n"
			        "To measure the maximum performance, make sure that this binary\n"
			        "is compiled in release mode with optimizations turned on and\n"
			        "with the features your processor supports. For instance,\n"
			        "omission to compile with SSE support while CPU supports it\n"
			        "would lead to lower performance results as SSE instructions\n"
			        "will not be used." << endl;
			return 99;
		}

		// processor info
		cout << "Processor info:\n"
		        " < missing >" << endl;

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
				uint64_t ts1 = getCpuTimestamp();
				for(unsigned z=0; z<workgroupCountZ; z++)
					for(unsigned y=0; y<workgroupCountY; y++)
						for(unsigned x=0; x<workgroupCountX; x++)
							workgroupInvocation(shaderInvocationFunc, x, y, z);
				uint64_t ts2 = getCpuTimestamp();

				// return time as float in seconds
				return float(ts2 - ts1) * cpuTimestampPeriod;

			};

		auto processResult =
			[](float time, size_t numWorkgroups, vector<float>& performanceList) {
				if(time >= 0.01f) {
					uint64_t numInstructions = uint64_t(20000) * 128 * numWorkgroups;
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

		constexpr const size_t arraySize = 8;
		array<size_t,arraySize> numWorkgroups = { 1,1,1,1, 1,1,1,1 };
		array<vector<float>,arraySize> performanceList;
		cpuTimestampPeriod = getCpuTimestampPeriod();
		chrono::time_point startTime = chrono::high_resolution_clock::now();
		do {

			// perform tests
			array<float,arraySize> t;
			t[0] = performTest(shaderInvocation1<float>, numWorkgroups[0]);
			t[1] = performTest(shaderInvocation2<float>, numWorkgroups[1]);
			t[2] = performTest(shaderInvocation3<float>, numWorkgroups[2]);
			t[3] = performTest(shaderInvocation4<float>, numWorkgroups[3]);
			t[4] = performTest(shaderInvocation1<double>, numWorkgroups[4]);
			t[5] = performTest(shaderInvocation2<double>, numWorkgroups[5]);
			t[6] = performTest(shaderInvocation4<double>, numWorkgroups[6]);
			t[7] = performTest(shaderInvocation4<double>, numWorkgroups[7]);
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
						cout << "  (Q1: " << formatFloatSI(performanceList[performanceList.size()/4]) << "FLOPS";
						cout << " Q3: " << formatFloatSI(performanceList[performanceList.size()/4*3]) << "FLOPS)";
						cout << endl;
					}
				}
				else
					cout << "not supported" << endl;
			};
		printResult("Float (float32) performance for parallelism level 1:   ", true, performanceList[0]);
		printResult("Float (float32) performance for parallelism level 2:   ", true, performanceList[1]);
		printResult("Float (float32) performance for parallelism level 3:   ", true, performanceList[2]);
		printResult("Float (float32) performance for parallelism level 4:   ", true, performanceList[3]);
		printResult("Double (float64) performance for parallelism level 1:  ", true, performanceList[4]);
		printResult("Double (float64) performance for parallelism level 2:  ", true, performanceList[5]);
		printResult("Double (float64) performance for parallelism level 3:  ", true, performanceList[6]);
		printResult("Double (float64) performance for parallelism level 4:  ", true, performanceList[7]);

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
