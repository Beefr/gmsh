#ifndef _GEO_H_
#define _GEO_H_

#define ELEMENTARY 1
#define PHYSICAL   2

#define INFILE     1
#define INSTRING   2
#define STRINGMAX  1024

#define NMAX_SPLINE   100

#define ENT_POINT    1
#define ENT_LINE     2
#define ENT_SURFACE  3
#define ENT_VOLUME   4

#define MSH_TRSF_LINE        1
#define MSH_TRSF_SURFACE     2
#define MSH_TRSF_VOLUME      3

#define MSH_ASSOCIATION      6

#define MSH_RECOMBINE        5

#define MSH_POINT            1

#define MSH_SEGM_LINE        2
#define MSH_SEGM_SPLN        3
#define MSH_SEGM_CIRC        4
#define MSH_SEGM_CIRC_INV    5
#define MSH_SEGM_ELLI        6
#define MSH_SEGM_ELLI_INV    7
#define MSH_SEGM_LOOP        8
#define MSH_SEGM_PARAMETRIC  888
#define MSH_SEGM_MERGEDSEGS  889

#define MSH_SURF_PLAN        9
#define MSH_SURF_REGL        10
#define MSH_SURF_TRIC        11
#define MSH_SURF_NURBS       12
#define MSH_SURF_LOOP        13
#define MSH_SURF_CYLNDR      1299
#define MSH_SURF_TORUS       1399
#define MSH_SURF_CONE        1499
#define MSH_SURF_TRIMMED     1599
#define MSH_SURF_STL         1599

#define MSH_VOLUME                   14
#define MSH_SEGM_BSPLN       15
#define MSH_SEGM_URBS        16
#define MSH_SEGM_NURBS       17
#define MSH_SEGM_BEZIER      18

#define MSH_PHYSICAL_POINT   300
#define MSH_PHYSICAL_LINE    310
#define MSH_PHYSICAL_SURFACE 320
#define MSH_PHYSICAL_VOLUME  330

typedef struct {
  int Type;
  int Num;
  union {
    int I;
    double F;
    double V[4];
    List_T *ListDouble;
  } obj;
} Shape;

// static strings for parser interaction
extern char x_text[100], y_text[100], z_text[100];
extern char l_text[100];
extern char tx_text[100], ty_text[100], tz_text[100];
extern char attrx_text[100], attry_text[100], attrz_text[100];
extern char attrdec_text[100];
extern char px_text[100], py_text[100], pz_text[100];
extern char angle_text[100] ;
extern char ax_text[100], ay_text[100], az_text[100];
extern char dx_text[100], dy_text[100], dz_text[100], df_text[100];
extern char sa_text[100], sb_text[100], sc_text[100], sd_text[100];
extern char nb_pts[100], mode_value[100];
extern char trsf_pts_text[100], trsf_type_text[100];
extern char trsf_vol_text[100];
extern char char_length_text[100];

double evaluate_scalarfunction (char *var, double val, char *funct);

void delet(int p1, char *fich, char *what);
void add_infile(char *text, char *fich);
void add_trsfsurf (int N, int *l, char *fich);
void add_trsfvol (int N, int *l, char *fich);
void add_ellipticsurf (int N, int *l, char *fich);
void add_charlength (int N, int *l, char *fich);
void add_recosurf (int N, int *l, char *fich);
void add_trsfline (int N, int *l, char *fich);
void add_param (char *par, char *value, char *fich);
void add_point(char *fich);
void add_attractor(char *fich, int ip, int typ);
void add_line(int p1, int p2, char *fich);
void add_circ(int p1, int p2, int p3, char *fich);
void add_ell(int p1, int p2, int p3, int p4, char *fich);
void add_spline(int N, int *p, char *fich);
void add_bezier(int N, int *p, char *fich);
void add_bspline(int N, int *p, char *fich);
void add_multline(int N, int *p, char *fich);
void add_loop(List_T *list, char *fich, int *numloop);
void add_surf(List_T *list, char *fich, int support, int typ);
void add_vol(List_T *list, char *fich, int *numvol);
void add_multvol(List_T *list, char *fich);
void add_physical(List_T *list, char *fich, int type, int *num);
void translate(int add, int s, char *fich, char*what);
void rotate(int add, int s, char *fich, char *what);
void dilate(int add, int s, char *fich, char *what);
void symmetry(int add, int s, char *fich, char *what);
void extrude(int s, char *fich, char *what);
void protude(int s, char *fich, char *what);

#endif
