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

constexpr unsigned int nvmteoverlaplayers[2] = { 2, 2 }; // Number of Overlap VM modules per coarse phi region

// Number of bits used for the VMs for different layers and disks
// E.g. 32 VMs would use 5 vmbits
constexpr int nvmbitslayer[6] = { 5, 5, 4, 5, 4, 5 }; // Could be computed using the number of VMs...
constexpr int nvmbitsdisk[5] = { 4, 4, 4, 4, 4 };
constexpr int nvmbitsoverlap[2] = { 4, 3 };

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

constexpr int nzbitsrzbitsoverlaptable = 7;
constexpr int nrbitsrzbitsoverlaptable = 3;

// Number of MSBs used for r index in phicorr LUTs
constexpr int nrbitsphicorrtable = 3; // Found hardcoded in VMRouterphicorrtable.h

// Constants used for calculating which VM a stub belongs to
constexpr int nmaxvmbits = 5; // Maximum number of bits used for the VM number, i.e. 32
constexpr int nmaxvmolbits = 4; // Overlap

constexpr float maxvmbins = 1 << nmaxvmbits; // How many bins nmaxvmbits would correspond to
constexpr float maxvmolbins = 1 << nmaxvmolbits; // Overlap

constexpr int nphibitsraw = 7; // Number of bits used for calculating iPhiRawPlus/Minus

// The length of the masks used for the memories
constexpr int inmasksize = 6; // Input memories
constexpr int memasksize = 1 << nmaxvmbits; // ME memories
constexpr int teimasksize = 1 << nmaxvmbits; // TEInner memories
constexpr int olmasksize = 1 << nmaxvmolbits; // TEInner Overlap memories
constexpr int teomasksize = 1 << nmaxvmbits; // TEOuter memories

// Some ugly correction constants to make the overall II = 108... TODO: Find better way to do this?
constexpr int kMaxProcLayerCorr = 6;
constexpr int kMaxProcDiskCorr = 7;

// Constants for determining if the stub should be saved using the rzbitstables
constexpr int maxrzbits = 10; // Maximum number of bits used for each entry in the rzbits tables
constexpr int maxrz = (1 << maxrzbits) - 1; // Anything above this value would correspond to -1, i.e. a non-valid stub



//////////////////////////////////////
// Functions used by the VMR

// Returns top 5 (nmaxvmbits) bits of phi, i.e. max 31 in decimal
template<regionType InType>
inline ap_uint<nmaxvmbits> iphivmRaw(const typename AllStub<InType>::ASPHI phi) {

	ap_uint<nmaxvmbits> iphivm = phi.range(phi.length() - 1, phi.length() - nmaxvmbits);

	return iphivm;
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

// See above
template<regionType InType>
inline ap_uint<nmaxvmbits> iphivmRawMinus(const typename AllStub<InType>::ASPHI phi) {

	ap_uint<nphibitsraw> tmp(phi.range(phi.length() - 1, phi.length() - nphibitsraw));
	--tmp;
	ap_uint<nmaxvmbits> minus(tmp.range(tmp.length() - 1, nphibitsraw - nmaxvmbits));

	return minus;
}

// Get the corrected phi, i.e. phi at the average radius of the barrel
// Corrected phi is used by ME and TE memories in the barrel
template<regionType InType>
inline typename AllStub<InType>::ASPHI getPhiCorr(
		const typename AllStub<InType>::ASPHI phi,
		const typename AllStub<InType>::ASR r,
		const typename AllStub<InType>::ASBEND bend, const int phicorrtable[]) {

	if (InType == DISKPS || InType == DISK2S)
		return phi; // Do nothing if disks

	constexpr auto rbins = 1 << nrbitsphicorrtable; // The number of bins for r

	ap_uint<nrbitsphicorrtable> rbin = (r + (1 << (r.length() - 1)))
			>> (r.length() - nrbitsphicorrtable); // Which bin r belongs to. Note r = 0 is mid radius
	auto index = bend * rbins + rbin; // Index for where we find our correction value
	auto corrval = phicorrtable[index]; // The amount we need to correct our phi

	auto phicorr = phi - corrval; // the corrected phi

	// Check for overflow
	if (phicorr < 0)
		phicorr = 0; // can't be less than 0
	if (phicorr >= 1 << phi.length())
		phicorr = (1 << phi.length()) - 1;  // can't be more than the max value

	return phicorr;
}

// Get the number of the first ME/TE memory for the current VMRouter
// I.e. the position of the first non-zero bit in the mask
// L1PHIE17 would return 16
inline ap_uint<nmaxvmbits> getFirstMemNumber(const ap_uint<static_cast<int>(maxvmbins)> mask) {
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
void clear1DMemoryArray(const BXType bx, const int arraysize, const MaskType mask, const int firstmem, MemType memArray[]) {
	#pragma HLS inline
	for (int i = 0; i < arraysize; i++) {
		#pragma HLS UNROLL
		// Only clear the memory if it is used by the VMR (defined by the mask)
		if (mask[i + firstmem]) memArray[i].clear(bx);
	}
}

// Clears the memories of two-dimensional memory arrays
template<int MaxCopies, class MaskType, class MemType>
void clear2DMemoryArray(const BXType bx, const int nvm, const MaskType mask, const int firstmem, MemType memArray[][MaxCopies]) {
	#pragma HLS inline
	for (int i = 0; i < nvm; i++) {
		#pragma HLS UNROLL
		// Only clear the memory if it is used by the VMR (defined by the mask)
		if (mask[i + firstmem]) {
			for (int j = 0; j < MaxCopies; j++) {
				#pragma HLS UNROLL
				memArray[i][j].clear(bx);
			}
		}
	}
}

// Clears a 2D array of ints by setting everything to 0
template<int MaxCopies>
void clear2DArray(int nvm, int array[][MaxCopies]) {
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
		const int index, const bool negdisk, const int finebintable[],
		const int phicorrtable[], int& ivmPlus, int& ivmMinus, int& bin) {

	// The MEStub that is going to be returned
	VMStubME<OutType> stubme;

	// Values from InputStub
	auto z = stub.getZ();
	auto r = stub.getR();
	auto bend = stub.getBend();
	auto phi = stub.getPhi();
	auto phicorr = getPhiCorr<InType>(phi, r, bend, phicorrtable); // Corrected phi, i.e. phi at nominal radius (what about disks?)

	int nrbits = r.length(); // Number of bits for r
	int nzbits = z.length(); // Number of bits for z
	int nbendbits = bend.length(); // Number of bits for bend

	// Some variables
	constexpr int nbitszfinebintable =
			(Layer) ? nbitszfinebintablelayer : nbitszfinebintabledisk; // Number of MSBs of z used in finebintable
	constexpr int nbitsrfinebintable =
			(Layer) ? nbitsrfinebintablelayer : nbitsrfinebintabledisk; // Number of MSBs of r used in finebintable
	static const int nfinerzbits = stubme.getFineZ().length(); // Number of bits for finer/z

	// Total number of VMs for ME for a layer/disk in a whole sector
	constexpr int ntotvmme =
			Layer != 0 ?
					nallstubslayers[Layer - 1] * nvmmelayers[Layer - 1] :
					nallstubsdisks[Disk - 1] * nvmmedisks[Disk - 1];

	// Some sort of normalisation thing used for determining which VM the stub belongs to
	static const ap_ufixed<nmaxvmbits, nmaxvmbits-1> d_me = ntotvmme / maxvmbins;


	// Set values to VMStubME

	stubme.setBend(bend);
	stubme.setIndex(typename VMStubME<OutType>::VMSMEID(index));

	auto iphiRaw = iphivmRaw<InType>(phicorr); // Top 5 bits of phi
	auto iphiRawPlus = iphivmRawPlus<InType>(phicorr); // Top 5 bits of phi after adding a small number
	auto iphiRawMinus = iphivmRawMinus<InType>(phicorr); // Top 5 bits of phi after subtracting a small number

	int ivm = iphiRaw * d_me; // The VM number
	ivmPlus = iphiRawPlus * d_me;
	ivmMinus = iphiRawMinus * d_me;

	// To avoid overflow
	if (ivmMinus > ivm)
		ivmMinus = 0;
	if (ivmPlus < ivm)
		ivmPlus = ntotvmme - 1;

	// Stubs can only end up in the neighbouring VM after calculating iphivmrawplus/minus
	assert(std::abs(ivmMinus - ivmPlus) <= 1);

	// Indices used to find the rzfine value in finebintable
	// finebintable returns the top 6 bits of a corrected z
	// Note: not the index that is being saved to the stub
	ap_uint<nbitszfinebintable + nbitsrfinebintable> indexz = ((z
			+ (1 << (nzbits - 1))) >> (nzbits - nbitszfinebintable)); // Make z unsigned and take the top "nbitszfinebintable" bits
	ap_uint<nbitsrfinebintable> indexr = 0;

	if (Disk) {
		if (negdisk) {
			indexz = (1 << nbitszfinebintable) - indexz;
		}
		indexr = r >> (nrbits - nbitsrfinebintable); // Take the top "nbitsrfinebintable" bits
	} else { // Layer
		indexr = ((r + (1 << (nrbits - 1)))
				>> (nrbits - nbitsrfinebintable)); // Make r unsigned and take the top "nbitsrfinebintable" bits
	}

	// The index for finebintable
	ap_uint<nbitszfinebintable + nbitsrfinebintable> finebinindex = (indexz
			<< nbitsrfinebintable) + indexr;

	// Get the corrected r/z position
	auto rzcorr = finebintable[finebinindex];

	// Coarse z. The bin the stub is going to be put in, in the memory
	bin = rzcorr >> nfinerzbits; // 3 bits, i.e. max 8 bins within each VM
	if (negdisk)
		bin += 1 << MEBinsBits; // bin 8-16 are for negative disks

	// Set rzfine, i.e. the r/z bits within a coarse r/z region
	auto rzfine = rzcorr & ((1 << nfinerzbits) - 1); // the 3 LSB as rzfine
	stubme.setFineZ(rzfine);

	assert(rzfine >= 0);

	return stubme;
};


// Returns a TE Inner stub with all the values set
template<regionType InType, regionType OutType, int Layer, int Disk>
inline VMStubTEInner<OutType> createStubTEInner(const InputStub<InType> stub,
		const int index, const bool negdisk, const int rzbitsinnertable[],
		const int phicorrtable[], int& ivm, int& rzbits) {

	// The TEInner Stub that is going to be returned
	VMStubTEInner<OutType> stubte;

	// Values from InputStub
	auto z = stub.getZ();
	auto r = stub.getR();
	auto bend = stub.getBend();
	auto phi = stub.getPhi();
	auto phicorr = getPhiCorr<InType>(phi, r, bend, phicorrtable); // Corrected phi, i.e. phi at nominal radius (what about disks?)

	int nrbits = r.length(); // Number of bits for r
	int nzbits = z.length(); // Number of bits for z
	int nbendbits = bend.length(); // Number of bits for bend

	// Some variables
	constexpr auto vmbits =
			(Layer) ? nvmbitslayer[Layer - 1] : nvmbitsdisk[Disk - 1]; // Number of bits for VMs
	static const auto finephibits = stubte.getFinePhi().length();  // Number of bits for finephi

	// Total number of VMs for TE in a whole sector
	constexpr int ntotvmte =
			Layer != 0 ?
					nallstubslayers[Layer - 1] * nvmtelayers[Layer - 1] :
					nallstubsdisks[Disk - 1] * nvmtedisks[Disk - 1];

	// Some sort of normalisation thing used for determining which VM the stub belongs to
	static const ap_ufixed<nmaxvmbits, nmaxvmbits-1> d_te = ntotvmte / maxvmbins;


	// Set values to VMStubeTEInner

	stubte.setBend(bend);
	stubte.setIndex(typename VMStubTEInner<OutType>::VMSTEIID(index));

	auto iphiRaw = iphivmRaw<InType>(phicorr); // Top 5 bits of phi

	ivm = iphiRaw * d_te; // The VM number

	// Layer
	if (Layer != 0) {

		constexpr auto zbins = (1 << nzbitsinnertablelayer); // Number of bins in z
		constexpr auto rbins = (1 << nrbitsinnertablelayer); // Number of bins in r
		ap_uint<nzbitsinnertablelayer> zbin = (z + (1 << (nzbits - 1)))
				>> (nzbits - nzbitsinnertablelayer); // Make z positive and take the 7 (nzbitsinnertablelayer) MSBs
		ap_uint<nrbitsinnertablelayer> rbin = (r + (1 << (nrbits - 1)))
				>> (nrbits - nrbitsinnertablelayer);

		int rzbitsindex = zbin * rbins + rbin; // Index for rzbits LUT

		rzbits = rzbitsinnertable[rzbitsindex]; // The z/r information bits saved for TE Inner memories.

		stubte.setZBits(rzbits);
		stubte.setFinePhi(
				iphivmFineBins<InType>(phicorr, vmbits, finephibits));

	} else { // Disk
		assert(Disk != 0);

		constexpr int zbins = (1 << nzbitsinnertabledisk); // Number of bins in z
		constexpr int rbins = (1 << nrbitsinnertabledisk); // Number of bins in r
		ap_uint<nzbitsinnertabledisk> zbin = (z + (1 << (nzbits - 1)))
				>> (nzbits - nzbitsinnertabledisk); // Make z positive and take the 7 (nzbitsinnertabledisk) MSBs
		ap_uint<nrbitsinnertabledisk> rbin = r
				>> (nrbits - nrbitsinnertabledisk);

	// Special case if negative disk
		if (negdisk)
			zbin = zbins - 1 - zbin; // Inverted for negative disks

		int rzbitsindex = rbin * zbins + zbin; // Index for rzbits LUT

		rzbits = rzbitsinnertable[rzbitsindex]; // The z/r information bits saved for TE Inner memories.

		stubte.setZBits(rzbits);
		stubte.setFinePhi(
				iphivmFineBins<InType>(phicorr, vmbits, finephibits));
	}

	return stubte;
}


// Returns a TE Outer stub with all the values set
template<regionType InType, regionType OutType, int Layer, int Disk>
inline VMStubTEOuter<OutType> createStubTEOuter(const InputStub<InType> stub,
		const int index, const bool negdisk, const int rzbitsoutertable[],
		const int phicorrtable[], int& ivm, int& bin) {

	// The TEOuter stub that is going to be returned
	VMStubTEOuter<OutType> stubte;

	// Values from InputStub
	auto z = stub.getZ();
	auto r = stub.getR();
	auto bend = stub.getBend();
	auto phi = stub.getPhi();
	auto phicorr = getPhiCorr<InType>(phi, r, bend, phicorrtable); // Corrected phi, i.e. phi at nominal radius

	int nrbits = r.length(); // Number of bits for r
	int nzbits = z.length(); // Number of bits for z
	int nbendbits = bend.length(); // Number of bits for bend

	// Some variables
	constexpr auto vmbits =
			(Layer) ? nvmbitslayer[Layer - 1] : nvmbitsdisk[Disk - 1]; // Number of bits for VMs
	static const int finephibits = stubte.getFinePhi().length(); // Number of bits for finephi
	static const int nfinerzbits = stubte.getFineZ().length(); // Number of bits for finer/z

	// Total number of VMs for TE in a whole sector
	constexpr int ntotvmte =
			Layer != 0 ?
					nallstubslayers[Layer - 1] * nvmtelayers[Layer - 1] :
					nallstubsdisks[Disk - 1] * nvmtedisks[Disk - 1];

	// Some sort of normalisation thing used for determining which VM the stub belongs to
	static const ap_ufixed<nmaxvmbits, nmaxvmbits-1> d_te = ntotvmte / maxvmbins;


	// Set values to VMSTubTE Outer

	stubte.setBend(bend);
	stubte.setIndex(typename VMStubTEOuter<OutType>::VMSTEOID(index));

	auto iphiRaw = iphivmRaw<InType>(phicorr); // Top 5 bits of phi
	ivm = iphiRaw * d_te; // The VM number

	// Layer
	if (Layer != 0) {

		stubte.setFinePhi(
				iphivmFineBins<InType>(phicorr, vmbits, finephibits)); // is this the right vmbits

		constexpr auto zbins = (1 << nzbitsoutertablelayer); // Number of bins in z
		constexpr auto rbins = (1 << nrbitsoutertablelayer); // Number of bins in r
		ap_uint<nzbitsoutertablelayer> zbin = (z + (1 << (nzbits - 1)))
				>> (nzbits - nzbitsoutertablelayer); // Make z positive and take the 7 MSBs
		ap_uint<nrbitsoutertablelayer> rbin = (r + (1 << (nrbits - 1)))
				>> (nrbits - nrbitsoutertablelayer);

		// Find the VM bit information in rzbits LUT
		// First 3 MSB is the binning in z, and the 3 LSB is the fine z within a bin
		auto rzbitsindex = zbin * rbins + rbin; // number of bins
		auto rzbits = rzbitsoutertable[rzbitsindex];

		bin = rzbits >> nfinerzbits; // Remove the 3 LSBs, i.e. the finebin bits

		// Set fine z
		auto zfine = rzbits & nfinerzbits;
		stubte.setFineZ(zfine);

	} else { // Disks
		assert(Disk != 0);

		stubte.setFinePhi(
				iphivmFineBins<InType>(phicorr, vmbits, finephibits));

		constexpr auto zbins = (1 << nzbitsoutertabledisk); // Number of bins in z
		constexpr auto rbins = (1 << nrbitsoutertabledisk); // Number of bins in r
		ap_uint<nzbitsoutertabledisk> zbin = (z + (1 << (nzbits - 1)))
				>> (nzbits - nzbitsoutertabledisk); // Make z positive and take the 7 MSBs
		ap_uint<nrbitsoutertabledisk> rbin = r
				>> (nrbits - nrbitsoutertabledisk);

		// Special case if negative disk
		if (negdisk) {
			zbin = zbins - 1 - zbin; // Inverted for negative disk
		}

		// Find the VM bit information in rzbits LUT
		// First 2 MSB is the binning in r, and the 3 LSB is the fine r within a bin
		auto rzbitsindex = rbin * zbins + zbin; // Index for LUT
		ap_uint<nmaxvmbits> rzbits = rzbitsoutertable[rzbitsindex];

		bin = rzbits >> nfinerzbits; // Remove the 3 LSBs, i.e. the finebin bits

		// Half the bins, i.e. bin 4-7, are used for negative disks
		if (negdisk)
			bin += (1 << MEBinsBits)/2; // += 4

		// Set fine r
		auto rfine = rzbits & ((1 << nfinerzbits) - 1); // Take the 3 (nfinerzbits) LSBs
		stubte.setFineZ(rfine);
	}

	return stubte;
}


// Returns a TE Overlap stub with all the values set
template<regionType InType, int Layer>
inline VMStubTEInner<BARRELOL> createStubTEOverlap(const InputStub<InType> stub,
		const int index, const int rzbitsoverlaptable[],
		const int phicorrtable[], int& ivm, int& rzbits) {

	// The overlap stub that is going to be returned
	VMStubTEInner<BARRELOL> stubol;

	// Values from InputStub
	auto z = stub.getZ();
	auto r = stub.getR();
	auto bend = stub.getBend();
	auto phi = stub.getPhi();
	auto phicorr = getPhiCorr<InType>(phi, r, bend, phicorrtable); // Corrected phi, i.e. phi at nominal radius (what about disks?)

	int nrbits = r.length(); // Number of bits for r
	int nzbits = z.length(); // Number of bits for z
	int nbendbits = bend.length(); // Number of bits for bend

	// Some variables
	constexpr auto ntotvmol =
			(Layer) ? nallstubslayers[Layer - 1] * nvmteoverlaplayers[Layer - 1] : 0; // Total number of VMs for ME in a whole sector
	constexpr auto vmbits = (Layer) ? nvmbitsoverlap[Layer - 1] : 0; // Number of bits used for VMs
	static const auto finephibits = stubol.getFinePhi().length(); // Number of bits used for fine phi

	// Some sort of normalisation thing used for determining which VM the stub belongs to
	static const ap_ufixed<nmaxvmbits, nmaxvmolbits-1> d_ol = ntotvmol / maxvmolbins;

	// Set values to Overlap stub

	// 16 overlap vms per layer
	auto iphiRawOl = iphivmRaw<InType>(phicorr) >> 1; // Top 4 bits of phi

	ivm = iphiRawOl * d_ol; // Which VM it belongs to

	constexpr auto zbins = (1 << nzbitsrzbitsoverlaptable); // Number of bins in z
	constexpr auto rbins = (1 << nrbitsrzbitsoverlaptable); // Number of bins in r
	ap_uint<nzbitsrzbitsoverlaptable> zbin = (z + (1 << (nzbits - 1)))
			>> (nzbits - nzbitsrzbitsoverlaptable); // Make z positive and take the 7 MSBs
	ap_uint<nrbitsrzbitsoverlaptable> rbin = (r + (1 << (nrbits - 1)))
			>> (nrbits - nrbitsrzbitsoverlaptable);

	int rzbitsindex = zbin * rbins + rbin; // Index for rzbitsoverlaptable

	rzbits = rzbitsoverlaptable[rzbitsindex];

	stubol.setBend(bend);
	stubol.setIndex(typename VMStubTEInner<BARRELOL>::VMSTEIID(index));
	stubol.setZBits(rzbits);
	stubol.setFinePhi(
	iphivmFineBins<InType>(phicorr, vmbits,
			finephibits));

	return stubol;
}


/////////////////////////////////
// Main function

// InType DISK2S - Two input region types InType and DISK2S due to the disks having both 2S and PS inputs.
// 		According to wiring script, there's two DISK2S and half the inputs are for negative disks.
// Layer Disk - Specifies the layer or disk number
// MAXCopies - The maximum number of copies of a memory type
// NBitsBin number of bits used for the bins in MEMemories
template<regionType InType, regionType OutType, int Layer, int Disk, int MaxAllCopies, int MaxTEICopies, int MaxOLCopies, int MaxTEOCopies, int NBitsBin, int BendTableSize>
void VMRouter(const BXType bx, const int finebintable[], const int phicorrtable[],
		// rzbitstables, aka binlookup in emulation
		const int rzbitsinnertable[], const int rzbitsoverlaptable[], const int rzbitsoutertable[],
		// bendcut tables
		const ap_uint<BendTableSize> bendinnertable[], const ap_uint<BendTableSize> bendoverlaptable[], const ap_uint<BendTableSize> bendoutertable[],
		// Input memories
		const ap_uint<inmasksize>& inmask,
		const InputStubMemory<InType> inputStub[],
		const InputStubMemory<DISK2S> inputStubDisk2S[],
		// AllStub memory
		AllStubMemory<OutType> allStub[],
		// ME memories
		const ap_uint<memasksize>& memask, VMStubMEMemory<OutType, NBitsBin> meMemories[],
		// Inner TE memories, non-overlap
		const ap_uint<teimasksize>& teimask, VMStubTEInnerMemory<OutType> teiMemories[][MaxTEICopies],
		// TE Inner memories, overlap
		const ap_uint<olmasksize>& olmask, VMStubTEInnerMemory<BARRELOL> olMemories[][MaxOLCopies],
		// TE Outer memories
		const ap_uint<teomasksize>& teomask, VMStubTEOuterMemory<OutType> teoMemories[][MaxTEOCopies]) {

#pragma HLS inline
#pragma HLS array_partition variable=bendinnertable complete dim=1
#pragma HLS array_partition variable=bendoverlaptable complete dim=1
#pragma HLS array_partition variable=bendoutertable complete dim=1
#pragma HLS array_partition variable=inputStub complete dim=1
#pragma HLS array_partition variable=inputStubDisk2S complete dim=1
#pragma HLS array_partition variable=allstub complete dim=1
#pragma HLS array_partition variable=meMemories complete dim=1
#pragma HLS array_partition variable=teiMemories complete dim=2
#pragma HLS array_partition variable=olMemories complete dim=2
#pragma HLS array_partition variable=teoMemories complete dim=2


	// The first memory numbers, the position of the first non-zero bit in the mask
	static const int firstme = getFirstMemNumber(memask); // ME memory
	static const int firstte = (teimask) ? getFirstMemNumber(teimask) : getFirstMemNumber(teomask); // TE memory
	static const int firstol = (olmask) ? static_cast<int>(getFirstMemNumber(olmask)) : 0; // TE Overlap memory

	// Number of memories/VMs for one coarse phi region
	constexpr int nvmme = (Layer) ? nvmmelayers[Layer-1] : nvmmedisks[Disk-1]; // ME memories
	constexpr int nvmte = (Layer) ? nvmtelayers[Layer-1] : nvmtedisks[Disk-1]; // TE memories
	constexpr int nvmol = ((Layer == 1) || (Layer == 2)) ? nvmteoverlaplayers[Layer-1] : 0; // TE Overlap memories

	// Maximum number of stubs that can be processed (memory depth)
	constexpr int MAXVMROUTER = (Layer) ? kMaxProc - kMaxProcLayerCorr : kMaxProc - kMaxProcDiskCorr; // To get overall latency to 108 for barrel... TODO: find better way to do this


	// Reset address counters in output memories
	// Only clear if the masks says that memory is used
	static const ap_uint<MaxAllCopies> allmask = (1 << MaxAllCopies) - 1; // Binary number corresponding to 'MaxAllCopies' of 1s
	clear1DMemoryArray(bx, MaxAllCopies, allmask, 0, allStub);

	if (memask) {
		clear1DMemoryArray(bx, nvmme, memask, firstme, meMemories);
	}
	if (teimask) {
		clear2DMemoryArray<MaxTEICopies>(bx, nvmte, teimask, firstte, teiMemories);
	}
	if (teomask) {
		clear2DMemoryArray<MaxTEOCopies>(bx, nvmte, teomask, firstte, teoMemories);
	}
	if (olmask) {
		clear2DMemoryArray<MaxOLCopies>(bx, nvmol, olmask, firstol, olMemories);
	}


	// Create variables that keep track of which memory address to read and write to
	ap_uint<kNBits_MemAddr> read_addr(0); // Reading of input stubs

	int addrCountTEI[nvmte][MaxTEICopies]; // Writing of TE Inner stubs
	if (teimask) {
		clear2DArray<MaxTEICopies>(nvmte, addrCountTEI);
	}

	int addrCountOL[nvmol][MaxOLCopies]; // Writing of TE Overlap stubs
	if (olmask) {
		clear2DArray<MaxOLCopies>(nvmol, addrCountOL);
	}


	// Number of data in each input memory
	typename InputStubMemory<InType>::NEntryT ninputs[inmasksize]; // Array containing the number of inputs. Last two indices are for DISK2S
	#pragma HLS array_partition variable=ninputs complete dim=0

	const typename InputStubMemory<InType>::NEntryT zero(0);

	ap_uint<7> ntotal = 0; // Total number of inputs

	for (int i = 0; i < inmasksize; i++) {
		#pragma HLS UNROLL
		ap_uint<7> tmp;
		if (i < 4) {
			tmp = inmask[i] != 0 ? inputStub[i].getEntries(bx) : zero;
		} else { // For DISK2S
			tmp = inmask[i] != 0 ? inputStubDisk2S[i-4].getEntries(bx) : zero;
		}
		ninputs[i] = tmp;
		ntotal += tmp;
	}


/////////////////////////////////////
// Main Loop

	TOPLEVEL: for (auto i = 0; i < MAXVMROUTER; ++i) {
#pragma HLS PIPELINE II=1
//#pragma HLS latency max=5
		// Stop processing stubs if we have gone through all data
		if (!ntotal)
			continue;

		bool resetNext = false; // Used to reset read_addr
		bool disk2S = false; // Used to determine if DISK2S
		bool negdisk = false; // Used to determine if it's negative disk, the last 3 inputs memories

		InputStub<InType> stub;
		InputStub<DISK2S> stubDisk2S; // Used for disks. TODO: Find a better way to do this...?

		// Read stub from memory in turn.
		// Reading is ordered as in wiring script to pass testbench
		if (ninputs[4]) { // For DISK2S
			assert(Disk);
			stubDisk2S = inputStubDisk2S[0].read_mem(bx, read_addr);
			disk2S = true;
			--ninputs[4];
			if (ninputs[4] == 0)
				resetNext = true;
		} else if (ninputs[0]) {
			stub = inputStub[0].read_mem(bx, read_addr);
			--ninputs[0];
			if (ninputs[0] == 0)
				resetNext = true;
		} else if (ninputs[1]) {
			stub = inputStub[1].read_mem(bx, read_addr);
			--ninputs[1];
			if (ninputs[1] == 0)
				resetNext = true;
		} else if (ninputs[5]) { // For DISK2S
			assert(Disk);
			stubDisk2S = inputStubDisk2S[1].read_mem(bx, read_addr);
			disk2S = true;
			negdisk = (Disk) ? true : false;
			--ninputs[5];
			if (ninputs[5] == 0)
				resetNext = true;
		} else if (ninputs[2]) {
			stub = inputStub[2].read_mem(bx, read_addr);
			negdisk = (Disk) ? true : false;
			--ninputs[2];
			if (ninputs[2] == 0)
				resetNext = true;
		} else if (ninputs[3]) {
			stub = inputStub[3].read_mem(bx, read_addr);
			negdisk = (Disk) ? true : false;
			--ninputs[3];
			if (ninputs[3] == 0)
				resetNext = true;
		}

		--ntotal;

		// Increment the read address, or reset it to zero when all stubs in a memory has been read
		if (resetNext)
			read_addr = 0;
		else
			++read_addr;


////////////////////////////////////////
// AllStub memories

		AllStub<OutType> allstubd =
				(disk2S) ? stubDisk2S.raw() : stub.raw();

		// Write stub to all memory copies
		for (int n = 0; n < MaxAllCopies; n++) {
			#pragma HLS UNROLL
			allStub[n].write_mem(bx, allstubd, i);
		}

		// For debugging
#ifndef __SYNTHESIS__
		std::cout << "Out put stub: " << std::hex << allstubd.raw() << std::dec << std::endl;
#endif // DEBUG


/////////////////////////////////////////////
// ME memories

		if (memask != 0) {

			// Virtual modules to write to
			int ivmPlus;
			int ivmMinus;

			int bin; // Coarse z. The bin the stub is going to be put in, in the memory

			// Create the ME stub to save
			VMStubME<OutType> stubme = (disk2S) ?
					createStubME<DISK2S, OutType, Layer, Disk>(stubDisk2S, i, negdisk, finebintable, phicorrtable, ivmPlus, ivmMinus, bin) :
					createStubME<InType, OutType, Layer, Disk>(stub, i, negdisk, finebintable, phicorrtable, ivmPlus, ivmMinus, bin);;

// For debugging
#ifndef __SYNTHESIS__
			std::cout << "ME stub " << std::hex << stubme.raw() << std::endl;
			std::cout << "ivm Minus,Plus = " << std::dec << ivmMinus << " " << ivmPlus << " " << "\t0x"
					<< std::setfill('0') << std::setw(4) << std::hex
					<< stubme.raw().to_int() << std::dec << ", to bin " << bin << std::endl;
			if (!memask[ivmPlus]) {
				std::cerr << "Trying to write to non-existent memory for ivm = " << ivmPlus << std::endl;
					}
			if (!memask[ivmMinus]) {
				std::cerr << "Trying to write to non-existent memory for ivm = " << ivmMinus << std::endl;
			}
#endif // DEBUG

		// Write the ME stub to the correct memory.
		// If stub is close to a border (ivmPlus != ivmMinus)
		// write it to the adjacent memory as well
		// #pragma HLS dependence variable=meMemories intra false
		for (int n = 0; n < maxvmbins; n++) {
			#pragma HLS UNROLL
			if (memask[n]) {
					if ((ivmMinus == n) || (ivmPlus == n))
						meMemories[n-firstme].write_mem(bx, bin, stubme);
				}
			}
		} // End ME memories



//////////////////////////////////
// TE Inner Memories

		// No stubs for DISK2S
		if ((teimask != 0) && (!disk2S)) {

			int ivm;// Which VM to write to

			// The z/r information bits saved for TE Inner memories.
			// Which VMs to look at in the outer layer.
			// Note: not the z/r coordinate for the inner stub
			// Called binlookup in emulation
			int rzbits;

			// Create the TE Inner stub to save
			VMStubTEInner<OutType> stubte = createStubTEInner<InType, OutType, Layer, Disk>(stub, i, negdisk, rzbitsinnertable, phicorrtable, ivm, rzbits);

// For debugging
#ifndef __SYNTHESIS__
			std::cout << "TEInner stub " << std::hex << stubte.raw()
					<< std::endl;
			std::cout << "ivm: " << std::dec << ivm <<std::endl
					<< std::endl;
#endif // DEBUG

			// Write the TE Inner stub to the correct memory
			// Only if it has a valid rzbits/binlookup value, less than maxrz/1023 (table uses 1048575 as "-1"),
			// and a valid bend
			if ((rzbits < maxrz) && teimask[ivm]) {
				int memindex = ivm-firstte; // Index for the correct memory in memory array
				int bendindex = memindex*MaxTEICopies; // Index for bendcut LUTs
				for (int n = 0; n < MaxTEICopies; n++) {
					#pragma HLS UNROLL
					bool passbend = bendinnertable[bendindex][stubte.getBend()];
					if (passbend) {
						teiMemories[memindex][n].write_mem(bx, stubte, addrCountTEI[memindex][n]);
						addrCountTEI[memindex][n] += 1; // Count the memory addresses we have written to
					}
					bendindex++; // Use next bendcut table for the next memory "copy"
				}
			}
		} // End TE Inner memories

////////////////////////////////////
// TE Outer memories

		if ((teomask != 0) && (!disk2S)) {

			int ivm; // The VM number
			int bin; // Coarse z. The bin the stub is going to be put in, in the memory

			// Create the TE Inner stub to save
			VMStubTEOuter<OutType> stubte = createStubTEOuter<InType, OutType, Layer, Disk>(stub, i, negdisk, rzbitsoutertable, phicorrtable, ivm, bin);

// For debugging
#ifndef __SYNTHESIS__
			std::cout << "TEOuter stub " << std::hex << stubte.raw()
					<< std::endl;
			std::cout << "ivm: " << std::dec << ivm << "       to bin " << bin << std::endl
					<< std::endl;
#endif // DEBUG

			// Write the TE Outer stub to the correct memory
			// Only if it has a valid bend
			if (teomask[ivm]) {
				int memindex = ivm-firstte; // Index for the correct memory in memory array and address
				int bendindex = memindex*MaxTEOCopies; // Index for bendcut LUTs
				for (int n = 0; n < MaxTEOCopies; n++) {
					#pragma HLS UNROLL
					bool passbend = bendoutertable[bendindex][stubte.getBend()]; // Check if stub passes bend cut
					if (passbend) {
						teoMemories[memindex][n].write_mem(bx, bin, stubte);
					}
					bendindex++; // Use next bendcut table for the next memory "copy"
				}
			}
		} // End TE Outer memories


/////////////////////////////////////
// OVERLAP Memories

		if (olmask != 0) {

			assert(Layer == 1 || Layer == 2); // Make sure that only run layer 1 and 2

			int ivm; // Which VM to write to

			// The z/r information bits saved for TE Inner memories.
			// Which VMs to look at in the outer layer.
			// Note: not the z/r coordinate for the inner stub
			// Called binlookup in emulation
			int rzbits;

			// Create the TE Inner Overlap stub to save
			VMStubTEInner<BARRELOL> stubol = createStubTEOverlap<InType, Layer>(stub, i, rzbitsoverlaptable, phicorrtable, ivm, rzbits);

// For debugging
#ifndef __SYNTHESIS__
			std::cout << "Overlap stub " << " " << std::hex
					<< stubol.raw() << std::endl;
			std::cout << "ivm: " << std::dec << ivm << std::endl
					<< std::endl;
#endif // DEBUG

			// Save stub to Overlap memories
			// maxrz is like "-1" if we had signed stuff...
			if (olmask[ivm] && (rzbits < maxrz)) {
				int memindex = ivm - firstol; // The memory index in array and addrcount
				int bendindex = memindex*MaxOLCopies; // Index for bendcut LUTs
				for (int n = 0; n < MaxOLCopies; n++) {
					#pragma HLS UNROLL
					bool passbend = bendoverlaptable[bendindex][stubol.getBend()];
					if (passbend) {
						olMemories[memindex][n].write_mem(bx, stubol, addrCountOL[memindex][n]);
						addrCountOL[memindex][n] += 1;
					}
					bendindex++;
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
