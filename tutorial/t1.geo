/********************************************************************* 
 *
 *  Gmsh tutorial 1
 * 
 *  Variables, Elementary entities (Points, Lines, Surfaces), Physical
 *  entities (Points, Lines, Surfaces), Background mesh
 *
 *********************************************************************/

// All geometry description in Gmsh is made by means of a special
// language (looking somewhat similar to C). The simplest construction
// of this language is the 'affectation'. 

// The following command (all commands end with a semi colon) defines
// a variable called 'lc' and affects the value 0.007 to 'lc':

lc = 0.007 ;

// This newly created variable can be used to define the first Gmsh
// elementary entity, a 'Point'. A Point is defined by a list of four
// numbers: its three coordinates (x, y and z), and a characteristic
// length which sets the target mesh size at the point:

Point(1) = {0,  0,  0, 9.e-1 * lc} ;

// As can be seen in this definition, more complex expressions can be
// constructed from variables. Here, the product of the variable 'lc'
// by the constant 9.e-1 is given as the fourth argument of the list
// defining the point.
//
// The following general syntax rule is applied for the definition of
// all geometrical entities:
//
//    "If a number defines a new entity, it is enclosed between
//    parentheses. If a number refers to a previously defined entity,
//    it is enclosed between braces."
//
// Three additional points are then defined:

Point(2) = {.1, 0,  0, lc} ;
Point(3) = {.1, .3, 0, lc} ;
Point(4) = {0,  .3, 0, lc} ;

// The second elementary geometrical entity in Gmsh is the
// curve. Amongst curves, straight lines are the simplest. A straight
// line is defined by a list of point numbers. For example, line 1
// starts at point 1 and ends at point 2:

Line(1) = {1,2} ;
Line(2) = {3,2} ;
Line(3) = {3,4} ;
Line(4) = {4,1} ;

// The third elementary entity is the surface. In order to define a
// simple rectangular surface from the four lines defined above, a
// line loop has first to be defined. A line loop is a list of
// connected lines, a sign being associated with each line (depending
// on the orientation of the line).

Line Loop(5) = {4,1,-2,3} ;

// The surface is then defined as a list of line loops (only one
// here):

Plane Surface(6) = {5} ;

// At this level, Gmsh knows everything to display the rectangular
// surface 6 and to mesh it. But a supplementary step is needed in
// order to assign region numbers to the various elements in the mesh
// (the points, the lines and the triangles discretizing points 1 to
// 4, lines 1 to 4 and surface 6). This is achieved by the definition
// of Physical entities. Physical entities will group elements
// belonging to several elementary entities by giving them a common
// number (a region number), and specifying their orientation.
//
// For example, the two points 1 and 2 can be grouped into the
// physical entity 1:

Physical Point(1) = {1,2} ;

// Consequently, two punctual elements will be saved in the output
// files, both with the region number 1. The mechanism is identical
// for line or surface elements:

Physical Line(10) = {1,2,4} ;
Physical Surface(100) = {6} ;

// All the line elements which will be created during the mesh of
// lines 1, 2 and 4 will be saved in the output file with the region
// number 10; and all the triangular elements resulting from the
// discretization of surface 6 will be given the region number 100.

// It is important to notice that only those elements which belong to
// physical groups will be saved in the output file if the file format
// is the msh format (the native mesh file format for Gmsh). For a
// description of the mesh and post-processing formats, see the
// FORMATS file.

