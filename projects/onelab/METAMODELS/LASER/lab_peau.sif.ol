# in a name.xxx.ol file onelab parameter definition lines must be enclosed 
# between "OL.begin" and "OL.end" or start with "OL.line"
OL.block
#TimeStep.number(1, Parameters/Elmer/2,"Time step [ms]");
#TimeEnd.number(0.25,Parameters/Elmer/3,"Simulation end time [s]");
OL.endblock

Header
  Mesh DB "." "mesh"
  echo on
End
Simulation
  Max Output Level = 3
  Coordinate System =  Axi Symmetric 
  Simulation Type = Transient 
  Timestep Intervals (1) = OL.eval(floor(OL.get(Parameters/Elmer/3TimeEnd)/OL.get(Parameters/Elmer/2TimeStep)*1000))
  Timestep sizes (1) = OL.eval(OL.get(Parameters/Elmer/2TimeStep)/1000)
  Timestepping Method = Implicit Euler
!  BDF Order = 2
  Output Intervals = 1
End

!*********** Bodies ************
Body 1 !Dermis
  Equation = 1
  Material = OL.region(Dermis)
  Initial Condition = 1
  Body Force = 1
End

Body 2 !Epidermis
  Equation = 1
  Material = OL.region(Epidermis)
  Initial Condition = 1
  Body Force = 1
End

!*********** Equations ************
Equation 1
  Active Solvers(3) = 3 1 2
  !Convection = None
End
Solver 1
  Equation = "Heat Equation"
  Variable = String "Temperature"
  Linear System Solver = "Iterative"
  Linear System Iterative Method = "BiCGStab"
  Linear System Max Iterations = 350
  Linear System Convergence Tolerance = 1.0e-08
  Linear System Abort Not Converged = True
  Linear System Preconditioning = "ILU0"
  Linear System Residual Output = 1
  Steady State Convergence Tolerance = 1.0e-05
  Stabilize = True 
  Nonlinear System Convergence Tolerance = 1.0e-05
  Nonlinear System Max Iterations = 1
  Nonlinear System Newton After Iterations = 3
  Nonlinear System Newton After Tolerance = 1.0e-02
  Nonlinear System Relaxation Factor = 1.0
  Exported Variable 1 = String "DensityBis"
  Exported Variable 1 DOFs = 1
  Exported Variable 2 = String "Teneur"
  Exported Variable 2 DOFs = 1
  Exported Variable 3 = String "Qvolume"
  Exported Variable 3 DOFs = 1
  Exported Variable 4 = String "Pin"
  Exported Variable 4 DOFs = 1
  Exported Variable 5 = String "Timposed"
  Exported Variable 5 DOFs = 1
  NonLinear Update Exported Variables = Logical True
End
Solver 2
   Exec Solver = "After saving"
   Equation = String "ResultOutput"
   Procedure = File "ResultOutputSolve" "ResultOutputSolver"
   Output File Name = String "solutionOL.get(Solution/0TAG).pos"
   Output Format = String gmsh	
   Scalar Field 1 = String Temperature
   !Scalar Field 2 = String Teneur
   !Scalar Field 3 = String DensityBis
End

Solver 3 !ElmerModelsManuel page 187
  Exec solver = "Before timestep"
  Equation = "SaveScalars"
  Procedure = "SaveData" "SaveScalars"
  Variable 1 = String Temperature
  Variable 2 = Time
  Save Coordinates(2,2) = 1e-6 OL.eval(OL.get(PostPro/3SKINWIDTH)*1e-3) 1e-6 OL.eval( OL.get(Parameters/Skin/3DERMIS)*1e-3)
  Filename = "temp.txt"
  Target Variable 2 = String "Tsensor" 
End


!*********** Variables ************

$teneurw = OL.get(Parameters/Skin/WCONTENT)
$refl    = OL.get(Parameters/Skin/REFLECTIVITY)
$r       = OL.get(Parameters/Laser/2BEAMRADIUS)/1000
$mua     = OL.get(Parameters/Laser/ABSORPTION)
$tlaser  = OL.get(Parameters/Laser/STIMTIME)
$hp      = OL.get(PostPro/3SKINWIDTH)*1e-3
$ylaser  = hp
$temp    = OL.get(Parameters/Laser/LASERTEMP)
$power   = OL.get(Parameters/Laser/LASERPOWER)
$dT      = 4.0
$bb      = 0.1

!*********** Materials ************
Material 1 !dermis
OL.include( skinMaterial.ol )
End

Material 2 !epidermis
OL.include( skinMaterial.ol )
End

!*********** Initial condition ************

Initial Condition 1
  Temperature = Variable Coordinate 2
  Real MATC "OL.get(Parameters/Skin/TBASE) + (1-tx/hp)*(OL.get(Parameters/Skin/TCORE) - OL.get(Parameters/Skin/TBASE))"

  Timposed = Real OL.get(Parameters/Skin/TBASE)

  Teneur = Variable Coordinate 1, Coordinate 2
OL.if( OL.get(Parameters/Skin/SKINTYPE) == 1) # hairy
  Real MATC "0.25+0.4/(1+exp(-0.25*((hp-tx(1))*10^6-15)))"
OL.else # glabrous
  Real MATC "if((hp-tx(1))*10^6<80){0.15/80*(hp-tx(1))*10^6+0.25} else {0.25+0.35/(1+exp(-0.25*((hp-tx(1))*10^6-80)))}"
OL.endif

OL.if( OL.get(Parameters/Skin/TENEUR) )
  DensityBis = Variable Teneur
  Real MATC "1000/(0.0616*tx(0)+0.938)"
OL.else
  DensityBis = Real MATC "1000/(0.0616*teneurw+0.938)" 
OL.endif

  Qvolume = Variable DensityBis, Coordinate 1, Coordinate 2
OL.if( OL.get(Parameters/Laser/LASERSHAPE) == 1) # Gaussian
  Real MATC "(1-refl)*2/(pi*r*r)*mua*exp(-mua*(ylaser-tx(2))-2*tx(1)^2/(r*r))/tx(0)"
OL.else # Flat-top
  Real MATC "if(tx(1)<r){(1-refl)/(pi*r*r)*mua*exp(-mua*(ylaser-tx(2)))/tx(0)}else{0}"
OL.endif

End


!*********** Volume heat source ************
Body Force 1

OL.if( OL.get(Parameters/Laser/LASERTYPE) == 1)   # imposed power density

OL.iftrue(Parameters/Laser/QSKINFILE)
  OL.include(OL.get(Parameters/Laser/QSKINFILE));
  Heat Source = Variable Pin, Qvolume
  Real MATC " tx(0) * tx(1)"
OL.else
  Pin = Variable Time, Tsensor, Timposed
        Real MATC " power "
  Heat Source = Variable Time, Pin, Qvolume
  Real MATC " if(tx(0)<=tlaser) {tx(1) * tx(2)} else {0} "
OL.endif
OL.endif


#Remarks:
#$function soften(X) { _soften = min( 1.0+(bb-1.0)/(1.0-aa)*(X(1)/X(2)-aa)  1.0) }
#It seems one cannot have Variables in function definitions
#real constants should be written e.g. 1.0 and not 1.
# Attention: This is correct 
#  Real MATC "power * min((1.0+(bb-1.0)*(tx(1)-tx(2)+dT)/dT) 1.0)"
# This not (bug?)
#  Real MATC "power * min((1.0+(bb-1.0)/dT*(tx(1)-tx(2)+dT)) 1.0)"

OL.if( OL.get(Parameters/Laser/LASERTYPE) == 2)
  # temperature controlled power density
  # Heat Source = Variable Qvolume, Time, Tsensor/Temperature
OL.iftrue(Parameters/Laser/TSKINFILE)
   OL.include(OL.get(Parameters/Laser/TSKINFILE));
OL.else
   Timposed = Variable Time
     Real
     0.       OL.get(Parameters/Laser/7TEMP1)
     OL.get(Parameters/Laser/8TIME1)  OL.get(Parameters/Laser/7TEMP1)
     OL.eval(OL.get(Parameters/Laser/8TIME1)+OL.get(Parameters/Laser/9RAMPTIME))  OL.get(Parameters/Laser/5LASERTEMP)
     OL.eval(OL.get(Parameters/Laser/8TIME1)+OL.get(Parameters/Laser/9RAMPTIME)+OL.get(Parameters/Laser/6STIMTIME))  OL.get(Parameters/Laser/5LASERTEMP)
     OL.eval(OL.get(Parameters/Laser/8TIME1)+OL.get(Parameters/Laser/9RAMPTIME)+OL.get(Parameters/Laser/6STIMTIME)+1.e-3)  OL.get(Parameters/Skin/TBASE)
     10  OL.get(Parameters/Skin/6TBASE)
   End
OL.endif
  Pin = Variable Time, Tsensor, Timposed
  Real MATC "power * min((1.0+(bb-1.0)*(tx(1)-tx(2)+dT)/dT) 1.0)"
  Heat Source = Variable Pin, Qvolume
  Real MATC "if(tx(0)>0) {tx(0) * tx(1)} else {0}"
OL.endif
End

!*********** Boundary conditions ************
Boundary Condition 1 ! "Zero flux on axis"
  Target Boundaries(2) = OL.region(Axis)  OL.region(Side)
  Heat Flux BC = Logical true
  Heat Flux Real = Real 0.0
End

Boundary Condition 2  ! "body temperature on  bottom"
  Target Boundaries(1) =  OL.region(Bottom)
  Temperature = Variable Coordinate 2
  Real MATC "OL.get(Parameters/Skin/TCORE)"
End

OL.if( OL.get(Parameters/Laser/LASERTYPE) == 2) # controlled temperature

# In this case, laser power is injected through the volume heat source
# B.C. zero flux or convection applied at skin surface
Boundary Condition 3 ! 
  Target Boundaries(2) = OL.region(LaserSpot) OL.region(FreeSkin)
OL.if( OL.get(Parameters/Skin/CONVBC) ) # convection
  Heat Transfer Coefficient = Real OL.get(Parameters/Skin/HCONV)
  External Temperature = Real OL.get(Parameters/Skin/TAMBIANT)
OL.else # zero heat flux
  Heat Flux BC = Logical true
  Heat Flux Real = Real 0.0
OL.endif
End

OL.endif
