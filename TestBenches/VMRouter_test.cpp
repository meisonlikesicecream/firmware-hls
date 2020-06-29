// Test bench for VMRouter
#include "VMRouterTop.h"

#include <algorithm>
#include <iterator>

#include "FileReadUtility.h"
#include "Constants.h"


const int nevents = 100;  //number of events to run

using namespace std;

// VMRouter Test for Layer 1, Allstub region E

int main()
{
  // error counts
  int err = 0;

  // Code dependent on which VMR it is.

  // The following are the same as maxCopies unless we are at the border of a sector
  constexpr int minTEICopies = maxTEICopies; // Minimum number of TE Inner copies
  
  constexpr bool firstPhiRegion = false; // I.e. PHIA
  constexpr bool lastPhiRegion = false; 

  std::string finNames[numInputs] = {"VMR_L1PHIE/InputStubs_IL_L1PHIE_PS10G_1_B_04.dat",
                      "VMR_L1PHIE/InputStubs_IL_L1PHIE_PS10G_2_B_04.dat",
                      "VMR_L1PHIE/InputStubs_IL_L1PHIE_neg_PS10G_1_B_04.dat",
                      "VMR_L1PHIE/InputStubs_IL_L1PHIE_neg_PS10G_2_B_04.dat"};

  std::string allStubName = "VMR_L1PHIE/AllStubs_AS_L1PHIEn"; //Start of AllStub file name

  std::string meNames[numME] = {"VMR_L1PHIE/VMStubs_VMSME_L1PHIE17n1",
                      "VMR_L1PHIE/VMStubs_VMSME_L1PHIE18n1",
                      "VMR_L1PHIE/VMStubs_VMSME_L1PHIE19n1",
                      "VMR_L1PHIE/VMStubs_VMSME_L1PHIE20n1"};

  std::string teiNames[numTEI] = {"VMR_L1PHIE/VMStubs_VMSTE_L1PHIE17n",
                      "VMR_L1PHIE/VMStubs_VMSTE_L1PHIE18n",
                      "VMR_L1PHIE/VMStubs_VMSTE_L1PHIE19n",
                      "VMR_L1PHIE/VMStubs_VMSTE_L1PHIE20n"};

  std::string olNames[numOL] = {"VMR_L1PHIE/VMStubs_VMSTE_L1PHIQ9n",
                      "VMR_L1PHIE/VMStubs_VMSTE_L1PHIQ10n"};

  std::string fileEnding = "_04.dat"; //All files ends with .dat, _04 specifies which sector


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
    // add 1 per stub that is incorrect
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
      int numCopies = (firstPhiRegion) ? minTEICopies : maxTEICopies;
      for (unsigned int j = 0; j < numCopies; j++) {
        err += compareMemWithFile<VMStubTEInnerMemory<outputType>>(teiMemories[i][j], fout_vmstubtei[i][j], ievt, "VMStubTEInner" + to_string(i), truncation);
      }
      if (firstPhiRegion && (numCopies < maxTEICopies)) numCopies++;
      if (lastPhiRegion && (numCopies > minTEICopies)) numCopies--;
    }
    
    // TE Inner Overlap memories
    for (unsigned int i = 0; i < numOL; i++) {
      for (unsigned int j = 0; j < maxOLCopies; j++) {
        err += compareMemWithFile<VMStubTEInnerMemory<BARRELOL>>(olMemories[i][j], fout_vmstubteol[i][j], ievt, "VMStubTEOverlap" + to_string(i), truncation);
      }
    }
  
  } // end of event loop

	std::cerr << "Exiting with return value " << err << std::endl;
	return err;

}
