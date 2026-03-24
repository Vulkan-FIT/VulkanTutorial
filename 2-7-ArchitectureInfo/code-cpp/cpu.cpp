#include <cstdint>
#include <iostream>
#if _M_X64
#include <intrin.h>
#endif

using namespace std;


void printCpuInfo()
{
	bool cpuidSupported = true;
	if(cpuidSupported) {


		// highest cpuid function
		uint32_t highestBasicFunction;
		uint32_t highestExtendedFunction;
		char manufacturerIdString[4*3+1];
		manufacturerIdString[4*3] = '\0';
		char *p = &manufacturerIdString[0];

	# if __GNUC__

	#  if __SIZEOF_POINTER__ == 8

		asm volatile(  // 64-bit gcc assembler code
			"xor %%eax,%%eax\n"
			"mov %2,%%rdi\n"
			"cpuid\n"
			"mov %%eax,%0\n"
			"mov %%ebx,0x0(%%rdi)\n"
			"mov %%edx,0x4(%%rdi)\n"
			"mov %%ecx,0x8(%%rdi)\n"
			"mov $0x80000000,%%eax\n"
			"cpuid\n"
			"mov %%eax,%1\n"

				: "=rm"(highestBasicFunction), "=rm"(highestExtendedFunction)
				: "rm"(p)
				: "%rax", "%rbx", "%rcx", "%rdx", "%rdi");

	#  else

		asm volatile(  // 32-bit gcc assembler code
			"xor %%eax,%%eax\n"
			"mov %2,%%edi\n"
			"cpuid\n"
			"mov %%eax,%0\n"
			"mov %%ebx,0x0(%%edi)\n"
			"mov %%edx,0x4(%%edi)\n"
			"mov %%ecx,0x8(%%edi)\n"
			"mov $0x80000000,%%eax\n"
			"cpuid\n"
			"mov %%eax,%1\n"

				: "=rm"(highestBasicFunction), "=rm"(highestExtendedFunction)
				: "rm"(p)
				: "%eax", "%ebx", "%ecx", "%edx", "%edi");

	#  endif

	# else

	#  if _M_X64  // 64-bit assembler of Intel, Visual C++,...

		int info[4];
		__cpuid(info, 0);
		highestBasicFunction = info[0];
		*reinterpret_cast<int*>(p+0) = info[1];
		*reinterpret_cast<int*>(p+4) = info[3];
		*reinterpret_cast<int*>(p+8) = info[2];
		__cpuid(info, 0x80000000);
		highestExtendedFunction = info[0];

	#  else

		__asm {  // 32-bit assembler of Intel, Visual C++,...
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

	#  endif
	# endif


		// processor name
		char processorName[48+1];
		processorName[0] = '\0';
		if(highestExtendedFunction >= 0x80000004) {

			processorName[48] = '\0';
			char *p = &processorName[0];

		# if __GNUC__

		#  if __SIZEOF_POINTER__ == 8

			asm volatile(  // 64-bit gcc assembler code
				"mov $0x80000002,%%eax\n"
				"mov %0,%%rdi\n"
				"cpuid\n"
				"mov %%eax,0x00(%%rdi)\n"
				"mov %%ebx,0x04(%%rdi)\n"
				"mov %%ecx,0x08(%%rdi)\n"
				"mov %%edx,0x0c(%%rdi)\n"
				"mov $0x80000003,%%eax\n"
				"cpuid\n"
				"mov %%eax,0x10(%%rdi)\n"
				"mov %%ebx,0x14(%%rdi)\n"
				"mov %%ecx,0x18(%%rdi)\n"
				"mov %%edx,0x1c(%%rdi)\n"
				"mov $0x80000004,%%eax\n"
				"cpuid\n"
				"mov %%eax,0x20(%%rdi)\n"
				"mov %%ebx,0x24(%%rdi)\n"
				"mov %%ecx,0x28(%%rdi)\n"
				"mov %%edx,0x2c(%%rdi)\n"

					:
					: "rm"(p)
					: "%rax", "%rbx", "%rcx", "%rdx", "%rdi");

		#  else

			asm volatile(  // 32-bit gcc assembler code
				"mov $0x80000002,%%eax\n"
				"mov %0,%%edi\n"
				"cpuid\n"
				"mov %%eax,0x0(%%edi)\n"
				"mov %%ebx,0x4(%%edi)\n"
				"mov %%ecx,0x8(%%edi)\n"
				"mov %%edx,0xc(%%edi)\n"
				"mov $0x80000003,%%eax\n"
				"cpuid\n"
				"mov %%eax,0x10(%%edi)\n"
				"mov %%ebx,0x14(%%edi)\n"
				"mov %%ecx,0x18(%%edi)\n"
				"mov %%edx,0x1c(%%edi)\n"
				"mov $0x80000004,%%eax\n"
				"cpuid\n"
				"mov %%eax,0x20(%%edi)\n"
				"mov %%ebx,0x24(%%edi)\n"
				"mov %%ecx,0x28(%%edi)\n"
				"mov %%edx,0x2c(%%edi)\n"

					:
					: "rm"(p)
					: "%eax", "%ebx", "%ecx", "%edx", "%edi");

		#  endif

		# else

		#  if _M_X64  // 64-bit assembler of Intel, Visual C++,...

			int info[4];
			__cpuid(info, 0x80000002);
			*reinterpret_cast<int*>(p+0) = info[0];
			*reinterpret_cast<int*>(p+4) = info[1];
			*reinterpret_cast<int*>(p+8) = info[2];
			*reinterpret_cast<int*>(p+12) = info[3];
			__cpuid(info, 0x80000003);
			*reinterpret_cast<int*>(p+16) = info[0];
			*reinterpret_cast<int*>(p+20) = info[1];
			*reinterpret_cast<int*>(p+24) = info[2];
			*reinterpret_cast<int*>(p+28) = info[3];
			__cpuid(info, 0x80000004);
			*reinterpret_cast<int*>(p+32) = info[0];
			*reinterpret_cast<int*>(p+36) = info[1];
			*reinterpret_cast<int*>(p+40) = info[2];
			*reinterpret_cast<int*>(p+44) = info[3];

		#  else

			__asm {  // 32-bit assembler of Intel, Visual C++,...
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

		#  endif
		# endif

		}


		// frequency info
		uint32_t coreCrystalClockFrequencyHz;
		uint32_t baseFrequencyMHz;
		uint32_t maximumFrequencyMHz;
		uint32_t busFrequencyMHz;
		bool frequencyInfoPresent = (highestBasicFunction >= 0x16);
		if(frequencyInfoPresent) {

		# if __GNUC__

			// gcc assembler code
			asm volatile(

				"mov $0x15,%%eax\n"
				"cpuid\n"
				"mov %%ecx,%0\n"
				"mov $0x16,%%eax\n"
				"cpuid\n"
				"and $0xffff,%%eax\n"
				"and $0xffff,%%ebx\n"
				"and $0xffff,%%ecx\n"
				"mov %%eax,%1\n"
				"mov %%ebx,%2\n"
				"mov %%ecx,%3\n"

					: "=rm"(coreCrystalClockFrequencyHz), "=rm"(baseFrequencyMHz),
					  "=rm"(maximumFrequencyMHz), "=rm"(busFrequencyMHz)
					:
					: "%eax", "%ebx", "%ecx", "%edx");

		# else

			// assembler of Intel, Visual C++,...
		#  if _M_X64

			int info[4];
			__cpuid(info, 0x15);
			coreCrystalClockFrequencyHz = info[2];
			__cpuid(info, 0x16);
			baseFrequencyMHz = info[0] & 0xffff;
			maximumFrequencyMHz = info[1] & 0xffff;
			busFrequencyMHz = info[2] & 0xffff;

		#  else

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

		#  endif
		# endif

		}


		// CPU signature and feature bits
		uint32_t cpuSignature;
		uint32_t featureFlagsEBX;
		uint32_t featureFlagsECX;
		uint32_t featureFlagsEDX;
		if(highestBasicFunction >= 1) {

		# if __GNUC__

			// gcc assembler code
			asm volatile(

				"mov $1,%%eax\n"
				"cpuid\n"
				"mov %%eax,%0\n"
				"mov %%ebx,%1\n"
				"mov %%ecx,%2\n"
				"mov %%edx,%3\n"

					: "=rm"(cpuSignature), "=rm"(featureFlagsEBX),
					  "=rm"(featureFlagsECX), "=rm"(featureFlagsEDX)
					:
					: "%eax", "%ebx", "%ecx", "%edx");

		# else

			// assembler of Intel, Visual C++,...
		#  if _M_X64

			int info[4];
			__cpuid(info, 1);
			cpuSignature = info[0];
			featureFlagsEBX = info[1];
			featureFlagsECX = info[2];
			featureFlagsEDX = info[3];

		#  else

			__asm {
				mov eax,1
				cpuid
				mov cpuSignature,eax
				mov featureFlagsEBX,ebx
				mov featureFlagsECX,ecx
				mov featureFlagsEDX,edx
			};

		#  endif
		# endif

		}
		else
			cpuSignature = 0;


		// extended features
		uint32_t highestExtendedFeatures;
		uint32_t extendedFeatureFlags0EBX;
		uint32_t extendedFeatureFlags0ECX;
		uint32_t extendedFeatureFlags0EDX;
		uint32_t extendedFeatureFlags1EAX;
		uint32_t extendedFeatureFlags1EBX;
		uint32_t extendedFeatureFlags1ECX;
		uint32_t extendedFeatureFlags1EDX;
		bool hasExtendedFeatures0 = (highestBasicFunction >= 7);
		bool hasExtendedFeatures1 = false;
		if(hasExtendedFeatures0) {

		# if __GNUC__

			// gcc assembler code
			asm volatile(

				"mov $7,%%eax\n"
				"xor %%ecx,%%ecx\n"
				"cpuid\n"
				"mov %%eax,%0\n"
				"mov %%ebx,%1\n"
				"mov %%ecx,%2\n"
				"mov %%edx,%3\n"

					: "=rm"(highestExtendedFeatures), "=rm"(extendedFeatureFlags0EBX),
					  "=rm"(extendedFeatureFlags0ECX), "=rm"(extendedFeatureFlags0EDX)
					:
					: "%eax", "%ebx", "%ecx", "%edx");

		# else

			// assembler of Intel, Visual C++,...
		#  if _M_X64

			int info[4];
			__cpuidex(info, 7, 0);
			highestExtendedFeatures = info[0];
			extendedFeatureFlags0EBX = info[1];
			extendedFeatureFlags0ECX = info[2];
			extendedFeatureFlags0EDX = info[3];

		#  else

			__asm {
				mov eax,7
				xor ecx,ecx
				cpuid
				mov highestExtendedFeatures,eax
				mov extendedFeatureFlags0EBX,ebx
				mov extendedFeatureFlags0ECX,ecx
				mov extendedFeatureFlags0EDX,edx
			};

		#  endif
		# endif


			hasExtendedFeatures1 = (highestExtendedFeatures >= 1);
			if(hasExtendedFeatures1) {

			# if __GNUC__

				// gcc assembler code
				asm volatile(

					"mov $7,%%eax\n"
					"mov $1,%%ecx\n"
					"cpuid\n"
					"mov %%eax,%0\n"
					"mov %%ebx,%1\n"
					"mov %%ecx,%2\n"
					"mov %%edx,%3\n"

						: "=rm"(extendedFeatureFlags1EAX), "=rm"(extendedFeatureFlags1EBX),
						  "=rm"(extendedFeatureFlags1ECX), "=rm"(extendedFeatureFlags1EDX)
						:
						: "%eax", "%ebx", "%ecx", "%edx");

			# else

				// assembler of Intel, Visual C++,...
			#  if _M_X64

				int info[4];
				__cpuidex(info, 7, 1);
				extendedFeatureFlags1EAX = info[0];
				extendedFeatureFlags1EBX = info[1];
				extendedFeatureFlags1ECX = info[2];
				extendedFeatureFlags1EDX = info[3];

			#  else

				__asm {
					mov eax,7
					mov ecx,1
					cpuid
					mov extendedFeatureFlags1EAX,eax
					mov extendedFeatureFlags1EBX,ebx
					mov extendedFeatureFlags1ECX,ecx
					mov extendedFeatureFlags1EDX,edx
				};

			#  endif
			# endif

			}
		}


		// AVX10 info
		uint32_t highestAVX10Subleaf;
		uint32_t avx10FeatureInfo;
		if(highestBasicFunction >= 0x24) {

		# if __GNUC__

			// gcc assembler code
			asm volatile(

				"mov $0x24,%%eax\n"
				"xor %%ecx,%%ecx\n"
				"cpuid\n"
				"mov %%eax,%0\n"
				"mov %%ebx,%1\n"

					: "=rm"(highestAVX10Subleaf), "=rm"(avx10FeatureInfo)
					:
					: "%eax", "%ebx", "%ecx", "%edx");

		# else

			// assembler of Intel, Visual C++,...
		#  if _M_X64

			int info[4];
			__cpuidex(info, 0x24, 0);
			highestAVX10Subleaf = info[0];
			avx10FeatureInfo = info[1];

		#  else

			__asm {
				mov eax,0x24
				xor ecx,ecx
				cpuid
				mov highestAVX10Subleaf,eax
				mov avx10FeatureInfo,ebx
			};

		#  endif
		# endif

		}
		else {
			highestAVX10Subleaf = 0;
			avx10FeatureInfo = 0;
		}


		// print processor name
		cout << "   Name:  ";
		if(processorName[0] == '\0')
			cout << "< unknown>" << endl;
		else
			cout << processorName << endl;

		// print manufacturer string
		cout << "   Manufacturer string:  " << manufacturerIdString << endl;

		// print frequency info
		if(frequencyInfoPresent) {
			cout << "   Base frequency:       ";
			if(baseFrequencyMHz == 0)  cout << "not enumerated\n";
			else  cout << baseFrequencyMHz << " MHz\n";
			cout << "   Maximum frequency:    ";
			if(maximumFrequencyMHz == 0)  cout << "not enumerated\n";
			else  cout << maximumFrequencyMHz << " MHz\n";
			cout << "   Bus frequency:        ";
			if(busFrequencyMHz == 0)  cout << "not enumerated\n";
			else  cout << busFrequencyMHz << " MHz\n";
			cout << "   Crystal frequency:    ";
			if(coreCrystalClockFrequencyHz == 0)  cout << "not enumerated\n";
			else  cout << coreCrystalClockFrequencyHz << " MHz\n";
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

			cout << "   Processor signature:  0x" << hex << cpuSignature << dec << endl;
			cout << "   Family:    ";
			if(family < 10)  cout << family;
			else if(family<0x10)  cout << "0x0" << hex << family << dec;
			else  cout << "0x" << hex << family << dec;
			cout << "\n"
			        "   Model:     " << model << "\n"
			        "   Stepping:  " << stepping << endl;

			cout << "   x87 FPU:  " << ((featureFlagsEDX & 0x00000001) ? "yes" : "no") << "\n"
			        "   MMX:      " << ((featureFlagsEDX & 0x00800000) ? "yes" : "no") << "\n"
			        "   SSE:      " << ((featureFlagsEDX & 0x02000000) ? "yes" : "no") << "\n"
			        "   SSE2:     " << ((featureFlagsEDX & 0x04000000) ? "yes" : "no") << "\n"
			        "   SSE3:     " << ((featureFlagsECX & 0x00000001) ? "yes" : "no") << "\n"
			        "   SSSE3:    " << ((featureFlagsECX & 0x00000200) ? "yes" : "no") << "\n"
			        "   FMA3:     " << ((featureFlagsECX & 0x00001000) ? "yes" : "no") << "\n"
			        "   SSE4.1:   " << ((featureFlagsECX & 0x00080000) ? "yes" : "no") << "\n"
			        "   SSE4.2:   " << ((featureFlagsECX & 0x00100000) ? "yes" : "no") << "\n"
			        "   AVX:      " << ((featureFlagsECX & 0x10000000) ? "yes" : "no") << endl;

		}
		else
			cout << "   Processor signature:  n/a\n"
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
			        "   AVX:      n/a\n";

		// print extended features 0
		if(hasExtendedFeatures0)
			cout << "   AVX2:     " << ((extendedFeatureFlags0EBX & 0x00000020) ? "yes" : "no") << "\n"
			        "   ADX:      " << ((extendedFeatureFlags0EBX & 0x00080000) ? "yes" : "no") << "\n"
			        "   AVX512-f:   " << ((extendedFeatureFlags0EBX & 0x00010000) ? "yes" : "no") << "\n"
			        "   AVX512-dq:  " << ((extendedFeatureFlags0EBX & 0x00010000) ? "yes" : "no") << "\n"
			        "   AVX512-4fmaps:  " << ((extendedFeatureFlags0EDX & 0x00000008) ? "yes" : "no") << "\n"
			        "   AVX512-fp16:  " << ((extendedFeatureFlags0EDX & 0x00800000) ? "yes" : "no") << "\n";
		else
			cout << "   AVX2:     n/a\n"
			        "   ADX:      n/a\n"
			        "   AVX512-f:   n/a\n"
			        "   AVX512-dq:  n/a\n"
			        "   AVX512-4fmaps:  n/a\n"
			        "   AVX512-fp16:  n/a\n";

		// print extended features 1
		if(hasExtendedFeatures1)
			cout << "   APX:      " << ((extendedFeatureFlags1EDX & 0x00200000) ? "yes" : "no") << "\n"
			        "   AVX10:    " << ((extendedFeatureFlags1EDX & 0x00080000) ? "yes" : "no") << "\n"
			        "   AMX-fp16:  " << ((extendedFeatureFlags1EAX & 0x00200000) ? "yes" : "no") << "\n";
		else
			cout << "   APX:      n/a\n"
			        "   AVX10:    n/a\n"
			        "   AMX-fp16:  n/a\n";

		// print AVX10 info
		if(avx10FeatureInfo != 0)
			cout << "   AVX10 version:  " << (avx10FeatureInfo & 0xFF) << "\n";
		else
			cout << "   AVX10 version:  n/a\n";

		cout << flush;

	}
}
