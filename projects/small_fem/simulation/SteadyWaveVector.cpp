#include <iostream>

#include "Mesh.h"
#include "System.h"
#include "Interpolator.h"
#include "WriterMsh.h"

#include "FormulationSteadyWaveVector.h"
#include "FormulationSteadyWaveVectorSlow.h"

#include "Timer.h"
#include "SmallFem.h"

using namespace std;

double pi = 3.14159265359;

fullVector<double> fSource(fullVector<double>& xyz){
  fullVector<double> res(3);

  res(0) = 0;
  res(1) = 1;
  res(2) = 0;

  return res;
}

fullVector<double> fWall(fullVector<double>& xyz){
  fullVector<double> res(3);

  res(0) = 0;
  res(1) = 0;
  res(2) = 0;

  return res;
}

void compute(int argc, char** argv){
  // 'nopos' string //
  const char* nopos = "nopos";

  // Start Timer //
  Timer timer;
  timer.start();

  // Writer //
  WriterMsh writer;

  // Get Domains //
  Mesh msh(argv[1]);
  GroupOfElement domain = msh.getFromPhysical(7);
  GroupOfElement source = msh.getFromPhysical(5);
  GroupOfElement wall   = msh.getFromPhysical(6);

  // Get Parameters //
  const double       puls  = atof(argv[2]);
  const unsigned int order = atoi(argv[3]);

  // SteadyWaveVector //
  FormulationSteadyWaveVector sWave(domain, puls * 1, order);
  System sys(sWave);

  sys.dirichlet(source, fSource);
  sys.dirichlet(wall,   fWall);

  //sys.fixCoef(source, 1);
  //sys.fixCoef(wall,   0);

  cout << "Vectorial Steady Wave (Order: " << order
       << " --- Pulsation: "               << puls
       << "): " << sys.getSize()           << endl;

  sys.assemble();
  cout << "Assembled" << endl << flush;

  sys.solve();
  cout << "Solved"    << endl << flush;

  if(argc == 5){
    if(strcmp(argv[4], nopos)){
      // Interpolated View //
      // Visu Mesh
      Mesh visuMsh(argv[4]);
      GroupOfElement visu = visuMsh.getFromPhysical(7);

      Interpolator interp(sys, visu);
      interp.write("swavev", writer);
    }

    // If argv[4] == nopos --> do nothing //
  }

  else{
    // Adaptive View //
    writer.setValues(sys);
    writer.write("swavev");
  }

  // Timer -- Finalize -- Return //
  timer.stop();

  cout << "Elapsed Time: " << timer.time()
       << " s"             << endl;
}

int main(int argc, char** argv){
  // Init SmallFem //
  SmallFem::Initialize(argc, argv);

  compute(argc, argv);

  SmallFem::Finalize();
  return 0;
}
