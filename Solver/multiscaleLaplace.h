#ifndef _MULTISCALE_LAPLACE_H_
#define _MULTISCALE_LAPLACE_H_

#include <vector>
#include <map>
#include "SPoint2.h"
#include "linearSystemGMM.h"

class MElement;
class MVertex;

struct multiscaleLaplaceLevel {
  SPoint2 center;
  double  scale;
  int recur,region;
  std::vector<MElement *> elements;
  std::map<MVertex*,SPoint2> coordinates;
};

class multiscaleLaplace{
  linearSystem<double> *_lsys;
  std::vector<multiscaleLaplaceLevel*> levels;
  void parametrize (multiscaleLaplaceLevel &);
  
public:
  multiscaleLaplace (std::vector<MElement *> &elements,
		     std::vector<MVertex*> &boundaryNodes,
		     std::vector<double> &linearAbscissa) ;
  
  
};
#endif
