// Gmsh - Copyright (C) 1997-2008 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _GUI_H_
#define _GUI_H_

#include <vector>
#include <FL/Fl.H>

#define GMSH_WINDOW_BOX FL_FLAT_BOX
#define NB_BUTT_SCROLL 25
#define NB_HISTORY_MAX 1000
#define IW (10 * _fontsize)    // input field width
#define BB (7 * _fontsize)     // width of a button with internal label
#define BH (2 * _fontsize + 1) // button height
#define WB (7)                 // window border

class graphicWindow;
class menuWindow;
class optionWindow;
class fieldWindow;
class pluginWindow;
class statisticsWindow;
class visibilityWindow;
class clippingWindow;
class messageWindow;
class manipWindow;
class geometryContextWindow;
class meshContextWindow;
class aboutWindow;
class solverWindow;

class GUI{
 private:
  int _fontsize;
  static GUI *_instance;
 public:
  char onscreen_buffer[2][256];
  int selection, try_selection, quit_selection, end_selection;
  int undo_selection, invert_selection;
  int try_selection_xywh[4];
 public:
  std::vector<graphicWindow*> graph;
  menuWindow *menu;
  optionWindow *options;
  fieldWindow *fields;
  pluginWindow *plugins;
  statisticsWindow *stats;
  visibilityWindow *visibility;
  clippingWindow *clipping;
  messageWindow *messages;
  manipWindow *manip;
  geometryContextWindow *geoContext;
  meshContextWindow *meshContext;
  aboutWindow *about;
  std::vector<solverWindow*> solver;
 public:
  GUI(int argc, char **argv);
  ~GUI(){}
  // return the single static instance of the GUI
  static GUI *instance(int argc=0, char **argv=0);
  // check if the GUI is available
  static bool GUI::available(){ return (_instance != 0); }
  // run the GUI until there's no window left
  int run(){ return Fl::run(); }
  // check (now!) if there are any pending events, and process them
  void check(){ Fl::check(); }
  // wait (possibly indefinitely) for any events, then process them
  void wait(){ Fl::wait(); }
  // wait (at most time seconds) for any events, then process them
  void wait(double time){ Fl::wait(time); }
  // test application-level keyboard shortcuts
  int testGlobalShortcuts(int event);
  // test the arrow shortcuts (not in the application-level shortcuts)
  // since it is used elsewhere (where we want to override widget
  // navigation). This is necessary since FLTK 1.1.
  int testArrowShortcuts();
  // set the size of the graphic window
  void setGraphicSize(int w, int h);
  // set the title of the graphic window
  void setGraphicTitle(const char *str);
  // update the GUI when views get added or deleted
  void updateViews();
  // update the GUI when fields change
  void updateFields();
  // reset the visibility window
  void resetVisibility();
  // store current window positions and sizes in CTX
  void storeCurrentWindowsInfo();
  // display status message
  void setStatus(const char *msg, int num);
  // create the window for physical context dependant definitions
  void callForSolverPlugin(int dim);
};

// utility functions
int GetFontIndex(const char *fontname);
int GetFontEnum(int index);
const char *GetFontName(int index);
int GetFontAlign(const char *alignstr);
int GetFontSize();

class Fl_Browser;
void add_multiline_in_browser(Fl_Browser *o, const char *prefix, 
                              const char *str, int wrap=0);

#endif
