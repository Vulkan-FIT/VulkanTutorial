#include <stdio.h>


void floatComputationC(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
	// no parallelism, number of used registers: 3
#define F_MULADD_C_10 \
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

#define F_MULADD_C_100 \
	F_MULADD_C_10; \
	F_MULADD_C_10; \
	F_MULADD_C_10; \
	F_MULADD_C_10; \
	F_MULADD_C_10; \
	F_MULADD_C_10; \
	F_MULADD_C_10; \
	F_MULADD_C_10; \
	F_MULADD_C_10; \
	F_MULADD_C_10

	// initial values for the computation
	// (make each variable in the range 0.0 to 0.16383)
	float x = (float)(globalInvocationIdX & 0x3fff) * 0.00001;
	float y = (float)(globalInvocationIdY & 0x3fff) * 0.00001;
	float z = (float)(globalInvocationIdZ & 0x3fff) * 0.00001;

	F_MULADD_C_100;
	F_MULADD_C_100;
	F_MULADD_C_100;
	F_MULADD_C_100;
	F_MULADD_C_100;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x == 10.f) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*(float*)(globalInvocationIdZ) = y;
	}

}


void doubleComputationC(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
	// no parallelism, number of used registers: 3
#define D_MULADD_C_10 \
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

#define D_MULADD_C_100 \
	D_MULADD_C_10; \
	D_MULADD_C_10; \
	D_MULADD_C_10; \
	D_MULADD_C_10; \
	D_MULADD_C_10; \
	D_MULADD_C_10; \
	D_MULADD_C_10; \
	D_MULADD_C_10; \
	D_MULADD_C_10; \
	D_MULADD_C_10

	// initial values for the computation
	// (make each variable in the range 0.0 to 0.16383)
	double x = (double)(globalInvocationIdX & 0x3fff) * 0.00001;
	double y = (double)(globalInvocationIdY & 0x3fff) * 0.00001;
	double z = (double)(globalInvocationIdZ & 0x3fff) * 0.00001;

	D_MULADD_C_100;
	D_MULADD_C_100;
	D_MULADD_C_100;
	D_MULADD_C_100;
	D_MULADD_C_100;

	// condition that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and to optimize the final shader code by their removal)
	if(x == 10.) {
		// write to artificially generated address
		// (the write will never happen in reality)
		*(double*)(globalInvocationIdZ) = y;
	}

}


static short savedX87ControlWord = 0;
static short currentX87ControlWord = 0;


void setFloatPrecisionX87()
{
	__asm {
		fstcw savedX87ControlWord
		mov   ax,savedX87ControlWord
		and   ax,0fcffh  // clear bits 8 and 9 to set single precision
		mov   currentX87ControlWord,ax
		fldcw currentX87ControlWord
	}
}


void setDoublePrecisionX87()
{
	__asm {
		fstcw savedX87ControlWord
		mov   ax,savedX87ControlWord
		and   ax,0feffh  // clear bit 8 to set double precision
		or    ax,00200h  // set bit 9 to set double precision
		mov   currentX87ControlWord,ax
		fldcw currentX87ControlWord
	}
}


void restorePrecisionX87()
{
#if defined(__WATCOMC__)
	short tmp = 0;  // kill warning on Open Watcom 1.9
#else
	short tmp;
#endif
	__asm {
		fstcw tmp
		fldcw savedX87ControlWord
	}
	if(tmp != currentX87ControlWord)
		printf("X87 control word was modified during measurement. Results might be spoiled.\n");
}


const double initialValueMultiplier = 0.00001;
union {
	float asFloat;
	unsigned long asLong;
	unsigned short asShort[2];
} x87computationResult;
union {
	float asFloat;
	unsigned long asLong;
	unsigned short asShort[2];
} neverExpectedResult = { 8.f };


// x87 assembler, 1000 floating point instructions,
// straight computation without parallelism, number of used registers: 3
void straightComputation1kX87(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{

	// initial values for the computation
	// (make each variable in the range 0..16383)
	unsigned x = globalInvocationIdX & 0x3fff;
	unsigned y = globalInvocationIdY & 0x3fff;
	unsigned z = globalInvocationIdZ & 0x3fff;

	__asm {

		// initial values for the computation
		// (make each variable in the range 0.0 to 0.16383)
		fild   z
		fild   y
		fild   x
		fld   initialValueMultiplier
		fmul  st(3),st
		fmul  st(2),st
		fmulp st(1),st

		 #include "x87-asm-1k.h"

		// store the result and clean up registers
		fstp  dword ptr x87computationResult
		fstp  st(0)
		fstp  st(0)

	};

	// make comparison that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and discard all our computations)
	// (it is enough to compare second short on little endian architectures because
	// x87computationResult is expected to be between 0..1; the second shortand of such a number
	// is always smaller than the second short of 8.f stored in neverExpectedResult)
	if(x87computationResult.asShort[1] >= neverExpectedResult.asShort[1])
		printf("Wrong result. Negative value or positive value bigger than 8 was detected. Measurements might be garbage.\n");
}


// x87 assembler, 20'000 floating point instructions,
// straight computation without parallelism, number of used registers: 3
void straightComputation20kX87(
	unsigned globalInvocationIdX, unsigned globalInvocationIdY, unsigned globalInvocationIdZ)
{
	// initial values for the computation
	// (make each variable in the range 0..16383)
	unsigned x = globalInvocationIdX & 0x3fff;
	unsigned y = globalInvocationIdY & 0x3fff;
	unsigned z = globalInvocationIdZ & 0x3fff;

	__asm {

		// initial values for the computation
		// (make each variable in the range 0.0 to 0.16383)
		fild   z
		fild   y
		fild   x
		fld   initialValueMultiplier
		fmul  st(3),st
		fmul  st(2),st
		fmulp st(1),st

		#include "x87-asm-1k.h"

	};

	// following includes are split into separate assember blocks
	// because of internal limits of Open Watcom 1.9 compiler;
	// moreover, it was necessary to use -zm (put functions in separate segments)
	// to overcome additional limits when targeting 16-bit applications
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};

	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {
		#include "x87-asm-1k.h"
	};
	__asm {

		#include "x87-asm-1k.h"

		// store the result and clean up registers
		fstp  dword ptr x87computationResult
		fstp  st(0)
		fstp  st(0)

	};

	// make comparison that will never be true in reality
	// (this avoids optimizer to consider the results of previous computations as unused
	// and discard all our computations)
	// (it is enough to compare second short on little endian architectures because
	// x87computationResult is expected to be between 0..1; the second shortand of such a number
	// is always smaller than the second short of 8.f stored in neverExpectedResult)
	if(x87computationResult.asShort[1] >= neverExpectedResult.asShort[1])
		printf("Wrong result. Negative value or positive value bigger than 8 was detected. Measurements might be garbage.\n");
}
