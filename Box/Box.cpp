/* $Id: Box.cpp,v 1.15 2000-12-20 12:17:03 geuzaine Exp $ */

#include <signal.h>

#include "Gmsh.h"
#include "Const.h"
#include "Geo.h"
#include "Mesh.h"
#include "Views.h"
#include "Parser.h"
#include "Context.h"
#include "Main.h"
#include "MinMax.h"
#include "Version.h"

#include "Static.h"

extern List_T *Post_ViewList;

char *TheFileNameTab[MAX_OPEN_FILES];
char *ThePathForIncludes=NULL, *TheBgmFileName=NULL;
int   VERBOSE = 0 ;

char gmsh_progname[]  = "This is Gmsh (non-interactive)" ;
char gmsh_copyright[] = "Copyright (C) 1997-2000 J.-F. Remacle, C. Geuzaine";
char gmsh_version[]   = "Version          : " ;
char gmsh_os[]        = "Operating System : " GMSH_OS ;
char gmsh_date[]      = "Build Date       : " GMSH_DATE ;
char gmsh_host[]      = "Build Host       : " GMSH_HOST ;
char gmsh_packager[]  = "Packager         : " GMSH_PACKAGER ;
char gmsh_url[]       = "URL              : http://www.geuz.org/gmsh/" ;
char gmsh_email[]     = "E-Mail           : Christophe.Geuzaine@ulg.ac.be\n"
                        "                   Remacle@scorec.rpi.edu" ;
char gmsh_help[]      = 
  "Usage: %s [options] [files]\n"
  "Geometry options:\n"
  "  -0                    output flattened parsed geometry and exit\n"
  "Mesh options:\n"
  "  -1, -2, -3            perform batch 1D, 2D and 3D mesh generation\n"
  "  -smooth int           set mesh smoothing (default: 3)\n"
  "  -degree int           set mesh degree (default: 1)\n"
  "  -format msh|unv|gref  set output mesh format (default: msh)\n"
  "  -algo iso|aniso       select mesh algorithm (default: iso)\n"
  "  -scale float          set global scaling factor (default: 1.0)\n"
  "  -clscale float        set characteristic length scaling factor (default: 1.0)\n"
  "  -bgm file             load backround mesh from file\n"
  "Other options:\n"      
  "  -v                    print debug information\n"
  "  -path string          set path for included files\n"
  "  -version              show version number\n"
  "  -info                 show detailed version information\n"
  "  -help                 show this message\n"
  ;


/* dummy defs for link purposes */

void ZeroHighlight(Mesh *){}
void AddViewInUI(int, char *, int){}
void draw_polygon_2d (double, double, double, int, double *, double *, double *){}
void set_r(int, double){}
void Init(void){}
void Draw(void){}
void Replot(void){}
void Get_AnimTime(void){}
void CreateFile(char *, int){}

/* ------------------------------------------------------------------------ */
/*  I n f o                                                                 */
/* ------------------------------------------------------------------------ */

void Info (int level, char *arg0){
  switch(level){
  case 0 :
    fprintf(stderr, "%s\n", gmsh_progname);
    fprintf(stderr, "%s\n", gmsh_copyright);
    fprintf(stderr, gmsh_help, arg0);
    exit(1);
  case 1:
    fprintf(stderr, "%.3f\n", GMSH_VERSION);
    exit(1) ; 
  case 2:
    fprintf(stderr, "%s%.3f\n", gmsh_version, GMSH_VERSION);
    fprintf(stderr, "%s\n", gmsh_os);
    fprintf(stderr, "%s\n", gmsh_date);
    fprintf(stderr, "%s\n", gmsh_host);
    fprintf(stderr, "%s\n", gmsh_packager);
    fprintf(stderr, "%s\n", gmsh_url);
    fprintf(stderr, "%s\n", gmsh_email);
    exit(1) ; 
  default :
    break;
  }
}

/* ------------------------------------------------------------------------ */
/*  p a r s e                                                               */
/* ------------------------------------------------------------------------ */

void ParseFile(char *f){
  char String[256];

  strncpy(yyname,f,NAME_STR_L);
  yyerrorstate=0;
  yylineno=1;

  if(!(yyin = fopen(yyname,"r"))){
    Msg(INFO, "File '%s' Does not Exist", f);
    return;
  }
  
  fpos_t position;
  fgetpos(yyin, &position);
  fgets(String, sizeof(String), yyin) ; 
  fsetpos(yyin, &position);

  if(!strncmp(String, "$PTS", 4) || 
     !strncmp(String, "$NO", 3) || 
     !strncmp(String, "$ELM", 4)){
    if(THEM->status < 0) mai3d(THEM, 0);
    Read_Mesh(THEM, yyin, FORMAT_MSH);
  }
  else if(!strncmp(String, "$PostFormat", 11) ||
          !strncmp(String, "$View", 5)){
    Read_View(yyin, yyname);
  }
  else{
    while(!feof(yyin)) yyparse();
  }
  fclose(yyin);
}


void MergeProblem(char *name){
  Msg(INFOS, "Merging %s",name); 

  ParseFile(name);  
  if (yyerrorstate) return;
}

void OpenProblem(char *name){
  char ext[6];
  
  InitSymbols();
  Init_Mesh(&M, 1);

  strncpy(TheFileName,name,NAME_STR_L);
  strncpy(TheBaseFileName,name,NAME_STR_L);

  strcpy(ext,name+(strlen(name)-4));
  if(!strcmp(ext,".GEO") || 
     !strcmp(ext,".geo") || 
     !strcmp(ext,".msh") || 
     !strcmp(ext,".pos")){
    TheBaseFileName[strlen(name)-4] = '\0';
  }
  else{
    strcat(TheFileName,".geo");
  }

  strncpy(THEM->name, TheBaseFileName,NAME_STR_L);

  Msg(INFOS, "Opening %s", TheFileName); 

  ParseFile(TheFileName);  

  ApplyLcFactor(THEM);
  mai3d(THEM,0);  
  Maillage_Dimension_0(&M);
  ZeroHighlight(&M); 
  CalculateMinMax(THEM->Points);  
}

/* ------------------------------------------------------------------------ */
/*  G e t _ O p t i o n s                                                   */
/* ------------------------------------------------------------------------ */

void Get_Options (int argc, char *argv[], int *nbfiles) {
  int i=1;

  if(argc < 2) Info(0,argv[0]);

  TheFileNameTab[0] = "unnamed.geo" ;
  *nbfiles = 0;
  
  while (i < argc) {
    
    if (argv[i][0] == '-') {
      
      if(!strcmp(argv[i]+1, "0")){ 
        CTX.interactive = -1; i++;
      }
      else if(!strcmp(argv[i]+1, "1")){ 
        CTX.interactive = 1; i++;
      }
      else if(!strcmp(argv[i]+1, "2")){ 
        CTX.interactive = 2; i++;
      }
      else if(!strcmp(argv[i]+1, "3")){ 
        CTX.interactive = 3; i++;
      }
      else if(!strcmp(argv[i]+1, "v")){ 
        VERBOSE = 1; i++;
      }
      else if(!strcmp(argv[i]+1, "path")){ 
        i++;
        if(argv[i] != NULL) ThePathForIncludes = argv[i++];
        else{
          fprintf(stderr, ERROR_STR "Missing String\n");
          exit(1);
        }
      }
      else if(!strcmp(argv[i]+1, "bgm")){ 
        i++;
        if(argv[i] != NULL) TheBgmFileName = argv[i++];
        else{
          fprintf(stderr, ERROR_STR "Missing File Name\n");
          exit(1);
        }
      }
      else if(!strcmp(argv[i]+1, "smooth")){ 
        i++;
        if(argv[i] != NULL) CTX.mesh.nb_smoothing = atoi(argv[i++]);
        else{
          fprintf(stderr, ERROR_STR "Missing Number\n");
          exit(1);
        }
      }
      else if(!strcmp(argv[i]+1, "scale")){
        i++;
        if(argv[i] != NULL) CTX.mesh.scaling_factor = atof(argv[i++]);
        else{
          fprintf(stderr, ERROR_STR "Missing Number\n");
          exit(1);
        }
      }
      else if(!strcmp(argv[i]+1, "clscale")){
        i++;
        if(argv[i]!=NULL){
          CTX.mesh.lc_factor = atof(argv[i++]);
          if(CTX.mesh.lc_factor <= 0.0){
            fprintf(stderr, ERROR_STR 
                    "Characteristic Length Factor Must be > 0\n");
            exit(1);
          }
        }
        else {    
          fprintf(stderr, ERROR_STR "Missing Number\n");
          exit(1);
        }
      }
      else if(!strcmp(argv[i]+1, "degree")){  
        i++;
        if(argv[i]!=NULL){
          CTX.mesh.degree = atoi(argv[i++]);
          if(CTX.mesh.degree != 1 || CTX.mesh.degree != 2){
            fprintf(stderr, ERROR_STR "Wrong degree\n");
            exit(1);
          }
        }
        else {    
          fprintf(stderr, ERROR_STR "Missing Number\n");
          exit(1);
        }
      }
      else if(!strcmp(argv[i]+1, "format")){  
        i++;
        if(argv[i]!=NULL){
          if(!strcmp(argv[i],"msh"))
            CTX.mesh.format = FORMAT_MSH ;
          else if(!strcmp(argv[i],"unv"))
            CTX.mesh.format = FORMAT_UNV ;
          else if(!strcmp(argv[i],"gref"))
            CTX.mesh.format = FORMAT_GREF ;
          else{
            fprintf(stderr, ERROR_STR "Unknown mesh format\n");
            exit(1);
          }
          i++;
        }
        else {    
          fprintf(stderr, ERROR_STR "Missing format\n");
          exit(1);
        }
      }
      else if(!strcmp(argv[i]+1, "algo")){  
        i++;
        if(argv[i]!=NULL){
          if(!strcmp(argv[i],"iso"))
            CTX.mesh.algo = DELAUNAY_OLDALGO ;
          else if(!strcmp(argv[i],"aniso"))
            CTX.mesh.algo = DELAUNAY_NEWALGO ;
          else{
            fprintf(stderr, ERROR_STR "Unknown mesh algorithm\n");
            exit(1);
          }
          i++;
        }
        else {    
          fprintf(stderr, ERROR_STR "Missing algorithm\n");
          exit(1);
        }
      }
      else if(!strcmp(argv[i]+1, "info")){
        Info(2,argv[0]); 
      }
      else if(!strcmp(argv[i]+1, "version")){
        Info(1,argv[0]); 
      }
      else if(!strcmp(argv[i]+1, "help")){
        Info(0,argv[0]);
      }
      else{
        fprintf(stderr, WARNING_STR "Unknown option '%s'\n", argv[i]);
        Info(0,argv[0]);
      }
    }

    else {
      if(*nbfiles < MAX_OPEN_FILES){
        TheFileNameTab[(*nbfiles)++] = argv[i++]; 
      }
      else{
        fprintf(stderr, ERROR_STR "Too many input files\n");
        exit(1);
      }
    }

  }

  strncpy(TheFileName, TheFileNameTab[0], NAME_STR_L);

}

/* ------------------------------------------------------------------------ */
/*  m a i n                                                                 */
/* ------------------------------------------------------------------------ */

int main(int argc, char *argv[]){
  int     i, nbf;

  Init_Context();
  Get_Options(argc, argv, &nbf);

  signal(SIGINT,  Signal); 
  signal(SIGSEGV, Signal);
  signal(SIGFPE,  Signal); 

  OpenProblem(TheFileName);
  if(yyerrorstate)
    exit(1);
  else{
    if(nbf>1){
      for(i=1;i<nbf;i++) MergeProblem(TheFileNameTab[i]);
    }
    if(TheBgmFileName){
      MergeProblem(TheBgmFileName);
      if(List_Nbr(Post_ViewList))
        BGMWithView((Post_View*)List_Pointer(Post_ViewList, List_Nbr(Post_ViewList)-1));
      else
        fprintf(stderr, ERROR_STR "Invalid BGM (no view)\n"); exit(1);
    }
    if(CTX.interactive > 0){
      mai3d(THEM, CTX.interactive);
      Print_Mesh(THEM,NULL,CTX.mesh.format);
    }
    exit(1);
  }    

}


/* ------------------------------------------------------------------------ */
/*  S i g n a l                                                             */
/* ------------------------------------------------------------------------ */


void Signal (int sig_num){

  switch (sig_num){
  case SIGSEGV : Msg(FATAL, "Segmentation Violation (Invalid Memory Reference)"); break;
  case SIGFPE  : Msg(FATAL, "Floating Point Exception (Division by Zero?)"); break;
  case SIGINT  : Msg(FATAL, "Interrupt (Generated from Terminal Special Char)"); break;
  default      : Msg(FATAL, "Unknown Signal"); break;
  }
}


/* ------------------------------------------------------------------------ */
/*  M s g                                                                   */
/* ------------------------------------------------------------------------ */

void Msg(int level, char *fmt, ...){
  va_list  args;
  int      abort=0;
  int      nb, nbvis;

  va_start (args, fmt);

  switch(level){

  case FATAL :
    fprintf(stderr, FATAL_STR);
    vfprintf(stderr, fmt, args); fprintf(stderr, "\n");
    abort = 1 ;
    break ;

  case ERROR :
    fprintf(stderr, ERROR_STR);
    vfprintf(stderr, fmt, args); fprintf(stderr, "\n");
    abort = 1 ;
    break ;

  case WARNING :
    fprintf(stderr, WARNING_STR);
    vfprintf(stderr, fmt,args); fprintf(stderr, "\n");
    break;

  case PARSER_ERROR :
    fprintf(stderr, PARSER_ERROR_STR); 
    vfprintf(stderr, fmt, args); fprintf(stderr, "\n");
    break ;

  case PARSER_INFO :
    if(VERBOSE){
      fprintf(stderr, PARSER_INFO_STR);
      vfprintf(stderr, fmt, args); fprintf(stderr, "\n");
    }
    break ;

  case DEBUG :
  case INFOS :
  case INFO :
  case SELECT :
  case STATUS :
    if(VERBOSE){
      fprintf(stderr, INFO_STR);
      vfprintf(stderr, fmt, args); fprintf(stderr, "\n");
    }
    break;
  }

  va_end (args);

  if(abort) exit(1);

}

/* ------------------------------------------------------------------------ */
/*  C p u                                                                   */
/* ------------------------------------------------------------------------ */

double Cpu(void){
  return 0.;
}

/* ------------------------------------------------------------------------ */
/*  P r o g r e s s                                                         */
/* ------------------------------------------------------------------------ */

void Progress(int i){
}

void   AddALineInTheEditGeometryForm (char* line){
};
