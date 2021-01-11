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
//          - the included top function in VMRouter_test.cpp (if file name is changed)
//          - the top function and memory directory in script_VMR.tcl (if file name is changed)
//          - add the phi region in emData/download.sh, make sure to also run clean
//          - and the changes listed in VMRouterTop.cc/h


// Finds all memory names for the specified processing module found in the wiring file,
// and adds them to the fileNames array with the memory files directory, excluding the copies nX.
// The number of copies are kept track in the numCopiesArray.
// E.g. finds all memories that start with "VMSME_L1PHIE", such as "VMSME_L1PHIE17" etc.
// Returns false if wiring file isn't found.
template<int arraySize>
bool findFileNames(string wireFileName, string memID, string nameList[arraySize], int numCopiesArray[arraySize]) {

  ifstream wireFile; // Will contain the wiring file

  int nvmMemories = 0; // Number of memories found
  char delimeter = (memID.substr(0,2) == "IL") ? ' ' : 'n'; // Different delimeter if input or output memories

  bool valid = openDataFile(wireFile, wireFileName); // Open the wiring file

  // Check if the wiring file was opened properly.
  if (not valid) {
    cout << "Could not find wiring file." << endl;
    return false;
  }

  // Loop over all lines in the wiring file
  for (string inputLine; getline(wireFile, inputLine); ) {

    // If we find the memory we are looking for
    if (inputLine.find(memID) != string::npos) {

      string tmpMemoryDir = inputLine.substr(0, inputLine.find(delimeter)); // The directory and the name of the memory (first part of the line)

      auto isInNameList = find(nameList, nameList+arraySize, tmpMemoryDir); // Check if tmpMemoryDir is in nameList

      // Add the start of the memory name to the list if we haven't added it before, otherwise increment the number of copies
      if (isInNameList == nameList+arraySize) {
        nameList[nvmMemories] = tmpMemoryDir;
        numCopiesArray[nvmMemories]++;
        nvmMemories++;
      } else {
        numCopiesArray[distance(nameList, isInNameList)]++;
      }
    }
  }
  return true;
}

// Read lookup table and returns it in an array
template<typename apInt>
void readTable(apInt table[], string tableName) {

  ifstream file;
  openDataFile(file, tableName);
  
  int i = 0;

  for (string inputLine; getline(file, inputLine); ) {
    if ((inputLine != "{") && (inputLine != "};")) {
      int value = stoi(inputLine.substr(0, inputLine.find(','))); // Convert char to int
      table[i] = value;
      i++;
    }
  }
}


// Reads bendcut table and returns it as an ap_uint
ap_uint<bendCutTableSize> readBendCutTable(string tableName) {

  ifstream file;
  openDataFile(file, tableName);
  
  ap_uint<bendCutTableSize> table;
  int i = 0;

  for (string inputLine; getline(file, inputLine); ) {
    if ((inputLine != "{") && (inputLine != "};")) {
      int value = inputLine.at(0) - '0'; // Convert char to int
      table[i] = value;
      i++;
    }
  }

  return table;
}


// Creates a bendcut table
// Each element in array is an ap_uint and correspond to one TE memory
template <int lutSize, int arraySize>
void createBendCutTable(ap_uint<bendCutTableSize> lutTable[lutSize], string nameList[arraySize], int numCopiesArray[arraySize]) {

  int numMaxCopies = lutSize/arraySize;

  for (int i = 0; i < arraySize; i++) {
    for (int j = 0; j < numMaxCopies; j++) {

      int tableIndex = i * numMaxCopies + j;

      if (j < numCopiesArray[i]) {
        string tableName = "tables/" + nameList[i] + "n" + to_string(j+1) + "_vmbendcut.tab";
        lutTable[tableIndex] = readBendCutTable(tableName);
      } else {
        lutTable[tableIndex] = 0;
      }
    }
  }
}


int main() {

  ////////////////////////////////////////////////////////////////
  // Get lists of the input/output memory directory and file names
  // I.e. the names of the test vector files

  string fileEnding = (sector < 10) ? "_0" + to_string(sector) + ".dat" :  "_" + to_string(sector) + ".dat"; //All files ends with .dat. "_XX" specifies which sector

  // Uses wires_hourglass.dat wiring file
  string wireFileName = "wires_hourglass.dat"; // The wiring file name with directory
  
  char overlapPhiRegion[] = {'X', 'Y', 'Z', 'W', 'Q', 'R', 'S', 'T'}; // Special naming for the TE overlap memories, and outer memories in Disk 1
  char extraPhiRegion[] = {'I', 'J', 'K', 'L'}; // Special naming for the extra memories TEInner L2 and TEOuter L3.


  // Input file names
  string inputNameList[nPhiRegions][numInputs];
  int inputNumCopies[nPhiRegions][numInputs] = {0}; // Array containing the number of copies of each memory
  
  // Start of AllStub file names, excluding the copy number
  string allstubName[nPhiRegions];
  
  // Start of MEStub file names, excluding the copy number, i.e. "n1" as they only have one copy
  string nameListME[nPhiRegions][nvmME];
  int numCopiesME[nPhiRegions][nvmME] = {0}; // Array containing the number of copies of each memory
  
  // Start of TEInnerStub file names, excluding the copy number "nX"
  string nameListTEI[nPhiRegions][nvmTEI];
  int numCopiesTEI[nPhiRegions][nvmTEI] = {0}; // Array containing the number of copies of each memory
  
  // Start of TEInnerStub Overlap file names, excluding the copy number
  string nameListOL[nPhiRegions][nvmOL];
  int numCopiesOL[nPhiRegions][nvmOL] = {0}; // Array containing the number of copies of each memory
  
  // Start of TEOuterStub file names, excluding the copy number "nX"
  string nameListTEO[nPhiRegions][nvmTEO];
  int numCopiesTEO[nPhiRegions][nvmTEO] = {0}; // Array containing the number of copies of each memory
  
  for (unsigned int i = 0; i < nPhiRegions; i++) {
    
    char phiRegion = phiRegionList[i];
    string layerID = (kLAYER) ? "L" + to_string(kLAYER) + "PHI" + phiRegion : "D" + to_string(kDISK) + "PHI" + phiRegion; // Which layer/disk and phi region

    // Input file names
    string inMemID = "IL_" + layerID; // Input memory ID for the specified phi region
    // Get the input file names and check that the wiring file can be opened properly
    if (not findFileNames<numInputs>(wireFileName, inMemID, inputNameList[i], inputNumCopies[i])) return -1;
    

    // Start of AllStub file names, excluding the copy number
    allstubName[i] = "AS_" + layerID;


    // Start of MEStub file names, excluding the copy number, i.e. "n1" as they only have one copy
    string meMemID  =  "VMSME_" + layerID; // ME memory ID for the specified phi region
    findFileNames<nvmME>(wireFileName, meMemID, nameListME[i], numCopiesME[i]);


    // Start of TEInnerStub file names, excluding the copy number "nX"
    if (maxTEICopies > 1) {
      string teiMemID = (kLAYER != 2) ? "VMSTE_" + layerID : string("VMSTE_L2PHI") + extraPhiRegion[phiRegion - 'A']; // TE Inner memory ID for the specified phi region
      findFileNames<nvmTEI>(wireFileName, teiMemID, nameListTEI[i], numCopiesTEI[i]);
    }


    // Start of TEInnerStub Overlap file names, excluding the copy number
    if (maxOLCopies > 1) {
      string olMemID = "VMSTE_L" + to_string(kLAYER) + "PHI" + overlapPhiRegion[phiRegion - 'A']; // TE Inner memory ID for the specified phi region
      findFileNames<nvmOL>(wireFileName, olMemID, nameListOL[i], numCopiesOL[i]);
    }


    // Start of TEOuterStub file names, excluding the copy number "nX"
    if (maxTEOCopies > 1) {
      string teoMemID; // TE Outer memory ID for the specified phi region

      if (kDISK == 1) {
        teoMemID = string("VMSTE_D1PHI") + overlapPhiRegion[phiRegion - 'A'];
      }
      else if (kLAYER == 3) {
        teoMemID = string("VMSTE_L3PHI") + extraPhiRegion[phiRegion - 'A'];
      }
      else {
        teoMemID = "VMSTE_" + layerID;
      }

      findFileNames<nvmTEO>(wireFileName, teoMemID, nameListTEO[i], numCopiesTEO[i]);
    }
  }


  ///////////////////////////
  // input memories
  static InputStubMemory<inputType> inputStub[nPhiRegions][numInputs];
  static InputStubMemory<DISK2S> inputStubDisk2S[nPhiRegions][numInputsDisk2S]; //Only used for Disks

  // output memories
  static AllStubMemory<outputType> memoriesAS[nPhiRegions][maxASCopies];
  // ME memories
  static VMStubMEMemory<outputType, nbitsbin> memoriesME[nPhiRegions][nvmME];
  // TE Inner memories, including copies
  static VMStubTEInnerMemory<outputType> memoriesTEI[nPhiRegions][nvmTEI][maxTEICopies];
  // TE Inner Overlap memories, including copies
  static VMStubTEInnerMemory<BARRELOL> memoriesOL[nPhiRegions][nvmOL][maxOLCopies];
  // TE Outer memories
  static VMStubTEOuterMemory<outputType> memoriesTEO[nPhiRegions][nvmTEO][maxTEOCopies];

  
  ///////////////////////////
  // open input and ouput files
  cout << "Open files..." << endl;

  ifstream fin_inputstub[nPhiRegions][numInputs];
  
  // AllStub
  ifstream fout_allstub[nPhiRegions][maxASCopies];
  // ME memories
  ifstream fout_vmstubme[nPhiRegions][nvmME];
  // TE Inner
  ifstream fout_vmstubtei[nPhiRegions][nvmTEI][maxTEICopies];
  // TE Inner Overlap
  ifstream fout_vmstubteol[nPhiRegions][nvmOL][maxOLCopies];
  // TE Outer
  ifstream fout_vmstubteo[nPhiRegions][nvmTEO][maxTEOCopies];
  
  for (unsigned int i = 0; i < nPhiRegions; i++) {
    
    string layerID = (kLAYER) ? "L" + to_string(kLAYER) + "PHI" + phiRegionList[i]: "D" + to_string(kDISK) + "PHI" + phiRegionList[i]; // Which layer/disk and phi region
    string testDataDirectory = "VMR/VMR_" + layerID;
    
    for (unsigned int j = 0; j < numInputs; j++) {
      bool valid = openDataFile(fin_inputstub[i][j], testDataDirectory + "/InputStubs_" + inputNameList[i][j] + fileEnding);
      if (not valid) return -1;
    }
    
    // AllStub
    for (unsigned int j = 0; j < maxASCopies; j++) {
      bool valid = openDataFile(fout_allstub[i][j], testDataDirectory + "/AllStubs_" + allstubName[i] + "n" + to_string(j+1) + fileEnding);
      if (not valid) return -1;
    }
    
    // ME memories
    for (unsigned int j = 0; j < nvmME; j++) {
      bool valid =  openDataFile(fout_vmstubme[i][j], testDataDirectory + "/VMStubs_" + nameListME[i][j] + "n1" + fileEnding);
      if (not valid) return -1;
    }
    
    // TE Inner
    if (maxTEICopies > 1) {
      for (unsigned int j = 0; j < nvmTEI; j++) {
        for (unsigned int k = 0; k < numCopiesTEI[i][j]; k++) {
          bool valid = openDataFile(fout_vmstubtei[i][j][k], testDataDirectory + "/VMStubs_" + nameListTEI[i][j] + "n" + to_string(k+1) + fileEnding);
          if (not valid) return -1;
        }
      }
    }
    
    // TE Inner Overlap
    if (maxOLCopies > 1) {
      for (unsigned int j = 0; j < nvmOL; j++) {
        for (unsigned int k = 0; k < numCopiesOL[i][j]; k++) {
          bool valid = openDataFile(fout_vmstubteol[i][j][k], testDataDirectory + "/VMStubs_" + nameListOL[i][j] + "n" + to_string(k+1) + fileEnding);
          if (not valid) return -1;
        }
      }
    }
    
    // TE Outer
    if (maxTEOCopies > 1) {
      for (unsigned int j = 0; j < nvmTEO; j++) {
        for (unsigned int k = 0; k < numCopiesTEO[i][j]; k++) {
          bool valid = openDataFile(fout_vmstubteo[i][j][k], testDataDirectory + "/VMStubs_" + nameListTEO[i][j] + "n" + to_string(k+1) + fileEnding);
          if (not valid) return -1;
        }
      }
    }
  }

  //////////////////////////////////
  // Open LUTs

  ap_uint<bendCutTableSize> bendCutInnerTable[nPhiRegions][nvmTEI*maxTEICopies];
  ap_uint<bendCutTableSize> bendCutOverlapTable[nPhiRegions][nvmOL*maxOLCopies];
  ap_uint<bendCutTableSize> bendCutOuterTable[nPhiRegions][nvmTEO*maxTEOCopies];
  
  // LUT with the corrected r/z. It is corrected for the average r (z) of the barrel (disk).
  // Includes both coarse r/z position (bin), and finer region each r/z bin is divided into.
  // Indexed using r and z position bits
  ap_uint<6> fineBinTables[nPhiRegions][1<<11];

  // LUT with phi corrections to project the stub to the average radius in a layer.
  // Only used by layers.
  // Indexed using phi and bend bits
  ap_int<10> phiCorrTables[nPhiRegions][1<<11]; 
  
  // LUT with the Z/R bits for TE memories
  // Contain information about where in z to look for valid stub pairs
  // Indexed using z and r position bits

  ap_int<11> rzBitsInnerTable[nPhiRegions][1<<11];

  ap_int<11> rzBitsOverlapTable[nPhiRegions][1<<11];

  // 	static const int rzBitsOuterTable[] = // 11 bits used for LUT
  // #include "../emData/VMR/tables/VMTableOuterXX.tab"

  for (unsigned int i = 0; i < nPhiRegions; i++) {
    if (maxTEICopies > 1) createBendCutTable<nvmTEI*maxTEICopies, nvmTEI>(bendCutInnerTable[i], nameListTEI[i], numCopiesTEI[i]);
    if (maxOLCopies > 1) createBendCutTable<nvmOL*maxOLCopies, nvmOL>(bendCutOverlapTable[i], nameListOL[i], numCopiesOL[i]);
    if (maxTEOCopies > 1) createBendCutTable<nvmTEO*maxTEOCopies, nvmTEO>(bendCutOuterTable[i], nameListTEO[i], numCopiesTEO[i]);
    
    readTable(fineBinTables[i], "tables/VMR_L1PHIE_finebin.tab");
    readTable(phiCorrTables[i], "tables/VMPhiCorrL" + to_string(kLAYER) + ".tab");
    readTable(rzBitsInnerTable[i], "tables/VMTableInnerL1L2.tab");
    readTable(rzBitsOverlapTable[i], "tables/VMTableInnerL1D1.tab");
  }


  ///////////////////////////
  // loop over events

  // error count
  int err = 0;

  cout << "Start event loop ..." << endl;
  for (unsigned int ievt = 0; ievt < nEvents; ++ievt) {
    cout << "Event: " << dec << ievt << endl;

    // clear output memories
    for (unsigned int i = 0; i < nPhiRegions; i++) {
      for (int j=0; j<maxASCopies; ++j) {
        memoriesAS[i][j].clear();
      }
      for (int j=0; j<nvmME; ++j) {
        memoriesME[i][j].clear();
      }
      for (int j=0; j<nvmTEI; ++j) {
        for (int k=0; k<maxTEICopies; k++) {
          memoriesTEI[i][j][k].clear();
        }
      }
      for (int j=0; j<nvmOL; ++j) {
        for (int k=0; k<maxOLCopies; k++) {
          memoriesOL[i][j][k].clear();
        }
      }
      for (int j=0; j<nvmTEO; ++j) {
        for (int k=0; k<maxTEOCopies; k++) {
          memoriesTEO[i][j][k].clear();
        }
      }
    }

    int num2S = 0; // Keeps track of how many DISK 2S modules we have written to

    // read event and write to memories
    for (unsigned int i = 0; i < nPhiRegions; i++) {
      for (unsigned int j = 0; j < numInputs; j++) {
        if (kLAYER) {
          writeMemFromFile<InputStubMemory<inputType>>(inputStub[i][j], fin_inputstub[i][j], ievt);
        } else {
          if (i == 0 || i == 3) {
            writeMemFromFile<InputStubMemory<DISK2S>>(inputStubDisk2S[i][num2S], fin_inputstub[i][j], ievt);
            num2S++;
          } else {
            writeMemFromFile<InputStubMemory<inputType>>(inputStub[i][j - num2S], fin_inputstub[i][j], ievt);
          }
        }
      }
    }

    // bx - bunch crossing
    BXType bx = ievt;

    // Unit Under Test
    SuperVMRouterTop(bx,
      // Lookup tables
      fineBinTables,
      phiCorrTables,
      rzBitsInnerTable,
      rzBitsOverlapTable,
      // Memories
      inputStub
#if kDISK > 0
        , inputStubDisk2S
#endif
        , memoriesAS, memoriesME
#if kLAYER == 1 || kLAYER  == 2 || kLAYER == 3 || kLAYER == 5 || kDISK == 1 || kDISK == 3
        , bendCutInnerTable, memoriesTEI
#endif
#if kLAYER == 1 || kLAYER == 2
        , bendCutOverlapTable, memoriesOL
#endif
#if kLAYER == 2 || kLAYER == 3 || kLAYER == 4 || kLAYER == 6 || kDISK == 1 || kDISK == 2 || kDISK == 4
        , bendCutOuterTable, memoriesTEO
#endif
      );

    // compare the computed outputs with the expected ones
    // add 1 to the error count per stub that is incorrect

    bool truncation = false;
    
    for (unsigned int i = 0; i < nPhiRegions; i++) {
      // AllStub
      for (unsigned int j = 0; j < maxASCopies; j++) {
        err += compareMemWithFile<AllStubMemory<outputType>>(memoriesAS[i][j], fout_allstub[i][j], ievt, "AllStub", truncation);
      }
      
      // ME Memories
      for (unsigned int j = 0; j < nvmME; j++) {
        err += compareBinnedMemWithFile<VMStubMEMemory<outputType, nbitsbin>>(memoriesME[i][j], fout_vmstubme[i][j], ievt, "VMStubME" + to_string(j), truncation);
      }
      
      // TE Inner Memories
      if (maxTEICopies > 1) {
        for (unsigned int j = 0; j < nvmTEI; j++) {
          for (unsigned int k = 0; k < numCopiesTEI[i][j]; k++) {
            err += compareMemWithFile<VMStubTEInnerMemory<outputType>>(memoriesTEI[i][j][k], fout_vmstubtei[i][j][k], ievt, "VMStubTEInner" + to_string(j) + "n" + to_string(k+1), truncation);
          }
        }
      }
      
      // TE Inner Overlap memories
      if (maxOLCopies > 1) {
        for (unsigned int j = 0; j < nvmOL; j++) {
          for (unsigned int k = 0; k < numCopiesOL[i][j]; k++) {
            err += compareMemWithFile<VMStubTEInnerMemory<BARRELOL>>(memoriesOL[i][j][k], fout_vmstubteol[i][j][k], ievt, "VMStubTEOverlap" + to_string(j) + "n" + to_string(k+1), truncation);
          }
        }
      }
      
      // TE Outer memories
      if (maxTEOCopies > 1) {
        for (unsigned int j = 0; j < nvmTEO; j++) {
          for (unsigned int k = 0; k < numCopiesTEO[i][j]; k++) {
            err += compareBinnedMemWithFile<VMStubTEOuterMemory<outputType>>(memoriesTEO[i][j][k], fout_vmstubteo[i][j][k], ievt, "VMStubTEOuter" + to_string(j) + "n" + to_string(k+1), truncation);
          }
        }
      }
    }
  } // end of event loop

	cerr << "Exiting with return value " << err << endl;
	// This is necessary because HLS seems to only return an 8-bit error count, so if err%256==0, the test bench can falsely pass
	if (err > 255) err = 255;
	return err;

}
