#include "BDS.h"
#include <map>
#include <math.h>
#include "Numeric.h"

/*
  (X-Xc)^2 = R^2

  

*/



BDS_Sphere :: BDS_Sphere( BDS_Point *p1, BDS_Point *p2, BDS_Point *p3, BDS_Point *p4)
{
  double mat[3][3], b[3], dum,res[3];
  int i;
  double X[4] = {p1->X,p2->X,p3->X,p4->X};
  double Y[4] = {p1->Y,p2->Y,p3->Y,p4->Y};
  double Z[4] = {p1->Z,p2->Z,p3->Z,p4->Z};

  b[0] = X[1] * X[1] - X[0] * X[0] +
    Y[1] * Y[1] - Y[0] * Y[0] + Z[1] * Z[1] - Z[0] * Z[0];
  b[1] = X[2] * X[2] - X[1] * X[1] +
    Y[2] * Y[2] - Y[1] * Y[1] + Z[2] * Z[2] - Z[1] * Z[1];
  b[2] = X[3] * X[3] - X[2] * X[2] +
    Y[3] * Y[3] - Y[2] * Y[2] + Z[3] * Z[3] - Z[2] * Z[2];

  for(i = 0; i < 3; i++)
    b[i] *= 0.5;

  mat[0][0] = X[1] - X[0];
  mat[0][1] = Y[1] - Y[0];
  mat[0][2] = Z[1] - Z[0];
  mat[1][0] = X[2] - X[1];
  mat[1][1] = Y[2] - Y[1];
  mat[1][2] = Z[2] - Z[1];
  mat[2][0] = X[3] - X[2];
  mat[2][1] = Y[3] - Y[2];
  mat[2][2] = Z[3] - Z[2];


//  printf("X = %g %g %g %g\n",X[0],X[1],X[2],X[3]);
//  printf("Y = %g %g %g %g\n",Y[0],Y[1],Y[2],Y[3]);
//  printf("Z = %g %g %g %g\n",Z[0],Z[1],Z[2],Z[3]);

  if(!sys3x3(mat, b, res, &dum)) {
      R = 1.e22;
      xc = yc = zc = 1.e33;
//      printf("ahrgh\n");
  }
  else
  {
      xc = res[0];
      yc = res[1];
      zc = res[2];
      R = sqrt ((p1->X-xc)*(p1->X-xc)+(p1->Y-yc)*(p1->Y-yc)+(p1->Z-zc)*(p1->Z-zc));
  }
}

double dist_droites_gauches(BDS_Point *p1, BDS_Point *p2, 
			    BDS_Point *p3 ,BDS_Point *p4)
{
    BDS_Vector u1 ( *p2, *p1 );
    BDS_Vector u2 ( *p4, *p3 );
    BDS_Vector a  ( *p2, *p4 );
    BDS_Vector u1xu2 = u1%u2;
    double x = sqrt (u1xu2 * u1xu2 );
    // les droites sont paralleles
    if (x == 0)
    {
	throw;
    }
    // les droites sont gauches
    else
    {
	double y = fabs((a % u1) * u2);
	return y/x; 
    }
    
}

void proj_point_plane ( double xa, double ya, double za,
			BDS_Point *p1, BDS_Point *p2, BDS_Point *p3 ,
			double &x, double &y, double &z)
{
    /// plane ax+by+cz+1 = 0;

    double mat[3][3];
    mat[0][0] = p1->X; mat[0][1] = p1->Y; mat[0][2] = p1->Z;
    mat[1][0] = p2->X; mat[1][1] = p2->Y; mat[1][2] = p2->Z;
    mat[2][0] = p3->X; mat[2][1] = p3->Y; mat[2][2] = p3->Z;
    double b[3] = {-1,-1,-1};
    double pl[3] , det ;
    sys3x3 (mat,b,pl,&det);
    double n[3] = {pl[0],pl[1],pl[2]}; 
//    norme(n);
    // pp(x,y,z) = p + k n
    // pp belongs to the plane
    // a x + b y + c z + 1 = 0
    // x = xa + k nx
    // y = ya + k ny
    // z = za + k nz
    // a (xa + k nx) + b ( ya + k ny ) + c (  za + k nz ) + 1 = 0
    // k = - ( a xa + b ya + c za + 1) / (a nx + b ny + c nz )
    double k = - ( pl[0] * xa +  pl[1] * ya +  pl[2] * za + 1 ) / ( pl[0] * n[0] + pl[1] * n[1] + pl[2] * n[2] ); 
    x = xa + k * n[0];
    y = ya + k * n[1];
    z = za + k * n[2];
}

void print_face (BDS_Triangle *t)
{
    BDS_Point *pts[3];
    t->getNodes (pts); 
    printf("face %p with nodes %d %d %d and edges %p (%d %d) %p (%d %d) %p (%d %d)\n",(void*)t,pts[0]->iD,pts[1]->iD,pts[2]->iD,
	   (void*)t->e1,t->e1->p1->iD,t->e1->p2->iD,(void*)t->e2,t->e2->p1->iD,t->e2->p2->iD,(void*)t->e3,t->e3->p1->iD,t->e3->p2->iD);
}
void print_edge (BDS_Edge *e)
{
  printf("edge %p with nodes %d %d ------------------\n",(void*)e,e->p1->iD,e->p2->iD);
    printf("faces : \n ");
    for (int i=0;i<e->numfaces();++i)
	print_face (e->faces(i));

    printf("----------------------------------------------\n ");
}

void vector_triangle (BDS_Point *p1, BDS_Point *p2, BDS_Point *p3, double c[3]) 
{
    double a[3] = { p1->X - p2->X,p1->Y - p2->Y,p1->Z - p2->Z};
    double b[3] = { p1->X - p3->X,p1->Y - p3->Y,p1->Z - p3->Z};
    c[2] = a[0] * b[1] - a[1] * b[0];
    c[1] = -a[0] * b[2] + a[2] * b[0];
    c[0] = a[1] * b[2] - a[2] * b[1];
}

void normal_triangle (BDS_Point *p1, BDS_Point *p2, BDS_Point *p3, double c[3]) 
{
    vector_triangle (p1,p2,p3,c);
    double l = sqrt (c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);
    if (l==0)return;
    c[0]/=l;
    c[1]/=l;
    c[2]/=l;
}

double surface_triangle (BDS_Point *p1, BDS_Point *p2, BDS_Point *p3) 
{
    double c[3];
    vector_triangle (p1,p2,p3,c);
    return 0.5*sqrt (c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);
}
double quality_triangle  (BDS_Point *p1, BDS_Point *p2, BDS_Point *p3)
{
    double e12 =  ((p1->X - p2->X)*(p1->X - p2->X)+
		   (p1->Y - p2->Y)*(p1->Y - p2->Y)+
		   (p1->Z - p2->Z)*(p1->Z - p2->Z));
    double e22 =  ((p1->X - p3->X)*(p1->X - p3->X)+
		   (p1->Y - p3->Y)*(p1->Y - p3->Y)+
		   (p1->Z - p3->Z)*(p1->Z - p3->Z));
    double e32 =  ((p3->X - p2->X)*(p3->X - p2->X)+
		   (p3->Y - p2->Y)*(p3->Y - p2->Y)+
		   (p3->Z - p2->Z)*(p3->Z - p2->Z));
    double a =  surface_triangle ( p1,p2,p3);
    return a / (e12+e22+e32);
}

BDS_Plane::BDS_Plane (BDS_Point *p1, BDS_Point *p2, BDS_Point *p3)
{
    double mat[3][3];
    mat[0][0] = p1->X; mat[0][1] = p1->Y; mat[0][2] = p1->Z;
    mat[1][0] = p2->X; mat[1][1] = p2->Y; mat[1][2] = p2->Z;
    mat[2][0] = p3->X; mat[2][1] = p3->Y; mat[2][2] = p3->Z;
    double rhs[3] = {-1,-1,-1};
    double pl[3] , det ;
    sys3x3 (mat,rhs,pl,&det);
    a = pl[0];
    b = pl[1];
    c = pl[2];
}


void BDS_Point::getTriangles (std::list<BDS_Triangle *> &t)
{
  t.clear ();
  std::list<BDS_Edge*>::iterator it  = edges.begin();
  std::list<BDS_Edge*>::iterator ite = edges.end();
  while(it!=ite)
    {
      int NF = (*it)->numfaces();
      for (int i=0;i<NF;++i)
	{
	  BDS_Triangle *tt = (*it)->faces(i);
	  if (tt)
	    {
	      std::list<BDS_Triangle*>::iterator tit  = t.begin();
	      std::list<BDS_Triangle*>::iterator tite = t.end();
	      int found = 0;
	      while (tit!=tite) 
		{
		  if (tt == *tit)found = 1;
		  ++tit;
		}
	      if (!found) t.push_back (tt);
	    }		    
	}
      ++it;
    }
}

BDS_Point* BDS_Mesh :: add_point (int num , double x, double y,double z)
{
    BDS_Point *pp = new BDS_Point ( num, x, y, z);
    points.insert ( pp );    
    MAXPOINTNUMBER = ( MAXPOINTNUMBER < num )? num : MAXPOINTNUMBER;
    return pp;
}

BDS_Point *BDS_Mesh :: find_point  (int p)
{
    BDS_Point P ( p ) ;
    std::set<BDS_Point*,PointLessThan>::iterator it = points.find(&P);
    if (it != points.end()) return (BDS_Point*) (*it);
    else return 0;
}

BDS_Edge *BDS_Mesh :: find_edge  (int num1, int num2)
{
    BDS_Point P1 ( num1 ) ;
    BDS_Point P2 ( num2 ) ;
    BDS_Edge  E (&P1, &P2);
    std::set<BDS_Edge*, EdgeLessThan>::iterator it = edges.find(&E);
    if (it != edges.end()) return (BDS_Edge*) (*it);
    else return 0;
}

BDS_Edge *BDS_Mesh :: find_edge  (BDS_Point *p1, BDS_Point *p2, BDS_Triangle *t) const
{
    BDS_Point P1 ( p1->iD ) ;
    BDS_Point P2 ( p2->iD ) ;
    BDS_Edge  E (&P1, &P2);
    if (t->e1->p1->iD == E.p1->iD && t->e1->p2->iD == E.p2->iD)return t->e1;
    if (t->e2->p1->iD == E.p1->iD && t->e2->p2->iD == E.p2->iD)return t->e2;
    if (t->e3->p1->iD == E.p1->iD && t->e3->p2->iD == E.p2->iD)return t->e3;
    return 0;
}

BDS_Edge *BDS_Mesh :: add_edge  (int p1, int p2)
{
    BDS_Edge *efound = find_edge (p1,p2);
    if (efound)return efound;

    BDS_Point *pp1=find_point(p1);
    BDS_Point *pp2=find_point(p2);
    if (!pp1 || !pp2)throw;
    BDS_Edge *e = new BDS_Edge ( pp1, pp2 );
    edges.insert ( e );
    return e;
}

BDS_Triangle * BDS_Mesh :: add_triangle  (int p1, int p2, int p3 )
{
    add_edge (p1,p2);
    add_edge (p2,p3);
    add_edge (p3,p1);
    BDS_Edge *e1 = find_edge (p1,p2); 
    BDS_Edge *e2 = find_edge (p2,p3); 
    BDS_Edge *e3 = find_edge (p3,p1); 
    try {
	BDS_Triangle *t  = new BDS_Triangle ( e1, e2, e3 );    
	triangles.push_back ( t );
        return t;
    }
    catch (...)
    {
      return 0;
    }
}
void  BDS_Mesh :: del_triangle  (BDS_Triangle *t)
{
    t->e1->del ( t );
    t->e2->del ( t );
    t->e3->del ( t );
    t->deleted = true;
}
void  BDS_Mesh :: del_edge  (BDS_Edge *e)
{
    edges.erase ( e );
    e->p1->del ( e );
    e->p2->del ( e );
    e->deleted = true;
    edges_to_delete.insert (e);
}
void  BDS_Mesh :: del_point  (BDS_Point *p)
{
    points.erase ( p );
    delete p;
}
void BDS_Mesh :: add_geom  (int p1, int p2)
{
    geom.insert ( new BDS_GeomEntity ( p1,p2 ) ) ;        
}

void BDS_Edge::oppositeof (BDS_Point * oface[2]) const
{
    
    oface[0] = oface[1] = 0;
    if (faces(0))
    {
	BDS_Point *pts[3];
	faces(0)->getNodes (pts); 
	if (pts[0] != p1 && pts[0] != p2)oface[0] = pts[0];
	else if (pts[1] != p1 && pts[1] != p2)oface[0] = pts[1];
	else oface[0] = pts[2];
    }
    if (faces(1))
    {
	BDS_Point *pts[3];
	faces(1)->getNodes (pts); 
	if (pts[0] != p1 && pts[0] != p2)oface[1] = pts[0];
	else if (pts[1] != p1 && pts[1] != p2)oface[1] = pts[1];
	else oface[1] = pts[2];
    }
}



BDS_GeomEntity * BDS_Mesh :: get_geom  (int p1, int p2)
{
    BDS_GeomEntity ge ( p1,p2 );
    std::set<BDS_GeomEntity*,GeomLessThan>::iterator it = geom.find(&ge);
    if (it == geom.end())return 0;
    return (BDS_GeomEntity*) (*it);
}


void recur_tag ( BDS_Triangle *t , BDS_GeomEntity *g )
{
  if (!t->g)
    {
      t->g = g;
      if ( ! t->e1->g && t->e1->numfaces() == 2)
	{
	  recur_tag (t->e1->otherFace (t) , g);
	}
      if ( ! t->e2->g && t->e2->numfaces() == 2)
	{
	  recur_tag (t->e2->otherFace (t) , g);
	}
      if ( ! t->e3->g && t->e3->numfaces() == 2)
	{
	  recur_tag (t->e3->otherFace (t) , g);
	}
    }
}

BDS_Vector BDS_Triangle :: N () const 
{
    BDS_Point *pts[3];
    getNodes (pts); 
    double c[3];
    normal_triangle (pts[0],pts[1],pts[2],c); 
    return BDS_Vector ( c[0], c[1], c[2] );
}


void BDS_Mesh :: reverseEngineerCAD ( ) 
{
    for (std::set<BDS_GeomEntity*,GeomLessThan>::iterator it = geom.begin();
	 it != geom.end();
	 ++it)
    {
	if ((*it)->classif_degree == 2)
	{
	    std::set<BDS_Point*,PointLessThan> pts;

	    std::list<BDS_Triangle*>::iterator tit  = triangles.begin();
	    std::list<BDS_Triangle*>::iterator tite = triangles.end();
	    while (tit!=tite)
	    {
		if ((*tit)->g == (*it))
		{
		    BDS_Point *nod[3];
		    (*tit)->getNodes (nod);
		    pts.insert(nod[0]);
		    pts.insert(nod[1]);
		    pts.insert(nod[2]);
		}
		tit++;
	    }
	    if(pts.size() > 3)
	    {
		// TEST PLANE 
		{
		    std::set<BDS_Point*,PointLessThan>::iterator pit  = pts.begin();
		    std::set<BDS_Point*,PointLessThan>::iterator pite = pts.end();
		    
		    BDS_Point *p1 = *pit;++pit;
		    BDS_Point *p2 = *pit;++pit;
		    BDS_Point *p3 = *pit;++pit;
		    BDS_Plane *plane = new BDS_Plane (  p1 , p2 , p3 );
		    double distmax = 0;
		    while (pit!=pite)
		    {
			double dist = plane->signedDistanceTo (*pit);
			if (fabs(dist) > distmax) distmax = fabs(dist);
			++pit;
		    }
		    if (distmax < 1.e-6 * LC ) 
		    {
			(*it)->surf = plane;
			printf("plane surface found %g\n",distmax); 
		    }
		    else
		    { 
//			printf("non plane surface found %g\n",distmax); 
			delete plane;
		    }
		}
	    }
	    if(pts.size() > 4 && !(*it)->surf)
	    {
		// TEST SPHERE
		{
		    std::set<BDS_Point*,PointLessThan>::iterator pit  = pts.begin();
		    std::set<BDS_Point*,PointLessThan>::iterator pite = pts.end();
		    
		    BDS_Point *p1 = *pit;++pit;
		    BDS_Point *p2 = *pit;++pit;
		    BDS_Point *p3 = *pit;++pit;
		    BDS_Point *p4 = *pit;++pit;
		    BDS_Sphere *sphere = new BDS_Sphere (  p1 , p2 , p3 , p4);
		    double distmax = 0;
		    while (pit!=pite)
		    {
			double dist = sphere->signedDistanceTo (*pit);
			if (fabs(dist) > distmax) distmax = fabs(dist);
			++pit;
		    }
		    if (distmax < 1.e-116 * LC ) 
		    {
			(*it)->surf = sphere;
			printf("spherical surface %d found %g (R=%g,LC=%g)\n",(*it)->classif_tag,distmax,sphere->R,LC); 
		    }
		    else
		    { 
//			printf("non spherical surface found %g\n",distmax); 
			delete sphere;
		    }
		}
	    }
	}
    }
}

void BDS_Mesh :: classify ( double angle )
{

  printf("  classifying \n");
 
    static BDS_GeomEntity EDGE_CLAS (0,1);
    {
	std::set<BDS_Edge*, EdgeLessThan>::iterator it  = edges.begin();
	std::set<BDS_Edge*, EdgeLessThan>::iterator ite = edges.end();
	while (it!=ite)
	{
	    BDS_Edge &e = *((BDS_Edge *) *it);
	    if ( e.numfaces() == 1) 
	    {
		e.g = &EDGE_CLAS;		
	    }

	    else if (e.numfaces() == 2 && 
		     e.faces(0)->g != e.faces(1)->g )
	      {
		e.g = &EDGE_CLAS;
	      }

	    else if (e.numfaces() == 2)
	    {
		BDS_Vector N0 = e.faces(0)->N();
		BDS_Vector N1 = e.faces(1)->N();
		double ag = N0.angle (N1);
		if (fabs(ag) > angle)e.g = &EDGE_CLAS;
	    }
	    else
	    {
		e.g = &EDGE_CLAS;
	    }
	    ++it;
	}
    }

    {
      std::list<BDS_Triangle*>::iterator it  = triangles.begin();
      std::list<BDS_Triangle*>::iterator ite = triangles.end();
      while (it!=ite)
	{
	  (*it)->g = 0;
	  ++it;
	}
      geom.clear();
    }

    {
      int tag = 1;
      while (1)
	{
	  std::list<BDS_Triangle*>::iterator it  = triangles.begin();
	  std::list<BDS_Triangle*>::iterator ite = triangles.end();
	  BDS_Triangle *start = 0;
	  while (it!=ite)
	    {
	      if (!(*it)->deleted && !(*it)->g)
		{
		  start = (BDS_Triangle*) (*it);
		}
	      if (start)break;
	      ++it;
	    }
	  if (!start)break;	  
	  add_geom (tag, 2);
	  BDS_GeomEntity *g = get_geom (tag,2);
	  recur_tag ( start , g );
	  tag++;
	}
    }
    int edgetag = 1;
    {
	std::map< std::pair<int,int> , int > edgetags;
	std::set<BDS_Edge*, EdgeLessThan>::iterator it  = edges.begin();
	std::set<BDS_Edge*, EdgeLessThan>::iterator ite = edges.end();	
	while (it!=ite)
	{
	    BDS_Edge &e = *((BDS_Edge *) *it);
	    
	    if (e.g)
	    {	    
		std::map< std::pair<int,int> , int >::iterator found;
		BDS_GeomEntity *g;
		if ( e.numfaces() == 1)
		{
		    found = edgetags.find (std::make_pair ( e.faces(0)->g->classif_tag,-1));
		}
		else if (e.numfaces() == 2 && e.faces(0)->g->classif_tag != e.faces(1)->g->classif_tag)
		{
		    if (e.faces(0)->g->classif_tag > e.faces(1)->g->classif_tag)
			found = edgetags.find (std::make_pair ( e.faces(1)->g->classif_tag,e.faces(0)->g->classif_tag));
		    else
			found = edgetags.find (std::make_pair ( e.faces(0)->g->classif_tag,e.faces(1)->g->classif_tag));
		}

		if (e.g)
		{
		    if (found == edgetags.end())
		    {
			add_geom (edgetag, 1);
			g = get_geom (edgetag,1);
			if ( e.numfaces() == 1)
			{
			    edgetags[std::make_pair ( e.faces(0)->g->classif_tag,-1)] = edgetag;
			}
			else if (e.numfaces() == 2)
			{
			    if (e.faces(0)->g->classif_tag > e.faces(1)->g->classif_tag)
				edgetags[std::make_pair ( e.faces(1)->g->classif_tag,e.faces(0)->g->classif_tag)]
				    = edgetag;
			    else
				edgetags[std::make_pair ( e.faces(0)->g->classif_tag,e.faces(1)->g->classif_tag)] 
				    = edgetag;
			}
			edgetag++;
		    }
		    else
		    {
			g = get_geom(found->second,1);
		    }
		    e.g = g;
		}
	    }
	    else
	    {
		e.g = e.faces(0)->g;
	    }	    
	    ++it;
	}
    }
    int vertextag = 1;
    {
	std::set<BDS_Point*,PointLessThan>::iterator it  = points.begin();
	std::set<BDS_Point*,PointLessThan>::iterator ite = points.end();
	while (it != ite)
	{
	    std::list<BDS_Edge *>::iterator eit  = (*it)->edges.begin(); 	
	    std::list<BDS_Edge *>::iterator eite =  (*it)->edges.end();
	    std::set<BDS_GeomEntity*> geoms;
	    BDS_GeomEntity* vg = 0;
	    while (eit!=eite)
	    {
		if ((*eit)->g && (*eit)->g->classif_degree == 1)
		    geoms.insert ( (*eit)->g );
		else vg = (*eit)->g;
		    
		++eit;
	    }
	    if ( geoms.size() == 0){
		(*it)->g = vg;
	    }
	    else if ( geoms.size() == 1) {
		(*it)->g = (*(geoms.begin()));
	    }
	    else {
		add_geom (vertextag, 0);
		(*it)->g = get_geom(vertextag++,0);
	    }	    
	    if (!(*it)->g)printf("argh\n");
	    ++it;
	}	
    }
    {
	std::set<BDS_Point*,PointLessThan>::iterator it  = points.begin();
	std::set<BDS_Point*,PointLessThan>::iterator ite = points.end();
	while (it != ite)
	{
	  if ((*it)->g->classif_degree > 1)
	    {
	      std::list<BDS_Triangle *> t; 	
	      (*it)->getTriangles (t);
	      std::list<BDS_Triangle *>::iterator tit  = t.begin(); 	
	      std::list<BDS_Triangle *>::iterator tite =  t.end();
	      BDS_Vector SumN;
	      while (tit!=tite)
		{
		  SumN += (*tit)->N();
		  ++tit;
		}
	      double max_angle = 0;
	      double NORM = sqrt(SumN*SumN);
	      SumN /= NORM;
	      tit  = t.begin(); 	
	      while (tit!=tite)
		{
		  double ag = SumN.angle ((*tit)->N());
		  if (fabs(ag) > max_angle) max_angle = fabs(ag);
		  ++tit;
		}
	      if (fabs(max_angle) > angle)
		{
		  add_geom (vertextag, 0);
		  (*it)->g = get_geom(vertextag++,0);
		}
	    }	    
	  ++it;
	}	
    }
    reverseEngineerCAD ( ) ;

  printf("  end classifying %d edgetags %d vertextags\n",edgetag-1,vertextag-1);
}
double PointLessThanLexicographic::t = 0;

bool BDS_Mesh :: read_stl ( const char *filename , const double tolerance)
{

    FILE *f = fopen ( filename, "r");
    if (!f)return false;
    char buffer [256], name[256];

    PointLessThanLexicographic::t = tolerance;
       
    fgets (buffer,255,f);
    
    std::set < BDS_Point *, PointLessThanLexicographic > pts; 

    sscanf (buffer,"%s",name);
    // ASCII STL    
    if (!strcmp(name,"solid"))
    {
	rewind(f);
	fgets (buffer,255,f);

	Min[0] = Min[1] = Min[2] = 1.e12;
	Max[0] = Max[1] = Max[2] = -1.e12;

	double XCG = 0;
	double YCG = 0;
	double ZCG = 0;

	int Nf = 0;
	while (!feof(f))
	{
	    char s1[256],s2[256];
	    float x,y,z;
	    fgets (buffer,255,f);
	    sscanf(buffer,"%s",s1);
	    if (!strcmp(s1,"endsolid"))break;
	    sscanf(buffer,"%s %s %f %f %f",s1,s2,&x,&y,&z);
	    fgets (buffer,255,f);
	    fgets (buffer,255,f); // 1st point
	    sscanf(buffer,"%s %f %f %f",s2,&x,&y,&z);
	    BDS_Point P1 ( 0 , x,y,z);
	    fgets (buffer,255,f); // 2nd point
	    sscanf(buffer,"%s %f %f %f",s2,&x,&y,&z);
	    BDS_Point P2 ( 0 , x,y,z); 
	    fgets (buffer,255,f); // 3rd point
	    sscanf(buffer,"%s %f %f %f",s2,&x,&y,&z);
	    BDS_Point P3 ( 0 , x,y,z);
	    fgets (buffer,255,f); // end loop
	    fgets (buffer,255,f);// end facet

	    Nf ++;

	    if ( P1.X < Min [0]) Min[0] = P1.X;
	    if ( P2.X < Min [0]) Min[0] = P2.X;
	    if ( P3.X < Min [0]) Min[0] = P3.X; 

	    if ( P1.Y < Min [1]) Min[1] = P1.Y;
	    if ( P2.Y < Min [1]) Min[1] = P2.Y;
	    if ( P3.Y < Min [1]) Min[1] = P3.Y;

	    if ( P1.Z < Min [2]) Min[2] = P1.Z;
	    if ( P2.Z < Min [2]) Min[2] = P2.Z;
	    if ( P3.Z < Min [2]) Min[2] = P3.Z;

	    if ( P1.X > Max [0]) Max[0] = P1.X;
	    if ( P2.X > Max [0]) Max[0] = P2.X;
	    if ( P3.X > Max [0]) Max[0] = P3.X; 

	    if ( P1.Y > Max [1]) Max[1] = P1.Y;
	    if ( P2.Y > Max [1]) Max[1] = P2.Y;
	    if ( P3.Y > Max [1]) Max[1] = P3.Y;

	    if ( P1.Z > Max [2]) Max[2] = P1.Z;
	    if ( P2.Z > Max [2]) Max[2] = P2.Z;
	    if ( P3.Z > Max [2]) Max[2] = P3.Z;

	    XCG += (P1.X + P2.X + P3.X);
	    YCG += (P1.Y + P2.Y + P3.Y);
	    ZCG += (P1.Z + P2.Z + P3.Z);
	}

	XCG /= (3 * Nf);
	YCG /= (3 * Nf);
	ZCG /= (3 * Nf);

	Min [0] -= XCG;
	Min [1] -= YCG;
	Min [2] -= ZCG;
	Max [0] -= XCG;
	Max [1] -= YCG;
	Max [2] -= ZCG;

	LC = sqrt ((Min[0]-Max[0])*(Min[0]-Max[0])+
		   (Min[1]-Max[1])*(Min[1]-Max[1])+
		   (Min[2]-Max[2])*(Min[2]-Max[2]));

	printf("LC = %g\n",LC);

	PointLessThanLexicographic::t = LC * tolerance;

	rewind(f);
	fgets (buffer,255,f);
	while (!feof(f))
	{
	    char s1[256],s2[256];
	    float x,y,z;
	    fgets (buffer,255,f);
	    sscanf(buffer,"%s",s1);
	    if (!strcmp(s1,"endsolid"))break;
	    sscanf(buffer,"%s %s %f %f %f",s1,s2,&x,&y,&z);
	    fgets (buffer,255,f);
	    fgets (buffer,255,f); // 1st point
	    sscanf(buffer,"%s %f %f %f",s2,&x,&y,&z);
	    BDS_Point P1 ( 0 , x-XCG,y-YCG,z-ZCG);
	    fgets (buffer,255,f); // 2nd point
	    sscanf(buffer,"%s %f %f %f",s2,&x,&y,&z);
	    BDS_Point P2 ( 0 , x-XCG,y-YCG,z-ZCG); 
	    fgets (buffer,255,f); // 3rd point
	    sscanf(buffer,"%s %f %f %f",s2,&x,&y,&z);
	    BDS_Point P3 ( 0 , x-XCG,y-YCG,z-ZCG);
	    fgets (buffer,255,f); // end loop
	    fgets (buffer,255,f);// end facet
	    BDS_Point *p1 = 0;
	    BDS_Point *p2 = 0;
	    BDS_Point *p3 = 0;
	    std::set < BDS_Point *, PointLessThanLexicographic >::iterator it1 = pts.find(&P1); 
	    if ( it1 != pts.end())
	    {
		p1 = *it1;
	    }
	    else
	    {
		MAXPOINTNUMBER++;
		p1 =  add_point (MAXPOINTNUMBER , P1.X, P1.Y,P1.Z );
		pts.insert (p1);
	    }
	    std::set < BDS_Point *, PointLessThanLexicographic >::iterator it2 = pts.find(&P2); 
	    if ( it2 != pts.end())
	    {
		p2 = *it2;
	    }
	    else
	    {
		MAXPOINTNUMBER++;
		p2 =  add_point (MAXPOINTNUMBER , P2.X, P2.Y,P2.Z );
		pts.insert (p2);
	    }
	    std::set < BDS_Point *, PointLessThanLexicographic >::iterator it3 = pts.find(&P3); 
	    if ( it3 != pts.end())
	    {
		p3 = *it3;
	    }
	    else
	    {
		MAXPOINTNUMBER++;
		p3 =  add_point (MAXPOINTNUMBER , P3.X, P3.Y,P3.Z );
		pts.insert (p3);
	    }	
	    add_triangle ( p1->iD, p2->iD, p3->iD); 
	}		
    }
    // BINARY STL 
    else
    {
	rewind(f);
	char DUMMY[80];
	fread (DUMMY,1,80,f);

	unsigned long int Nf;
	size_t x = fread (&Nf,sizeof (unsigned long int),1,f);	
	char *DATA = new char [Nf * 50 * sizeof (char)];
	x = fread (DATA,sizeof (char),Nf * 50,f);

	printf("BINARY STL data read, %ld facets \n",Nf);

	Min[0] = Min[1] = Min[2] = 1.e12;
	Max[0] = Max[1] = Max[2] = -1.e12;

	double XCG = 0;
	double YCG = 0;
	double ZCG = 0;

	for (unsigned int i=0;i<Nf;++i)
	{
	    float *X = (float*) &DATA[i * 50 * sizeof (char)];
	    
	    if ( X[3] < Min [0]) Min[0] = X[3];
	    if ( X[6] < Min [0]) Min[0] = X[6];
	    if ( X[9] < Min [0]) Min[0] = X[9];

	    if ( X[4] < Min [1]) Min[1] = X[4];
	    if ( X[7] < Min [1]) Min[1] = X[7];
	    if ( X[10] < Min [1]) Min[1] = X[10];

	    if ( X[5] < Min [2]) Min[2] = X[5];
	    if ( X[8] < Min [2]) Min[2] = X[8];
	    if ( X[11] < Min [2]) Min[2] = X[11];

	    if ( X[3] > Max [0]) Max[0] = X[3];
	    if ( X[6] > Max [0]) Max[0] = X[6];
	    if ( X[9] > Max [0]) Max[0] = X[9];

	    if ( X[4] > Max [1]) Max[1] = X[4];
	    if ( X[7] > Max [1]) Max[1] = X[7];
	    if ( X[10] > Max [1]) Max[1] = X[10];

	    if ( X[5] > Max [2]) Max[2] = X[5];
	    if ( X[8] > Max [2]) Max[2] = X[8];
	    if ( X[11] > Max [2]) Max[2] = X[11];
	    XCG += (X[3] + X[6] + X[9]);
	    YCG += (X[4] + X[7] + X[10]);
	    ZCG += (X[5] + X[8] + X[11]);
	}

	XCG /= (3 * Nf);
	YCG /= (3 * Nf);
	ZCG /= (3 * Nf);

	Min [0] -= XCG;
	Min [1] -= YCG;
	Min [2] -= ZCG;
	Max [0] -= XCG;
	Max [1] -= YCG;
	Max [2] -= ZCG;
	LC = sqrt ((Min[0]-Max[0])*(Min[0]-Max[0])+
		   (Min[1]-Max[1])*(Min[1]-Max[1])+
		   (Min[2]-Max[2])*(Min[2]-Max[2]));

	PointLessThanLexicographic::t = LC * tolerance;

	for (unsigned int i=0;i<Nf;++i)
	{
	    float *X = (float*) &DATA[i * 50 * sizeof (char)];
//	    printf("%g %g %g %g %g %g %g %g %g %g %g %g\n",X[0],X[1],X[2],X[3],X[4],X[5],X[6],X[7],X[8],X[9],X[10],X[11]);
	    BDS_Point P1 ( 0 , X[3]-XCG,  X[4]-YCG,  X[5]-ZCG);
	    BDS_Point P2 ( 0 , X[6]-XCG,  X[7]-YCG,  X[8]-ZCG);
	    BDS_Point P3 ( 0 , X[9]-XCG,  X[10]-YCG,  X[11]-ZCG);
	    BDS_Point *p1 = 0;
	    BDS_Point *p2 = 0;
	    BDS_Point *p3 = 0;
	    std::set < BDS_Point *, PointLessThanLexicographic >::iterator it1 = pts.find(&P1); 
	    if ( it1 != pts.end())
	    {
		p1 = *it1;
	    }
	    else
	    {
		MAXPOINTNUMBER++;
		p1 =  add_point (MAXPOINTNUMBER , P1.X, P1.Y,P1.Z );
		pts.insert (p1);
	    }
	    std::set < BDS_Point *, PointLessThanLexicographic >::iterator it2 = pts.find(&P2); 
	    if ( it2 != pts.end())
	    {
		p2 = *it2;
	    }
	    else
	    {
		MAXPOINTNUMBER++;
		p2 =  add_point (MAXPOINTNUMBER , P2.X, P2.Y,P2.Z );
		pts.insert (p2);
	    }
	    std::set < BDS_Point *, PointLessThanLexicographic >::iterator it3 = pts.find(&P3); 
	    if ( it3 != pts.end())
	    {
		p3 = *it3;
	    }
	    else
	    {
		MAXPOINTNUMBER++;
		p3 =  add_point (MAXPOINTNUMBER , P3.X, P3.Y,P3.Z );
		pts.insert (p3);
	    }		
	    add_triangle ( p1->iD, p2->iD, p3->iD); 
	}
	delete [] DATA;
    }
    fclose (f);    

    return true;
}

// INRIA FORMAT
bool BDS_Mesh :: read_mesh ( const char *filename )
{
  FILE *f = fopen ( filename, "r");
  if (!f)return false;
  char buffer [256], name[256];
  
  fgets (buffer,255,f);
  
  int format;
  
  sscanf (buffer,"%s %d",name,&format);
  // ASCII MESH  
  if (format == 1)
    {      

      printf("format = 1\n");
      while (!feof(f))
	{
	  fgets (buffer,255,f);
	  printf("%s\n",buffer);
	  if (buffer[0] != '#') // skip comments
	    {
	      sscanf(buffer,"%s",name);

	      if (!strcmp (name,"Dimension"))
		fgets (buffer,255,f);
	      else if (!strcmp (name,"Vertices"))
		{
		  Min[0] = Min[1] = Min[2] = 1.e12;
		  Max[0] = Max[1] = Max[2] = -1.e12;
		  int nbv,cl;
		  double x,y,z;
		  fgets (buffer,255,f);
		  sscanf (buffer,"%d",&nbv);
		  printf("%d Vertices\n",nbv);
		  for (int i=0;i<nbv;i++)
		    {
		      fgets (buffer,255,f);
		      sscanf (buffer,"%lf %lf %lf %d",&x,&y,&z,&cl);
		      Min[0] = (Min[0] < x)?Min[0]:x;
		      Min[1] = (Min[1] < y)?Min[1]:y;
		      Min[2] = (Min[2] < z)?Min[2]:z;
		      Max[0] = (Max[0] > x)?Max[0]:x;
		      Max[1] = (Max[1] > y)?Max[1]:y;
		      Max[2] = (Max[2] > z)?Max[2]:z;
		      add_point (i+1 , x,y,z );
		    }
		  MAXPOINTNUMBER = nbv+1;
		}
	      else if (!strcmp (name,"Triangles"))
		{

		  int nbt,cl,n1,n2,n3;
		  fgets (buffer,255,f);
		  sscanf (buffer,"%d",&nbt);
		  printf("%d Triangles\n",nbt);
		  for (int i=0;i<nbt;i++)
		    {
		      fgets (buffer,255,f);
		      sscanf (buffer,"%d %d %d %d",&n1,&n2,&n3,&cl);
		      BDS_Triangle *t = add_triangle(n1,n2,n3);
		      t->g = get_geom  (cl,2);
		      if (!t->g)
			{
			  add_geom (cl,2);
			  t->g = get_geom  (cl,2);
			}
		    }
		}
	      else if (!strcmp (name,"Quadrilaterals"))
		{
		  
		  int nbt,cl,n1,n2,n3,n4;
		  fgets (buffer,255,f);
		  sscanf (buffer,"%d",&nbt);
		  printf("%d Quadrilaterals\n",nbt);
		  for (int i=0;i<nbt;i++)
		    {
		      fgets (buffer,255,f);
		      sscanf (buffer,"%d %d %d %d %d",&n1,&n2,&n3,&n4,&cl);
		      BDS_Triangle *t1 = add_triangle(n1,n2,n3);
		      // pas 12
		      // pas 13
		      BDS_Triangle *t2 = add_triangle(n1,n3,n4);
		      t1->g = get_geom  (cl,2);
		      if (!t1->g)
			{
			  add_geom (cl,2);
			  t1->g = get_geom  (cl,2);
			}
		      t2->g = t1->g;
		    }
		}
	    }
	}
     
      LC = sqrt ((Min[0]-Max[0])*(Min[0]-Max[0])+
		 (Min[1]-Max[1])*(Min[1]-Max[1])+
		 (Min[2]-Max[2])*(Min[2]-Max[2]));
    }
  else
    {
      throw;
    }

  return true;
}

void BDS_Mesh :: save_gmsh_format ( const char *filename )
{
    cleanup();

    int nbModelVertex = 0;

    FILE *f = fopen ( filename, "w");
    {
	std::set<BDS_Point*,PointLessThan>::iterator it  = points.begin();
	std::set<BDS_Point*,PointLessThan>::iterator ite = points.end();
	
	fprintf(f,"$NOD\n");
	fprintf(f,"%d\n",(int)points.size());
	while(it!=ite)
	{
	    if ((*it)->g && (*it)->g->classif_degree == 0)nbModelVertex++;
	    fprintf(f,"%d %g %g %g\n",(*it)->iD,(*it)->X,(*it)->Y,(*it)->Z);
	    ++it;
	}
	fprintf(f,"$ENDNOD\n");
    }
    {
	fprintf(f,"$ELM\n");
	
	int nbClasEdges = 0;
	{
	    std::set<BDS_Edge*, EdgeLessThan>::iterator it  = edges.begin();
	    std::set<BDS_Edge*, EdgeLessThan>::iterator ite = edges.end();
	    while(it!=ite)
	    {
		if ((*it)->g && (*it)->g->classif_degree == 1)nbClasEdges++;
		++it;
	    }
	}
	
	fprintf(f,"%ld\n",nbClasEdges+nbModelVertex+triangles.size());
	
	int k=1;  
	{
	    std::set<BDS_Point*,PointLessThan>::iterator it  = points.begin();
	    std::set<BDS_Point*,PointLessThan>::iterator ite = points.end();
	    while(it!=ite)
	    {
		if ((*it)->g && (*it)->g->classif_degree == 0)
		{
		    fprintf(f,"%d %d %d %d %d %d\n",
			    k++,15,(*it)->g->classif_tag,(*it)->g->classif_tag,1,
			    (*it)->iD);
		}
		++it;
	    }
	}
	{
	    std::set<BDS_Edge*, EdgeLessThan>::iterator it  = edges.begin();
	    std::set<BDS_Edge*, EdgeLessThan>::iterator ite = edges.end();
	    while(it!=ite)
	    {
		if ((*it)->g && (*it)->g->classif_degree == 1)
		    fprintf(f,"%d %d %d %d %d %d %d\n",
			    k++,1,(*it)->g->classif_tag,(*it)->g->classif_tag,2,
			    (*it)->p1->iD, (*it)->p2->iD);
		++it;
		
	    }
	}
	{
	    std::list<BDS_Triangle*>::iterator it  = triangles.begin();
	    std::list<BDS_Triangle*>::iterator ite = triangles.end();
	    while(it!=ite)
	    {
		if ((*it)->deleted)
		{
		    printf("aahg\n");
		    throw;
		}
		BDS_Point *nod[3];
		(*it)->getNodes (nod);
		if ((*it)->g)
		    fprintf(f,"%d %d %d %d %d %d %d %d\n",
			    k++,2,(*it)->g->classif_tag,(*it)->g->classif_tag,3,
			nod[0]->iD,nod[1]->iD,nod[2]->iD);
		else
		    fprintf(f,"%d %d %d %d %d %d %d %d\n",
			    k++,2,1,1,3,
			    nod[0]->iD,nod[1]->iD,nod[2]->iD);
		++it;
		
	    }
	}
	fprintf(f,"$ENDELM\n");
    }
    fclose (f);
}

template <class IT>
void DESTROOOY ( IT beg , IT end)
{
    while (beg != end)
    {
	delete *beg;
	beg ++;
    }
}
void BDS_Mesh :: cleanup()
{
    for (std::list<BDS_Triangle*> :: iterator it = triangles.begin();
	 it != triangles.end();
	 it++)
    {
	while (it != triangles.end() && (*it)->deleted)
	{
	    delete *it;
	    it = triangles.erase (it);
	}
    }	    
    DESTROOOY ( edges_to_delete.begin(),edges_to_delete.end());
    edges_to_delete.clear();
}

BDS_Mesh ::~ BDS_Mesh ()
{
    DESTROOOY ( geom.begin(),geom.end());
    DESTROOOY ( points.begin(),points.end());
    DESTROOOY ( edges.begin(),edges.end());
    cleanup();
    DESTROOOY ( triangles.begin(),triangles.end());
}

bool BDS_Mesh ::split_edge ( BDS_Edge *e, double coord)
{


  /*

            p1
          / | \
         /  |  \
     op1/ 0mid1 \op2
        \   |   /
         \  |  /
          \ p2/

//  p1,op1,mid -
//  p2,op2,mid -
//  p2,op1,mid +
//  p1,op2,mid +

  */

    int nbFaces = e->numfaces();

    if (nbFaces != 2)return false;

    BDS_Point *op[2];
    BDS_Point *p1 = e->p1;
    BDS_Point *p2 = e->p2;

    BDS_Point *pts1[3];
    e->faces(0)->getNodes (pts1); 


    int orientation = 0;
    for (int i=0;i<3;i++)
      {
	if (pts1[i] == p1) 
	  {
	    if (pts1[(i+1)%3] == p2)
	      orientation = 1;
	    else
	      orientation = -1;

	    break;
	  }
      }


    MAXPOINTNUMBER++;
    BDS_Point *mid = add_point (MAXPOINTNUMBER , 
				coord * p1->X + (1-coord) * p2->X, 
				coord * p1->Y + (1-coord) * p2->Y, 
				coord * p1->Z + (1-coord) * p2->Z);

    // we should project 
    e->oppositeof (op);
    BDS_GeomEntity *g1=0,*g2=0,*ge=e->g;


    BDS_Edge *p1_op1 = find_edge ( p1, op[0], e->faces(0) );
    BDS_Edge *op1_p2 = find_edge ( op[0],p2,e->faces(0)  );
    BDS_Edge *p1_op2 = find_edge ( p1, op[1],e->faces(1) );
    BDS_Edge *op2_p2 = find_edge ( op[1],p2,e->faces(1)   );

    if (e->faces(0))
    {
	g1 = e->faces(0)->g;
	del_triangle (e->faces(0));
    }
// not a bug !!!
    if (e->faces(0))
    {
	g2 = e->faces(0)->g;
	del_triangle (e->faces(0));
    } 
    

    del_edge (e);


    BDS_Edge *p1_mid = new BDS_Edge ( p1, mid );
    edges.insert ( p1_mid );
    BDS_Edge *mid_p2 = new BDS_Edge ( mid, p2 );
    edges.insert ( mid_p2 );
    BDS_Edge *op1_mid = new BDS_Edge ( op[0], mid );
    edges.insert ( op1_mid );
    BDS_Edge *mid_op2 = new BDS_Edge ( mid, op[1] );
    edges.insert ( mid_op2 );

//    printf("split ends 1 %d (%d %d) %d %d \n",p1_op1->numfaces(), p1->iD, op[0]->iD, op1_mid->numfaces(),p1_mid->numfaces());
    BDS_Triangle *t1,*t2,*t3,*t4;
      if (orientation == 1)
      {
	t1 =  new BDS_Triangle ( op1_mid,  p1_op1,p1_mid );
	t2 =  new BDS_Triangle ( mid_op2, op2_p2, mid_p2 );
	t3 =  new BDS_Triangle ( op1_p2, op1_mid, mid_p2 );
	t4 =  new BDS_Triangle ( p1_op2, mid_op2, p1_mid );
      }
    else
      {
	t1 =  new BDS_Triangle (   p1_op1,op1_mid,p1_mid );
	t2 =  new BDS_Triangle (  op2_p2, mid_op2,mid_p2 );
	t3 =  new BDS_Triangle (  op1_mid,op1_p2, mid_p2 );
	t4 =  new BDS_Triangle (  mid_op2, p1_op2,p1_mid );
      }
    t1->g = g1;
    t2->g = g2;
    t3->g = g1;
    t4->g = g2;

    p1_mid->g = ge;
    mid_p2->g = ge;
    op1_mid->g = g1;
    mid_op2->g = g2;

    mid->g = ge;

    triangles.push_back(t1); 
    triangles.push_back (t2); 
    triangles.push_back (t3); 
    triangles.push_back (t4); 

    return true;
}

bool BDS_Mesh ::swap_edge ( BDS_Edge *e)
{

  /*

            p1
          / | \
         /  |  \
     op1/ 0 | 1 \op2
        \   |   /
         \  |  /
          \ p2/

    // op1 p1 op2
    // op1 op2 p2

  */


    if (e->deleted)return false;

    int nbFaces = e->numfaces();

    if (nbFaces != 2)return false;


    if (e->g && e->g->classif_degree != 2)return false;

    BDS_Point *op[2];
    BDS_Point *p1 = e->p1;
    BDS_Point *p2 = e->p2;
    e->oppositeof (op);
    BDS_GeomEntity *g1=0,*g2=0,*ge=e->g;

    BDS_Point *pts1[3];
    e->faces(0)->getNodes (pts1); 


    int orientation = 0;
    for (int i=0;i<3;i++)
      {
	if (pts1[i] == p1) 
	  {
	    if (pts1[(i+1)%3] == p2)
	      orientation = 1;
	    else
	      orientation = -1;

	    break;
	  }
      }

    BDS_Edge *p1_op1 = find_edge ( p1, op[0], e->faces(0) );
    BDS_Edge *op1_p2 = find_edge ( op[0],p2,e->faces(0)  );
    BDS_Edge *p1_op2 = find_edge ( p1, op[1],e->faces(1) );
    BDS_Edge *op2_p2 = find_edge ( op[1],p2,e->faces(1)   );

    if (e->faces(0))
    {
	g1 = e->faces(0)->g;
	del_triangle (e->faces(0));
    }
// not a bug !!!
    if (e->faces(0))
    {
	g2 = e->faces(0)->g;
	del_triangle (e->faces(0));
    } 
    del_edge (e);

    BDS_Edge *op1_op2 = new BDS_Edge ( op[0], op[1] );
    edges.insert ( op1_op2 );

    BDS_Triangle*t1,*t2;
    if (orientation == 1)
      {
	t1 =  new BDS_Triangle ( p1_op1, p1_op2, op1_op2  );
	t2 =  new BDS_Triangle ( op1_op2, op2_p2, op1_p2  );
      }
    else
      {
	t1 =  new BDS_Triangle (  p1_op2,p1_op1, op1_op2  );
	t2 =  new BDS_Triangle (  op2_p2,op1_op2, op1_p2  );
      }

    t1->g = g1;
    t2->g = g2;

    op1_op2->g = ge;

    triangles.push_back(t1); 
    triangles.push_back(t2); 
    return true;
}
bool BDS_Mesh ::collapse_edge ( BDS_Edge *e, BDS_Point *p, const double eps)
{

    if (e->numfaces() != 2)return false;
    if (p->g && p->g->classif_degree == 0)return false;
    if (e->g && e->g->classif_degree == 1)return false;

    std::list<BDS_Triangle *> t;
    BDS_Point *o = e->othervertex (p);

    if (!p->g)printf("argh1\n");
    if (!o->g)printf("argh2\n");
    if (!e->g)printf("argh3\n");

    if (o->g != p->g)return false;
    
    double X = p->X;
    double Y = p->Y;
    double Z = p->Z;

//    printf("collapsing an edge :");
//    print_edge(e);

    static int pt[3][1024];
    static BDS_GeomEntity *gs[1024];
    static int ept[2][1024];
    static BDS_GeomEntity *egs[1024];
    int nt=0;

    p->getTriangles (t); 	
    {
	std::list<BDS_Triangle *>::iterator it = t.begin();
	std::list<BDS_Triangle *>::iterator ite = t.end();

	while ( it != ite )
	{
	    BDS_Triangle *t = *it;
//	    print_face(t);
	    if (t->e1 != e && t->e2 != e && t->e3 != e)
	      {
		double n1[3],n2[3];
 		BDS_Point *pts[3];
		t->getNodes (pts); 
		p->X = o->X;
		p->Y = o->Y;
		p->Z = o->Z;
		double s_after = surface_triangle (pts[0],pts[1],pts[2]); 
		normal_triangle (pts[0],pts[1],pts[2], n1); // normal after collapse
		p->X = X;
		p->Y = Y;
		p->Z = Z;
		normal_triangle (pts[0],pts[1],pts[2], n2); // normal before collapse
		double s_before = surface_triangle (pts[0],pts[1],pts[2]); 
		// normals should not be opposed or change too dramatically
		// this does not concern the triangles with the small edge that
		// are collapsed too
		double ps = n1[0] * n2[0] + n1[1] * n2[1] + n1[2] * n2[2];
		if ( ps < eps ) return false;
		if (fabs (s_after - s_before) < 0.01 * fabs (s_after + s_before))return false; 
		gs[nt] = t->g;
		pt[0][nt]   = (pts[0] == p) ? o->iD : pts[0]->iD ;
		pt[1][nt]   = (pts[1] == p) ? o->iD : pts[1]->iD ;
		pt[2][nt++] = (pts[2] == p) ? o->iD : pts[2]->iD ;
	      }
	    ++it;
	}
    }


    {
	std::list<BDS_Triangle *>::iterator it = t.begin();
	std::list<BDS_Triangle *>::iterator ite = t.end();
	
	while ( it != ite )
	{
	    BDS_Triangle *t = *it;
	    del_triangle (t);
	    ++it;
	}
    }

//    printf("%d triangles 2 add\n",nt);

    int kk = 0;
    {
	std::list<BDS_Edge *>edges (p->edges);
	std::list<BDS_Edge *>::iterator eit  = edges.begin(); 	
	std::list<BDS_Edge *>::iterator eite = edges.end();
	while (eit!=eite)
	{
	    ept[0][kk] = ((*eit)->p1 == p)?o->iD:(*eit)->p1->iD;
	    ept[1][kk] = ((*eit)->p2 == p)?o->iD:(*eit)->p2->iD;
	    egs[kk++] = (*eit)->g;
	    del_edge (*eit);
	    ++eit;
	}
    }

    del_point (p);
    
    {
	for (int k=0;k<nt;k++)
	{
	    BDS_Triangle *t = add_triangle(pt[0][k],pt[1][k],pt[2][k]);
	    t->g = gs[k];
	}
    }

    for (int i=0;i<kk;++i)
    {
	BDS_Edge *e = find_edge (ept[0][i],ept[1][i]);
	if (e) e->g = egs[i];
    }


    return true;
}

bool BDS_Mesh ::smooth_point ( BDS_Point *p , BDS_Mesh *geom_mesh )
{

    if (p->g && p->g->classif_degree <= 1)return false;

    double X = 0;
    double Y = 0;
    double Z = 0;

    std::list<BDS_Edge*>::iterator eit  = p->edges.begin(); 

    while (eit!=p->edges.end())
    {
	BDS_Point *op = ((*eit)->p1 == p)?(*eit)->p2:(*eit)->p1; 	
	X+=op->X;
	Y+=op->Y;
	Z+=op->Z;
	++eit;
    }

    X /= p->edges.size();
    Y /= p->edges.size();
    Z /= p->edges.size();

    if (p->g->surf)
    {
	p->g->surf->projection ( X,Y,Z,p->X,p->Y,p->Z);
//	printf("coucou\n");
    }
    else
    {
	std::list<BDS_Triangle *> t; 	
	std::list<BDS_Triangle *>::iterator it ;
	std::list<BDS_Triangle *>::iterator ite;
	if (geom_mesh)
	{
	    it = geom_mesh->triangles.begin();
	    ite = geom_mesh->triangles.end();
	}
	else
	{
	    p->getTriangles (t); 
	    it = t.begin();	
	    ite = t.end();
	}
	
	double dist = 1.e22;
	double XX = X,YY=Y,ZZ=Z;
	
	int p_classif = p->g->classif_tag;
	
	while (it != ite)
	{ 
	    if ((*it)->g->classif_tag == p_classif)
	    {
		BDS_Point *pts[3];
		(*it)->getNodes (pts); 
		double xp,yp,zp;
		proj_point_plane ( X,Y,Z,pts[0],pts[1],pts[2],xp,yp,zp);
		double d = sqrt ((xp-X)*(xp-X)+(yp-Y)*(yp-Y)+(zp-Z)*(zp-Z));
		if (d < dist)
		{
		    XX = xp; YY = yp; ZZ = zp;
		    dist = d;
		}
	    }
	    ++it;
	}
	X = XX;
	Y = YY;
	Z = ZZ;
	
	p->X = X;
	p->Y = Y;
	p->Z = Z;
    }
    return true;
}
    
int BDS_Mesh :: adapt_mesh ( double l, bool smooth, BDS_Mesh *geom_mesh) 
{
    int nb_modif = 0;


    // add initial set of edges in a list
    std::set<BDS_Edge*, EdgeLessThan> small_to_long;
    {
	std::set<BDS_Edge*, EdgeLessThan>::iterator it = edges.begin();
	std::set<BDS_Edge*, EdgeLessThan>::iterator ite  = edges.end();
	while (it != ite)
	{
	  //	    if ((*it)->numfaces()==1)printf("one face\n");
	    small_to_long.insert (*it);  
	    ++it;
	}
    }
    // split edges
    {
      std::set<BDS_Edge*, EdgeLessThan>::iterator it = small_to_long.begin();
      std::set<BDS_Edge*, EdgeLessThan>::iterator ite  = small_to_long.end();
            
      while (it != ite)
	{
	  if ((*it)->numfaces()==2)
	    {
	      BDS_Point *op[2];
	      (*it)->oppositeof (op);
	      double ll = sqrt((op[0]->X-op[1]->X)*(op[0]->X-op[1]->X)+
			       (op[0]->Y-op[1]->Y)*(op[0]->Y-op[1]->Y)+
			       (op[0]->Z-op[1]->Z)*(op[0]->Z-op[1]->Z));
	      if (!(*it)->deleted && (*it)->length() > l/0.7 && ll > 0.25 * l){
		split_edge (*it, 0.5 );
		nb_modif++;
	      }
	    }
	  ++it;
	}
    }

    // re-create small_to_long
    cleanup();    
    {
	small_to_long.clear();
	std::set<BDS_Edge*, EdgeLessThan>::iterator it = edges.begin();
	std::set<BDS_Edge*, EdgeLessThan>::iterator ite  = edges.end();
	while (it != ite)
	{
	    small_to_long.insert (*it);  
	    ++it;
	}
    }
    // collapse 
    {    	
	std::set<BDS_Edge*, EdgeLessThan>::iterator it = small_to_long.begin();
	std::set<BDS_Edge*, EdgeLessThan>::iterator ite  = small_to_long.end();
	while (it != ite)
	{
	    if (!(*it)->deleted && (*it)->length() < 0.7 * l ){
		if (collapse_edge (*it, (*it)->p1, 0.1 ))
		    nb_modif++;
	    }
	    ++it;
	}
    }
    cleanup();  
    {
	small_to_long.clear();
	std::set<BDS_Edge*, EdgeLessThan>::iterator it = edges.begin();
	std::set<BDS_Edge*, EdgeLessThan>::iterator ite  = edges.end();
	while (it != ite)
	{
	    small_to_long.insert (*it);  
	    ++it;
	}
    }
    {    
	std::set<BDS_Edge*, EdgeLessThan>::iterator it = small_to_long.begin();
	std::set<BDS_Edge*, EdgeLessThan>::iterator ite  = small_to_long.end();
	while (it != ite)
	{
	    if (!(*it)->deleted && (*it)->numfaces()==2)
	    {
		BDS_Point *op[2];
		(*it)->oppositeof (op);
		
		double a1 = surface_triangle ( (*it)->p1 , (*it)->p2 , op[0] );
		double a2 = surface_triangle ( (*it)->p1 , (*it)->p2 , op[1] );
		double b1 = surface_triangle ( (*it)->p1 , op[0] , op[1] );
		double b2 = surface_triangle ( (*it)->p2 , op[0] , op[1] );
		
		double cb1[3],cb2[3];
		normal_triangle ( (*it)->p1 , op[0] , op[1],cb1 );
		normal_triangle ( (*it)->p2 , op[0] , op[1],cb2 );
		
		double prosc = cb1[0]*cb2[0]+cb1[1]*cb2[1]+cb1[2]*cb2[2];
		
		if (fabs(a1+a2-b1-b2) < 0.1 * (a1+a2) && prosc < 0 )//&& fabs (prosc) > 0.7)
		{
		    double qa1 = quality_triangle ( (*it)->p1 , (*it)->p2 , op[0] );
		    double qa2 = quality_triangle ( (*it)->p1 , (*it)->p2 , op[1] );
		    double qb1 = quality_triangle ( (*it)->p1 , op[0] , op[1] );
		    double qb2 = quality_triangle ( (*it)->p2 , op[0] , op[1] );
		    
		    double d = dist_droites_gauches((*it)->p1, (*it)->p2, 
						    op[0],op[1]);
		    BDS_Vector v1 (*((*it)->p1), *((*it)->p2));
		    BDS_Vector v2 (*(op[0]),*(op[1]));

		    double dd = sqrt (v1*v1 + v2*v2);

		    double qa = (qa1<qa2)?qa1:qa2; 
		    double qb = (qb1<qb2)?qb1:qb2; 
//		  printf("qa %g qb %g ..\n",qa,qb);
		    if (qb > qa && d < 0.05 * dd )
		    {
//		      printf("swop ..\n");
			nb_modif++;
			swap_edge ( *it );
		    }
		}
	    }
	    ++it;
	}
    }
    cleanup();  
    if (smooth)
    {    
	std::set<BDS_Point*, PointLessThan>::iterator it   = points.begin();
	std::set<BDS_Point*, PointLessThan>::iterator ite  = points.end();
	while (it != ite)
	{
	    smooth_point(*it,geom_mesh);
	    ++it;
	}
    }
    return nb_modif;
}


BDS_Mesh::BDS_Mesh (const BDS_Mesh &other)
{

    for (std::set<BDS_GeomEntity*,GeomLessThan>::const_iterator it = other.geom.begin();
	 it != other.geom.end();
	 ++it)
    {
	add_geom((*it)->classif_tag,(*it)->classif_degree);
	BDS_GeomEntity* g = get_geom((*it)->classif_tag,(*it)->classif_degree);
	g->surf = (*it)->surf;
    }
    for (std::set<BDS_Point*,PointLessThan>::const_iterator it  = other.points.begin();
	 it != other.points.end();
	 ++it)
    {
	BDS_Point *p =  add_point((*it)->iD,(*it)->X,(*it)->Y,(*it)->Z);
	p->g = ((*it)->g)? get_geom  ((*it)->g->classif_tag,(*it)->g->classif_degree) : 0;
    }
    for ( std::set<BDS_Edge*, EdgeLessThan>::const_iterator it  = other.edges.begin();
	  it != other.edges.end();
	  ++it)
    {
	BDS_Edge  * e = add_edge ((*it)->p1->iD,(*it)->p2->iD);
	e->g = ((*it)->g)? get_geom  ((*it)->g->classif_tag,(*it)->g->classif_degree) : 0;
    }
    for (std::list<BDS_Triangle*>::const_iterator it  = other.triangles.begin();
	 it != other.triangles.end();
	 ++it)
    {
	BDS_Point *n[3];
        (*it)->getNodes (n);
	BDS_Triangle *t = add_triangle(n[0]->iD,n[1]->iD,n[2]->iD);
	t->g = get_geom  ((*it)->g->classif_tag,(*it)->g->classif_degree);
    }
}
