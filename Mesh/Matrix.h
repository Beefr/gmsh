#ifndef _GMSH_MATRIX_
#define _GMSH_MATRIX_

#define _TAILLE_ 3

class Vector3
{
   double mat[_TAILLE_];
   double zero;
   public:
      Vector3 (const double init = 0)
      {
	zero = init;
	for(int i=0;i<_TAILLE_;i++)
	  mat[i] = zero;
      }
      Vector3 (const double& init, double z[3])
      {
	zero = init;
	for(int i=0;i<_TAILLE_;i++)
	  mat[i] = z[i];
      }
      Vector3 (const double& init, double x, double y, double z)
      {
	zero = init;
	mat[0] = x;
	mat[1] = y;
	mat[2] = z;
      }
      Vector3& operator = (const Vector3 &autre)
      {
	for(int i=0;i<_TAILLE_;i++)
	  mat[i] = autre.mat[i];
	return *this;
      }

      Vector3 operator * (double autre)
      {
	for(int i=0;i<_TAILLE_;i++)
	  mat[i] *= autre;
	return *this;
      }
      Vector3 operator + (const Vector3 &autre)
      {
	Vector3 m(0.);
	for(int i=0;i<_TAILLE_;i++)
	  m.mat[i] = mat[i] + autre.mat[i];
	return m;
      }
      Vector3 operator - (const Vector3 &autre)
      {
	Vector3 m(0.);
	for(int i=0;i<_TAILLE_;i++)
	  m.mat[i] = mat[i] - autre.mat[i];
	return m;
      }

      double & operator [] (int i)
	{
	  return mat[i];
	}

      double  length ()
	{
	  return sqrt(mat[0]*mat[0]+mat[1]*mat[1]+mat[2]*mat[2]);
	}

      double operator * (const Vector3 &autre)
	{

	  double m = zero;
	  for(int i=0;i<_TAILLE_;i++)
	    m += mat[i] * autre.mat[i];
	  return m;
      }
};

class Matrix3x3
{
   double mat[_TAILLE_][_TAILLE_];
   Vector3 eVec[_TAILLE_];
   double eVal[_TAILLE_];
   double zero;
   public:
      void print()
	{
	  /*
	  cout << "| " << mat[0][0] << " " << mat[0][1] << " " << mat[0][2] << "|"<<endl;
	  cout << "| " << mat[1][0] << " " << mat[1][1] << " " << mat[1][2] << "|"<<endl;
	  cout << "| " << mat[2][0] << " " << mat[2][1] << " " << mat[2][2] << "|"<<endl;
	  */
	}
      Matrix3x3 ()
      {
	zero = 0.0;
      }
      Matrix3x3 (Vector3 &v1, Vector3 &v2, Vector3 &v3)
      {
	setColumn(0,v1);
	setColumn(1,v2);
	setColumn(2,v3);
      }
      void getEigen(int i,Vector3 &v, double &l)
	{
	  l = eVal[i];
	  v = eVec[i];
	}
      void setEigen(int i, double v1, double v2, double v3, double l)
	{
	  (eVec[i])[0] = v1;
	  (eVec[i])[1] = v2;
	  (eVec[i])[2] = v3;
	  eVal[i] = l;
	}
      void setMetric ()
	{
	  Matrix3x3 rot (eVec[0],eVec[1],eVec[2]);
	  Matrix3x3 rotT (eVec[0],eVec[1],eVec[2]);
	  Matrix3x3 Id (0.0);
	  rotT.transpose();
	  Id.Identity(1.0);
	  for(int i=0;i<3;i++)Id.mat[i][i] = eVal[i];
	  Matrix3x3 m = rotT * (Id * rot);
	  for(int i=0;i<_TAILLE_;i++)
	    for(int j=0;j<_TAILLE_;j++)
	      mat[i][j] = m.mat[i][j];
	}

      Matrix3x3 (const double& init)
      {
            zero = init;
      		for(int i=0;i<_TAILLE_;i++)
      			for(int j=0;j<_TAILLE_;j++)
                	mat[i][j] = zero;
      }
      Matrix3x3 (const double& init, double z[3][3])
      {
	
	Identity(init);
	zero = init;
	for(int i=0;i<_TAILLE_;i++)
	  for(int j=0;j<_TAILLE_;j++)
	    mat[i][j] = z[i][j];
      }
      Matrix3x3 operator + (const Matrix3x3 &autre)
      {
      		Matrix3x3 m(0.);
      		for(int i=0;i<_TAILLE_;i++)
      			for(int j=0;j<_TAILLE_;j++)
                	m.mat[i][j] = mat[i][j] + autre.mat[i][j];
      		return m;
      }
      Matrix3x3 operator - (const Matrix3x3 &autre)
      {
            Matrix3x3 m(0.);
      		for(int i=0;i<_TAILLE_;i++)
      			for(int j=0;j<_TAILLE_;j++)
                	m.mat[i][j] = mat[i][j] - autre.mat[i][j];
      		return m;
      }
      double* operator [] (int i)
      {
             return mat[i];
      }
      void Identity(double id)
      {
	for(int i=0;i<_TAILLE_;i++)
	  mat[i][i] = id;
	setEigen(0,1,0,0,1);
	setEigen(1,0,1,0,1);
	setEigen(2,0,0,1,1);
      }

      void copy(double m[3][3])
      {

      		for(int i=0;i<_TAILLE_;i++)
      			for(int j=0;j<_TAILLE_;j++)
                    m[i][j] = mat[i][j];
      }

      Vector3 getColumn (int i)
	{
	  return Vector3 (0.0,mat[0][i],mat[1][i],mat[2][i]);
	}

      void setColumn (int i, Vector3 &col)
	{
	  mat[0][i] = col[0];
	  mat[1][i] = col[1];
	  mat[2][i] = col[2];
	}

      double get(int i, int j) const
	{
	  return mat[i][j];
	}
      void set(int i, int j, double k) 
	{
	  mat[i][j] = k;
	}

      Matrix3x3 operator * (const Matrix3x3 &autre)
      {

                Matrix3x3 m(0.);
      		for(int i=0;i<_TAILLE_;i++)
                for(int j=0;j<_TAILLE_;j++)
                {
                    m.mat[i][j] = zero;
                    for(int k=0;k<_TAILLE_;k++)
                    	m.mat[i][j] += mat[i][k] * autre.mat[k][j];
                }
      		return m;
      }
      /*
      bool invert ()
      {
            double det = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
            if(det == zero)return false;
            mat[0][0] = mat[1][1] / det;
            mat[1][1] = mat[0][0] / det;
            mat[1][0] = -mat[1][0] / det;
            mat[0][1] = -mat[0][1] / det;
      }
      */

      double quadraticFormEval (Vector3 & v)
	{
	  return v[0] * v[0] * mat[0][0] +
	    v[1] * v[1] * mat[1][1] +
	    v[2] * v[2] * mat[2][2] +
	    2. * v[0] * v[1] * mat[0][1] +
	    2. * v[0] * v[2] * mat[0][2] +
	    2. * v[1] * v[2] * mat[1][2];
	}

      void transpose()
      {
            double temp;
            for(int i=0;i<_TAILLE_;i++)
                for(int j=0;j<i;j++)
                {
                    if(i!=j)
                    {
                    	temp = mat[i][j];
                        mat[i][j] = mat[j][i];
                        mat[j][i] = temp;
                    }
                }
      }
};

#undef _TAILLE_
#endif
