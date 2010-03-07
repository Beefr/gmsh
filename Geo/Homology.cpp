// Gmsh - Copyright (C) 1997-2010 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributed by Matti Pellikka <matti.pellikka@tut.fi>.
 

#include "Homology.h"

#if defined(HAVE_KBIPACK)

Homology::Homology(GModel* model, std::vector<int> physicalDomain, 
		   std::vector<int> physicalSubdomain)
{ 
  _model = model; 
  _domain = physicalDomain;
  _subdomain = physicalSubdomain;
  _fileName = "";

  // default to the whole model
  if(_domain.empty()){
    int dim = _model->getDim();
    std::vector<GEntity*> entities;
    _model->getEntities(entities);
    for(std::vector<GEntity*>::iterator it = entities.begin();
        it != entities.end(); it++){
      if((*it)->dim() == dim) _domainEntities.push_back(*it);
    }
  }

  std::map<int, std::vector<GEntity*> > groups[4];
  model->getPhysicalGroups(groups);
  std::map<int, std::vector<GEntity*> >::iterator it;
  
  for(unsigned int i = 0; i < _domain.size(); i++){
    for(int j = 0; j < 4; j++){
      it = groups[j].find(_domain.at(i));
      if(it != groups[j].end()){
	std::vector<GEntity*> physicalGroup = (*it).second;
	for(unsigned int k = 0; k < physicalGroup.size(); k++){
	  _domainEntities.push_back(physicalGroup.at(k));
	}
      }
    }
  }
  for(unsigned int i = 0; i < _subdomain.size(); i++){           
    for(int j = 0; j < 4; j++){
      it = groups[j].find(_subdomain.at(i));
      if(it != groups[j].end()){
	std::vector<GEntity*> physicalGroup = (*it).second;
	for(unsigned int k = 0; k < physicalGroup.size(); k++){
	  _subdomainEntities.push_back(physicalGroup.at(k));
	}	  
      }
    }
  }
  
}

CellComplex* Homology::createCellComplex(std::vector<GEntity*>& domainEntities,
			    std::vector<GEntity*>& subdomainEntities){
  Msg::Info("Creating a Cell Complex...");
  Msg::StatusBar(1, false, "Cell Complex...");
  Msg::StatusBar(2, false, "");
  double t1 = Cpu();

  
  if(domainEntities.empty()) Msg::Error("Domain is empty.");
  if(subdomainEntities.empty()) Msg::Info("Subdomain is empty.");
  
  std::vector<MElement*> domainElements;
  std::vector<MElement*> subdomainElements;
  for(unsigned int j=0; j < domainEntities.size(); j++) {
    for(unsigned int i=0; i < domainEntities.at(j)->getNumMeshElements(); i++){
      MElement* element = domainEntities.at(j)->getMeshElement(i);
      domainElements.push_back(element);
    }
  }
  for(unsigned int j=0; j < subdomainEntities.size(); j++) {
    for(unsigned int i=0; i < subdomainEntities.at(j)->getNumMeshElements();
	i++){
      MElement* element = subdomainEntities.at(j)->getMeshElement(i);
      subdomainElements.push_back(element);
    }
  }

  CellComplex* cellComplex =  new CellComplex(domainElements, 
					      subdomainElements);

  if(cellComplex->getSize(0) == 0){ 
    Msg::Error("Cell Complex is empty!");
    Msg::Error("Check the domain & the mesh.");
  }
  double t2 = Cpu();
  Msg::Info("Cell Complex complete (%g s).", t2 - t1);
  Msg::Info("%d volumes, %d faces, %d edges and %d vertices.",
            cellComplex->getSize(3), cellComplex->getSize(2), 
	    cellComplex->getSize(1), cellComplex->getSize(0));
  Msg::StatusBar(2, false, "%d V, %d F, %d E, %d V.",
		 cellComplex->getSize(3), cellComplex->getSize(2), 
		 cellComplex->getSize(1), cellComplex->getSize(0));
  return cellComplex;
}

Homology::~Homology()
{ 
  
}

void Homology::findGenerators()
{
  CellComplex* cellComplex = createCellComplex(_domainEntities, 
					       _subdomainEntities);
  std::string domainString = getDomainString(_domain, _subdomain);

  Msg::Info("Reducing the Cell Complex...");
  Msg::StatusBar(1, false, "Reducing...");
  double t1 = Cpu();

  printf("Cell Complex: \n %d volumes, %d faces, %d edges and %d vertices. \n",
	 cellComplex->getSize(3), cellComplex->getSize(2),
	 cellComplex->getSize(1), cellComplex->getSize(0));

  int omitted = cellComplex->reduceComplex();

  printf(" %d volumes, %d faces, %d edges and %d vertices. \n",
         cellComplex->getSize(3), cellComplex->getSize(2),
         cellComplex->getSize(1), cellComplex->getSize(0));

  cellComplex->combine(3);
  cellComplex->reduction(2);
  cellComplex->combine(2);
  cellComplex->reduction(1);
  cellComplex->combine(1);

  printf(" %d volumes, %d faces, %d edges and %d vertices. \n",
	 cellComplex->getSize(3), cellComplex->getSize(2),
	 cellComplex->getSize(1), cellComplex->getSize(0));
  
  cellComplex->checkCoherence();
  
  double t2 = Cpu();
  Msg::Info("Cell Complex reduction complete (%g s).", t2 - t1);
  Msg::Info("%d volumes, %d faces, %d edges and %d vertices.",
            cellComplex->getSize(3), cellComplex->getSize(2), 
	    cellComplex->getSize(1), cellComplex->getSize(0));
  Msg::StatusBar(2, false, "%d V, %d F, %d E, %d N.",
		 cellComplex->getSize(3), cellComplex->getSize(2), 
		 cellComplex->getSize(1), cellComplex->getSize(0));
  
  Msg::Info("Computing homology spaces...");
  Msg::StatusBar(1, false, "Computing...");
  t1 = Cpu();
  ChainComplex* chains = new ChainComplex(cellComplex);
  chains->computeHomology();
  t2 = Cpu();
  Msg::Info("Homology Computation complete (%g s).", t2 - t1);
  
  int HRank[4];
  for(int j = 0; j < 4; j++){
    HRank[j] = 0;
    std::string dimension = "";
    convert(j, dimension);
    for(int i = 1; i <= chains->getBasisSize(j); i++){
      
      std::string generator = "";
      convert(i, generator);
      
      std::string name = "H" + dimension + domainString + generator;
      std::set<Cell*, Less_Cell> cells;
      cellComplex->getCells(cells, j);
      Chain* chain = new Chain(cells,
                               chains->getCoeffVector(j,i), cellComplex,
                               _model, name, chains->getTorsion(j,i));
      t1 = Cpu();
      int start = chain->getSize();
      chain->smoothenChain();
      t2 = Cpu();
      Msg::Info("Smoothened H%d %d from %d cells to %d cells (%g s).", 
		j, i, start, chain->getSize(), t2 - t1);
      if(chain->getSize() != 0) {
        HRank[j] = HRank[j] + 1;
        if(chain->getTorsion() != 1){
	  Msg::Warning("H%d %d has torsion coefficient %d!", 
		       j, i, chain->getTorsion());
	}
      }
      _generators.push_back(chain->createPGroup());
	delete chain;
    }
    if(j == cellComplex->getDim() && cellComplex->getNumOmitted() > 0){
      for(int i = 0; i < cellComplex->getNumOmitted(); i++){
        std::string generator;
        convert(i+1, generator);
        std::string name = "H" + dimension + domainString + generator;
        std::vector<int> coeffs (cellComplex->getOmitted(i).size(),1);
        Chain* chain = new Chain(cellComplex->getOmitted(i), coeffs, 
				 cellComplex, _model, name, 1);
        if(chain->getSize() != 0) HRank[j] = HRank[j] + 1;
	_generators.push_back(chain->createPGroup());
	delete chain;
      }
    }
  }
  
  if(_fileName != "") writeGeneratorsMSH();
  
  delete cellComplex;
  delete chains;
  
  Msg::Info("Ranks of homology spaces for primal cell complex:");
  Msg::Info("H0 = %d", HRank[0]);
  Msg::Info("H1 = %d", HRank[1]);
  Msg::Info("H2 = %d", HRank[2]);
  Msg::Info("H3 = %d", HRank[3]);
  if(omitted != 0) Msg::Info("The computation of generators in the highest dimension was omitted.");
  
  Msg::StatusBar(1, false, "Homology");
  Msg::StatusBar(2, false, "H0: %d, H1: %d, H2: %d, H3: %d.", 
		 HRank[0], HRank[1], HRank[2], HRank[3]);
}

void Homology::findDualGenerators()
{ 
  CellComplex* cellComplex = createCellComplex(_domainEntities, 
					       _subdomainEntities);

  Msg::Info("Reducing Cell Complex...");
  Msg::StatusBar(1, false, "Reducing...");
  double t1 = Cpu();
  printf("Cell Complex: \n %d volumes, %d faces, %d edges and %d vertices. \n",
	 cellComplex->getSize(3), cellComplex->getSize(2),
	 cellComplex->getSize(1), cellComplex->getSize(0));
  
  int omitted = cellComplex->coreduceComplex();
  
  printf(" %d volumes, %d faces, %d edges and %d vertices. \n",
         cellComplex->getSize(3), cellComplex->getSize(2),
         cellComplex->getSize(1), cellComplex->getSize(0));
  
  cellComplex->cocombine(0);
  cellComplex->coreduction(1);
  cellComplex->cocombine(1);
  cellComplex->coreduction(2);
  cellComplex->cocombine(2);
  cellComplex->coreduction(3);
  cellComplex->checkCoherence();

 printf(" %d volumes, %d faces, %d edges and %d vertices. \n",
	 cellComplex->getSize(3), cellComplex->getSize(2),
	 cellComplex->getSize(1), cellComplex->getSize(0));


  double t2 = Cpu();
  Msg::Info("Cell Complex reduction complete (%g s).", t2 - t1);
  Msg::Info("%d volumes, %d faces, %d edges and %d vertices.",
            cellComplex->getSize(3), cellComplex->getSize(2), 
	    cellComplex->getSize(1), cellComplex->getSize(0));
  Msg::StatusBar(2, false, "%d V, %d F, %d E, %d N.",
		 cellComplex->getSize(3), cellComplex->getSize(2), 
		 cellComplex->getSize(1), cellComplex->getSize(0));
   
  Msg::Info("Computing homology spaces...");
  Msg::StatusBar(1, false, "Computing...");
  t1 = Cpu();
  ChainComplex* chains = new ChainComplex(cellComplex);
  chains->transposeHMatrices();
  chains->computeHomology(true);
  t2 = Cpu();
  Msg::Info("Homology Computation complete (%g s).", t2- t1);
  
  int dim = cellComplex->getDim();
   
  int HRank[4];
  for(int i = 0; i < 4; i++) HRank[i] = 0;
  for(int j = 3; j > -1; j--){
    std::string dimension = "";
    convert(dim-j, dimension);

    for(int i = 1; i <= chains->getBasisSize(j); i++){
      
      std::string generator = "";
      convert(i, generator);
      
      std::string name = "H" + dimension + "*" + 
	getDomainString(_domain, _subdomain) + generator;
      std::set<Cell*, Less_Cell> cells;
      cellComplex->getCells(cells, j);
      Chain* chain = new Chain(cells, 
			       chains->getCoeffVector(j,i), cellComplex, 
			       _model, name, chains->getTorsion(j,i));
      _generators.push_back(chain->createPGroup());
      delete chain;
      if(chain->getSize() != 0){
        HRank[dim-j] = HRank[dim-j] + 1;
        if(chain->getTorsion() != 1){ 
	  Msg::Warning("H%d* %d has torsion coefficient %d!", 
		       dim-j, i, chain->getTorsion());
	}
      }     
    }
    
    
    if(j == 0 && cellComplex->getNumOmitted() > 0){
      for(int i = 0; i < cellComplex->getNumOmitted(); i++){
        std::string generator;
        convert(i+1, generator);
        std::string name 
	  = "H" + dimension + "*" + 
	  getDomainString(_domain, _subdomain) + generator;
        std::vector<int> coeffs (cellComplex->getOmitted(i).size(),1);
        Chain* chain = new Chain(cellComplex->getOmitted(i), coeffs, 
				 cellComplex, _model, name, 1);
        if(chain->getSize() != 0) HRank[dim-j] = HRank[dim-j] + 1;
	_generators.push_back(chain->createPGroup());
	delete chain;
      }
    }
  }

  if(_fileName != "") writeGeneratorsMSH();

  delete cellComplex;
  delete chains;
  
  Msg::Info("Ranks of homology spaces for the dual cell complex:");
  Msg::Info("H0* = %d", HRank[0]);
  Msg::Info("H1* = %d", HRank[1]);
  Msg::Info("H2* = %d", HRank[2]);
  Msg::Info("H3* = %d", HRank[3]);
  if(omitted != 0) Msg::Info("The computation of %d highest dimension dual generators was omitted.", omitted);
  
  Msg::StatusBar(1, false, "Homology");
  Msg::StatusBar(2, false, "H0*: %d, H1*: %d, H2*: %d, H3*: %d.", 
		 HRank[0], HRank[1], HRank[2], HRank[3]);
}

void Homology::findHomSequence(){
  CellComplex* cellComplex = createCellComplex(_domainEntities, 
					       _subdomainEntities);
  Msg::Info("Reducing the Cell Complex...");
  Msg::StatusBar(1, false, "Reducing...");
  double t1 = Cpu();

  printf("Cell Complex: \n %d volumes, %d faces, %d edges and %d vertices. \n",
	 cellComplex->getSize(3), cellComplex->getSize(2),
	 cellComplex->getSize(1), cellComplex->getSize(0));

  cellComplex->reduceComplex(false);

  printf(" %d volumes, %d faces, %d edges and %d vertices. \n",
         cellComplex->getSize(3), cellComplex->getSize(2),
         cellComplex->getSize(1), cellComplex->getSize(0));

  cellComplex->combine(3);
  cellComplex->reduction(2);
  cellComplex->combine(2);
  cellComplex->reduction(1);
  cellComplex->combine(1);

  printf(" %d volumes, %d faces, %d edges and %d vertices. \n",
	 cellComplex->getSize(3), cellComplex->getSize(2),
	 cellComplex->getSize(1), cellComplex->getSize(0));
  
  cellComplex->checkCoherence();
  
  double t2 = Cpu();
  Msg::Info("Cell Complex reduction complete (%g s).", t2 - t1);
  Msg::Info("%d volumes, %d faces, %d edges and %d vertices.",
            cellComplex->getSize(3), cellComplex->getSize(2), 
	    cellComplex->getSize(1), cellComplex->getSize(0));
  Msg::StatusBar(2, false, "%d V, %d F, %d E, %d N.",
		 cellComplex->getSize(3), cellComplex->getSize(2), 
		 cellComplex->getSize(1), cellComplex->getSize(0));
  
  Msg::Info("Computing homology spaces...");
  Msg::StatusBar(1, false, "Computing...");
  t1 = Cpu();
  
  for(int task = 0; task < 3; task++){
    ChainComplex* chains = new ChainComplex(cellComplex, task);
    std::string domainString = "";
    std::vector<int> empty;
    if(task == 0) domainString = getDomainString(_domain, _subdomain);
    else if(task == 1) domainString = getDomainString(_domain, empty);
    else if(task == 2) domainString = getDomainString(_subdomain, empty);
    chains->computeHomology();
    t2 = Cpu();
    Msg::Info("Homology Computation complete (%g s).", t2 - t1);
    
    int HRank[4];
    for(int j = 0; j < 4; j++){
      HRank[j] = 0;
      std::string dimension = "";
      convert(j, dimension);
      for(int i = 1; i <= chains->getBasisSize(j); i++){
	
	std::string generator = "";
	convert(i, generator);
      
	std::string name = "H" + dimension + domainString + generator;
	std::set<Cell*, Less_Cell> cells;
	cellComplex->getCells(cells, j, task);
	Chain* chain = new Chain(cells, 
				 chains->getCoeffVector(j,i), 
				 cellComplex, _model, name, 
				 chains->getTorsion(j,i));
	t1 = Cpu();
	int start = chain->getSize();
	//chain->smoothenChain();
	t2 = Cpu();
	//Msg::Info("Smoothened H%d %d from %d cells to %d cells (%g s).", 
	//	  j, i, start, chain->getSize(), t2 - t1);
	if(chain->getSize() != 0) {
	  HRank[j] = HRank[j] + 1;
	  if(chain->getTorsion() != 1){
	  Msg::Warning("H%d %d has torsion coefficient %d!", 
		       j, i, chain->getTorsion());
	  }
	}
	_generators.push_back(chain->createPGroup());
	delete chain;
      }
      
    }
    if(task == 0){
      Msg::Info("Ranks of relative homology spaces:");
    }
    if(task == 1){
      Msg::Info("Ranks of absolute homology spaces:");
    }
    if(task == 2){
      Msg::Info("Ranks of absolute homology spaces of relative subdomain:");
    }
    Msg::Info("H0 = %d", HRank[0]);
    Msg::Info("H1 = %d", HRank[1]);
    Msg::Info("H2 = %d", HRank[2]);
    Msg::Info("H3 = %d", HRank[3]);

    Msg::StatusBar(1, false, "Homology");
    Msg::StatusBar(2, false, "H0: %d, H1: %d, H2: %d, H3: %d.",
		   HRank[0], HRank[1], HRank[2], HRank[3]);
    delete chains;
  }

  if(_fileName != "") writeGeneratorsMSH();
  delete cellComplex;  
}

std::string Homology::getDomainString(const std::vector<int>& domain,
				      const std::vector<int>& subdomain) 
{
  std::string domainString = "({";
  if(domain.empty()) domainString += "0";
  else{
    for(unsigned int i = 0; i < domain.size(); i++){
      std::string temp = "";
      convert(domain.at(i),temp);
      domainString += temp;
      if (domain.size()-1 > i){ 
	domainString += ", ";
      }
    }
  }
  domainString += "}";
  
  if(!subdomain.empty()){
    domainString += ", {";    
    for(unsigned int i = 0; i < subdomain.size(); i++){
      std::string temp = "";
      convert(subdomain.at(i),temp);
      domainString += temp;
      if (subdomain.size()-1 > i){
        domainString += ", ";
      }
    } 
    domainString += "}";
  }
  domainString += ") ";
  return domainString;
}

bool Homology::writeGeneratorsMSH(bool binary)
{
  if(_fileName.empty()) return false;
  if(!_model->writeMSH(_fileName, 2.0, binary)) return false;
  Msg::Info("Wrote homology computation results to %s.", _fileName.c_str());
  return true;
}
Chain::Chain(std::set<Cell*, Less_Cell> cells, std::vector<int> coeffs, 
	     CellComplex* cellComplex, GModel* model,
	     std::string name, int torsion)
{  
  int i = 0;
  for(std::set<Cell*, Less_Cell>::iterator cit = cells.begin();
      cit != cells.end(); cit++){
    Cell* cell = *cit;
    _dim = cell->getDim();
    if((int)coeffs.size() > i){
      if(coeffs.at(i) != 0){
        std::list< std::pair<int, Cell*> > subCells;
	cell->getCells(subCells);
        for(std::list< std::pair<int, Cell*> >::iterator it = 
	      subCells.begin(); it != subCells.end(); it++){
          Cell* subCell = (*it).second;
          int coeff = (*it).first;
          _cells.insert( std::make_pair(subCell, coeffs.at(i)*coeff));
        }
      }
      i++;
    }
    
  }
  _name = name;
  _cellComplex = cellComplex;
  _torsion = torsion;
  _model = model;
  
}

void Homology::storeCells(CellComplex* cellComplex, int dim)
{
  std::vector<MElement*> elements;

  for(CellComplex::citer cit = cellComplex->firstCell(dim); 
      cit != cellComplex->lastCell(dim); cit++){
    Cell* cell = *cit;
    
    std::list< std::pair<int, Cell*> > cells;
    cell->getCells(cells);
    for(std::list< std::pair<int, Cell*> >::iterator it = cells.begin();
	it != cells.end(); it++){
      Cell* subCell = it->second;
      
      MElement* e = subCell->getImageMElement();
      subCell->setDeleteImage(false);
      elements.push_back(e);
    }
  }

  int max[4];
  for(int i = 0; i < 4; i++) max[i] = _model->getMaxElementaryNumber(i);
  int entityNum = *std::max_element(max,max+4) + 1;
  for(int i = 0; i < 4; i++) max[i] = _model->getMaxPhysicalNumber(i);
  int physicalNum = *std::max_element(max,max+4) + 1;

  std::map<int, std::vector<MElement*> > entityMap;
  entityMap[entityNum] = elements;
  std::map<int, std::map<int, std::string> > physicalMap;
  std::map<int, std::string> physicalInfo;
  physicalInfo[physicalNum]="Cell Complex";
  physicalMap[entityNum] = physicalInfo;

  _model->storeChain(dim, entityMap, physicalMap);
  _model->setPhysicalName("Cell Complex", dim, physicalNum);
}

Chain::Chain(std::map<Cell*, int, Less_Cell>& chain, 
	     CellComplex* cellComplex, GModel* model,
	     std::string name, int torsion)
{  
  _cells = chain;
  if(!_cells.empty()) _dim = firstCell()->first->getDim();
  else _dim = 0;
  _name = name;
  _cellComplex = cellComplex;
  _torsion = torsion;
  _model = model; 
  eraseNullCells();
}

bool Chain::deform(std::map<Cell*, int, Less_Cell>& cellsInChain, 
		   std::map<Cell*, int, Less_Cell>& cellsNotInChain)
{
  std::vector<int> cc;
  std::vector<int> bc;
  
  for(citer cit = cellsInChain.begin(); cit != cellsInChain.end(); cit++){
    Cell* c = (*cit).first;
    c->setImmune(false);
    if(!c->inSubdomain()) {
      cc.push_back(getCoeff(c));
      bc.push_back((*cit).second);
    }
  }
  
  if(cc.empty() || (getDim() == 2 && cc.size() < 2) ) return false;
  int inout = cc[0]*bc[0];
  for(unsigned int i = 0; i < cc.size(); i++){
    if(cc[i]*bc[i] != inout) return false;  
  }
  
  for(citer cit = cellsInChain.begin(); cit != cellsInChain.end(); cit++){
    removeCell((*cit).first);
  }
  
  int n = 1;
  for(citer cit = cellsNotInChain.begin(); cit != cellsNotInChain.end();
      cit++){
    Cell* c = (*cit).first;
    if(n == 2) c->setImmune(true);
    else c->setImmune(false);
    int coeff = -1*inout*(*cit).second;
    addCell(c, coeff);
    n++;
  }
  
  return true;
}

bool Chain::deformChain(std::pair<Cell*, int> cell, bool bend)
{  
  Cell* c1 = cell.first;
  for(citer cit = c1->firstCoboundary(true); cit != c1->lastCoboundary(true);
      cit++){
    
    std::map<Cell*, int, Less_Cell> cellsInChain;
    std::map<Cell*, int, Less_Cell> cellsNotInChain;
    Cell* c1CbdCell = (*cit).first;

    for(citer cit2 = c1CbdCell->firstBoundary(true);
	cit2 != c1CbdCell->lastBoundary(true); cit2++){
      Cell* c1CbdBdCell = (*cit2).first;
      int coeff = (*cit2).second;
      if( (hasCell(c1CbdBdCell) && getCoeff(c1CbdBdCell) != 0) 
	  || c1CbdBdCell->inSubdomain()){
	cellsInChain.insert(std::make_pair(c1CbdBdCell, coeff));
      }
      else cellsNotInChain.insert(std::make_pair(c1CbdBdCell, coeff));
    }
    
    bool next = false;
    
    for(citer cit2 = cellsInChain.begin(); cit2 != cellsInChain.end(); cit2++){
      Cell* c = (*cit2).first;
      int coeff = getCoeff(c);
      if(c->getImmune()) next = true;
      if(c->inSubdomain()) bend = false;
      if(coeff > 1 || coeff < -1) next = true; 
    }
    
    for(citer cit2 = cellsNotInChain.begin(); cit2 != cellsNotInChain.end();
	cit2++){
      Cell* c = (*cit2).first;
      if(c->inSubdomain()) next = true;
    }    
    if(next) continue;
    
    if( (getDim() == 1 && cellsInChain.size() == 2 
	 && cellsNotInChain.size() == 1) || 
	(getDim() == 2 && cellsInChain.size() == 3 
	 && cellsNotInChain.size() == 1)){
      //printf("straighten \n");
      return deform(cellsInChain, cellsNotInChain);
    }
    else if ( (getDim() == 1 && cellsInChain.size() == 1 
	       && cellsNotInChain.size() == 2 && bend) ||
              (getDim() == 2 && cellsInChain.size() == 2 
	       && cellsNotInChain.size() == 2 && bend)){
      //printf("bend \n");
      return deform(cellsInChain, cellsNotInChain);
    }
    else if ((getDim() == 1 && cellsInChain.size() == 3 
	      && cellsNotInChain.size() == 0) ||
             (getDim() == 2 && cellsInChain.size() == 4 
	      && cellsNotInChain.size() == 0)){
      //printf("remove boundary \n");
      return deform(cellsInChain, cellsNotInChain);
    }
  }
  
  return false;
}

void Chain::smoothenChain()
{
  if(!_cellComplex->simplicial()) return;
  
  int start = getSize();
  double t1 = Cpu();
  
  int useless = 0;
  for(int i = 0; i < 20; i++){
    int size = getSize();
    for(citer cit = _cells.begin(); cit != _cells.end(); cit++){
      //if(!deformChain(*cit, false) && getDim() == 2) deformChain(*cit, true);
      if(getDim() == 2) deformChain(*cit, true);
      deformChain(*cit, false);      
    }
    deImmuneCells();
    eraseNullCells();
    if (size >= getSize()) useless++;
    else useless = 0;
    if (useless > 5) break;
  }
  
  deImmuneCells();
  for(citer cit = _cells.begin(); cit != _cells.end(); cit++){
    deformChain(*cit, false);
  }
  
  eraseNullCells();
  double t2 = Cpu();
  Msg::Debug("Smoothened a %d-chain from %d cells to %d cells (%g s).\n",
	     getDim(), start, getSize(), t2-t1);
  return;
}


int Chain::writeChainMSH(const std::string &name)
{  
  if(getSize() == 0) return 1;
  
  FILE *fp = fopen(name.c_str(), "a");
  if(!fp){
    Msg::Error("Unable to open file '%s'", name.c_str());
    Msg::Debug("Unable to open file.");
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
  
  for(citer cit = _cells.begin(); cit != _cells.end(); cit++){
    Cell* cell = (*cit).first;
    int coeff = (*cit).second;
    fprintf(fp, "%d %d \n", cell->getImageMElement()->getNum(), coeff );
  }
  
  fprintf(fp, "$EndElementData\n");
  fclose(fp);
  
  return 1;
}

int Chain::createPGroup()
{  
  std::vector<MElement*> elements;
  std::map<int, std::vector<double> > data;
  MElementFactory factory;

  for(citer cit = _cells.begin(); cit != _cells.end(); cit++){
    Cell* cell = (*cit).first;
    int coeff = (*cit).second;

    MElement* e = cell->getImageMElement();
    std::vector<MVertex*> v;
    for(int i = 0; i < e->getNumVertices(); i++){
      v.push_back(e->getVertex(i));
    }
    MElement* ne = factory.create(e->getTypeForMSH(), v, 0, e->getPartition());
    
    if(cell->getDim() > 0 && coeff < 0) ne->revert(); // flip orientation
    for(int i = 0; i < abs(coeff); i++) elements.push_back(ne);    

    std::vector<double> coeffs (1,abs(coeff));
    data[ne->getNum()] = coeffs;
  }
  
  int max[4];
  for(int i = 0; i < 4; i++) max[i] = _model->getMaxElementaryNumber(i);
  int entityNum = *std::max_element(max,max+4) + 1;
  for(int i = 0; i < 4; i++) max[i] = _model->getMaxPhysicalNumber(i);
  int physicalNum = *std::max_element(max,max+4) + 1;
  setNum(physicalNum);
  
  std::map<int, std::vector<MElement*> > entityMap;
  entityMap[entityNum] = elements;
  std::map<int, std::map<int, std::string> > physicalMap;
  std::map<int, std::string> physicalInfo;
  physicalInfo[physicalNum]=getName();
  physicalMap[entityNum] = physicalInfo;
  
  // hide mesh
  opt_mesh_points(0, GMSH_SET, 0);
  opt_mesh_lines(0, GMSH_SET, 0);
  opt_mesh_triangles(0, GMSH_SET, 0);
  opt_mesh_quadrangles(0, GMSH_SET, 0);
  opt_mesh_tetrahedra(0, GMSH_SET, 0);
  opt_mesh_hexahedra(0, GMSH_SET, 0);
  opt_mesh_prisms(0, GMSH_SET, 0);
  opt_mesh_pyramids(0, GMSH_SET, 0);

  // show post-processing normals, tangents and element boundaries
  //opt_view_normals(0, GMSH_SET, 20);
  //opt_view_tangents(0, GMSH_SET, 20);
  opt_view_show_element(0, GMSH_SET, 1);
  
  if(!data.empty()){
    _model->storeChain(getDim(), entityMap, physicalMap);
    _model->setPhysicalName(getName(), getDim(), physicalNum);
    
    // create PView for visualization
    PView* chain = new PView(getName(), "ElementData", getGModel(), 
			     data, 0, 1);
  }
   
  return physicalNum;
}


void Chain::removeCell(Cell* cell) 
{
  citer it = _cells.find(cell);
  if(it != _cells.end()){
    (*it).second = 0;
  }
  return;
}

void Chain::addCell(Cell* cell, int coeff) 
{
  std::pair<citer,bool> insert = _cells.insert( std::make_pair( cell, coeff));
  if(!insert.second && (*insert.first).second == 0){
    (*insert.first).second = coeff; 
  }
  else if (!insert.second && (*insert.first).second != 0){
    Msg::Debug("Error: invalid chain smoothening add! \n");
  }
  return;
}

bool Chain::hasCell(Cell* c)
{
  citer it = _cells.find(c);
  if(it != _cells.end() && (*it).second != 0) return true;
  return false;
}
   
Cell* Chain::findCell(Cell* c)
{
  citer it = _cells.find(c);
  if(it != _cells.end() && (*it).second != 0) return (*it).first;
  return NULL;
}

int Chain::getCoeff(Cell* c)
{
  citer it = _cells.find(c);
  if(it != _cells.end()) return (*it).second;
  return 0;
}

void Chain::eraseNullCells()
{
  std::vector<Cell*> toRemove;
  for(int i = 0; i < 4; i++){
    for(citer cit = _cells.begin(); cit != _cells.end(); ++cit){
      if((*cit).second == 0) toRemove.push_back((*cit).first);
    }
  }
  for(unsigned int i = 0; i < toRemove.size(); i++) _cells.erase(toRemove[i]);
  return;
}

void Chain::deImmuneCells()
{
  for(citer cit = _cells.begin(); cit != _cells.end(); cit++){
    Cell* cell = (*cit).first;
    cell->setImmune(false);
  }
}

#endif

