#ifndef _FUNCTION_H_
#define _FUNCTION_H_

#include "fullMatrix.h"
#include <map>
#include <set>
#include <list>
#include <string>
#include <vector>
class dataCacheMap;
class MElement;
class binding;
class dgDofContainer;

// those classes manage complex function dependencies and keep their values in cache so that they are not recomputed when it is not necessary. To do this, we use three classes : function, dataCache and dataCacheMap. The workflow is :
// a) while parsing the input file and during the initialisation of the conservationLaw : all user-defined instance of function are inserted in the function map. (for example an user can create a function named "wind" of the class functionMathex with parameter "0.1*sin(xyz(0)/1e6); 0"  and then give the string "wind" as parameter to it's conservation law to let the law know that this is the function to use as wind forcing)
//
// b) before starting to iterate over the element :
// b1) the algorithm create a new dataCacheMap instance.
// b2) The algo insert in the dataCacheMap the dataCache it will provide (ie the quadrature points, the solution value,...) 
// b3) Then the algo give this dataCacheMap to the law and ask the source term of the law. This source term will be given as a dataCache, i.e. a cached value, a function to update it and a node in the dependency tree. In this example, to create this datacache two new dataCache will be required : one for the "wind" and one for "xyz". So, the source dataCache will ask the dataCacheMap for the dataCache called "wind". If this one does not already exist, the dataCacheMap will ask the function called "wind" to create it. In turn, the constructor of the dataCache associated with the "wind" will ask for the for the dataCache "xyz" wich will be created by the function "xyz" if it does not already exist and so on. The dataCacheMap contain also a special dataCache associated with the current element. During it's construction it's dataCache keep references to all the dataCache needed to compute it. When they request for the dataCache they need, the dataCacheMap is informed and the dependency tree is built. 
//
// c) When iterating over elements, for each element : the Algo will change the dataCache element so that it point to the current element. This will invalidate everything that depend on the current on the current element (in this case, xyz, wind (because it depends on xyz) and the source dataCache (because it depends on wind). So, in this case everything (exept the current element dataCache) is marked as invalid. Now the algo want to access the value of the 'source' dataCache this one is invalid, so it is recomputed. To recompute it, the value of the wind is accessed and it's value is recomputed, and so on. Now, if some other function access the wind or the position (xyz), they are already marked as valid and are not re-computed.
//
// d) after the loop over the elements the dataCacheMap is deleted and it delete all its dataCaches.
//
// NB) in the case of integration over faces, there is two dataCacheMap : one for the Right side and one for the left side i.e. two sets of cached values and two dependency trees. Some dataCache (ie the value of the flux) have dependencies in both tree. This is not a problem, the only difference with other dataCaches is that they are not owned by the dataCacheMap and have to be deleted manually.
// 

// a node in the dependency tree. The usefull field is _dependOnMe which is the set of every other nodes that depend on me. When the value of this node change all nodes depending on this one are marked as "invalid" and will be recomputed the next time their data are accessed. To be able to maintain _dependOnMe up to date when a new node is inserted in the tree, we need _iDependOn list. So we do not really store a tree but instead each node contain a complete list of all it's parents and all it's children (and the parents of the parents of ... of its parents and the children of the children of ... of it's children). This way invalidate all the dependencies of a node is really fast and does not involve a complex walk accross the tree structure.

class function;
class dataCacheDouble;

// An abstract interface to functions 
// more explanation at the head of this file
class function {
  int _nbCol;
  bool _invalidatedOnElement;
  protected :
  virtual void call (dataCacheMap *m, fullMatrix<double> &res) {throw;}
  virtual void call (dataCacheMap *m, const fullMatrix<double> &arg0, fullMatrix<double> &res) {throw;};
  virtual void call (dataCacheMap *m, const fullMatrix<double> &arg0, const fullMatrix<double> &arg1, fullMatrix<double> &res) {throw;};
  virtual void call (dataCacheMap *m, const fullMatrix<double> &arg0, const fullMatrix<double> &arg1, const fullMatrix<double> &arg2, fullMatrix<double> &res) {throw;};
  virtual void call (dataCacheMap *m, const fullMatrix<double> &arg0, const fullMatrix<double> &arg1, const fullMatrix<double> &arg2, const fullMatrix<double> &arg3, fullMatrix<double> &res) {throw;};
  virtual void call (dataCacheMap *m, const fullMatrix<double> &arg0, const fullMatrix<double> &arg1, const fullMatrix<double> &arg2, const fullMatrix<double> &arg3, const fullMatrix<double> &arg4, fullMatrix<double> &res) {throw;};
  virtual void call (dataCacheMap *m, const fullMatrix<double> &arg0, const fullMatrix<double> &arg1, const fullMatrix<double> &arg2, const fullMatrix<double> &arg3, const fullMatrix<double> &arg4, const fullMatrix<double> &arg5, fullMatrix<double> &res) {throw;};
  public :
  std::vector<std::pair<int, const function*> > arguments;
  void addArgument(const function *f, int iMap = 0) {
    //iMap is the id of the dataCacheMap, e.g. on interfaces
    arguments.push_back(std::pair<int, const function*>(iMap, f));
  }
  virtual ~function(){};
  static void registerBindings(binding *b);
  virtual void call (dataCacheMap *m, fullMatrix<double> &res, std::vector<const fullMatrix<double>*> &depM);
  function(int nbCol, bool invalidatedOnElement = true);
  inline int getNbCol()const {return _nbCol;}
  inline bool isInvalitedOnElement() { return _invalidatedOnElement;}
  
  static function *getSolution();
  static function *getSolutionGradient();
  static function *getParametricCoordinates();
  static function *getNormals();
};

// dataCache when the value is a  matrix of double 
// the user should provide the number of rows by evaluating points and the number of columns
// then the size of the matrix is automatically adjusted
class dataCacheDouble {
  friend class dataCacheMap;
  // pointers to all of the dataCache depending on me
  std::set<dataCacheDouble*> _dependOnMe;
  std::set<dataCacheDouble*> _iDependOn;
protected :
  // invalidates all the cached data that depends on me
  inline void _invalidateDependencies()
  {
    // if this is too slow we can keep a C array cache of the _dependOnMe set
    for(std::set<dataCacheDouble*>::iterator it = _dependOnMe.begin();
      it!=_dependOnMe.end(); it++)
        (*it)->_valid=false;
  }
public :
  bool _valid;
  // dataCacheMap is the only one supposed to call this
  void addMeAsDependencyOf (dataCacheDouble *newDep);
  inline bool somethingDependOnMe() {
    return !_dependOnMe.empty();
  }
  std::vector<dataCacheDouble*> _dependencies;
  std::vector<const fullMatrix<double>*> _depM;

  int _nRowByPoint;
  dataCacheMap &_cacheMap;
  function *_function;
 protected:
  fullMatrix<double> _value;
  // do the actual computation and put the result into _value
  // still virtual because it is overrided by conservation law terms, as soon as conservation law terms will be regular functions, we will remove this
  virtual void _eval()
  {
    for(int i=0;i<_dependencies.size(); i++)
      _depM[i] = &(*_dependencies[i])();
    _function->call(&_cacheMap, _value, _depM);
  }
 public:
  //set the value (without computing it by _eval) and invalidate the dependencies
  // this function is needed to be able to pass the _value to functions like gemm or mult
  // but you cannot keep the reference to the _value, you should always use the set function 
  // to modify the _value
  // take care if you use this to set a proxy you must ensure that the value pointed to are not modified
  // without further call to set because the dependencies won't be invalidate
  inline fullMatrix<double> &set() {
    if(_valid)
      _invalidateDependencies();
    _valid = true;
    return _value;
  }
  inline const double &operator () (int i, int j)
  {
    if(!_valid) {
      _eval();
      _valid = true;
    }
    return _value(i,j);
  }
  //access _value and compute it if necessary
  inline const fullMatrix<double> &operator () ()
  {
    if(!_valid) {
      _eval();
      _valid=true;
    }
    return _value;
  }
  void resize();
  dataCacheDouble(dataCacheMap *,function *f);
  dataCacheDouble(dataCacheMap &m, int nRowByPoint, int nbCol);
  virtual ~dataCacheDouble(){};
};


// more explanation at the head of this file
class dataCacheMap {
  friend class dataCacheDouble;
  dataCacheMap  *_parent;
  std::list<dataCacheMap*> _children;
  int _nbEvaluationPoints;
  std::map<const function*, dataCacheDouble*> _cacheDoubleMap;
  std::set<dataCacheDouble*> _toDelete;
  std::set<dataCacheDouble*> _toResize;
  std::set<dataCacheDouble*> _toInvalidateOnElement;

  MElement *_element;

 protected:
  void addDataCache(dataCacheDouble *data){
    _toDelete.insert(data);
  }
  void addDataCacheDouble(dataCacheDouble *data, bool invalidatedOnElement){
    _toResize.insert(data);
    if(invalidatedOnElement)
      _toInvalidateOnElement.insert(data);
  }
 public:
  dataCacheDouble &get(const function *f, dataCacheDouble *caller=0);
  dataCacheDouble &substitute(const function *f);
  inline void setElement(MElement *element) {
    _element=element;
    for(std::set<dataCacheDouble*>::iterator it = _toInvalidateOnElement.begin(); it!= _toInvalidateOnElement.end(); it++) {
      (*it)->_valid=false;
    }
    for(std::list<dataCacheMap*>::iterator it = _children.begin(); it!= _children.end(); it++) {
      (*it)->setElement(element);
    }
  }
  inline MElement *getElement() {return _element;}
  dataCacheMap() {
    _nbEvaluationPoints = 0;
    _parent=NULL;
  }
  dataCacheMap(dataCacheMap *parent) {
    _parent = parent;
    _parent->_children.push_back(this);
    _nbEvaluationPoints = 0;
  }
  void setNbEvaluationPoints(int nbEvaluationPoints);

  inline int getNbEvaluationPoints(){return _nbEvaluationPoints;}
  ~dataCacheMap();
};

function *functionConstantNew(const std::vector<double>&);
function *functionConstantNew(double);

#endif
