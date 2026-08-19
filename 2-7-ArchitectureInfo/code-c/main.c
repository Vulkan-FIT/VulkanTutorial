#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#if defined(__DOS__)
# include <bios.h>
#endif
#include "FloatVector.h"

static const char appName[] = "2-7-ArchtectureInfo-c";
static float timestampPeriod = 1.f / CLOCKS_PER_SEC;

void printCpuInfo();
void fmaFloatComputation1(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ);
void fmaDoubleComputation1(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ);


static unsigned long getTimestamp()
{
#if 0
	long startTicks;
	_bios_timeofday(_TIME_GETCLOCK, &startTicks);
	return startTicks;
#else
	return (unsigned long)clock();
#endif
}


float performTest(void (*invocationFunc)(unsigned,unsigned,unsigned), unsigned long numWorkgroups)
{
	unsigned workgroupCountX;
	unsigned workgroupCountY;
	unsigned workgroupCountZ;
	unsigned long ts1, ts2;
	unsigned x,y,z;

	// compute workgroup grid dimensions
	// (avoid any dimension to go over 10000)
	if(numWorkgroups > 10000 * 10000) {
		unsigned long remainder;
		workgroupCountZ = 1 + ((numWorkgroups - 1) / (10000 * 10000));
		remainder = numWorkgroups / workgroupCountZ;
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
	ts1 = getTimestamp();
	for(z=0; z<workgroupCountZ; z++)
		for(y=0; y<workgroupCountY; y++)
			for(x=0; x<workgroupCountX; x++)
				invocationFunc(x, y, z);
	ts2 = getTimestamp();

	// return time as float in seconds
	return (float)(ts2 - ts1) * timestampPeriod;
}


// record the performance in the list
void processResult(float time, unsigned numWorkgroups, struct FloatVector* performanceList)
{
	if(time >= 0.01f) {
		unsigned long numInstructions = (unsigned long)2000 * numWorkgroups;
		float performance = (float)numInstructions / time;
		vector_push_back(performanceList, performance);
	}
}


// compute number of workgroups
// to reach computation time of about 20ms
unsigned long computeNumWorkgroups(unsigned lastNumWorkgroups, float lastTime)
{
	const float targetTime = 0.02;
	if(lastTime < (targetTime / 10.f)) {
		// multiply numWorkgroups by 10
		return lastNumWorkgroups * 10;
	}
	else {
		// multiply numWorkgroups by ratio
		float ratio = targetTime / lastTime;
		unsigned newNumWorkgroups = (unsigned)(lastNumWorkgroups * ratio);
		return (newNumWorkgroups >= 1) ? newNumWorkgroups : 1;
	}
};


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
static void printFloatSI(float v)
{
	char buffer[7];
	char n[4];
	int thousandNumber;
	int dotPos;

	// make exponent ready to index into SI prefix table
	const char siPrefix[13] = {
		'a', 'f', 'p', 'n', 'u', 'm', ' ', 'K', 'M', 'G', 'T', 'P', 'E'
	};

	// compute significand and exponent
	int exponent = floor(log10(v));
	float divisor = exp((float)(exponent - 2) * log(10));  // this computes exp10(exponent - 2)
	int significand = (int)(v / divisor + 0.5f);  // value is >=100 and <1000, actually it might be
		// a little out this range because of small floating computation imprecisions; +0.5 makes proper
		// rounding and avoids underflow to 99, but might cause overflow to 1000 (or even 1001?)

	// convert significand to numbers
	n[3] = significand % 10;
	significand /= 10;
	n[2] = significand % 10;
	significand /= 10;
	n[1] = significand % 10;
	thousandNumber = significand / 10;  // thousandNumber is 0 or 1; value 1 is present in some extreme cases
	n[0] = thousandNumber;
	exponent += thousandNumber;  // increment exponent if n contains >=1000

	exponent += 18;  // make zero exponent point on the ' ' in siPrefixes
	if(exponent < 0) {
		printf("   0  ");
		return;
	}
	if(exponent >= 39) {
		printf("+inf  ");
		return;
	}

	// create final string
	buffer[6] = 0;
	buffer[5] = siPrefix[exponent / 3];
	buffer[4] = ' ';
	dotPos = (exponent % 3) + 1;
	if(dotPos == 3)
		buffer[0] = ' ';
	else
		buffer[dotPos] = '.';
	buffer[3] = '0' + n[3 - thousandNumber];
	buffer[dotPos==2 ? 1 : 2] = '0' + n[2 - thousandNumber];
	buffer[dotPos==3 ? 1 : 0] = '0' + n[1 - thousandNumber];
	printf(buffer);
}


// print results
void printResult(const char* text, int supported, struct FloatVector* performanceList)
{
	printf(text);
	if(supported) {
		if(vector_empty(performanceList) != 0)
			printf("measurement error\n");
		else {

			// print median
			printFloatSI(vector_get(performanceList, vector_size(performanceList) / 2));
			printf("FLOPS");

			// print dispersion using IQR (Interquartile Range);
			// Q1 is the value in 25% and Q3 in 75%
			printf("  (Q1: ");
			printFloatSI(vector_get(performanceList, vector_size(performanceList) / 4));
			printf("FLOPS, Q3: ");
			printFloatSI(vector_get(performanceList, vector_size(performanceList) * 3 / 4));
			printf("FLOPS)\n");
		}
	}
	else
		printf("not supported\n");
}


int main(int argc, char* argv[])
{
	assert(sizeof(long) == 4 && "Wrong long type size.");

	printf("%s prints the performance of the CPU\n\n", appName);
	printCpuInfo();

	printf("Running tests...\n");
	{
		enum { arraySize = 2 };
		unsigned i;
		unsigned numWorkgroups[arraySize] = { 1,1 };
		struct FloatVector performanceList[arraySize];
		unsigned long startTick = getTimestamp();
		for(i=0; i<arraySize; i++)
			vector_init(&performanceList[i]);
		do {

			// perform tests
			float totalTime;
			float t[arraySize];
			t[0] = performTest(fmaFloatComputation1, numWorkgroups[0]);
			t[1] = performTest(fmaDoubleComputation1, numWorkgroups[1]);
			for(i=0; i<arraySize; i++)
				processResult(t[i], numWorkgroups[i], &performanceList[i]);

			// stop measurements after three seconds
			totalTime = (float)(getTimestamp() - startTick) * timestampPeriod;
			if(totalTime >= 20.f)
				break;

			// compute new numWorkgroups
			for(i=0; i<arraySize; i++)
				numWorkgroups[i] = computeNumWorkgroups(numWorkgroups[i], t[i]);

		} while(1);

		// sort the results
		for(i=0; i<arraySize; i++)
			vector_sort(&performanceList[i]);

		// print results
		printf("Float (float32) performance\n");
		printResult("   non-parallel FMA:    ", 1, &performanceList[0]);
		printf("Double (float64) performance\n");
		printResult("   non-parallel FMA:    ", 1, &performanceList[1]);
		printf("\n");

		// destroy lists
		for(i=0; i<arraySize; i++)
			vector_destroy(&performanceList[i]);
	}

	return 0;
}
