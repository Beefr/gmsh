#include <set>
#include "dgAlgorithm.h"
#include "dgGroupOfElements.h"
#include "dgConservationLaw.h"
#include "GEntity.h"
#include "MElement.h"
#include "GModel.h"
#include "MEdge.h"
#include "function.h"
/*
  compute 
    \int \vec{f} \cdot \grad \phi dv   
*/

void dgAlgorithm::residualVolume ( //dofManager &dof, // the DOF manager (maybe useless here)
				   const dgConservationLaw &claw,   // the conservation law
				   const dgGroupOfElements & group, 
				   const fullMatrix<double> &solution,
				   fullMatrix<double> &residual // the residual
           )
{ 
  // ----- 1 ----  get the solution at quadrature points
  // ----- 1.1 --- allocate a matrix of size (nbFields * nbElements, nbQuadraturePoints) 
  int nbFields = claw.nbFields();
  fullMatrix<double> solutionQP (group.getNbIntegrationPoints(),group.getNbElements() * nbFields);
  // ----- 1.2 --- multiply the solution by the collocation matrix
  group.getCollocationMatrix().mult(solution  , solutionQP); 

  // ----- 2 ----  compute all fluxes (diffusive and convective) at integration points
  // ----- 2.1 --- allocate elementary fluxes (compued in XYZ coordinates)
  fullMatrix<double> fConv[3] = {fullMatrix<double>( group.getNbIntegrationPoints(), nbFields ),
				 fullMatrix<double>( group.getNbIntegrationPoints(), nbFields ),
				 fullMatrix<double>( group.getNbIntegrationPoints(), nbFields )};
  fullMatrix<double> fDiff[3] = {fullMatrix<double>( group.getNbIntegrationPoints(), nbFields ),
				 fullMatrix<double>( group.getNbIntegrationPoints(), nbFields ),
				 fullMatrix<double>( group.getNbIntegrationPoints(), nbFields )};
  fullMatrix<double> Source = fullMatrix<double> (group.getNbIntegrationPoints(),group.getNbElements() * nbFields);
  // ----- 2.2 --- allocate parametric fluxes (computed in UVW coordinates) for all elements at all integration points
  fullMatrix<double> Fuvw[3] = {fullMatrix<double> ( group.getNbIntegrationPoints(), group.getNbElements() * nbFields),
				fullMatrix<double> (group.getNbIntegrationPoints(), group.getNbElements() * nbFields),
				fullMatrix<double> (group.getNbIntegrationPoints(), group.getNbElements() * nbFields)};
  dataCacheMap cacheMap;
  dataCacheElement &cacheElement = cacheMap.getElement();
  // provided dataCache
  cacheMap.provideData("UVW").set(group.getIntegrationPointsMatrix());
  dataCacheDouble &solutionQPe = cacheMap.provideData("Solution");
  dataCacheDouble &gradSolutionQPe = cacheMap.provideData("gradSolution");
  // needed dataCache
  dataCacheDouble *sourceTerm = claw.newSourceTerm(cacheMap);
    // fluxes in  XYZ coordinates;
  dataCacheDouble *convectiveFlux = claw.newConvectiveFlux(cacheMap);
  dataCacheDouble *diffusiveFlux = claw.newDiffusiveFlux(cacheMap);

  // compute the gradient of solution if necessary
  fullMatrix<double> *gradientSolutionQP= 0;
  if (gradSolutionQPe.somethingDependOnMe()) {
    gradientSolutionQP = new  fullMatrix<double> (group.getNbIntegrationPoints(), group.getNbElements() * nbFields * 3);
    //group.getGradientOfSolution().mult ( group.getCollocationMatrix() , *gradientSolutionQP); 
  }
  fullMatrix<double> fuvwe;
  fullMatrix<double> source;
  // ----- 2.3 --- iterate on elements
  for (int iElement=0 ; iElement<group.getNbElements() ;++iElement) {
    // ----- 2.3.1 --- build a small object that contains elementary solution, jacobians, gmsh element
    solutionQPe.setAsProxy(solutionQP, iElement*nbFields, nbFields );
    if (gradSolutionQPe.somethingDependOnMe())
      gradSolutionQPe.setAsProxy(*gradientSolutionQP, 3*iElement*nbFields,3*nbFields );
    cacheElement.set(group.getElement(iElement));
    if(convectiveFlux || diffusiveFlux) {
      // ----- 2.3.3 --- compute fluxes in UVW coordinates
      for (int iUVW=0;iUVW<group.getDimUVW();iUVW++) {
        // ----- 2.3.3.1 --- get a proxy on the big local flux matrix
        fuvwe.setAsProxy(Fuvw[iUVW], iElement*nbFields,nbFields);
        // ----- 2.3.3.1 --- compute \vec{f}^{UVW} =\vec{f}^{XYZ} J^{-1} and multiply bt detJ
        for (int iXYZ=0;iXYZ<group.getDimXYZ();iXYZ++) {
          for (int iPt =0; iPt< group.getNbIntegrationPoints(); iPt++) {
            // get the right inv jacobian matrix dU/dX element
            const double invJ = group.getInvJ (iElement, iPt, iUVW, iXYZ);
            // get the detJ at this point
            const double detJ = group.getDetJ (iElement, iPt);
            const double factor = invJ * detJ;
            // compute fluxes in the reference coordinate system at this integration point
            for (int k=0;k<nbFields;k++){
              if(convectiveFlux)
                fuvwe(iPt,k) += ( (*convectiveFlux)(iPt,iXYZ*nbFields+k)) * factor;
              if(diffusiveFlux)
                fuvwe(iPt,k) += ( (*diffusiveFlux)(iPt,iXYZ*nbFields+k)) * factor;
            }
          }
        } 
      }
    }
    if (sourceTerm){
      source.setAsProxy(Source, iElement*claw.nbFields(),claw.nbFields());
      for (int iPt =0; iPt< group.getNbIntegrationPoints(); iPt++) {
        const double detJ = group.getDetJ (iElement, iPt);
        for (int k=0;k<nbFields;k++){
          source(iPt,k) = (*sourceTerm)(iPt,k)*detJ;
        }
      }
    }
    if(gradientSolutionQP)
      delete gradientSolutionQP;
  }
  // ----- 3 ---- do the redistribution at nodes using as many BLAS3 operations as there are local coordinates
  if(convectiveFlux || diffusiveFlux){
    for (int iUVW=0;iUVW<group.getDimUVW();iUVW++){
      residual.gemm(group.getFluxRedistributionMatrix(iUVW),Fuvw[iUVW]);
    }
    if(convectiveFlux)
      delete convectiveFlux;
    if(diffusiveFlux)
      delete diffusiveFlux;
  }
  if(sourceTerm){
    residual.gemm(group.getSourceRedistributionMatrix(),Source);
    delete sourceTerm;
  }
}

void dgAlgorithm::residualInterface ( //dofManager &dof, // the DOF manager (maybe useless here)
				   const dgConservationLaw &claw,   // the conservation law
				   const dgGroupOfFaces &group, 
           const fullMatrix<double> &solution, // solution !! at faces nodes
           fullMatrix<double> &solutionLeft, 
           fullMatrix<double> &solutionRight, 
           fullMatrix<double> &residual // residual !! at faces nodes
           )
{ 
  // A) global operations before entering the loop over the faces
  // A1 ) copy locally some references from the group for the sake of lisibility
  int nbFields = claw.nbFields();
  int nbNodesLeft = group.getGroupLeft().getNbNodes();
  int nbNodesRight = group.getGroupRight().getNbNodes();
  int nbFaces = group.getNbElements();
  //get matrices needed to compute the gradient on both sides
  fullMatrix<double> &DPsiLeftDx = group.getDPsiLeftDxMatrix();
  fullMatrix<double> &DPsiRightDx = group.getDPsiRightDxMatrix();

  // ----- 1 ----  get the solution at quadrature points
  fullMatrix<double> solutionQP (group.getNbIntegrationPoints(),nbFaces * nbFields*2);
  group.getCollocationMatrix().mult(solution, solutionQP); 
  // needed tocompute normal fluxes  at integration points
  fullMatrix<double> NormalFluxQP ( group.getNbIntegrationPoints(), nbFaces*nbFields*2);

  // create one dataCache for each side
  dataCacheMap cacheMapLeft, cacheMapRight;

  // data this algorithm provide to the cache map (we can maybe move each data to a separate function but
  // I think It's easier like this)
  dataCacheDouble &normals = cacheMapLeft.provideData("Normals");
  dataCacheElement &cacheElementLeft = cacheMapLeft.getElement();
  dataCacheElement &cacheElementRight = cacheMapRight.getElement();
  cacheMapLeft.provideData("UVW").set(group.getIntegrationPointsMatrix());
  dataCacheDouble &solutionQPLeft = cacheMapLeft.provideData("Solution");
  dataCacheDouble &solutionQPRight = cacheMapRight.provideData("Solution");
  dataCacheDouble &gradientSolutionLeft = cacheMapLeft.provideData("GradientSolution");
  dataCacheDouble &gradientSolutionRight = cacheMapRight.provideData("GradientSolution");
  //resize gradientSolutionLeft and Right
  gradientSolutionLeft.set(fullMatrix<double>(group.getNbIntegrationPoints()*3,nbFields));
  gradientSolutionRight.set(fullMatrix<double>(group.getNbIntegrationPoints()*3,nbFields));

  // A2 ) ask the law to provide the fluxes (possibly NULL)
  dataCacheDouble *riemannSolver = claw.newRiemannSolver(cacheMapLeft,cacheMapRight);
  dataCacheDouble *diffusiveFluxLeft = claw.newDiffusiveFlux(cacheMapLeft);
  dataCacheDouble *diffusiveFluxRight = claw.newDiffusiveFlux(cacheMapRight);
  fullMatrix<double> dPsiLeftDx,dPsiRightDx,dofsLeft,dofsRight,normalFluxQP;

  for (int iFace=0 ; iFace < nbFaces ; ++iFace) {
    // B1 )  adjust the proxies for this element
    //      NB  : the B1 section will almost completely disapear 
    //      if we write a function (from the function class) for moving proxies across big matrices
    // give the current elements to the dataCacheMap 
    cacheElementLeft.set(group.getElementLeft(iFace));
    cacheElementRight.set(group.getElementRight(iFace));
    // proxies for the solution
    solutionQPLeft.setAsProxy(solutionQP, iFace*2*nbFields, nbFields);
    solutionQPRight.setAsProxy(solutionQP, (iFace*2+1)*nbFields, nbFields);
    normals.setAsProxy(group.getNormals(), iFace*group.getNbIntegrationPoints(),group.getNbIntegrationPoints());
    // proxies needed to compute the gradient of the solution and the IP term
    dPsiLeftDx.setAsProxy(DPsiLeftDx,iFace*nbNodesLeft,nbNodesLeft);
    dPsiRightDx.setAsProxy(DPsiLeftDx,iFace*nbNodesRight,nbNodesRight);
    dofsLeft.setAsProxy(solutionLeft, nbFields*group.getElementLeftId(iFace), nbFields);
    dofsRight.setAsProxy(solutionRight, nbFields*group.getElementRightId(iFace), nbFields);
    // proxies for the flux
    normalFluxQP.setAsProxy(NormalFluxQP, iFace*nbFields*2, nbFields*2);

    // B2 ) compute the gradient of the solution
    if(gradientSolutionLeft.somethingDependOnMe()){
      dPsiLeftDx.mult(dofsLeft, gradientSolutionLeft.set());
      dPsiRightDx.mult(dofsRight, gradientSolutionRight.set());
    }

    // B3 ) compute fluxes

    //JF : here you can test (diffusiveFluxLeft!=NULL) to know if the law is diffusive
    //you can access the values by (*diffusiveFluxLeft)(iQP*3+iXYZ, iField)
    //use gradientSolutionLef(IQP*3+IXYZ, iField) and dPsiLeftDx(iQP*3+iXYZ, iPsi)

    for (int iPt =0; iPt< group.getNbIntegrationPoints(); iPt++) {
      const double detJ = group.getDetJ (iFace, iPt);
      for (int k=0;k<nbFields*2;k++)
        normalFluxQP(iPt,k) = (*riemannSolver)(iPt,k)*detJ;
    }
  }

  // C ) redistribute the flux to the residual (at Faces nodes)
  residual.gemm(group.getRedistributionMatrix(),NormalFluxQP);

  // D ) delete the dataCacheDouble provided by the law
  delete riemannSolver;
  if(diffusiveFluxLeft) {
    delete diffusiveFluxLeft;
    delete diffusiveFluxRight;
  }
}

void dgAlgorithm::multAddInverseMassMatrix ( /*dofManager &dof,*/
          const dgGroupOfElements & group,
          fullMatrix<double> &residu,
          fullMatrix<double> &sol)
{
  fullMatrix<double> residuEl;
  fullMatrix<double> solEl;
  fullMatrix<double> iMassEl;
  int nFields=sol.size2()/group.getNbElements();
  for(int i=0;i<group.getNbElements();i++) {
    residuEl.setAsProxy(residu,i*nFields,nFields);
    solEl.setAsProxy(sol,i*nFields,nFields);
    iMassEl.setAsProxy(group.getInverseMassMatrix(),i*group.getNbNodes(),group.getNbNodes());
    solEl.gemm(iMassEl,residuEl);
  }
}

 void dgAlgorithm::rungeKutta (
			const dgConservationLaw &claw,				// conservation law
			std::vector<dgGroupOfElements*> &eGroups,	// group of elements
			std::vector<dgGroupOfFaces*> &fGroups,		// group of interfacs
			std::vector<dgGroupOfFaces*> &bGroups,		// group of boundaries
			double h,									// time-step
			fullMatrix<double> &residu,					
			fullMatrix<double> &sol,					
			int orderRK)								// order of RK integrator
{
  // U_{n+1}=U_n+h*(SUM_i a_i*K_i)
  // K_i=M^(-1)R(U_n+b_i*K_{i-1})

  double a[4] = {h/6.0,h/3.0,h/3.0,h/6.0};
  double b[4] = {0.,h/2.0,h/2.0,h};

  fullMatrix<double> K(sol);
  // Current updated solution
  fullMatrix<double> Unp(sol);
  fullMatrix<double> residuEl, KEl;
  fullMatrix<double> iMassEl;

  int nbNodes = eGroups[0]->getNbNodes();
  int nbFields = sol.size2()/eGroups[0]->getNbElements(); 

  for(int j=0; j<orderRK;j++){
    if(j!=0){
      K.scale(b[j]);
      K.add(sol);
    }
    this->residual(claw,eGroups,fGroups,bGroups,K,residu);
    K.scale(0.);
    for(int i=0;i<eGroups[0]->getNbElements();i++) {
      residuEl.setAsProxy(residu,i*nbFields,nbFields);
      KEl.setAsProxy(K,i*nbFields,nbFields);
      iMassEl.setAsProxy(eGroups[0]->getInverseMassMatrix(),i*nbNodes,nbNodes);
      iMassEl.mult(residuEl,KEl);
    }
    Unp.add(K,a[j]);
  }
  sol=Unp;
}


void dgAlgorithm::residualBoundary ( //dofManager &dof, // the DOF manager (maybe useless here)
				   const dgConservationLaw &claw,   // the conservation law
				   const dgGroupOfFaces &group, 
           const fullMatrix<double> &solution, // solution !! at faces nodes
           fullMatrix<double> &solutionLeft, 
           fullMatrix<double> &residual // residual !! at faces nodes
           )
{ 
  int nbFields = claw.nbFields();
  const dgBoundaryCondition *boundaryCondition = claw.getBoundaryCondition(group.getBoundaryTag());
  // ----- 1 ----  get the solution at quadrature points
  fullMatrix<double> solutionQP (group.getNbIntegrationPoints(),group.getNbElements() * nbFields);
  group.getCollocationMatrix().mult(solution, solutionQP); 
  // ----- 2 ----  compute normal fluxes  at integration points
  fullMatrix<double> NormalFluxQP ( group.getNbIntegrationPoints(), group.getNbElements()*nbFields);

  dataCacheMap cacheMapLeft;
  // provided dataCache
  cacheMapLeft.provideData("UVW").set(group.getIntegrationPointsMatrix());
  dataCacheDouble &solutionQPLeft = cacheMapLeft.provideData("Solution");
  dataCacheDouble &normals = cacheMapLeft.provideData("Normals");
  dataCacheElement &cacheElementLeft = cacheMapLeft.getElement();
  // required data
  dataCacheDouble *boundaryTerm = boundaryCondition->newBoundaryTerm(cacheMapLeft);
  fullMatrix<double> normalFluxQP;

  for (int iFace=0 ; iFace<group.getNbElements() ;++iFace) {
    normalFluxQP.setAsProxy(NormalFluxQP, iFace*nbFields, nbFields);
    // ----- 2.3.1 --- provide the data to the cacheMap
    solutionQPLeft.setAsProxy(solutionQP, iFace*nbFields, nbFields);
    normals.setAsProxy(group.getNormals(),iFace*group.getNbIntegrationPoints(),group.getNbIntegrationPoints());
    cacheElementLeft.set(group.getElementLeft(iFace));

    // ----- 2.3.2 --- compute fluxes
    for (int iPt =0; iPt< group.getNbIntegrationPoints(); iPt++) {
      const double detJ = group.getDetJ (iFace, iPt);
      for (int k=0;k<nbFields;k++)
        normalFluxQP(iPt,k) = (*boundaryTerm)(iPt,k)*detJ;
    }
  }
  // ----- 3 ---- do the redistribution at face nodes using BLAS3
  residual.gemm(group.getRedistributionMatrix(),NormalFluxQP);
  delete boundaryTerm;
}

void dgAlgorithm::buildGroups(GModel *model, int dim, int order,
    std::vector<dgGroupOfElements*> &eGroups,
    std::vector<dgGroupOfFaces*> &fGroups,
    std::vector<dgGroupOfFaces*> &bGroups) 
{
  std::map<const std::string,std::set<MEdge, Less_Edge> > boundaryEdges;
  std::map<const std::string,std::set<MFace, Less_Face> > boundaryFaces;
  std::vector<GEntity*> entities;
  model->getEntities(entities);
  std::vector<MElement *> allElements;
  for(unsigned int i = 0; i < entities.size(); i++){
    GEntity *entity = entities[i];
    if(entity->dim() == dim-1){
      for(unsigned int j = 0; j < entity->physicals.size(); j++){
        const std::string physicalName = model->getPhysicalName(entity->dim(), entity->physicals[j]);
        for (int k = 0; k < entity->getNumMeshElements(); k++) {
          MElement *element = entity->getMeshElement(k);
          if(dim==2)
            boundaryEdges[physicalName].insert( MEdge(element->getVertex(0), element->getVertex(1)) );
          else
            boundaryFaces[physicalName].insert( MFace(element->getVertex(0), element->getVertex(1),element->getVertex(2)) );
        }
      }
    }else if(entity->dim() == dim){
      for (int iel=0; iel<entity->getNumMeshElements(); iel++)
        allElements.push_back(entity->getMeshElement(iel));
    }
  }
  eGroups.push_back(new dgGroupOfElements(allElements,order));
  fGroups.push_back(new dgGroupOfFaces(*eGroups[0],order));
  if(dim==2){
    std::map<const std::string, std::set<MEdge, Less_Edge> >::iterator mapIt;
    for(mapIt=boundaryEdges.begin(); mapIt!=boundaryEdges.end(); mapIt++) {
      bGroups.push_back(new dgGroupOfFaces(*eGroups[0],mapIt->first,order,mapIt->second));
    }
  }else if(dim=3){
    std::map<const std::string, std::set<MFace, Less_Face> >::iterator mapIt;
    for(mapIt=boundaryFaces.begin(); mapIt!=boundaryFaces.end(); mapIt++) {
      bGroups.push_back(new dgGroupOfFaces(*eGroups[0],mapIt->first,order,mapIt->second));
    }
  }else throw;
}

// works only if there is only 1 group of element
void dgAlgorithm::residual( const dgConservationLaw &claw,
    std::vector<dgGroupOfElements*> &eGroups, //group of elements
    std::vector<dgGroupOfFaces*> &fGroups,  // group of interfacs
    std::vector<dgGroupOfFaces*> &bGroups, // group of boundaries
    fullMatrix<double> &solution, // solution
    fullMatrix<double> &residu) // residual
{
  int nbFields=claw.nbFields();
  dgAlgorithm algo;
  residu.scale(0);
  //volume term
  for(size_t i=0;i<eGroups.size() ; i++) {
    algo.residualVolume(claw,*eGroups[i],solution,residu);
  }
  //interface term
  for(size_t i=0;i<fGroups.size() ; i++) {
    dgGroupOfFaces &faces = *fGroups[i];
    fullMatrix<double> solInterface(faces.getNbNodes(),faces.getNbElements()*2*nbFields);
    fullMatrix<double> residuInterface(faces.getNbNodes(),faces.getNbElements()*2*nbFields);
    faces.mapToInterface(nbFields, solution, solution, solInterface);
    algo.residualInterface(claw,faces,solInterface,solution,solution,residuInterface);
    faces.mapFromInterface(nbFields, residuInterface, residu, residu);
  }
  //boundaries
  for(size_t i=0;i<bGroups.size() ; i++) {
    dgGroupOfFaces &faces = *bGroups[i];
    fullMatrix<double> solInterface(faces.getNbNodes(),faces.getNbElements()*nbFields);
    fullMatrix<double> residuInterface(faces.getNbNodes(),faces.getNbElements()*nbFields);
    faces.mapToInterface(nbFields, solution, solution, solInterface);
    algo.residualBoundary(claw,faces,solInterface,solution,residuInterface);
    faces.mapFromInterface(nbFields, residuInterface, residu, residu);
  }
}
