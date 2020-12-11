#include "VMRouterTop.h"

// VMRouter Top Function for layer 1, AllStub region E
// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).

// NOTE: to run a different phi region, change the following
//          - constants specified in VMRouterTop.h
//          - the input parameters to VMRouterTop in VMRouterTop.h/.cc
//          - the the number and directories to the LUTs
//          - add/remove pragmas depending on inputStub in VMRouterTop.cc
//          - the call to VMRouter() in VMRouterTop.cc
//          - the included top function in VMRouter_test.cpp (if file name is changed)
//          - the top function and memory directory in script_VMR.tcl (if file name is changed)
//          - add the phi region in emData/download.sh, make sure to also run clean

template<int N, int STOP>
void VMRouterLoop(BXType bx,
	// Input memories
	const InputStubMemory<inputType> inputStub[nPhiRegions][numInputs],

	// Output memories
	AllStubMemory<outputType> memoriesAS[nPhiRegions][maxASCopies],
	VMStubMEMemory<outputType, nbitsbin> memoriesME[nPhiRegions][nvmME],
	ap_uint<bendCutTableSize> bendCutInnerTable[nPhiRegions][nvmTEI*maxTEICopies], VMStubTEInnerMemory<outputType> memoriesTEI[nPhiRegions][nvmTEI][maxTEICopies],
	ap_uint<bendCutTableSize> bendCutOverlapTable[nPhiRegions][nvmOL*maxOLCopies], VMStubTEInnerMemory<BARRELOL> memoriesOL[nPhiRegions][nvmOL][maxOLCopies]) {
		
		///////////////////////////
		// Open Lookup tables
		// NOTE: needs to be changed manually if run for a different phi region

		// LUT with the corrected r/z. It is corrected for the average r (z) of the barrel (disk).
		// Includes both coarse r/z position (bin), and finer region each r/z bin is divided into.
		// Indexed using r and z position bits
		static const int fineBinTable[] =
	#include "../emData/VMR/tables/VMR_L1PHIE_finebin.tab"


		// LUT with phi corrections to project the stub to the average radius in a layer.
		// Only used by layers.
		// Indexed using phi and bend bits
		static const int phiCorrTable[] =
	#include "../emData/VMR/tables/VMPhiCorrL1.tab"


		// LUT with the Z/R bits for TE memories
		// Contain information about where in z to look for valid stub pairs
		// Indexed using z and r position bits

		static const int rzBitsInnerTable[] =
	#include "../emData/VMR/tables/VMTableInnerL1L2.tab" // 11 bits used for LUT

		static const int rzBitsOverlapTable[] = // 11 bits used for LUT
	#include "../emData/VMR/tables/VMTableInnerL1D1.tab"

	// 	static const int rzBitsOuterTable[] = // 11 bits used for LUT
	// #include "../emData/VMR/tables/VMTableOuterXX.tab"

	// Takes 2 clock cycles before on gets data, used at high frequencies
	//#pragma HLS resource variable=inputStub[0].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[1].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[2].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[3].get_mem() latency=2

	constexpr char phiRegion = phiRegionList[N];

	VMRouterLoop<N-1, STOP>(bx,
		// Input memories
		inputStub,
		// Output memories
		memoriesAS,
		memoriesME,
		bendCutInnerTable, memoriesTEI,
		bendCutOverlapTable, memoriesOL);

		/////////////////////////
		// Main function

		VMRouter<inputType, outputType, phiRegion, kLAYER, kDISK,  maxASCopies, maxTEICopies, maxOLCopies, maxTEOCopies, nbitsbin, bendCutTableSize>
		(bx, fineBinTable, phiCorrTable,
			rzBitsInnerTable, rzBitsOverlapTable, nullptr,
			bendCutInnerTable[N], bendCutOverlapTable[N], nullptr,
			// Input memories
			inputStub[N], nullptr,
			// AllStub memories
			memoriesAS[N],
			// ME memories
			memoriesME[N],
			// TEInner memories
			memoriesTEI[N],
			// TEInner Overlap memories
			memoriesOL[N],
			// TEOuter memories
			nullptr
			);

}

template<>
void VMRouterLoop<0, 0>(BXType bx,
	// Input memories
	const InputStubMemory<inputType> inputStub[nPhiRegions][numInputs],

	// Output memories
	AllStubMemory<outputType> memoriesAS[nPhiRegions][maxASCopies],
	VMStubMEMemory<outputType, nbitsbin> memoriesME[nPhiRegions][nvmME],
	ap_uint<bendCutTableSize> bendCutInnerTable[nPhiRegions][nvmTEI*maxTEICopies], VMStubTEInnerMemory<outputType> memoriesTEI[nPhiRegions][nvmTEI][maxTEICopies],
	ap_uint<bendCutTableSize> bendCutOverlapTable[nPhiRegions][nvmOL*maxOLCopies], VMStubTEInnerMemory<BARRELOL> memoriesOL[nPhiRegions][nvmOL][maxOLCopies]) {
		
		///////////////////////////
		// Open Lookup tables
		// NOTE: needs to be changed manually if run for a different phi region

		// LUT with the corrected r/z. It is corrected for the average r (z) of the barrel (disk).
		// Includes both coarse r/z position (bin), and finer region each r/z bin is divided into.
		// Indexed using r and z position bits
		static const int fineBinTable[] =
	#include "../emData/VMR/tables/VMR_L1PHIE_finebin.tab"


		// LUT with phi corrections to project the stub to the average radius in a layer.
		// Only used by layers.
		// Indexed using phi and bend bits
		static const int phiCorrTable[] =
	#include "../emData/VMR/tables/VMPhiCorrL1.tab"


		// LUT with the Z/R bits for TE memories
		// Contain information about where in z to look for valid stub pairs
		// Indexed using z and r position bits

		static const int rzBitsInnerTable[] =
	#include "../emData/VMR/tables/VMTableInnerL1L2.tab" // 11 bits used for LUT

		static const int rzBitsOverlapTable[] = // 11 bits used for LUT
	#include "../emData/VMR/tables/VMTableInnerL1D1.tab"

	// 	static const int rzBitsOuterTable[] = // 11 bits used for LUT
	// #include "../emData/VMR/tables/VMTableOuterXX.tab"

	// Takes 2 clock cycles before on gets data, used at high frequencies
	//#pragma HLS resource variable=inputStub[0].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[1].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[2].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[3].get_mem() latency=2

		
		constexpr char phiRegion = phiRegionList[0];


		/////////////////////////
		// Main function

		VMRouter<inputType, outputType, phiRegion, kLAYER, kDISK,  maxASCopies, maxTEICopies, maxOLCopies, maxTEOCopies, nbitsbin, bendCutTableSize>
		(bx, fineBinTable, phiCorrTable,
			rzBitsInnerTable, rzBitsOverlapTable, nullptr,
			bendCutInnerTable[0], bendCutOverlapTable[0], nullptr,
			// Input memories
			inputStub[0], nullptr,
			// AllStub memories
			memoriesAS[0],
			// ME memories
			memoriesME[0],
			// TEInner memories
			memoriesTEI[0],
			// TEInner Overlap memories
			memoriesOL[0],
			// TEOuter memories
			nullptr
			);
	}


void SuperVMRouterTop(BXType bx,
	// Input memories
	const InputStubMemory<inputType> inputStub[nPhiRegions][numInputs],

	// Output memories
	AllStubMemory<outputType> memoriesAS[nPhiRegions][maxASCopies],
	VMStubMEMemory<outputType, nbitsbin> memoriesME[nPhiRegions][nvmME],
	ap_uint<bendCutTableSize> bendCutInnerTable[nPhiRegions][nvmTEI*maxTEICopies], VMStubTEInnerMemory<outputType> memoriesTEI[nPhiRegions][nvmTEI][maxTEICopies],
	ap_uint<bendCutTableSize> bendCutOverlapTable[nPhiRegions][nvmOL*maxOLCopies], VMStubTEInnerMemory<BARRELOL> memoriesOL[nPhiRegions][nvmOL][maxOLCopies])
 {

	#pragma HLS inline region recursive
	#pragma HLS array_partition variable=inputStub complete
	#pragma HLS array_partition variable=memoriesAS complete
	#pragma HLS array_partition variable=memoriesME complete
	#pragma HLS array_partition variable=bendCutInnerTable complete
	#pragma HLS array_partition variable=memoriesTEI complete
	#pragma HLS array_partition variable=bendCutOverlapTable complete
	#pragma HLS array_partition variable=memoriesOL complete
	
	VMRouterLoop<nPhiRegions-1, 0>(bx,
		// Input memories
		inputStub,
		// Output memories
		memoriesAS,
		memoriesME,
		bendCutInnerTable, memoriesTEI,
		bendCutOverlapTable, memoriesOL);
}
	
