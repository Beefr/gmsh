/* $Id: Context.cpp,v 1.19 2000-12-09 17:33:39 geuzaine Exp $ */

#include "Gmsh.h"
#include "Const.h"
#include "Geo.h"
#include "Mesh.h"
#include "Draw.h"
#include "Context.h"
#include "trackball.c"

extern Context_T CTX ;

extern StringXString GeneralOptions_String[] ;
extern StringXString GeometryOptions_String[] ;
extern StringXString MeshOptions_String[] ;
extern StringXString PostProcessingOptions_String[] ;
extern StringXString PrintOptions_String[] ;

extern StringXNumber GeneralOptions_Number[] ;
extern StringXNumber GeometryOptions_Number[] ;
extern StringXNumber MeshOptions_Number[] ;
extern StringXNumber PostProcessingOptions_Number[] ;
extern StringXNumber PrintOptions_Number[] ;

extern StringXArray GeneralOptions_Array[] ;
extern StringXArray GeometryOptions_Array[] ;
extern StringXArray MeshOptions_Array[] ;
extern StringXArray PostProcessingOptions_Array[] ;
extern StringXArray PrintOptions_Array[] ;

extern StringXColor GeneralOptions_Color[] ;
extern StringXColor GeometryOptions_Color[] ;
extern StringXColor MeshOptions_Color[] ;
extern StringXColor PostProcessingOptions_Color[] ;
extern StringXColor PrintOptions_Color[] ;


// STRING OPTIONS

StringXString * Get_StringOptionCategory(char * cat){
  if     (!strcmp(cat,"General"))        return GeneralOptions_String ;
  else if(!strcmp(cat,"Geometry"))       return GeometryOptions_String ;
  else if(!strcmp(cat,"Mesh"))           return MeshOptions_String ;
  else if(!strcmp(cat,"PostProcessing")) return PostProcessingOptions_String ;
  else if(!strcmp(cat,"Print"))          return PrintOptions_String ;
  else                                   return NULL ;
}

void Set_DefaultStringOptions(StringXString s[]){
  int i = 0;
  while(s[i].str){
    *s[i].ptr = s[i].def ;
    i++;
  }
}

char ** Get_StringOption(char *str, StringXString s[]){
  int i = 0;
  while ((s[i].str != NULL) && (strcmp(s[i].str, str))) i++ ;
  if(!s[i].str)
    return NULL;
  else
    return s[i].ptr;
}

void Print_StringOptions(StringXString s[], char *prefix, FILE *file){
  int i = 0;
  while(s[i].str){
    fprintf(file, "%s%s = \"%s\";\n", prefix, s[i].str, *s[i].ptr) ;
    i++;
  }
}

// NUMBER OPTIONS

StringXNumber * Get_NumberOptionCategory(char * cat){
  if     (!strcmp(cat,"General"))        return GeneralOptions_Number ;
  else if(!strcmp(cat,"Geometry"))       return GeometryOptions_Number ;
  else if(!strcmp(cat,"Mesh"))           return MeshOptions_Number ;
  else if(!strcmp(cat,"PostProcessing")) return PostProcessingOptions_Number ;
  else if(!strcmp(cat,"Print"))          return PrintOptions_Number ;
  else                                   return NULL ;
}

void Set_DefaultNumberOptions(StringXNumber s[]){
  int i = 0;
  while(s[i].str){
    switch(s[i].type){
    case GMSH_INT: *(int*)s[i].ptr = (int)s[i].def; break;
    case GMSH_LONG: *(long*)s[i].ptr = (long)s[i].def; break;
    case GMSH_FLOAT: *(float*)s[i].ptr = (float)s[i].def; break;
    case GMSH_DOUBLE: *(double*)s[i].ptr = (double)s[i].def; break;
    }
    i++;
  }
}

void * Get_NumberOption(char *str, StringXNumber s[], int *type){
  int i = 0;

  while ((s[i].str != NULL) && (strcmp(s[i].str, str))) i++ ;
  if(!s[i].str)
    return NULL;
  else{
    *type = s[i].type ;
    return s[i].ptr;
  }
}

void Print_NumberOptions(StringXNumber s[], char *prefix, FILE *file){
  int i = 0;
  while(s[i].str){
    fprintf(file, "%s%s = ", prefix, s[i].str);
    switch(s[i].type){
    case GMSH_INT : fprintf(file, "%d;\n", *(int*)s[i].ptr); break;
    case GMSH_LONG : fprintf(file, "%ld;\n", *(long*)s[i].ptr); break;
    case GMSH_FLOAT : fprintf(file, "%g;\n", *(float*)s[i].ptr); break;
    case GMSH_DOUBLE : fprintf(file, "%g;\n", *(double*)s[i].ptr); break;
    }
    i++;
  }
}


// ARRAY OPTIONS

StringXArray * Get_ArrayOptionCategory(char * cat){
  if     (!strcmp(cat,"General"))        return GeneralOptions_Array ;
  else if(!strcmp(cat,"Geometry"))       return GeometryOptions_Array ;
  else if(!strcmp(cat,"Mesh"))           return MeshOptions_Array ;
  else if(!strcmp(cat,"PostProcessing")) return PostProcessingOptions_Array ;
  else if(!strcmp(cat,"Print"))          return PrintOptions_Array ;
  else                                   return NULL ;
}

void Set_DefaultArrayOptions(StringXArray s[]){
  int i = 0;
  
  while(s[i].str){
    switch(s[i].type){
    case GMSH_INT:
      ((int*)s[i].ptr)[0] = (int)s[i].def1;
      ((int*)s[i].ptr)[1] = (int)s[i].def2;
      ((int*)s[i].ptr)[2] = (int)s[i].def3;
      ((int*)s[i].ptr)[3] = (int)s[i].def4;
      break;
    case GMSH_LONG:
      ((long*)s[i].ptr)[0] = (long)s[i].def1;
      ((long*)s[i].ptr)[1] = (long)s[i].def2;
      ((long*)s[i].ptr)[2] = (long)s[i].def3;
      ((long*)s[i].ptr)[3] = (long)s[i].def4;
      break;
    case GMSH_FLOAT:
      ((float*)s[i].ptr)[0] = (float)s[i].def1;
      ((float*)s[i].ptr)[1] = (float)s[i].def2;
      ((float*)s[i].ptr)[2] = (float)s[i].def3;
      ((float*)s[i].ptr)[3] = (float)s[i].def4;
      break;
    case GMSH_DOUBLE:
      ((double*)s[i].ptr)[0] = (double)s[i].def1;
      ((double*)s[i].ptr)[1] = (double)s[i].def2;
      ((double*)s[i].ptr)[2] = (double)s[i].def3;
      ((double*)s[i].ptr)[3] = (double)s[i].def4;
      break;
    }
    i++;
  }
}

void * Get_ArrayOption(char *str, StringXArray s[], int *type){
  int i = 0;
  while ((s[i].str != NULL) && (strcmp(s[i].str, str))) i++ ;
  if(!s[i].str)
    return NULL;
  else{
    *type = s[i].type ;
    return s[i].ptr;
  }
}

void Print_ArrayOptions(StringXArray s[], char *prefix, FILE *file){
  int i = 0;
  while(s[i].str){
    fprintf(file, "%s%s = {", prefix, s[i].str);
    switch(s[i].type){
    case GMSH_INT : 
      fprintf(file, "%d,%d,%d,%d};\n", 
	      ((int*)s[i].ptr)[0], ((int*)s[i].ptr)[1], 
	      ((int*)s[i].ptr)[2], ((int*)s[i].ptr)[3]); 
      break;
    case GMSH_LONG : 
      fprintf(file, "%ld,%ld,%ld,%ld};\n", 
	      ((long*)s[i].ptr)[0], ((long*)s[i].ptr)[1], 
	      ((long*)s[i].ptr)[2], ((long*)s[i].ptr)[3]); 
      break;
    case GMSH_FLOAT : 
      fprintf(file, "%g,%g,%g,%g};\n", 
	      ((float*)s[i].ptr)[0], ((float*)s[i].ptr)[1], 
	      ((float*)s[i].ptr)[2], ((float*)s[i].ptr)[3]); 
      break;
    case GMSH_DOUBLE : 
      fprintf(file, "%g,%g,%g,%g};\n", 
	      ((double*)s[i].ptr)[0], ((double*)s[i].ptr)[1], 
	      ((double*)s[i].ptr)[2], ((double*)s[i].ptr)[3]); 
      break;
    }
    i++;
  }
}


// COLORS

StringXColor * Get_ColorOptionCategory(char * cat){
  if     (!strcmp(cat,"General"))        return GeneralOptions_Color ;
  else if(!strcmp(cat,"Geometry"))       return GeometryOptions_Color ;
  else if(!strcmp(cat,"Mesh"))           return MeshOptions_Color ;
  else if(!strcmp(cat,"PostProcessing")) return PostProcessingOptions_Color ;
  else if(!strcmp(cat,"Print"))          return PrintOptions_Color ;
  else                                   return NULL ;
}

void Set_DefaultColorOptions(StringXColor s[], int num){
  int i = 0;
  while(s[i].str){
    switch(num){
    case 0 : *s[i].ptr = s[i].def1; break;
    case 1 : *s[i].ptr = s[i].def2; break;
    case 2 : *s[i].ptr = s[i].def3; break;
    }
    i++;
  }
}

unsigned int * Get_ColorOption(char *str, StringXColor s[]) {
  int i = 0;
  while ((s[i].str != NULL) && (strcmp(s[i].str, str))) i++ ;
  if(!s[i].str)
    return NULL;
  else
    return s[i].ptr;
}

void Print_ColorOptions(StringXColor s[], char *prefix, FILE *file){
  int i = 0;
  while(s[i].str){
    fprintf(file, "%sColor.%s = {%d,%d,%d};\n", 
	    prefix, s[i].str,
	    UNPACK_RED(*s[i].ptr),
	    UNPACK_GREEN(*s[i].ptr),
	    UNPACK_BLUE(*s[i].ptr));
    i++;
  }
}

int Get_ColorForString(StringX4Int SX4I[], int alpha, 
		       char * str, int * FlagError) {
  int  i = 0 ;
  while ((SX4I[i].str != NULL) && (strcmp(SX4I[i].str, str)))  i++ ;
  *FlagError = (SX4I[i].str == NULL)? 1 : 0 ;
  if(alpha > 0)
    return PACK_COLOR(SX4I[i].int1,SX4I[i].int2,SX4I[i].int3,alpha) ;
  else
    return PACK_COLOR(SX4I[i].int1,SX4I[i].int2,SX4I[i].int3,SX4I[i].int4) ;
}


/* ------------------------------------------------------------------------ */
/*  C o n t e x t                                                           */
/* ------------------------------------------------------------------------ */

void Init_Colors(int num){
  if(num < 0 || num > 2){
    Msg(ERROR, "Unknown Default Color Scheme");
    return;
  }
  CTX.color.id = num ;
  Set_DefaultColorOptions(GeneralOptions_Color, num);
  Set_DefaultColorOptions(GeometryOptions_Color, num);
  Set_DefaultColorOptions(MeshOptions_Color, num);
  Set_DefaultColorOptions(PostProcessingOptions_Color, num);
  Set_DefaultColorOptions(PrintOptions_Color, num);
}

void Init_Context(void){

  // Cannot be set by the user 
  CTX.expose       = 0 ;
  CTX.db           = 1 ;
  CTX.overlay      = 1 ;
  CTX.stream       = TO_SCREEN ;
  CTX.command_win  = 0 ;
  CTX.threads      = 1 ; // effective on Unix only with -D_USETHREADS
  CTX.threads_lock = 0 ;

  CTX.font = "-*-helvetica-medium-r-*-*-*-*-*-*-*-*-*-*" ;
  CTX.fixed_font = "fixed" ;

  CTX.min[0]   = CTX.min[1]   = CTX.min[2]   = 0.0 ;
  CTX.max[0]   = CTX.max[1]   = CTX.max[2]   = 1.0 ;
  CTX.range[0] = CTX.range[1] = CTX.range[2] = 1.0 ;

  CTX.viewport[0] = CTX.viewport[1] = 0 ;
  CTX.viewport[2] = CTX.viewport[3] = 1 ;

  CTX.vxmin = CTX.vymin = CTX.vxmax = CTX.vymax = 0. ;

  CTX.render_mode    = GMSH_RENDER ;
  CTX.pixel_equiv_x  = CTX.pixel_equiv_y = 0. ; 
  CTX.geom.vis_type  = 0 ;
  CTX.geom.level     = ELEMENTARY ;
  CTX.mesh.vis_type  = 0 ;
  CTX.mesh.draw      = 1 ;  
  CTX.post.draw      = 1 ;

  // Default string options
  Set_DefaultStringOptions(GeneralOptions_String);
  Set_DefaultStringOptions(GeometryOptions_String);
  Set_DefaultStringOptions(MeshOptions_String);
  Set_DefaultStringOptions(PostProcessingOptions_String);
  Set_DefaultStringOptions(PrintOptions_String);

  // Default number options
  Set_DefaultNumberOptions(GeneralOptions_Number);
  Set_DefaultNumberOptions(GeometryOptions_Number);
  Set_DefaultNumberOptions(MeshOptions_Number);
  Set_DefaultNumberOptions(PostProcessingOptions_Number);
  Set_DefaultNumberOptions(PrintOptions_Number);

  // Default array options
  Set_DefaultArrayOptions(GeneralOptions_Array);
  Set_DefaultArrayOptions(GeometryOptions_Array);
  Set_DefaultArrayOptions(MeshOptions_Array);
  Set_DefaultArrayOptions(PostProcessingOptions_Array);
  Set_DefaultArrayOptions(PrintOptions_Array);

  // Default color options
  Init_Colors(0);

}

extern List_T  *Post_ViewList;

void Print_Context(char *filename){
  FILE *file;
  int i ;
  
  if(filename){
    file = fopen(filename,"w");
    if(!file){
      Msg(WARNING, "Unable to Open File '%s'", filename);
      return;
    }
  }
  else
    file = stdout;

  Print_StringOptions(GeneralOptions_String, "General.", file);
  Print_NumberOptions(GeneralOptions_Number, "General.", file);
  Print_ArrayOptions(GeneralOptions_Array, "General.", file);
  Print_ColorOptions(GeneralOptions_Color, "General.", file);
  fprintf(file, "\n");
  Print_StringOptions(GeometryOptions_String, "Geometry.", file);
  Print_NumberOptions(GeometryOptions_Number, "Geometry.", file);
  Print_ArrayOptions(GeometryOptions_Array, "Geometry.", file);
  Print_ColorOptions(GeometryOptions_Color, "Geometry.", file);
  fprintf(file, "\n");
  Print_StringOptions(MeshOptions_String, "Mesh.", file);
  Print_NumberOptions(MeshOptions_Number, "Mesh.", file);
  Print_ArrayOptions(MeshOptions_Array, "Mesh.", file);
  Print_ColorOptions(MeshOptions_Color, "Mesh.", file);
  fprintf(file, "\n");
  Print_StringOptions(PostProcessingOptions_String, "PostProcessing.", file);
  Print_NumberOptions(PostProcessingOptions_Number, "PostProcessing.", file);
  Print_ArrayOptions(PostProcessingOptions_Array, "PostProcessing.", file);
  Print_ColorOptions(PostProcessingOptions_Color, "PostProcessing.", file);
  for(i=0; i<List_Nbr(Post_ViewList) ; i++){
    Print_StringViewOptions(i, file);
    Print_NumberViewOptions(i, file);
    Print_ArrayViewOptions(i, file);
  }
  fprintf(file, "\n");
  Print_StringOptions(PrintOptions_String, "Print.", file);
  Print_NumberOptions(PrintOptions_Number, "Print.", file);
  Print_ArrayOptions(PrintOptions_Array, "Print.", file);
  Print_ColorOptions(PrintOptions_Color, "Print.", file);

  if(filename){
    Msg (INFOS, "Options Output Complete '%s'", filename);
    Msg (INFO, "Wrote File '%s'", filename);
    fclose(file);
  }
}

void Context_T::buildRotmatrix(float m[4][4])
{
  double r0, r1, r2;
  extern void set_r(int i, double val);

  build_rotmatrix(m, quaternion);

  r1 = atan2(-m[0][2],sqrt(m[1][2]*m[1][2] + m[2][2]*m[2][2]));

  double c = cos(r1);  
  if(c != 0.0)
    {
      r0 = atan2(m[1][2]/c,m[2][2]/c) ;
      r2 = atan2(-m[1][0]/c,m[0][0]/c) ;
      r0 *= 180./(Pi);
      r2 *= 180./(Pi);
    }
  // lazyyyyyy
  set_r(0, r0);
  set_r(1, r1 * 180./(Pi));
  set_r(2, r2);
}

void Context_T::addQuaternion (float p1x, float p1y, float p2x, float p2y)
{
  float quat[4];
  trackball(quat,p1x,p1y,p2x,p2y);
  add_quats(quat, quaternion, quaternion);  
}

void Context_T::setQuaternion (float p1x, float p1y, float p2x, float p2y)
{
  trackball(quaternion,p1x,p1y,p2x,p2y);
}
