/*********************************************************************
 *
 *  Gmsh tutorial 3
 * 
 *  Extruded meshes, options
 *
 *********************************************************************/

// Again, we start by including the first tutorial:

Include "t1.geo";

// As in `t2.geo', we plan to perform an extrusion along the z axis.
// But here, instead of only extruding the geometry, we also want to
// extrude the 2D mesh. This is done with the same `Extrude' command,
// but by specifying the number of layers (4 in this case, with 8, 4,
// 2 and 1 subdivisions, respectively), with volume numbers 9000 to
// 9003 and respective heights equal to h/4:

h = 0.1;

Extrude Surface { 6, {0,0,h} } { 
  Layers { {8,4,2,1}, {9000:9003}, {0.25,0.5,0.75,1} }; 
};

// The extrusion can also be performed with a rotation instead of a
// translation, and the resulting mesh can be recombined into prisms
// (wedges). All rotations are specified by an axis direction
// ({0,1,0}), an axis point ({-0.1,0,0.1}) and a rotation angle
// (-Pi/2):

Extrude Surface { 122, {0,1,0} , {-0.1,0,0.1} , -Pi/2 } { 
  Recombine; Layers { 7, 9004, 1 }; 
};

// Note that a translation ({-2*h,0,0}) and a rotation ({1,0,0},
// {0,0.15,0.25}, Pi/2) can also be combined:

aa[] = Extrude Surface {news-1, {-2*h,0,0}, {1,0,0} , {0,0.15,0.25} , Pi/2}{ 
  Layers { 10, 1 }; Recombine; 
}; ;

// In this last extrusion command we didn't specify an explicit
// volume number (which is equivalent to setting it to "0"),
// which means that the elements will simply belong the automatically
// created volume (whose number we get from the aa[] list).

// We finally define a new physical volume to save all the tetrahedra
// with a common region number (101):

Physical Volume(101) = {9000:9004, aa[1]};

// Let us now change some options... Since all interactive options are
// accessible in Gmsh's scripting language, we can for example define
// a global characteristic length factor, redefine some background
// colors, disable the display of the axes, and select an initial
// viewpoint in XYZ mode (disabling the interactive trackball-like
// rotation mode) directly in the input file:

Mesh.CharacteristicLengthFactor = 4;
General.Color.Background = {120,120,120};
General.Color.Foreground = {255,255,255};
General.Color.Text = White;
Geometry.Color.Points = Orange;
General.Axes = 0;
General.Trackball = 0;
General.RotationCenterGravity = 0;
General.RotationCenterX = 0;
General.RotationCenterY = 0;
General.RotationCenterZ = 0;
General.RotationX = 10;
General.RotationY = 70;
General.TranslationX = -0.2;

// Note that all colors can be defined literally or numerically, i.e.
// `General.Color.Background = Red' is equivalent to
// `General.Color.Background = {255,0,0}'; and also note that, as with
// user-defined variables, the options can be used either as right or
// left hand sides, so that the following command will set the surface
// color to the same color as the points:

Geometry.Color.Surfaces = Geometry.Color.Points;

// You can click on the `?'  button in the status bar of the graphic
// window to see the current values of all options. To save all the
// options in a file, you can use the `File->Save as->Gmsh options'
// menu. To save the current options as the default options for all
// future Gmsh sessions, you should use the `Tools->Options->Save'
// button.
