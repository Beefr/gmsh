// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributed by Matti Pellikka.
 

#include "Homology.h"
#include "ChainComplex.h"

#if defined(HAVE_KBIPACK)
Homology::Homology(GModel* model, std::vector<int> physicalDomain, std::vector<int> physicalSubdomain){
  
  _model = model;
  
  Msg::Info("Creating a Cell Complex...");
  
  
  std::map<int, std::vector<GEntity*> > groups[4];
  model->getPhysicalGroups(groups);
  
  
  std::map<int, std::vector<GEntity*> >::iterator it;
  std::vector<GEntity*> domainEntities;
  std::vector<GEntity*> subdomainEntities;
    
  for(int i = 0; i < physicalDomain.size(); i++){
    for(int j = 0; j < 4; j++){
      it = groups[j].find(physicalDomain.at(i));
      if(it != groups[j].end()){
        std::vector<GEntity*> physicalGroup = (*it).second;
        for(int k = 0; k < physicalGroup.size(); k++){
          domainEntities.push_back(physicalGroup.at(k));
        }
      }
    }
  }
  for(int i = 0; i < physicalSubdomain.size(); i++){           
    for(int j = 0; j < 4; j++){
      it = groups[j].find(physicalSubdomain.at(i));
      if(it != groups[j].end()){
        std::vector<GEntity*> physicalGroup = (*it).second;
        for(int k = 0; k < physicalGroup.size(); k++){
          subdomainEntities.push_back(physicalGroup.at(k));
        }
        
      }
    }
  }
  
  if(domainEntities.empty()) Msg::Warning("Domain is empty.");
  if(subdomainEntities.empty()) Msg::Info("Subdomain is empty.");
  
  
  _cellComplex =  new CellComplex(domainEntities, subdomainEntities);
  
  if(_cellComplex->getSize(0) == 0){ 
    Msg::Error("Cell Complex is empty!");
    Msg::Error("Check the domain & the mesh.");
    return;
  }
  
  Msg::Info("Cell Complex complete.");
  Msg::Info("%d volumes, %d faces, %d edges and %d vertices.",
            _cellComplex->getSize(3), _cellComplex->getSize(2), _cellComplex->getSize(1), _cellComplex->getSize(0));
  
}


void Homology::findGenerators(std::string fileName){
  
  Msg::Info("Reducing Cell Complex...");
  int omitted = _cellComplex->reduceComplex(true);
  _cellComplex->combine(3);
  _cellComplex->combine(2);
  _cellComplex->combine(1);
  Msg::Info("Cell Complex reduction complete.");
  Msg::Info("%d volumes, %d faces, %d edges and %d vertices.",
            _cellComplex->getSize(3), _cellComplex->getSize(2), _cellComplex->getSize(1), _cellComplex->getSize(0));

  _cellComplex->writeComplexMSH(fileName);
  
  Msg::Info("Computing homology groups...");
  ChainComplex* chains = new ChainComplex(_cellComplex);
  chains->computeHomology();
  Msg::Info("Homology Computation complete.");
  
  for(int j = 0; j < 4; j++){
    for(int i = 1; i <= chains->getBasisSize(j); i++){
      
      std::string generator;
      std::string dimension;
      convert(i, generator);
      convert(j, dimension);
      
      std::string name = dimension + "D Generator " + generator;
      Chain* chain = new Chain(_cellComplex->getCells(j), chains->getCoeffVector(j,i), _cellComplex, name);
      chain->writeChainMSH(fileName);
      delete chain;
    }
  }
  
  Msg::Info("Ranks of homology groups for primal cell complex:");
  Msg::Info("H0 = %d", chains->getBasisSize(0));
  Msg::Info("H1 = %d", chains->getBasisSize(1));
  Msg::Info("H2 = %d", chains->getBasisSize(2));
  Msg::Info("H3 = %d", chains->getBasisSize(3));
  if(omitted != 0) Msg::Info("%d 0D generators are not shown completely.", omitted);
  
  
  Msg::Info("Wrote results to %s.", fileName.c_str());
  
  delete chains;
  
  return;
}

void Homology::findThickCuts(std::string fileName){
  
  Msg::Info("Reducing Cell Complex...");
  int omitted = _cellComplex->coreduceComplex(true);
  _cellComplex->cocombine(0);
  _cellComplex->cocombine(1);
  _cellComplex->cocombine(2);
  Msg::Info("Cell Complex reduction complete.");
  Msg::Info("%d volumes, %d faces, %d edges and %d vertices.",
            _cellComplex->getSize(3), _cellComplex->getSize(2), _cellComplex->getSize(1), _cellComplex->getSize(0));
 
  _cellComplex->writeComplexMSH(fileName);
  
  Msg::Info("Computing homology groups...");
  ChainComplex* chains = new ChainComplex(_cellComplex);
  chains->transposeHMatrices();
  chains->computeHomology(true);
  Msg::Info("Homology Computation complete.");
  
  for(int j = 3; j > -1; j--){
    for(int i = 1; i <= chains->getBasisSize(j); i++){
      
      std::string generator;
      std::string dimension;
      convert(i, generator);
      convert(3-j, dimension);
      
      std::string name = dimension + "D Thick cut " + generator;
      Chain* chain = new Chain(_cellComplex->getCells(j), chains->getCoeffVector(j,i), _cellComplex, name);
      chain->writeChainMSH(fileName);
      delete chain;
            
    }
  }
  
  Msg::Info("Ranks of homology groups for dual cell complex:");
  Msg::Info("H0 = %d", chains->getBasisSize(3));
  Msg::Info("H1 = %d", chains->getBasisSize(2));
  Msg::Info("H2 = %d", chains->getBasisSize(1));
  Msg::Info("H3 = %d", chains->getBasisSize(0));
  if(omitted != 0) Msg::Info("%d 3D thick cuts are not shown completely.", omitted);
  
  
  Msg::Info("Wrote results to %s.", fileName.c_str());
  
  delete chains;
  
}
#endif
