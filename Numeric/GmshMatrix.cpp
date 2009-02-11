// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#include <complex>
#include "GmshConfig.h"
#include "GmshMatrix.h"
#include "GmshMessage.h"

#if defined(HAVE_BLAS)

extern "C" {
  void dgemm_(const char *transa, const char *transb, int *m, int *n, int *k, 
              double *alpha, double *a, int *lda, 
              double *b, int *ldb, double *beta, 
              double *c, int *ldc);
  void zgemm_(const char *transa, const char *transb, int *m, int *n, int *k, 
              std::complex<double> *alpha, std::complex<double> *a, int *lda, 
              std::complex<double> *b, int *ldb, std::complex<double> *beta, 
              std::complex<double> *c, int *ldc);
  void dgemv_(const char *trans, int *m, int *n, 
              double *alpha, double *a, int *lda, 
              double *x, int *incx, double *beta, 
              double *y, int *incy);
  void zgemv_(const char *trans, int *m, int *n, 
              std::complex<double> *alpha, std::complex<double> *a, int *lda, 
              std::complex<double> *x, int *incx, std::complex<double> *beta, 
              std::complex<double> *y, int *incy);
}

template<> 
void gmshMatrix<double>::mult(const gmshMatrix<double> &b, gmshMatrix<double> &c)
{
  int M = c.size1(), N = c.size2(), K = _c;
  int LDA = _r, LDB = b.size1(), LDC = c.size1();
  double alpha = 1., beta = 0.;
  dgemm_("N", "N", &M, &N, &K, &alpha, _data, &LDA, b._data, &LDB, 
         &beta, c._data, &LDC);
}

template<> 
void gmshMatrix<std::complex<double> >::mult(const gmshMatrix<std::complex<double> > &b, 
                                             gmshMatrix<std::complex<double> > &c)
{
  int M = c.size1(), N = c.size2(), K = _c;
  int LDA = _r, LDB = b.size1(), LDC = c.size1();
  std::complex<double> alpha = 1., beta = 0.;
  zgemm_("N", "N", &M, &N, &K, &alpha, _data, &LDA, b._data, &LDB, 
         &beta, c._data, &LDC);
}

template<> 
void gmshMatrix<double>::gemm(gmshMatrix<double> &a, gmshMatrix<double> &b, 
                              double alpha, double beta)
{
  int M = size1(), N = size2(), K = a.size2();
  int LDA = a.size1(), LDB = b.size1(), LDC = size1();
  dgemm_("N", "N", &M, &N, &K, &alpha, a._data, &LDA, b._data, &LDB, 
         &beta, _data, &LDC);
}

template<> 
void gmshMatrix<std::complex<double> >::gemm(gmshMatrix<std::complex<double> > &a, 
                                             gmshMatrix<std::complex<double> > &b, 
                                             std::complex<double> alpha, 
                                             std::complex<double> beta)
{
  int M = size1(), N = size2(), K = a.size2();
  int LDA = a.size1(), LDB = b.size1(), LDC = size1();
  zgemm_("N", "N", &M, &N, &K, &alpha, a._data, &LDA, b._data, &LDB, 
         &beta, _data, &LDC);
}

template<> 
void gmshMatrix<double>::mult(const gmshVector<double> &x, gmshVector<double> &y)
{
  int M = _r, N = _c, LDA = _r, INCX = 1, INCY = 1;
  double alpha = 1., beta = 0.;
  dgemv_("N", &M, &N, &alpha, _data, &LDA, x._data, &INCX,
         &beta, y._data, &INCY);
}

template<> 
void gmshMatrix<std::complex<double> >::mult(const gmshVector<std::complex<double> > &x, 
                                             gmshVector<std::complex<double> > &y)
{
  int M = _r, N = _c, LDA = _r, INCX = 1, INCY = 1;
  std::complex<double> alpha = 1., beta = 0.;
  zgemv_("N", &M, &N, &alpha, _data, &LDA, x._data, &INCX,
         &beta, y._data, &INCY);
}

#endif

#if defined(HAVE_LAPACK)

extern "C" {
  void dgesv_(int *N, int *nrhs, double *A, int *lda, int *ipiv, 
              double *b, int *ldb, int *info);
  void dgetrf_(int *M, int *N, double *A, int *lda, int *ipiv, int *info);
  void dgesvd_(const char* jobu, const char *jobvt, int *M, int *N,
               double *A, int *lda, double *S, double* U, int *ldu,
               double *VT, int *ldvt, double *work, int *lwork, int *info);
}

template<> 
bool gmshMatrix<double>::lu_solve(const gmshVector<double> &rhs, gmshVector<double> &result)
{
  int N = size1(), nrhs = 1, lda = N, ldb = N, info;
  int *ipiv = new int[N];
  for(int i = 0; i < N; i++) result(i) = rhs(i);
  dgesv_(&N, &nrhs, _data, &lda, ipiv, result._data, &ldb, &info);
  delete [] ipiv;
  if(info == 0) return true;
  if(info > 0)
    Msg::Error("U(%d,%d)=0 in LU decomposition", info, info);
  else
    Msg::Error("Wrong %d-th argument in LU decomposition", -info);
  return false;
}

template<> 
double gmshMatrix<double>::determinant() const
{
  gmshMatrix<double> tmp(*this);
  int M = size1(), N = size2(), lda = size1(), info;
  int *ipiv = new int[std::min(M, N)];
  dgetrf_(&M, &N, tmp._data, &lda, ipiv, &info);
  double det = 1.;
  if(info == 0){
    for(int i = 0; i < size1(); i++){
      det *= tmp(i, i);
      if(ipiv[i] != i + 1) det = -det;
    }
  }
  else if(info > 0)
    det = 0.;
  else
    Msg::Error("Wrong %d-th argument in matrix factorization", -info);
  delete [] ipiv;  
  return det;
}

template<> 
bool gmshMatrix<double>::svd(gmshMatrix<double> &V, gmshVector<double> &S)
{
  gmshMatrix<double> VT(V.size2(), V.size1());
  int M = size1(), N = size2(), LDA = size1(), LDVT = VT.size1(), info;
  int LWORK = std::max(3 * std::min(M, N) + std::max(M, N), 5 * std::min(M, N));
  gmshVector<double> WORK(LWORK);
  dgesvd_("O", "A", &M, &N, _data, &LDA, S._data, _data, &LDA,
          VT._data, &LDVT, WORK._data, &LWORK, &info);
  V = VT.transpose();
  if(info == 0) return true;
  if(info > 0)
    Msg::Error("SVD did not converge");
  else
    Msg::Error("Wrong %d-th argument in SVD decomposition", -info);
  return false;
}

#endif
