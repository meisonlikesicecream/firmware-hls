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
	VMStubMEMemory<outputType, nbitsbin> memoriesME[numME],
	VMStubTEInnerMemory<BARRELOL> memoriesOL[numOL][maxOLCopies],
	VMStubTEOuterMemory<outputType> memoriesTEO[numTEI][maxTEOCopies])
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
	static const int rzbitstable[] =
#include "../emData/VMR/tables/VMTableOuterL2.tab"


	// LUT with the Z/R bits for TE Overlap memories
	// Only used for layer 1 and 2, and disk 1
	// Indexed using z and r position bits
	static const int rzbitsextratable[] = // 11 bits used for LUT
#include "../emData/VMR/tables/VMTableInnerL2D1.tab"


	// LUT with bend-cuts for the TE memories
	// The cuts are different depending on the memory version (nX)
	// Indexed using bend bits

	// TE Memory 1
	ap_uint<1> tmpBendTable1_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHID25n1_vmbendcut.tab"

	ap_uint<1> tmpBendTable1_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHID25n2_vmbendcut.tab"

	ap_uint<1> tmpBendTable1_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHID25n3_vmbendcut.tab"

	ap_uint<1> tmpBendTable1_n4[] =
#include "../emData/VMR/tables/VMSTE_L2PHID25n4_vmbendcut.tab"

	ap_uint<1> tmpBendTable1_n5[] =
#include "../emData/VMR/tables/VMSTE_L2PHID25n5_vmbendcut.tab"

	// TE Memory 2
	ap_uint<1> tmpBendTable2_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHID26n1_vmbendcut.tab"

	ap_uint<1> tmpBendTable2_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHID26n2_vmbendcut.tab"

	ap_uint<1> tmpBendTable2_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHID26n3_vmbendcut.tab"

	ap_uint<1> tmpBendTable2_n4[] =
#include "../emData/VMR/tables/VMSTE_L2PHID26n4_vmbendcut.tab"

	ap_uint<1> tmpBendTable2_n5[] =
#include "../emData/VMR/tables/VMSTE_L2PHID26n5_vmbendcut.tab"

	// TE Memory 3
	ap_uint<1> tmpBendTable3_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHID27n1_vmbendcut.tab"

	ap_uint<1> tmpBendTable3_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHID27n2_vmbendcut.tab"

	ap_uint<1> tmpBendTable3_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHID27n3_vmbendcut.tab"

	ap_uint<1> tmpBendTable3_n4[] =
#include "../emData/VMR/tables/VMSTE_L2PHID27n4_vmbendcut.tab"

	ap_uint<1> tmpBendTable3_n5[] =
#include "../emData/VMR/tables/VMSTE_L2PHID27n5_vmbendcut.tab"

// TE Memory 4
	ap_uint<1> tmpBendTable4_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHID28n1_vmbendcut.tab"

	ap_uint<1> tmpBendTable4_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHID28n2_vmbendcut.tab"

	ap_uint<1> tmpBendTable4_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHID28n3_vmbendcut.tab"

	ap_uint<1> tmpBendTable4_n4[] =
#include "../emData/VMR/tables/VMSTE_L2PHID28n4_vmbendcut.tab"

	ap_uint<1> tmpBendTable4_n5[] =
#include "../emData/VMR/tables/VMSTE_L2PHID28n5_vmbendcut.tab"

// TE Memory 5
ap_uint<1> tmpBendTable5_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHID29n1_vmbendcut.tab"

ap_uint<1> tmpBendTable5_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHID29n2_vmbendcut.tab"

ap_uint<1> tmpBendTable5_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHID29n3_vmbendcut.tab"

ap_uint<1> tmpBendTable5_n4[] =
#include "../emData/VMR/tables/VMSTE_L2PHID29n4_vmbendcut.tab"

ap_uint<1> tmpBendTable5_n5[] =
#include "../emData/VMR/tables/VMSTE_L2PHID29n5_vmbendcut.tab"

// TE Memory 6
ap_uint<1> tmpBendTable6_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHID30n1_vmbendcut.tab"

ap_uint<1> tmpBendTable6_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHID30n2_vmbendcut.tab"

ap_uint<1> tmpBendTable6_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHID30n3_vmbendcut.tab"

ap_uint<1> tmpBendTable6_n4[] =
#include "../emData/VMR/tables/VMSTE_L2PHID30n4_vmbendcut.tab"

ap_uint<1> tmpBendTable6_n5[] =
#include "../emData/VMR/tables/VMSTE_L2PHID30n5_vmbendcut.tab"

// TE Memory 7
ap_uint<1> tmpBendTable7_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHID31n1_vmbendcut.tab"

ap_uint<1> tmpBendTable7_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHID31n2_vmbendcut.tab"

ap_uint<1> tmpBendTable7_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHID31n3_vmbendcut.tab"

ap_uint<1> tmpBendTable7_n4[] =
#include "../emData/VMR/tables/VMSTE_L2PHID31n4_vmbendcut.tab"

ap_uint<1> tmpBendTable7_n5[bendCutTableSize] = {0};

// TE Memory 8
ap_uint<1> tmpBendTable8_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHID32n1_vmbendcut.tab"

ap_uint<1> tmpBendTable8_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHID32n2_vmbendcut.tab"

ap_uint<1> tmpBendTable8_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHID32n3_vmbendcut.tab"

ap_uint<1> tmpBendTable8_n4[bendCutTableSize] = {0};

ap_uint<1> tmpBendTable8_n5[bendCutTableSize] = {0};


	// Combine all the temporary tables into one big table
	static const ap_uint<bendCutTableSize> bendCutTable[] = {
		arrayToInt<bendCutTableSize>(tmpBendTable1_n1), arrayToInt<bendCutTableSize>(tmpBendTable1_n2), arrayToInt<bendCutTableSize>(tmpBendTable1_n3), arrayToInt<bendCutTableSize>(tmpBendTable1_n4), arrayToInt<bendCutTableSize>(tmpBendTable1_n5),
		arrayToInt<bendCutTableSize>(tmpBendTable2_n1), arrayToInt<bendCutTableSize>(tmpBendTable2_n2), arrayToInt<bendCutTableSize>(tmpBendTable2_n3), arrayToInt<bendCutTableSize>(tmpBendTable2_n4), arrayToInt<bendCutTableSize>(tmpBendTable2_n5),
		arrayToInt<bendCutTableSize>(tmpBendTable3_n1), arrayToInt<bendCutTableSize>(tmpBendTable3_n2), arrayToInt<bendCutTableSize>(tmpBendTable3_n3), arrayToInt<bendCutTableSize>(tmpBendTable3_n4), arrayToInt<bendCutTableSize>(tmpBendTable3_n5),
		arrayToInt<bendCutTableSize>(tmpBendTable4_n1), arrayToInt<bendCutTableSize>(tmpBendTable4_n2), arrayToInt<bendCutTableSize>(tmpBendTable4_n3), arrayToInt<bendCutTableSize>(tmpBendTable4_n4), arrayToInt<bendCutTableSize>(tmpBendTable4_n5),
		arrayToInt<bendCutTableSize>(tmpBendTable5_n1), arrayToInt<bendCutTableSize>(tmpBendTable5_n2), arrayToInt<bendCutTableSize>(tmpBendTable5_n3), arrayToInt<bendCutTableSize>(tmpBendTable5_n4), arrayToInt<bendCutTableSize>(tmpBendTable5_n5),
		arrayToInt<bendCutTableSize>(tmpBendTable6_n1), arrayToInt<bendCutTableSize>(tmpBendTable6_n2), arrayToInt<bendCutTableSize>(tmpBendTable6_n3), arrayToInt<bendCutTableSize>(tmpBendTable6_n4), arrayToInt<bendCutTableSize>(tmpBendTable6_n5),
		arrayToInt<bendCutTableSize>(tmpBendTable7_n1), arrayToInt<bendCutTableSize>(tmpBendTable7_n2), arrayToInt<bendCutTableSize>(tmpBendTable7_n3), arrayToInt<bendCutTableSize>(tmpBendTable7_n4), arrayToInt<bendCutTableSize>(tmpBendTable7_n5),
		arrayToInt<bendCutTableSize>(tmpBendTable8_n1), arrayToInt<bendCutTableSize>(tmpBendTable8_n2), arrayToInt<bendCutTableSize>(tmpBendTable8_n3), arrayToInt<bendCutTableSize>(tmpBendTable8_n4), arrayToInt<bendCutTableSize>(tmpBendTable8_n5)};


	// TE Overlap Memory 1
	ap_uint<1> tmpBendExtraTable1_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW7n1_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable1_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW7n2_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable1_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW7n3_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable1_n4[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW7n4_vmbendcut.tab"

	// TE Overlap Memory 2
	ap_uint<1> tmpBendExtraTable2_n1[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW8n1_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable2_n2[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW8n2_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable2_n3[] =
#include "../emData/VMR/tables/VMSTE_L2PHIW8n3_vmbendcut.tab"

	ap_uint<1> tmpBendExtraTable2_n4[bendCutTableSize] = {0};

	// Combine all the temporary extra tables into one big table
	static const ap_uint<bendCutTableSize> bendCutExtraTable[] = {
		arrayToInt<bendCutTableSize>(tmpBendExtraTable1_n1), arrayToInt<bendCutTableSize>(tmpBendExtraTable1_n2), arrayToInt<bendCutTableSize>(tmpBendExtraTable1_n3), arrayToInt<bendCutTableSize>(tmpBendExtraTable1_n4),
		arrayToInt<bendCutTableSize>(tmpBendExtraTable2_n1), arrayToInt<bendCutTableSize>(tmpBendExtraTable2_n2), arrayToInt<bendCutTableSize>(tmpBendExtraTable2_n3), arrayToInt<bendCutTableSize>(tmpBendExtraTable2_n4)};

// Takes 2 clock cycles before on gets data, used at high frequencies
#pragma HLS resource variable=inputStub[0].get_mem() latency=2
#pragma HLS resource variable=inputStub[1].get_mem() latency=2

#pragma HLS resource variable=fineBinTable latency=2
#pragma HLS resource variable=rzbitstable latency=2
#pragma HLS resource variable=rzbitsextratable latency=2
// #pragma HLS resource variable=bendCutTable latency=2
// #pragma HLS resource variable=bendCutExtraTable latency=2
// phiCorrTable and bendCutTable seems to be using LUTs as they relatively small?


	//////////////////////////////////
	// Create memory masks

	constexpr int nvmME = (kLAYER) ? nvmmelayers[kLAYER-1] : nvmmedisks[kDISK-1]; // ME memories
	constexpr int nvmTE = (kLAYER) ? nvmtelayers[kLAYER-1] : nvmtedisks[kDISK-1]; // TE memories
	constexpr int nvmOL = ((kLAYER == 1) || (kLAYER == 2)) ? nvmollayers[kLAYER-1] : 0; // TE Overlap memories

	// Masks of which memories that are being used. The first memory is represented by the LSB
	// and a "1" implies that the specified memory is used for this phi region
	// Create "nvm" 1s, e.g. "1111", shift the mask until it corresponds to the correct phi region
	static const ap_uint<maskISsize> maskIS = ((1 << numInputs) - 1); // Input memories
	static const ap_uint<maskMEsize> maskME = ((1 << nvmME) - 1) << (nvmME * (phiRegion - 'A')); // ME memories
	static const ap_uint<maskTEIsize> maskTEI = ((kLAYER % 2) || (kDISK % 2)) ? ((1 << nvmTE) - 1) << (nvmTE * (phiRegion - 'A')) : 0x0; // TE Inner memories, only used for odd layers/disk
	static const ap_uint<maskOLsize> maskOL = (nvmOL) ? ((1 << nvmOL) - 1) << (nvmOL * (phiRegion - 'A')) : 0x0; // TE Inner Overlap memories, only used for layer 1 and 2
	static const ap_uint<maskTEOsize> maskTEO = (!((kLAYER % 2) || (kDISK % 2)) || (kDISK == 1)) ? ((1 << nvmTE) - 1) << (nvmTE * (phiRegion - 'A')) : 0x0; // TE Outer memories, only for even layers/disks and disk 1


	/////////////////////////
	// Main function

	VMRouter<inputType, outputType, kLAYER, kDISK,  maxASCopies, maxTEICopies, maxOLCopies, maxTEOCopies, nbitsbin, bendCutTableSize>
	(bx, fineBinTable, phiCorrTable,
		nullptr, rzbitsextratable, rzbitstable,
		nullptr, bendCutExtraTable, bendCutTable,
		// Input memories
		maskIS, inputStub, nullptr,
		// AllStub memories
		memoriesAS,
		// ME memories
		maskME, memoriesME,
		// TEInner memories
		maskTEI, nullptr,
		// TEInner Overlap memories
		maskOL, memoriesOL,
		// TEOuter memories
		maskTEO, memoriesTEO
		);

	return;
}
