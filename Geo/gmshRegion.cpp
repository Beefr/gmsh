#include "gmshModel.h"
#include "gmshEdge.h"
#include "gmshFace.h"
#include "gmshRegion.h"
#include "Interpolation.h"
#include "CAD.h"
#include "Geo.h"

gmshRegion::gmshRegion(GModel *m,::Volume * volume):GRegion (m,volume->Num), v(volume)
{
  Surface *s;
  int ori;
  for (int i=0 ; i< List_Nbr ( v->Surfaces ) ; i++)
    {
      List_Read ( v->Surfaces , i, & s );
      List_Read ( v->SurfacesOrientations , i, & ori );
      GFace *f = m->faceByTag ( abs(s->Num) );
      if ( ! f ) throw;
      l_faces.push_back(f);
      l_dirs.push_back(ori);
    }
}


