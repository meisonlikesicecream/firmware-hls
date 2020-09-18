#ifndef TrackletAlgorithm_Constants_h
#define TrackletAlgorithm_Constants_h

#include "ap_int.h"

//#include <cmath>
//#include "hls_math.h"

constexpr int kMaxStubsFromLink = 256;

constexpr int kTMUX = 18;   //For hourglass project
constexpr int kMaxProc = kTMUX * 6;

// Memory
constexpr int kNBits_MemAddr = 7;
constexpr int kMemDepth = (1<<kNBits_MemAddr);
constexpr int kNBits_BX = 3;

constexpr int kNBits_MemAddrBinned = 4;
constexpr int kMemDepthBinned = (1<<kNBits_MemAddrBinned);

constexpr int kNBits_z = 3;

constexpr int MEBinsBits = 3;
constexpr int TEBinsBits = 3;

// physical constants
constexpr double c = 0.299792458; // m/ns

// stub digitization constants
constexpr double kr = 0.0292969;
constexpr double kphi = 7.71867e-06;
constexpr double kz = 0.0585938;

// tracklet digitization constants
constexpr double krinv = 1.02916e-06;
constexpr double kphi0 = 1.54373e-05;
constexpr double kt = 0.00195312;
constexpr double kz0 = 0.0585938;

// layer projection digitization constants
constexpr double kphiproj456 = 7.71867e-06;
constexpr double kphider = 8.23325e-06;
constexpr double kzproj = 0.0585938;
constexpr double kzder = 0.015625;

// disk projection digitization constants
constexpr double kphiprojdisk = 6.17494e-05;
constexpr double kphiprojderdisk = 1.64665e-05;
constexpr double krprojdisk = 0.0585938;
constexpr double krprojderdisk = 0.0078125;

// detector constants
constexpr double bfield = 3.8112; // T
constexpr int rmean[6] = { 851, 1269, 1784, 2347, 2936, 3697 };
constexpr int zmean[5] = { 2239, 2645, 3163, 3782, 4523 };
constexpr double zlength = 120.0;
constexpr double rmindiskvm = 22.5;
constexpr double rmaxdisk = 120.0;

// cut constants
constexpr double ptcut = 1.91;
constexpr double rinvcut = 0.01 * c * bfield / ptcut;
constexpr double z0cut = 15.0;

// List of regions for memory template parameters
enum regionType {BARRELPS, BARREL2S, BARRELOL, BARREL, DISKPS, DISK2S, DISK};

// Global BX type
typedef ap_uint<kNBits_BX> BXType;  // temporary definition. need to be revisited

#endif
