#ifndef TrackletAlgorithm_VMRouterTop_D1PHIA_h
#define TrackletAlgorithm_VMRouterTop_D1PHIA_h

#include "VMRouter.h"

// VMRouter Top Function for Disk 1, AllStub region A

//////////////////////////////////
// Variables for that are specified with regards to the VMR region

constexpr int layer(0); // Which barrel layer number the data is coming from, 0 if not barrel
constexpr int disk(1); // Which disk number the data is coming from, 0 if not disk

// Which modules the input and output consist of
constexpr regionType inputType = DISKPS; // Could be determined from the layer/disk
constexpr regionType inputType2 = DISK2S;
constexpr regionType outputType = DISK;

// Maximum number of memory "copies" for this Phi region
constexpr int maxAllCopies(6); // Allstub memory
constexpr int maxTEICopies(3); // TE Inner memories
constexpr int maxOLCopies(1); // Can't use 0 even if we don't have any TE Inner Overlap memories
constexpr int maxTEOCopies(5); // TE Outer memories

// Number of VMs.
constexpr int numInputs(6); // Input memories
constexpr int numInputsDiskPS(4);
constexpr int numInputsDisk2S(numInputs-numInputsDiskPS); // inputType2 inputs

constexpr int numME = (layer) ? nvmmelayers[layer-1] : nvmmedisks[disk-1]; // ME memories
constexpr int numTEI = (layer) ? nvmtelayers[layer-1] : nvmtedisks[disk-1]; // TE Inner memories
constexpr int numOL = (layer) ? nvmteoverlaplayers[layer-1] : 0; // TE Inner Overlap memories
constexpr int numTEO = (layer) ? nvmtelayers[layer-1] : nvmtedisks[disk-1]; // TE Outer memories

// Number of bits used for the bins in VMStubeME memories
constexpr int nbitsbin = (layer) ? 3 : 4;

void VMRouterTop(BXType bx,
	// Input memories
	const InputStubMemory<inputType> inputStub[numInputsDiskPS],
	const InputStubMemory<inputType2> inputStubDisk2S[numInputsDisk2S],
	// Output memories
	AllStubMemory<outputType> allStub[maxAllCopies],
	VMStubMEMemory<outputType, nbitsbin> meMemories[numME],
	VMStubTEInnerMemory<outputType> teiMemories[numTEI][maxTEICopies],
	VMStubTEOuterMemory<outputType> teoMemories[numTEO][maxTEOCopies]
	);

#endif // TrackletAlgorithm_VMRouterTop_D1PHIA_h
