// $Id: PViewData.cpp,v 1.21 2008-05-04 08:31:24 geuzaine Exp $
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
#include "List.h"
#include "adaptiveData.h"
#include "Numeric.h"
#include "Message.h"

PViewData::PViewData()
  : _dirty(true), _fileIndex(0), _adaptive(0)
{
}

PViewData::~PViewData()
{
  if(_adaptive) delete _adaptive;
}

bool PViewData::finalize()
{ 
  if(!_adaptive && _interpolation.size()){
    Msg::Info("Initializing adaptive data %p interp size=%d",
	this, _interpolation.size());
    _adaptive = new adaptiveData(this);
    _adaptive->initWithLowResolution(0);
  }
  _dirty = false;
  return true;
}

bool PViewData::empty()
{
  return (!getNumElements() && !getNumStrings2D() && !getNumStrings3D());
}

void PViewData::getScalarValue(int step, int ent, int ele, int nod, double &val)
{
  int numComp = getNumComponents(step, ent, ele);
  if(numComp == 1){
    getValue(step, ent, ele, nod, 0, val);
  }
  else{
    std::vector<double> d(numComp);
    for(int comp = 0; comp < numComp; comp++)
      getValue(step, ent, ele, nod, comp, d[comp]);
    val = ComputeScalarRep(numComp, &d[0]);
  }
}

void PViewData::setNode(int step, int ent, int ele, int nod, double x, double y, double z)
{
  Msg::Error("Cannot change node coordinates in this view");
}

void PViewData::setValue(int step, int ent, int ele, int nod, int comp, double val)
{
  Msg::Error("Cannot change field value in this view");
}

void PViewData::setInterpolationScheme(int type, List_T *coef, List_T *pol, 
				       List_T *coefGeo, List_T *polGeo)
{
  Msg::Debug("Storing interpolation scheme %d in view %p", type, this);
  if(!type || !_interpolation[type].empty()) return;
  if(coef) _interpolation[type].push_back(coef);
  if(pol) _interpolation[type].push_back(pol);
  if(coefGeo) _interpolation[type].push_back(coefGeo);
  if(polGeo) _interpolation[type].push_back(polGeo);
}

int PViewData::getInterpolationScheme(int type, std::vector<List_T*> &p)
{
  if(_interpolation.count(type)){
    p = _interpolation[type];
    return p.size();
  }
  return 0;
}

void PViewData::smooth()
{
  Msg::Error("Smoothing is not implemented for this type of data");
}

bool PViewData::combineTime(nameData &nd)
{ 
  Msg::Error("Combine time is not implemented for this type of data");
  return false; 
}

bool PViewData::combineSpace(nameData &nd)
{ 
  Msg::Error("Combine space is not implemented for this type of data");
  return false; 
}
