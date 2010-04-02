#ifndef _CARTESIAN_H_
#define _CARTESIAN_H_

#include <set>
#include <map>
#include <vector>
#include <stdio.h>
#include "SVector3.h"
#include "SPoint3.h"

// this is a cartesian mesh that encompasses an oriented box with _NX
// x _NY x _NZ hexaderal cells

/*             j
   +---+---+---+---+---+---+
   |   |   |   |ij |   |   |
 i +---+---+---+---+---+---+
   |   |   |   |   |   |   |
   +---+---+---+---+---+---+

   cell ij -> nodes i;j , i+i;j , i+1;j+1, i;j+1
   
   active = CELLS : store i+N*j
*/

class hiddenANN;

template <class scalar>
class cartesianBox {
  mutable hiddenANN *_ann;
 private:  
  int _Nxi, _Neta, _Nzeta;  
  std::set<int> _active;
  double _X, _Y, _Z, _dxi, _deta, _dzeta;
  SVector3 _xiAxis, _etaAxis, _zetaAxis;
  typename std::map<int, scalar> _nodalValues;
  std::vector<SVector3> _normals;
  std::vector<SPoint3> _points;
 public:
  std::vector<SPoint3> & points() { return _points; }
  std::vector<SVector3> & normals() { return _normals; }
  cartesianBox (double X, double Y, double Z, 
                const SVector3 &DXI, 
                const SVector3 &DETA, 
                const SVector3 &DZETA, 
                int NXI, int NETA, int NZETA)
    : _ann(0), _X(X), _Y(Y), _Z(Z), 
      _dxi(norm(DXI)), 
      _deta(norm(DETA)),
      _dzeta(norm(DZETA)),
      _xiAxis(DXI),
      _etaAxis(DETA),
      _zetaAxis(DZETA),
      _Nxi(NXI), _Neta(NETA), _Nzeta(NZETA) 
  {
    _xiAxis.normalize();
    _etaAxis.normalize();
    _zetaAxis.normalize();
  }  

  typename std::map<int, scalar>::const_iterator begin() const { return _nodalValues.begin(); }
  typename std::map<int, scalar>::const_iterator end() const { return _nodalValues.end(); }

  // add that in the ann search tool
  void insert_point (double x, double y, double z)
  {
    _points.push_back(SPoint3(x,y,z));
  }    
  // compute distance
  double distance (double x, double y, double z) const;
  // set the value
  void setValue(int i, scalar s)
  {
    _nodalValues[i] = s;
  }
  inline int index_of_element (double x, double y, double z) const
  {
    // P = P_0 + xi * _vdx + eta * _vdy + zeta *vdz
    // DP = P-P_0 * _vdx = xi
    SVector3 DP (x-_X,y-_Y,z-_Z);

    double xi   = dot(DP,_xiAxis);
    double eta  = dot(DP,_etaAxis);
    double zeta = dot(DP,_zetaAxis);    

    int i = xi/_dxi * _Nxi;
    int j = eta/_deta * _Neta;
    int k = zeta/_dzeta * _Nzeta;

    if (i < 0) i = 0;if (i >= _Nxi) i = _Nxi-1;
    if (j < 0) j = 0;if (j >= _Neta) j = _Neta-1;
    if (k < 0) k = 0;if (k >= _Nzeta) k = _Nzeta-1;

    return element_index(i,j,k);
  }
  inline SPoint3 coordinates_of_node (const int &t) const
  {
    int i, j, k;
    node_ijk(t, i, j, k);

    // SVector3 DP (x-_X,y-_Y,z-_Z);

    const double xi   = i*_dxi/_Nxi;
    const double eta  = j*_deta/_Neta;
    const double zeta = k*_dzeta/_Nzeta;

    // printf("index %d -> %d %d %d xi %g %g %g X %g %g %g N %d %d %d D %g %g %g\n",
    //     t,i,j,k,xi,eta,zeta,_X,_Y,_Z,_Nxi,_Neta,_Nzeta,_dxi,_deta,_dzeta);
    
    SVector3 D = xi * _xiAxis  + eta * _etaAxis + zeta * _zetaAxis;
    
    return SPoint3(_X+D.x(),_Y+D.y(),_Z+D.z());
  }
  void insert (const int &t)
  {
    _active.insert(t);
  }
  inline int element_index (int i, int j, int k) const 
  {
    return i + _Nxi * j + _Nxi *_Neta * k;
  }
  inline int node_index (int i, int j, int k) const 
  {
    return i + (_Nxi+1) * j + (_Nxi+1) * (_Neta+1) * k;
  }
  inline void element_ijk (int index, int &i, int &j, int &k) const 
  {
    k = index / (_Nxi *_Neta);
    j = (index-k*(_Nxi *_Neta))/_Nxi;
    i = (index-k*(_Nxi *_Neta)-j*_Nxi);
  }
  inline void node_ijk (int index, int &i, int &j, int &k) const 
  {
    k = index / ((_Nxi+1) *(_Neta+1));
    j = (index-k*((_Nxi+1) *(_Neta+1)))/(_Nxi+1);
    i = (index-k*((_Nxi+1) *(_Neta+1))-j*(_Nxi+1));
  }
  inline void create_nodes()
  {
    std::set<int>::const_iterator it = _active.begin();
    for( ; it != _active.end() ; ++it){
      const int &t = *it;
      int i,j,k;
      element_ijk(t,i,j,k);
      for (int I=0;I<2;I++)
        for (int J=0;J<2;J++)
          for (int K=0;K<2;K++)
            _nodalValues[node_index(i+I,j+J,k+K)] = 0.0;
    }
  }
  void writeMSH (const std::string &filename, bool simplex=false) const 
  {
    
//     const bool simplex=false;
    
    FILE *f = fopen (filename.c_str(), "w");
    fprintf(f,"$MeshFormat\n2.1 0 8\n$EndMeshFormat\n");
    {
      fprintf(f,"$Nodes\n%d\n",_nodalValues.size());
      typename std::map<int, scalar>::const_iterator it = _nodalValues.begin();
      for ( ; it!=_nodalValues.end();++it){
        SPoint3 p = coordinates_of_node(it->first);
        fprintf(f,"%d %g %g %g\n",it->first,p.x(),p.y(),p.z());
      }    
      fprintf(f,"$EndNodes\n");
    }
    {
      int coeff=1;
      if(simplex) coeff=6;
      fprintf(f,"$Elements\n%d\n",coeff*_active.size());
      std::set<int>::const_iterator it = _active.begin();
      for ( ; it!=_active.end();++it){
        if(!simplex){
          fprintf(f,"%d 5 3 1 1 1",*it);
          int i,j,k;
          element_ijk(*it,i,j,k);
          fprintf(f," %d",node_index(i,j,k));
          fprintf(f," %d",node_index(i+1,j,k));
          fprintf(f," %d",node_index(i+1,j+1,k));
          fprintf(f," %d",node_index(i,j+1,k));
          fprintf(f," %d",node_index(i,j,k+1));
          fprintf(f," %d",node_index(i+1,j,k+1));
          fprintf(f," %d",node_index(i+1,j+1,k+1));
          fprintf(f," %d",node_index(i,j+1,k+1));
          fprintf(f,"\n");
        }else{
          int i,j,k;
          element_ijk(*it,i,j,k);

          //Elt1
          fprintf(f,"%d 4 3 1 1 1",*it);
          fprintf(f," %d",node_index(i,j+1,k));
          fprintf(f," %d",node_index(i,j+1,k+1));
          fprintf(f," %d",node_index(i+1,j,k+1));
          fprintf(f," %d",node_index(i+1,j+1,k+1));
          fprintf(f,"\n");
          
          //Elt2
          fprintf(f,"%d 4 3 1 1 1",*it);
          fprintf(f," %d",node_index(i,j+1,k));
          fprintf(f," %d",node_index(i+1,j+1,k+1));
          fprintf(f," %d",node_index(i+1,j,k+1));
          fprintf(f," %d",node_index(i+1,j+1,k));
          fprintf(f,"\n");
          
          //Elt3
          fprintf(f,"%d 4 3 1 1 1",*it);
          fprintf(f," %d",node_index(i,j+1,k));
          fprintf(f," %d",node_index(i,j,k+1));
          fprintf(f," %d",node_index(i+1,j,k+1));
          fprintf(f," %d",node_index(i,j+1,k+1));
          fprintf(f,"\n");
          
          //Elt4
          fprintf(f,"%d 4 3 1 1 1",*it);
          fprintf(f," %d",node_index(i,j+1,k));
          fprintf(f," %d",node_index(i+1,j+1,k));
          fprintf(f," %d",node_index(i+1,j,k+1));
          fprintf(f," %d",node_index(i+1,j,k));
          fprintf(f,"\n");
          
          //Elt5
          fprintf(f,"%d 4 3 1 1 1",*it);
          fprintf(f," %d",node_index(i,j+1,k));
          fprintf(f," %d",node_index(i+1,j,k));
          fprintf(f," %d",node_index(i+1,j,k+1));
          fprintf(f," %d",node_index(i,j,k));
          fprintf(f,"\n");
          
          //Elt6
          fprintf(f,"%d 4 3 1 1 1",*it);
          fprintf(f," %d",node_index(i,j+1,k));
          fprintf(f," %d",node_index(i,j,k));
          fprintf(f," %d",node_index(i+1,j,k+1));
          fprintf(f," %d",node_index(i,j,k+1));
          fprintf(f,"\n");
        }
      }    
      fprintf(f,"$EndElements\n");    
    }
    {
      fprintf(f,"$NodeData\n1\n\"distance\"\n 1\n 0.0\n3\n0\n 1\n %d\n",_nodalValues.size());
      typename std::map<int, scalar>::const_iterator it = _nodalValues.begin();
      for ( ; it!=_nodalValues.end();++it){
        SPoint3 p = coordinates_of_node(it->first);
        //      fprintf(f,"%d %g\n",it->first,distance(p.x(),p.y(),p.z()));
        fprintf(f,"%d %g\n",it->first,it->second);
      }    
      fprintf(f,"$EndNodeData\n");
    }
    fclose (f);
  }
};

#endif
