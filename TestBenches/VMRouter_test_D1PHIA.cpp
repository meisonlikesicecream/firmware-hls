// Test bench for VMRouter
#include "VMRouterTop_D1PHIA.h"

#include <algorithm>
#include <iterator>

#include "FileReadUtility.h"

using namespace std;

const int nevents = 100;  //number of events to run

// VMRouter Top Function for Disk 1, AllStub region A
// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).

int main()
{

  // Code dependent on which VMR it is.
  // Note that one also needs to change if one has TEInner/Outer and Overlap

  // The following are the same as maxCopies unless we are at the border of a sector
  // Note that the testbench assumes that the difference in the number of copies
  // between two neighbouring memories, e.g. PHIA1 and PHIA2, is at most 1
  constexpr int minTEICopies = 2; // Minimum number of TE Inner copies
  constexpr int minTEOCopies = 3;

  constexpr bool firstPhiRegion = true; // I.e. PHIA
  constexpr bool lastPhiRegion = false;

  // Input file names
  string inNames[numInputs] = {"VMR_D1PHIA/InputStubs_IL_D1PHIA_2S_5_A_04.dat",
                      "VMR_D1PHIA/InputStubs_IL_D1PHIA_PS10G_2_A_04.dat",
                      "VMR_D1PHIA/InputStubs_IL_D1PHIA_PS5G_4_A_04.dat",
                      "VMR_D1PHIA/InputStubs_IL_D1PHIA_neg_2S_5_A_04.dat",
                      "VMR_D1PHIA/InputStubs_IL_D1PHIA_neg_PS10G_2_A_04.dat",
                      "VMR_D1PHIA/InputStubs_IL_D1PHIA_neg_PS5G_4_A_04.dat"};

  // Start of AllStub file names, excluding the copy number
  string allStubName = "VMR_D1PHIA/AllStubs_AS_D1PHIAn"; //Start of AllStub file name

  // Start of MEStub file names, including copy number, i.e. n1, as they only have one copy
  string meNames[numME] = {"VMR_D1PHIA/VMStubs_VMSME_D1PHIA1n1",
                      "VMR_D1PHIA/VMStubs_VMSME_D1PHIA2n1",
                      "VMR_D1PHIA/VMStubs_VMSME_D1PHIA3n1",
                      "VMR_D1PHIA/VMStubs_VMSME_D1PHIA4n1",
                      "VMR_D1PHIA/VMStubs_VMSME_D1PHIA5n1",
                      "VMR_D1PHIA/VMStubs_VMSME_D1PHIA6n1",
                      "VMR_D1PHIA/VMStubs_VMSME_D1PHIA7n1",
                      "VMR_D1PHIA/VMStubs_VMSME_D1PHIA8n1"};

  // Start of TEInnerStub file names, excluding the copy number
  string teiNames[numTEI] = {"VMR_D1PHIA/VMStubs_VMSTE_D1PHIA1n",
                      "VMR_D1PHIA/VMStubs_VMSTE_D1PHIA2n",
                      "VMR_D1PHIA/VMStubs_VMSTE_D1PHIA3n",
                      "VMR_D1PHIA/VMStubs_VMSTE_D1PHIA4n"};

  // Start of TEInnerStub Overlap file names, excluding the copy number
  string olNames[numOL];

  // Start of TEOuter file names, excluding the copy number
  string teoNames[numTEO] = {"VMR_D1PHIA/VMStubs_VMSTE_D1PHIX1n",
                      "VMR_D1PHIA/VMStubs_VMSTE_D1PHIX2n",
                      "VMR_D1PHIA/VMStubs_VMSTE_D1PHIX3n",
                      "VMR_D1PHIA/VMStubs_VMSTE_D1PHIX4n"};

  string fileEnding = "_04.dat"; //All files ends with .dat, _04 specifies which sector


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
      bool valid = openDataFile(fin_inputstub[i], inNames[i]);
      if (not valid) return -1;
    }

  ///////////////////////////
  // open output files

  // AllStub
  ifstream fout_allstub[maxAllCopies];

  for (unsigned int i = 0; i < maxAllCopies; i++) {
    bool valid = openDataFile(fout_allstub[i], allStubName + to_string(i+1) + fileEnding);
    if (not valid) return -1;
  }

  // ME memories
  ifstream fout_vmstubme[numME];

  for (unsigned int i = 0; i < numME; i++) {
    bool valid =  openDataFile(fout_vmstubme[i], meNames[i] + fileEnding);
    if (not valid) return -1;
  }

  // TE Inner
	ifstream fout_vmstubtei[numTEI][maxTEICopies];

  if (maxTEICopies > 1) {
    int numCopies = (firstPhiRegion) ? minTEICopies : maxTEICopies;
    for (unsigned int i = 0; i < numTEI; i++) {
      for (unsigned int j = 0; j < numCopies; j++) {
        bool valid = openDataFile(fout_vmstubtei[i][j], teiNames[i] + to_string(j+1) + fileEnding);
        if (not valid) return -1;
      }
      if (firstPhiRegion && (numCopies < maxTEICopies)) numCopies++;
      if (lastPhiRegion && (numCopies > minTEICopies)) numCopies--;
    }
  }

  // TE Inner Overlap
	ifstream fout_vmstubteol[numOL][maxOLCopies];

  if (maxOLCopies > 1) {
    for (unsigned int i = 0; i < numOL; i++) {
      for (unsigned int j = 0; j < maxOLCopies; j++) {
        bool valid = openDataFile(fout_vmstubteol[i][j], olNames[i] + to_string(j+1) + fileEnding);
        if (not valid) return -1;
      }
    }
  }

  // TE Outer
  ifstream fout_vmstubteo[numTEO][maxTEOCopies];

  if (maxTEOCopies > 1) {
    int numCopies = (firstPhiRegion) ? minTEOCopies : maxTEOCopies;
    for (unsigned int i = 0; i < numTEO; i++) {
      for (unsigned int j = 0; j < numCopies; j++) {
        bool valid = openDataFile(fout_vmstubteo[i][j], teoNames[i] + to_string(j+1) + fileEnding);
        if (not valid) return -1;
      }
      if (firstPhiRegion && (numCopies < maxTEOCopies)) numCopies++;
      if (lastPhiRegion && (numCopies > minTEOCopies)) numCopies--;
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
        #if kLAYER == 2 || kLAYER == 4 || kLAYER == 6 || kDISK == 1 || kDISK == 3 || kDISK == 4
        teoMemories,
        #endif
        allStub, meMemories);

    // compare the computed outputs with the expected ones
    // add 1 to the error count per stub that is incorrect

    bool truncation = false;

    // AllStub
    for (unsigned int i = 0; i < numInputs; i++) {
      err += compareMemWithFile<AllStubMemory<outputType>>(allStub[i], fout_allstub[i], ievt, "AllStubn" + to_string(i + 1), truncation);
    }

    // ME Memories
    for (unsigned int i = 0; i < numME; i++) {
      err += compareBinnedMemWithFile<VMStubMEMemory<outputType, nbitsbin>>(meMemories[i], fout_vmstubme[i], ievt, "VMStubME" + to_string(i), truncation);
    }

    // TE Inner Memories
    if (maxTEICopies > 1) {
      int numCopies = (firstPhiRegion) ? minTEICopies : maxTEICopies; // Special case for the first phi region, PHIA
      for (unsigned int i = 0; i < numTEI; i++) {
        for (unsigned int j = 0; j < numCopies; j++) {
          err += compareMemWithFile<VMStubTEInnerMemory<outputType>>(teiMemories[i][j], fout_vmstubtei[i][j], ievt, "VMStubTEInner" + to_string(i) + "n" + to_string(j + 1), truncation);
        }
        if (firstPhiRegion && (numCopies < maxTEICopies)) numCopies++; // Special case for the first phi region, PHIA
        if (lastPhiRegion && (numCopies > minTEICopies)) numCopies--; // Special case for the last phi region
      }
    }

    // TE Inner Overlap memories
    if (maxOLCopies > 1) {
      for (unsigned int i = 0; i < numOL; i++) {
        for (unsigned int j = 0; j < maxOLCopies; j++) {
          err += compareMemWithFile<VMStubTEInnerMemory<BARRELOL>>(olMemories[i][j], fout_vmstubteol[i][j], ievt, "VMStubTEOverlap" + to_string(i) + "n" + to_string(j + 1), truncation);
        }
      }
    }

    // TE Outer memories
    if (maxTEOCopies > 1) {
      int numCopies = (firstPhiRegion) ? minTEOCopies : maxTEOCopies; // Special case for the first phi region, PHIA
      for (unsigned int i = 0; i < numTEO; i++) {
        for (unsigned int j = 0; j < numCopies; j++) {
          err += compareBinnedMemWithFile<VMStubTEOuterMemory<outputType>>(teoMemories[i][j], fout_vmstubteo[i][j], ievt, "VMStubTEOuter" + to_string(i) + "n" + to_string(j + 1), truncation);
        }
        if (firstPhiRegion && (numCopies < maxTEOCopies)) numCopies++; // Special case for the first phi region, PHIA
        if (lastPhiRegion && (numCopies > minTEOCopies)) numCopies--; // Special case for the last phi region
      }
    }
  } // end of event loop

	cerr << "Exiting with return value " << err << endl;
	return err;

}
