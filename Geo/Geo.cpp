// $Id: Geo.cpp,v 1.25 2002-04-06 00:59:48 geuzaine Exp $

#include "Gmsh.h"
#include "Numeric.h"
#include "Geo.h"
#include "CAD.h"
#include "DataBase.h"
#include "Parser.h"
#include "Context.h"

extern Context_T CTX ;

#define BUFFSIZE 32000

// Ceci, c'est horrible. 

char x_text[100]  = "0.0", y_text[100]  = "0.0", z_text[100]  = "0.0";
char l_text[100] = "1.0" ;
char tx_text[100] = "0.0", ty_text[100] = "0.0", tz_text[100] = "0.0";
char attrx_text[100] = "1.0", attry_text[100] = "1.0", attrz_text[100] = "1.0" ;
char attrdec_text[100] = "2.0";
char px_text[100] = "0.0", py_text[100] = "0.0", pz_text[100] = "0.0" ;
char angle_text[100] = "3.14159/2" ;
char ax_text[100] = "0.0", ay_text[100] = "0.0", az_text[100] = "1.0";
char dx_text[100] = "0.0", dy_text[100] = "0.0", dz_text[100] = "0.0", df_text[100] = "1.0";
char sa_text[100] = "0.0", sb_text[100] = "0.0", sc_text[100] = "0.0", sd_text[100] = "0.0";
char trsf_pts_text[100] = "2", trsf_type_text[100] = "Progression 1.";
char trsf_vol_text[100] = "1";
char char_length_text[100] = "1.";

double evaluate_scalarfunction (char *var, double val, char *funct){
  FILE *tempf;
  tempf = yyin;
  
  if(!(yyin = fopen(CTX.tmp_filename,"w"))){
    Msg(GERROR, "Unable to open temporary file '%s'", CTX.tmp_filename);
    return 0.;
  }

  /* On pose la variable = la fonction et on evalue la fonction */

  fprintf(yyin,"%s = %.16g ;\n",var,val);
  fprintf(yyin,"ValeurTemporaire__ = %s ;\n",funct);
  fclose(yyin);
  yyin = fopen(CTX.tmp_filename,"r");
  while(!feof(yyin)){
    yyparse();
  }
  fclose(yyin);
  Symbol TheSymbol;
  TheSymbol.Name = (char*)malloc(100);
  strcpy(TheSymbol.Name,"ValeurTemporaire__");
  yyin = tempf;
  if (!List_Query(Symbol_L, &TheSymbol, CompareSymbols)) {
    free(TheSymbol.Name);
    return 0.0;
  }
  free(TheSymbol.Name);
  return *(double*)List_Pointer_Fast(TheSymbol.val,0);
}

void add_infile(char *text, char *fich){
  FILE *file;

  if(!(yyin = fopen(CTX.tmp_filename,"w"))){
    Msg(GERROR, "Unable to open temporary file '%s'", CTX.tmp_filename);
    return;
  }
  if(!(file = fopen(fich,"a"))){
    Msg(GERROR, "Unable to open file '%s'", fich);
    return;
  }
  fprintf(yyin,"%s\n",text);
  Msg(STATUS1,"%s",text);
  fclose(yyin);
  yyin = fopen(CTX.tmp_filename,"r");
  while(!feof(yyin)){
    yyparse();
  }
  fclose(yyin);
  fprintf(file,"%s\n",text);
  fclose(file);
  AddALineInTheEditGeometryForm (text);
}

void delet(int p1, char *fich, char *what){
  char text[BUFFSIZE];

  sprintf(text,"Delete {\n %s{%d};\n}",what,p1);
  add_infile(text,fich);
}

void add_trsfsurf (int N, int *l, char *fich){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i;
  sprintf(text,"Transfinite Surface {%d} = {",l[0]);
  for(i=1;i<N;i++){
    if(i==1)sprintf(text2,"%d",l[i]);
    else sprintf(text2,",%d",l[i]);
    strcat(text,text2);
  }
  sprintf(text2,"};");
  strcat(text,text2);
  add_infile(text,fich);
}

void add_ellipticsurf (int N, int *l, char *fich){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i;
  sprintf(text,"Elliptic Surface {%d} = {",l[0]);
  for(i=1;i<N;i++){
    if(i==1)sprintf(text2,"%d",l[i]);
    else sprintf(text2,",%d",l[i]);
    strcat(text,text2);
  }
  sprintf(text2,"};");
  strcat(text,text2);
  add_infile(text,fich);
}

void add_charlength (int N, int *l, char *fich){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i;
  sprintf(text,"Characteristic Length {");
  for(i=0;i<N;i++){
    if(i==0)sprintf(text2,"%d",l[i]);
    else sprintf(text2,",%d",l[i]);
    strcat(text,text2);
  }
  sprintf(text2,"} = %s;", char_length_text);
  strcat(text,text2);
  add_infile(text,fich);
}

void add_recosurf (int N, int *l, char *fich){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i;
  sprintf(text,"Recombine Surface {");
  for(i=0;i<N;i++){
    if(i==0)sprintf(text2,"%d",l[i]);
    else sprintf(text2,",%d",l[i]);
    strcat(text,text2);
  }
  sprintf(text2,"};");
  strcat(text,text2);
  add_infile(text,fich);
}


void add_trsfline (int N, int *l, char *fich){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i;
  sprintf(text,"Transfinite Line {");
  for(i=0;i<N;i++){
    if(!i)sprintf(text2,"%d",l[i]);
    else sprintf(text2,",%d",l[i]);
    strcat(text,text2);
  }
  if(strlen(trsf_type_text))
    sprintf(text2,"} = %s Using %s;", trsf_pts_text, trsf_type_text);
  else
    sprintf(text2,"} = %s;", trsf_pts_text);
  strcat(text,text2);
  add_infile(text,fich);
}


void add_param (char *par, char *value, char *fich){
  char text[BUFFSIZE];
  sprintf(text,"%s = %s;",par,value);
  add_infile(text,fich);
}

void add_point(char *fich){
  char text[BUFFSIZE];
  int ip;

  ip = NEWPOINT();
  sprintf(text,"Point(%d) = {%s,%s,%s,%s};",ip,x_text,y_text,z_text,l_text);
  add_infile(text,fich);
}

void add_attractor(char *fich, int ip, int typ){
  char text[BUFFSIZE];
  if(typ == 0) {
    sprintf(text,"Attractor Point {%d} = {%s,%s,%s} = ;",
            ip,attrx_text,attry_text,attrdec_text);
  }
  else if(typ == 1){
    sprintf(text,"Attractor Line {%d} = {%s,%s,%s};",
            ip, attrx_text,attry_text,attrdec_text);
  }
  else if(typ == 2) {
    sprintf(text,"Attractor Surface {%d} = {%s,%s,%s};",
            ip,attrx_text,attry_text,attrdec_text);
  }
  add_infile(text,fich);
}


void add_line(int p1, int p2, char *fich){
  char text[BUFFSIZE];
  int iseg;
  List_T *list = List_Create(2,2,sizeof(int));
  List_Add(list,&p1);
  List_Add(list,&p2);
  if((recognize_seg(MSH_SEGM_LINE,list,&iseg))){
    List_Delete(list);
    return;
  }
  List_Delete(list);
  
  sprintf(text,"Line(%d) = {%d,%d};",NEWLINE(),p1,p2);
  add_infile(text,fich);
}

void add_circ(int p1, int p2, int p3, char *fich){
  char text[BUFFSIZE];

  sprintf(text,"Circle(%d) = {%d,%d,%d};",NEWLINE(),p1,p2,p3);
  add_infile(text,fich);
}

void add_ell(int p1, int p2, int p3, int p4, char *fich){
  char text[BUFFSIZE];

  sprintf(text,"Ellipse(%d) = {%d,%d,%d,%d};",NEWLINE(),p1,p2,p3,p4);
  add_infile(text,fich);
}

void add_spline(int N, int *p, char *fich){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i;

  sprintf(text,"CatmullRom(%d) = {",NEWLINE());
  for(i=0;i<N;i++){
    if(i != N-1)
      sprintf(text2,"%d,",p[i]);
    else
      sprintf(text2,"%d};",p[i]);
    strcat(text,text2);
  }
  add_infile(text,fich);
}

void add_bezier(int N, int *p, char *fich){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i;

  sprintf(text,"Bezier(%d) = {",NEWLINE());
  for(i=0;i<N;i++){
    if(i != N-1)
      sprintf(text2,"%d,",p[i]);
    else
      sprintf(text2,"%d};",p[i]);
    strcat(text,text2);
  }
  add_infile(text,fich);
}


void add_bspline(int N, int *p, char *fich){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i;

  sprintf(text,"BSpline(%d) = {",NEWLINE());
  for(i=0;i<N;i++){
    if(i != N-1)
      sprintf(text2,"%d,",p[i]);
    else
      sprintf(text2,"%d};",p[i]);
    strcat(text,text2);
  }
  add_infile(text,fich);
}

void add_multline(int N, int *p, char *fich){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i;

  int iseg;
  List_T *list = List_Create(N,2,sizeof(int));
  for(i=0;i<N;i++)
    List_Add(list,&p[i]);
  if((recognize_seg(MSH_SEGM_LINE,list,&iseg))){
    List_Delete(list);
    return;
  }
  List_Delete(list);

  sprintf(text,"Line(%d) = {",NEWLINE());
  for(i=0;i<N;i++){
    if(i != N-1)
      sprintf(text2,"%d,",p[i]);
    else
      sprintf(text2,"%d};",p[i]);
    strcat(text,text2);
  }
  add_infile(text,fich);
}

void add_loop(List_T *list, char *fich, int *numloop){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i,seg;

  if((recognize_loop(list,numloop))) return;

  *numloop = NEWLINELOOP();
  sprintf(text,"Line Loop(%d) = {",*numloop);
  for(i=0;i<List_Nbr(list);i++){
      List_Read(list,i,&seg);
    if(i != List_Nbr(list)-1)
      sprintf(text2,"%d,",seg);
    else
      sprintf(text2,"%d};",seg);
    strcat(text,text2);
  }
  add_infile(text,fich);
}


void add_surf(List_T *list, char *fich, int support, int typ){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i,seg;

  if(typ ==1){
    sprintf(text,"Ruled Surface(%d) = {",NEWSURFACE());
  }
  else if (typ == 2){
    sprintf(text,"Plane Surface(%d) = {",NEWSURFACE());
  }
  else
  {
    sprintf(text,"Trimmed Surface(%d) = %d {",NEWSURFACE(),support);
  }
  for(i=0;i<List_Nbr(list);i++){
      List_Read(list,i,&seg);
    if(i != List_Nbr(list)-1)
      sprintf(text2,"%d,",seg);
    else
      sprintf(text2,"%d};",seg);
    strcat(text,text2);
  }
  add_infile(text,fich);
}

void add_vol(List_T *list, char *fich, int *numvol){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i,seg;

  if((recognize_surfloop(list,numvol))) return;

  *numvol = NEWSURFACELOOP();
  sprintf(text,"Surface Loop(%d) = {",*numvol);
  for(i=0;i<List_Nbr(list);i++){
    List_Read(list,i,&seg);
    if(i != List_Nbr(list)-1)
      sprintf(text2,"%d,",seg);
    else
      sprintf(text2,"%d};",seg);
    strcat(text,text2);
  }
  add_infile(text,fich);
}

void add_multvol(List_T *list, char *fich){
  char text[BUFFSIZE];
  char text2[BUFFSIZE];
  int i,seg;

  sprintf(text,"Volume(%d) = {",NEWVOLUME());
  for(i=0;i<List_Nbr(list);i++){
    List_Read(list,i,&seg);
    if(i != List_Nbr(list)-1)
      sprintf(text2,"%d,",seg);
    else
      sprintf(text2,"%d};",seg);
    strcat(text,text2);
  }
  add_infile(text,fich);
}

void add_trsfvol(int N, int *l, char *fich){
  char text[BUFFSIZE], text2[BUFFSIZE];
  int i;

  sprintf(text,"Transfinite Volume{%s} = {", trsf_vol_text);
  for(i=0;i<N;i++){
    if(i==0)sprintf(text2,"%d",l[i]);
    else sprintf(text2,",%d",l[i]);
    strcat(text,text2);
  }
  sprintf(text2,"};");
  strcat(text,text2);
  add_infile(text,fich);
}


void add_physical(List_T *list, char *fich, int type, int *num){
  char text[BUFFSIZE], text2[BUFFSIZE];
  int  i, elementary_entity;

  *num = NEWPHYSICAL();
  switch(type){
  case ENT_POINT : sprintf(text, "Physical Point(%d) = {", *num); break;
  case ENT_LINE : sprintf(text, "Physical Line(%d) = {", *num); break;
  case ENT_SURFACE : sprintf(text, "Physical Surface(%d) = {", *num); break;
  case ENT_VOLUME : sprintf(text, "Physical Volume(%d) = {", *num); break;
  }

  for(i=0; i<List_Nbr(list); i++){
    List_Read(list, i, &elementary_entity);
    if(i != List_Nbr(list)-1)
      sprintf(text2, "%d,", elementary_entity);
    else
      sprintf(text2, "%d};", elementary_entity);
    strcat(text, text2);
  }
  add_infile(text, fich);
}

void translate(int add, int s, char *fich, char *what){
  char text[BUFFSIZE];

  if(add)
    sprintf(text,"Translate {%s,%s,%s} {\n  Duplicata { %s{%d}; }\n}",
            tx_text,ty_text,tz_text,what,s);
  else
    sprintf(text,"Translate {%s,%s,%s} {\n  %s{%d};\n}",
            tx_text,ty_text,tz_text,what,s);
  add_infile(text,fich);
}
void rotate(int add, int s, char *fich, char *quoi){
  char text[BUFFSIZE];

  if(add)
    sprintf(text,"Rotate { {%s,%s,%s},{%s,%s,%s},%s } {\n  Duplicata { %s{%d}; }\n}",
            ax_text,ay_text,az_text,px_text,py_text,pz_text,angle_text, quoi,s);
  else
    sprintf(text,"Rotate { {%s,%s,%s},{%s,%s,%s},%s } {\n   %s{%d};\n  }",
            ax_text,ay_text,az_text,px_text,py_text,pz_text,angle_text, quoi,s);
  add_infile(text,fich);
}
void dilate(int add, int s, char *fich, char *quoi){
  char text[BUFFSIZE];

  if(add)
    sprintf(text,"Dilate { {%s,%s,%s},%s } {\n  Duplicata { %s{%d}; }\n}",
            dx_text,dy_text,dz_text,df_text, quoi,s);
  else
    sprintf(text,"Dilate { {%s,%s,%s},%s } {\n   %s{%d};\n  }",
            dx_text,dy_text,dz_text,df_text, quoi,s);
  add_infile(text,fich);
}
void symmetry(int add, int s, char *fich, char *quoi){
  char text[BUFFSIZE];

  if(add)
    sprintf(text,"Symmetry { %s,%s,%s,%s } {\n  Duplicata { %s{%d}; }\n}",
            sa_text,sb_text,sc_text,sd_text, quoi,s);
  else
    sprintf(text,"Symmetry { %s,%s,%s,%s } {\n   %s{%d};\n  }",
            sa_text,sb_text,sc_text,sd_text, quoi,s);
  add_infile(text,fich);

}
void extrude(int s, char *fich, char *what){
  char text[BUFFSIZE];

  sprintf(text,"Extrude %s {%d, {%s,%s,%s}};",what,s,tx_text,ty_text,tz_text);
  add_infile(text,fich);
}
void protude(int s, char *fich, char *what){
  char text[BUFFSIZE];

  sprintf(text,"Extrude %s {%d, {%s,%s,%s}, {%s,%s,%s}, %s};",what,s,ax_text,ay_text,
          az_text,px_text,py_text,pz_text,angle_text);
  add_infile(text,fich);
}
