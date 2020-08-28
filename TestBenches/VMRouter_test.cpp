// Test bench for VMRouter
#include "VMRouterTop.h"

#include <algorithm>
#include <iterator>

#include "FileReadUtility.h"

using namespace std;

const int nevents = 100;  //number of events to run

// VMRouter Test that works for all regions
// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).

// NOTE: to run a different phi region, change the following
//          - the included top function (if file name is changed)
//          - and the changes listed in VMRouterTop.cc/h


// Finds all memory names for the specified processing module found in the wiring file,
// and adds them to the fileNames array with the memory files directory, excluding the copies nX.
// The number of copies are kept track in the numCopiesArray.
// E.g. finds all memories that start with "VMSME_L1PHIE", such as "VMSME_L1PHIE17" etc.
template<int arraySize>
bool getFileNames(string fileDirStart, string wireFileName, string memID, string nameList[arraySize], int numCopiesArray[arraySize]) {

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
  string inNameList[numInputs];
  int inNumCopies[numInputs] = {0}; // Array containing the number of copies of each memory

  string inDir = "InputStubs/InputStubs"; // Directory of InputStubs, including the first part of the file name
  string inMemID = "IL_" + layerID; // Input memory ID for the specified phi region

  // Get the input file names and check that the wiring file can be opened properly
  if (not getFileNames<numInputs>(testDataDirectory + inDir, wireFileName, inMemID, inNameList, inNumCopies)) return -1;


  // Start of AllStub file names, excluding the copy number
  string allStubName = testDataDirectory + "Stubs/AllStubs_AS_" + layerID;


  // Start of MEStub file names, excluding the copy number, i.e. "n1" as they only have one copy
  string meNameList[numME];
  int meNumCopies[numME] = {0}; // Array containing the number of copies of each memory

  string meDir = "VMStubsME/VMStubs"; // Directory of MEStubs, including the first part of the file name
  string meMemID  =  "VMSME_" + layerID; // ME memory ID for the specified phi region

  getFileNames<numME>(testDataDirectory + meDir, wireFileName, meMemID, meNameList, meNumCopies);


  // Start of TEInnerStub file names, excluding the copy number "nX"
  string teiNameList[numTEI];
  int teiNumCopies[numTEI] = {0}; // Array containing the number of copies of each memory

  if (maxTEICopies > 1) {
    string teiDir = "VMStubsTE/VMStubs"; // Directory of MEStubs, including the first part of the file name
    string teiMemID = "VMSTE_" + layerID; // TE Inner memory ID for the specified phi region

    getFileNames<numTEI>(testDataDirectory + teiDir, wireFileName, teiMemID, teiNameList, teiNumCopies);
  }


  // Start of TEInnerStub Overlap file names, excluding the copy number
  string olNameList[numOL];
  int olNumCopies[numOL] = {0}; // Array containing the number of copies of each memory

  if (maxOLCopies > 1) {
    string olDir = "VMStubsTE/VMStubs"; // Directory of MEStubs, including the first part of the file name
    string olMemID = "VMSTE_L" + to_string(kLAYER) + "PHI" + specialPhiRegion[phiRegion - 'A']; // TE Inner memory ID for the specified phi region

    getFileNames<numOL>(testDataDirectory + olDir, wireFileName, olMemID, olNameList, olNumCopies);
  }


  // Start of TEOuterStub file names, excluding the copy number "nX"
  string teoNameList[numTEO];
  int teoNumCopies[numTEO] = {0}; // Array containing the number of copies of each memory

  if (maxTEOCopies > 1) {
    string teoDir = "VMStubsTE/VMStubs"; // Directory of MEStubs, including the first part of the file name
    string teoMemID = (kDISK != 1) ? "VMSTE_" + layerID : string("VMSTE_D1PHI") + specialPhiRegion[phiRegion - 'A']; // TE Outer memory ID for the specified phi region

    getFileNames<numTEO>(testDataDirectory + teoDir, wireFileName, teoMemID, teoNameList, teoNumCopies);
  }


  ///////////////////////////
  // input memories
  static InputStubMemory<inputType> inputStub[numInputs];
  static InputStubMemory<DISK2S> inputStubDisk2S[numInputsDisk2S]; //Only used for Disks

  // output memories
  static AllStubMemory<outputType> allStub[maxAllCopies];
  // ME memories
  static VMStubMEMemory<outputType, nbitsbin> meMemories[numME];
  // TE Inner memories, including copies
  static VMStubTEInnerMemory<outputType> teiMemories[numTEI][maxTEICopies];
  // TE Inner Overlap memories, including copies
  static VMStubTEInnerMemory<BARRELOL> olMemories[numOL][maxOLCopies];
  // TE Outer memories
  static VMStubTEOuterMemory<outputType> teoMemories[numTEO][maxTEOCopies];


  ///////////////////////////
  // open input files
    cout << "Open files..." << endl;

    ifstream fin_inputstub[numInputs];

    for (unsigned int i = 0; i < numInputs; i++) {
      bool valid = openDataFile(fin_inputstub[i], inNameList[i] + fileEnding);
      if (not valid) return -1;
    }


  ///////////////////////////
  // open output files

  // AllStub
  ifstream fout_allstub[maxAllCopies];

  for (unsigned int i = 0; i < maxAllCopies; i++) {
    bool valid = openDataFile(fout_allstub[i], allStubName + "n" + to_string(i+1) + fileEnding);
    if (not valid) return -1;
  }

  // ME memories
  ifstream fout_vmstubme[numME];

  for (unsigned int i = 0; i < numME; i++) {
    bool valid =  openDataFile(fout_vmstubme[i], meNameList[i] + "n1" + fileEnding);
    if (not valid) return -1;
  }

  // TE Inner
	ifstream fout_vmstubtei[numTEI][maxTEICopies];

  if (maxTEICopies > 1) {
    for (unsigned int i = 0; i < numTEI; i++) {
      for (unsigned int j = 0; j < teiNumCopies[i]; j++) {
        bool valid = openDataFile(fout_vmstubtei[i][j], teiNameList[i] + "n" + to_string(j+1) + fileEnding);
        if (not valid) return -1;
      }
    }
  }

  // TE Inner Overlap
	ifstream fout_vmstubteol[numOL][maxOLCopies];

  if (maxOLCopies > 1) {
    for (unsigned int i = 0; i < numOL; i++) {
      for (unsigned int j = 0; j < olNumCopies[i]; j++) {
        bool valid = openDataFile(fout_vmstubteol[i][j], olNameList[i] + "n" + to_string(j+1) + fileEnding);
        if (not valid) return -1;
      }
    }
  }

  // TE Outer
  ifstream fout_vmstubteo[numTEO][maxTEOCopies];

  if (maxTEOCopies > 1) {
    for (unsigned int i = 0; i < numTEO; i++) {
      for (unsigned int j = 0; j < teoNumCopies[i]; j++) {
        bool valid = openDataFile(fout_vmstubteo[i][j], teoNameList[i] + "n" + to_string(j+1) + fileEnding);
        if (not valid) return -1;
      }
    }
  }

  // error count
  int err = 0;

  ///////////////////////////
  // loop over events
  cout << "Start event loop ..." << endl;
  for (unsigned int ievt = 0; ievt < nevents; ++ievt) {
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
    BXType bx_out;

    // Unit Under Test
  	VMRouterTop(bx, inputStub,
        #if kDISK > 0
        inputStubDisk2S,
        #endif
        #if kLAYER == 1 || kLAYER == 3 || kLAYER == 5 || kDISK == 1 || kDISK == 3
        teiMemories,
        #endif
        #if kLAYER == 1 || kLAYER == 2
        olMemories,
        #endif
        #if kLAYER == 2 || kLAYER == 4 || kLAYER == 6 || kDISK == 1 || kDISK == 2 || kDISK == 4
        teoMemories,
        #endif
        allStub, meMemories);

    // compare the computed outputs with the expected ones
    // add 1 to the error count per stub that is incorrect

    bool truncation = false;

    // AllStub
    for (unsigned int i = 0; i < numInputs; i++) {
      err += compareMemWithFile<AllStubMemory<outputType>>(allStub[i], fout_allstub[i], ievt, "AllStub", truncation);
    }

    // ME Memories
    for (unsigned int i = 0; i < numME; i++) {
      err += compareBinnedMemWithFile<VMStubMEMemory<outputType, nbitsbin>>(meMemories[i], fout_vmstubme[i], ievt, "VMStubME" + to_string(i), truncation);
    }

    // TE Inner Memories
    if (maxTEICopies > 1) {
      for (unsigned int i = 0; i < numTEI; i++) {
        for (unsigned int j = 0; j < teiNumCopies[i]; j++) {
          err += compareMemWithFile<VMStubTEInnerMemory<outputType>>(teiMemories[i][j], fout_vmstubtei[i][j], ievt, "VMStubTEInner" + to_string(i), truncation);
        }
      }
    }

    // TE Inner Overlap memories
    if (maxOLCopies > 1) {
      for (unsigned int i = 0; i < numOL; i++) {
        for (unsigned int j = 0; j < olNumCopies[i]; j++) {
          err += compareMemWithFile<VMStubTEInnerMemory<BARRELOL>>(olMemories[i][j], fout_vmstubteol[i][j], ievt, "VMStubTEOverlap" + to_string(i), truncation);
        }
      }
    }

    // TE Outer memories
    if (maxTEOCopies > 1) {
      for (unsigned int i = 0; i < numTEO; i++) {
        cout << teoNumCopies[i] << endl;
        for (unsigned int j = 0; j < teoNumCopies[i]; j++) {
          err += compareBinnedMemWithFile<VMStubTEOuterMemory<outputType>>(teoMemories[i][j], fout_vmstubteo[i][j], ievt, "VMStubTEOuter" + to_string(i), truncation);
        }
      }
    }
  } // end of event loop

	cerr << "Exiting with return value " << err << endl;
	return err;

}
