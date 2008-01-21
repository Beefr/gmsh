// $Id: OpenFile.cpp,v 1.169 2008-01-21 20:34:54 geuzaine Exp $
//
// Copyright (C) 1997-2007 C. Geuzaine, J.-F. Remacle
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
// Please report all bugs and problems to <gmsh@geuz.org>.

#if defined(__CYGWIN__)
#include <sys/cygwin.h>
#endif

#include "Message.h"
#include "Geo.h"
#include "GModel.h"
#include "Numeric.h"
#include "Context.h"
#include "Parser.h"
#include "OpenFile.h"
#include "CommandLine.h"
#include "PView.h"
#include "ReadImg.h"
#include "OS.h"
#include "HighOrder.h"

#if defined(HAVE_FLTK)
#include "GmshUI.h"
#include "Draw.h"
#include "SelectBuffer.h"
#include "GUI.h"
extern GUI *WID;
#endif

extern Context_T CTX;

void FixRelativePath(const char *in, char *out){
  if(in[0] == '/' || in[0] == '\\' || (strlen(in)>2 && in[1] == ':')){
    // do nothing: 'in' is an absolute path
    strcpy(out, in);
  }
  else{
    // append 'in' to the path of the parent file
    strcpy(out, gmsh_yyname);
    int i = strlen(out)-1 ;
    while(i >= 0 && gmsh_yyname[i] != '/' && gmsh_yyname[i] != '\\') i-- ;
    out[i+1] = '\0';
    strcat(out, in);
  }
}

void FixWindowsPath(const char *in, char *out){
#if defined(__CYGWIN__)
  cygwin_conv_to_win32_path(in, out);
#else
  strcpy(out, in);
#endif
}

void SplitFileName(const char *name, char *no_ext, char *ext, char *base)
{
  strcpy(no_ext, name);
  strcpy(ext, "");
  for(int i = strlen(name) - 1; i >= 0; i--){
    if(name[i] == '.'){
      strcpy(ext, &name[i]);
      no_ext[i] = '\0';
      break;
    }
  }
  strcpy(base, no_ext);
  for(int i = strlen(no_ext) - 1; i >= 0; i--){
    if(no_ext[i] == '/' || no_ext[i] == '\\'){
      strcpy(base, &no_ext[i + 1]);
      break;
    }
  }
}

static void FinishUpBoundingBox()
{
  double range[3];

  for(int i = 0; i < 3; i++){
    CTX.cg[i] = 0.5 * (CTX.min[i] + CTX.max[i]);
    range[i] = CTX.max[i] - CTX.min[i];
  }

  if(range[0] == 0. && range[1] == 0. && range[2] == 0.) {
    CTX.min[0] -= 1.; CTX.min[1] -= 1.;
    CTX.max[0] += 1.; CTX.max[1] += 1.;
    CTX.lc = 1.;
  }
  else if(range[0] == 0. && range[1] == 0.) {
    CTX.lc = range[2];
    CTX.min[0] -= CTX.lc; CTX.min[1] -= CTX.lc;
    CTX.max[0] += CTX.lc; CTX.max[1] += CTX.lc;
  }
  else if(range[0] == 0. && range[2] == 0.) {
    CTX.lc = range[1];
    CTX.min[0] -= CTX.lc; CTX.max[0] += CTX.lc;
  }
  else if(range[1] == 0. && range[2] == 0.) {
    CTX.lc = range[0];
    CTX.min[1] -= CTX.lc; CTX.max[1] += CTX.lc;
  }
  else if(range[0] == 0.) {
    CTX.lc = sqrt(DSQR(range[1]) + DSQR(range[2]));
    CTX.min[0] -= CTX.lc; CTX.max[0] += CTX.lc;
  }
  else if(range[1] == 0.) {
    CTX.lc = sqrt(DSQR(range[0]) + DSQR(range[2]));
    CTX.min[1] -= CTX.lc; CTX.max[1] += CTX.lc;
  }
  else if(range[2] == 0.) {
    CTX.lc = sqrt(DSQR(range[0]) + DSQR(range[1]));
  }
  else {
    CTX.lc = sqrt(DSQR(range[0]) + DSQR(range[1]) + DSQR(range[2]));
  }
}

void SetBoundingBox(double xmin, double xmax,
		    double ymin, double ymax, 
		    double zmin, double zmax)
{
  CTX.min[0] = xmin; CTX.max[0] = xmax;
  CTX.min[1] = ymin; CTX.max[1] = ymax;
  CTX.min[2] = zmin; CTX.max[2] = zmax;
  FinishUpBoundingBox();
}

void SetBoundingBox(void)
{
  if(CTX.forced_bbox) return;

  SBoundingBox3d bb = GModel::current()->bounds();
  
  if(bb.empty()) {
    for(unsigned int i = 0; i < PView::list.size(); i++)
      if(!PView::list[i]->getData()->getBoundingBox().empty())
	bb += PView::list[i]->getData()->getBoundingBox();
  }
  
  if(bb.empty()){
    bb += SPoint3(-1., -1., -1.);
    bb += SPoint3(1., 1., 1.);
  }
  
  CTX.min[0] = bb.min().x(); CTX.max[0] = bb.max().x();
  CTX.min[1] = bb.min().y(); CTX.max[1] = bb.max().y();
  CTX.min[2] = bb.min().z(); CTX.max[2] = bb.max().z();
  FinishUpBoundingBox();
}

// FIXME: this is necessary for now to have an approximate CTX.lc
// *while* parsing input files (it's important since some of the
// geometrical operations use a tolerance that depends on
// CTX.lc). This will be removed once the new database is filled
// directly during the parsing step
static SBoundingBox3d temp_bb;

void ResetTemporaryBoundingBox()
{
  temp_bb.reset();
}

void AddToTemporaryBoundingBox(double x, double y, double z)
{
  temp_bb += SPoint3(x, y, z);
  CTX.min[0] = temp_bb.min().x(); CTX.max[0] = temp_bb.max().x();
  CTX.min[1] = temp_bb.min().y(); CTX.max[1] = temp_bb.max().y();
  CTX.min[2] = temp_bb.min().z(); CTX.max[2] = temp_bb.max().z();
  FinishUpBoundingBox();
}

int ParseFile(const char *f, int close, int warn_if_missing)
{
  char yyname_old[256], tmp[256];
  FILE *yyin_old, *fp;
  int yylineno_old, yyerrorstate_old, yyviewindex_old;

  // add 'b' for pure Windows programs: opening in text mode messes up
  // fsetpos/fgetpos (used e.g. for user-defined functions)
  if(!(fp = fopen(f, "rb"))){
    if(warn_if_missing) Msg(WARNING, "Unable to open file '%s'", f);
    return 0;
  }

  int numViewsBefore = PView::list.size();

  strncpy(yyname_old, gmsh_yyname, 255);
  yyin_old = gmsh_yyin;
  yyerrorstate_old = gmsh_yyerrorstate;
  yylineno_old = gmsh_yylineno;
  yyviewindex_old = gmsh_yyviewindex;

  strncpy(gmsh_yyname, f, 255);
  gmsh_yyin = fp;
  gmsh_yyerrorstate = 0;
  gmsh_yylineno = 1;
  gmsh_yyviewindex = 0;

  fpos_t position;
  fgetpos(gmsh_yyin, &position);
  fgets(tmp, sizeof(tmp), gmsh_yyin);
  fsetpos(gmsh_yyin, &position);

  while(!feof(gmsh_yyin)){
    gmsh_yyparse();
    if(gmsh_yyerrorstate > 20){
      Msg(GERROR, "Too many errors: aborting...");
      force_yyflush();
      break;
    }
  }

  if(close) fclose(gmsh_yyin);

  strncpy(gmsh_yyname, yyname_old, 255);
  gmsh_yyin = yyin_old;
  gmsh_yyerrorstate = yyerrorstate_old;
  gmsh_yylineno = yylineno_old;
  gmsh_yyviewindex = yyviewindex_old;

#if defined(HAVE_FLTK)
  if(!CTX.batch && numViewsBefore != (int)PView::list.size())
    WID->update_views();
#endif

  return 1;
}

void ParseString(const char *str)
{
  if(!str) return;
  FILE *fp;
  if((fp = fopen(CTX.tmp_filename_fullpath, "w"))) {
    fprintf(fp, str);
    fprintf(fp, "\n");
    fclose(fp);
    ParseFile(CTX.tmp_filename_fullpath, 1);
    GModel::current()->importGEOInternals();
  }
}

void SetProjectName(const char *name)
{
  char no_ext[256], ext[256], base[256];
  SplitFileName(name, no_ext, ext, base);

  if(CTX.filename != name) // yes, we mean to compare the pointers
    strncpy(CTX.filename, name, 255);
  strncpy(CTX.no_ext_filename, no_ext, 255);
  strncpy(CTX.base_filename, base, 255);

#if defined(HAVE_FLTK)
  if(!CTX.batch) WID->set_title(CTX.filename);
#endif
}

int MergeFile(const char *name, int warn_if_missing)
{
  // added 'b' for pure Windows programs, since some of these files
  // contain binary data
  FILE *fp = fopen(name, "rb");
  if(!fp){
    if(warn_if_missing) Msg(WARNING, "Unable to open file '%s'", name);
    return 0;
  }

  char header[256];
  fgets(header, sizeof(header), fp);
  fclose(fp);

  Msg(STATUS2, "Reading '%s'", name);

  char no_ext[256], ext[256], base[256];
  SplitFileName(name, no_ext, ext, base);

#if defined(HAVE_FLTK)
  if(!CTX.batch) {
    if(!strcmp(ext, ".gz")) {
      // the real solution would be to rewrite all our I/O functions in
      // terms of gzFile, but until then, this is better than nothing
      if(fl_choice("File '%s' is in gzip format.\n\nDo you want to uncompress it?", 
		   "Cancel", "Uncompress", NULL, name)){
	char tmp[256];
	sprintf(tmp, "gunzip -c %s > %s", name, no_ext);
	if(SystemCall(tmp))
	  Msg(GERROR, "Failed to uncompress `%s': check directory permissions", name);
	if(!strcmp(CTX.filename, name)) // this is the project file
	  SetProjectName(no_ext);
	return MergeFile(no_ext);
      }
    }
  }
#endif

  CTX.geom.draw = 0; // don't try to draw the model while reading

  GModel *m = GModel::current();

  int numViewsBefore = PView::list.size();

  int status = 0;
  if(!strcmp(ext, ".stl") || !strcmp(ext, ".STL")){
    status = m->readSTL(name, CTX.geom.tolerance);
  }
  else if(!strcmp(ext, ".brep") || !strcmp(ext, ".rle") ||
	  !strcmp(ext, ".brp") || !strcmp(ext, ".BRP")){
    status = m->readOCCBREP(std::string(name));
  }
  else if(!strcmp(ext, ".iges") || !strcmp(ext, ".IGES") ||
	  !strcmp(ext, ".igs") || !strcmp(ext, ".IGS")){
    status = m->readOCCIGES(std::string(name));
  }
  else if(!strcmp(ext, ".step") || !strcmp(ext, ".STEP") ||
	  !strcmp(ext, ".stp") || !strcmp(ext, ".STP")){
    status = m->readOCCSTEP(std::string(name));
  }
  else if(!strcmp(ext, ".unv") || !strcmp(ext, ".UNV")){
    status = m->readUNV(name);
  }
  else if(!strcmp(ext, ".wrl") || !strcmp(ext, ".WRL") || 
	  !strcmp(ext, ".vrml") || !strcmp(ext, ".VRML") ||
	  !strcmp(ext, ".iv") || !strcmp(ext, ".IV")){
    status = m->readVRML(name);
  }
  else if(!strcmp(ext, ".mesh") || !strcmp(ext, ".MESH")){
    status = m->readMESH(name);
  }
  else if(!strcmp(ext, ".bdf") || !strcmp(ext, ".BDF") ||
	  !strcmp(ext, ".nas") || !strcmp(ext, ".NAS")){
    status = m->readBDF(name);
  }
  else if(!strcmp(ext, ".p3d") || !strcmp(ext, ".P3D")){
    status = m->readP3D(name);
  }
  else if(!strcmp(ext, ".fm") || !strcmp(ext, ".FM")) {
    status = m->readFourier(name);
  }
#if defined(HAVE_FLTK)
  else if(!strcmp(ext, ".pnm") || !strcmp(ext, ".PNM") ||
	  !strcmp(ext, ".pbm") || !strcmp(ext, ".PBM") ||
	  !strcmp(ext, ".pgm") || !strcmp(ext, ".PGM") ||
	  !strcmp(ext, ".ppm") || !strcmp(ext, ".PPM")) {
    status = read_pnm(name);
  }
  else if(!strcmp(ext, ".bmp") || !strcmp(ext, ".BMP")) {
    status = read_bmp(name);
  }
#if defined(HAVE_LIBJPEG)
  else if(!strcmp(ext, ".jpg") || !strcmp(ext, ".JPG") ||
	  !strcmp(ext, ".jpeg") || !strcmp(ext, ".JPEG")) {
    status = read_jpeg(name);
  }
#endif
#if defined(HAVE_LIBPNG)
  else if(!strcmp(ext, ".png") || !strcmp(ext, ".PNG")) {
    status = read_png(name);
  }
#endif
#endif
  else {
    CTX.geom.draw = 1;
    if(!strncmp(header, "$PTS", 4) || !strncmp(header, "$NO", 3) || 
       !strncmp(header, "$PARA", 5) || !strncmp(header, "$ELM", 4) ||
       !strncmp(header, "$MeshFormat", 11)) {
      status = m->readMSH(name);
    }
    else if(!strncmp(header, "$PostFormat", 11) || 
	    !strncmp(header, "$View", 5)) {
      status = PView::read(name);
    }
    else {
      status = m->readGEO(name);
    }
  }

  SetBoundingBox();

  CTX.geom.draw = 1;
  CTX.mesh.changed = ENT_ALL;

  checkHighOrderTriangles(m);

#if defined(HAVE_FLTK)
  if(!CTX.batch && numViewsBefore != (int)PView::list.size())
    WID->update_views();
#endif

  Msg(STATUS2, "Read '%s'", name);
  return status;
}

void OpenProject(const char *name)
{
  if(CTX.threads_lock) {
    Msg(INFO, "I'm busy! Ask me that later...");
    return;
  }
  CTX.threads_lock = 1;

  GModel::current()->destroy();
  GModel::current()->getGEOInternals()->destroy();

  // Initialize pseudo random mesh generator to the same seed
  srand(1);

  // temporary hack until we fill the current GModel on the fly during
  // parsing
  ResetTemporaryBoundingBox();

  SetProjectName(name);
  MergeFile(name);

  CTX.threads_lock = 0;

#if defined(HAVE_FLTK)
  if(!CTX.batch) WID->reset_visibility();
  ZeroHighlight();
#endif
}

void OpenProjectMacFinder(const char *filename)
{
  static int first = 1;
  if(first || CTX.batch){
    // just copy the filename: it will be opened when the GUI is ready
    // in main()
    strncpy(CTX.filename, filename, 255);
    first = 0;
  }
  else{
    OpenProject((char*)filename);
#if defined(HAVE_FLTK)
    Draw();
#endif
  }
}

