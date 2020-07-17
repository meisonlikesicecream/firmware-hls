// Test bench for VMRouter
#include "VMRouterTop.h"

#include <algorithm>
#include <iterator>

#include "FileReadUtility.h"

using namespace std;

const int nevents = 100;  //number of events to run

// VMRouter Test for Layer 1, Allstub region E

int main()
{

  // Code dependent on which VMR it is.
  // Note that one also needs to change if one has TEInner/Outer and Overlap

  // The following are the same as maxCopies unless we are at the border of a sector
  // Note that the testbench assumes that the difference in the number of copies
  // between two neighbouring memories, e.g. PHIA1 and PHIA2, is at most 1
  constexpr int minTEICopies = maxTEICopies; // Minimum number of TE Inner copies

  // The first and the last phi region is a special case due to the number of copies
  constexpr bool firstPhiRegion = false; // I.e. PHIA
  constexpr bool lastPhiRegion = false;

  // Input file names
  string finNames[numInputs] = {"VMR_L1PHIE/InputStubs_IL_L1PHIE_PS10G_1_B_04.dat",
                      "VMR_L1PHIE/InputStubs_IL_L1PHIE_PS10G_2_B_04.dat",
                      "VMR_L1PHIE/InputStubs_IL_L1PHIE_neg_PS10G_1_B_04.dat",
                      "VMR_L1PHIE/InputStubs_IL_L1PHIE_neg_PS10G_2_B_04.dat"};

  // Start of AllStub file names, excluding the copy number
  string allStubName = "VMR_L1PHIE/AllStubs_AS_L1PHIEn";

  // Start of MEStub file names, including copy number, i.e. n1, as they only have one copy
  string meNames[numME] = {"VMR_L1PHIE/VMStubs_VMSME_L1PHIE17n1",
                      "VMR_L1PHIE/VMStubs_VMSME_L1PHIE18n1",
                      "VMR_L1PHIE/VMStubs_VMSME_L1PHIE19n1",
                      "VMR_L1PHIE/VMStubs_VMSME_L1PHIE20n1"};

  // Start of TEInnerStub file names, excluding the copy number
  string teiNames[numTEI] = {"VMR_L1PHIE/VMStubs_VMSTE_L1PHIE17n",
                      "VMR_L1PHIE/VMStubs_VMSTE_L1PHIE18n",
                      "VMR_L1PHIE/VMStubs_VMSTE_L1PHIE19n",
                      "VMR_L1PHIE/VMStubs_VMSTE_L1PHIE20n"};

  // Start of TEInnerStub Overlap file names, excluding the copy number
  string olNames[numOL] = {"VMR_L1PHIE/VMStubs_VMSTE_L1PHIQ9n",
                      "VMR_L1PHIE/VMStubs_VMSTE_L1PHIQ10n"};

  string fileEnding = "_04.dat"; //All files ends with .dat, _04 specifies which sector


  ///////////////////////////
  // input memories
  static InputStubMemory<inputType> inputStub[numInputs];
  // output memories
  static AllStubMemory<outputType> allStub[maxAllCopies];
  // ME memories
  static VMStubMEMemory<outputType, nbitsbin> meMemories[numME];
  // TE Inner memories, including copies
  static VMStubTEInnerMemory<outputType> teiMemories[numTEI][maxTEICopies];
  // TE Inner Overlap memories, including copies
  static VMStubTEInnerMemory<BARRELOL> olMemories[numOL][maxOLCopies];


///////////////////////////
// open input files
  cout << "Open files..." << endl;

  ifstream fin_inputstub[numInputs];

  for (unsigned int i = 0; i < numInputs; i++) {
    bool valid = openDataFile(fin_inputstub[i], finNames[i]);
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

  for (unsigned int i = 0; i < numTEI; i++) {
    int numCopies = (firstPhiRegion) ? minTEICopies : maxTEICopies;
    for (unsigned int j = 0; j < numCopies; j++) {
      bool valid = openDataFile(fout_vmstubtei[i][j], teiNames[i] + to_string(j+1) + fileEnding);
      if (not valid) return -1;
    }
    if (firstPhiRegion && (numCopies < maxTEICopies)) numCopies++;
    if (lastPhiRegion && (numCopies > minTEICopies)) numCopies--;
  }

  // TE Inner Overlap
	ifstream fout_vmstubteol[numOL][maxOLCopies];

  for (unsigned int i = 0; i < numOL; i++) {
    for (unsigned int j = 0; j < maxOLCopies; j++) {
      bool valid = openDataFile(fout_vmstubteol[i][j], olNames[i] + to_string(j+1) + fileEnding);
      if (not valid) return -1;
    }
  }

  // error count
  int err = 0;

  ///////////////////////////
  // loop over events
  cout << "Start event loop ..." << endl;
  for (unsigned int ievt = 0; ievt < nevents; ++ievt) {
    cout << "Event: " << dec << ievt << endl;

    // read event and write to memories
    for (unsigned int i = 0; i < numInputs; i++) {
      writeMemFromFile<InputStubMemory<inputType>>(inputStub[i], fin_inputstub[i], ievt);
    }

    // bx - bunch crossing
    BXType bx = ievt;
    BXType bx_out;


    // Unit Under Test
  	VMRouterTop(bx, inputStub,
  			allStub, meMemories, teiMemories, olMemories);

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
    for (unsigned int i = 0; i < numTEI; i++) {
      int numCopies = (firstPhiRegion) ? minTEICopies : maxTEICopies; // Special case for the first phi region, PHIA
      for (unsigned int j = 0; j < numCopies; j++) {
        err += compareMemWithFile<VMStubTEInnerMemory<outputType>>(teiMemories[i][j], fout_vmstubtei[i][j], ievt, "VMStubTEInner" + to_string(i), truncation);
      }
      if (firstPhiRegion && (numCopies < maxTEICopies)) numCopies++; // Special case for the first phi region, PHIA
      if (lastPhiRegion && (numCopies > minTEICopies)) numCopies--; // Special case for the last phi region
    }

    // TE Inner Overlap memories
    for (unsigned int i = 0; i < numOL; i++) {
      for (unsigned int j = 0; j < maxOLCopies; j++) {
        err += compareMemWithFile<VMStubTEInnerMemory<BARRELOL>>(olMemories[i][j], fout_vmstubteol[i][j], ievt, "VMStubTEOverlap" + to_string(i), truncation);
      }
    }
  } // end of event loop

	cerr << "Exiting with return value " << err << endl;
	return err;

}
