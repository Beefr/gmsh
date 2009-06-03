// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributed by Matti Pellikka

#include <stdlib.h>
#include "Gmsh.h"
#include "GmshConfig.h"
#include "GModel.h"
#include "Homology.h"
#include "PViewDataGModel.h"
#include "HomologyComputation.h"


StringXNumber HomologyComputationOptions_Number[] = {
  {GMSH_FULLRC, "Physical group for domain", NULL, 0.},
  {GMSH_FULLRC, "Physical group for subdomain", NULL, 0.},
  {GMSH_FULLRC, "Compute generators", NULL, 1.},
  {GMSH_FULLRC, "Compute thick cuts", NULL, 0.},
  {GMSH_FULLRC, "Swap subdomain", NULL, 0.},
};

StringXString HomologyComputationOptions_String[] = {
  {GMSH_FULLRC, "Filename", NULL, "homology.msh"}
};

extern "C"
{
  GMSH_Plugin *GMSH_RegisterHomologyComputationPlugin()
  {
    return new GMSH_HomologyComputationPlugin();
  }
}

void GMSH_HomologyComputationPlugin::getName(char *name) const
{
  strcpy(name, "Homology Computation");
}

void GMSH_HomologyComputationPlugin::getInfos(char *author, char *copyright,
                                   char *help_text) const
{
  strcpy(author, "Matti Pellikka (matti.pellikka@tut.fi)");
  strcpy(copyright, "C. Geuzaine, J.-F. Remacle");
  strcpy(help_text,
         "Plugin(HomologyComputation) computes generators \n"
         "for (relative) homology groups and their thick cuts. \n"
         "\n"
         "Plugin(HomologyComputation) creates new views.\n");
}

int GMSH_HomologyComputationPlugin::getNbOptions() const
{
  return sizeof(HomologyComputationOptions_Number) / sizeof(StringXNumber);
}

StringXNumber *GMSH_HomologyComputationPlugin::getOption(int iopt)
{
  return &HomologyComputationOptions_Number[iopt];
}

int GMSH_HomologyComputationPlugin::getNbOptionsStr() const
{
  return sizeof(HomologyComputationOptions_String) / sizeof(StringXString);
}

StringXString *GMSH_HomologyComputationPlugin::getOptionStr(int iopt)
{
  return &HomologyComputationOptions_String[iopt];
}

void GMSH_HomologyComputationPlugin::catchErrorMessage(char *errorMessage) const
{
  strcpy(errorMessage, "Homology Computation failed...");
}


PView *GMSH_HomologyComputationPlugin::execute(PView *v)
{
  std::string fileName = HomologyComputationOptions_String[0].def;
  if(fileName.empty()) {
    Msg::Error("Filename not given!");
    return 0;
  }
  
  std::vector<int> domain;
  std::vector<int> subdomain;
  
  domain.push_back((int)HomologyComputationOptions_Number[0].def);
  subdomain.push_back((int)HomologyComputationOptions_Number[1].def);

  int gen = (int)HomologyComputationOptions_Number[2].def;
  int cuts = (int)HomologyComputationOptions_Number[3].def;
  int swap = (int)HomologyComputationOptions_Number[4].def;

  GModel *m = GModel::current();
  
  Homology* homology = new Homology(m, domain, subdomain);
  
  if(swap == 1) homology->swapSubdomain();
  if(gen == 1 && cuts != 1) {
    homology->findGenerators(fileName);
    GmshMergeFile(fileName);
  }
  else if(cuts == 1 && gen != 1) {
    homology->findThickCuts(fileName);
    GmshMergeFile(fileName);
  }
  else Msg::Error("Choose either generators or thick cuts to compute.");
  
  delete homology; 
  
  
  return 0;
}
