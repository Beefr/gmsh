#include "GmshConfig.h"
#include <sstream>
#include <fstream>
#include "function.h"
#include "SPoint3.h"
#include "MElement.h"
#include "GModel.h"
#include "OS.h"
#include "Bindings.h"

#if defined(HAVE_DLOPEN)
#include <dlfcn.h>
#endif


// function

void function::addFunctionReplace(functionReplace &fr) 
{
  fr._master = this;
  _functionReplaces.push_back(&fr);
}

void function::setArgument(fullMatrix<double> &v, const function *f, int iMap)
{
  if (f == NULL)
    throw;
  arguments.push_back(argument(v, iMap, f));
  dependencies.insert(dependency(iMap, f));
  for (std::set<dependency>::const_iterator it = f->dependencies.begin(); 
       it != f->dependencies.end(); it++) {
    if (it->iMap > 0 && iMap > 0)
      Msg::Error("Consecutive secondary caches");
    dependencies.insert(dependency(iMap + it->iMap, it->f));
  }
  for (unsigned int i = 0; i < _functionReplaces.size(); i++) {
    functionReplace &replace = *_functionReplaces[i];
    for (std::set<dependency>::iterator it = replace._fromParent.begin(); 
         it != replace._fromParent.end(); it++) {
      if (it->iMap > 0 && iMap > 0)
        Msg::Error("Consecutive secondary caches");
      dependencies.insert(dependency(iMap + it->iMap, it->f));
    }
  }
}

functionConstant *function::_timeFunction = NULL;
functionConstant *function::_dtFunction = NULL;

functionConstant *function::getTime()
{
  if (! _timeFunction)
    _timeFunction = new functionConstant(0.);
  return _timeFunction;
}

functionConstant *function::getDT()
{
  if (! _dtFunction)
    _dtFunction = new functionConstant(0.);
  return _dtFunction;
}

functionSolution *functionSolution::_instance = NULL;

function *function::getSolution() 
{
  return functionSolution::get();
}


// Get Solution Gradient + Additionnal class

functionSolutionGradient::functionSolutionGradient():function(0){} 
void functionSolutionGradient::call(dataCacheMap *m, fullMatrix<double> &sol)
{
  Msg::Error("a function requires the gradient of the solution but "
      "this algorithm does not provide the gradient of the solution");
  throw;
}
function *functionSolutionGradient::get()
{
  if(!_instance)
    _instance = new functionSolutionGradient();
  return _instance;
}

functionSolutionGradient *functionSolutionGradient::_instance = NULL;

function *function::getSolutionGradient() 
{
  return functionSolutionGradient::get();
}


// Get Parametric Coordinates + Additionnal class

class functionParametricCoordinates : public function {
  static functionParametricCoordinates *_instance;
  // constructor is private only 1 instance can exists, call get to
  // access the instance
  functionParametricCoordinates():function(0, false){} 
 public:
  void call(dataCacheMap *m, fullMatrix<double> &sol) 
  {
    Msg::Error("a function requires the parametric coordinates but this "
               "algorithm does not provide the parametric coordinates");
    throw;
  }
  static function *get() 
  {
    if(!_instance)
      _instance = new functionParametricCoordinates();
    return _instance;
  }
};

functionParametricCoordinates *functionParametricCoordinates::_instance = NULL;

function *function::getParametricCoordinates() 
{
  return functionParametricCoordinates::get();
}

// Get Normals + Additionnal class

class functionNormals : public function {
  static functionNormals *_instance;
  // constructor is private only 1 instance can exists, call get to
  // access the instance
  functionNormals() : function(0){} 
 public:
  void call(dataCacheMap *m, fullMatrix<double> &sol) 
  {
    Msg::Error("a function requires the normals coordinates but this "
               "algorithm does not provide the normals");
    throw;
  }
  static function *get() 
  {
    if(!_instance)
      _instance = new functionNormals();
    return _instance;
  }
};

functionNormals *functionNormals::_instance = NULL;

function *function::getNormals() 
{
  return functionNormals::get();
}

// functionReplace

functionReplace::functionReplace() 
{
  _nChildren = 0;
}

void functionReplace::get(fullMatrix<double> &v, const function *f, int iMap) 
{
  bool allDepFromParent = true;
  for (std::set<function::dependency>::const_iterator itDep = f->dependencies.begin();
       itDep != f->dependencies.end(); itDep++){
    bool depFromParent = (_replaced.count(*itDep) == 0);
    for (std::set<function::dependency>::const_iterator itDep2 = itDep->f->dependencies.begin(); 
         itDep2 != itDep->f->dependencies.end() && depFromParent; itDep2++)
      depFromParent &= (_replaced.count(*itDep2) == 0);
    if(depFromParent)
      _master->dependencies.insert(*itDep);
    else
      allDepFromParent = false;
  }
  function::dependency asDep(iMap, f);
  if (allDepFromParent && _replaced.count(asDep) == 0)
    _master->dependencies.insert(asDep);
  _toCompute.push_back(function::argument(v, iMap, f));
}

void functionReplace::replace(fullMatrix<double> &v, const function *f, int iMap) 
{
  _replaced.insert(function::dependency(iMap, f));
  _toReplace.push_back(function::argument(v, iMap, f));
}

void functionReplace::addChild() 
{
  _nChildren++;
}

void functionReplace::compute() 
{
  for (unsigned i = 0; i < _toReplace.size(); i++)
    currentCache->toReplace[i]->set();
  for (unsigned i = 0; i < _toCompute.size(); i++)
    _toCompute[i].val->setAsProxy(currentCache->toCompute[i]->get());
};


// dataCacheDouble 

dataCacheDouble::dataCacheDouble(dataCacheMap *m, function *f):
  _cacheMap(*m), _value(m->getNbEvaluationPoints(), f->getNbCol()), _function(f)
{
  m->addDataCacheDouble(this, f->isInvalitedOnElement());
  _valid = false;
}

void dataCacheDouble::resize(int nrow) 
{
  _value.resize(nrow, _value.size2());
  _valid = false;
}

void dataCacheDouble::_eval() 
{
  for(unsigned int i = 0; i < _directDependencies.size(); i++){
    _function->arguments[i].val->setAsProxy(_directDependencies[i]->get());
  }
  for (unsigned i = 0; i < _function->_functionReplaces.size(); i++) {
    _function->_functionReplaces[i]->currentCache = &functionReplaceCaches[i];
    for (unsigned j = 0; j < functionReplaceCaches[i].toReplace.size(); j++){
      _function->_functionReplaces[i]->_toReplace[j].val->setAsProxy
        ((*functionReplaceCaches[i].toReplace[j])._value);
    }
  }
  _function->call(&_cacheMap, _value);
  _valid = true;
}

const function * dataCacheMap::_translate(const function *f) const 
{
  //special case
  if (f == function::getSolution()) {
    f = _functionSolution;
    if (f == NULL) {
      dataCacheMap *parent = _parent;
      while (parent) {
        f = parent->_functionSolution;
        if (f) break;
        parent = parent->_parent;
      }
      if (f == NULL) 
        Msg::Error ("solution function has not been set");
    }
  } else if (f == function::getSolutionGradient()) {
    f = _functionSolutionGradient;
    if (f == NULL) {
      dataCacheMap *parent = _parent;
      while (parent) {
        f = parent->_functionSolutionGradient;
        if (f) break;
        parent = parent->_parent;
      }
      if (f == NULL) 
      Msg::Error ("solution function gradient has not been set");
    }
  }
  return f;
}

dataCacheDouble *dataCacheMap::get(const function *f, dataCacheDouble *caller, bool createIfNotPresent)
{
  f = _translate(f);
  // do I have a cache for this function ?
  dataCacheDouble *&r = _cacheDoubleMap[f];
  // can I use the cache of my parent ?
  if(_parent && r == NULL) {
    bool okFromParent = true;
    for (std::set<function::dependency>::const_iterator it = f->dependencies.begin(); 
         it != f->dependencies.end(); it++) {
      if (it->iMap > _parent->_secondaryCaches.size())
        okFromParent = false;
      dataCacheMap *m = getSecondaryCache(it->iMap);
      if (m->_cacheDoubleMap.find(it->f) != m->_cacheDoubleMap.end()) {
        okFromParent = false;
        break;
      }
    }
    if (okFromParent)
      r = _parent->get (f, caller);
  }
  // no cache found, create a new one
  if (r == NULL) {
    if (!createIfNotPresent) 
      return NULL;
    r = new dataCacheDouble (this, (function*)(f));
    r->_directDependencies.resize (f->arguments.size());
    for (unsigned int i = 0; i < f->arguments.size(); i++) {
      r->_directDependencies[i] = 
        getSecondaryCache(f->arguments[i].iMap)->get(f->arguments[i].f, r);
    }
    for (unsigned i = 0; i < f->_functionReplaces.size(); i++) {
      functionReplaceCache replaceCache;
      functionReplace *replace = f->_functionReplaces[i];
      dataCacheMap *rMap = newChild();
      for (unsigned i = 0; i < _secondaryCaches.size(); i++)
        rMap->addSecondaryCache (getSecondaryCache(i+1)->newChild());
      for (int i = 0; i < replace->_nChildren; i++)
        rMap->addSecondaryCache (rMap->newChild());
      for (std::vector<function::argument>::iterator it = replace->_toReplace.begin();
           it!= replace->_toReplace.end(); it++) {
        dataCacheMap *m = rMap->getSecondaryCache(it->iMap);
        dataCacheDouble *s = new dataCacheDouble(m, (function*)_translate(it->f));
        m->_cacheDoubleMap[_translate(it->f)] = s;
        replaceCache.toReplace.push_back(s);
      }
      for (std::vector<function::argument>::iterator it = replace->_toCompute.begin();
           it!= replace->_toCompute.end(); it++ ) {
        replaceCache.toCompute.push_back(rMap->getSecondaryCache(it->iMap)->get(it->f, r));
      }
      replaceCache.map = rMap;
      r->functionReplaceCaches.push_back (replaceCache); 
    }
    ((function*)f)->registerInDataCacheMap(this, r);
  }

  // update the dependency tree
  if (caller) {
    r->_dependOnMe.insert(caller);
    caller->_iDependOn.insert(r);
    for(std::set<dataCacheDouble*>::iterator it = r->_iDependOn.begin();
        it != r->_iDependOn.end(); it++) {
      (*it)->_dependOnMe.insert(caller);
      caller->_iDependOn.insert(*it);
    }
  }
  return r;
}

// dataCacheMap

dataCacheMap::~dataCacheMap()
{
  for (std::set<dataCacheDouble*>::iterator it = _allDataCaches.begin();
      it!=_allDataCaches.end(); it++) {
    delete *it;
  }
  for (std::list<dataCacheMap*>::iterator it = _children.begin();
       it != _children.end(); it++) {
    delete *it;
  }
}

void dataCacheMap::setNbEvaluationPoints(int nbEvaluationPoints) 
{
  if (_nbEvaluationPoints == nbEvaluationPoints)
    return;
  _nbEvaluationPoints = nbEvaluationPoints;
  for(std::set<dataCacheDouble*>::iterator it = _allDataCaches.begin();
      it != _allDataCaches.end(); it++){
    (*it)->resize(nbEvaluationPoints);
  }
    for(std::list<dataCacheMap*>::iterator it = _children.begin();
        it != _children.end(); it++) {
      (*it)->setNbEvaluationPoints(nbEvaluationPoints);
    }
}

// Some examples of functions

// functionConstant (constant values copied over each line)

void functionConstant::set(double val) 
{
  if(getNbCol() != 1)
    Msg::Error ("set scalar value on a vectorial constant function");
  _source(0, 0) = val;
}

void functionConstant::call(dataCacheMap *m, fullMatrix<double> &val)
{
  for (int i = 0; i < val.size1(); i++)
    for (int j = 0; j < _source.size1(); j++)
      val(i, j)=_source(j, 0);
}

functionConstant::functionConstant(std::vector<double> source) : function(source.size())
{
  _source = fullMatrix<double>(source.size(), 1);
  for (size_t i = 0; i < source.size(); i++)
    _source(i, 0) = source[i];
}

functionConstant::functionConstant(double source) : function(1)
{
  _source.resize(1, 1);
  _source(0, 0) = source;
}

// functionSum

class functionSum : public function {
 public:
  fullMatrix<double> _f0, _f1;
  void call(dataCacheMap *m, fullMatrix<double> &val) 
  {
    for (int i = 0; i < val.size1(); i++)
      for (int j = 0; j < val.size2(); j++)
        val(i, j)= _f0(i, j) + _f1(i, j);
  }
  functionSum(const function *f0, const function *f1) : function(f0->getNbCol()) 
  {
    if (f0->getNbCol() != f1->getNbCol()) {
      Msg::Error("trying to sum 2 functions of different sizes: %d %d\n",
                 f0->getNbCol(), f1->getNbCol());
      throw;
    }
    setArgument (_f0, f0);
    setArgument (_f1, f1);
  }
};

function *functionSumNew(const function *f0, const function *f1) 
{
  return new functionSum (f0, f1);
}

// functionLevelset

class functionLevelset : public function {
 public:
  fullMatrix<double> _f0;
  double _valMin, _valPlus;
  void call(dataCacheMap *m, fullMatrix<double> &val) 
  {
    for (int i = 0; i < val.size1(); i++)
      for (int j = 0; j < val.size2(); j++){
	val(i, j)= _valPlus;
	if (_f0(i,j) < 0.0)
	  val(i,j) = _valMin;
      }
  }
  functionLevelset(const function *f0, const double valMin, const double valPlus) : function(f0->getNbCol()) 
  {
    setArgument (_f0, f0);
    _valMin  = valMin;
    _valPlus = valPlus;
  }
};

function *functionLevelsetNew(const function *f0, const double valMin, const double valPlus) 
{
  return new functionLevelset (f0, valMin, valPlus);
}

class functionLevelsetSmooth : public function {
 public:
  fullMatrix<double> _f0;
  double _valMin, _valPlus, _E;
  void call(dataCacheMap *m, fullMatrix<double> &val) 
  {
    
    for (int i = 0; i < val.size1(); i++)
      for (int j = 0; j < val.size2(); j++){
	double phi = _f0(i,j);
	double Heps= 0.5+0.5*phi/_E + 0.5/3.14*sin(3.14*phi/_E);
	if ( fabs(phi) < _E)  val(i, j)= Heps*_valPlus + (1-Heps)*_valMin;
	else if (phi >  _E)   val(i, j) = _valPlus;
	else if (phi < -_E)   val(i, j) = _valMin;
      }
  }
  functionLevelsetSmooth(const function *f0, const double valMin, const double valPlus, const double E) : function(f0->getNbCol()) 
  {
    setArgument (_f0, f0);
    _valMin  = valMin;
    _valPlus = valPlus;
    _E = E;
  }
};

function *functionLevelsetSmoothNew(const function *f0, const double valMin, const double valPlus, const double E) 
{
  return new functionLevelsetSmooth (f0, valMin, valPlus, E);
}


// functionProd

class functionProd : public function {
 public:
  fullMatrix<double> _f0, _f1;
  void call(dataCacheMap *m, fullMatrix<double> &val) 
  {
    for (int i = 0; i < val.size1(); i++)
      for (int j = 0; j < val.size2(); j++)
        val(i, j)= _f0(i, j) * _f1(i, j);
  }
  functionProd(const function *f0, const function *f1) : function(f0->getNbCol()) 
  {
    if (f0->getNbCol() != f1->getNbCol()) {
      Msg::Error("trying to compute product of 2 functions of different sizes: %d %d\n",
                 f0->getNbCol(), f1->getNbCol());
      throw;
    }
    setArgument (_f0, f0);
    setArgument (_f1, f1);
  }
};

function *functionProdNew(const function *f0, const function *f1) 
{
  return new functionProd (f0, f1);
}

// functionExtractComp

class functionExtractComp : public function {
  public:
  fullMatrix<double> _f0;
  int _iComp;
  void call(dataCacheMap *m, fullMatrix<double> &val) 
  {
    for (int i = 0; i < val.size1(); i++)
        val(i, 0) = _f0(i, _iComp);
  }
  functionExtractComp(const function *f0, const int iComp) : function(1) 
  {
    setArgument (_f0, f0);
    _iComp = iComp;
  }
};

function *functionExtractCompNew(const function *f0, const int iComp) 
{
  return new functionExtractComp (f0, iComp);
}

// functionCatComp

class functionCatComp : public function {
  public:
  int _nComp;
  std::vector<fullMatrix<double> > _fMatrix;
  void call(dataCacheMap *m, fullMatrix<double> &val)
  {
    for (int i = 0; i<val.size1(); i++)
      for (int comp = 0; comp < _nComp; comp++)
        val(i,comp) = _fMatrix[comp](i, 0);
  }
  functionCatComp(std::vector<const function *> fArray) : function(fArray.size())
  {
    _nComp = fArray.size();
    _fMatrix.resize(_nComp);
    for (int i = 0; i < _nComp; i++)
      setArgument (_fMatrix[i], fArray[i]);
  }
};

function *functionCatCompNew(std::vector<const function *> fArray)
{
  return new functionCatComp (fArray);
}

// functionScale

class functionScale : public function {
 public:
  fullMatrix<double> _f0;
  double _s;
  void call(dataCacheMap *m, fullMatrix<double> &val) 
  {
    for(int i = 0; i < val.size1(); i++)
      for(int j = 0; j < val.size2(); j++)
        val(i, j) = _f0(i, j)*_s;
  }
  functionScale(const function *f0, const double s) : function(f0->getNbCol()) 
  {
    setArgument (_f0, f0);
    _s = s;
  }
};

function *functionScaleNew(const function *f0, const double s) {
  return new functionScale (f0, s);
}

// functionMean

class functionMean : public function {
  fullMatrix<double> _f0;
public:
  functionMean(const function *f0) : function(f0->getNbCol()) {
    setArgument (_f0, f0);
  }
  void call(dataCacheMap *m, fullMatrix<double> &val)
  {
    double mean;
    for(int j = 0; j < val.size2(); j++) {
      mean = 0;
      for(int i = 0; i < val.size1(); i++)
        mean += _f0(i, j);
      mean /= (double) val.size1();
      for(int i = 0; i < val.size1(); i++)
        val(i, j) = mean;
    }
  }
};

function *functionMeanNew(const function *f0) {
  return new functionMean (f0);
}


// functionCoordinates (get XYZ coordinates)

class functionCoordinates : public function {
  static functionCoordinates *_instance;
  fullMatrix<double> uvw;
  void call (dataCacheMap *m, fullMatrix<double> &xyz) 
  {
    for (int i = 0; i < uvw.size1(); i++) {
      SPoint3 p;
      m->getElement()->pnt(uvw(i, 0), uvw(i, 1), uvw(i, 2), p);
      xyz(i, 0) = p.x();
      xyz(i, 1) = p.y();
      xyz(i, 2) = p.z();
    }
  }
  functionCoordinates() : function(3) 
  { 
    // constructor is private only 1 instance can exists, call get to
    // access the instance
    setArgument(uvw, function::getParametricCoordinates());
  };
 public:
  static function *get() 
  {
    if(!_instance)
      _instance = new functionCoordinates();
    return _instance;
  }
};

functionCoordinates *functionCoordinates::_instance = NULL;

function *getFunctionCoordinates() 
{
  return functionCoordinates::get();
}

// functionStructuredGridFile

class functionStructuredGridFile : public function {
  fullMatrix<double> coord;
 public:
  int n[3];
  double d[3], o[3];
  double get(int i, int j, int k) 
  {
    return v[(i * n[1] + j) * n[2] + k];
  }
  double *v;
  void call(dataCacheMap *m, fullMatrix<double> &val) 
  {
    for (int pt = 0; pt < val.size1(); pt++) {
      double xi[3];
      int id[3];
      for(int i = 0; i < 3; i++){
        id[i] = (int)((coord(pt, i) - o[i]) / d[i]);
        int a = id[i];
        id[i] = std::max(0, std::min(n[i] - 1, id[i]));
        xi[i] = (coord(pt,i) - o[i] - id[i] * d[i]) / d[i];
        xi[i] = std::min(1., std::max(0., xi[i]));
      }
      val(pt, 0) =
        get(id[0]  , id[1]  , id[2]   ) * (1-xi[0]) * (1-xi[1]) * (1-xi[2]) +
        get(id[0]  , id[1]  , id[2]+1 ) * (1-xi[0]) * (1-xi[1]) * (  xi[2]) +
        get(id[0]  , id[1]+1, id[2]   ) * (1-xi[0]) * (  xi[1]) * (1-xi[2]) +
        get(id[0]  , id[1]+1, id[2]+1 ) * (1-xi[0]) * (  xi[1]) * (  xi[2]) +
        get(id[0]+1, id[1]  , id[2]   ) * (  xi[0]) * (1-xi[1]) * (1-xi[2]) +
        get(id[0]+1, id[1]  , id[2]+1 ) * (  xi[0]) * (1-xi[1]) * (  xi[2]) +
        get(id[0]+1, id[1]+1, id[2]   ) * (  xi[0]) * (  xi[1]) * (1-xi[2]) +
        get(id[0]+1, id[1]+1, id[2]+1 ) * (  xi[0]) * (  xi[1]) * (  xi[2]);
    }
  }
  functionStructuredGridFile(const std::string filename, const function *coordFunction)
    : function(1) 
  {
    setArgument(coord, coordFunction);
    std::ifstream input(filename.c_str());
    if(!input)
      Msg::Error("cannot open file : %s",filename.c_str());
    if(filename.substr(filename.size()-4, 4) != ".bin") {
      input >> o[0] >> o[1] >> o[2] >> d[0] >> d[1] >> d[2] >> n[0] >> n[1] >> n[2];
      int nt = n[0] * n[1] * n[2];
      v = new double [nt];
      for (int i = 0; i < nt; i++)
        input >> v[i];
    } else {
      input.read((char *)o, 3 * sizeof(double));
      input.read((char *)d, 3 * sizeof(double));
      input.read((char *)n, 3 * sizeof(int));
      int nt = n[0] * n[1] * n[2];
      v = new double[nt];
      input.read((char *)v, nt * sizeof(double));
    }
  }
  ~functionStructuredGridFile() 
  {
    delete []v;
  }
};

// functionLua

#ifdef HAVE_LUA
class functionLua : public function {
  lua_State *_L;
  std::string _luaFunctionName;
  std::vector<fullMatrix<double> > args;
 public:
  void call (dataCacheMap *m, fullMatrix<double> &res) 
  {
    lua_getfield(_L, LUA_GLOBALSINDEX, _luaFunctionName.c_str());
    for (int i = 0; i < arguments.size(); i++)
      luaStack<const fullMatrix<double>*>::push(_L, &args[i]);
    luaStack<const fullMatrix<double>*>::push(_L, &res);
    lua_call(_L, arguments.size()+1, 0);
  }
  functionLua (int nbCol, std::string luaFunctionName, 
               std::vector<const function*> dependencies, lua_State *L)
    : function(nbCol), _luaFunctionName(luaFunctionName), _L(L)
  {
    args.resize(dependencies.size());
    for (int i = 0; i < dependencies.size(); i++)
      setArgument(args[i], dependencies[i]);
  }
};
#endif

// functionC
void functionC::buildLibraryFromFile(const std::string cfilename, const std::string libfilename) {
  FILE *tmpMake = fopen("_tmpMake", "w");
  fprintf(tmpMake, 
      "include $(DG_BUILD_DIR)/CMakeFiles/dgshared.dir/flags.make\n"
      "%s : %s\n"
      "\tg++ -fPIC -shared -o $@ $(CXX_FLAGS) $(CXX_DEFINES) $<\n",
      libfilename.c_str(), cfilename.c_str());
  fclose(tmpMake);
  if(system("make -f _tmpMake"))
    Msg::Error("make command failed\n");
  UnlinkFile("_tmpMake.cpp");
}

void functionC::buildLibrary(std::string code, std::string filename) 
{
  FILE *tmpSrc = fopen("_tmpSrc.cpp", "w");
  fprintf(tmpSrc, "%s\n",code.c_str());
  fclose(tmpSrc);
  buildLibraryFromFile("_tmpSrc.cpp", filename);
  UnlinkFile("_tmpSrc.cpp");
}
void functionC::call (dataCacheMap *m, fullMatrix<double> &val) 
{
  switch (args.size()) {
    case 0 : 
      ((void (*)(dataCacheMap*, fullMatrix<double> &))(callback))(m, val);
      break;
    case 1 : 
      ((void (*)(dataCacheMap*, fullMatrix<double> &, const fullMatrix<double>&))
       (callback)) (m, val, args[0]);
      break;
    case 2 : 
      ((void (*)(dataCacheMap*, fullMatrix<double> &, const fullMatrix<double>&, 
                 const fullMatrix<double> &))
       (callback)) (m, val, args[0], args[1]);
      break;
    case 3 : 
      ((void (*)(dataCacheMap*, fullMatrix<double> &, const fullMatrix<double>&, 
                 const fullMatrix<double>&, const fullMatrix<double>&))
       (callback)) (m, val, args[0], args[1], args[2]);
      break;
    case 4 : 
      ((void (*)(dataCacheMap*, fullMatrix<double> &, const fullMatrix<double>&,
                 const fullMatrix<double>&, const fullMatrix<double>&,
                 const fullMatrix<double>&))
       (callback)) (m, val, args[0], args[1], args[2], args[3]);
      break;
    case 5 : 
      ((void (*)(dataCacheMap*, fullMatrix<double> &, const fullMatrix<double>&, 
                 const fullMatrix<double>&, const fullMatrix<double>&,
                 const fullMatrix<double>&, const fullMatrix<double>&))
       (callback)) (m, val, args[0], args[1], args[2], args[3], args[4]);
      break;
    case 6 : 
      ((void (*)(dataCacheMap*, fullMatrix<double> &, const fullMatrix<double>&,
                 const fullMatrix<double>&, const fullMatrix<double>&,
                 const fullMatrix<double>&, const fullMatrix<double>&, 
                 const fullMatrix<double>&))
       (callback)) (m, val, args[0], args[1], args[2], args[3], args[4], args[5]);
      break;
    default :
      Msg::Error("C callback not implemented for %i argurments", args.size());
  }
}
functionC::functionC (std::string file, std::string symbol, int nbCol, 
    std::vector<const function *> dependencies):
  function(nbCol)
{
#if defined(HAVE_DLOPEN)
  args.resize(dependencies.size());
  for(int i = 0; i < dependencies.size(); i++) {
    setArgument(args[i], dependencies[i]);
  }
  void *dlHandler;
  dlHandler = dlopen(file.c_str(), RTLD_NOW);
  callback = (void(*)(void))dlsym(dlHandler, symbol.c_str());
  if(!callback)
    Msg::Error("Cannot get the callback to the compiled C function");
#else
  Msg::Error("Cannot construct functionC without dlopen");
#endif
}


void function::registerBindings(binding *b)
{
  classBinding *cb = b->addClass<function>("Function");
  cb->setDescription("A generic function that can be evaluated on a set of points. "
                     "Functions can call other functions and their values are cached "
                     "so that if two different functions call the same function f, "
                     "f is only evaluated once.");
  methodBinding *mb;

  mb = cb->addMethod("getTime", &function::getTime);
  mb->setDescription("Return function constant in space which contains the time value");

  cb = b->addClass<functionConstant>("functionConstant");
  cb->setDescription("A constant (scalar or vector) function");
  mb = cb->setConstructor<functionConstant, std::vector <double> >();
  mb->setArgNames("v", NULL);
  mb->setDescription("A new constant function wich values 'v' everywhere. v can be a row-vector.");
  cb->setParentClass<function>();

  cb = b->addClass<functionSum>("functionSum");
  cb->setDescription("A sum of two functions 'a + b'. The arguments a, b must have same dimension.");
  mb = cb->setConstructor<functionSum, const function*, const function*>();
  mb->setArgNames("a", "b", NULL);
  mb->setDescription("Creates a new functionSum instance with given arguments");
  cb->setParentClass<function>();

  cb = b->addClass<functionProd>("functionProd");
  cb->setDescription("A pointwise product of two functions 'a(i,j)*b(i,j)'. The arguments a, b must have same dimension.");
  mb = cb->setConstructor<functionProd, const function*, const function*>();
  mb->setArgNames("a", "b", NULL);
  mb->setDescription("Creates a new functionProd instance with given arguments");
  cb->setParentClass<function>();

  cb = b->addClass<functionExtractComp>("functionExtractComp");
  cb->setDescription("Extracts a given component of the vector valued function.");
  mb = cb->setConstructor<functionExtractComp, const function*, int>();
  mb->setArgNames("function","component", NULL);
  mb->setDescription("Creates a new functionExtractComp instance with given arguments");
  cb->setParentClass<function>();

  cb = b->addClass<functionCatComp>("functionCatComp");
  cb->setDescription("Creates a vector valued function by concatenating the given scalar functions. Uses only the first component of each function.");
  mb = cb->setConstructor<functionCatComp, std::vector <const function*> >();
  mb->setArgNames("functionArray", NULL);
  mb->setDescription("Creates a new functionCatComp instance with given arguments");
  cb->setParentClass<function>();

  cb = b->addClass<functionScale>("functionScale");
  cb->setDescription("Scales a function by a given scalar.");
  mb = cb->setConstructor<functionScale, const function*, double>();
  mb->setArgNames("function","scalar", NULL);
  mb->setDescription("Creates a new functionScale instance with given arguments");
  cb->setParentClass<function>();

  cb = b->addClass<functionCoordinates>("functionCoordinates");
  cb->setDescription("A function to access the coordinates (xyz). This is a "
                     "single-instance class, use the 'get' member to access the instance.");
  mb = cb->addMethod("get", &functionCoordinates::get);
  mb->setDescription("return the unique instance of this class");
  cb->setParentClass<function>();

  cb = b->addClass<functionSolution>("functionSolution");
  cb->setDescription("A function to access the solution. This is a single-instance "
                     "class, use the 'get' member to access the instance.");
  mb = cb->addMethod("get", &functionSolution::get);
  mb->setDescription("return the unique instance of this class");
  cb->setParentClass<function>();

  cb = b->addClass<functionNormals>("functionNormals");
  cb->setDescription("A function to access the face normals. This is a single-instance "
                     "class, use the 'get' member to access the instance.");
  mb = cb->addMethod("get", &functionNormals::get);
  mb->setDescription("return the unique instance of this class");
  cb->setParentClass<function>();

  cb = b->addClass<functionSolutionGradient>("functionSolutionGradient");
  cb->setDescription("A function to access the gradient of the solution. This is "
                     "a single-instance class, use the 'get' member to access the instance.");
  mb = cb->addMethod("get", &functionCoordinates::get);
  mb->setDescription("return the unique instance of this class");
  cb->setParentClass<function>();

  cb = b->addClass<functionStructuredGridFile>("functionStructuredGridFile");
  cb->setParentClass<function>();
  cb->setDescription("A function to interpolate through data given on a structured grid");
  mb = cb->setConstructor<functionStructuredGridFile, std::string, const function*>();
  mb->setArgNames("fileName","coordinateFunction",NULL);
  mb->setDescription("Tri-linearly interpolate through data in file 'fileName' at "
                     "coordinate given by 'coordinateFunction'.\nThe file format "
                     "is :\nx0 y0 z0\ndx dy dz\nnx ny nz\nv(0,0,0) v(0,0,1) v(0 0 2) ...");

#if defined(HAVE_DLOPEN)
  cb = b->addClass<functionC>("functionC");
  cb->setDescription("A function that compile a C code");
  mb = cb->setConstructor<functionC, std::string, std::string, int, std::vector<const function*> >();
  mb->setArgNames("file", "symbol", "nbCol", "arguments", NULL);
  mb->setDescription("  ");
  mb = cb->addMethod("buildLibrary", &functionC::buildLibrary);
  mb->setArgNames("code", "libraryFileName", NULL);
  mb->setDescription("build a dynamic library from the given code");
  cb->setParentClass<function>();
#endif

#if defined (HAVE_LUA)
  cb= b->addClass<functionLua>("functionLua");
  cb->setDescription("A function (see the 'function' documentation entry) defined in LUA.");
  mb = cb->setConstructor<functionLua, int, std::string, std::vector<const function*>, lua_State*>();
  mb->setArgNames("d", "f", "dep", NULL);
  mb->setDescription("A new functionLua which evaluates a vector of dimension 'd' "
                     "using the lua function 'f'. This function can take other functions "
                     "as arguments listed by the 'dep' vector.");
  cb->setParentClass<function>();
#endif
}

