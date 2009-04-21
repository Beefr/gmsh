// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributed by Matti Pellikka.

#ifndef _CHAINCOMPLEX_H_
#define _CHAINCOMPLEX_H_

#include <stdio.h>
#include <string>
#include <algorithm>
#include <set>
#include <queue>
#include "GmshConfig.h"
#include "MElement.h"
#include "GModel.h"
#include "GEntity.h"
#include "GRegion.h"
#include "GFace.h"
#include "GVertex.h"
#include "CellComplex.h"

#if defined(HAVE_KBIPACK)

#include "gmp.h"
extern "C" {
  #include "gmp_normal_form.h" // perhaps make c++ headers instead?
}

// A class representing a chain complex of a cell complex.
// This should only be constructed for a reduced cell complex because of
// dense matrix representations and great computational complexity in its methods.
class ChainComplex{
  private:
   // boundary operator matrices for this chain complex
   // h_k: C_k -> C_(k-1)
   gmp_matrix* _HMatrix[4];
   
   // Basis matrices for the kernels and codomains of the boundary operator matrices
   gmp_matrix* _kerH[4];
   gmp_matrix* _codH[4];
   
   gmp_matrix* _JMatrix[4];
   gmp_matrix* _QMatrix[4];
   std::vector<long int> _torsion[4];
   
   // bases for homology groups
   gmp_matrix* _Hbasis[4];
   
  public:
   
   ChainComplex(CellComplex* cellComplex);
      
   ChainComplex(){
     for(int i = 0; i < 4; i++){
       _HMatrix[i] = create_gmp_matrix_zero(1,1);
       _kerH[i] = NULL;
       _codH[i] = NULL;
       _JMatrix[i] = NULL;
       _QMatrix[i] = NULL;
       _Hbasis[i] = NULL;
     }
   }
   virtual ~ChainComplex(){}
   
   // get the boundary operator matrix dim->dim-1
   virtual gmp_matrix* getHMatrix(int dim) { if(dim > -1 || dim < 4) return _HMatrix[dim]; else return NULL;}
   virtual gmp_matrix* getKerHMatrix(int dim) { if(dim > -1 || dim < 4) return _kerH[dim]; else return NULL;}
   virtual gmp_matrix* getCodHMatrix(int dim) { if(dim > -1 || dim < 4) return _codH[dim]; else return NULL;}
   virtual gmp_matrix* getJMatrix(int dim) { if(dim > -1 || dim < 4) return _JMatrix[dim]; else return NULL;}
   virtual gmp_matrix* getQMatrix(int dim) { if(dim > -1 || dim < 4) return _QMatrix[dim]; else return NULL;}
   virtual gmp_matrix* getHbasis(int dim) { if(dim > -1 || dim < 4) return _Hbasis[dim]; else return NULL;}
   
   // Compute basis for kernel and codomain of boundary operator matrix of dimension dim (ie. ker(h_dim) and cod(h_dim) )
   virtual void KerCod(int dim);
   // Compute matrix representation J for inclusion relation from dim-cells who are boundary of dim+1-cells 
   // to cycles of dim-cells (ie. j: cod(h_(dim+1)) -> ker(h_dim) )
   virtual void Inclusion(int dim);
   // Compute quotient problem for given inclusion relation j to find representatives of homology groups
   // and possible torsion coeffcients
   virtual void Quotient(int dim);
   
   // Compute bases for the homology groups of this chain complex 
   virtual void computeHomology();
   
   virtual int printMatrix(gmp_matrix* matrix){ 
     printf("%d rows and %d columns\n", gmp_matrix_rows(matrix), gmp_matrix_cols(matrix)); 
     return gmp_matrix_printf(matrix); } 
     
   // debugging aid
   virtual void matrixTest();
};


#endif

#endif
