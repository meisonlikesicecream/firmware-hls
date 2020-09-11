// Test bench for VMRouter
#include "VMRouterTop.h"
//#include "VMRouterTop_D1PHIA.h"

#include <algorithm>
#include <iterator>

#include "FileReadUtility.h"

using namespace std;

const int nEvents = 100;  //number of events to run

// VMRouter Test that works for all regions
// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).

// NOTE: to run a different phi region, change the following
//          - the included top function (if file name is changed)
//          - the top function in script_VMR.tcl (if file name is changed)
//          - and the changes listed in VMRouterTop.cc/h


// Finds all memory names for the specified processing module found in the wiring file,
// and adds them to the fileNames array with the memory files directory, excluding the copies nX.
// The number of copies are kept track in the numCopiesArray.
// E.g. finds all memories that start with "VMSME_L1PHIE", such as "VMSME_L1PHIE17" etc.
// Returns 0 if wiring file isn't found.
template<int arraySize>
bool findFileNames(string fileDirStart, string wireFileName, string memID, string nameList[arraySize], int numCopiesArray[arraySize]) {

  ifstream wireFile; // Will contain the wiring file

  int numMemories = 0; // Number of memories found
  char delimeter = (memID.substr(0,2) == "IL") ? ' ' : 'n'; // Different delimeter if input or output memories

  bool valid = openDataFile(wireFile, wireFileName); // Open the wiring file

  // Check if the wiring file was opened properly.
  if (not valid) {
    cout << "Could not find wiring file." << endl;
    return 0;
  }

  // Loop over all lines in the wiring file
  for (string inputLine; getline(wireFile, inputLine); ) {

    // If we find the memory we are looking for
    if (inputLine.find(memID) != string::npos) {

      string tmpMemoryDir = fileDirStart + "_" + inputLine.substr(0, inputLine.find(delimeter)); // The directory and the name of the memory (first part of the line)

      auto isInNameList = find(nameList, nameList+arraySize, tmpMemoryDir); // Check if tmpMemoryDir is in nameList

      // Add the start of the memory name to the list if we haven't added it before, otherwise increment the number of copies
      if (isInNameList == nameList+arraySize) {
        nameList[numMemories] = tmpMemoryDir;
        numCopiesArray[numMemories]++;
        numMemories++;
      } else {
        numCopiesArray[distance(nameList, isInNameList)]++;
      }
    }
  }

  return 1;
}



int main() {

  ////////////////////////////////////////////////////////////////
  // Get lists of the input/output memory directory and file names
  // I.e. the names of the test vector files

  // Uses wires_hourglass.dat wiring file
  string wireFileName = "emData/wires_hourglass.dat"; // The wiring file name with directory
  string testDataDirectory = "emData/MemPrints/"; // Directory for the test data

  string layerID = (kLAYER) ? "L" + to_string(kLAYER) + "PHI" + phiRegion : "D" + to_string(kDISK) + "PHI" + phiRegion; // Which layer/disk and phi region
  string fileEnding = (sector < 10) ? "_0" + to_string(sector) + ".dat" :  "_" + to_string(sector) + ".dat"; //All files ends with .dat. "_XX" specifies which sector

  char specialPhiRegion[] = {'X', 'Y', 'Z', 'W', 'Q', 'R', 'S', 'T'}; // Special naming for the TE overlap memories, and outer memories in Disk 1

  // Input file names
  string inputNameList[numInputs];
  int inputNumCopies[numInputs] = {0}; // Array containing the number of copies of each memory

  string inputDir = "InputStubs/InputStubs"; // Directory of InputStubs, including the first part of the file name
  string inMemID = "IL_" + layerID; // Input memory ID for the specified phi region

  // Get the input file names and check that the wiring file can be opened properly
  if (not findFileNames<numInputs>(testDataDirectory + inputDir, wireFileName, inMemID, inputNameList, inputNumCopies)) return -1;


  // Start of AllStub file names, excluding the copy number
  string allstubName = testDataDirectory + "Stubs/AllStubs_AS_" + layerID;


  // Start of MEStub file names, excluding the copy number, i.e. "n1" as they only have one copy
  string nameListME[numME];
  int numCopiesME[numME] = {0}; // Array containing the number of copies of each memory

  string meDir = "VMStubsME/VMStubs"; // Directory of MEStubs, including the first part of the file name
  string meMemID  =  "VMSME_" + layerID; // ME memory ID for the specified phi region

  findFileNames<numME>(testDataDirectory + meDir, wireFileName, meMemID, nameListME, numCopiesME);


  // Start of TEInnerStub file names, excluding the copy number "nX"
  string nameListTEI[numTEI];
  int numCopiesTEI[numTEI] = {0}; // Array containing the number of copies of each memory

  if (maxTEICopies > 1) {
    string teiDir = "VMStubsTE/VMStubs"; // Directory of MEStubs, including the first part of the file name
    string teiMemID = "VMSTE_" + layerID; // TE Inner memory ID for the specified phi region

    findFileNames<numTEI>(testDataDirectory + teiDir, wireFileName, teiMemID, nameListTEI, numCopiesTEI);
  }


  // Start of TEInnerStub Overlap file names, excluding the copy number
  string nameListOL[numOL];
  int numCopiesOL[numOL] = {0}; // Array containing the number of copies of each memory

  if (maxOLCopies > 1) {
    string olDir = "VMStubsTE/VMStubs"; // Directory of MEStubs, including the first part of the file name
    string olMemID = "VMSTE_L" + to_string(kLAYER) + "PHI" + specialPhiRegion[phiRegion - 'A']; // TE Inner memory ID for the specified phi region

    findFileNames<numOL>(testDataDirectory + olDir, wireFileName, olMemID, nameListOL, numCopiesOL);
  }


  // Start of TEOuterStub file names, excluding the copy number "nX"
  string nameListTEO[numTEO];
  int numCopiesTEO[numTEO] = {0}; // Array containing the number of copies of each memory

  if (maxTEOCopies > 1) {
    string teoDir = "VMStubsTE/VMStubs"; // Directory of MEStubs, including the first part of the file name
    string teoMemID = (kDISK != 1) ? "VMSTE_" + layerID : string("VMSTE_D1PHI") + specialPhiRegion[phiRegion - 'A']; // TE Outer memory ID for the specified phi region

    findFileNames<numTEO>(testDataDirectory + teoDir, wireFileName, teoMemID, nameListTEO, numCopiesTEO);
  }


  ///////////////////////////
  // input memories
  static InputStubMemory<inputType> inputStub[numInputs];
  static InputStubMemory<DISK2S> inputStubDisk2S[numInputsDisk2S]; //Only used for Disks

  // output memories
  static AllStubMemory<outputType> memoriesAS[maxASCopies];
  // ME memories
  static VMStubMEMemory<outputType, nbitsbin> memoriesME[numME];
  // TE Inner memories, including copies
  static VMStubTEInnerMemory<outputType> memoriesTEI[numTEI][maxTEICopies];
  // TE Inner Overlap memories, including copies
  static VMStubTEInnerMemory<BARRELOL> memoriesOL[numOL][maxOLCopies];
  // TE Outer memories
  static VMStubTEOuterMemory<outputType> memoriesTEO[numTEO][maxTEOCopies];


  ///////////////////////////
  // open input files
    cout << "Open files..." << endl;

    ifstream fin_inputstub[numInputs];

    for (unsigned int i = 0; i < numInputs; i++) {
      bool valid = openDataFile(fin_inputstub[i], inputNameList[i] + fileEnding);
      if (not valid) return -1;
    }


  ///////////////////////////
  // open output files

  // AllStub
  ifstream fout_allstub[maxASCopies];

  for (unsigned int i = 0; i < maxASCopies; i++) {
    bool valid = openDataFile(fout_allstub[i], allstubName + "n" + to_string(i+1) + fileEnding);
    if (not valid) return -1;
  }

  // ME memories
  ifstream fout_vmstubme[numME];

  for (unsigned int i = 0; i < numME; i++) {
    bool valid =  openDataFile(fout_vmstubme[i], nameListME[i] + "n1" + fileEnding);
    if (not valid) return -1;
  }

  // TE Inner
	ifstream fout_vmstubtei[numTEI][maxTEICopies];

  if (maxTEICopies > 1) {
    for (unsigned int i = 0; i < numTEI; i++) {
      for (unsigned int j = 0; j < numCopiesTEI[i]; j++) {
        bool valid = openDataFile(fout_vmstubtei[i][j], nameListTEI[i] + "n" + to_string(j+1) + fileEnding);
        if (not valid) return -1;
      }
    }
  }

  // TE Inner Overlap
	ifstream fout_vmstubteol[numOL][maxOLCopies];

  if (maxOLCopies > 1) {
    for (unsigned int i = 0; i < numOL; i++) {
      for (unsigned int j = 0; j < numCopiesOL[i]; j++) {
        bool valid = openDataFile(fout_vmstubteol[i][j], nameListOL[i] + "n" + to_string(j+1) + fileEnding);
        if (not valid) return -1;
      }
    }
  }

  // TE Outer
  ifstream fout_vmstubteo[numTEO][maxTEOCopies];

  if (maxTEOCopies > 1) {
    for (unsigned int i = 0; i < numTEO; i++) {
      for (unsigned int j = 0; j < numCopiesTEO[i]; j++) {
        bool valid = openDataFile(fout_vmstubteo[i][j], nameListTEO[i] + "n" + to_string(j+1) + fileEnding);
        if (not valid) return -1;
      }
    }
  }

  // error count
  int err = 0;

  ///////////////////////////
  // loop over events
  cout << "Start event loop ..." << endl;
  for (unsigned int ievt = 0; ievt < nEvents; ++ievt) {
    cout << "Event: " << dec << ievt << endl;

    int num2S = 0; // Keeps track of how many DISK 2S modules we have written to

    // read event and write to memories
    for (unsigned int i = 0; i < numInputs; i++) {
      if (kLAYER) {
        writeMemFromFile<InputStubMemory<inputType>>(inputStub[i], fin_inputstub[i], ievt);
      } else {
        if (i == 0 || i == 3) {
          writeMemFromFile<InputStubMemory<DISK2S>>(inputStubDisk2S[num2S], fin_inputstub[i], ievt);
          num2S++;
        } else {
          writeMemFromFile<InputStubMemory<inputType>>(inputStub[i - num2S], fin_inputstub[i], ievt);
        }
      }
    }

    // bx - bunch crossing
    BXType bx = ievt;

    // Unit Under Test
    VMRouterTop(bx, inputStub
#if kDISK > 0
        , inputStubDisk2S
#endif
        , memoriesAS, memoriesME
#if kLAYER == 1 || kLAYER == 3 || kLAYER == 5 || kDISK == 1 || kDISK == 3
        , memoriesTEI
#endif
#if kLAYER == 1 || kLAYER == 2
        , memoriesOL
#endif
#if kLAYER == 2 || kLAYER == 4 || kLAYER == 6 || kDISK == 1 || kDISK == 2 || kDISK == 4
        , memoriesTEO
#endif
      );

    // compare the computed outputs with the expected ones
    // add 1 to the error count per stub that is incorrect

    bool truncation = false;

    // AllStub
    for (unsigned int i = 0; i < numInputs; i++) {
      err += compareMemWithFile<AllStubMemory<outputType>>(memoriesAS[i], fout_allstub[i], ievt, "AllStub", truncation);
    }

    // ME Memories
    for (unsigned int i = 0; i < numME; i++) {
      err += compareBinnedMemWithFile<VMStubMEMemory<outputType, nbitsbin>>(memoriesME[i], fout_vmstubme[i], ievt, "VMStubME" + to_string(i), truncation);
    }

    // TE Inner Memories
    if (maxTEICopies > 1) {
      for (unsigned int i = 0; i < numTEI; i++) {
        for (unsigned int j = 0; j < numCopiesTEI[i]; j++) {
          err += compareMemWithFile<VMStubTEInnerMemory<outputType>>(memoriesTEI[i][j], fout_vmstubtei[i][j], ievt, "VMStubTEInner" + to_string(i) + "n" + to_string(j+1), truncation);
        }
      }
    }

    // TE Inner Overlap memories
    if (maxOLCopies > 1) {
      for (unsigned int i = 0; i < numOL; i++) {
        for (unsigned int j = 0; j < numCopiesOL[i]; j++) {
          err += compareMemWithFile<VMStubTEInnerMemory<BARRELOL>>(memoriesOL[i][j], fout_vmstubteol[i][j], ievt, "VMStubTEOverlap" + to_string(i) + "n" + to_string(j+1), truncation);
        }
      }
    }

    // TE Outer memories
    if (maxTEOCopies > 1) {
      for (unsigned int i = 0; i < numTEO; i++) {
        for (unsigned int j = 0; j < numCopiesTEO[i]; j++) {
          err += compareBinnedMemWithFile<VMStubTEOuterMemory<outputType>>(memoriesTEO[i][j], fout_vmstubteo[i][j], ievt, "VMStubTEOuter" + to_string(i) + "n" + to_string(j+1), truncation);
        }
      }
    }
  } // end of event loop

	cerr << "Exiting with return value " << err << endl;
	return err;

}
