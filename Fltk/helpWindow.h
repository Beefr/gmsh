// Gmsh - Copyright (C) 1997-2013 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to the public mailing list <gmsh@geuz.org>.

#ifndef _HELP_WINDOW_H_
#define _HELP_WINDOW_H_

#include <FL/Fl_Window.H>
#include <FL/Fl_Browser.H>

class helpWindow{
 public:
  Fl_Window *about, *basic, *options;
  Fl_Check_Button *modified, *showhelp;
  Fl_Input *search;
  Fl_Browser *browser;
 public:
  helpWindow();
};

void help_options_cb(Fl_Widget *w, void *data);

#endif
