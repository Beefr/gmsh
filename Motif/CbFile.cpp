// $Id: CbFile.cpp,v 1.6 2001-02-17 22:04:05 geuzaine Exp $

#include "Gmsh.h"
#include "GmshUI.h"
#include "OpenFile.h"
#include "Mesh.h"
#include "Draw.h"
#include "Widgets.h"
#include "Context.h"
#include "Options.h"
#include "CreateFile.h"

#include "CbFile.h"
#include "CbColorbar.h"

extern Context_T   CTX;
extern Widgets_T   WID;
extern Mesh        M;

void SaveToDisk (char *FileName, Widget warning, 
                 void (*function)(char *filename, int format)){
  FILE    *fp ;
  static char KeepFileName[256];

  if(FileName){
    fp = fopen(FileName,"r");
    if(fp) {      
      XtManageChild(warning);
      strcpy(KeepFileName,FileName);
      fclose(fp);
      return;
    }
    else{
      strcpy(KeepFileName,FileName);
    }
  }

  function(KeepFileName, CTX.print.format);
}

/* ------------------------------------------------------------------------ 
    F i l e C b                                                       
   ------------------------------------------------------------------------ */

void FileCb(Widget w, XtPointer client_data, XtPointer call_data){
  char      *c;
  XmString  xms;

  switch ((long int)client_data) {
  case FILE_SAVE_MESH :
    Print_Mesh(&M, NULL, CTX.mesh.format); 
    return;
  case FILE_SAVE_AS_OVERWRITE :
    SaveToDisk(NULL, WID.ED.saveAsDialog, CreateOutputFile);
    return;
  }

  XtVaGetValues(w, XmNtextString, &xms, NULL);
  XmStringGetLtoR(xms, XmSTRING_DEFAULT_CHARSET, &c);
  XmStringFree(xms);
  
  switch ((long int)client_data) {
  case FILE_LOAD_GEOM       : OpenProblem(c); Draw(); break;
  case FILE_LOAD_POST       : MergeProblem(c); ColorBarRedraw(); Draw(); break;
  case FILE_SAVE_AS         : SaveToDisk(c, WID.ED.saveAsDialog, CreateOutputFile); break;
  case FILE_SAVE_OPTIONS_AS : Print_Context(0,GMSH_FULLRC,c); break;
  default :
    Msg(WARNING, "Unknown event in FileCb : %d", (long int)client_data); 
    break;
  }

}

