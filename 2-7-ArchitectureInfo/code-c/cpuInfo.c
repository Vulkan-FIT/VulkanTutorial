
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
	assert(sizeof(long) == 4 && "Wrong long type size.");
#if defined(__386__)  // __386__ is defined on Watcom for 32-bit applications
	assert(sizeof(int) == 4 && "Wrong long type size.");
#endif

	printf("Processor info:\n");

# if defined(__X86__)  // __X86__ is defined on Watcom when compiling using Intel instruction set
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

#  if defined(__I86__)  // __I86__ is defined on Watcom for 16-bit applications
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

					// variables used by cpuid routines
				#if defined(__WATCOMC__)  // useless initialization for variables bellow is workaround for Watcom warning W200: 'result' has been referenced but never assigned a value
					long highestBasicFunction = 0;
					long highestExtendedFunction = 0;
					unsigned coreCrystalClockFrequencyHz = 0;
					unsigned baseFrequencyMHz = 0;
					unsigned maximumFrequencyMHz = 0;
					unsigned busFrequencyMHz = 0;
					unsigned featureFlagsEBX = 0;
					unsigned featureFlagsECX = 0;
					unsigned featureFlagsEDX = 0;
					unsigned highestExtendedFeatures = 0;
					unsigned extendedFeatureFlags0EBX = 0;
					unsigned extendedFeatureFlags0ECX = 0;
					unsigned extendedFeatureFlags0EDX = 0;
					unsigned extendedFeatureFlags1EAX = 0;
					unsigned extendedFeatureFlags1EBX = 0;
					unsigned extendedFeatureFlags1ECX = 0;
					unsigned extendedFeatureFlags1EDX = 0;
					unsigned cpuSignature = 0;  // useless initialization here is workaround for Watcom error E1091: aux offset used with register symbol (Watcom probably assigned variable to the cpu register instead on the stack)
					unsigned highestAVX10Subleaf = 0;  // useless initialization here is workaround for Watcom error E1091: aux offset used with register symbol (Watcom probably assigned variable to the cpu register instead on the stack)
					unsigned avx10FeatureInfo = 0;  // useless initialization here is workaround for Watcom error E1091: aux offset used with register symbol (Watcom probably assigned variable to the cpu register instead on the stack)
				#else
					long highestBasicFunction;
					long highestExtendedFunction;
					unsigned coreCrystalClockFrequencyHz;
					unsigned baseFrequencyMHz;
					unsigned maximumFrequencyMHz;
					unsigned busFrequencyMHz;
					unsigned featureFlagsEBX;
					unsigned featureFlagsECX;
					unsigned featureFlagsEDX;
					unsigned highestExtendedFeatures;
					unsigned extendedFeatureFlags0EBX;
					unsigned extendedFeatureFlags0ECX;
					unsigned extendedFeatureFlags0EDX;
					unsigned extendedFeatureFlags1EAX;
					unsigned extendedFeatureFlags1EBX;
					unsigned extendedFeatureFlags1ECX;
					unsigned extendedFeatureFlags1EDX;
					unsigned cpuSignature;
					unsigned highestAVX10Subleaf;
					unsigned avx10FeatureInfo;
				#endif
					char manufacturerIdString[4*3+1];
					char *p = &manufacturerIdString[0];
					char processorName[48+1];
					int frequencyInfoPresent = (highestBasicFunction >= 0x16);
					int hasExtendedFeatures0 = (highestBasicFunction >= 7);
					int hasExtendedFeatures1 = 0;
					manufacturerIdString[4*3] = '\0';
					processorName[0] = '\0';

					// detect cpu by cpuid
					__asm {
						xor eax,eax
						cpuid
						mov highestBasicFunction,eax
						mov eax,p
						mov [eax+0],ebx
						mov [eax+4],edx
						mov [eax+8],ecx
						mov eax,0x80000000
						cpuid
						mov highestExtendedFunction,eax
					};

					// processor name
					if(highestExtendedFunction >= 0x80000004) {
						char *p = &processorName[0];
						processorName[48] = '\0';
						__asm {
							push edi
							mov eax,0x80000002
							mov edi,p
							cpuid
							mov [edi+0],eax
							mov [edi+4],ebx
							mov [edi+8],ecx
							mov [edi+12],edx
							mov eax,0x80000003
							cpuid
							mov [edi+16],eax
							mov [edi+20],ebx
							mov [edi+24],ecx
							mov [edi+28],edx
							mov eax,0x80000004
							cpuid
							mov [edi+32],eax
							mov [edi+36],ebx
							mov [edi+40],ecx
							mov [edi+44],edx
							pop edi
						};
					}

					// frequency info
					if(frequencyInfoPresent)
						__asm {
							mov eax,0x15
							cpuid
							mov coreCrystalClockFrequencyHz,ecx
							mov eax,0x16
							cpuid
							and eax,0xffff
							and ebx,0xffff
							and ecx,0xffff
							mov baseFrequencyMHz,eax
							mov maximumFrequencyMHz,ebx
							mov busFrequencyMHz,ecx
						};

					// CPU signature and feature bits
					if(highestBasicFunction >= 1)
						__asm {
							mov eax,1
							cpuid
							mov cpuSignature,eax
							mov featureFlagsEBX,ebx
							mov featureFlagsECX,ecx
							mov featureFlagsEDX,edx
						};
					else
						cpuSignature = 0;

					// extended features
					if(hasExtendedFeatures0) {
						__asm {
							mov eax,7
							xor ecx,ecx
							cpuid
							mov highestExtendedFeatures,eax
							mov extendedFeatureFlags0EBX,ebx
							mov extendedFeatureFlags0ECX,ecx
							mov extendedFeatureFlags0EDX,edx
						};
						hasExtendedFeatures1 = (highestExtendedFeatures >= 1);
						if(hasExtendedFeatures1) {
							__asm {
								mov eax,7
								mov ecx,1
								cpuid
								mov extendedFeatureFlags1EAX,eax
								mov extendedFeatureFlags1EBX,ebx
								mov extendedFeatureFlags1ECX,ecx
								mov extendedFeatureFlags1EDX,edx
							};
						}
					}

					// AVX10 info
					if(highestBasicFunction >= 0x24) {
						__asm {
							mov eax,0x24
							xor ecx,ecx
							cpuid
							mov highestAVX10Subleaf,eax
							mov avx10FeatureInfo,ebx
						};
					}
					else {
						highestAVX10Subleaf = 0;
						avx10FeatureInfo = 0;
					}

					// print processor name
					printf("   Name:  ");
					if(processorName[0] == '\0')
						printf("< unknown >");
					else
						printf(processorName);
					printf("\n");

					// print manufacturer string
					printf("   Manufacturer string:  ");
					printf(manufacturerIdString);
					printf("\n");

					// print frequency info
					if(frequencyInfoPresent) {
						printf("   Base frequency:       ");
						if(baseFrequencyMHz == 0)  printf("not enumerated\n");
						else  printf("%u MHz\n", baseFrequencyMHz);
						printf("   Maximum frequency:    ");
						if(maximumFrequencyMHz == 0)  printf("not enumerated\n");
						else  printf("%u MHz\n", maximumFrequencyMHz);
						printf("   Bus frequency:        ");
						if(busFrequencyMHz == 0)  printf("not enumerated\n");
						else  printf("%u MHz\n", busFrequencyMHz);
						printf("   Crystal frequency:    ");
						if(coreCrystalClockFrequencyHz == 0)  printf("not enumerated\n");
						else  printf("%u MHz\n", coreCrystalClockFrequencyHz);
					}

					// print processor signature and feature flags
					if(cpuSignature != 0) {

						unsigned family = (cpuSignature & 0x0F00) >> 8;
						unsigned model = (cpuSignature & 0xF0) >> 4;
						unsigned stepping = cpuSignature & 0x0F;
						if(family == 6 || family == 15)
							model = ((cpuSignature & 0x000F0000) >> 12) | model;
						if(family == 15)
							family += (cpuSignature & 0x0FF00000) >> 20;

						printf("   Processor signature:  0x%x\n", cpuSignature);
						printf("   Family:    ");
						if(family < 10)  printf("%u", family);
						else if(family<0x10)  printf("0x0%x", family);
						else  printf("0x%x", family);
						printf("\n"
						       "   Model:     %u\n"
						       "   Stepping:  %u\n", model, stepping);

						printf("   x87 FPU:  %s\n", (featureFlagsEDX & 0x00000001) ? "yes" : "no");
						printf("   MMX:      %s\n", (featureFlagsEDX & 0x00800000) ? "yes" : "no");
						printf("   SSE:      %s\n", (featureFlagsEDX & 0x02000000) ? "yes" : "no");
						printf("   SSE2:     %s\n", (featureFlagsEDX & 0x04000000) ? "yes" : "no");
						printf("   SSE3:     %s\n", (featureFlagsECX & 0x00000001) ? "yes" : "no");
						printf("   SSSE3:    %s\n", (featureFlagsECX & 0x00000200) ? "yes" : "no");
						printf("   FMA3:     %s\n", (featureFlagsECX & 0x00001000) ? "yes" : "no");
						printf("   SSE4.1:   %s\n", (featureFlagsECX & 0x00080000) ? "yes" : "no");
						printf("   SSE4.2:   %s\n", (featureFlagsECX & 0x00100000) ? "yes" : "no");
						printf("   AVX:      %s\n", (featureFlagsECX & 0x10000000) ? "yes" : "no");

					}
					else
						printf("   Processor signature:  n/a\n"
						       "   Family:    n/a\n"
						       "   Model:     n/a\n"
						       "   Stepping:  n/a\n"
						       "   x87 FPU:  n/a\n"
						       "   MMX:      n/a\n"
						       "   SSE:      n/a\n"
						       "   SSE2:     n/a\n"
						       "   SSE3:     n/a\n"
						       "   SSSE3:    n/a\n"
						       "   FMA3:     n/a\n"
						       "   SSE4.1:   n/a\n"
						       "   SSE4.2:   n/a\n"
						       "   AVX:      n/a\n");

					// print extended features 0
					if(hasExtendedFeatures0) {
						printf("   AVX2:     %s\n", (extendedFeatureFlags0EBX & 0x00000020) ? "yes" : "no");
						printf("   ADX:      %s\n", (extendedFeatureFlags0EBX & 0x00080000) ? "yes" : "no");
						printf("   AVX512-f:   %s\n", (extendedFeatureFlags0EBX & 0x00010000) ? "yes" : "no");
						printf("   AVX512-dq:  %s\n", (extendedFeatureFlags0EBX & 0x00010000) ? "yes" : "no");
						printf("   AVX512-4fmaps:  %s\n", (extendedFeatureFlags0EDX & 0x00000008) ? "yes" : "no");
						printf("   AVX512-fp16:  %s\n", (extendedFeatureFlags0EDX & 0x00800000) ? "yes" : "no");
					}
					else
						printf("   AVX2:     n/a\n"
						       "   ADX:      n/a\n"
						       "   AVX512-f:   n/a\n"
						       "   AVX512-dq:  n/a\n"
						       "   AVX512-4fmaps:  n/a\n"
						       "   AVX512-fp16:  n/a\n");

					// print extended features 1
					if(hasExtendedFeatures1) {
						printf("   APX:      %s\n", (extendedFeatureFlags1EDX & 0x00200000) ? "yes" : "no");
						printf("   AVX10:    %s\n", (extendedFeatureFlags1EDX & 0x00080000) ? "yes" : "no");
						printf("   AMX-fp16:  %s\n", (extendedFeatureFlags1EAX & 0x00200000) ? "yes" : "no");
					}
					else
						printf("   APX:      n/a\n"
						       "   AVX10:    n/a\n"
						       "   AMX-fp16:  n/a\n");

					// print AVX10 info
					if(avx10FeatureInfo != 0)
						printf("   AVX10 version:  %u\n", avx10FeatureInfo & 0xff);
					else
						printf("   AVX10 version:  n/a\n");
				}
			}
#  endif
		}
	}
# else
	printf("   Unknown\n");
# endif
#endif
}
