#include "VMRouterTop.h"

// VMRouter Top Function for layer 1
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


// VMRouter loop function which calls the VMR and itself but with template parameter N-1
template<int N>
void VMRouterLoop(BXType bx,
	// Lookup tables
	const ap_uint<6> fineBinTable[nPhiRegions][1<<11],
	const ap_int<10> phiCorrTable[nPhiRegions][1<<11],
	const ap_int<11> rzBitsInnerTable[nPhiRegions][1<<11],
	const ap_int<11> rzBitsOverlapTable[nPhiRegions][1<<11],
	const ap_uint<bendCutTableSize> bendCutInnerTable[nPhiRegions][nvmTEI*maxTEICopies],
	const ap_uint<bendCutTableSize> bendCutOverlapTable[nPhiRegions][nvmOL*maxOLCopies],
	
	// Input memories
	const InputStubMemory<inputType> inputStub[nPhiRegions][numInputs],

	// Output memories
	AllStubMemory<outputType> memoriesAS[nPhiRegions][maxASCopies],
	VMStubMEMemory<outputType, nbitsbin> memoriesME[nPhiRegions][nvmME],
	VMStubTEInnerMemory<outputType> memoriesTEI[nPhiRegions][nvmTEI][maxTEICopies],
	VMStubTEInnerMemory<BARRELOL> memoriesOL[nPhiRegions][nvmOL][maxOLCopies]) {


	// Takes 2 clock cycles before on gets data, used at high frequencies
	//#pragma HLS resource variable=inputStub[0].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[1].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[2].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[3].get_mem() latency=2

	constexpr char phiRegion = phiRegionList[N];

	VMRouterLoop<N-1>(bx,
		// Lookup tables
		fineBinTable,
		phiCorrTable,
		rzBitsInnerTable,
		rzBitsOverlapTable,
		bendCutInnerTable,
		bendCutOverlapTable,
		// Input memories
		inputStub,
		// Output memories
		memoriesAS,
		memoriesME,
		memoriesTEI,
		memoriesOL);

		/////////////////////////
		// Main function

		VMRouter<inputType, outputType, phiRegion, kLAYER, kDISK,  maxASCopies, maxTEICopies, maxOLCopies, maxTEOCopies, nbitsbin, bendCutTableSize>
		(bx, fineBinTable[N], phiCorrTable[N],
			rzBitsInnerTable[N], rzBitsOverlapTable[N], nullptr,
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

// The last VMRouter loop function which only calls the VMR
template<>
void VMRouterLoop<0>(BXType bx,
	// Lookup tables
	const ap_uint<6> fineBinTable[nPhiRegions][1<<11],
	const ap_int<10> phiCorrTable[nPhiRegions][1<<11],
	const ap_int<11> rzBitsInnerTable[nPhiRegions][1<<11],
	const ap_int<11> rzBitsOverlapTable[nPhiRegions][1<<11],
	const ap_uint<bendCutTableSize> bendCutInnerTable[nPhiRegions][nvmTEI*maxTEICopies],
	const ap_uint<bendCutTableSize> bendCutOverlapTable[nPhiRegions][nvmOL*maxOLCopies],
	
	// Input memories
	const InputStubMemory<inputType> inputStub[nPhiRegions][numInputs],

	// Output memories
	AllStubMemory<outputType> memoriesAS[nPhiRegions][maxASCopies],
	VMStubMEMemory<outputType, nbitsbin> memoriesME[nPhiRegions][nvmME],
	VMStubTEInnerMemory<outputType> memoriesTEI[nPhiRegions][nvmTEI][maxTEICopies],
	VMStubTEInnerMemory<BARRELOL> memoriesOL[nPhiRegions][nvmOL][maxOLCopies]) {

	// Takes 2 clock cycles before on gets data, used at high frequencies
	//#pragma HLS resource variable=inputStub[0].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[1].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[2].get_mem() latency=2
	//#pragma HLS resource variable=inputStub[3].get_mem() latency=2

		
		constexpr char phiRegion = phiRegionList[0];


		/////////////////////////
		// Main function

		VMRouter<inputType, outputType, phiRegion, kLAYER, kDISK,  maxASCopies, maxTEICopies, maxOLCopies, maxTEOCopies, nbitsbin, bendCutTableSize>
		(bx, fineBinTable[0], phiCorrTable[0],
			rzBitsInnerTable[0], rzBitsOverlapTable[0], nullptr,
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

// Super VMR Top function
// Calls VMRouterLoop recursively
void SuperVMRouterTop(BXType bx,
	// Lookup tables
	const ap_uint<6> fineBinTable[nPhiRegions][1<<11],
	const ap_int<10> phiCorrTable[nPhiRegions][1<<11],
	const ap_int<11> rzBitsInnerTable[nPhiRegions][1<<11],
	const ap_int<11> rzBitsOverlapTable[nPhiRegions][1<<11],
	
	// Input memories
	const InputStubMemory<inputType> inputStub[nPhiRegions][numInputs],

	// Output memories
	AllStubMemory<outputType> memoriesAS[nPhiRegions][maxASCopies],
	VMStubMEMemory<outputType, nbitsbin> memoriesME[nPhiRegions][nvmME],
	const ap_uint<bendCutTableSize> bendCutInnerTable[nPhiRegions][nvmTEI*maxTEICopies], VMStubTEInnerMemory<outputType> memoriesTEI[nPhiRegions][nvmTEI][maxTEICopies],
	const ap_uint<bendCutTableSize> bendCutOverlapTable[nPhiRegions][nvmOL*maxOLCopies], VMStubTEInnerMemory<BARRELOL> memoriesOL[nPhiRegions][nvmOL][maxOLCopies])
 {

	#pragma HLS inline region recursive
	#pragma HLS array_partition variable=inputStub complete
	#pragma HLS array_partition variable=memoriesAS complete
	#pragma HLS array_partition variable=memoriesME complete
	#pragma HLS array_partition variable=bendCutInnerTable complete
	#pragma HLS array_partition variable=memoriesTEI complete
	#pragma HLS array_partition variable=bendCutOverlapTable complete
	#pragma HLS array_partition variable=memoriesOL complete
	#pragma HLS array_partition variable=phiCorrTable
	#pragma HLS array_partition variable=fineBinTable
	#pragma HLS array_partition variable=rzBitsInnerTable
	#pragma HLS array_partition variable=rzBitsOverlapTable
	
	VMRouterLoop<nPhiRegions-1>(bx,
		// Lookup tables
		fineBinTable,
		phiCorrTable,
		rzBitsInnerTable,
		rzBitsOverlapTable,
		bendCutInnerTable,
		bendCutOverlapTable,
		// Input memories
		inputStub,
		// Output memories
		memoriesAS,
		memoriesME,
		memoriesTEI,
		memoriesOL
	);
}
	
