// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributed by Matti Pellikka.

#include "ChainComplex.h"

#if defined(HAVE_KBIPACK)

ChainComplex::ChainComplex(CellComplex* cellComplex){
  
  _dim = 0;
  _HMatrix[0] = NULL;
  _kerH[0] = NULL;
  _codH[0] = NULL;
  _JMatrix[0] = NULL;
  _QMatrix[0] = NULL;
  _Hbasis[0] = NULL;
  
  for(int dim = 1; dim < 4; dim++){
    unsigned int cols = cellComplex->getSize(dim);
    unsigned int rows =  cellComplex->getSize(dim-1);
  
    
    
    // ignore subdomain cells
    for(std::set<Cell*, Less_Cell>::iterator cit = cellComplex->firstCell(dim); cit != cellComplex->lastCell(dim); cit++){
      Cell* cell = *cit;
      if(cell->inSubdomain()) cols--;
    }
    for(std::set<Cell*, Less_Cell>::iterator cit = cellComplex->firstCell(dim-1); cit != cellComplex->lastCell(dim-1); cit++){
      Cell* cell = *cit;
      if(cell->inSubdomain()) rows--;
    }
    
    
    if( cols == 0 ){
      //_HMatrix[dim] = create_gmp_matrix_zero(rows, 1);
      _HMatrix[dim] = NULL;
    }
    else if( rows == 0){
      _HMatrix[dim] = create_gmp_matrix_zero(1, cols);
      //_HMatrix[dim] = NULL;
    }
    
    
    else{
      
      long int elems[rows*cols];
      
      std::set<Cell*, Less_Cell>::iterator high = cellComplex->firstCell(dim);
      std::set<Cell*, Less_Cell>::iterator low = cellComplex->firstCell(dim-1);
      
      unsigned int i = 0;
      while(i < rows*cols){
        while(low != cellComplex->lastCell(dim-1)){
          Cell* lowcell = *low;
          Cell* highcell = *high;
          if(!(highcell->inSubdomain() || lowcell->inSubdomain())){
            elems[i] = cellComplex->kappa(*high, *low);
            i++;
          }
          low++;
        }
        low = cellComplex->firstCell(dim-1);
        high++;
      }
      _HMatrix[dim] = create_gmp_matrix_int(rows, cols, elems);      
    }
    
    _kerH[dim] = NULL;
    _codH[dim] = NULL;
    _JMatrix[dim] = NULL;
    _QMatrix[dim] = NULL;
    _Hbasis[dim] = NULL; 
    
    if(cellComplex->getSize(dim) != 0) _dim = dim;
    
  }
  
  return;
}


void ChainComplex::KerCod(int dim){
  
  if(dim < 0 || dim > 3 || _HMatrix[dim] == NULL) return;
  
  gmp_matrix* HMatrix = copy_gmp_matrix(_HMatrix[dim], 1, 1, 
                                        gmp_matrix_rows(_HMatrix[dim]), gmp_matrix_cols(_HMatrix[dim]));
  
  gmp_normal_form* normalForm = create_gmp_Hermite_normal_form(HMatrix, NOT_INVERTED, INVERTED);
  //printMatrix(normalForm->left);
  //printMatrix(normalForm->canonical);
  //printMatrix(normalForm->right);
  
  int minRowCol = std::min(gmp_matrix_rows(normalForm->canonical), gmp_matrix_cols(normalForm->canonical));
  int rank = 0;
  mpz_t elem;
  mpz_init(elem);
  
  while(rank < minRowCol){
    gmp_matrix_get_elem(elem, rank+1, rank+1, normalForm->canonical);
    if(mpz_cmp_si(elem,0) == 0) break;
    rank++;
  }
  
  if(rank != gmp_matrix_cols(normalForm->canonical)){
    _kerH[dim] = copy_gmp_matrix(normalForm->right, 1, rank+1, 
                                 gmp_matrix_rows(normalForm->right),  gmp_matrix_cols(normalForm->right));
  }
  
  if(rank > 0){
     _codH[dim] = copy_gmp_matrix(normalForm->canonical, 1, 1,
                                  gmp_matrix_rows(normalForm->canonical), rank);
    gmp_matrix_left_mult(normalForm->left, _codH[dim]);
  }
  
  mpz_clear(elem);
  destroy_gmp_normal_form(normalForm);
  
  return;
}

//j:B_(k+1)->Z_k
void ChainComplex::Inclusion(int lowDim, int highDim){
  
  if(getKerHMatrix(lowDim) == NULL || getCodHMatrix(highDim) == NULL || abs(lowDim-highDim) != 1) return;
  
  gmp_matrix* Zbasis = copy_gmp_matrix(_kerH[lowDim], 1, 1,
                                       gmp_matrix_rows(_kerH[lowDim]), gmp_matrix_cols(_kerH[lowDim]));
  gmp_matrix* Bbasis = copy_gmp_matrix(_codH[highDim], 1, 1,
                                       gmp_matrix_rows(_codH[highDim]), gmp_matrix_cols(_codH[highDim]));
  
  
  int rows = gmp_matrix_rows(Bbasis);
  int cols = gmp_matrix_cols(Bbasis);
  if(rows < cols) return;
  
  rows = gmp_matrix_rows(Zbasis);
  cols = gmp_matrix_cols(Zbasis);
  if(rows < cols) return;
  
  
  // A*inv(V) = U*S
  gmp_normal_form* normalForm = create_gmp_Smith_normal_form(Zbasis, INVERTED, INVERTED);
  
  mpz_t elem;
  mpz_init(elem);
  
  
  for(int i = 1; i <= cols; i++){
  
    gmp_matrix_get_elem(elem, i, i, normalForm->canonical);
    if(mpz_cmp_si(elem,0) == 0){
      destroy_gmp_normal_form(normalForm);
      return;
    }
  }
  
  gmp_matrix_left_mult(normalForm->left, Bbasis);
  
  
  
  gmp_matrix* LB = copy_gmp_matrix(Bbasis, 1, 1, gmp_matrix_cols(Zbasis), gmp_matrix_cols(Bbasis));
  destroy_gmp_matrix(Bbasis);
  
  rows = gmp_matrix_rows(LB);
  cols = gmp_matrix_cols(LB);
  
  mpz_t divisor;
  mpz_init(divisor);
  mpz_t remainder;
  mpz_init(remainder);
  mpz_t result;
  mpz_init(result);
  
  for(int i = 1; i <= rows; i++){
    gmp_matrix_get_elem(divisor, i, i, normalForm->canonical);
    for(int j = 1; j <= cols; j++){
      gmp_matrix_get_elem(elem, i, j, LB);
      mpz_cdiv_qr(result, remainder, elem, divisor);
      if(mpz_cmp_si(remainder, 0) == 0){
        gmp_matrix_set_elem(result, i, j, LB);
      }
      else return;
    }
  }
  
  gmp_matrix_left_mult(normalForm->right, LB);
  
  setJMatrix(lowDim, LB);
  
  mpz_clear(elem);
  mpz_clear(divisor);
  mpz_clear(result);
  destroy_gmp_normal_form(normalForm);
  
  return;
  
}

void ChainComplex::Quotient(int dim){
  
  if(dim < 0 || dim > 3 || _JMatrix[dim] == NULL) return;
  
  gmp_matrix* JMatrix = copy_gmp_matrix(_JMatrix[dim], 1, 1,
                                        gmp_matrix_rows(_JMatrix[dim]), gmp_matrix_cols(_JMatrix[dim]));
  int rows = gmp_matrix_rows(JMatrix);
  int cols = gmp_matrix_cols(JMatrix);
  
  gmp_normal_form* normalForm = create_gmp_Smith_normal_form(JMatrix, NOT_INVERTED, NOT_INVERTED);

  //printMatrix(normalForm->left);
  //printMatrix(normalForm->canonical);
  //printMatrix(normalForm->right);
  
  
  mpz_t elem;
  mpz_init(elem);
  
  for(int i = 1; i <= cols; i++){
    gmp_matrix_get_elem(elem, i, i, normalForm->canonical);
    if(mpz_cmp_si(elem,0) == 0){
      destroy_gmp_normal_form(normalForm);
      return;
    }
    if(mpz_cmp_si(elem,1) > 0){
      _torsion[dim].push_back(mpz_get_si(elem));
    }
  }
  
  int rank = cols - _torsion[dim].size();
  if(rows - rank > 0){
    gmp_matrix* Hbasis = copy_gmp_matrix(normalForm->left, 1, rank+1, rows, rows);
    _QMatrix[dim] = Hbasis;
  }
  
  mpz_clear(elem);
  destroy_gmp_normal_form(normalForm);
  return; 
}

void ChainComplex::computeHomology(bool dual){
  
  int lowDim = 0;
  int highDim = 0;
  
  for(int i=0; i < 4; i++){
    
    if(dual){
      lowDim = getDim()+1-i; 
      highDim = getDim()+1-(i+1);
      //KerCod(lowDim);
    }
    else{
      lowDim = i;
      highDim = i+1;
      //KerCod(highDim);
    }
    
    printf("Homology computation process: step %d of 4 \n", i+1 );
    
    KerCod(highDim);
    
    // 1) no edges, but zero cells
    if(lowDim == 0 &&  gmp_matrix_cols(getHMatrix(lowDim)) > 0 && getHMatrix(highDim) == NULL) {
      setHbasis( lowDim, create_gmp_matrix_identity(gmp_matrix_cols(getHMatrix(lowDim))) );
    }    
    // 2) this dimension is empty
    else if(getHMatrix(lowDim) == NULL && getHMatrix(highDim) == NULL){
      setHbasis(lowDim, NULL);
    }
    // 3) No higher dimension cells -> none of the cycles are boundaries
    else if(getHMatrix(highDim) == NULL){
      setHbasis( lowDim, copy_gmp_matrix(getKerHMatrix(lowDim), 1, 1,
                                         gmp_matrix_rows(getKerHMatrix(lowDim)), 
                                         gmp_matrix_cols(getKerHMatrix(lowDim))) );
    }

    // 5) General case:
    //   1) Find the bases of boundaries B and cycles Z 
    //   2) find j: B -> Z and
    //   3) find quotient Z/j(B) 
    else {
      
      // 4) No lower dimension cells -> all chains are cycles
      if(getHMatrix(lowDim) == NULL){
        setKerHMatrix(lowDim, create_gmp_matrix_identity(gmp_matrix_rows(getHMatrix(highDim))) );
      }
      Inclusion(lowDim, highDim);
      Quotient(lowDim);
      
      if(getCodHMatrix(highDim) == NULL){
        setHbasis(lowDim, copy_gmp_matrix(getKerHMatrix(lowDim), 1, 1,
                                          gmp_matrix_rows(getKerHMatrix(lowDim)), 
                                          gmp_matrix_cols(getKerHMatrix(lowDim))) );
      }  
      else if(getJMatrix(lowDim) == NULL || getQMatrix(lowDim) == NULL){
        setHbasis(lowDim, NULL);
      } 
      else{
        setHbasis(lowDim, copy_gmp_matrix(getKerHMatrix(lowDim), 1, 1, 
                                          gmp_matrix_rows(getKerHMatrix(lowDim)), 
                                          gmp_matrix_cols(getKerHMatrix(lowDim))) );
        
        gmp_matrix_right_mult(getHbasis(lowDim), getQMatrix(lowDim));
      } 
      
    } 
    
    
    destroy_gmp_matrix(getKerHMatrix(lowDim));
    destroy_gmp_matrix(getCodHMatrix(lowDim));
    destroy_gmp_matrix(getJMatrix(lowDim));
    destroy_gmp_matrix(getQMatrix(lowDim));
    
    setKerHMatrix(lowDim, NULL);
    setCodHMatrix(lowDim, NULL);
    setJMatrix(lowDim, NULL);
    setQMatrix(lowDim, NULL);
   
  }
  
  
  return;
}


void ChainComplex::matrixTest(){
  
  int rows = 3;
  int cols = 6;
  
  long int elems[rows*cols];
  for(int i = 1; i<=rows*cols; i++) elems[i-1] = i;
  
  gmp_matrix* matrix = create_gmp_matrix_int(rows, cols, elems);
  
  gmp_matrix* copymatrix = copy_gmp_matrix(matrix, 3, 2, 3, 5);
  
  printMatrix(matrix);
  printMatrix(copymatrix);
  
  return; 
}

std::vector<int> ChainComplex::getCoeffVector(int dim, int chainNumber){
  
  std::vector<int> coeffVector;
  
  if(dim < 0 || dim > 3) return coeffVector;
  if(_Hbasis[dim] == NULL || gmp_matrix_cols(_Hbasis[dim]) < chainNumber) return coeffVector;
  
  int rows = gmp_matrix_rows(_Hbasis[dim]);
  
  int elemi;
  long int elemli;
  mpz_t elem;
  mpz_init(elem);
  
  for(int i = 1; i <= rows; i++){
    gmp_matrix_get_elem(elem, i, chainNumber, _Hbasis[dim]);
    elemli = mpz_get_si(elem);
    elemi = elemli;
    coeffVector.push_back(elemi);
    //printf("coeff: %d \n", coeffVector.at(i-1));
  }
  
  mpz_clear(elem);
  return coeffVector;
  
}

#endif

Chain::Chain(std::set<Cell*, Less_Cell> cells, std::vector<int> coeffs, CellComplex* cellComplex, std::string name){
  
  int i = 0;
  for(std::set<Cell*, Less_Cell>::iterator cit = cells.begin(); cit != cells.end(); cit++){
    Cell* cell = *cit;
    if(!cell->inSubdomain() && coeffs.size() > i){
      if(coeffs.at(i) != 0) _cells.push_back( std::make_pair(cell, coeffs.at(i)) );
      i++;
    }
  }
  
  _name = name;
  _cellComplex = cellComplex;
  
}

int Chain::writeChainMSH(const std::string &name){

  //_cellComplex->writeComplexMSH(name);
  
  if(getSize() == 0) return 0;
  
  FILE *fp = fopen(name.c_str(), "a");
  if(!fp){
    Msg::Error("Unable to open file '%s'", name.c_str());
        printf("Unable to open file.");
        return 0;
  }

  fprintf(fp, "\n$ElementData\n");
  
  fprintf(fp, "1 \n");
  fprintf(fp, "\"%s\" \n", getName().c_str());
  fprintf(fp, "1 \n");
  fprintf(fp, "0.0 \n");
  fprintf(fp, "4 \n");
  fprintf(fp, "0 \n");
  fprintf(fp, "1 \n");
  fprintf(fp, "%d \n", getSize());
  fprintf(fp, "0 \n");
  
  for(int i = 0; i < getSize(); i++){
    
    fprintf(fp, "%d %d \n", getCell(i)->getTag(), getCoeff(i));
    
  }
  
  fprintf(fp, "$EndElementData\n");
  
  fclose(fp);
  
  return 1;
  
}
