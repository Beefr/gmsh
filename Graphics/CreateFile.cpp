// $Id: CreateFile.cpp,v 1.47 2003-09-23 00:54:05 geuzaine Exp $
//
// Copyright (C) 1997-2003 C. Geuzaine, J.-F. Remacle
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
// 
// Please report all bugs and problems to "gmsh@geuz.org".

#include "Gmsh.h"
#include "GmshUI.h"
#include "Mesh.h"
#include "OpenFile.h"
#include "Draw.h"
#include "Context.h"
#include "Options.h"

extern Context_T CTX;
extern Mesh M;

#include "gl2ps.h"
#include "gl2gif.h"
#include "gl2jpeg.h"
#include "gl2png.h"
#include "gl2ppm.h"
#include "gl2yuv.h"

void FillBuffer(void)
{
  InitOpengl();
  ClearOpengl();
  Draw3d();
  Draw2d();
}

void CreateOutputFile(char *name, int format)
{
  FILE *fp;
  GLint size3d, viewport[4];
  char ext[256];
  int len, res, oldformat, psformat, pssort, psoptions;

  if(!name || !strlen(name))
    return;

  oldformat = CTX.print.format;
  CTX.print.format = format;

  for(int i = 0; i < 4; i++) viewport[i] = CTX.viewport[i];

  switch (format) {

  case FORMAT_AUTO:
    for(len = strlen(name) - 1; len >= 0; len--) {
      if(name[len] == '.') {
        strcpy(ext, &name[len]);
        break;
      }
    }
    if(len <= 0)
      strcpy(ext, "");

    if(!strcmp(ext, ".geo"))
      CreateOutputFile(name, FORMAT_GEO);
    else if(!strcmp(ext, ".opt"))
      CreateOutputFile(name, FORMAT_OPT);
    else if(!strcmp(ext, ".msh"))
      CreateOutputFile(name, FORMAT_MSH);
    else if(!strcmp(ext, ".unv"))
      CreateOutputFile(name, FORMAT_UNV);
    else if(!strcmp(ext, ".gif"))
      CreateOutputFile(name, FORMAT_GIF);
    else if(!strcmp(ext, ".jpg"))
      CreateOutputFile(name, FORMAT_JPEG);
    else if(!strcmp(ext, ".jpeg"))
      CreateOutputFile(name, FORMAT_JPEG);
    else if(!strcmp(ext, ".png"))
      CreateOutputFile(name, FORMAT_PNG);
    else if(!strcmp(ext, ".ps"))
      CreateOutputFile(name, FORMAT_PS);
    else if(!strcmp(ext, ".eps"))
      CreateOutputFile(name, FORMAT_EPS);
    else if(!strcmp(ext, ".pdf"))
      CreateOutputFile(name, FORMAT_PDF);
    else if(!strcmp(ext, ".tex"))
      CreateOutputFile(name, FORMAT_TEX);
    else if(!strcmp(ext, ".pstex"))
      CreateOutputFile(name, FORMAT_PSTEX);
    else if(!strcmp(ext, ".epstex"))
      CreateOutputFile(name, FORMAT_EPSTEX);
    else if(!strcmp(ext, ".pdftex"))
      CreateOutputFile(name, FORMAT_PDFTEX);
    else if(!strcmp(ext, ".jpegtex"))
      CreateOutputFile(name, FORMAT_JPEGTEX);
    else if(!strcmp(ext, ".ppm"))
      CreateOutputFile(name, FORMAT_PPM);
    else if(!strcmp(ext, ".yuv"))
      CreateOutputFile(name, FORMAT_YUV);
    else if(!strcmp(ext, ".gref"))
      CreateOutputFile(name, FORMAT_GREF);
    else if(!strcmp(ext, ".Gref"))
      CreateOutputFile(name, FORMAT_GREF);
    else if(!strcmp(ext, ".wrl"))
      CreateOutputFile(name, FORMAT_VRML);
    else
      Msg(GERROR, "Unknown extension '%s' for automatic format detection",
          ext);
    break;

  case FORMAT_GEO:
    Print_Geo(&M, name);
    break;

  case FORMAT_OPT:
    Print_Options(0, GMSH_FULLRC, name);
    break;

  case FORMAT_MSH:
  case FORMAT_UNV:
  case FORMAT_GREF:
  case FORMAT_VRML:
    Print_Mesh(&M, name, format);
    break;

  case FORMAT_JPEG:
  case FORMAT_JPEGTEX:
  case FORMAT_PNG:
  case FORMAT_PNGTEX:
    if(!(fp = fopen(name, "wb"))) {
      Msg(GERROR, "Unable to open file '%s'", name);
      return;
    }
    if(format == FORMAT_JPEGTEX || format == FORMAT_PNGTEX){
      CTX.print.gl_fonts = 0;
    }
    FillBuffer();
    CTX.print.gl_fonts = 1;
    if(format == FORMAT_JPEG || format == FORMAT_JPEGTEX){
      create_jpeg(fp, viewport[2]-viewport[0], viewport[3]-viewport[1], CTX.print.jpeg_quality);
      Msg(INFO, "JPEG creation complete '%s'", name);
    }
    else{
      create_png(fp, viewport[2]-viewport[0], viewport[3]-viewport[1], 100);
      Msg(INFO, "PNG creation complete '%s'", name);
    }
    Msg(STATUS2, "Wrote '%s'", name);
    fclose(fp);
    break;

  case FORMAT_PPM:
  case FORMAT_YUV:
  case FORMAT_GIF:
    if(!(fp = fopen(name, "wb"))) {
      Msg(GERROR, "Unable to open file '%s'", name);
      return;
    }
    FillBuffer();
    if(format == FORMAT_PPM){
      create_ppm(fp, viewport[2]-viewport[0], viewport[3]-viewport[1]);
      Msg(INFO, "PPM creation complete '%s'", name);
    }
    else if (format == FORMAT_YUV){
      create_yuv(fp, viewport[2]-viewport[0], viewport[3]-viewport[1]);
      Msg(INFO, "YUV creation complete '%s'", name);
    }
    else{
      create_gif(fp, viewport[2]-viewport[0], viewport[3]-viewport[1],
		 CTX.print.gif_dither,
		 CTX.print.gif_sort,
		 CTX.print.gif_interlace,
		 CTX.print.gif_transparent,
		 UNPACK_RED(CTX.color.bg),
		 UNPACK_GREEN(CTX.color.bg), UNPACK_BLUE(CTX.color.bg));
      Msg(INFO, "GIF creation complete '%s'", name);
    }
    Msg(STATUS2, "Wrote '%s'", name);
    fclose(fp);
    break;

  case FORMAT_PS:
  case FORMAT_PSTEX:
  case FORMAT_EPS:
  case FORMAT_EPSTEX:
  case FORMAT_PDF:
  case FORMAT_PDFTEX:
    if(format == FORMAT_PDF || format == FORMAT_PDFTEX){
      fp = fopen(name, "wb");
      psformat = GL2PS_PDF;
    }
    else{
      fp = fopen(name, "w");
      if(format == FORMAT_PS || format == FORMAT_PSTEX)
	psformat = GL2PS_PS;
      else
	psformat = GL2PS_EPS;
    }

    if(!fp) {
      Msg(GERROR, "Unable to open file '%s'", name);
      return;
    }
    pssort = (CTX.print.eps_quality == 1) ? GL2PS_SIMPLE_SORT : GL2PS_BSP_SORT;
    psoptions =
      GL2PS_SIMPLE_LINE_OFFSET | GL2PS_SILENT | 
      (CTX.print.eps_occlusion_culling ? GL2PS_OCCLUSION_CULL : 0) |
      (CTX.print.eps_best_root ? GL2PS_BEST_ROOT : 0) |
      (CTX.print.eps_background ? GL2PS_DRAW_BACKGROUND : 0) |
      (format == FORMAT_PSTEX ? GL2PS_NO_TEXT : 0) |
      (format == FORMAT_EPSTEX ? GL2PS_NO_TEXT : 0) |
      (format == FORMAT_PDFTEX ? GL2PS_NO_TEXT : 0);

    size3d = 0;
    res = GL2PS_OVERFLOW;
    while(res == GL2PS_OVERFLOW) {
      size3d += 2048 * 2048;
      gl2psBeginPage(CTX.base_filename, "Gmsh", viewport, 
		     psformat, pssort, psoptions, GL_RGBA, 0, NULL, 
		     0, 0, 0, size3d, fp, name);
      CTX.print.gl_fonts = 0;
      FillBuffer();
      CTX.print.gl_fonts = 1;
      res = gl2psEndPage();
    }
    if(psformat == GL2PS_PDF)
      Msg(INFO, "PDF creation complete '%s'", name);
    else
      Msg(INFO, "PS/EPS creation complete '%s'", name);
    Msg(STATUS2, "Wrote '%s'", name);
    fclose(fp);
    break;

  case FORMAT_TEX:
    if(!(fp = fopen(name, "w"))) {
      Msg(GERROR, "Unable to open file '%s'", name);
      return;
    }
    gl2psBeginPage(CTX.base_filename, "Gmsh", viewport,
                   GL2PS_TEX, GL2PS_NO_SORT, GL2PS_SILENT, GL_RGBA, 0, NULL, 
		   0, 0, 0, 1000, fp, name);
    CTX.print.gl_fonts = 0;
    FillBuffer();
    CTX.print.gl_fonts = 1;
    res = gl2psEndPage();
    Msg(INFO, "TEX creation complete '%s'", name);
    Msg(STATUS2, "Wrote '%s'", name);
    fclose(fp);
    break;

  default:
    Msg(WARNING, "Unknown print format");
    break;
  }

  CTX.print.format = oldformat;

}
