#include <assert.h>
#include <math.h>
#include <string.h>  // memcmp()
#include <stdio.h>
#include <stdlib.h>  // exit()
#include <time.h>
#if defined(__DOS__)
# include <bios.h>
# include <dos.h>  // dostime_t, _dos_gettime()
#endif
#include "FloatVector.h"
#include "PIT0Timer.h"

static const char appName[] = "2-7-ArchtectureInfo-c";

void printCpuInfo();
void fmaFloatComputation1(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ);
void fmaDoubleComputation1(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ);


float performTest(void (*invocationFunc)(unsigned,unsigned,unsigned), unsigned long numWorkgroups)
{
	unsigned workgroupCountX;
	unsigned workgroupCountY;
	unsigned workgroupCountZ;
	__int64 ts1, ts2;
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
	ts1 = readPIT0Ticks();
	for(z=0; z<workgroupCountZ; z++)
		for(y=0; y<workgroupCountY; y++)
			for(x=0; x<workgroupCountX; x++)
				invocationFunc(x, y, z);
	ts2 = readPIT0Ticks();

	// return time as float in seconds
	return ((float)(ts2 - ts1)) * getPIT0TickTime();
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

	printf("\n%s prints the performance of the CPU\n\n", appName);
	printCpuInfo();

	// init Intel 8253 Programmable Interval Timer (PIT)
	// (channel 0 controlling interrupt 8 needs to be switched
	// to mode 2 (Rate Generator) while default mode is 3 (Square Wave Generator))
	initPIT0Timer();

	printf("\nMeasuring time precision...\n");
	{
		// variables
		__int64 ticks1,ticks2,ticks3,ticks4;
		struct dostime_t tStart,t1,t2;
		clock_t clockStart,clockFinish,clock1,clock2;
		long todStart,tod1,tod2;
		unsigned long tNumChanges = 0;
		unsigned long todNumChanges = 0;
		unsigned long clockNumChanges = 0;
		unsigned long deltaTime;
		long i,n;

		// wait for the time update
		_dos_gettime(&t1);
		do {
			_dos_gettime(&t2);
		} while(memcmp(&t1, &t2, sizeof(struct dostime_t)) == 0);

		// init start time variables
		tStart = t2;
		t1 = t2;
		clockStart = clock();
		clockFinish = clockStart + (CLOCKS_PER_SEC / 2);  // make finish time half of second ahead
		clock1 = clockStart;
		_bios_timeofday(_TIME_GETCLOCK, &todStart);
		tod1 = todStart;
		ticks1 = readPIT0Ticks();

		// measure for half of second
		do {
			_dos_gettime(&t2);  // make _dos_gettime() before getTimestamp(), just because we expect that _dos_gettime() is always driven by interrupt 8 (each ~55ms)
			clock2 = clock();
			_bios_timeofday(_TIME_GETCLOCK, &tod2);
			if(clock2 != clock1) {
				clockNumChanges++;
				clock1 = clock2;
			}
			if(tod2 != tod1) {
				todNumChanges++;
				tod1 = tod2;
			}
			if(memcmp(&t2, &t1, sizeof(struct dostime_t)) != 0) {
				tNumChanges++;
				t1 = t2;

				// stop the measurement after certain time
				// (ts2 and tsFinish are used because unsigned long is
				// simpler to compare than dostime_t structs t2 and tStart)
				if(clock2 >= clockFinish)
					break;
			}
		} while(1);
		ticks2 = readPIT0Ticks();

		// print results
		deltaTime =
			(t2.hour >= tStart.hour)
				? (t2.hour - tStart.hour) * 3600 * 100
				: 86400 * 100 + ((int)t2.hour - tStart.hour) * 3600 * 100;
		deltaTime += ((int)t2.minute - tStart.minute) * 60 * 100;
		deltaTime += ((int)t2.second - tStart.second) * 100;
		deltaTime += (int)t2.hsecond - tStart.hsecond;
		printf("   Measurement time:  %lu ms\n", deltaTime * 10);
		printf("   _dos_gettime() update time: %.2f ms, update frequency: %.2f Hz\n"
		       "      (the time was updated %lu times while indicating %lu ms time difference)\n",
		       ((float)deltaTime * 10.f) / (float)tNumChanges,
		       (float)tNumChanges / ((float)deltaTime * 0.01f),
		       tNumChanges, deltaTime * 10);
		printf("   clock() update time: %.2f ms, update frequency: %.2f Hz\n"
		       "      (the time was updated %lu times while indicating %u ms time difference)\n",
		       ((float)deltaTime * 10.f) / (float)clockNumChanges,
		       (float)clockNumChanges / ((float)deltaTime * 0.01f),
		       clockNumChanges, (unsigned)((float)(clock2 - clockStart) / (float)CLOCKS_PER_SEC * 1000.f + 0.5f));
		printf("   _bios_timeofday() update time: %.2f ms, update frequency: %.2f Hz\n"
		       "      (the time was updated %lu times while indicating %u ms time difference)\n",
		       ((float)deltaTime * 10.f) / (float)todNumChanges,
		       (float)todNumChanges / ((float)deltaTime * 0.01f),
		       todNumChanges, (unsigned)(((float)todNumChanges) * 65536.f / 1193181.6666f * 1000.f + 0.5f));  // 1 193 182 / 65 536 = ~18.2

		// PIT0 results
		printf("   PIT0 reports %ld ticks in %lu ms.\n", (long)(ticks2 - ticks1), deltaTime * 10);
		printf("   PIT0 frequency: %f Hz, tick time: %f us\n", getPIT0Frequency(), getPIT0TickTime() * (float)1e6);
		for(n=100; n<1000000000; n*=10) {
			ticks1 = readPIT0Ticks();
			for(i=1; i<n; i++)
				readPIT0Ticks();
			ticks2 = readPIT0Ticks();
			if(ticks2 < ticks1) {
				printf("Time going backward!\n");
				exit(-100);
			}
			ticks3 = ticks2 - ticks1;
			if((float)ticks3 * getPIT0TickTime() > 0.05f)
				break;
		}
		printf("   PIT0 read time: ");
		printFloatSI((float)ticks3 / n * getPIT0TickTime());
		printf("s (%ld reads in %ld ticks)\n", n, (long)ticks3);
		ticks1 = readPIT0Ticks();
		ticks4 = 0;
		for(i=0; i<n; i++) {
			ticks2 = readPIT0Ticks();
			if(ticks2 < ticks1) {
				printf("Time going backward!\n");
				exit(-100);
			}
			ticks3 = ticks2 - ticks1;
			if(ticks4 < ticks3)
				ticks4 = ticks3;
			ticks1 = ticks2;
		}
		printf("   PIT0 biggest tick difference: %ld (", (long)ticks4);
		printFloatSI((float)(long)ticks4 * getPIT0TickTime());
		printf("s)\n");
	}

	printf("\nRunning tests...\n");
	{
		enum { arraySize = 2 };
		unsigned i;
		unsigned numWorkgroups[arraySize] = { 1,1 };
		struct FloatVector performanceList[arraySize];
		__int64 startTick = readPIT0Ticks();
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
			totalTime = ((float)(readPIT0Ticks() - startTick)) * getPIT0TickTime();
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

	// restore Intel 8253 Programmable Interval Timer (PIT)
	// channel 0 mode back to 3
	restoreDefaultPIT0Timer();
	return 0;
}
