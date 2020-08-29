#include "VMRouterTop.h"

// VMRouter Top Function for layer 1, AllStub region E
// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).

// NOTE: to run a different phi region, change the following
//          - constants specified in VMRouterTop.h
//          - the directories to the LUTs
//          - the call to VMRouter() in VMRouterTop.cc
//          - pragmas?
//          - the included top function in VMRouter_test.cpp (if file name is changed)
//          - the top function in script_VMR.tcl (if file name is changed)


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


	///////////////////////////
	// Open Lookup tables
	// NOTE: needs to be changed manually if run for a different phi region

	// LUT with the corrected r/z. It is corrected for the average r (z) of the barrel (disk).
	// Includes both coarse r/z position (bin), and finer region each r/z bin is divided into.
	// Indexed using r and z position bits
	static const int finebintable[] =
#include "../emData/VMR/tables/VMR_L1PHIE_finebin.tab"


	// LUT with phi corrections to project the stub to the average radius in a layer.
	// Only used by layers.
	// Indexed using phi and bend bits
	static const int phicorrtable[] =
#include "../emData/VMR/tables/VMPhiCorrL1.txt"


	// LUT with the Z/R bits for TE memories
	// Contain information about where in z to look for valid stub pairs
	// Indexed using z and r position bits
	static const int rzbitstable[] =
#include "../emData/VMR/tables/VMTableInnerL1L2.tab"


	// LUT with the Z/R bits for TE Overlap memories
	// Only used for layer 1 and 2, and disk 1
	// Indexed using z and r position bits
	static const int rzbitsextratable[] =// 10 bits used for LUT
#include "../emData/VMR/tables/VMTableInnerL1D1.tab"


	// LUT with bend-cuts for the TE memories
	// The cuts are different depending on the memory version (nX)
	// Indexed using bend bits

	// TE Memory 1
	ap_uint<1> tmptable1_n1[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE17n1_vmbendcut.tab"

	ap_uint<1> tmptable1_n2[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE17n2_vmbendcut.tab"

	ap_uint<1> tmptable1_n3[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE17n3_vmbendcut.tab"

	ap_uint<1> tmptable1_n4[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE17n4_vmbendcut.tab"

	ap_uint<1> tmptable1_n5[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE17n5_vmbendcut.tab"

	// TE Memory 2
	ap_uint<1> tmptable2_n1[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE18n1_vmbendcut.tab"

	ap_uint<1> tmptable2_n2[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE18n2_vmbendcut.tab"

	ap_uint<1> tmptable2_n3[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE18n3_vmbendcut.tab"

	ap_uint<1> tmptable2_n4[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE18n4_vmbendcut.tab"

	ap_uint<1> tmptable2_n5[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE18n5_vmbendcut.tab"

	// TE Memory 3
	ap_uint<1> tmptable3_n1[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE19n1_vmbendcut.tab"

	ap_uint<1> tmptable3_n2[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE19n2_vmbendcut.tab"

	ap_uint<1> tmptable3_n3[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE19n3_vmbendcut.tab"

	ap_uint<1> tmptable3_n4[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE19n4_vmbendcut.tab"

	ap_uint<1> tmptable3_n5[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE19n5_vmbendcut.tab"

// TE Memory 4
	ap_uint<1> tmptable4_n1[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE20n1_vmbendcut.tab"

	ap_uint<1> tmptable4_n2[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE20n2_vmbendcut.tab"

	ap_uint<1> tmptable4_n3[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE20n3_vmbendcut.tab"

	ap_uint<1> tmptable4_n4[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE20n4_vmbendcut.tab"

	ap_uint<1> tmptable4_n5[] =
#include "../emData/VMR/tables/VMSTE_L1PHIE20n5_vmbendcut.tab"

	// Combine all the temporary tables into one big table
	static const ap_uint<bendtablesize> bendtable[] = {
		arrayToInt<bendtablesize>(tmptable1_n1), arrayToInt<bendtablesize>(tmptable1_n2), arrayToInt<bendtablesize>(tmptable1_n3), arrayToInt<bendtablesize>(tmptable1_n4), arrayToInt<bendtablesize>(tmptable1_n5),
		arrayToInt<bendtablesize>(tmptable2_n1), arrayToInt<bendtablesize>(tmptable2_n2), arrayToInt<bendtablesize>(tmptable2_n3), arrayToInt<bendtablesize>(tmptable2_n4), arrayToInt<bendtablesize>(tmptable2_n5),
		arrayToInt<bendtablesize>(tmptable3_n1), arrayToInt<bendtablesize>(tmptable3_n2), arrayToInt<bendtablesize>(tmptable3_n3), arrayToInt<bendtablesize>(tmptable3_n4), arrayToInt<bendtablesize>(tmptable3_n5),
		arrayToInt<bendtablesize>(tmptable4_n1), arrayToInt<bendtablesize>(tmptable4_n2), arrayToInt<bendtablesize>(tmptable4_n3), arrayToInt<bendtablesize>(tmptable4_n4), arrayToInt<bendtablesize>(tmptable4_n5)};


	// TE Overlap Memory 1
	ap_uint<1> tmpextratable1_n1[] =
#include "../emData/VMR/tables/VMSTE_L1PHIQ9n1_vmbendcut.tab"

	ap_uint<1> tmpextratable1_n2[] =
#include "../emData/VMR/tables/VMSTE_L1PHIQ9n2_vmbendcut.tab"

	ap_uint<1> tmpextratable1_n3[] =
#include "../emData/VMR/tables/VMSTE_L1PHIQ9n3_vmbendcut.tab"

	// TE Overlap Memory 2
	ap_uint<1> tmpextratable2_n1[] =
#include "../emData/VMR/tables/VMSTE_L1PHIQ10n1_vmbendcut.tab"

	ap_uint<1> tmpextratable2_n2[] =
#include "../emData/VMR/tables/VMSTE_L1PHIQ10n2_vmbendcut.tab"

	ap_uint<1> tmpextratable2_n3[] =
#include "../emData/VMR/tables/VMSTE_L1PHIQ10n3_vmbendcut.tab"

	// Combine all the temporary extra tables into one big table
	static const ap_uint<bendtablesize> bendextratable[] = {
		arrayToInt<bendtablesize>(tmpextratable1_n1), arrayToInt<bendtablesize>(tmpextratable1_n2), arrayToInt<bendtablesize>(tmpextratable1_n3),
		arrayToInt<bendtablesize>(tmpextratable2_n1), arrayToInt<bendtablesize>(tmpextratable2_n2), arrayToInt<bendtablesize>(tmpextratable2_n3)};

// Takes 2 clock cycles before on gets data, used at high frequencies
#pragma HLS resource variable=inputStub[0].get_mem() latency=2
#pragma HLS resource variable=inputStub[1].get_mem() latency=2
#pragma HLS resource variable=inputStub[2].get_mem() latency=2
#pragma HLS resource variable=inputStub[3].get_mem() latency=2

#pragma HLS resource variable=finebintable latency=2
#pragma HLS resource variable=rzbitstable latency=2
#pragma HLS resource variable=rzbitsextratable latency=2
// #pragma HLS resource variable=bendtable latency=2
// #pragma HLS resource variable=bendextratable latency=2
// phicorrtable and bendtable seems to be using LUTs as they relatively small?


	//////////////////////////////////
	// Create memory masks

	constexpr int nvmme = (kLAYER) ? nvmmelayers[kLAYER-1] : nvmmedisks[kDISK-1]; // ME memories
	constexpr int nvmte = (kLAYER) ? nvmtelayers[kLAYER-1] : nvmtedisks[kDISK-1]; // TE memories
	constexpr int nvmol = ((kLAYER == 1) || (kLAYER == 2)) ? nvmteoverlaplayers[kLAYER-1] : 0; // TE Overlap memories

	// Masks of which memories that are being used. The first memory is represented by the LSB
	// and a "1" implies that the specified memory is used for this phi region
	// Create "nvm" 1s, e.g. "1111", shift the mask until it corresponds to the correct phi region
	static const ap_uint<inmasksize> inmask = ((1 << numInputs) - 1); // Input memories
	static const ap_uint<memasksize> memask = ((1 << nvmme) - 1) << (nvmme * (phiRegion - 'A')); // ME memories, won't synthesise if I use createMask?!
	static const ap_uint<teimasksize> teimask = ((kLAYER % 2) || (kDISK % 2)) ? ((1 << nvmte) - 1) << (nvmte * (phiRegion - 'A')) : 0x0; // TE Inner memories, only used for odd layers/disk
	static const ap_uint<olmasksize> olmask = (nvmol) ? ((1 << nvmol) - 1) << (nvmol * (phiRegion - 'A')) : 0x0; // TE Inner Overlap memories, only used for layer 1 and 2
	static const ap_uint<teomasksize> teomask = (!((kLAYER % 2) || (kDISK % 2)) || (kDISK == 1)) ? ((1 << nvmte) - 1) << (nvmte * (phiRegion - 'A')) : 0x0; // TE Outer memories, only for even layers/disks and disk 1


	/////////////////////////
	// Main function

	VMRouter<inputType, outputType, kLAYER, kDISK,  maxAllCopies, maxTEICopies, maxOLCopies, maxTEOCopies, nbitsbin, bendtablesize>
	(bx, finebintable, phicorrtable,
		rzbitstable, rzbitsextratable, nullptr,
		bendtable, bendextratable, nullptr,
// Input memories
		inmask, inputStub, nullptr,
// AllStub memories
		allStub,
// ME memories
		memask, meMemories,
// TEInner memories
		teimask, teiMemories,
// TEInner Overlap memories
		olmask, olMemories,
// TEOuter memories
		teomask, nullptr
		);

	return;
}
