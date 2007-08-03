#ifndef _F_PATCH_H_
#define _F_PATCH_H_

#include <cmath>
#include <vector>
#include <fftw3.h>
#include <complex>
#include "Patch.h"
#include "PartitionOfUnity.h"
#include "ProjectionSurface.h"

// The base class for the patches
class FPatch : public Patch {
 protected:
  // Data Size
  int _nData;

  // Number of Fourier Modes
  int _uModes, _vModes, _nModes;

  // Number of Modes in reprocessed Fourier/Chebyshev series
  int _uM, _vM;

   // Period Information
  double _periodU, _periodV;

  // Limits of Fourier Series
  int _uModesLower, _uModesUpper, _vModesLower, _vModesUpper;

  // Limits in the new series
  int _uMLower, _uMUpper, _vMLower, _vMUpper;

  // data (and its first 2 derivatives)
  std::complex<double> **_coeffOriginalData;
  std::complex<double> **_coeffData, **_coeffDerivU, **_coeffDerivV;
  std::complex<double> **_coeffDerivUU, **_coeffDerivVV, **_coeffDerivUV;

  // temporary interpolation variables
  std::vector< std::complex<double> > _tmpOrigCoeff, _tmpOrigInterp;
  std::vector< std::complex<double> > _tmpCoeff, _tmpInterp;

  // polynomial evaluator
  std::complex<double> _PolyEval(std::vector< std::complex<double> > _coeff,
                                 std::complex<double> x);
  // interpolation wrapper
  std::complex<double> _Interpolate(double u,double v);
  std::complex<double> _Interpolate(double u,double v,int uDer,int vDer);

  // other internal functions
  void _ReprocessSeriesCoeff();

  // persistent fftw data (used to avoid recomputing the FFTW plans
  // and reallocating memory every time)
  static int _forwardSize, _backwardSize;
  static fftw_plan _forwardPlan, _backwardPlan;
  static fftw_complex *_forwardData, *_backwardData;
  void _SetForwardPlan(int n);
  void _SetBackwardPlan(int n);

  // This routine computes the forward FFT (ignoring the last element
  // of the data)
  void _ForwardFft(int n, std::complex<double> *fftData);

  // This routine computes the inverse FFT (ignoring the last element
  // in the array), returns values in entire array (by using the
  // periodic extension)
  void _BackwardFft(int n, std::complex<double> *fftData);

 public:
  FPatch
    (int tag, ProjectionSurface* ps,
     std::vector<double> &u, std::vector<double> &v,
     std::vector< std::complex<double> > &data, int derivative = 3,
     int uModes = 10, int vModes = 8, int uM = 17, int vM = 16,
     bool hardEdge0 = false, bool hardEdge1 = false, bool hardEdge2 = false,
     bool hardEdge3 = false);
  virtual ~FPatch();

  // Abstract functions of Patch

  virtual double GetPou(double u, double v);
  virtual void F(double u, double v, double &x, double &y, double &z);
  virtual bool Inverse(double x,double y,double z,double &u,double &v);
  virtual void Dfdu(double u, double v, double &x, double &y, double &z);
  virtual void Dfdv(double u, double v, double &x, double &y, double &z);
  virtual void Dfdfdudu(double u,double v,double &x,double &y,double &z);
  virtual void Dfdfdudv(double u,double v,double &x,double &y,double &z);
  virtual void Dfdfdvdv(double u,double v,double &x,double &y,double &z);
};

#endif
