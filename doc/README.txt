$Id: README.txt,v 1.6 2003-02-21 06:56:57 geuzaine Exp $

For Windows versions of Gmsh only:
==================================

1) About opengl32.dll and glu32.dll:

If your system complains about missing opengl32.dll or glu32.dll
libraries, then OpenGL is not properly installed... Go download OpenGL
on Microsoft's web site or on http://www.opengl.org.

2) About cygwin1.dll:

If you plan to use other programs than Gmsh that depend on the
cygwin1.dll library, you should keep only one version of this library
on your system (i.e., move cygwin1.dll to the Windows system
directory--usually C:\Windows\System\--and suppress all other versions
of cygwin1.dll). Failing to do so may result in an incorrect behaviour
of applications sharing the library and running simultaneously.

3) About configuration files:

Gmsh saves session information and default options in the $TMP (or
$TEMP) directory. If the variables $TMP and $TEMP are undefined, Gmsh
will save/load its configuration files from the current working
directory.

