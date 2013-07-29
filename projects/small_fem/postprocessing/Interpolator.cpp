#include "Interpolator.h"

#include "FunctionSpaceScalar.h"
#include "FunctionSpaceVector.h"

#include "Exception.h"
#include "GModel.h"
#include "MElement.h"
#include "MVertex.h"

#include "Dof.h"

using namespace std;

void Interpolator::initSystem(const System& system){
  // Save some data
  this->dofM = &(system.getDofManager());
  this->fs   = &(system.getFunctionSpace());

  // Get Mesh
  this->mesh = &(fs->getSupport().getMesh());

  // Get Solution
  ownSol = false;
  sol    = &(system.getSol());

  // Init
  nodalScalarValue = NULL;
  nodalVectorValue = NULL;

  // Scalar or Vector ?
  scalar = fs->getBasis(0).isScalar();
}

void Interpolator::initSystem(const SystemEigen& system,
                              size_t eigenNumber){
  // Save some data
  this->dofM = &(system.getDofManager());
  this->fs   = &(system.getFunctionSpace());

  // Get Mesh
  this->mesh = &(fs->getSupport().getMesh());

  // Get Solution
  ownSol = true;
  sol    = getSol(system.getEigenVectors(), eigenNumber);

  // Init
  nodalScalarValue = NULL;
  nodalVectorValue = NULL;

  // Scalar or Vector ?
  scalar = fs->getBasis(0).isScalar();
}

Interpolator::Interpolator(const System& system){
  // Init
  initSystem(system);

  // Get Visu Domain
  this->visuDomain = &(fs->getSupport());

  // Interpolate
  // NB: interpolate() is faster than
  // interpolateOnVisu() (no Octree)
  interpolate();
}

Interpolator::Interpolator(const System& system,
                           const GroupOfElement& visu){
  // Init
  initSystem(system);

  // Get Visu Domain
  this->visuDomain = &visu;

  // Interpolate
  // NB: Can't use interpolate(), because
  // we don't have *all* nodes (visu mesh)
  // with Dofs
  interpolateOnVisu();
}

Interpolator::Interpolator(const SystemEigen& system,
                           size_t eigenNumber){
  // Init
  initSystem(system, eigenNumber);

  // Get Visu Domain
  this->visuDomain = &(fs->getSupport());

  // Interpolate
  // NB: interpolate() is faster than
  // interpolateOnVisu() (no Octree)
  interpolate();
}

Interpolator::Interpolator(const SystemEigen& system,
                           size_t eigenNumber,
                           const GroupOfElement& visu){
  // Init
  initSystem(system, eigenNumber);

  // Get Visu Domain
  this->visuDomain = &visu;

  // Interpolate
  // NB: Can't use interpolate(), because
  // we don't have *all* nodes (visu mesh)
  // with Dofs
  interpolateOnVisu();
}

Interpolator::Interpolator(double (*f)(fullVector<double>& xyz),
                           const GroupOfElement& visu){
  // Init
  scalar = true;
  ownSol = false;
  nodalScalarValue = NULL;
  nodalVectorValue = NULL;

  // Get Visu Domain
  this->visuDomain = &visu;

  // Get Function
  fScalar = f;

  // Evaluate f on visu
  evaluateF();
}

Interpolator::Interpolator(fullVector<double> (*f)(fullVector<double>& xyz),
                           const GroupOfElement& visu){
  // Init
  scalar = false;
  ownSol = false;
  nodalScalarValue = NULL;
  nodalVectorValue = NULL;

  // Get Visu Domain
  this->visuDomain = &visu;

  // Get Function
  fVector = f;

  // Evaluate f on visu
  evaluateF();
}

Interpolator::~Interpolator(void){
  if(ownSol)
    delete sol;

  if(scalar)
    delete nodalScalarValue;

  else
    delete nodalVectorValue;
}

void Interpolator::write(const std::string name, Writer& writer) const{
  // Set Writer
  writer.setDomain(*visuDomain);

  if(scalar)
    writer.setValues(*nodalScalarValue);

  else
    writer.setValues(*nodalVectorValue);

  // Write
  writer.write(name);
}

void Interpolator::interpolate(void){
  // Init
  const size_t nTotVertex             = mesh->getVertexNumber();
  const std::vector<GroupOfDof*>& god = fs->getAllGroups();
  const size_t nGod                   = god.size();

  vector<bool> isInterpolated(nTotVertex, false);
  vector<MVertex*> node;

  // Scalar or Vector ?
  const FunctionSpaceScalar* fsScalar = NULL;
  const FunctionSpaceVector* fsVector = NULL;

  // Temporary //
  size_t globalId;

  if(scalar){
    nodalScalarValue = new vector<double>(nTotVertex);
    fsScalar = static_cast<const FunctionSpaceScalar*>(fs);
  }

  else{
    nodalVectorValue = new vector<fullVector<double> >(nTotVertex);
    fsVector = static_cast<const FunctionSpaceVector*>(fs);
  }


  // Iterate on GroupOfElement
  for(size_t i = 0; i < nGod; i++){
    // Get Element
    MElement& element =
      const_cast<MElement&>(god[i]->getElement());

    // Get NodeS
    element.getVertices(node);
    const size_t nNode = node.size();

    // Iterate on Node
    for(size_t j = 0; j < nNode; j++){
      // Get *GMSH* Id
      const size_t id = node[j]->getNum() - 1;

      // If not interpolated: interpolate :-P !
      if(!isInterpolated[id]){
        // Get Dof
        const vector<Dof>& dof  = god[i]->getDof();
        const size_t       size = dof.size();

        // Get Coef
        vector<double> coef(size);
        for(size_t k = 0; k < size; k++){
          // Dof Global ID
          globalId = dofM->getGlobalId(dof[k]);

          // If non fixed Dof: look in Solution
          if(globalId != DofManager::isFixedId())
            coef[k] = (*sol)(globalId);

          // If Dof is fixed: get fixed value
          else
            coef[k] = dofM->getValue(dof[k]);
        }

        // Get Node coordinate
        fullVector<double> xyz(3);
        xyz(0) = node[j]->x();
        xyz(1) = node[j]->y();
        xyz(2) = node[j]->z();

        // Interpolate (AT LAST !!)
        if(scalar)
          (*nodalScalarValue)[id] =
            fsScalar->interpolate(element, coef, xyz);

        else
          (*nodalVectorValue)[id] =
            fsVector->interpolate(element, coef, xyz);

        isInterpolated[id] = true;
      }
    }
  }
}

void Interpolator::interpolateOnVisu(void){
  // Init
  const Mesh& visuMesh              = visuDomain->getMesh();
  const size_t nTotVertex           = visuMesh.getVertexNumber();
  const vector<const MVertex*> node = visuMesh.getAllVertex();

  // Scalar or Vector ?
  const FunctionSpaceScalar* fsScalar = NULL;
  const FunctionSpaceVector* fsVector = NULL;

  // Temporary //
  size_t globalId;

  if(scalar){
    nodalScalarValue = new vector<double>(nTotVertex);
    fsScalar = static_cast<const FunctionSpaceScalar*>(fs);
  }

  else{
    nodalVectorValue = new vector<fullVector<double> >(nTotVertex);
    fsVector = static_cast<const FunctionSpaceVector*>(fs);
  }

  // Get Model for Octrees
  GModel& model = mesh->getModel();
  const int dim = model.getDim();

  // Iterate on *NODES*
  for(size_t i = 0; i < nTotVertex; i++){
    // Search element (in System Mesh) containg this
    // visu node
    SPoint3   point   = node[i]->point();
    MElement* element = model.getMeshElementByCoord(point, dim, true);

    // WARNING: if no element found, set to zero
    if(!element){
      if(scalar)
        (*nodalScalarValue)[node[i]->getNum() - 1] = 0;

      else
        (*nodalVectorValue)[node[i]->getNum() - 1] = 0;
    }

    else{
      // Get GroupOfDof related to this Element
      const GroupOfDof& god = fs->getGoDFromElement(*element);

      // Get Dof
      const vector<Dof>& dof  = god.getDof();
      const size_t       size = dof.size();

      // Get Coef
      vector<double> coef(size);
      for(size_t k = 0; k < size; k++){
        // Dof Global ID
        globalId = dofM->getGlobalId(dof[k]);

        // If non fixed Dof: look in Solution
        if(globalId != DofManager::isFixedId())
          coef[k] = (*sol)(globalId);

        // If Dof is fixed: get fixed value
        else
          coef[k] = dofM->getValue(dof[k]);
      }

      // Get Node coordinate
      fullVector<double> xyz(3);
      xyz(0) = node[i]->x();
      xyz(1) = node[i]->y();
      xyz(2) = node[i]->z();

      // Interpolate (AT LAST !!)
      if(scalar)
        (*nodalScalarValue)[node[i]->getNum() - 1] =
          fsScalar->interpolate(*element, coef, xyz);

      else
        (*nodalVectorValue)[node[i]->getNum() - 1] =
          fsVector->interpolate(*element, coef, xyz);
    }
  }
}

void Interpolator::evaluateF(void){
  // Init
  const Mesh& visuMesh              = visuDomain->getMesh();
  const size_t nTotVertex           = visuMesh.getVertexNumber();
  const vector<const MVertex*> node = visuMesh.getAllVertex();

  // Scalar or Vector ?
  if(scalar)
    nodalScalarValue = new vector<double>(nTotVertex);

  else
    nodalVectorValue = new vector<fullVector<double> >(nTotVertex);

  // Iterate on *NODES*
  for(size_t i = 0; i < nTotVertex; i++){
    // Get Node coordinate
    fullVector<double> xyz(3);
    xyz(0) = node[i]->x();
    xyz(1) = node[i]->y();
    xyz(2) = node[i]->z();

    // Evaluate (AT LAST !!)
    if(scalar)
      (*nodalScalarValue)[node[i]->getNum() - 1] =
        fScalar(xyz);

    else
      (*nodalVectorValue)[node[i]->getNum() - 1] =
        fVector(xyz);
  }
}

const fullVector<double>* Interpolator::
getSol(const vector<fullVector<complex<double> > >& eVector,
       size_t eigenNumber){

  // Init
  size_t size             = eVector[eigenNumber].size();
  fullVector<double>* sol = new fullVector<double>(size);

  // Get Sol
  for(size_t i = 0; i < size; i++){
    (*sol)(i) = real(eVector[eigenNumber](i));
  }

  // Return
  return sol;
}

vector<double>& Interpolator::getNodalScalarValue(void) const{
  if(!scalar)
    throw Exception("Interpolator: try to get Scalar value in a Vectorial Interpolation");

  else
    return *nodalScalarValue;
}

vector<fullVector<double> >& Interpolator::getNodalVectorValue(void) const{
  if(scalar)
    throw Exception("Interpolator: try to get Vectorial value in a Scalar Interpolation");

  else
    return *nodalVectorValue;
}
