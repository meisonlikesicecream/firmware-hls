#include "VMRouterTop_D1PHIA.h"

// VMRouter Top Function for Disk 1, AllStub region A
// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).

void VMRouterTop(BXType bx,
	// Input memories
	const InputStubMemory<inputType> inputStub[numInputs],
#if kDISK > 0
	const InputStubMemory<DISK2S> inputStubDisk2S[numInputsDisk2S], // Only disks has 2S modules
#endif

	// Output memories
#if kLAYER == 1 || kLAYER == 3 || kLAYER == 5 || kDISK == 1 || kDISK == 3
	VMStubTEInnerMemory<outputType> teiMemories[numTEI][maxTEICopies],
#endif

#if kLAYER == 1 || kLAYER == 2
	VMStubTEInnerMemory<BARRELOL> olMemories[numOL][maxOLCopies],
#endif

#if kLAYER == 2 || kLAYER == 4 || kLAYER == 6 || kDISK == 1 || kDISK == 2 || kDISK == 4
	VMStubTEOuterMemory<outputType> teoMemories[numTEO][maxTEOCopies],
#endif

	AllStubMemory<outputType> allStub[maxAllCopies],
	VMStubMEMemory<outputType, nbitsbin> meMemories[numME]) {

//////////////////////////////////
// Variables for that are specified with regards to the VMR region

	// Masks of which memories that are being used. The first memory is represented by the LSB
	static const ap_uint<inmasksize> imask(0x3F); // Input memories
	static const ap_uint<memasksize> memask(0x000000FFUL); // ME memories
	static const ap_uint<teimasksize> teimask(0x0000000FUL); // TE Inner memories
	static const ap_uint<olmasksize> olmask(0x000); // TE Inner Overlap memories
	static const ap_uint<teomasksize> teomask(0x0000000FUL); // TE Outer memories


	///////////////////////////
	// Open Lookup tables

	// LUT with the corrected r/z. It is corrected for the average r (z) of the barrel (disk).
	// Includes both coarse r/z position (bin), and finer region each r/z bin is divided into.
	// Indexed using r and z position bits
	static const int finebintable[] =
#include "../emData/VMR/tables/VMR_D1PHIA_finebin.tab"
	;

	// LUT with phi corrections to project the stub to the nominal radius.
	// Only used by layers.
	// Indexed using phi and bend bits
	// 	static const int phicorrtable[] =
	// #include "../emData/VMR/tables/VMPhiCorrL1.txt"
	// 	;

	// LUT with the Z/R bits for TE memories
	// Contain information about where in z to look for valid stub pairs
	// Indexed using z and r position bits
	static const int rzbitstable[] = // 11 bits used for LUT
#include "../emData/VMR/tables/VMTableInnerD1D2.tab" // Only for Layer 1


	// LUT with the Z/R bits for TE Overlap memories
	// Only used for Layer 1 and 2, and Disk 1
	// Indexed using z and r position bits
	static const int rzbitsextratable[] = // 10 bits used for LUT
#include "../emData/VMR/tables/VMTableOuterD1.tab" // Only for Layer 1


	// LUT with bend-cuts for the TE memories
	// The cuts are different depending on the memory version (nX)
	// Indexed using bend bits

	// TE Memory 1
	ap_uint<1> tmptable1_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA1n1_vmbendcut.tab"

	ap_uint<1> tmptable1_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA1n2_vmbendcut.tab"

	ap_uint<1> tmptable1_n3[bendtablesize] = {0};

	// TE Memory 2
	ap_uint<1> tmptable2_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA2n1_vmbendcut.tab"

	ap_uint<1> tmptable2_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA2n2_vmbendcut.tab"

	ap_uint<1> tmptable2_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA2n3_vmbendcut.tab"


	// TE Memory 3
	ap_uint<1> tmptable3_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA3n1_vmbendcut.tab"

	ap_uint<1> tmptable3_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA3n2_vmbendcut.tab"

	ap_uint<1> tmptable3_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA3n3_vmbendcut.tab"


	// TE Memory 4
	ap_uint<1> tmptable4_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA4n1_vmbendcut.tab"

	ap_uint<1> tmptable4_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA4n2_vmbendcut.tab"

	ap_uint<1> tmptable4_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA4n3_vmbendcut.tab"


	// Combine all the temporary tables into one big table
	static const ap_uint<bendtablesize> bendtable[] = {
		arrayToInt<bendtablesize>(tmptable1_n1), arrayToInt<bendtablesize>(tmptable1_n2), arrayToInt<bendtablesize>(tmptable1_n3),
		arrayToInt<bendtablesize>(tmptable2_n1), arrayToInt<bendtablesize>(tmptable2_n2), arrayToInt<bendtablesize>(tmptable2_n3),
		arrayToInt<bendtablesize>(tmptable3_n1), arrayToInt<bendtablesize>(tmptable3_n2), arrayToInt<bendtablesize>(tmptable3_n3),
		arrayToInt<bendtablesize>(tmptable4_n1), arrayToInt<bendtablesize>(tmptable4_n2), arrayToInt<bendtablesize>(tmptable4_n3)};

	// TE Overlap Memory 1
	ap_uint<1> tmpextratable1_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX1n1_vmbendcut.tab"

	ap_uint<1> tmpextratable1_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX1n2_vmbendcut.tab"

	ap_uint<1> tmpextratable1_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX1n3_vmbendcut.tab"

	ap_uint<1> tmpextratable1_n4[bendtablesize] = {0};

	ap_uint<1> tmpextratable1_n5[bendtablesize] = {0};

	// TE Overlap Memory 2
	ap_uint<1> tmpextratable2_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX2n1_vmbendcut.tab"

	ap_uint<1> tmpextratable2_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX2n2_vmbendcut.tab"

	ap_uint<1> tmpextratable2_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX2n3_vmbendcut.tab"

	ap_uint<1> tmpextratable2_n4[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX2n4_vmbendcut.tab"

	ap_uint<1> tmpextratable2_n5[bendtablesize] = {0};

	// TE Overlap Memory 3
	ap_uint<1> tmpextratable3_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX3n1_vmbendcut.tab"

	ap_uint<1> tmpextratable3_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX3n2_vmbendcut.tab"

	ap_uint<1> tmpextratable3_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX3n3_vmbendcut.tab"

	ap_uint<1> tmpextratable3_n4[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX3n4_vmbendcut.tab"

	ap_uint<1> tmpextratable3_n5[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX3n5_vmbendcut.tab"


	// TE Overlap Memory 4
	ap_uint<1> tmpextratable4_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX4n1_vmbendcut.tab"

	ap_uint<1> tmpextratable4_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX4n2_vmbendcut.tab"

	ap_uint<1> tmpextratable4_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX4n3_vmbendcut.tab"

	ap_uint<1> tmpextratable4_n4[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX4n4_vmbendcut.tab"

	ap_uint<1> tmpextratable4_n5[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX4n5_vmbendcut.tab"


	// Combine all the temporary extra tables into one big table
	// Combine all the temporary extra tables into one big table
	static const ap_uint<bendtablesize> bendextratable[] = {
		arrayToInt<bendtablesize>(tmpextratable1_n1), arrayToInt<bendtablesize>(tmpextratable1_n2), arrayToInt<bendtablesize>(tmpextratable1_n3), arrayToInt<bendtablesize>(tmpextratable1_n4), arrayToInt<bendtablesize>(tmpextratable1_n5),
		arrayToInt<bendtablesize>(tmpextratable2_n1), arrayToInt<bendtablesize>(tmpextratable2_n2), arrayToInt<bendtablesize>(tmpextratable2_n3), arrayToInt<bendtablesize>(tmpextratable2_n4), arrayToInt<bendtablesize>(tmpextratable2_n5),
		arrayToInt<bendtablesize>(tmpextratable3_n1), arrayToInt<bendtablesize>(tmpextratable3_n2), arrayToInt<bendtablesize>(tmpextratable3_n3), arrayToInt<bendtablesize>(tmpextratable3_n4), arrayToInt<bendtablesize>(tmpextratable3_n5),
		arrayToInt<bendtablesize>(tmpextratable4_n1), arrayToInt<bendtablesize>(tmpextratable4_n2), arrayToInt<bendtablesize>(tmpextratable4_n3), arrayToInt<bendtablesize>(tmpextratable4_n4), arrayToInt<bendtablesize>(tmpextratable4_n5)};


// Takes 2 clock cycles before on gets data, used at high frequencies
#pragma HLS resource variable=inputStub[0].get_mem() latency=2
#pragma HLS resource variable=inputStub[1].get_mem() latency=2
#pragma HLS resource variable=inputStub[2].get_mem() latency=2
#pragma HLS resource variable=inputStub[3].get_mem() latency=2
#pragma HLS resource variable=inputStubDisk2S[0].get_mem() latency=2
#pragma HLS resource variable=inputStubDisk2S[1].get_mem() latency=2

#pragma HLS resource variable=finebintable latency=2
#pragma HLS resource variable=rzbitstable latency=2
#pragma HLS resource variable=rzbitsextratable latency=2
//#pragma HLS resource variable=bendtable latency=2
//#pragma HLS resource variable=bendextratable latency=2
//phicorrtable and bendtable seems to be using LUTs as they relatively small?


/////////////////////////
// Main function

	// template<regionType InType, regionType OutType, int Layer, int Disk, int MaxAllCopies, int MaxTEICopies, int MaxOLCopies, int MaxTEOCopies>
	VMRouter<inputType, outputType, kLAYER, kDISK,  maxAllCopies, maxTEICopies, maxOLCopies, maxTEOCopies, nbitsbin, bendtablesize>
	(bx, finebintable, nullptr,
		rzbitstable, nullptr, rzbitsextratable,
		bendtable, nullptr, bendextratable,
// Input memories
		imask, inputStub, inputStubDisk2S,
// AllStub memories
		allStub,
// ME memories
		memask, meMemories,
// TEInner memories
		teimask, teiMemories,
// TEInner Overlap memories
		olmask, nullptr,
// TEOuter memories
		teomask, teoMemories
		 );

	return;
}

///////////////////////////////////////////////////////
// Help functions

// Converts an array of 0s and 1s to an ap_uint
template<int arraysize>
inline ap_uint<arraysize> arrayToInt(ap_uint<1> array[arraysize]) {
	ap_uint<arraysize> number;

	for(int i = 0; i < arraysize; i++) {
		#pragma HLS unroll
		number[i] = array[i];
	}

	return number;
}

// Creates (memory) masks for a specific phi region with "nvm" VMs.
// LSB corresponds to the first memory and MSB to the last,
// a "1" implies that the specified memory is used for this phi region
template<int masksize>
inline ap_uint<masksize> createMask(int phi, int nvm) {
	ap_uint<masksize> mask = (1 << nvm) - 1; // Create "nvm" 1s, e.g. "1111"
	mask = mask << nvm * (phi - 'A'); // Shift the mask until it corresponds to the correct phi region

	return mask;
}
