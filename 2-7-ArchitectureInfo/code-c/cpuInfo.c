
// Sources of information:
// https://cosmodoc.org/topics/processor-detection/
// https://www.rcollins.org/ddj/Sep96/Sep96.html
// https://reverseengineering.stackexchange.com/questions/19394/how-did-this-80286-detection-code-work
// https://wiki.osdev.org/User:ChosenOreo/CPU_Detection

#include <assert.h>
#include <stdio.h>


void printCpuInfo()
{
	// variables
#if defined(__DOS__)
# if defined(__X86__)
#  if defined(__WATCOMC__)
	unsigned short result = 0;  // useless initialization is workaround for Watcom warning W200: 'result' has been referenced but never assigned a value
#  else
	unsigned short result;
#  endif
# endif

	assert(sizeof(short) == 2 && "Wrong short type size.");

	printf("Processor info:\n");

# if defined(__X86__)
	// detect 286+ processor
	// (286+ processors allows writing flag bits 12..15
	// while prior processors keep these bits on 1)
	__asm {
		pushf
		pop   ax
		mov   cx,ax
		and   ax,0fffh
		push  ax
		popf
		pushf
		pop   ax
		push  cx
		popf
		mov   result,ax
	};

	// handle pre-286 processors
	if(result & 0xf000 == 0xf000) {

		// detect 80186/80188
		// (shr by 33 is equal to shr by 1 on 80186/80188)
		__asm {
			mov   ax,0ffh
			mov   cl,021h
			shr   al,cl
			mov   result,ax
		};
		if(result != 0)
			printf("   80186/80188 cpu detected\n");
		else {

			// detect NEC V20/V30
			// (NEC does not have a bug that drops first of two instruction prefixes
			// of rep lods; interrupt comes from regular 55ms system timer)
			__asm {
				sti
				push  si
				mov   si,0
				mov   cx,0ffffh
				rep lods [BYTE ptr es:si]
				pop   si
				mov   result,cx
			};
			if(result == 0)
				printf("   NEC V20/V30 (Intel 8086/8088 drop-in replacement) cpu detected\n");
			else
				// only remaining cpu is 8086/8088
				printf("   8086/8088 cpu detected\n");
		}
	}
	else {

		// detect 286 processor
		// (286 disallows modification of flag bits 12..14)
		__asm {
			pushf
			mov   ax,7000h
			push  ax
			popf
			pushf
			pop   ax
			popf
			mov   result,ax
		};
		if((result & 0x7000) == 0)
			printf("   80286 cpu detected\n");
		else {

#  if defined(__I86__)
			// 16-bit application
			printf("   80386 or newer cpu detected\n");
			printf("   (use 32-bit version of this application for further cpu detection)\n");
#  else
			// 32-bit application can use 32-bit instructions
			// to detect 386 processor
			__asm {
				pushfd
				pop   eax
				mov   ecx,eax
				xor   eax,40000h  // flip AC bit
				push  eax
				popfd
				pushfd
				pop   eax
				xor   eax,ecx
				push  ecx
				shr   eax,16
				popfd
				mov   result,ax
			};
			if((result & 0x4) == 0)
				printf("   80386 cpu detected\n");
			else {

				// detect cpuid support on 486 or newer processor
				__asm {
					pushfd
					pop   eax
					mov   ecx,eax
					xor   eax,200000h  // flip ID bit
					push  eax
					popfd
					pushfd
					pop   eax
					xor   eax,ecx
					push  ecx
					shr   eax,16
					popfd
					mov   result,ax
				};
				if((result & 0x20) == 0)
					printf("   80486 cpu without cpuid instruction detected\n");
				else {

					// detect cpu by cpuid
					__asm {
					};
					printf("   80486 or newer cpu with cpuid instruction detected\n");

				}
			}
#  endif
		}
	}
#else
	printf("   Unknown\n");
#endif
#endif
}
