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
const double value01 = 0.1;
const double value0165 = 0.165;
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
void straightComputation1kOpX87(
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

		#include "x87-asm-1k-op.h"

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
void straightComputation20kOpX87(
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

		#include "x87-asm-1k-op.h"

	};

	// following includes are split into separate assember blocks
	// because of internal limits of Open Watcom 1.9 compiler;
	// moreover, it was necessary to use -zm (put functions in separate segments)
	// to overcome additional limits when targeting 16-bit applications
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};

	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {
		#include "x87-asm-1k-op.h"
	};
	__asm {

		#include "x87-asm-1k-op.h"

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


// x87 assembler, 500 floating point instructions,
// three computations in parallel, number of used registers: 7
void parallelComputation500OpX87(
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
		fild  x  // x3
		fld   initialValueMultiplier  // place of x2
		fmul  st(1),st
		fld   st(1)  // x1
		fild  z  // z
		fmul  st,st(2)
		fild  y  // y3
		fmul  st,st(3)
		fld   st(1)  // y1
		fld   st(1)  // y2
		fld   st(6)
		fstp  st(6)  // update x2
		// st(0) - y2
		// st(1) - y1
		// st(2) - y3
		// st(3) - z
		// st(4) - x1
		// st(5) - x2
		// st(6) - x3
		fld   value01
		fadd  st(1),st  // y2 += 0.1
		faddp st(7),st  // x3 += 0.1
		fld   value0165
		fadd  st(3),st  // y3 += 0.165
		faddp st(6),st  // x2 += 0.165

		// make the computation of 250 x fmul and 250 x fadd
		// registers:
		// st(0) - y2
		// st(1) - y1
		// st(2) - y3
		// st(3) - z
		// st(4) - x1
		// st(5) - x2
		// st(6) - x3
		fmul  st(5),st  // x2 *= y2
		fxch  st(1)  // stack: y1,y2,y3,z,x1,x2,x3
		#include "x87-asm-498-op-parallel.h"
		// stack: y1,y2,y3,z,x1,x2,x3;
		// last missing fadd operation is not added here,
		// because we have two of fadd bellow

		// clean up registers
		fstp  st(0)  // y1
		fstp  st(0)  // y2
		fstp  st(0)  // y3
		fstp  st(0)  // z
		faddp st(1),st  // pop x1
		faddp st(1),st  // pop x2

		// store the result
		fstp  dword ptr x87computationResult

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


// x87 assembler, 10'000 floating point instructions,
// three computations in parallel, number of used registers: 7
void parallelComputation10kOpX87(
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
		fild  x  // x3
		fld   initialValueMultiplier  // place of x2
		fmul  st(1),st
		fld   st(1)  // x1
		fild  z  // z
		fmul  st,st(2)
		fild  y  // y3
		fmul  st,st(3)
		fld   st(1)  // y1
		fld   st(1)  // y2
		fld   st(6)
		fstp  st(6)  // update x2
		// st(0) - y2
		// st(1) - y1
		// st(2) - y3
		// st(3) - z
		// st(4) - x1
		// st(5) - x2
		// st(6) - x3
		fld   value01
		fadd  st(1),st  // y2 += 0.1
		faddp st(7),st  // x3 += 0.1
		fld   value0165
		fadd  st(3),st  // y3 += 0.165
		faddp st(6),st  // x2 += 0.165

		// make the computation of 250 x fmul and 250 x fadd
		// registers:
		// st(0) - y2
		// st(1) - y1
		// st(2) - y3
		// st(3) - z
		// st(4) - x1
		// st(5) - x2
		// st(6) - x3
		fmul  st(5),st  // x2 *= y2
		fxch  st(1)  // stack: y1,y2,y3,z,x1,x2,x3
		#include "x87-asm-498-op-parallel.h"
	};

	// following includes are split into separate assember blocks
	// because of internal limits of Open Watcom 1.9 compiler;
	// moreover, it was necessary to use -zm (put functions in separate segments)
	// to overcome additional limits when targeting 16-bit applications
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};

	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"
	};
	__asm {
		#include "x87-asm-498-op-parallel.h"

		// x87-asm-498-op-parallel.h contains 498 operations,
		// so, included 20x, it makes 20x498 = 9'960 operations;
		// following code blocks are making it closer to 10k;
		// each block contains 3 x fmul + 3 x fadd
		fmul  st(4),st  // x1 *= y1
		fxch  st(3)  // stack: z,y2,y3,y1,...
		fadd  st(5),st  // x2 += z
		fxch  st(2)  // stack: y3,y2,z,y1,...
		fmul  st(6),st  // x3 *= y3
		fxch  st(2)  // stack: z,y2,y3,y1,...
		fadd  st(4),st  // x1 += z
		fxch  st(1)  // stack: y2,z,y3,y1,...
		fmul  st(5),st  // x2 *= y2
		fxch  st(1)  // stack: z,y2,y3,y1,...
		fadd  st(6),st  // x3 += z
		fxch  st(3)  // stack: y1,y2,y3,z,...

		fmul  st(4),st  // x1 *= y1
		fxch  st(3)  // stack: z,y2,y3,y1,...
		fadd  st(5),st  // x2 += z
		fxch  st(2)  // stack: y3,y2,z,y1,...
		fmul  st(6),st  // x3 *= y3
		fxch  st(2)  // stack: z,y2,y3,y1,...
		fadd  st(4),st  // x1 += z
		fxch  st(1)  // stack: y2,z,y3,y1,...
		fmul  st(5),st  // x2 *= y2
		fxch  st(1)  // stack: z,y2,y3,y1,...
		fadd  st(6),st  // x3 += z
		fxch  st(3)  // stack: y1,y2,y3,z,...

		fmul  st(4),st  // x1 *= y1
		fxch  st(3)  // stack: z,y2,y3,y1,...
		fadd  st(5),st  // x2 += z
		fxch  st(2)  // stack: y3,y2,z,y1,...
		fmul  st(6),st  // x3 *= y3
		fxch  st(2)  // stack: z,y2,y3,y1,...
		fadd  st(4),st  // x1 += z
		fxch  st(1)  // stack: y2,z,y3,y1,...
		fmul  st(5),st  // x2 *= y2
		fxch  st(1)  // stack: z,y2,y3,y1,...
		fadd  st(6),st  // x3 += z
		fxch  st(3)  // stack: y1,y2,y3,z,...

		fmul  st(4),st  // x1 *= y1
		fxch  st(3)  // stack: z,y2,y3,y1,...
		fadd  st(5),st  // x2 += z
		fxch  st(2)  // stack: y3,y2,z,y1,...
		fmul  st(6),st  // x3 *= y3
		fxch  st(2)  // stack: z,y2,y3,y1,...
		fadd  st(4),st  // x1 += z
		fxch  st(1)  // stack: y2,z,y3,y1,...
		fmul  st(5),st  // x2 *= y2
		fxch  st(1)  // stack: z,y2,y3,y1,...
		fadd  st(6),st  // x3 += z
		fxch  st(3)  // stack: y1,y2,y3,z,...

		fmul  st(4),st  // x1 *= y1
		fxch  st(3)  // stack: z,y2,y3,y1,...
		fadd  st(5),st  // x2 += z
		fxch  st(2)  // stack: y3,y2,z,y1,...
		fmul  st(6),st  // x3 *= y3
		fxch  st(2)  // stack: z,y2,y3,y1,...
		fadd  st(4),st  // x1 += z
		fxch  st(1)  // stack: y2,z,y3,y1,...
		fmul  st(5),st  // x2 *= y2
		fxch  st(1)  // stack: z,y2,y3,y1,...
		fadd  st(6),st  // x3 += z
		fxch  st(3)  // stack: y1,y2,y3,z,...

		fmul  st(4),st  // x1 *= y1
		fxch  st(3)  // stack: z,y2,y3,y1,...
		fadd  st(5),st  // x2 += z
		fxch  st(2)  // stack: y3,y2,z,y1,...
		fmul  st(6),st  // x3 *= y3
		fxch  st(2)  // stack: z,y2,y3,y1,...
		fadd  st(4),st  // x1 += z
		fxch  st(1)  // stack: y2,z,y3,y1,...
		fmul  st(5),st  // x2 *= y2
		fxch  st(1)  // stack: z,y2,y3,y1,...
		fadd  st(6),st  // x3 += z
		fxch  st(3)  // stack: y1,y2,y3,z,...

		// stack: y1,y2,y3,z,...
		faddp st(5),st  // pop y1

		// clean up registers
		fstp  st(0)  // y2
		fstp  st(0)  // y3
		fstp  st(0)  // z
		faddp st(1),st  // pop x1
		faddp st(1),st  // pop x2

		// store the result
		fstp  dword ptr x87computationResult

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
