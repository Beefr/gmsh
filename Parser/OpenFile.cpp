// $Id: OpenFile.cpp,v 1.13 2001-04-26 17:58:00 remacle Exp $
#include "Gmsh.h"
#include "Const.h"
#include "Context.h"
#include "Parser.h"
#include "OpenFile.h"
#include "Geo.h"
#include "Mesh.h"
#include "Views.h"
#include "MinMax.h"
#include "Visibility.h"

#ifndef _BLACKBOX
#include "GmshUI.h"
#include "Draw.h"
#endif

#if _XMOTIF
#include "Widgets.h"
extern Widgets_T WID;
#elif _FLTK
#include "GUI.h"
extern GUI *WID;
#endif

extern Mesh      *THEM, M;
extern Context_T  CTX;

int ParseFile(char *f){
  char String[256];
  int status;

  strncpy(yyname,f,NAME_STR_L);
  yyerrorstate=0;
  yylineno=1;

  if(!(yyin = fopen(yyname,"r")))
    return 0;
  
  Msg(STATUS2, "Loading '%s'", yyname); 

  fpos_t position;
  fgetpos(yyin, &position);
  fgets(String, sizeof(String), yyin) ; 
  fsetpos(yyin, &position);

  if(!strncmp(String, "$PTS", 4) || 
     !strncmp(String, "$NO", 3) || 
     !strncmp(String, "$ELM", 4)){
    if(THEM->status < 0) mai3d(THEM, 0);
    Read_Mesh(THEM, yyin, FORMAT_MSH);
    status = THEM->status;
  }
  else if(!strncmp(String, "sms", 3))
  {
    if(THEM->status < 0) mai3d(THEM, 0);
    Read_Mesh(THEM, yyin, FORMAT_SMS);
    status = THEM->status;
  }
  else if(!strncmp(String, "$PostFormat", 11) ||
          !strncmp(String, "$View", 5)){
    Read_View(yyin, yyname);
    status = 0;
  }
  else{
    while(!feof(yyin)) yyparse();
    status = 0;
  }
  fclose(yyin);

  Msg(STATUS2, "Loaded '%s'", yyname); 
  return status;
}

void MergeProblem(char *name){

  ParseFile(name);  
  if (yyerrorstate) return;

#ifndef _BLACKBOX
  if (!EntitesVisibles) {
    RemplirEntitesVisibles(1);
    SHOW_ALL_ENTITIES = 1;
  }
#endif
}

void OpenProblem(char *name){
  char ext[6];
  int status;

  if(CTX.threads_lock){
    Msg(INFO, "I'm busy! Ask me that later...");
    return;
  }
  
  InitSymbols();
  Init_Mesh(&M, 1);

  strncpy(CTX.filename,name,NAME_STR_L);
  strncpy(CTX.basefilename,name,NAME_STR_L);

  strcpy(ext,name+(strlen(name)-4));
  if(!strcmp(ext,".geo") || !strcmp(ext,".GEO") ||
     !strcmp(ext,".msh") || !strcmp(ext,".MSH") ||
     !strcmp(ext,".stl") || !strcmp(ext,".MSH") ||
     !strcmp(ext,".sms") || !strcmp(ext,".SMS") ||
     !strcmp(ext,".pos") || !strcmp(ext,".POS")){
    CTX.basefilename[strlen(name)-4] = '\0';
  }
  else{
    strcat(CTX.filename,".geo");
  }

  strncpy(THEM->name, CTX.basefilename,NAME_STR_L);

  if(!CTX.batch){
#if _XMOTIF
    XtVaSetValues(WID.G.shell,
                  XmNtitle, CTX.filename,
                  XmNiconName, CTX.basefilename,
                  NULL);
#elif _FLTK
    WID->set_title(CTX.filename);
#endif
  }

  int nb = List_Nbr(Post_ViewList);

  status = ParseFile(CTX.filename);  

  ApplyLcFactor(THEM);

  if(!status) mai3d(THEM,0);  

  Maillage_Dimension_0(&M);

#ifndef _BLACKBOX
  ZeroHighlight(&M); 
#endif
  
  if(List_Nbr(Post_ViewList) > nb)
    CalculateMinMax(NULL, ((Post_View*)List_Pointer
			   (Post_ViewList,List_Nbr(Post_ViewList)-1))->BBox);
  else if(!status) 
    CalculateMinMax(THEM->Points,NULL);
  else
    CalculateMinMax(THEM->Vertices,NULL);

#ifndef _BLACKBOX
  if (!EntitesVisibles) {
    RemplirEntitesVisibles(1);
    SHOW_ALL_ENTITIES = 1;
  }
#endif

}

