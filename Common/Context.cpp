// $Id: Context.cpp,v 1.54 2005-01-01 19:35:27 geuzaine Exp $
//
// Copyright (C) 1997-2005 C. Geuzaine, J.-F. Remacle
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
// 
// Please report all bugs and problems to <gmsh@geuz.org>.

#include "Gmsh.h"
#include "Numeric.h"
#include "Geo.h"
#include "Mesh.h"
#include "Draw.h"
#include "Context.h"
#include "Options.h"
#include "DefaultOptions.h"
#include "Trackball.h"

/*
00 01 02 03    0  1  2  3
10 11 12 13    4  5  6  7
20 21 22 23    8  9  10 11
30 31 32 33    12 13 14 15
*/

void Context_T::buildRotmatrix(void)
{
  if(useTrackball) {
    build_rotmatrix(rot, quaternion);

    // get Euler angles from rotation matrix
    r[1] = asin(rot[8]); // Calculate Y-axis angle
    double C =  cos(r[1]);
    r[1] *=  180. / Pi;
    if(fabs(C) > 0.005){ // Gimball lock?
      double tmpx =  rot[10] / C; // No, so get X-axis angle
      double tmpy = -rot[9] / C;
      r[0] = atan2(tmpy, tmpx) * 180. / Pi;
      tmpx =  rot[0] / C; // Get Z-axis angle
      tmpy = -rot[4] / C;
      r[2] = atan2(tmpy, tmpx) * 180. / Pi;
    }
    else{ // Gimball lock has occurred
      r[0] = 0.; // Set X-axis angle to zero
      double tmpx = rot[5]; // And calculate Z-axis angle
      double tmpy = rot[1];
      r[2] = atan2(tmpy, tmpx) * 180. / Pi;
    }
    // return only positive angles in [0,360]
    if(r[0] < 0.) r[0] += 360.;
    if(r[1] < 0.) r[1] += 360.;
    if(r[2] < 0.) r[2] += 360.;
  }
  else {
    double x = r[0] * Pi / 180.;
    double y = r[1] * Pi / 180.;
    double z = r[2] * Pi / 180.;
    double A = cos(x);
    double B = sin(x);
    double C = cos(y);
    double D = sin(y);
    double E = cos(z);
    double F = sin(z);
    double AD = A * D;
    double BD = B * D;
    rot[0] = C*E; rot[1] = BD*E+A*F; rot[2] =-AD*E+B*F; rot[3] = 0.;
    rot[4] =-C*F; rot[5] =-BD*F+A*E; rot[6] = AD*F+B*E; rot[7] = 0.;
    rot[8] = D;   rot[9] =-B*C;	     rot[10] = A*C;     rot[11] = 0.;
    rot[12] = 0.; rot[13] = 0.;      rot[14] = 0.;      rot[15] = 1.;

    // get the quaternion from the Euler angles
    // todo
  }
}

void Context_T::addQuaternion(double p1x, double p1y, double p2x, double p2y)
{
  double quat[4];
  trackball(quat, p1x, p1y, p2x, p2y);
  add_quats(quat, quaternion, quaternion);
}

void Context_T::setQuaternion(double q0, double q1, double q2, double q3)
{
  quaternion[0] = q0;
  quaternion[1] = q1;
  quaternion[2] = q2;
  quaternion[3] = q3;
}
