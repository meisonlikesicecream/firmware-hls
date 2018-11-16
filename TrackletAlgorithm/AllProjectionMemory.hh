#ifndef ALLPROJECTION_HH
#define ALLPROJECTION_HH

#include "Constants.hh"
#include "MemoryTemplate.hh"

constexpr int NBits_aptcid = 13;
constexpr int NBits_apphi = 14;
constexpr int NBits_apz = 12;
constexpr int NBits_apphid = 11;
constexpr int NBits_apzd = 10;
constexpr int NBits_apdata =
  1 + 1 + NBits_aptcid + NBits_apphi + NBits_apz + NBits_apphid + NBits_apzd;

// Data object definition
class AllProjection
{
public:

  typedef ap_uint<NBits_aptcid> AProjTCID;
  typedef ap_uint<NBits_apphi> AProjPHI;
  typedef ap_int<NBits_apz> AProjZ;
  typedef ap_int<NBits_apphid> AProjPHIDER;
  typedef ap_int<NBits_apzd> AProjZDER;
  
  typedef ap_uint<NBits_apdata> AllProjData;
  
  // Constructors
  AllProjection(AllProjData newdata):
    data_(newdata)
  {}

  AllProjection(bool plusneighbor, bool minusneighbor, AProjTCID tcid,
                AProjPHI phi, AProjZ z, AProjPHIDER phider, AProjZDER zder):
    data_(((((((plusneighbor,minusneighbor),tcid),phi),z),phider),zder))
  {}
  
  AllProjection():
    data_(0)
  {}

  AllProjection(const char* datastr, int base = 16)
  {
    AllProjData newdata(datastr, base);
    data_ = newdata;
  }
  
  // copy constructor
  AllProjection(const AllProjection& rhs):
    data_(rhs.raw())
  {}

  // Getter
  AllProjData raw() const {return data_;}
  
  bool IsPlusNeighbor() const {return data_.range(61,61);}
  bool IsMinusNeighbor() const {return data_.range(60,60);}
  AProjTCID GetTrackletIndex() const {return data_.range(59,47);}
  AProjPHI GetPhi() const {return data_.range(46,33);}
  AProjZ GetZ() const {return data_.range(32,21);}
  AProjPHIDER GetPhiDer() {return data_.range(20,10);}
  AProjZDER GetZDer() {return data_.range(9,0);}

  // Setter
  void SetIsPlusNeighbor(bool isplusneighbor)
  {
    data_.range(61,61) = isplusneighbor;
  }

  void SetIsMinusNeighbor(bool isminusneighbor)
  {
    data_.range(60,60) = isminusneighbor;
  }

  void SetTrackletIndex(AProjTCID id)
  {
    data_.range(59,47) = id;
  }

  void SetPhi(AProjPHI phi)
  {
    data_.range(46,33) = phi;
  }

  void SetZ(AProjZ z)
  {
    data_.range(32,21) = z;
  }

  void SetPhiDer(AProjPHIDER phider)
  {
    data_.range(20,10) = phider;
  }

  void SetZDer(AProjZDER zder)
  {
    data_.range(9,0) = zder;
  }

private:

  AllProjData data_;
  
};

// Memory definition
typedef MemoryTemplate<AllProjection, 3, kNBits_MemAddr> AllProjectionMemory;
// FIXME: double check number of bits for bx and for memory address

#endif