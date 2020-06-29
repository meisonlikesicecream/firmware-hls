#ifndef TrackletAlgorithm_VMRouterTop_h
#define TrackletAlgorithm_VMRouterTop_h

#include "VMRouter.h"

//////////////////////////////////
// Variables for that are specified with regards to the VMR region

constexpr int layer(1); // Which barrel layer number the data is coming from, 0 if not barrel
constexpr int disk(0); // Which disk number the data is coming from, 0 if not disk

// Number of VMs. Could use constants from VMRouter.h...
constexpr int numInputs(4); // Input memories
constexpr int numME(4); // ME memories
constexpr int numTEI(4); // TE Inner memories
constexpr int numOL(2); // TE Inner Overlap memories

// Maximum number of memory "copies" for this Phi region
constexpr int maxAllCopies(6); // Allstub memory
constexpr int maxTEICopies(5); // TE Inner memories
constexpr int maxOLCopies(3); // TE Inner Overlap memories
constexpr int maxTEOCopies(1); // Can't use 0 even if we don't have any TE Outer memories

// Which modules the input and output consist of
constexpr regionType inputType = BARRELPS; // Could be determined from the layer/disk
constexpr regionType outputType = BARRELPS;

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
