// $Id: PViewData.cpp,v 1.17 2008-04-06 09:20:17 geuzaine Exp $
//
// Copyright (C) 1997-2008 C. Geuzaine, J.-F. Remacle
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
//
// Contributor(s):
// 

#include "PViewData.h"
#include "Numeric.h"
#include "Message.h"

PViewData::PViewData()
  : _dirty(true), _fileIndex(0)
{
}

bool PViewData::empty()
{
  return (!getNumElements() && !getNumStrings2D() && !getNumStrings3D());
}

void PViewData::getScalarValue(int step, int ent, int ele, int nod, double &val)
{
  int numComp = getNumComponents(step, ent, ele);
  std::vector<double> d(numComp);
  for(int comp = 0; comp < numComp; comp++)
    getValue(step, ent, ele, nod, comp, d[comp]);
  val = ComputeScalarRep(numComp, &d[0]);
}

void PViewData::setNode(int step, int ent, int ele, int nod, double x, double y, double z)
{
  Msg(GERROR, "Cannot change node coordinates in this view");
}

void PViewData::setValue(int step, int ent, int ele, int nod, int comp, double val)
{
  Msg(GERROR, "Cannot change field value in this view");
}
