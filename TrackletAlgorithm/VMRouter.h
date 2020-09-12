// VMRouter
// Log
// -------
// First tracklet 2.0 version -- December 2018 -- wittich

// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).
// Several types of memories depending on which module that is going to read it.
// Each VMRouter correspond to one phi/AllStub region.
// Each VM correspond to one ME/TE memory.
// Each memory type contain different bits of the same stub.
// AllStub and TE memories has several versions/copies of the VM.

// Assumes at most 4 inputs in the layers, and 4 (PS) + 2 (2S) inputs in the disks

// NOTE: Nothing in VMRouter.h needs to be changed to run a different phi region


#ifndef TrackletAlgorithm_VMRouter_h
#define TrackletAlgorithm_VMRouter_h

#include "ap_int.h"
#include <cassert>

#include "Constants.h"
#include "InputStubMemory.h"
#include "AllStubMemory.h"
#include "VMStubMEMemory.h"
#include "VMStubTEInnerMemory.h"
#include "VMStubTEOuterMemory.h"


/////////////////////////////////////////
// Constants

// from Constants.hh -- needs a final home?
constexpr unsigned int nallstubslayers[6] = { 8, 4, 4, 4, 4, 4 }; // Number of AllStub memories, i.e. coarse phi regions, per sector
constexpr unsigned int nallstubsdisks[5] = { 4, 4, 4, 4, 4 };

constexpr unsigned int nvmmelayers[6] = { 4, 8, 8, 8, 8, 8 }; // Number of ME VM modules per coarse phi region
constexpr unsigned int nvmmedisks[5] = { 8, 4, 4, 4, 4 };

constexpr unsigned int nvmtelayers[6] = { 4, 8, 4, 8, 4, 8 }; // Number of TE VM modules per coarse phi region
constexpr unsigned int nvmtedisks[5] = { 4, 4, 4, 4, 4 };

constexpr unsigned int nvmollayers[2] = { 2, 2 }; // Number of Overlap VM modules per coarse phi region

// Number of bits used for the VMs for different layers and disks
// E.g. 32 VMs would use 5 vmbits
constexpr int nbitsvmlayer[6] = { 5, 5, 4, 5, 4, 5 }; // Could be computed using the number of VMs...
constexpr int nbitsvmdisk[5] = { 4, 4, 4, 4, 4 };
constexpr int nbitsvmoverlap[2] = { 4, 3 };

// Number of most significant bits (MSBs) of z and r used for index in finebin LUTs
constexpr int nbitszfinebintablelayer = 7;
constexpr int nbitsrfinebintablelayer = 4;

constexpr int nbitszfinebintabledisk = 3;
constexpr int nbitsrfinebintabledisk = 7;

constexpr int nzbitsinnertablelayer = 7;
constexpr int nrbitsinnertablelayer = 4;

constexpr int nzbitsinnertabledisk = 3;
constexpr int nrbitsinnertabledisk = 8;

constexpr int nzbitsoutertablelayer = 7;
constexpr int nrbitsoutertablelayer = 4;

constexpr int nzbitsoutertabledisk = 3;
constexpr int nrbitsoutertabledisk = 7;

constexpr int nzbitsrzbitsOverlapTable = 7;
constexpr int nrbitsrzbitsOverlapTable = 3;

// Number of MSBs used for r index in phiCorr LUTs
constexpr int nrBitsPhiCorrTable = 3; // Found hardcoded in VMRouterphiCorrTable.h

// Constants for determining if the stub should be saved using the rzbitstables
constexpr int maxrzbits = 10; // Maximum number of bits used for each entry in the rzbits tables
constexpr int maxrz = (1 << maxrzbits) - 1; // Anything above this value would correspond to -1, i.e. a non-valid stub

// Constants used for calculating which VM a stub belongs to
constexpr int nmaxvmbits = 5; // Maximum number of bits used for the VM number, i.e. 32
constexpr int nmaxvmolbits = 4; // Overlap

constexpr float maxvmbins = 1 << nmaxvmbits; // How many bins maxvmbits would correspond to
constexpr float maxvmolbins = 1 << nmaxvmolbits; // Overlap

constexpr int nphibitsraw = 7; // Number of bits used for calculating iPhiRawPlus/Minus

// The length of the masks used for the memories
constexpr int maskISsize = 6; // Input memories
constexpr int maskMEsize = 1 << nmaxvmbits; // ME memories
constexpr int maskTEIsize = 1 << nmaxvmbits; // TEInner memories
constexpr int maskOLsize = 1 << nmaxvmolbits; // TEInner Overlap memories
constexpr int maskTEOsize = 1 << nmaxvmbits; // TEOuter memories


//////////////////////////////////////
// Functions used by the VMR


// Converts an array of 0s and 1s to an ap_uint
template<int arraySize>
inline ap_uint<arraySize> arrayToInt(ap_uint<1> array[arraySize]) {
	ap_uint<arraySize> number;

	for(int i = 0; i < arraySize; i++) {
		#pragma HLS unroll
		number[i] = array[i];
	}

	return number;
}

// Returns top 5 (nmaxvmbits) bits of phi, i.e. max 31 in decimal
template<regionType InType>
inline ap_uint<nmaxvmbits> iphivmRaw(const typename AllStub<InType>::ASPHI phi) {

	ap_uint<nmaxvmbits> iphivm = phi.range(phi.length() - 1, phi.length() - nmaxvmbits);

	return iphivm;
}

// Returns a number from 0 to 31. for both the plus and the minus:
// we add a small amount to the raw value; if it's not the same
// as the central value we copy the data to the adjacent memory as well.
template<regionType InType>
inline ap_uint<nmaxvmbits> iphivmRawPlus(const typename AllStub<InType>::ASPHI phi) {

	ap_uint<nphibitsraw> tmp(phi.range(phi.length() - 1, phi.length() - nphibitsraw));
	++tmp;
	ap_uint<nmaxvmbits> plus(tmp.range(tmp.length() - 1, nphibitsraw - nmaxvmbits));

	return plus;
}

// See above.
template<regionType InType>
inline ap_uint<nmaxvmbits> iphivmRawMinus(const typename AllStub<InType>::ASPHI phi) {

	ap_uint<nphibitsraw> tmp(phi.range(phi.length() - 1, phi.length() - nphibitsraw));
	--tmp;
	ap_uint<nmaxvmbits> minus(tmp.range(tmp.length() - 1, nphibitsraw - nmaxvmbits));

	return minus;
}

// Returns the bits of phi corresponding to finephi, i.e. phi regions within a VM
// vmbits is the number of bits for the VMs, i.e. coarse phi region. E.g. 32 VMs would use vmbits=5
// finebits is the number of bits within the VM
template<regionType InType>
inline int iphivmFineBins(const typename AllStub<InType>::ASPHI phi,
		const int vmbits, const int finebits) {

	auto finebin = (phi.range(phi.length() - 1 - vmbits, phi.length() - vmbits - finebits));

	return finebin;
}

// Get the corrected phi, i.e. phi at the average radius of the barrel
// Corrected phi is used by ME and TE memories in the barrel
template<regionType InType>
inline typename AllStub<InType>::ASPHI getPhiCorr(
		const typename AllStub<InType>::ASPHI phi,
		const typename AllStub<InType>::ASR r,
		const typename AllStub<InType>::ASBEND bend, const int phiCorrTable[]) {

	if (InType == DISKPS || InType == DISK2S)
		return phi; // Do nothing if disks

	constexpr auto rBins = 1 << nrBitsPhiCorrTable; // The number of bins for r

	ap_uint<nrBitsPhiCorrTable> rBin = (r + (1 << (r.length() - 1)))
			>> (r.length() - nrBitsPhiCorrTable); // Which bin r belongs to. Note r = 0 is mid radius
	auto index = bend * rBins + rBin; // Index for where we find our correction value
	auto corrValue = phiCorrTable[index]; // The amount we need to correct our phi

	auto phiCorr = phi - corrValue; // the corrected phi

	// Check for overflow
	if (phiCorr < 0)
		phiCorr = 0; // can't be less than 0
	if (phiCorr >= 1 << phi.length())
		phiCorr = (1 << phi.length()) - 1;  // can't be more than the max value

	return phiCorr;
}

// Returns the number of the first ME/TE memory for the current VMRouter
// I.e. the position of the first non-zero bit in the mask
// L1PHIE17 would return 16
inline ap_uint<nmaxvmbits> firstMemNumber(const ap_uint<static_cast<int>(maxvmbins)> mask) {
	ap_uint<static_cast<int>(maxvmbins)> i = 0;
	ap_uint<1> x = mask[i]; // Value of the i:th bit

	// Stop counter when we have reached the first non-zero bit
	while (x == 0 && i < (maxvmbins - 1)) {
		i++;
		x = mask[i];
	}

	return i;
}

// Clears the memories of one-dimensional memory arrays
template<class MaskType, class MemType>
void clear1DMemoryArray(const BXType bx, const int arraySize, const MaskType mask, const int firstMem, MemType memArray[]) {
#pragma HLS inline
	for (int i = 0; i < arraySize; i++) {
#pragma HLS UNROLL
		// Only clear the memory if it is used by the VMR (defined by the mask)
		if (mask[i + firstMem]) memArray[i].clear(bx);
	}
}

// Clears the memories of two-dimensional memory arrays
template<int MaxCopies, class MaskType, class MemType>
void clear2DMemoryArray(const BXType bx, const int nvm, const MaskType mask, const int firstMem, MemType memArray[][MaxCopies]) {
#pragma HLS inline
	for (int i = 0; i < nvm; i++) {
#pragma HLS UNROLL
		// Only clear the memory if it is used by the VMR (defined by the mask)
		if (mask[i + firstMem]) {
			for (int j = 0; j < MaxCopies; j++) {
#pragma HLS UNROLL
				memArray[i][j].clear(bx);
			}
		}
	}
}

// Clears a 2D array of ap_uints by setting everything to 0
template<int MaxCopies>
void clear2DArray(int nvm, ap_uint<kNBits_MemAddr> array[][MaxCopies]) {
#pragma HLS inline
#pragma HLS array_partition variable=array complete dim=0
	for (int i = 0; i < nvm; i++) {
#pragma HLS UNROLL
		for (int j = 0; j < MaxCopies; j++) {
#pragma HLS UNROLL
				array[i][j] = 0;
		}
	}
}

// Returns a ME stub with all the values set
template<regionType InType, regionType OutType, int Layer, int Disk>
inline VMStubME<OutType> createStubME(const InputStub<InType> stub,
		const int index, const bool negDisk, const int fineBinTable[],
		const int phiCorrTable[], int& ivmPlus, int& ivmMinus, int& bin) {

	// The MEStub that is going to be returned
	VMStubME<OutType> stubME;

	// Values from InputStub
	auto z = stub.getZ();
	auto r = stub.getR();
	auto bend = stub.getBend();
	auto phi = stub.getPhi();
	auto phiCorr = getPhiCorr<InType>(phi, r, bend, phiCorrTable); // Corrected phi, i.e. phi at nominal radius (what about disks?)

	int nrBits = r.length(); // Number of bits for r
	int nzBits = z.length(); // Number of bits for z
	int nbendBits = bend.length(); // Number of bits for bend

	// Number of bits
	constexpr int nbitszfinebintable =
			(Layer) ? nbitszfinebintablelayer : nbitszfinebintabledisk; // Number of MSBs of z used in fineBinTable
	constexpr int nbitsrfinebintable =
			(Layer) ? nbitsrfinebintablelayer : nbitsrfinebintabledisk; // Number of MSBs of r used in fineBinTable
	static const int nfinerzbits = stubME.getFineZ().length(); // Number of bits for finer/z

	// Total number of VMs for ME for a layer/disk in a whole sector
	constexpr int nvmTotME =
			Layer != 0 ?
					nallstubslayers[Layer - 1] * nvmmelayers[Layer - 1] :
					nallstubsdisks[Disk - 1] * nvmmedisks[Disk - 1];

	// Some sort of normalisation thing used for determining which VM the stub belongs to
	static const ap_ufixed<nmaxvmbits, nmaxvmbits-1> d_me = nvmTotME / maxvmbins;


	// Set values to VMStubME

	stubME.setBend(bend);
	stubME.setIndex(typename VMStubME<OutType>::VMSMEID(index));

	auto iphiRaw = iphivmRaw<InType>(phiCorr); // Top 5 bits of phi
	auto iphiRawPlus = iphivmRawPlus<InType>(phiCorr); // Top 5 bits of phi after adding a small number
	auto iphiRawMinus = iphivmRawMinus<InType>(phiCorr); // Top 5 bits of phi after subtracting a small number

	int ivm = iphiRaw * d_me; // The VM number
	ivmPlus = iphiRawPlus * d_me;
	ivmMinus = iphiRawMinus * d_me;

	// To avoid overflow
	if (ivmMinus > ivm)
		ivmMinus = 0;
	if (ivmPlus < ivm)
		ivmPlus = nvmTotME - 1;

	// Stubs can only end up in the neighbouring VM after calculating iphivmrawplus/minus
	assert(std::abs(ivmMinus - ivmPlus) <= 1);

	// Indices used to find the rzfine value in fineBinTable
	// fineBinTable returns the top 6 bits of a corrected z
	// Note: not the index that is being saved to the stub
	ap_uint<nbitszfinebintable + nbitsrfinebintable> indexz = ((z
			+ (1 << (nzBits - 1))) >> (nzBits - nbitszfinebintable)); // Make z unsigned and take the top "nbitszfinebintable" bits
	ap_uint<nbitsrfinebintable> indexr = 0;

	if (Disk) {
		if (negDisk) {
			indexz = (1 << nbitszfinebintable) - indexz;
		}
		indexr = r >> (nrBits - nbitsrfinebintable); // Take the top "nbitsrfinebintable" bits
	} else { // Layer
		indexr = ((r + (1 << (nrBits - 1)))
				>> (nrBits - nbitsrfinebintable)); // Make r unsigned and take the top "nbitsrfinebintable" bits
	}

	// The index for fineBinTable
	ap_uint<nbitszfinebintable + nbitsrfinebintable> finebinindex = (indexz
			<< nbitsrfinebintable) + indexr;

	// Get the corrected r/z position
	auto rzcorr = fineBinTable[finebinindex];

	// Coarse z. The bin the stub is going to be put in, in the memory
	bin = rzcorr >> nfinerzbits; // 3 bits, i.e. max 8 bins within each VM
	if (negDisk)
		bin += 1 << MEBinsBits; // bin 8-16 are for negative disks

	// Set rzfine, i.e. the r/z bits within a coarse r/z region
	auto rzfine = rzcorr & ((1 << nfinerzbits) - 1); // the 3 LSB as rzfine
	stubME.setFineZ(rzfine);

	assert(rzfine >= 0);

	return stubME;
};


// Returns a TE Inner stub with all the values set
template<regionType InType, regionType OutType, int Layer, int Disk>
inline VMStubTEInner<OutType> createStubTEInner(const InputStub<InType> stub,
		const int index, const bool negDisk, const int rzbitsInnerTable[],
		const int phiCorrTable[], int& ivm, int& rzbits) {

	// The TEInner Stub that is going to be returned
	VMStubTEInner<OutType> stubTE;

	// Values from InputStub
	auto z = stub.getZ();
	auto r = stub.getR();
	auto bend = stub.getBend();
	auto phi = stub.getPhi();
	auto phiCorr = getPhiCorr<InType>(phi, r, bend, phiCorrTable); // Corrected phi, i.e. phi at nominal radius (what about disks?)

	int nrBits = r.length(); // Number of bits for r
	int nzBits = z.length(); // Number of bits for z
	int nbendBits = bend.length(); // Number of bits for bend

	// Number of bits
	constexpr auto vmbits =
			(Layer) ? nbitsvmlayer[Layer - 1] : nbitsvmdisk[Disk - 1]; // Number of bits for VMs
	static const auto nFinePhiBits = stubTE.getFinePhi().length();  // Number of bits for finephi

	// Total number of VMs for TE in a whole sector
	constexpr int nvmTotTE =
			Layer != 0 ?
					nallstubslayers[Layer - 1] * nvmtelayers[Layer - 1] :
					nallstubsdisks[Disk - 1] * nvmtedisks[Disk - 1];

	// Some sort of normalisation thing used for determining which VM the stub belongs to
	static const ap_ufixed<nmaxvmbits, nmaxvmbits-1> d_te = nvmTotTE / maxvmbins;


	// Set values to VMStubeTEInner

	stubTE.setBend(bend);
	stubTE.setIndex(typename VMStubTEInner<OutType>::VMSTEIID(index));

	auto iphiRaw = iphivmRaw<InType>(phiCorr); // Top 5 bits of phi

	ivm = iphiRaw * d_te; // The VM number

	// Layer
	if (Layer != 0) {

		constexpr auto zBins = (1 << nzbitsinnertablelayer); // Number of bins in z
		constexpr auto rBins = (1 << nrbitsinnertablelayer); // Number of bins in r
		ap_uint<nzbitsinnertablelayer> zBin = (z + (1 << (nzBits - 1)))
				>> (nzBits - nzbitsinnertablelayer); // Make z positive and take the 7 (nzbitsinnertablelayer) MSBs
		ap_uint<nrbitsinnertablelayer> rBin = (r + (1 << (nrBits - 1)))
				>> (nrBits - nrbitsinnertablelayer);

		int rzbitsIndex = zBin * rBins + rBin; // Index for rzbits LUT

		rzbits = rzbitsInnerTable[rzbitsIndex]; // The z/r information bits saved for TE Inner memories.

		stubTE.setZBits(rzbits);
		stubTE.setFinePhi(
				iphivmFineBins<InType>(phiCorr, vmbits, nFinePhiBits));

	} else { // Disk
		assert(Disk != 0);

		constexpr auto zBins = (1 << nzbitsinnertabledisk); // Number of bins in z
		constexpr auto rBins = (1 << nrbitsinnertabledisk); // Number of bins in r
		ap_uint<nzbitsinnertabledisk> zBin = (z + (1 << (nzBits - 1)))
				>> (nzBits - nzbitsinnertabledisk); // Make z positive and take the 7 (nzbitsinnertabledisk) MSBs
		ap_uint<nrbitsinnertabledisk> rBin = r
				>> (nrBits - nrbitsinnertabledisk);

	// Special case if negative disk
		if (negDisk)
			zBin = zBins - 1 - zBin; // Inverted for negative disks

		int rzbitsIndex = rBin * zBins + zBin; // Index for rzbits LUT

		rzbits = rzbitsInnerTable[rzbitsIndex]; // The z/r information bits saved for TE Inner memories.

		stubTE.setZBits(rzbits);
		stubTE.setFinePhi(
				iphivmFineBins<InType>(phiCorr, vmbits, nFinePhiBits));
	}

	return stubTE;
}


// Returns a TE Outer stub with all the values set
template<regionType InType, regionType OutType, int Layer, int Disk>
inline VMStubTEOuter<OutType> createStubTEOuter(const InputStub<InType> stub,
		const int index, const bool negDisk, const int rzbitsOuterTable[],
		const int phiCorrTable[], int& ivm, int& bin) {

	// The TEOuter stub that is going to be returned
	VMStubTEOuter<OutType> stubTE;

	// Values from InputStub
	auto z = stub.getZ();
	auto r = stub.getR();
	auto bend = stub.getBend();
	auto phi = stub.getPhi();
	auto phiCorr = getPhiCorr<InType>(phi, r, bend, phiCorrTable); // Corrected phi, i.e. phi at nominal radius

	int nrBits = r.length(); // Number of bits for r
	int nzBits = z.length(); // Number of bits for z
	int nbendBits = bend.length(); // Number of bits for bend

	// Number of bits
	constexpr auto vmbits =
			(Layer) ? nbitsvmlayer[Layer - 1] : nbitsvmdisk[Disk - 1]; // Number of bits for VMs
	static const int nFinePhiBits = stubTE.getFinePhi().length(); // Number of bits for finePhi
	static const int nfinerzbits = stubTE.getFineZ().length(); // Number of bits for fineR/Z

	// Total number of VMs for TE in a whole sector
	constexpr int nvmTotTE =
			Layer != 0 ?
					nallstubslayers[Layer - 1] * nvmtelayers[Layer - 1] :
					nallstubsdisks[Disk - 1] * nvmtedisks[Disk - 1];

	// Some sort of normalisation thing used for determining which VM the stub belongs to
	static const ap_ufixed<nmaxvmbits, nmaxvmbits-1> d_te = nvmTotTE / maxvmbins;


	// Set values to VMSTubTE Outer

	stubTE.setBend(bend);
	stubTE.setIndex(typename VMStubTEOuter<OutType>::VMSTEOID(index));

	auto iphiRaw = iphivmRaw<InType>(phiCorr); // Top 5 bits of phi
	ivm = iphiRaw * d_te; // The VM number

	// Layer
	if (Layer != 0) {

		stubTE.setFinePhi(
				iphivmFineBins<InType>(phiCorr, vmbits, nFinePhiBits)); // is this the right vmbits

		constexpr auto zBins = (1 << nzbitsoutertablelayer); // Number of bins in z
		constexpr auto rBins = (1 << nrbitsoutertablelayer); // Number of bins in r
		ap_uint<nzbitsoutertablelayer> zBin = (z + (1 << (nzBits - 1)))
				>> (nzBits - nzbitsoutertablelayer); // Make z positive and take the 7 MSBs
		ap_uint<nrbitsoutertablelayer> rBin = (r + (1 << (nrBits - 1)))
				>> (nrBits - nrbitsoutertablelayer);

		// Find the VM bit information in rzbits LUT
		// First 3 MSB is the binning in z, and the 3 LSB is the fine z within a bin
		auto rzbitsIndex = zBin * rBins + rBin; // number of bins
		auto rzbits = rzbitsOuterTable[rzbitsIndex];

		bin = rzbits >> nfinerzbits; // Remove the 3 LSBs, i.e. the finebin bits

		// Set fine z
		auto zfine = rzbits & nfinerzbits;
		stubTE.setFineZ(zfine);

	} else { // Disks
		assert(Disk != 0);

		stubTE.setFinePhi(
				iphivmFineBins<InType>(phiCorr, vmbits, nFinePhiBits));

		constexpr auto zBins = (1 << nzbitsoutertabledisk); // Number of bins in z
		constexpr auto rBins = (1 << nrbitsoutertabledisk); // Number of bins in r
		ap_uint<nzbitsoutertabledisk> zBin = (z + (1 << (nzBits - 1)))
				>> (nzBits - nzbitsoutertabledisk); // Make z positive and take the 7 MSBs
		ap_uint<nrbitsoutertabledisk> rBin = r
				>> (nrBits - nrbitsoutertabledisk);

		// Special case if negative disk
		if (negDisk) {
			zBin = zBins - 1 - zBin; // Inverted for negative disk
		}

		// Find the VM bit information in rzbits LUT
		// First 2 MSB is the binning in r, and the 3 LSB is the fine r within a bin
		auto rzbitsIndex = rBin * zBins + zBin; // Index for LUT
		auto rzbits = rzbitsOuterTable[rzbitsIndex];

		bin = rzbits >> nfinerzbits; // Remove the 3 LSBs, i.e. the finebin bits

		// Half the bins, i.e. bin 4-7, are used for negative disks
		if (negDisk)
			bin += (1 << MEBinsBits)/2; // += 4

		// Set fine r
		auto rfine = rzbits & ((1 << nfinerzbits) - 1); // Take the 3 (nfinerzbits) LSBs
		stubTE.setFineZ(rfine);
	}

	return stubTE;
}


// Returns a TE Overlap stub with all the values set
template<regionType InType, int Layer>
inline VMStubTEInner<BARRELOL> createStubTEOverlap(const InputStub<InType> stub,
		const int index, const int rzbitsOverlapTable[],
		const int phiCorrTable[], int& ivm, int& rzbits) {

	// The overlap stub that is going to be returned
	VMStubTEInner<BARRELOL> stubOL;

	// Values from InputStub
	auto z = stub.getZ();
	auto r = stub.getR();
	auto bend = stub.getBend();
	auto phi = stub.getPhi();
	auto phiCorr = getPhiCorr<InType>(phi, r, bend, phiCorrTable); // Corrected phi, i.e. phi at nominal radius (what about disks?)

	int nrBits = r.length(); // Number of bits for r
	int nzBits = z.length(); // Number of bits for z
	int nbendBits = bend.length(); // Number of bits for bend

	// Number of bits
	constexpr auto nvmTotOL =
			(Layer) ? nallstubslayers[Layer - 1] * nvmollayers[Layer - 1] : 0; // Total number of VMs for Overlap in a whole sector
	constexpr auto vmbits = (Layer) ? nbitsvmoverlap[Layer - 1] : 0; // Number of bits used for VMs
	static const auto nFinePhiBits = stubOL.getFinePhi().length(); // Number of bits used for fine phi

	// Some sort of normalisation thing used for determining which VM the stub belongs to
	static const ap_ufixed<nmaxvmbits, nmaxvmolbits-1> d_ol = nvmTotOL / maxvmolbins;

	// Set values to Overlap stub

	// 16 overlap vms per layer
	auto iphiRawOl = iphivmRaw<InType>(phiCorr) >> 1; // Top 4 bits of phi

	ivm = iphiRawOl * d_ol; // Which VM it belongs to

	constexpr auto zBins = (1 << nzbitsrzbitsOverlapTable); // Number of bins in z
	constexpr auto rBins = (1 << nrbitsrzbitsOverlapTable); // Number of bins in r
	ap_uint<nzbitsrzbitsOverlapTable> zBin = (z + (1 << (nzBits - 1)))
			>> (nzBits - nzbitsrzbitsOverlapTable); // Make z positive and take the 7 MSBs
	ap_uint<nrbitsrzbitsOverlapTable> rBin = (r + (1 << (nrBits - 1)))
			>> (nrBits - nrbitsrzbitsOverlapTable);

	int rzbitsIndex = zBin * rBins + rBin; // Index for rzbitsOverlapTable

	rzbits = rzbitsOverlapTable[rzbitsIndex];

	stubOL.setBend(bend);
	stubOL.setIndex(typename VMStubTEInner<BARRELOL>::VMSTEIID(index));
	stubOL.setZBits(rzbits);
	stubOL.setFinePhi(
	iphivmFineBins<InType>(phiCorr, vmbits,
			nFinePhiBits));

	return stubOL;
}


/////////////////////////////////
// Main function

// InType DISK2S - Two input region types InType and DISK2S due to the disks having both 2S and PS inputs.
// 		According to wiring script, there's two DISK2S and half the inputs are for negative disks.
// Layer Disk - Specifies the layer or disk number
// MAXCopies - The maximum number of copies of a memory type
// NBitsBin number of bits used for the bins in MEMemories
template<regionType InType, regionType OutType, int Layer, int Disk, int MaxAllCopies, int MaxTEICopies, int MaxOLCopies, int MaxTEOCopies, int NBitsBin, int BendCutTableSize>
void VMRouter(const BXType bx, const int fineBinTable[], const int phiCorrTable[],
		// rzbitstables, aka binlookup in emulation
		const int rzbitsInnerTable[], const int rzbitsOverlapTable[], const int rzbitsOuterTable[],
		// bendcut tables
		const ap_uint<BendCutTableSize> bendCutInnerTable[], const ap_uint<BendCutTableSize> bendCutOverlapTable[], const ap_uint<BendCutTableSize> bendCutOuterTable[],
		// Input memories
		const ap_uint<maskISsize>& maskIS,
		const InputStubMemory<InType> inputStubs[],
		const InputStubMemory<DISK2S> inputStubsDisk2S[],
		// AllStub memory
		AllStubMemory<OutType> memoriesAS[],
		// ME memories
		const ap_uint<maskMEsize>& maskME, VMStubMEMemory<OutType, NBitsBin> memoriesME[],
		// Inner TE memories, non-overlap
		const ap_uint<maskTEIsize>& maskTEI, VMStubTEInnerMemory<OutType> memoriesTEI[][MaxTEICopies],
		// TE Inner memories, overlap
		const ap_uint<maskOLsize>& maskOL, VMStubTEInnerMemory<BARRELOL> memoriesOL[][MaxOLCopies],
		// TE Outer memories
		const ap_uint<maskTEOsize>& maskTEO, VMStubTEOuterMemory<OutType> memoriesTEO[][MaxTEOCopies]) {

#pragma HLS inline
#pragma HLS array_partition variable=bendCutInnerTable complete dim=1
#pragma HLS array_partition variable=bendCutOverlapTable complete dim=1
#pragma HLS array_partition variable=bendCutOuterTable complete dim=1
#pragma HLS array_partition variable=inputStubs complete dim=1
#pragma HLS array_partition variable=inputStubsDisk2S complete dim=1
#pragma HLS array_partition variable=memoriesAS complete dim=1
#pragma HLS array_partition variable=memoriesME complete dim=1
#pragma HLS array_partition variable=memoriesTEI complete dim=2
#pragma HLS array_partition variable=memoriesOL complete dim=2
#pragma HLS array_partition variable=memoriesTEO complete dim=2


	// The first memory numbers, the position of the first non-zero bit in the mask
	static const ap_uint<nmaxvmbits> firstME = firstMemNumber(maskME); // ME memory
	static const ap_uint<nmaxvmbits> firstTE = (maskTEI) ? firstMemNumber(maskTEI) : firstMemNumber(maskTEO); // TE memory
	static const ap_uint<nmaxvmolbits> firstOL = (maskOL) ? firstMemNumber(maskOL) : static_cast<ap_uint<nmaxvmbits>>(0); // TE Overlap memory

	// Number of memories/VMs for one coarse phi region
	constexpr int nvmME = (Layer) ? nvmmelayers[Layer-1] : nvmmedisks[Disk-1]; // ME memories
	constexpr int nvmTE = (Layer) ? nvmtelayers[Layer-1] : nvmtedisks[Disk-1]; // TE memories
	constexpr int nvmOL = ((Layer == 1) || (Layer == 2)) ? nvmollayers[Layer-1] : 0; // TE Overlap memories

	// Create variables that keep track of which memory address to read and write to
	ap_uint<kNBits_MemAddr> read_addr(0); // Reading of input stubs
	ap_uint<kNBits_MemAddr> addrCountTEI[nvmTE][MaxTEICopies]; // Writing of TE Inner stubs
	ap_uint<kNBits_MemAddr> addrCountOL[nvmOL][MaxOLCopies]; // Writing of TE Overlap stubs

	// Number of data in each input memory
	ap_uint<kNBits_MemAddr> nTotal = 0; // Total number of inputs
	typename InputStubMemory<InType>::NEntryT nInputs[maskISsize]; // Array containing the number of inputs. Last two indices are for DISK2S
	#pragma HLS array_partition variable=nInputs complete dim=0

	const typename InputStubMemory<InType>::NEntryT zero(0);


	/////////////////////////////////////
	// Main Loop

	TOPLEVEL: for (auto i = -1; i < kMaxProc - 1; ++i) {
#pragma HLS PIPELINE II=1 rewind

		// Only for the first loop iteration, no stub processed:
		// - Clear all memories and counters
		// - Calculate the number of entries in the input memories
		// Allows the TOPLEVEL loop to use REWIND and process kMaxProc - 1 stubs
		if (i == -1) {

			// Reset address counters in output memories
			// Only clear if the masks says that memory is used
			static const ap_uint<MaxAllCopies> maskAS = (1 << MaxAllCopies) - 1; // Binary number corresponding to 'MaxAllCopies' of 1s
			clear1DMemoryArray(bx, MaxAllCopies, maskAS, 0, memoriesAS);

			if (maskME) {
				clear1DMemoryArray(bx, nvmME, maskME, firstME, memoriesME);
			}
			if (maskTEI) {
				clear2DMemoryArray<MaxTEICopies>(bx, nvmTE, maskTEI, firstTE, memoriesTEI);
			}
			if (maskTEO) {
				clear2DMemoryArray<MaxTEOCopies>(bx, nvmTE, maskTEO, firstTE, memoriesTEO);
			}
			if (maskOL) {
				clear2DMemoryArray<MaxOLCopies>(bx, nvmOL, maskOL, firstOL, memoriesOL);
			}

			// Set all address counters to 0
			if (maskTEI) {
				clear2DArray<MaxTEICopies>(nvmTE, addrCountTEI);
			}
			if (maskOL) {
				clear2DArray<MaxOLCopies>(nvmOL, addrCountOL);
			}

			// Number of data in each input memory
			for (int j = 0; j < maskISsize; j++) {
#pragma HLS UNROLL
				ap_uint<kNBits_MemAddr> tmp;
				if (j < 4) {
					tmp = maskIS[j] != 0 ? inputStubs[j].getEntries(bx) : zero;
				} else { // For DISK2S
					tmp = maskIS[j] != 0 ? inputStubsDisk2S[j-4].getEntries(bx) : zero;
				}
				nInputs[j] = tmp;
				nTotal += tmp;
			}

			continue;
		}

		// Stop processing stubs if we have gone through all data
		if (!nTotal)
			continue;

		bool resetNext = false; // Used to reset read_addr
		bool disk2S = false; // Used to determine if DISK2S
		bool negDisk = false; // Used to determine if it's negative disk, the last 3 inputs memories

		InputStub<InType> stub;
		InputStub<DISK2S> stubDisk2S; // Used for disks. TODO: Find a better way to do this...?

		// Read stub from memory in turn.
		// Reading is ordered as in wiring script to pass testbench
		if (nInputs[4]) { // For DISK2S
			assert(Disk);
			stubDisk2S = inputStubsDisk2S[0].read_mem(bx, read_addr);
			disk2S = true;
			--nInputs[4];
			if (nInputs[4] == 0)
				resetNext = true;
		} else if (nInputs[0]) {
			stub = inputStubs[0].read_mem(bx, read_addr);
			--nInputs[0];
			if (nInputs[0] == 0)
				resetNext = true;
		} else if (nInputs[1]) {
			stub = inputStubs[1].read_mem(bx, read_addr);
			--nInputs[1];
			if (nInputs[1] == 0)
				resetNext = true;
		} else if (nInputs[5]) { // For DISK2S
			assert(Disk);
			stubDisk2S = inputStubsDisk2S[1].read_mem(bx, read_addr);
			disk2S = true;
			negDisk = (Disk) ? true : false;
			--nInputs[5];
			if (nInputs[5] == 0)
				resetNext = true;
		} else if (nInputs[2]) {
			stub = inputStubs[2].read_mem(bx, read_addr);
			negDisk = (Disk) ? true : false;
			--nInputs[2];
			if (nInputs[2] == 0)
				resetNext = true;
		} else if (nInputs[3]) {
			stub = inputStubs[3].read_mem(bx, read_addr);
			negDisk = (Disk) ? true : false;
			--nInputs[3];
			if (nInputs[3] == 0)
				resetNext = true;
		}

		--nTotal;

		// Increment the read address, or reset it to zero when all stubs in a memory has been read
		if (resetNext)
			read_addr = 0;
		else
			++read_addr;


		////////////////////////////////////////
		// AllStub memories

		AllStub<OutType> allstub =
				(disk2S) ? stubDisk2S.raw() : stub.raw();

		// Write stub to all memory copies
		for (int n = 0; n < MaxAllCopies; n++) {
#pragma HLS UNROLL
			memoriesAS[n].write_mem(bx, allstub, i);
		}

		// For debugging
#ifndef __SYNTHESIS__
		std::cout << "Output stub: " << std::hex << allstub.raw() << std::dec << std::endl;
#endif // DEBUG


		/////////////////////////////////////////////
		// ME memories

		if (maskME != 0) {

			// Virtual modules to write to
			int ivmPlus;
			int ivmMinus;

			int bin; // Coarse z. The bin the stub is going to be put in, in the memory

			// Create the ME stub to save
			VMStubME<OutType> stubME = (disk2S) ?
					createStubME<DISK2S, OutType, Layer, Disk>(stubDisk2S, i, negDisk, fineBinTable, phiCorrTable, ivmPlus, ivmMinus, bin) :
					createStubME<InType, OutType, Layer, Disk>(stub, i, negDisk, fineBinTable, phiCorrTable, ivmPlus, ivmMinus, bin);;

// For debugging
#ifndef __SYNTHESIS__
			std::cout << "ME stub " << std::hex << stubME.raw() << std::endl;
			std::cout << "ivm Minus,Plus = " << std::dec << ivmMinus << " " << ivmPlus << " " << "\t0x"
					<< std::setfill('0') << std::setw(4) << std::hex
					<< stubME.raw().to_int() << std::dec << ", to bin " << bin << std::endl;
			if (!maskME[ivmPlus]) {
				std::cerr << "Trying to write to non-existent memory for ivm = " << ivmPlus << std::endl;
					}
			if (!maskME[ivmMinus]) {
				std::cerr << "Trying to write to non-existent memory for ivm = " << ivmMinus << std::endl;
			}
#endif // DEBUG

		// Write the ME stub to the correct memory.
		// If stub is close to a border (ivmPlus != ivmMinus)
		// write it to the adjacent memory as well
		// #pragma HLS dependence variable=memoriesME intra false
		for (int n = 0; n < maxvmbins; n++) {
#pragma HLS UNROLL
			if (maskME[n]) {
					if ((ivmMinus == n) || (ivmPlus == n))
						memoriesME[n-firstME].write_mem(bx, bin, stubME);
				}
			}
		} // End ME memories


		//////////////////////////////////
		// TE Inner Memories

		// No stubs for DISK2S
		if ((maskTEI != 0) && (!disk2S)) {

			int ivm;// Which VM to write to

			// The z/r information bits saved for TE Inner memories.
			// Which VMs to look at in the outer layer.
			// Note: not the z/r coordinate for the inner stub
			// Called binlookup in emulation
			int rzbits;

			// Create the TE Inner stub to save
			VMStubTEInner<OutType> stubTE = createStubTEInner<InType, OutType, Layer, Disk>(stub, i, negDisk, rzbitsInnerTable, phiCorrTable, ivm, rzbits);

// For debugging
#ifndef __SYNTHESIS__
			std::cout << "TEInner stub " << std::hex << stubTE.raw()
					<< std::endl;
			std::cout << "ivm: " << std::dec << ivm <<std::endl
					<< std::endl;
#endif // DEBUG

			// Write the TE Inner stub to the correct memory
			// Only if it has a valid rzbits/binlookup value, less than maxrz/1023 (table uses 1048575 as "-1"),
			// and a valid bend
			if ((rzbits < maxrz) && maskTEI[ivm]) {
				int memIndex = ivm-firstTE; // Index for the correct memory in memory array
				int bendIndex = memIndex*MaxTEICopies; // Index for bendcut LUTs
				for (int n = 0; n < MaxTEICopies; n++) {
#pragma HLS UNROLL
					bool passBend = bendCutInnerTable[bendIndex][stubTE.getBend()];
					if (passBend) {
						memoriesTEI[memIndex][n].write_mem(bx, stubTE, addrCountTEI[memIndex][n]);
						addrCountTEI[memIndex][n] += 1; // Count the memory addresses we have written to
					}
					bendIndex++; // Use next bendcut table for the next memory "copy"
				}
			}
		} // End TE Inner memories


		////////////////////////////////////
		// TE Outer memories

		if ((maskTEO != 0) && (!disk2S)) {

			int ivm; // The VM number
			int bin; // Coarse z. The bin the stub is going to be put in, in the memory

			// Create the TE Inner stub to save
			VMStubTEOuter<OutType> stubTE = createStubTEOuter<InType, OutType, Layer, Disk>(stub, i, negDisk, rzbitsOuterTable, phiCorrTable, ivm, bin);

// For debugging
#ifndef __SYNTHESIS__
			std::cout << "TEOuter stub " << std::hex << stubTE.raw()
					<< std::endl;
			std::cout << "ivm: " << std::dec << ivm << "       to bin " << bin << std::endl
					<< std::endl;
#endif // DEBUG

			// Write the TE Outer stub to the correct memory
			// Only if it has a valid bend
			if (maskTEO[ivm]) {
				int memIndex = ivm-firstTE; // Index for the correct memory in memory array and address
				int bendIndex = memIndex*MaxTEOCopies; // Index for bendcut LUTs
				for (int n = 0; n < MaxTEOCopies; n++) {
#pragma HLS UNROLL
					bool passBend = bendCutOuterTable[bendIndex][stubTE.getBend()]; // Check if stub passes bend cut
					if (passBend) {
						memoriesTEO[memIndex][n].write_mem(bx, bin, stubTE);
					}
					bendIndex++; // Use next bendcut table for the next memory "copy"
				}
			}
		} // End TE Outer memories


		/////////////////////////////////////
		// OVERLAP Memories

		if (maskOL != 0) {

			assert(Layer == 1 || Layer == 2); // Make sure that only run layer 1 and 2

			int ivm; // Which VM to write to

			// The z/r information bits saved for TE Inner memories.
			// Which VMs to look at in the outer layer.
			// Note: not the z/r coordinate for the inner stub
			// Called binlookup in emulation
			int rzbits;

			// Create the TE Inner Overlap stub to save
			VMStubTEInner<BARRELOL> stubOL = createStubTEOverlap<InType, Layer>(stub, i, rzbitsOverlapTable, phiCorrTable, ivm, rzbits);

// For debugging
#ifndef __SYNTHESIS__
			std::cout << "Overlap stub " << " " << std::hex
					<< stubOL.raw() << std::endl;
			std::cout << "ivm: " << std::dec << ivm << std::endl
					<< std::endl;
#endif // DEBUG

			// Save stub to Overlap memories
			// maxrz is like "-1" if we had signed stuff...
			if (maskOL[ivm] && (rzbits < maxrz)) {
				int memIndex = ivm - firstOL; // The memory index in array and addrcount
				int bendIndex = memIndex*MaxOLCopies; // Index for bendcut LUTs
				for (int n = 0; n < MaxOLCopies; n++) {
#pragma HLS UNROLL
					bool passBend = bendCutOverlapTable[bendIndex][stubOL.getBend()];
					if (passBend) {
						memoriesOL[memIndex][n].write_mem(bx, stubOL, addrCountOL[memIndex][n]);
						addrCountOL[memIndex][n] += 1;
					}
					bendIndex++;
				}
			}

// For debugging
#ifndef __SYNTHESIS__
			else {
				std::cout << "NO OVERLAP" << std::endl << std::endl;
			}
#endif // DEBUG

		} // End TE Overlap memories
	} // Outside main loop
} // End VMRouter

#endif // TrackletAlgorithm_VMRouterTop_h
