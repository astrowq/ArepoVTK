/*
 * transfer.h
 * dnelson
 */
 
#ifndef AREPO_RT_TRANSFER_H
#define AREPO_RT_TRANSFER_H

#include "spectrum.h"

// TODO: bit field approach, such that TransferFunction keeps a record of all the 
//       SphP fields it will need to evaluate Lve(), and returns this upon request
//       to AdvanceRayOneCell() which will use it to form vals[] of the needed elements

#define TF_VAL_DENS       0
#define TF_VAL_TEMP       1
#define TF_VAL_VMAG       2
#define TF_VAL_ENTROPY    3
#define TF_VAL_METAL      4
#define TF_VAL_SZY        5
#define TF_VAL_XRAY       6
#define TF_VAL_BMAG       7
#define TF_VAL_SHOCKDEDT  8

class TransferFunc1D {
public:
  TransferFunc1D(short int ty, short int vn, vector<float> &params, vector<Spectrum> &spec, string ctName);
  ~TransferFunc1D();
  
  void CheckReverse();
  bool InRange(const vector<float> &vals);
  Spectrum Lve(const vector<float> &vals) const;
  
private:
  short int valNum;   // 1 - density, 2 - temp (etc, 1 greater than defined above)
  short int type;     // 1 - constant, 2 - tophat, 3 - gaussian,
                      // 4 - constant (discrete), 5 - tophat (discrete), 6 - gaussian (discrete)
                      // 7 - linear (interpolation between two colors based on value between min/max)
                      
  // all the following parameters only apply to certain types of transfer functions
  bool clamp; // false - return zero outside range
              // true - extrapolate constant values above/below range
                      
  float range[2]; // min,max of val to calculate TF on
  Spectrum le;
  float gaussParam[2]; // mean, sigma
  
  // discrete only
  string colorTable;
  int colorTableLen;
  vector<float> colorTableVals;
  float ctMinMax[2];
  float ctStep;
  
  // linear only
  float rgb_a[3];
  float rgb_b[3];
};

class TransferFunction {
public:
  // construction
  TransferFunction(const Spectrum &sig_a);
  ~TransferFunction();

  // methods (one rgb color)
  bool AddGaussian(int valNum, float midp, float sigma, Spectrum &spec);
  bool AddTophat(int valNum, float min, float max, Spectrum &spec);
  bool AddConstant(int valNum, Spectrum &spec);

  // methods (discrete color table)
  bool AddConstantDiscrete(int valNum, string ctName, float ctMin, float ctMax);
  bool AddTophatDiscrete(int valNum, string ctName, float ctMin, float ctMax, float min, float max);
  bool AddGaussianDiscrete(int valNum, string ctName, float ctMin, float ctMax, float midp, float sigma);

  // methods (matplotlib 'linear segmented')
  bool AddLinear(int valNum, float min, float max, Spectrum &s_min, Spectrum &s_max);

  // handle inputs
  bool AddParseString(string &addTFstr);

  // evaluation
  //Spectrum sigma_a(const Point &p, const Vector &, float) const {    }
  //Spectrum sigma_s(const Point &p, const Vector &, float) const {    }
  Spectrum sigma_t() const { return sig_t; }
  bool InRange(const vector<float> &vals) const;
  Spectrum Lve(const vector<float> &vals) const;
  //Spectrum tau(const Ray &r, float stepSize, float offset) const {   }
    
private:
  // data
  short int numFuncs;
  vector<TransferFunc1D *> f_1D;
    
  map<string,int> valNums;
    
  // tau = scatter + abs
  Spectrum sig_a, sig_s, sig_t;
};

#endif
