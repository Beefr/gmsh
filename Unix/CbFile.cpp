/* $Id: CbFile.cpp,v 1.17 2000-12-26 17:40:18 geuzaine Exp $ */

#include <unistd.h>

#include "Gmsh.h"
#include "GmshUI.h"
#include "Main.h"
#include "Mesh.h"
#include "Draw.h"
#include "Widgets.h"
#include "Context.h"
#include "XContext.h"

#include "CbFile.h"
#include "CbColorbar.h"

#include "XDump.h"
#include "gl2ps.h"
#include "gl2gif.h"
#include "gl2jpeg.h"
#include "gl2ppm.h"
#include "gl2yuv.h"

extern Context_T   CTX;
extern XContext_T  XCTX;
extern Widgets_T   WID;
extern Mesh        M;

/* ------------------------------------------------------------------------ */
/*  C r e a t e I m a g e                                                   */
/* ------------------------------------------------------------------------ */

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

void CreateFile (char *name, int format) {
  FILE    *tmp, *fp;
  GLint    size3d;
  char     cmd[1000], ext[10];
  char     *tmpFileName="tmp.xwd";
  int      res;

  CTX.print.gl_fonts = 1;

  switch(format){

  case FORMAT_AUTO :
    if(strlen(name) < 4)
      Msg(ERROR, "Unknown Extension for Automatic Format Detection");
    else{
      strcpy(ext,name+(strlen(name)-4));
      if(!strcmp(ext,".geo")) CreateFile(name, FORMAT_GEO);
      else if(!strcmp(ext,".msh")) CreateFile(name, FORMAT_MSH);
      else if(!strcmp(ext,".unv")) CreateFile(name, FORMAT_UNV);
      else if(!strcmp(ext,".gif")) CreateFile(name, FORMAT_GIF);
      else if(!strcmp(ext,".jpg")) CreateFile(name, FORMAT_JPEG);
      else if(!strcmp(ext,".eps")) CreateFile(name, FORMAT_EPS);
      else if(!strcmp(ext,".xpm")) CreateFile(name, FORMAT_XPM);
      else if(!strcmp(ext,".ppm")) CreateFile(name, FORMAT_PPM);
      else if(!strcmp(ext,".yuv")) CreateFile(name, FORMAT_YUV);
      else {
	if(strlen(name) < 5)
	  Msg(ERROR, "Unknown Extension for Automatic Format Detection");
	else{
	  strcpy(ext,name+(strlen(name)-5));
	  if(!strcmp(ext,".jpeg")) CreateFile(name, FORMAT_JPEG);
	  else if(!strcmp(ext,".gref")) CreateFile(name, FORMAT_GREF);
	  else if(!strcmp(ext,".Gref")) CreateFile(name, FORMAT_GREF);
	  else Msg(ERROR, "Unknown Extension \"%s\" for Automatic Format Detection", ext);
	}
      }
    }
    break;

  case FORMAT_GEO :
    Print_Geo(&M, name);
    break;
    
  case FORMAT_MSH :
    Print_Mesh(&M, name, FORMAT_MSH); 
    break;

  case FORMAT_UNV :
    Print_Mesh(&M, name, FORMAT_UNV); 
    break;
    
  case FORMAT_GREF :
    Print_Mesh(&M, name, FORMAT_GREF); 
    break;

  case FORMAT_XPM :
    if(!(fp = fopen(name,"wb"))) {
      Msg(WARNING, "Unable to Open File '%s'", name); 
      return;
    }
    Window_Dump(XCTX.display, XCTX.scrnum, XtWindow(WID.G.glw), fp);    
    Msg(INFOS, "XPM Creation Complete '%s'", name);
    Msg (INFO, "Wrote File '%s'", name);
    fclose(fp);
    break;

  case FORMAT_JPEG :
    if(!(fp = fopen(name,"wb"))) {
      Msg(WARNING, "Unable to Open File '%s'", name); 
      return;
    }
    Replot();
    create_jpeg(fp, CTX.viewport[2]-CTX.viewport[0],
		CTX.viewport[3]-CTX.viewport[1]);
    Msg(INFOS, "JPEG Creation Complete '%s'", name);
    Msg (INFO, "Wrote File '%s'", name);
    fclose(fp);
    break;

  case FORMAT_GIF :
    // have to replot for filling again buffer ...
    if(!(fp = fopen(name,"wb"))) {
      Msg(WARNING, "Unable to Open File '%s'", name); 
      return;
    }
    Replot();
    create_gif(fp, CTX.viewport[2]-CTX.viewport[0],
               CTX.viewport[3]-CTX.viewport[1]);
    Msg(INFOS, "GIF Creation Complete '%s'", name);
    Msg (INFO, "Wrote File '%s'", name);
    fclose(fp);
    break;

  case FORMAT_PPM :
    if(!(fp = fopen(name,"wb"))) {
      Msg(WARNING, "Unable to Open File '%s'", name); 
      return;
    }
    Replot();
    create_ppm(fp, CTX.viewport[2]-CTX.viewport[0],
	       CTX.viewport[3]-CTX.viewport[1]);
    Msg(INFOS, "PPM Creation Complete '%s'", name);
    Msg (INFO, "Wrote File '%s'", name);
    fclose(fp);
    break;

  case FORMAT_YUV :
    if(!(fp = fopen(name,"wb"))) {
      Msg(WARNING, "Unable to Open File '%s'", name); 
      return;
    }
    Replot();
    create_yuv(fp, CTX.viewport[2]-CTX.viewport[0],
	       CTX.viewport[3]-CTX.viewport[1]);
    Msg(INFOS, "YUV Creation Complete '%s'", name);
    Msg (INFO, "Wrote File '%s'", name);
    fclose(fp);
    break;

  case FORMAT_EPS :

    switch(CTX.print.eps_quality){

    case 0 : // Bitmap EPS
      if(!(fp = fopen(name,"w"))) {
	Msg(WARNING, "Unable to Open File '%s'", name); 
	return;
      }
      if(!(tmp = fopen(tmpFileName,"w"))){
	Msg(WARNING, "Unable to Open File '%s'", tmpFileName); 
	return;
      }
      Window_Dump(XCTX.display, XCTX.scrnum, XtWindow(WID.G.glw), tmp);
      fclose(tmp);
      sprintf(cmd, "xpr -device ps -gray 4 %s >%s", tmpFileName, name);
      Msg(INFOS, "Executing '%s'", cmd);
      system(cmd);
      unlink(tmpFileName);
      Msg(INFOS, "Bitmap EPS Creation Complete '%s'", name);
      Msg (INFO, "Wrote File '%s'", name);
      fclose(fp);
      break;
      
    default : // Vector EPS
      if(!(fp = fopen(name,"w"))) {
	Msg(WARNING, "Unable to Open File '%s'", name); 
	return;
      }
      CTX.print.gl_fonts = 0;
      size3d = 0 ;
      res = GL2PS_OVERFLOW ;
      while(res == GL2PS_OVERFLOW){
	size3d += 2048*2048 ;
	gl2psBeginPage(TheBaseFileName, "Gmsh", 
		       (CTX.print.eps_quality == 1 ? GL2PS_SIMPLE_SORT : GL2PS_BSP_SORT),
		       GL2PS_SIMPLE_LINE_OFFSET | GL2PS_DRAW_BACKGROUND,
		       GL_RGBA, 0, NULL, size3d, fp);
	CTX.stream = TO_FILE ;
	Init();
	Draw();
	CTX.stream = TO_SCREEN ;
	res = gl2psEndPage();
      }
      Msg(INFOS, "EPS Creation Complete '%s'", name);
      Msg (INFO, "Wrote File '%s'", name);
      fclose(fp);
      CTX.print.gl_fonts = 1;
      break;
      
    }
    break ;
    
  default :
    Msg(WARNING, "Unknown Print Format");
    break;
  }


}

/* ------------------------------------------------------------------------ */
/*  file                                                                    */
/* ------------------------------------------------------------------------ */

void FileCb(Widget w, XtPointer client_data, XtPointer call_data){
  char      *c;
  XmString  xms;

  switch ((long int)client_data) {
  case FILE_SAVE_MESH :
    Print_Mesh(&M, NULL, CTX.mesh.format); 
    return;
  case FILE_SAVE_AS_OVERWRITE :
    SaveToDisk(NULL, WID.ED.saveAsDialog, CreateFile);
    return;
  }

  XtVaGetValues(w, XmNtextString, &xms, NULL);
  XmStringGetLtoR(xms, XmSTRING_DEFAULT_CHARSET, &c);
  XmStringFree(xms);
  
  switch ((long int)client_data) {
  case FILE_LOAD_GEOM       : OpenProblem(c); Init(); Draw(); break;
  case FILE_LOAD_POST       : MergeProblem(c); ColorBarRedraw(); Init(); Draw(); break;
  case FILE_SAVE_AS         : SaveToDisk(c, WID.ED.saveAsDialog, CreateFile); break;
  case FILE_SAVE_OPTIONS_AS : Print_Context(c); break;
  default :
    Msg(WARNING, "Unknown event in FileCb : %d", (long int)client_data); 
    break;
  }

}

