// Gmsh - Copyright (C) 1997-2011 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _ONELAB_WINDOW_H_
#define _ONELAB_WINDOW_H_

#include "GmshConfig.h"
#if defined(HAVE_FL_TREE)
#include <vector>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include "onelab.h"

class onelabWindow{
 private:
  Fl_Window *_win;
  Fl_Tree *_tree;
  Fl_Button *_butt[2];
  Fl_Input *_model;
  std::vector<Fl_Widget*> _treeWidgets;
 public:
  onelabWindow(int deltaFontSize=0);
  void rebuildTree();
  void redrawTree();
  void show(){ _win->show(); }
  std::string getModelName(){ return _model->value(); }
  void setModelName(const std::string &name){ _model->value(name.c_str()); }
  std::string getPath(Fl_Tree_Item *item)
  {
    char path[1024];
    _tree->item_pathname(path, 1024, item);
    return std::string(path);
  }
};

void onelab_cb(Fl_Widget *w, void *data);

#endif
#endif
