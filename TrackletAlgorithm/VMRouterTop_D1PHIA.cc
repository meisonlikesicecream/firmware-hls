#include "VMRouterTop_D1PHIA.h"

// VMRouter Top Function for Disk 1, AllStub region A
// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).

// NOTE: To run this VMR, change the following
//          - the included top function in VMRouter_test.cpp
//          - the top function in script_VMR.tcl


void VMRouterTop(BXType bx,
	// Input memories
	const InputStubMemory<inputType> inputStub[numInputs],
	const InputStubMemory<DISK2S> inputStubDisk2S[numInputsDisk2S], // Only disks has 2S modules
	// Output memories
	AllStubMemory<outputType> memoriesAS[maxASCopies],
	VMStubMEMemory<outputType, nbitsbin> memoriesME[numME],
	VMStubTEInnerMemory<outputType> memoriesTEI[numTEI][maxTEICopies],
	VMStubTEOuterMemory<outputType> memoriesTEO[numTEO][maxTEOCopies])
 {

	///////////////////////////
	// Open Lookup tables

	// LUT with the corrected r/z. It is corrected for the average r (z) of the barrel (disk).
	// Includes both coarse r/z position (bin), and finer region each r/z bin is divided into.
	// Indexed using r and z position bits
	static const int fineBinTable[] =
#include "../emData/VMR/tables/VMR_D1PHIA_finebin.tab"
	;

	// LUT with phi corrections to project the stub to the nominal radius.
	// Only used by layers.
	// Indexed using phi and bend bits
	// 	static const int phiCorrtTable[] =
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
	static const int rzbitsextratable[] = // 11 bits used for LUT
#include "../emData/VMR/tables/VMTableOuterD1.tab" // Only for Layer 1


	// LUT with bend-cuts for the TE memories
	// The cuts are different depending on the memory version (nX)
	// Indexed using bend bits

	// TE Memory 1
	ap_uint<1> tmpBendTable1_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA1n1_vmbendcut.tab"

	ap_uint<1> tmpBendTable1_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA1n2_vmbendcut.tab"

	ap_uint<1> tmpBendTable1_n3[bendCutTableSize] = {0};


	// TE Memory 2
	ap_uint<1> tmpBendTable2_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA2n1_vmbendcut.tab"

	ap_uint<1> tmpBendTable2_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA2n2_vmbendcut.tab"

	ap_uint<1> tmpBendTable2_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA2n3_vmbendcut.tab"


	// TE Memory 3
	ap_uint<1> tmpBendTable3_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA3n1_vmbendcut.tab"

	ap_uint<1> tmpBendTable3_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA3n2_vmbendcut.tab"

	ap_uint<1> tmpBendTable3_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA3n3_vmbendcut.tab"


	// TE Memory 4
	ap_uint<1> tmpBendTable4_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA4n1_vmbendcut.tab"

	ap_uint<1> tmpBendTable4_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA4n2_vmbendcut.tab"

	ap_uint<1> tmpBendTable4_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIA4n3_vmbendcut.tab"


	// Combine all the temporary tables into one big table
	static const ap_uint<bendCutTableSize> bendCutTable[] = {
		arrayToInt<bendCutTableSize>(tmpBendTable1_n1), arrayToInt<bendCutTableSize>(tmpBendTable1_n2), arrayToInt<bendCutTableSize>(tmpBendTable1_n3),
		arrayToInt<bendCutTableSize>(tmpBendTable2_n1), arrayToInt<bendCutTableSize>(tmpBendTable2_n2), arrayToInt<bendCutTableSize>(tmpBendTable2_n3),
		arrayToInt<bendCutTableSize>(tmpBendTable3_n1), arrayToInt<bendCutTableSize>(tmpBendTable3_n2), arrayToInt<bendCutTableSize>(tmpBendTable3_n3),
		arrayToInt<bendCutTableSize>(tmpBendTable4_n1), arrayToInt<bendCutTableSize>(tmpBendTable4_n2), arrayToInt<bendCutTableSize>(tmpBendTable4_n3)};

	// TE Overlap Memory 1
	ap_uint<1> tmpBendExtraTable1_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX1n1_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable1_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX1n2_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable1_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX1n3_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable1_n4[bendCutTableSize] = {0};

	ap_uint<1> tmpBendExtraTable1_n5[bendCutTableSize] = {0};


	// TE Overlap Memory 2
	ap_uint<1> tmpBendExtraTable2_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX2n1_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable2_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX2n2_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable2_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX2n3_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable2_n4[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX2n4_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable2_n5[bendCutTableSize] =
#include "../emData/VMR/tables/VMSTE_D1PHIX2n5_vmbendcut.tab"

	// TE Overlap Memory 3
	ap_uint<1> tmpBendExtraTable3_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX3n1_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable3_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX3n2_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable3_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX3n3_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable3_n4[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX3n4_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable3_n5[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX3n5_vmbendcut.tab"


	// TE Overlap Memory 4
	ap_uint<1> tmpBendExtraTable4_n1[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX4n1_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable4_n2[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX4n2_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable4_n3[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX4n3_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable4_n4[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX4n4_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable4_n5[] =
#include "../emData/VMR/tables/VMSTE_D1PHIX4n5_vmbendcut.tab"


	// Combine all the temporary extra tables into one big table
	// Combine all the temporary extra tables into one big table
	static const ap_uint<bendCutTableSize> bendCutExtraTable[] = {
		arrayToInt<bendCutTableSize>(tmpBendExtraTable1_n1), arrayToInt<bendCutTableSize>(tmpBendExtraTable1_n2), arrayToInt<bendCutTableSize>(tmpBendExtraTable1_n3), arrayToInt<bendCutTableSize>(tmpBendExtraTable1_n4), arrayToInt<bendCutTableSize>(tmpBendExtraTable1_n5),
		arrayToInt<bendCutTableSize>(tmpBendExtraTable2_n1), arrayToInt<bendCutTableSize>(tmpBendExtraTable2_n2), arrayToInt<bendCutTableSize>(tmpBendExtraTable2_n3), arrayToInt<bendCutTableSize>(tmpBendExtraTable2_n4), arrayToInt<bendCutTableSize>(tmpBendExtraTable2_n5),
		arrayToInt<bendCutTableSize>(tmpBendExtraTable3_n1), arrayToInt<bendCutTableSize>(tmpBendExtraTable3_n2), arrayToInt<bendCutTableSize>(tmpBendExtraTable3_n3), arrayToInt<bendCutTableSize>(tmpBendExtraTable3_n4), arrayToInt<bendCutTableSize>(tmpBendExtraTable3_n5),
		arrayToInt<bendCutTableSize>(tmpBendExtraTable4_n1), arrayToInt<bendCutTableSize>(tmpBendExtraTable4_n2), arrayToInt<bendCutTableSize>(tmpBendExtraTable4_n3), arrayToInt<bendCutTableSize>(tmpBendExtraTable4_n4), arrayToInt<bendCutTableSize>(tmpBendExtraTable4_n5)};


// Takes 2 clock cycles before on gets data, used at high frequencies
#pragma HLS resource variable=inputStub[0].get_mem() latency=2
#pragma HLS resource variable=inputStub[1].get_mem() latency=2
#pragma HLS resource variable=inputStub[2].get_mem() latency=2
#pragma HLS resource variable=inputStub[3].get_mem() latency=2
#pragma HLS resource variable=inputStubDisk2S[0].get_mem() latency=2
#pragma HLS resource variable=inputStubDisk2S[1].get_mem() latency=2

#pragma HLS resource variable=fineBinTable latency=2
#pragma HLS resource variable=rzbitstable latency=2
#pragma HLS resource variable=rzbitsextratable latency=2
//#pragma HLS resource variable=bendCutTable latency=2
//#pragma HLS resource variable=bendCutExtraTable latency=2
//phiCorrtTable and bendCutTable seems to be using LUTs as they relatively small?


	//////////////////////////////////
	// Create memory masks

	constexpr int nvmME = (kLAYER) ? nvmmelayers[kLAYER-1] : nvmmedisks[kDISK-1]; // ME memories
	constexpr int nvmTE = (kLAYER) ? nvmtelayers[kLAYER-1] : nvmtedisks[kDISK-1]; // TE memories
	constexpr int nvmOL = ((kLAYER == 1) || (kLAYER == 2)) ? nvmollayers[kLAYER-1] : 0; // TE Overlap memories

	// Masks of which memories that are being used. The first memory is represented by the LSB
	// and a "1" implies that the specified memory is used for this phi region
	// Create "nvm" 1s, e.g. "1111", shift the mask until it corresponds to the correct phi region
	static const ap_uint<maskISsize> maskIS = ((1 << numInputs) - 1); // Input memories
	static const ap_uint<maskMEsize> maskME = ((1 << nvmME) - 1) << (nvmME * (phiRegion - 'A')); // ME memories, won't synthesise if I use createMask?!
	static const ap_uint<maskTEIsize> maskTEI = ((kLAYER % 2) || (kDISK % 2)) ? ((1 << nvmTE) - 1) << (nvmTE * (phiRegion - 'A')) : 0x0; // TE Inner memories, only used for odd layers/disk
	static const ap_uint<maskOLsize> maskOL = (nvmOL) ? ((1 << nvmOL) - 1) << (nvmOL * (phiRegion - 'A')) : 0x0; // TE Inner Overlap memories, only used for layer 1 and 2
	static const ap_uint<maskTEOsize> maskTEO = (!((kLAYER % 2) || (kDISK % 2)) || (kDISK == 1)) ? ((1 << nvmTE) - 1) << (nvmTE * (phiRegion - 'A')) : 0x0; // TE Outer memories, only for even layers/disks and disk 1


	/////////////////////////
	// Main function

	// template<regionType InType, regionType OutType, int Layer, int Disk, int MaxAllCopies, int MaxTEICopies, int MaxOLCopies, int MaxTEOCopies>
	VMRouter<inputType, outputType, kLAYER, kDISK,  maxASCopies, maxTEICopies, maxOLCopies, maxTEOCopies, nbitsbin, bendCutTableSize>
	(bx, fineBinTable, nullptr,
		rzbitstable, nullptr, rzbitsextratable,
		bendCutTable, nullptr, bendCutExtraTable,
		// Input memories
		maskIS, inputStub, inputStubDisk2S,
		// AllStub memories
		memoriesAS,
		// ME memories
		maskME, memoriesME,
		// TEInner memories
		maskTEI, memoriesTEI,
		// TEInner Overlap memories
		maskOL, nullptr,
		// TEOuter memories
		maskTEO, memoriesTEO
		 );

	return;
}
