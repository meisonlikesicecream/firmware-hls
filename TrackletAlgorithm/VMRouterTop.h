#ifndef TrackletAlgorithm_VMRouterTop_h
#define TrackletAlgorithm_VMRouterTop_h

#include "VMRouter.h"

// VMRouter Top Function for layer 1, AllStub region E
// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).

// NOTE: to run a different phi region, change the following
//          - the constants in VMRouterTop.h
//          - the input parameters of the VMRouterTop() function
//          - the directories to the LUTs
//          - the memory masks
//          - the call to VMRouter() in VMRouterTop.cc


//////////////////////////////////
// Variables for that are specified with regards to the phi region
// Note that one also needs to change if one has TEInner/Outer and Overlap

constexpr int layer(1); // Which barrel layer number the data is coming from, 0 if not barrel
constexpr int disk(0); // Which disk number the data is coming from, 0 if not disk

// Which modules the input and output consist of
constexpr regionType inputType = BARRELPS; // Could be determined from the layer/disk
constexpr regionType outputType = BARRELPS;

// Maximum number of memory "copies" for this Phi region
constexpr int maxAllCopies(6); // Allstub memory
constexpr int maxTEICopies(5); // TE Inner memories
constexpr int maxOLCopies(3); // TE Inner Overlap memories
constexpr int maxTEOCopies(1); // Can't use 0 even if we don't have any TE Outer memories

// Number of VMs
constexpr int numInputs(4); // Input memories
constexpr int numME = (layer) ? nvmmelayers[layer-1] : nvmmedisks[disk-1]; // ME memories
constexpr int numTEI = (layer) ? nvmtelayers[layer-1] : nvmtedisks[disk-1]; // TE Inner memories
constexpr int numOL = (layer) ? nvmteoverlaplayers[layer-1] : 0; // TE Inner Overlap memories

// Number of bits used for the bins in VMStubeME memories
constexpr int nbitsbin = (layer) ? 3 : 4;

// VMRouter Top Function for layer 1, AllStub region E
void VMRouterTop(BXType bx,
	// Input memories
	const InputStubMemory<inputType> inputStub[numInputs],
	// Output memories
	AllStubMemory<outputType> allStub[maxAllCopies],
	VMStubMEMemory<outputType, nbitsbin> meMemories[numME],
	VMStubTEInnerMemory<outputType> teiMemories[numTEI][maxTEICopies],
	VMStubTEInnerMemory<BARRELOL> olMemories[numOL][maxOLCopies]
	);

#endif // TrackletAlgorithm_VMRouterTop_h
