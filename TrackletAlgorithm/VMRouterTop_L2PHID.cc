#include "VMRouterTop_L2PHID.h"

// VMRouter Top Function for layer 1, AllStub region E
// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).

// NOTE: to run a different phi region, change the following
//          - constants specified in VMRouterTop.h
//          - the input parameters to VMRouterTop in VMRouterTop.h/.cc
//          - the the number and directories to the LUTs
//          - add/remove pragmas depending on inputStub in VMRouterTop.cc
//          - the call to VMRouter() in VMRouterTop.cc
//          - the included top function in VMRouter_test.cpp (if file name is changed)
//          - the top function in script_VMR.tcl (if file name is changed)


void VMRouterTop(BXType bx,
	// Input memories
	const InputStubMemory<inputType> inputStub[numInputs],

	// Output memories
	AllStubMemory<outputType> memoriesAS[maxASCopies],
	VMStubMEMemory<outputType, nbitsbin> memoriesME[nvmME],
	VMStubTEInnerMemory<outputType> memoriesTEI[nvmTEI][maxTEICopies],
	VMStubTEInnerMemory<BARRELOL> memoriesOL[nvmOL][maxOLCopies],
	VMStubTEOuterMemory<outputType> memoriesTEO[nvmTEO][maxTEOCopies])
 {


	///////////////////////////
	// Open Lookup tables
	// NOTE: needs to be changed manually if run for a different phi region

	// LUT with the corrected r/z. It is corrected for the average r (z) of the barrel (disk).
	// Includes both coarse r/z position (bin), and finer region each r/z bin is divided into.
	// Indexed using r and z position bits
	static const int fineBinTable[] =
#include "../emData/VMR/tables/VMR_L2PHID_finebin.tab"


	// LUT with phi corrections to project the stub to the average radius in a layer.
	// Only used by layers.
	// Indexed using phi and bend bits
	static const int phiCorrTable[] =
#include "../emData/VMR/tables/VMPhiCorrL2.tab"


	// LUT with the Z/R bits for TE memories
	// Contain information about where in z to look for valid stub pairs
	// Indexed using z and r position bits

	static const int rzBitsInnerTable[] = // 11 bits used for LUT
#include "../emData/VMR/tables/VMTableInnerL2L3.tab"

	static const int rzBitsOverlapTable[] = // 11 bits used for LUT
#include "../emData/VMR/tables/VMTableInnerL2D1.tab"

	static const int rzBitsOuterTable[] = // 11 bits used for LUT
#include "../emData/VMR/tables/VMTableOuterL2.tab"


	// LUT with bend-cuts for the TE memories
	// The cuts are different depending on the memory version (nX)
	// Indexed using bend bits

	// TE Memory 1
	ap_uint<1> tmpBendInnerTable1_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHIL13n1_vmbendcut.tab"

	ap_uint<1> tmpBendInnerTable1_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHIL13n2_vmbendcut.tab"

	ap_uint<1> tmpBendInnerTable1_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHIL13n3_vmbendcut.tab"

	// TE Memory 2
	ap_uint<1> tmpBendInnerTable2_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHIL14n1_vmbendcut.tab"

	ap_uint<1> tmpBendInnerTable2_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHIL14n2_vmbendcut.tab"

	ap_uint<1> tmpBendInnerTable2_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHIL14n3_vmbendcut.tab"

	// TE Memory 3
	ap_uint<1> tmpBendInnerTable3_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHIL15n1_vmbendcut.tab"

	ap_uint<1> tmpBendInnerTable3_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHIL15n2_vmbendcut.tab"

	ap_uint<1> tmpBendInnerTable3_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHIL15n3_vmbendcut.tab"

// TE Memory 4
	ap_uint<1> tmpBendInnerTable4_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHIL16n1_vmbendcut.tab"

	ap_uint<1> tmpBendInnerTable4_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHIL16n2_vmbendcut.tab"

	ap_uint<1> tmpBendInnerTable4_n3[bendCutTableSize] = {0};

	// Combine all the temporary tables into one big table
	static const ap_uint<bendCutTableSize> bendCutInnerTable[] = {
		arrayToInt<bendCutTableSize>(tmpBendInnerTable1_n1), arrayToInt<bendCutTableSize>(tmpBendInnerTable1_n2), arrayToInt<bendCutTableSize>(tmpBendInnerTable1_n3),
		arrayToInt<bendCutTableSize>(tmpBendInnerTable2_n1), arrayToInt<bendCutTableSize>(tmpBendInnerTable2_n2), arrayToInt<bendCutTableSize>(tmpBendInnerTable2_n3),
		arrayToInt<bendCutTableSize>(tmpBendInnerTable3_n1), arrayToInt<bendCutTableSize>(tmpBendInnerTable3_n2), arrayToInt<bendCutTableSize>(tmpBendInnerTable3_n3),
		arrayToInt<bendCutTableSize>(tmpBendInnerTable4_n1), arrayToInt<bendCutTableSize>(tmpBendInnerTable4_n2), arrayToInt<bendCutTableSize>(tmpBendInnerTable4_n3)};


	// TE Overlap Memory 1
	ap_uint<1> tmpBendOverlapTable1_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW7n1_vmbendcut.tab"

	ap_uint<1> tmpBendOverlapTable1_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW7n2_vmbendcut.tab"

	ap_uint<1> tmpBendOverlapTable1_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW7n3_vmbendcut.tab"

	ap_uint<1> tmpBendOverlapTable1_n4[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW7n4_vmbendcut.tab"

	// TE Overlap Memory 2
	ap_uint<1> tmpBendOverlapTable2_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW8n1_vmbendcut.tab"

	ap_uint<1> tmpBendOverlapTable2_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW8n2_vmbendcut.tab"

	ap_uint<1> tmpBendOverlapTable2_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW8n3_vmbendcut.tab"

	ap_uint<1> tmpBendOverlapTable2_n4[bendCutTableSize] = {0};

	// Combine all the temporary Overlap tables into one big table
	static const ap_uint<bendCutTableSize> bendCutOverlapTable[] = {
		arrayToInt<bendCutTableSize>(tmpBendOverlapTable1_n1), arrayToInt<bendCutTableSize>(tmpBendOverlapTable1_n2), arrayToInt<bendCutTableSize>(tmpBendOverlapTable1_n3), arrayToInt<bendCutTableSize>(tmpBendOverlapTable1_n4),
		arrayToInt<bendCutTableSize>(tmpBendOverlapTable2_n1), arrayToInt<bendCutTableSize>(tmpBendOverlapTable2_n2), arrayToInt<bendCutTableSize>(tmpBendOverlapTable2_n3), arrayToInt<bendCutTableSize>(tmpBendOverlapTable2_n4)};


		// TE Memory 1
		ap_uint<1> tmpBendOuterTable1_n1[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID25n1_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable1_n2[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID25n2_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable1_n3[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID25n3_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable1_n4[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID25n4_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable1_n5[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID25n5_vmbendcut.tab"

		// TE Memory 2
		ap_uint<1> tmpBendOuterTable2_n1[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID26n1_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable2_n2[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID26n2_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable2_n3[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID26n3_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable2_n4[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID26n4_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable2_n5[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID26n5_vmbendcut.tab"

		// TE Memory 3
		ap_uint<1> tmpBendOuterTable3_n1[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID27n1_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable3_n2[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID27n2_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable3_n3[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID27n3_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable3_n4[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID27n4_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable3_n5[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID27n5_vmbendcut.tab"

	// TE Memory 4
		ap_uint<1> tmpBendOuterTable4_n1[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID28n1_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable4_n2[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID28n2_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable4_n3[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID28n3_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable4_n4[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID28n4_vmbendcut.tab"

		ap_uint<1> tmpBendOuterTable4_n5[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID28n5_vmbendcut.tab"

	// TE Memory 5
	ap_uint<1> tmpBendOuterTable5_n1[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID29n1_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable5_n2[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID29n2_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable5_n3[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID29n3_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable5_n4[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID29n4_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable5_n5[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID29n5_vmbendcut.tab"

	// TE Memory 6
	ap_uint<1> tmpBendOuterTable6_n1[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID30n1_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable6_n2[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID30n2_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable6_n3[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID30n3_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable6_n4[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID30n4_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable6_n5[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID30n5_vmbendcut.tab"

	// TE Memory 7
	ap_uint<1> tmpBendOuterTable7_n1[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID31n1_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable7_n2[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID31n2_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable7_n3[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID31n3_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable7_n4[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID31n4_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable7_n5[bendCutTableSize] = {0};

	// TE Memory 8
	ap_uint<1> tmpBendOuterTable8_n1[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID32n1_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable8_n2[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID32n2_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable8_n3[] =
	#include "../emData/VMR/tables/VMSTE_L2PHID32n3_vmbendcut.tab"

	ap_uint<1> tmpBendOuterTable8_n4[bendCutTableSize] = {0};

	ap_uint<1> tmpBendOuterTable8_n5[bendCutTableSize] = {0};

		// Combine all the temporary tables into one big table
		static const ap_uint<bendCutTableSize> bendCutOuterTable[] = {
			arrayToInt<bendCutTableSize>(tmpBendOuterTable1_n1), arrayToInt<bendCutTableSize>(tmpBendOuterTable1_n2), arrayToInt<bendCutTableSize>(tmpBendOuterTable1_n3), arrayToInt<bendCutTableSize>(tmpBendOuterTable1_n4), arrayToInt<bendCutTableSize>(tmpBendOuterTable1_n5),
			arrayToInt<bendCutTableSize>(tmpBendOuterTable2_n1), arrayToInt<bendCutTableSize>(tmpBendOuterTable2_n2), arrayToInt<bendCutTableSize>(tmpBendOuterTable2_n3), arrayToInt<bendCutTableSize>(tmpBendOuterTable2_n4), arrayToInt<bendCutTableSize>(tmpBendOuterTable2_n5),
			arrayToInt<bendCutTableSize>(tmpBendOuterTable3_n1), arrayToInt<bendCutTableSize>(tmpBendOuterTable3_n2), arrayToInt<bendCutTableSize>(tmpBendOuterTable3_n3), arrayToInt<bendCutTableSize>(tmpBendOuterTable3_n4), arrayToInt<bendCutTableSize>(tmpBendOuterTable3_n5),
			arrayToInt<bendCutTableSize>(tmpBendOuterTable4_n1), arrayToInt<bendCutTableSize>(tmpBendOuterTable4_n2), arrayToInt<bendCutTableSize>(tmpBendOuterTable4_n3), arrayToInt<bendCutTableSize>(tmpBendOuterTable4_n4), arrayToInt<bendCutTableSize>(tmpBendOuterTable4_n5),
			arrayToInt<bendCutTableSize>(tmpBendOuterTable5_n1), arrayToInt<bendCutTableSize>(tmpBendOuterTable5_n2), arrayToInt<bendCutTableSize>(tmpBendOuterTable5_n3), arrayToInt<bendCutTableSize>(tmpBendOuterTable5_n4), arrayToInt<bendCutTableSize>(tmpBendOuterTable5_n5),
			arrayToInt<bendCutTableSize>(tmpBendOuterTable6_n1), arrayToInt<bendCutTableSize>(tmpBendOuterTable6_n2), arrayToInt<bendCutTableSize>(tmpBendOuterTable6_n3), arrayToInt<bendCutTableSize>(tmpBendOuterTable6_n4), arrayToInt<bendCutTableSize>(tmpBendOuterTable6_n5),
			arrayToInt<bendCutTableSize>(tmpBendOuterTable7_n1), arrayToInt<bendCutTableSize>(tmpBendOuterTable7_n2), arrayToInt<bendCutTableSize>(tmpBendOuterTable7_n3), arrayToInt<bendCutTableSize>(tmpBendOuterTable7_n4), arrayToInt<bendCutTableSize>(tmpBendOuterTable7_n5),
			arrayToInt<bendCutTableSize>(tmpBendOuterTable8_n1), arrayToInt<bendCutTableSize>(tmpBendOuterTable8_n2), arrayToInt<bendCutTableSize>(tmpBendOuterTable8_n3), arrayToInt<bendCutTableSize>(tmpBendOuterTable8_n4), arrayToInt<bendCutTableSize>(tmpBendOuterTable8_n5)};


// Takes 2 clock cycles before on gets data, used at high frequencies
#pragma HLS resource variable=inputStub[0].get_mem() latency=2
#pragma HLS resource variable=inputStub[1].get_mem() latency=2

#pragma HLS resource variable=fineBinTable latency=2
#pragma HLS resource variable=rzBitsInnerTable latency=2
#pragma HLS resource variable=rzBitsOuterTable latency=2
#pragma HLS resource variable=rzBitsOverlapTable latency=2
// #pragma HLS resource variable=bendCutInnerTable latency=2
// #pragma HLS resource variable=bendCutOverlapTable latency=2
// phiCorrTable and bendCutTable seems to be using LUTs as they relatively small?


	//////////////////////////////////
	// Create memory masks

	// Masks of which memories that are being used. The first memory is represented by the LSB
	// and a "1" implies that the specified memory is used for this phi region
	// Create "nvm" 1s, e.g. "1111", shift the mask until it corresponds to the correct phi region
	static const ap_uint<maskISsize> maskIS = ((1 << numInputs) - 1); // Input memories
	static const ap_uint<maskMEsize> maskME = ((1 << nvmME) - 1) << (nvmME * (phiRegion - 'A')); // ME memories
	static const ap_uint<maskTEIsize> maskTEI = ((kLAYER % 2) || (kDISK % 2) || (kLAYER == 2)) ? ((1 << nvmTEI) - 1) << (nvmTEI * (phiRegion - 'A')) : 0x0; // TE Inner memories, only used for odd layers/disk and layer 2
	static const ap_uint<maskOLsize> maskOL = ((kLAYER == 1) || (kLAYER == 2)) ? ((1 << nvmOL) - 1) << (nvmOL * (phiRegion - 'A')) : 0x0; // TE Inner Overlap memories, only used for layer 1 and 2
	static const ap_uint<maskTEOsize> maskTEO = (!((kLAYER % 2) || (kDISK % 2)) || (kLAYER == 3)|| (kDISK == 1)) ? ((1 << nvmTEO) - 1) << (nvmTEO * (phiRegion - 'A')) : 0x0; // TE Outer memories, only for even layers/disks, and layer and disk 1


	/////////////////////////
	// Main function

	VMRouter<inputType, outputType, kLAYER, kDISK,  maxASCopies, maxTEICopies, maxOLCopies, maxTEOCopies, nbitsbin, bendCutTableSize>
	(bx, fineBinTable, phiCorrTable,
		rzBitsInnerTable, rzBitsOverlapTable, rzBitsOuterTable,
		bendCutInnerTable, bendCutOverlapTable, bendCutOuterTable,
		// Input memories
		maskIS, inputStub, nullptr,
		// AllStub memories
		memoriesAS,
		// ME memories
		maskME, memoriesME,
		// TEInner memories
		maskTEI, memoriesTEI,
		// TEInner Overlap memories
		maskOL, memoriesOL,
		// TEOuter memories
		maskTEO, memoriesTEO
		);

	return;
}
