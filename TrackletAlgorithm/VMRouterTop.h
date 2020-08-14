#ifndef TrackletAlgorithm_VMRouterTop_h
#define TrackletAlgorithm_VMRouterTop_h

#include "VMRouter.h"

// VMRouter Top Function for layer 1, AllStub region E

////////////////////////////////////////////
// Variables for that are specified with regards to the VMR region
// NOTE: that one also needs to change if one has TEInner/Outer and Overlap in
// the Top Function as well as the LUT directories in the .cc Top function

#define kLAYER 1 // Which barrel layer number the data is coming from, 0 if not barrel
#define kDISK 0 // Which disk number the data is coming from, 0 if not disk

constexpr int phiRegion = 'E'; // Which AllStub/PhiRegion

// Maximum number of memory "copies" for this Phi region
constexpr int maxAllCopies(6); // Allstub memory
constexpr int maxTEICopies(5); // TE Inner memories
constexpr int maxOLCopies(3); // TE Inner Overlap memories
constexpr int maxTEOCopies(1); // Can't use 0 even if we don't have any TE Outer memories

constexpr int bendtablesize(8); // Number of entries in each bendcut table

constexpr int numInputs(4); // Number of input memories (excluding Disk2S modules)

///////////////////////////////////////////////
// Variables that don't need manual changing

// Number of VMs
constexpr int numME = (kLAYER) ? nvmmelayers[kLAYER-1] : nvmmedisks[kDISK-1]; // ME memories
constexpr int numTEI = (kLAYER) ? nvmtelayers[kLAYER-1] : nvmtedisks[kDISK-1]; // TE Inner memories
constexpr int numOL = (kLAYER) ? nvmteoverlaplayers[kLAYER-1] : 0; // TE Inner Overlap memories

// Number of bits used for the bins in VMStubeME memories
constexpr int nbitsbin = (kLAYER) ? 3 : 4;

// Which modules the input and output consist of
#if kDISK > 0
	constexpr int numInputs2S(2); // Number of 2S inputs
	constexpr regionType inputType = DISKPS;
	constexpr regionType inputType2S = DISK2S;
	constexpr regionType outputType = DISK;

#elif kLAYER > 3
	constexpr regionType inputType = BARREL2S;
	constexpr regionType outputType = BARREL2S;

#else
	constexpr regionType inputType = BARRELPS;
	constexpr regionType outputType = BARRELPS;
#endif


/////////////////////////////////////////////////////
// VMRouter Top Function

void VMRouterTop(BXType bx,
	// Input memories
	const InputStubMemory<inputType> inputStub[numInputs],
#if kDISK > 0
	const InputStubMemory<inputType2S> inputStub2S[numInputs2S], // Only disks has 2S modules
#endif

	// Output memories
#if kLAYER == 1 || kLAYER == 3 || kLAYER == 5 || kDISK == 1 || kDISK ==3
	VMStubTEInnerMemory<outputType> teiMemories[numTEI][maxTEICopies],
#endif

#if kLAYER == 2 || kLAYER == 4 || kLAYER == 6 || kDISK == 1 || kDISK == 3 || kDISK == 4
	VMStubTEOuterMemory<outputType> teoMemories[numTEO][maxTEOCopies],
#endif

#if kLAYER == 1 || kLAYER == 2
	VMStubTEInnerMemory<BARRELOL> olMemories[numOL][maxOLCopies],
#endif

	AllStubMemory<outputType> allStub[maxAllCopies],
	VMStubMEMemory<outputType, nbitsbin> meMemories[numME]
	);



///////////////////////////////////////////////////////
// Help functions

// Converts an array of 0s and 1s to an ap_uint
template<int arraysize>
inline ap_uint<arraysize> arrayToInt(ap_uint<1> array[arraysize]);

// Creates (memory) masks for a specific phi region with "nvm" VMs
template<int masksize>
inline ap_uint<masksize> createMask(int phi, int nvm);

#endif // TrackletAlgorithm_VMRouterTop_h
