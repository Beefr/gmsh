// $Id: GUI.cpp,v 1.336 2004-08-16 17:52:59 remacle Exp $
//
// Copyright (C) 1997-2004 C. Geuzaine, J.-F. Remacle
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

// To make the interface as visually consistent as possible, please:
// - use the IW, BB, BH, BW and WB values
// - examine what's already done before adding something new...

// Which buttons+labels should you put on a dialog window?
// - "OK" is to agree with what is in the dialog *AND* close the dialog
// - "Apply" is to apply the current values selected in the dialog, but
//    leave the dialog open
// - "Cancel" is to close (hide) the dialog and *NOT* apply the
//    changes that might have been made in the dialog
//
// The "Cancel" button, if present, should always be the last (-> at
// right)

// Don't indent this file
// *INDENT-OFF*

#include "PluginManager.h"
#include "Gmsh.h"
#include "GmshUI.h"
#include "Numeric.h"
#include "GmshVersion.h"
#include "Context.h"
#include "Options.h"
#include "Geo.h"
#include "CAD.h"
#include "Mesh.h"
#include "Draw.h"
#include "GUI.h"
#include "Callbacks.h"
#include "Bitmaps.h"
#include "Win32Icon.h"
#include "OpenFile.h"
#include "CommandLine.h"
#include "Solvers.h"

#define WINDOW_BOX FL_FLAT_BOX
#define TOGGLE_BOX FL_DOWN_BOX
#define TOGGLE_COLOR FL_BLACK
#define RADIO_BOX  FL_ROUND_DOWN_BOX
#define RADIO_COLOR FL_BLACK

#define IW (10*fontsize)  // input field width
#define BB (7*fontsize)   // width of a button with internal label
#define BH (2*fontsize+1) // button height
#define WB (6)            // window border

extern Context_T CTX;

// Definition of the static menus

// We shouldn't use the 'g', 'm' 's' and 'p' mnemonics since they are
// already defined as global shortcuts (for geometry, mesh, solver, post).

Fl_Menu_Item m_menubar_table[] = {
  {"&File", 0, 0, 0, FL_SUBMENU},
    {"&New...",     FL_CTRL+'n', (Fl_Callback *)file_new_cb, 0},
    {"&Open...",    FL_CTRL+'o', (Fl_Callback *)file_open_cb, 0},
    {"M&erge...",   FL_CTRL+'m', (Fl_Callback *)file_merge_cb, 0, FL_MENU_DIVIDER},
    {"Sa&ve mesh",  FL_CTRL+'s', (Fl_Callback *)mesh_save_cb, 0},
    {"Save &as...", FL_CTRL+FL_SHIFT+'s', (Fl_Callback *)file_save_as_cb, 0, FL_MENU_DIVIDER},
    {"&Quit",       FL_CTRL+'q', (Fl_Callback *)file_quit_cb, 0},
    {0},
  {"&Tools", 0, 0, 0, FL_SUBMENU},
    {"&Options...",         FL_SHIFT+'o', (Fl_Callback *)options_cb, 0},
    {"&Visibility...",      FL_SHIFT+'v', (Fl_Callback *)visibility_cb, 0},
    {"&Clipping planes...", FL_SHIFT+'c', (Fl_Callback *)clip_cb, 0},
    {"S&tatistics...",      FL_SHIFT+'i', (Fl_Callback *)statistics_cb, 0, FL_MENU_DIVIDER},
    {"M&essage console...", FL_SHIFT+'l', (Fl_Callback *)message_cb, 0},
    {0},
  {"&Help", 0, 0, 0, FL_SUBMENU},
    {"&Current options...",      0, (Fl_Callback *)status_xyz1p_cb, (void*)5},
    {"M&ouse actions...",        0, (Fl_Callback *)help_mouse_cb, 0},
    {"&Keyboard shortcuts...",   0, (Fl_Callback *)help_short_cb, 0},
    {"C&ommand line options...", 0, (Fl_Callback *)help_command_line_cb, 0, FL_MENU_DIVIDER},
    {"On&line documentation",    0, (Fl_Callback *)help_online_cb, 0, FL_MENU_DIVIDER},
    {"&About...",                0, (Fl_Callback *)help_about_cb, 0},
    {0},
  {0}
};

// Alternative items for the MacOS system menu bar (removed '&'
// shortcuts: they would cause spurious menu itmes to appear on the
// menu window; removed File->Quit; changed capitalization to match
// Apple's guidelines)

#if defined(__APPLE__) && defined(HAVE_FL_SYS_MENU_BAR)
Fl_Menu_Item m_sys_menubar_table[] = {
  {"File", 0, 0, 0, FL_SUBMENU},
    {"New...",     FL_CTRL+'n', (Fl_Callback *)file_new_cb, 0},
    {"Open...",    FL_CTRL+'o', (Fl_Callback *)file_open_cb, 0},
    {"Merge...",   FL_CTRL+'m', (Fl_Callback *)file_merge_cb, 0, FL_MENU_DIVIDER},
    {"Save Mesh",  FL_CTRL+'s', (Fl_Callback *)mesh_save_cb, 0},
    {"Save As...", FL_CTRL+FL_SHIFT+'s', (Fl_Callback *)file_save_as_cb, 0},
    {0},
  {"Tools",0,0,0,FL_SUBMENU},
    {"Options...",         FL_SHIFT+'o', (Fl_Callback *)options_cb, 0},
    {"Visibility...",      FL_SHIFT+'v', (Fl_Callback *)visibility_cb, 0},
    {"Clipping Planes...", FL_SHIFT+'c', (Fl_Callback *)clip_cb, 0},
    {"Statistics...",      FL_SHIFT+'i', (Fl_Callback *)statistics_cb, 0, FL_MENU_DIVIDER},
    {"Message Console...", FL_SHIFT+'l', (Fl_Callback *)message_cb, 0},
    {0},
  {"Help",0,0,0,FL_SUBMENU},
    {"Current Options...",      0, (Fl_Callback *)status_xyz1p_cb, (void*)5},
    {"Mouse Actions...",        0, (Fl_Callback *)help_mouse_cb, 0},
    {"Keyboard Shortcuts...",   0, (Fl_Callback *)help_short_cb, 0},
    {"Command Line Options...", 0, (Fl_Callback *)help_command_line_cb, 0, FL_MENU_DIVIDER},
    {"Online Documentation",    0, (Fl_Callback *)help_online_cb, 0, FL_MENU_DIVIDER},
    {"About Gmsh...",           0, (Fl_Callback *)help_about_cb, 0},
    {0},
  {0}
};
#endif

Fl_Menu_Item m_module_table[] = {
  {"Geometry",        'g', (Fl_Callback *)mod_geometry_cb, 0},
  {"Mesh",            'm', (Fl_Callback *)mod_mesh_cb, 0},
  {"Solver",          's', (Fl_Callback *)mod_solver_cb, 0},
  {"Post-processing", 'p', (Fl_Callback *)mod_post_cb, 0},
  {0}
};

// Definition of the dynamic contexts

Context_Item menu_geometry[] = {
  { "0Geometry", NULL } ,
  { "Elementary", (Fl_Callback *)geometry_elementary_cb } ,
  { "Physical",   (Fl_Callback *)geometry_physical_cb } ,
  { "Edit",       (Fl_Callback *)geometry_edit_cb } , 
  { "Reload",     (Fl_Callback *)geometry_reload_cb } , 
  { NULL }
};  
    Context_Item menu_geometry_elementary[] = {
      { "0Geometry > Elementary", NULL } ,
      { "Add",       (Fl_Callback *)geometry_elementary_add_cb } ,
      { "Translate", (Fl_Callback *)geometry_elementary_translate_cb } ,
      { "Rotate",    (Fl_Callback *)geometry_elementary_rotate_cb } ,
      { "Scale",     (Fl_Callback *)geometry_elementary_scale_cb } ,
      { "Symmetry",  (Fl_Callback *)geometry_elementary_symmetry_cb } ,
      { "Extrude",   (Fl_Callback *)geometry_elementary_extrude_cb } ,
      { "Delete",    (Fl_Callback *)geometry_elementary_delete_cb } ,
      { NULL } 
    };  
        Context_Item menu_geometry_elementary_add[] = {
	  { "0Geometry > Elementary > Add", NULL } ,
          { "New",       (Fl_Callback *)geometry_elementary_add_new_cb } ,
	  { "Translate", (Fl_Callback *)geometry_elementary_add_translate_cb } ,
	  { "Rotate",    (Fl_Callback *)geometry_elementary_add_rotate_cb } ,
	  { "Scale",     (Fl_Callback *)geometry_elementary_add_scale_cb } ,
	  { "Symmetry",  (Fl_Callback *)geometry_elementary_add_symmetry_cb } ,
	  { NULL } 
	};  
            Context_Item menu_geometry_elementary_add_new[] = {
	      { "0Geometry > Elementary > Add > New", NULL } ,
              { "Parameter",     (Fl_Callback *)geometry_elementary_add_new_parameter_cb } ,
	      { "Point",         (Fl_Callback *)geometry_elementary_add_new_point_cb } ,
	      { "Straight line", (Fl_Callback *)geometry_elementary_add_new_line_cb } ,
	      { "Spline",        (Fl_Callback *)geometry_elementary_add_new_spline_cb } ,
	      { "B-Spline",      (Fl_Callback *)geometry_elementary_add_new_bspline_cb } ,
	      { "Circle arc",    (Fl_Callback *)geometry_elementary_add_new_circle_cb } ,
	      { "Ellipse arc",   (Fl_Callback *)geometry_elementary_add_new_ellipse_cb } ,
	      { "Plane surface", (Fl_Callback *)geometry_elementary_add_new_planesurface_cb } ,
	      { "Ruled surface", (Fl_Callback *)geometry_elementary_add_new_ruledsurface_cb } ,
	      { "Volume",        (Fl_Callback *)geometry_elementary_add_new_volume_cb } ,
	      { NULL } 
	    };  
            Context_Item menu_geometry_elementary_add_translate[] = {
	      { "0Geometry > Elementary > Add > Translate", NULL } ,
              { "Point",   (Fl_Callback *)geometry_elementary_add_translate_point_cb } ,
	      { "Line",    (Fl_Callback *)geometry_elementary_add_translate_line_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_add_translate_surface_cb } ,
	      { NULL } 
	    };  
            Context_Item menu_geometry_elementary_add_rotate[] = {
	      { "0Geometry > Elementary > Add > Rotate", NULL } ,
              { "Point",   (Fl_Callback *)geometry_elementary_add_rotate_point_cb } ,
	      { "Line",    (Fl_Callback *)geometry_elementary_add_rotate_line_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_add_rotate_surface_cb } ,
	      { NULL } 
	    };  
            Context_Item menu_geometry_elementary_add_scale[] = {
	      { "0Geometry > Elementary > Add > Scale", NULL } ,
	      { "Point",   (Fl_Callback *)geometry_elementary_add_scale_point_cb } ,
	      { "Line",    (Fl_Callback *)geometry_elementary_add_scale_line_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_add_scale_surface_cb } ,
	      { NULL } 
	    };  
            Context_Item menu_geometry_elementary_add_symmetry[] = {
	      { "0Geometry > Elementary > Add > Symmetry", NULL } ,
	      { "Point",   (Fl_Callback *)geometry_elementary_add_symmetry_point_cb } ,
	      { "Line",    (Fl_Callback *)geometry_elementary_add_symmetry_line_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_add_symmetry_surface_cb } ,
	      { NULL } 
	    };  
        Context_Item menu_geometry_elementary_translate[] = {
	  { "0Geometry > Elementary > Translate", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_elementary_translate_point_cb } ,
	  { "Line",    (Fl_Callback *)geometry_elementary_translate_line_cb } ,
	  { "Surface", (Fl_Callback *)geometry_elementary_translate_surface_cb } ,
	  { NULL } 
	};  
        Context_Item menu_geometry_elementary_rotate[] = {
	  { "0Geometry > Elementary > Rotate", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_elementary_rotate_point_cb } ,
	  { "Line",    (Fl_Callback *)geometry_elementary_rotate_line_cb } ,
	  { "Surface", (Fl_Callback *)geometry_elementary_rotate_surface_cb } ,
	  { NULL } 
	};  
        Context_Item menu_geometry_elementary_scale[] = {
	  { "0Geometry > Elementary > Scale", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_elementary_scale_point_cb } ,
	  { "Line",    (Fl_Callback *)geometry_elementary_scale_line_cb } ,
	  { "Surface", (Fl_Callback *)geometry_elementary_scale_surface_cb } ,
	  { NULL } 
	};  
        Context_Item menu_geometry_elementary_symmetry[] = {
	  { "0Geometry > Elementary > Symmetry", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_elementary_symmetry_point_cb } ,
	  { "Line",    (Fl_Callback *)geometry_elementary_symmetry_line_cb } ,
	  { "Surface", (Fl_Callback *)geometry_elementary_symmetry_surface_cb } ,
	  { NULL } 
	};  
        Context_Item menu_geometry_elementary_extrude[] = {
	  { "0Geometry > Elementary > Extrude", NULL } ,
	  { "Translate", (Fl_Callback *)geometry_elementary_extrude_translate_cb } ,
	  { "Rotate",    (Fl_Callback *)geometry_elementary_extrude_rotate_cb } ,
	  { NULL } 
 	};  
            Context_Item menu_geometry_elementary_extrude_translate[] = {
	      { "0Geometry > Elementary > Extrude > Translate", NULL } ,
	      { "Point",   (Fl_Callback *)geometry_elementary_extrude_translate_point_cb } ,
	      { "Line",    (Fl_Callback *)geometry_elementary_extrude_translate_line_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_extrude_translate_surface_cb } ,
	      { NULL } 
	    };  
            Context_Item menu_geometry_elementary_extrude_rotate[] = {
	      { "0Geometry > Elementary > Extrude > Rotate", NULL } ,
	      { "Point",   (Fl_Callback *)geometry_elementary_extrude_rotate_point_cb } ,
	      { "Line",    (Fl_Callback *)geometry_elementary_extrude_rotate_line_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_extrude_rotate_surface_cb } ,
	      { NULL } 
	    };  
        Context_Item menu_geometry_elementary_delete[] = {
	  { "0Geometry > Elementary > Delete", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_elementary_delete_point_cb } ,
	  { "Line",    (Fl_Callback *)geometry_elementary_delete_line_cb } ,
	  { "Surface", (Fl_Callback *)geometry_elementary_delete_surface_cb } ,
	  { NULL } 
	};  
    Context_Item menu_geometry_physical[] = {
      { "0Geometry > Physical", NULL } ,
      { "Add",    (Fl_Callback *)geometry_physical_add_cb } ,
      { NULL } 
    };  
        Context_Item menu_geometry_physical_add[] = {
	  { "0Geometry > Physical > Add", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_physical_add_point_cb  } ,
	  { "Line",    (Fl_Callback *)geometry_physical_add_line_cb  } ,
	  { "Surface", (Fl_Callback *)geometry_physical_add_surface_cb  } ,
	  { "Volume",  (Fl_Callback *)geometry_physical_add_volume_cb  } ,
	  { NULL } 
	};  

Context_Item menu_mesh[] = {
  { "1Mesh", NULL } ,
  { "Define", (Fl_Callback *)mesh_define_cb } ,
  { "1D",     (Fl_Callback *)mesh_1d_cb } ,
  { "2D",     (Fl_Callback *)mesh_2d_cb } , 
  { "3D",     (Fl_Callback *)mesh_3d_cb } , 
  { "First order",  (Fl_Callback *)mesh_degree_cb, (void*)1 } , 
  { "Second order", (Fl_Callback *)mesh_degree_cb, (void*)2 } , 
#if defined(HAVE_NETGEN)
  { "Optimize quality", (Fl_Callback *)mesh_optimize_cb } , 
#endif
  { "Save",   (Fl_Callback *)mesh_save_cb } ,
  { NULL } 
};  
    Context_Item menu_mesh_define[] = {
      { "1Mesh > Define", NULL } ,
      { "Characteristic length", (Fl_Callback *)mesh_define_length_cb  } ,
      { "Recombine",   (Fl_Callback *)mesh_define_recombine_cb  } ,
      { "Transfinite", (Fl_Callback *)mesh_define_transfinite_cb  } , 
      { NULL } 
    };  
        Context_Item menu_mesh_define_transfinite[] = {
	  { "1Mesh > Define > Transfinite", NULL } ,
	  { "Line",    (Fl_Callback *)mesh_define_transfinite_line_cb } ,
	  { "Surface", (Fl_Callback *)mesh_define_transfinite_surface_cb } ,
	  { "Volume",  (Fl_Callback *)mesh_define_transfinite_volume_cb } , 
	  { NULL } 
	};  

// should create MAXSOLVERS items...
Context_Item menu_solver[] = {
  { "2Solver", NULL } ,
  { "Solver 0", (Fl_Callback *)solver_cb , (void*)0} ,
  { "Solver 1", (Fl_Callback *)solver_cb , (void*)1} ,
  { "Solver 2", (Fl_Callback *)solver_cb , (void*)2} ,
  { "Solver 3", (Fl_Callback *)solver_cb , (void*)3} ,
  { "Solver 4", (Fl_Callback *)solver_cb , (void*)4} ,
  { NULL } 
};

Context_Item menu_post[] = {
  { "3Post-processing", NULL } ,
  { NULL } 
};

// some other reusable menus

static Fl_Menu_Item menu_point_display[] = {
  {"Color dot",   0, 0, 0},
  {"3D sphere",   0, 0, 0},
  {0}
};

static Fl_Menu_Item menu_point_display_with_plugin[] = {
  {"Color dot",   0, 0, 0},
  {"3D sphere",   0, 0, 0},
  {"Use Solver Plugin",   0, 0, 0},
  {0}
};

static Fl_Menu_Item menu_line_display[] = {
  {"Color segment", 0, 0, 0},
  {"3D cylinder",   0, 0, 0},
  {0}
};

static Fl_Menu_Item menu_line_display_with_plugin[] = {
  {"Color segment", 0, 0, 0},
  {"3D cylinder",   0, 0, 0},
  {"Use Solver Plugin",   0, 0, 0},
  {0}
};

Fl_Menu_Item menu_font_names[] = {
  {"Times-Roman",           0, 0, (void*)FL_TIMES},
  {"Times-Bold",            0, 0, (void*)FL_TIMES_BOLD},
  {"Times-Italic",          0, 0, (void*)FL_TIMES_ITALIC},
  {"Times-BoldItalic",      0, 0, (void*)FL_TIMES_BOLD_ITALIC},
  {"Helvetica",             0, 0, (void*)FL_HELVETICA},
  {"Helvetica-Bold",        0, 0, (void*)FL_HELVETICA_BOLD},
  {"Helvetica-Oblique",     0, 0, (void*)FL_HELVETICA_ITALIC},
  {"Helvetica-BoldOblique", 0, 0, (void*)FL_HELVETICA_BOLD_ITALIC},
  {"Courier",               0, 0, (void*)FL_COURIER},
  {"Courier-Bold",          0, 0, (void*)FL_COURIER_BOLD},
  {"Courier-Oblique",       0, 0, (void*)FL_COURIER_ITALIC},
  {"Courier-BoldOblique",   0, 0, (void*)FL_COURIER_BOLD_ITALIC},
  {"Symbol",                0, 0, (void*)FL_SYMBOL},
  {"ZapfDingbats",          0, 0, (void*)FL_ZAPF_DINGBATS},
  {0}
};

// Definition of global shortcuts

int GUI::global_shortcuts(int event)
{
  int i, j;

  // we only handle shortcuts here
  if(event != FL_SHORTCUT)
    return 0;

  if(Fl::test_shortcut(FL_SHIFT + FL_Escape) ||
     Fl::test_shortcut(FL_CTRL + FL_Escape) ||
     Fl::test_shortcut(FL_ALT + FL_Escape)) {
    return 1;
  }

  if(Fl::test_shortcut('0') || Fl::test_shortcut(FL_Escape)) {
    geometry_reload_cb(0, 0);
    mod_geometry_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut('1') || Fl::test_shortcut(FL_F + 1)) {
    mesh_1d_cb(0, 0);
    mod_mesh_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut('2') || Fl::test_shortcut(FL_F + 2)) {
    mesh_2d_cb(0, 0);
    mod_mesh_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut('3') || Fl::test_shortcut(FL_F + 3)) {
    mesh_3d_cb(0, 0);
    mod_mesh_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut(FL_CTRL + 'q')){
    // only necessary when using the system menu bar, but hey, it
    // cannot hurt...
    file_quit_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut('g')) {
    mod_geometry_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut('m')) {
    mod_mesh_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut('s')) {
    mod_solver_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut('p')) {
    mod_post_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut('<')) {
    mod_back_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut('>')) {
    mod_forward_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut('e')) {
    end_selection = 1;
    return 0;   // trick: do as if we didn't use it
  }
  else if(Fl::test_shortcut('u')) {
    undo_selection = 1;
    return 0;   // trick: do as if we didn't use it
  }
  else if(Fl::test_shortcut('q')) {
    quit_selection = 1;
    return 0;   // trick: do as if we didn't use it
  }
  else if(Fl::test_shortcut(FL_SHIFT + 'a')) { 
    // raise all open windows (graphics first, then options, then menu)
    if(g_window && g_window->shown()) g_window->show();
    if(opt_window && opt_window->shown()) opt_window->show();
    if(vis_window && vis_window->shown()) vis_window->show();
    if(stat_window && stat_window->shown()) stat_window->show();
    if(msg_window && msg_window->shown()) msg_window->show();
    if(m_window && m_window->shown()) m_window->show();
    return 1;
  }
  else if(Fl::test_shortcut(FL_SHIFT + 'n')) {
    general_options_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut(FL_SHIFT + 'g')) {
    geometry_options_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut(FL_SHIFT + 'm')) {
    mesh_options_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut(FL_SHIFT + 's')) {
    solver_options_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut(FL_SHIFT + 'p')) {
    post_options_cb(0, 0);
    return 1;
  }
  else if(Fl::test_shortcut(FL_SHIFT + 'w')) {
    if(List_Nbr(CTX.post.list))
      if(view_number >= 0 && view_number < List_Nbr(CTX.post.list))
	create_view_options_window(view_number);
      else
	create_view_options_window(0);
    return 1;
  }
  else if(Fl::test_shortcut(FL_CTRL + FL_SHIFT + 'd')) {
    opt_post_anim_delay(0, GMSH_SET | GMSH_GUI,
                        opt_post_anim_delay(0, GMSH_GET, 0) + 0.01);
    return 1;
  }
  else if(Fl::test_shortcut(FL_SHIFT + 'd')) {
    opt_post_anim_delay(0, GMSH_SET | GMSH_GUI,
                        opt_post_anim_delay(0, GMSH_GET, 0) - 0.01);
    return 1;
  }
  else if(Fl::test_shortcut(FL_CTRL + 'z')) {
    g_window->iconize();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'f')) {
    opt_general_fast_redraw(0, GMSH_SET | GMSH_GUI,
			    !opt_general_fast_redraw(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'b')) {
    opt_general_draw_bounding_box(0, GMSH_SET | GMSH_GUI,
				  !opt_general_draw_bounding_box(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'i')) {
    opt_post_scales(0, GMSH_SET | GMSH_GUI, !opt_post_scales(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'o')) {
    opt_general_orthographic(0, GMSH_SET | GMSH_GUI,
                             !opt_general_orthographic(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'c')) {
    opt_general_color_scheme(0, GMSH_SET | GMSH_GUI,
                             opt_general_color_scheme(0, GMSH_GET, 0) + 1);
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'd')) {
    opt_mesh_surfaces_faces(0, GMSH_SET | GMSH_GUI,
			    !opt_mesh_surfaces_faces(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'w')) {
    opt_geometry_light(0, GMSH_SET | GMSH_GUI,
		       !opt_geometry_light(0, GMSH_GET, 0));
    opt_mesh_light(0, GMSH_SET | GMSH_GUI,
		   !opt_mesh_light(0, GMSH_GET, 0));
    for(i = 0; i < List_Nbr(CTX.post.list); i++)
      opt_view_light(i, GMSH_SET | GMSH_GUI,
		     !opt_view_light(i, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'x')) {
    status_xyz1p_cb(0, (void *)0);
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'y')) {
    status_xyz1p_cb(0, (void *)1);
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'z')) {
    status_xyz1p_cb(0, (void *)2);
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + FL_SHIFT + 'a')) {
    opt_general_axes(0, GMSH_SET | GMSH_GUI,
                     !opt_general_axes(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'a')) {
    opt_general_small_axes(0, GMSH_SET | GMSH_GUI,
                           !opt_general_small_axes(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'p')) {
    opt_geometry_points(0, GMSH_SET | GMSH_GUI,
                        !opt_geometry_points(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'l')) {
    opt_geometry_lines(0, GMSH_SET | GMSH_GUI,
                       !opt_geometry_lines(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 's')) {
    opt_geometry_surfaces(0, GMSH_SET | GMSH_GUI,
                          !opt_geometry_surfaces(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'v')) {
    opt_geometry_volumes(0, GMSH_SET | GMSH_GUI,
                         !opt_geometry_volumes(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + FL_SHIFT + 'p')) {
    opt_mesh_points(0, GMSH_SET | GMSH_GUI, !opt_mesh_points(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + FL_SHIFT + 'l')) {
    opt_mesh_surfaces_edges(0, GMSH_SET | GMSH_GUI, 
			    !opt_mesh_surfaces_edges(0, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + FL_SHIFT + 's')) {
    int old = opt_mesh_surfaces_edges(0, GMSH_GET, 0) || opt_mesh_surfaces_faces(0, GMSH_GET, 0);
    opt_mesh_surfaces_edges(0, GMSH_SET | GMSH_GUI, !old);
    opt_mesh_surfaces_faces(0, GMSH_SET | GMSH_GUI, !old);
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + FL_SHIFT + 'v')) {
    int old = opt_mesh_volumes_edges(0, GMSH_GET, 0) || opt_mesh_volumes_faces(0, GMSH_GET, 0);
    opt_mesh_volumes_edges(0, GMSH_SET | GMSH_GUI, !old);
    opt_mesh_volumes_faces(0, GMSH_SET | GMSH_GUI, !old);
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'm')) {
    int old = opt_mesh_points(0, GMSH_GET, 0) || opt_mesh_lines(0, GMSH_GET, 0) ||
      opt_mesh_surfaces_edges(0, GMSH_GET, 0) || opt_mesh_surfaces_faces(0, GMSH_GET, 0);
    opt_mesh_points(0, GMSH_SET | GMSH_GUI, !old);
    opt_mesh_lines(0, GMSH_SET | GMSH_GUI, !old);
    opt_mesh_surfaces_edges(0, GMSH_SET | GMSH_GUI, !old);
    opt_mesh_surfaces_faces(0, GMSH_SET | GMSH_GUI, !old);
    opt_mesh_volumes_edges(0, GMSH_SET | GMSH_GUI, !old);
    opt_mesh_volumes_faces(0, GMSH_SET | GMSH_GUI, !old);
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 't')) {
    for(i = 0; i < List_Nbr(CTX.post.list); i++) {
      if(opt_view_visible(i, GMSH_GET, 0)) {
        j = (int)opt_view_intervals_type(i, GMSH_GET, 0);
        opt_view_intervals_type(i, GMSH_SET | GMSH_GUI,
                                (j == DRAW_POST_ISO) ? DRAW_POST_DISCRETE :
                                (j == DRAW_POST_DISCRETE) ? DRAW_POST_CONTINUOUS :
                                DRAW_POST_ISO);
      }
    }
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'n')) {
    for(i = 0; i < List_Nbr(CTX.post.list); i++)
      if(opt_view_visible(i, GMSH_GET, 0))
        opt_view_draw_strings(i, GMSH_SET | GMSH_GUI,
                              !opt_view_draw_strings(i, GMSH_GET, 0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT + 'h')) {
    static int show = 0;
    for(i = 0; i < List_Nbr(CTX.post.list); i++)
      opt_view_visible(i, GMSH_SET | GMSH_GUI, show);
    redraw_opengl();
    show = !show;
    return 1;
  }
  else if(arrow_shortcuts()) {
    return 1;
  }

  return 0;
}

// Test the arrow shortcuts (this is not in the global_shortcuts)
// since it is used elsewhere (where we want to override widget
// navigation: necessary since FLTK>=1.1)

int GUI::arrow_shortcuts()
{
  if(Fl::test_shortcut(FL_Left)) {
    ManualPlay(1, -1);
    return 1;
  }
  else if(Fl::test_shortcut(FL_Right)) {
    ManualPlay(1, 1);
    return 1;
  }
  else if(Fl::test_shortcut(FL_Up)) {
    ManualPlay(0, -1);
    return 1;
  }
  else if(Fl::test_shortcut(FL_Down)) {
    ManualPlay(0, 1);
    return 1;
  }
  return 0;
}

// The GUI constructor

GUI::GUI(int argc, char **argv)
{
  int i;

  // initialize static windows
  m_window = NULL;
  g_window = NULL;
  opt_window = NULL;
  stat_window = NULL;
  msg_window = NULL;
  vis_window = NULL;
  clip_window = NULL;
  about_window = NULL;
  context_geometry_window = NULL;
  context_mesh_window = NULL;

  // initialize on-screen message buffer
  onscreen_buffer[0][0] = '\0';
  onscreen_buffer[1][0] = '\0';

  // set X display
  if(strlen(CTX.display))
    Fl::display(CTX.display);

  // add global shortcuts
  Fl::add_handler(SetGlobalShortcut);

  // store fontsize now: we don't want any subsequent change
  // (e.g. when doing a 'restore options') to be taken into account
  // in the dynamic GUI features (set_context, plugin, etc.)
  fontsize = CTX.fontsize;

  // set default font size
  FL_NORMAL_SIZE = fontsize;

  // handle themes and tooltip font size
  if(strlen(CTX.scheme))
    Fl::scheme(CTX.scheme);
  Fl_Tooltip::size(fontsize);

  // add callback to respond to the Mac Finder (when you click on a
  // document)
#if defined(__APPLE__)
  fl_open_callback(OpenProblemMacFinder);
#endif

  // All static windows are contructed (even if some are not
  // displayed) since the shortcuts should be valid even for hidden
  // windows, and we don't want to test for widget existence every time

  create_menu_window(argc, argv);
  create_graphic_window(argc, argv);

#if defined(WIN32)
  m_window->icon((char *)LoadImage(fl_display, MAKEINTRESOURCE(IDI_ICON),
                                   IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
#elif defined(__APPLE__)
  // Nothing to do here
#else
  fl_open_display();
  Pixmap p1 = XCreateBitmapFromData(fl_display, DefaultRootWindow(fl_display),
                                    gmsh32x32_bits, gmsh32x32_width,
                                    gmsh32x32_height);
  Pixmap p2 = XCreateBitmapFromData(fl_display, DefaultRootWindow(fl_display),
                                    gmsh32x32_bits, gmsh32x32_width,
                                    gmsh32x32_height);
  m_window->icon((char *)p1);
  g_window->icon((char *)p2);
#endif

  m_window->show(1, argv);
  g_window->show(1, argv);

  create_option_window();
  create_message_window();
  create_statistics_window();
  create_visibility_window();
  create_clip_window();
  create_about_window();
  create_geometry_context_window(0);
  call_for_solver_plugin (-1);
  create_mesh_context_window(0);
  for(i = 0; i < MAXSOLVERS; i++) {
    solver[i].window = NULL;
    create_solver_window(i);
  }

  // Draw the scene
  g_opengl_window->redraw();
}

// Run the GUI until no window is left

int GUI::run()
{
  return Fl::run();
}

// Check (now) if any pending events and run them

void GUI::check()
{
  Fl::check();
}

// Wait for any events and run them

void GUI::wait()
{
  Fl::wait();
}

// Create the menu window


void GUI::add_post_plugins(Fl_Menu_Button * button, int iView)
{
  char name[256], menuname[256];
  for(GMSH_PluginManager::iter it = GMSH_PluginManager::instance()->begin();
      it != GMSH_PluginManager::instance()->end(); ++it) {
    GMSH_Plugin *p = (*it).second;
    if(p->getType() == GMSH_Plugin::GMSH_POST_PLUGIN) {
      p->getName(name);
      std::pair < int, GMSH_Plugin * >*pair = new std::pair < int, GMSH_Plugin * >(iView, p);
      sprintf(menuname, "Plugins/%s...", name);
      button->add(menuname, 0, (Fl_Callback *) view_options_plugin_cb, (void *)(pair), 0);
      p->dialogBox = 0;
    }
  }
}

void GUI::create_menu_window(int argc, char **argv)
{
  int i, y;

  if(m_window) {
    m_window->show(1, argv);
    return;
  }

  int width = 13 * fontsize - fontsize / 2 - 2;

  // this is the initial height: no dynamic button is shown!
#if defined(__APPLE__) && defined(HAVE_FL_SYS_MENU_BAR)
  if(CTX.system_menu_bar){
    MH = BH + 6;  // the menu bar is not in the application!
  }
  else{
#endif
    MH = BH + BH + 6;
#if defined(__APPLE__) && defined(HAVE_FL_SYS_MENU_BAR)
  }
#endif

  m_window = new Fl_Window(width, MH, "Gmsh");
  m_window->box(WINDOW_BOX);
  m_window->callback(file_quit_cb);

#if defined(__APPLE__) && defined(HAVE_FL_SYS_MENU_BAR)
  if(CTX.system_menu_bar){
    // the system menubar is kind of a hack in fltk--it still creates
    // a real (invisible) menubar. So we make it a 1x1 pixel rectangle
    // (and 1 pixel off the edge, so it falls behind the navigation
    // buttons) to really really really avoid unwanted mouse click
    // events.
    m_sys_menu_bar = new Fl_Sys_Menu_Bar(1, 1, 1, 1);
    m_sys_menu_bar->menu(m_sys_menubar_table);
    m_sys_menu_bar->global();
    Fl_Box *o = new Fl_Box(0, 0, width, BH + 6);
    o->box(FL_UP_BOX);
    y = 3;
  }
  else{
#endif
    m_menu_bar = new Fl_Menu_Bar(0, 0, width, BH);
    m_menu_bar->menu(m_menubar_table);
    m_menu_bar->box(FL_UP_BOX);
    m_menu_bar->global();
    Fl_Box *o = new Fl_Box(0, BH, width, BH + 6);
    o->box(FL_UP_BOX);
    y = BH + 3;
#if defined(__APPLE__) && defined(HAVE_FL_SYS_MENU_BAR)
  }
#endif

  m_navig_butt[0] = new Fl_Button(1, y, 18, BH / 2, "@#<");
  m_navig_butt[0]->labeltype(FL_SYMBOL_LABEL);
  m_navig_butt[0]->labelsize(11);
  m_navig_butt[0]->box(FL_FLAT_BOX);
  m_navig_butt[0]->selection_color(FL_WHITE);
  m_navig_butt[0]->callback(mod_back_cb);
  m_navig_butt[0]->tooltip("Go back one in the menu history (<)");

  m_navig_butt[1] = new Fl_Button(1, y + BH / 2, 18, BH / 2, "@#>");
  m_navig_butt[1]->labeltype(FL_SYMBOL_LABEL);
  m_navig_butt[1]->labelsize(11);
  m_navig_butt[1]->box(FL_FLAT_BOX);
  m_navig_butt[1]->selection_color(FL_WHITE);
  m_navig_butt[1]->callback(mod_forward_cb);
  m_navig_butt[1]->tooltip("Go forward one in the menu history (>)");

  m_module_butt = new Fl_Choice(19, y, width - 24, BH);
  m_module_butt->menu(m_module_table);
  m_module_butt->box(FL_THIN_DOWN_BOX);
  // force the executation of the callback even if we didn't change
  // the selection (we want to go back to the top-level menu every
  // time we select one of the categories, even if the category is not
  // changed):
  m_module_butt->when(FL_WHEN_RELEASE_ALWAYS);
  y = MH;

  for(i = 0; i < NB_BUTT_MAX; i++) {
    m_push_butt[i] = new Fl_Button(0, y + i * BH, width, BH);
    m_push_butt[i]->hide();

    m_toggle_butt[i] = new Fl_Light_Button(0, y + i * BH, width - (fontsize + 4), BH);
    m_toggle_butt[i]->callback(view_toggle_cb, (void *)i);
    m_toggle_butt[i]->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    m_toggle_butt[i]->hide();

    m_toggle2_butt[i] = new Fl_Button(width - (fontsize + 4), y + i * BH, (fontsize + 4), BH, "@#>");
    m_toggle2_butt[i]->labeltype(FL_SYMBOL_LABEL);
    m_toggle2_butt[i]->labelsize(11);
    m_toggle2_butt[i]->align(FL_ALIGN_CENTER);
    m_toggle2_butt[i]->hide();
    m_toggle2_butt[i]->tooltip("Show view option menu (Shift+w)");

    m_popup_butt[i] = new Fl_Menu_Button(width - (fontsize + 4), y + i * BH, (fontsize + 4), BH);
    m_popup_butt[i]->type(Fl_Menu_Button::POPUP123);

    m_popup2_butt[i] = new Fl_Menu_Button(0, y + i * BH, width - (fontsize + 4), BH);
    m_popup2_butt[i]->type(Fl_Menu_Button::POPUP3);

    for(int j = 0; j < 2; j++) {
      Fl_Menu_Button *pop = j ? m_popup2_butt[i] : m_popup_butt[i];
      pop->add("Reload/View", 0, (Fl_Callback *) view_reload_cb, (void *)i, 0);
      pop->add("Reload/All views", 0, (Fl_Callback *) view_reload_all_cb, (void *)i, 0);
      pop->add("Reload/All visible views", 0, (Fl_Callback *) view_reload_visible_cb, (void *)i, 0);
      pop->add("Remove/View", FL_Delete, (Fl_Callback *) view_remove_cb, (void *)i, 0);
      pop->add("Remove/All views", 0, (Fl_Callback *) view_remove_all_cb, (void *)i, 0);
      pop->add("Remove/All visible views", 0, (Fl_Callback *) view_remove_visible_cb, (void *)i, 0);
      pop->add("Remove/All invisible views", 0, (Fl_Callback *) view_remove_invisible_cb, (void *)i, 0);
      pop->add("Duplicate/View without options", 0, (Fl_Callback *) view_duplicate_cb, (void *)i, 0);
      pop->add("Duplicate/View with options", 0, (Fl_Callback *) view_duplicate_with_options_cb, (void *)i, 0);
      pop->add("Combine/Elements/From all views", 0, 
	       (Fl_Callback *) view_combine_all_cb, (void *)i, 0);
      pop->add("Combine/Elements/From all views (and remove originals)", 0, 
	       (Fl_Callback *) view_combine_all_and_remove_cb, (void *)i, 0);
      pop->add("Combine/Elements/From visible views", 0, 
	       (Fl_Callback *) view_combine_visible_cb, (void *)i, 0);
      pop->add("Combine/Elements/From visible views (and remove originals)", 0, 
	       (Fl_Callback *) view_combine_visible_and_remove_cb, (void *)i, 0);
      pop->add("Combine/Time steps/From all views", 0, 
	       (Fl_Callback *) view_combine_time_all_cb, (void *)i, 0);
      pop->add("Combine/Time steps/From all views (and remove originals)", 0, 
	       (Fl_Callback *) view_combine_time_all_and_remove_cb, (void *)i, 0);
      pop->add("Combine/Time steps/From visible views", 0, 
	       (Fl_Callback *) view_combine_time_visible_cb, (void *)i, 0);
      pop->add("Combine/Time steps/From visible views (and remove originals)", 0, 
	       (Fl_Callback *) view_combine_time_visible_and_remove_cb, (void *)i, 0);
      pop->add("Combine/Time steps/By view name", 0, 
	       (Fl_Callback *) view_combine_time_by_name_cb, (void *)i, 0);
      pop->add("Combine/Time steps/By view name (and remove originals)", 0, 
	       (Fl_Callback *) view_combine_time_by_name_and_remove_cb, (void *)i, 0);
      pop->add("Save as/ASCII view...", 0, (Fl_Callback *) view_save_ascii_cb, (void *)i, 0);
      pop->add("Save as/Binary view...", 0, (Fl_Callback *) view_save_binary_cb, (void *)i, 0);
      add_post_plugins(pop, i);
      pop->add("Apply as background mesh", 0, (Fl_Callback *) view_applybgmesh_cb, (void *)i, FL_MENU_DIVIDER);
      pop->add("Options...", 'o', (Fl_Callback *) view_options_cb, (void *)i, 0);
      pop->hide();
    }
  }
  m_window->position(CTX.position[0], CTX.position[1]);
  m_window->end();
}

// Dynamically set the height of the menu window

void GUI::set_menu_size(int nb_butt)
{
  m_window->size(m_window->w(), MH + nb_butt * BH);
}

// Dynamically set the context

void GUI::set_context(Context_Item * menu_asked, int flag)
{
  static int nb_back = 0, nb_forward = 0, init_context = 0;
  static Context_Item *menu_history[NB_HISTORY_MAX];
  Context_Item *menu;
  Post_View *v;
  int i;

  if(!init_context) {
    init_context = 1;
    for(i = 0; i < NB_HISTORY_MAX; i++) {
      menu_history[i] = NULL;
    }
  }

  if(nb_back > NB_HISTORY_MAX - 2)
    nb_back = 1;        // we should do a circular list

  if(flag == -1) {
    if(nb_back > 1) {
      nb_back--;
      nb_forward++;
      menu = menu_history[nb_back - 1];
    }
    else
      return;
  }
  else if(flag == 1) {
    if(nb_forward > 0) {
      nb_back++;
      nb_forward--;
      menu = menu_history[nb_back - 1];
    }
    else
      return;
  }
  else {
    menu = menu_asked;
    if(!nb_back || menu_history[nb_back - 1] != menu) {
      menu_history[nb_back++] = menu;
    }
    nb_forward = 0;
  }

  int nb = 0;

  if(menu[0].label[0] == '0')
    m_module_butt->value(0);
  else if(menu[0].label[0] == '1')
    m_module_butt->value(1);
  else if(menu[0].label[0] == '2')
    m_module_butt->value(2);
  else if(menu[0].label[0] == '3')
    m_module_butt->value(3);
  else {
    Msg(WARNING, "Something is wrong in your dynamic context definition");
    return;
  }

  Msg(STATUS2N, menu[0].label + 1);

  switch (m_module_butt->value()) {
  case 3:      // post-processing contexts
    for(i = 0; i < List_Nbr(CTX.post.list); i++) {
      if(i == NB_BUTT_MAX)
        break;
      nb++;
      v = (Post_View *) List_Pointer(CTX.post.list, i);
      m_push_butt[i]->hide();
      m_toggle_butt[i]->show();
      m_toggle_butt[i]->value(v->Visible);
      m_toggle_butt[i]->label(v->Name);
      m_toggle_butt[i]->tooltip(v->FileName);
      m_toggle2_butt[i]->show();
      m_popup_butt[i]->show();
      m_popup2_butt[i]->show();
    }
    for(i = List_Nbr(CTX.post.list); i < NB_BUTT_MAX; i++) {
      m_push_butt[i]->hide();
      m_toggle_butt[i]->hide();
      m_toggle2_butt[i]->hide();
      m_popup_butt[i]->hide();
      m_popup2_butt[i]->hide();
    }
    break;
  default:     // geometry, mesh, solver contexts
    if(m_module_butt->value() == 2) {   //solver
      // should handle MAXSOLVERS
      menu[1].label = opt_solver_name0(0, GMSH_GET, 0);
      menu[2].label = opt_solver_name1(0, GMSH_GET, 0);
      menu[3].label = opt_solver_name2(0, GMSH_GET, 0);
      menu[4].label = opt_solver_name3(0, GMSH_GET, 0);
      menu[5].label = opt_solver_name4(0, GMSH_GET, 0);
      for(i = 0; i < MAXSOLVERS; i++) {
        if(!strlen(menu[i + 1].label))
          menu[i + 1].label = NULL;
      }
    }
    for(i = 0; i < NB_BUTT_MAX; i++) {
      m_toggle_butt[i]->hide();
      m_toggle2_butt[i]->hide();
      m_popup_butt[i]->hide();
      m_popup2_butt[i]->hide();
      if(menu[i + 1].label) {
        m_push_butt[i]->label(menu[i + 1].label);
        m_push_butt[i]->callback(menu[i + 1].callback, menu[i + 1].arg);
        m_push_butt[i]->redraw();
        m_push_butt[i]->show();
        nb++;
      }
      else
        break;
    }
    for(i = nb; i < NB_BUTT_MAX; i++) {
      m_toggle_butt[i]->hide();
      m_toggle2_butt[i]->hide();
      m_popup_butt[i]->hide();
      m_popup2_butt[i]->hide();
      m_push_butt[i]->hide();
    }
    break;
  }

  set_menu_size(nb);

}

int GUI::get_context()
{
  return m_module_butt->value();
}


// Create the graphic window

void GUI::create_graphic_window(int argc, char **argv)
{
  int i, x;

  if(g_window) {
    g_window->show(1, argv);
    return;
  }

  int sh = 2 * fontsize - 4;    // status bar height
  int sw = fontsize + 4;        // status button width
  int width = CTX.viewport[2] - CTX.viewport[0];
  int glheight = CTX.viewport[3] - CTX.viewport[1];
  int height = glheight + sh;

  g_window = new Fl_Window(width, height);
  g_window->callback(file_quit_cb);

  // bottom button bar

  Fl_Box *bottom = new Fl_Box(0, glheight, width, sh);
  bottom->box(FL_THIN_UP_BOX);

  x = 2;

  g_status_butt[0] = new Fl_Button(x, glheight + 2, sw, sh - 4, "X");
  x += sw;
  g_status_butt[0]->callback(status_xyz1p_cb, (void *)0);
  g_status_butt[0]->tooltip("Set X view (Alt+x)");

  g_status_butt[1] = new Fl_Button(x, glheight + 2, sw, sh - 4, "Y");
  x += sw;
  g_status_butt[1]->callback(status_xyz1p_cb, (void *)1);
  g_status_butt[1]->tooltip("Set Y view (Alt+y)");

  g_status_butt[2] = new Fl_Button(x, glheight + 2, sw, sh - 4, "Z");
  x += sw;
  g_status_butt[2]->callback(status_xyz1p_cb, (void *)2);
  g_status_butt[2]->tooltip("Set Z view (Alt+z)");

  g_status_butt[3] = new Fl_Button(x, glheight + 2, 2 * fontsize, sh - 4, "1:1");
  x += 2 * fontsize;
  g_status_butt[3]->callback(status_xyz1p_cb, (void *)3);
  g_status_butt[3]->tooltip("Set unit scale");

  //FIXME: remove this until we do the perspective projection properly
  //g_status_butt[4] = new Fl_Button(x, glheight + 2, sw, sh - 4);
  //x += sw;
  //g_status_butt[4]->callback(status_xyz1p_cb, (void *)4);
  //g_status_butt[4]->tooltip("Set orthographic/perspective projection");
  ortho_bmp = new Fl_Bitmap(ortho_bits, ortho_width, ortho_height);
  persp_bmp = new Fl_Bitmap(persp_bits, persp_width, persp_height);
  //persp_bmp->label(g_status_butt[4]);

  g_status_butt[5] = new Fl_Button(x, glheight + 2, sw, sh - 4, "?");
  x += sw;
  g_status_butt[5]->callback(status_xyz1p_cb, (void *)5);
  g_status_butt[5]->tooltip("Show current options");

  g_status_butt[6] = new Fl_Button(x, glheight + 2, sw, sh - 4);
  x += sw;
  g_status_butt[6]->callback(status_rewind_cb);
  rewind_bmp = new Fl_Bitmap(rewind_bits, rewind_width, rewind_height);
  rewind_bmp->label(g_status_butt[6]);
  g_status_butt[6]->deactivate();
  g_status_butt[6]->tooltip("Rewind animation");

  g_status_butt[7] = new Fl_Button(x, glheight + 2, sw, sh - 4);
  x += sw;
  g_status_butt[7]->callback(status_play_cb);
  g_status_butt[7]->tooltip("Play/pause animation");
  start_bmp = new Fl_Bitmap(start_bits, start_width, start_height);
  start_bmp->label(g_status_butt[7]);
  stop_bmp = new Fl_Bitmap(stop_bits, stop_width, stop_height);
  g_status_butt[7]->deactivate();

  for(i = 0; i < 8; i++) {
    //FIXME:
    if(i != 4){
      g_status_butt[i]->box(FL_FLAT_BOX);
      g_status_butt[i]->selection_color(FL_WHITE);
      g_status_butt[i]->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    }
  }

  g_status_label[0] = new Fl_Box(x, glheight + 2, (width - x) / 3, sh - 4);
  g_status_label[1] = new Fl_Box(x + (width - x) / 3, glheight + 2, (width - x) / 3, sh - 4);
  g_status_label[2] = new Fl_Box(x + 2 * (width - x) / 3, glheight + 2, (width - x) / 3 - 2, sh - 4);
  for(i = 0; i < 3; i++) {
    g_status_label[i]->box(FL_FLAT_BOX);
    g_status_label[i]->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
  }

  // dummy resizable box

  Dummy_Box *resize_box = new Dummy_Box(x, 0, width - x, glheight);
  g_window->resizable(resize_box);

  // opengl window

  g_opengl_window = new Opengl_Window(0, 0, width, glheight);
  if(!opt_general_double_buffer(0, GMSH_GET, 0)) {
    Msg(INFO, "Setting OpenGL visual to single buffered");
    g_opengl_window->mode(FL_RGB | FL_DEPTH | FL_SINGLE);
  }
  g_opengl_window->end();

  g_window->position(CTX.gl_position[0], CTX.gl_position[1]);
  g_window->end();
}

// Set the size of the graphical window

void GUI::set_size(int new_w, int new_h)
{
  g_window->size(new_w, new_h + g_window->h() - g_opengl_window->h());
}

// Set graphic window title

void GUI::set_title(char *str)
{
  g_window->label(str);
}

// Set animation button

void GUI::set_anim_buttons(int mode)
{
  if(mode) {
    g_status_butt[7]->callback(status_play_cb);
    start_bmp->label(g_status_butt[7]);
  }
  else {
    g_status_butt[7]->callback(status_pause_cb);
    stop_bmp->label(g_status_butt[7]);
  }
}

void GUI::check_anim_buttons()
{
  int i, play = 0;
  if(CTX.post.anim_cycle) {
    play = 1;
  }
  else {
    for(i = 0; i < List_Nbr(CTX.post.list); i++) {
      if(((Post_View *) List_Pointer(CTX.post.list, i))->NbTimeStep > 1) {
        play = 1;
        break;
      }
    }
  }
  if(!play) {
    g_status_butt[6]->deactivate();
    g_status_butt[7]->deactivate();
  }
  else {
    g_status_butt[6]->activate();
    g_status_butt[7]->activate();
  }
}

// Set the status messages

void GUI::set_status(char *msg, int num)
{
  if(num >= 0 && num < 3){
    g_status_label[num]->label(msg);
    g_status_label[num]->redraw();
  }
  else if (num == 3){
    int n = strlen(msg);
    int i = 0;
    while(i < n)
      if(msg[i++] == '\n') break;

    strncpy(onscreen_buffer[0], msg, 255);
    if(i < n) 
      strncpy(onscreen_buffer[1], &msg[i], 255);
    else
      onscreen_buffer[1][0] = '\0';
    onscreen_buffer[0][i-1] = '\0';
    redraw_opengl();
  }
}

// set the current drawing context 

void GUI::make_opengl_current()
{
  g_opengl_window->make_current();
}

// Draw the opengl window

void GUI::redraw_opengl()
{
  g_opengl_window->make_current();
  g_opengl_window->redraw();
  check();
}

// Create the option window

void GUI::hide_option_subwindows()
{
  gen_window->hide();
  geo_window->hide();
  mesh_window->hide();
  solver_window->hide();
  post_window->hide();
  view_window->hide();
}

void GUI::create_general_options_window()
{
  create_option_window();
  hide_option_subwindows();
  gen_window->show();
  opt_browser->value(1);
  opt_window->label("Options - General");
}

void GUI::create_geometry_options_window()
{
  create_option_window();
  hide_option_subwindows();
  geo_window->show();
  opt_browser->value(2);
  opt_window->label("Options - Geometry");
}

void GUI::create_mesh_options_window()
{
  create_option_window();
  hide_option_subwindows();
  mesh_window->show();
  opt_browser->value(3);
  opt_window->label("Options - Mesh");
}

void GUI::create_solver_options_window()
{
  create_option_window();
  hide_option_subwindows();
  solver_window->show();
  opt_browser->value(4);
  opt_window->label("Options - Solver");
}

void GUI::create_post_options_window()
{
  create_option_window();
  hide_option_subwindows();
  post_window->show();
  opt_browser->value(5);
  opt_window->label("Options - Post-processing");
}

void GUI::create_view_options_window(int num)
{
  create_option_window();
  hide_option_subwindows();
  update_view_window(num);
  view_window->show();
  opt_browser->value(6 + num);
  static char str[128];
  sprintf(str, "Options - View [%d]", num);
  opt_window->label(str);
}

void GUI::reset_option_browser()
{
  int i, select;
  char str[128];
  select = opt_browser->value();
  opt_browser->clear();
  opt_browser->add("General");
  opt_browser->add("Geometry");
  opt_browser->add("Mesh");
  opt_browser->add("Solver");
  opt_browser->add("Post-processing");
  for(i = 0; i < List_Nbr(CTX.post.list); i++) {
    if(i == NB_BUTT_MAX)
      break;
    sprintf(str, "View [%d]", i);
    opt_browser->add(str);
  }
  if(select <= opt_browser->size())
    opt_browser->value(select);
}

void GUI::check_rotation_center_button()
{
  if(gen_butt[15]->value()) {
    gen_push_butt[0]->deactivate();
    gen_value[8]->deactivate();
    gen_value[9]->deactivate();
    gen_value[10]->deactivate();
  }
  else {
    gen_push_butt[0]->activate();
    gen_value[8]->activate();
    gen_value[9]->activate();
    gen_value[10]->activate();
  }
}

void GUI::create_option_window()
{
  int width = 41 * fontsize;
  int height = 12 * BH + 5 * WB;
  int BROWSERW = 105 + WB;

  if(opt_window) {
    opt_window->show();
    return;
  }

  opt_window = new Fl_Window(width, height);
  opt_window->box(WINDOW_BOX);

  // Buttons

  {
    Fl_Return_Button *o = new Fl_Return_Button(width - 3 * BB - 3 * WB, height - BH - WB, BB, BH, "Apply");
    o->callback(options_ok_cb);
  }
  {
    Fl_Button *o = new Fl_Button(width - 2 * BB - 2 * WB, height - BH - WB, BB, BH, "Save");
    o->callback(options_save_cb);
  }
  {
    Fl_Button *o = new Fl_Button(width - BB - WB, height - BH - WB, BB, BH, "Cancel");
    o->callback(cancel_cb, (void *)opt_window);
  }

  // Selection browser

  opt_browser = new Fl_Hold_Browser(WB, WB, BROWSERW - WB, height - 3 * WB - BH);
  reset_option_browser();
  opt_browser->callback(options_browser_cb);
  opt_browser->value(1);
  opt_window->label("Options - General");

  width -= BROWSERW;
  int BW = width - 4 * WB;
  height -= WB + BH;

  // General options

  gen_window = new Fl_Window(BROWSERW, 0, width, height, "General options");
  {
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width - 2 * WB, height - 2 * WB);
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "General");

      gen_butt[13] = new Fl_Check_Button(2 * WB, 2 * WB + 1 * BH, BW, BH, "Show tooltips");
      gen_butt[13]->type(FL_TOGGLE_BUTTON);
      gen_butt[13]->down_box(TOGGLE_BOX);
      gen_butt[13]->selection_color(TOGGLE_COLOR);

      gen_butt[0] = new Fl_Check_Button(2 * WB, 2 * WB + 2 * BH, BW, BH, "Show moving axes");
      gen_butt[0]->tooltip("(Alt+Shift+a)");
      gen_butt[0]->type(FL_TOGGLE_BUTTON);
      gen_butt[0]->down_box(TOGGLE_BOX);
      gen_butt[0]->selection_color(TOGGLE_COLOR);

      gen_butt[1] = new Fl_Check_Button(2 * WB, 2 * WB + 3 * BH, BW, BH, "Show small axes");
      gen_butt[1]->tooltip("(Alt+a)");
      gen_butt[1]->type(FL_TOGGLE_BUTTON);
      gen_butt[1]->down_box(TOGGLE_BOX);
      gen_butt[1]->selection_color(TOGGLE_COLOR);
	
      gen_butt[6] = new Fl_Check_Button(2 * WB, 2 * WB + 4 * BH, BW, BH, "Show bounding boxes");
      gen_butt[6]->tooltip("(Alt+b)");
      gen_butt[6]->type(FL_TOGGLE_BUTTON);
      gen_butt[6]->down_box(TOGGLE_BOX);
      gen_butt[6]->selection_color(TOGGLE_COLOR);

      gen_butt[2] = new Fl_Check_Button(2 * WB, 2 * WB + 5 * BH, BW, BH, "Draw simplified model while rotating, panning and zooming");
      gen_butt[2]->tooltip("(Alt+f)");
      gen_butt[2]->type(FL_TOGGLE_BUTTON);
      gen_butt[2]->down_box(TOGGLE_BOX);
      gen_butt[2]->selection_color(TOGGLE_COLOR);

      gen_butt[3] = new Fl_Check_Button(2 * WB, 2 * WB + 6 * BH, BW, BH, "Enable double buffering");
      gen_butt[3]->type(FL_TOGGLE_BUTTON);
      gen_butt[3]->down_box(TOGGLE_BOX);
      gen_butt[3]->selection_color(TOGGLE_COLOR);

      gen_butt[4] = new Fl_Check_Button(2 * WB, 2 * WB + 7 * BH, BW, BH, "Use fake transparency mode");
      gen_butt[4]->type(FL_TOGGLE_BUTTON);
      gen_butt[4]->down_box(TOGGLE_BOX);
      gen_butt[4]->selection_color(TOGGLE_COLOR);

      gen_butt[5] = new Fl_Check_Button(2 * WB, 2 * WB + 8 * BH, BW, BH, "Use trackball rotation mode instead of Euler angles");
      gen_butt[5]->type(FL_TOGGLE_BUTTON);
      gen_butt[5]->down_box(TOGGLE_BOX);
      gen_butt[5]->selection_color(TOGGLE_COLOR);

      gen_butt[15] = new Fl_Check_Button(2 * WB, 2 * WB + 9 * BH, BW, BH, "Rotate around pseudo center of mass");
      gen_butt[15]->type(FL_TOGGLE_BUTTON);
      gen_butt[15]->down_box(TOGGLE_BOX);
      gen_butt[15]->selection_color(TOGGLE_COLOR);
      gen_butt[15]->callback(general_options_rotation_center_cb);

      gen_push_butt[0] = new Fl_Button(2 * IW - 2 * WB, 2 * WB + 10 * BH, BB, BH, "Select");
      gen_push_butt[0]->callback(general_options_rotation_center_select_cb);

      gen_value[8] = new Fl_Value_Input(2 * WB, 2 * WB + 10 * BH, IW / 3, BH);
      gen_value[9] = new Fl_Value_Input(2 * WB + IW / 3, 2 * WB + 10 * BH, IW / 3, BH);
      gen_value[10] = new Fl_Value_Input(2 * WB + 2 * IW / 3, 2 * WB + 10 * BH, IW / 3, BH, "Rotation center");
      gen_value[10]->align(FL_ALIGN_RIGHT);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Output");
      gen_butt[7] = new Fl_Check_Button(2 * WB, 2 * WB + 1 * BH, BW, BH, "Print messages on terminal");
      gen_butt[7]->type(FL_TOGGLE_BUTTON);
      gen_butt[7]->down_box(TOGGLE_BOX);
      gen_butt[7]->selection_color(TOGGLE_COLOR);

      gen_butt[8] = new Fl_Check_Button(2 * WB, 2 * WB + 2 * BH, BW, BH, "Save session information on exit");
      gen_butt[8]->type(FL_TOGGLE_BUTTON);
      gen_butt[8]->down_box(TOGGLE_BOX);
      gen_butt[8]->selection_color(TOGGLE_COLOR);

      gen_butt[9] = new Fl_Check_Button(2 * WB, 2 * WB + 3 * BH, BW/2-WB, BH, "Save options on exit");
      gen_butt[9]->type(FL_TOGGLE_BUTTON);
      gen_butt[9]->down_box(TOGGLE_BOX);
      gen_butt[9]->selection_color(TOGGLE_COLOR);

      Fl_Button *b0 = new Fl_Button(width / 2, 2 * WB + 3 * BH, 2 * BB, BH, "Restore default options");
      b0->callback(options_restore_defaults_cb);

      gen_butt[14] = new Fl_Check_Button(2 * WB, 2 * WB + 4 * BH, BW, BH, "Ask confirmation before overwriting files");
      gen_butt[14]->type(FL_TOGGLE_BUTTON);
      gen_butt[14]->down_box(TOGGLE_BOX);
      gen_butt[14]->selection_color(TOGGLE_COLOR);

      gen_value[5] = new Fl_Value_Input(2 * WB, 2 * WB + 5 * BH, IW, BH, "Message verbosity");
      gen_value[5]->minimum(0);
      gen_value[5]->maximum(10);
      gen_value[5]->step(1);
      gen_value[5]->align(FL_ALIGN_RIGHT);

      gen_input[0] = new Fl_Input(2 * WB, 2 * WB + 6 * BH, IW, BH, "Default file name");
      gen_input[0]->align(FL_ALIGN_RIGHT);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Helpers");

      gen_input[1] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "Text editor command");
      gen_input[1]->align(FL_ALIGN_RIGHT);

      gen_input[2] = new Fl_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "Web browser command");
      gen_input[2]->align(FL_ALIGN_RIGHT);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Aspect");
      o->hide();

      static Fl_Menu_Item menu_projection[] = {
	{"Orthographic", 0, 0, 0},
	{"Perspective", 0, 0, 0},
	{0}
      };
      gen_choice[2] = new Fl_Choice(2 * WB, 2 * WB + 1 * BH, IW, BH, "Projection mode");
      gen_choice[2]->menu(menu_projection);
      gen_choice[2]->align(FL_ALIGN_RIGHT);
      gen_choice[2]->tooltip("(Alt+o)");

      gen_value[11] = new Fl_Value_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "Number of quadric subdivisions");
      gen_value[11]->minimum(3);
      gen_value[11]->maximum(30);
      gen_value[11]->step(1);
      gen_value[11]->align(FL_ALIGN_RIGHT);

      gen_value[6] = new Fl_Value_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "Point size");
      gen_value[6]->minimum(0.1);
      gen_value[6]->maximum(50);
      gen_value[6]->step(0.1);
      gen_value[6]->align(FL_ALIGN_RIGHT);

      gen_value[7] = new Fl_Value_Input(2 * WB, 2 * WB + 4 * BH, IW, BH, "Line width");
      gen_value[7]->minimum(0.1);
      gen_value[7]->maximum(50);
      gen_value[7]->step(0.1);
      gen_value[7]->align(FL_ALIGN_RIGHT);

      static Fl_Menu_Item menu_genvectype[] = {
	{"Line", 0, 0, 0},
	{"Arrow", 0, 0, 0},
	{"Pyramid", 0, 0, 0},
	{"3D arrow", 0, 0, 0},
	{0}
      };
      gen_choice[0] = new Fl_Choice(2 * WB, 2 * WB + 5 * BH, IW, BH, "Vector display");
      gen_choice[0]->menu(menu_genvectype);
      gen_choice[0]->align(FL_ALIGN_RIGHT);

      Fl_Button *b = new Fl_Button(2 * IW - 2 * WB, 2 * WB + 5 * BH, (int)(1.5*BB), BH, "Edit arrow shape");
      b->callback(general_arrow_param_cb);

      gen_choice[1] = new Fl_Choice(2 * WB, 2 * WB + 6 * BH, IW, BH, "Font");
      gen_choice[1]->menu(menu_font_names);
      gen_choice[1]->align(FL_ALIGN_RIGHT);

      gen_value[12] = new Fl_Value_Input(2 * WB, 2 * WB + 7 * BH, IW, BH, "Font size");
      gen_value[12]->minimum(5);
      gen_value[12]->maximum(40);
      gen_value[12]->step(1);
      gen_value[12]->align(FL_ALIGN_RIGHT);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Light");
      o->hide();

      gen_value[2] = new Fl_Value_Input(2 * WB, 2 * WB + 1 * BH, IW/3, BH);
      gen_value[2]->minimum(-1);
      gen_value[2]->maximum(1);
      gen_value[2]->step(0.01);

      gen_value[3] = new Fl_Value_Input(2 * WB + IW / 3, 2 * WB + 1 * BH, IW/3, BH);
      gen_value[3]->minimum(-1);
      gen_value[3]->maximum(1);
      gen_value[3]->step(0.01);

      gen_value[4] = new Fl_Value_Input(2 * WB + 2 * IW / 3, 2 * WB + 1 * BH, IW/3, BH, "Light position");
      gen_value[4]->minimum(-1);
      gen_value[4]->maximum(1);
      gen_value[4]->step(0.01);
      gen_value[4]->align(FL_ALIGN_RIGHT);

      gen_value[1] = new Fl_Value_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "Material shininess");
      gen_value[1]->minimum(0);
      gen_value[1]->maximum(10);
      gen_value[1]->step(0.1);
      gen_value[1]->align(FL_ALIGN_RIGHT);

      gen_value[0] = new Fl_Value_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "Material shininess exponent");
      gen_value[0]->minimum(0);
      gen_value[0]->maximum(128);
      gen_value[0]->step(1);
      gen_value[0]->align(FL_ALIGN_RIGHT);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Color");
      o->hide();

      static Fl_Menu_Item menu_color_scheme[] = {
	{"Dark", 0, 0, 0},
	{"Light", 0, 0, 0},
	{"Grayscale", 0, 0, 0},
	{0}
      };

      gen_choice[3] = new Fl_Choice(2 * WB, 2 * WB + 1 * BH, IW, BH, "Predefined color scheme");
      gen_choice[3]->menu(menu_color_scheme);
      gen_choice[3]->callback(general_options_color_scheme_cb);
      gen_choice[3]->align(FL_ALIGN_RIGHT);
      gen_choice[3]->tooltip("(Alt+c)");

      Fl_Scroll *s = new Fl_Scroll(2 * WB, 3 * WB + 2 * BH, IW + 20, height - 5 * WB - 2 * BH);
      int i = 0;
      while(GeneralOptions_Color[i].str) {
        gen_col[i] = new Fl_Button(2 * WB, 3 * WB + (2 + i) * BH, IW, BH, GeneralOptions_Color[i].str);
        gen_col[i]->callback(color_cb, (void *)GeneralOptions_Color[i].function);
        i++;
      }
      s->end();

      o->end();
    }
    o->end();
  }
  gen_window->end();

  // Geometry options

  geo_window = new Fl_Window(BROWSERW, 0, width, height, "Geometry options");
  geo_window->hide();
  {
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width - 2 * WB, height - 2 * WB);
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "General");
      o->hide();
      geo_butt[8] = new Fl_Check_Button(2 * WB, 2 * WB + 1 * BH, BW, BH, "Remove duplicate entities automatically");
      geo_butt[8]->type(FL_TOGGLE_BUTTON);
      geo_butt[8]->down_box(TOGGLE_BOX);
      geo_butt[8]->selection_color(TOGGLE_COLOR);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Visibility");

      geo_butt[0] = new Fl_Check_Button(2 * WB, 2 * WB + 1 * BH, BW / 2 - WB, BH, "Points");
      geo_butt[0]->tooltip("(Alt+p)");
      geo_butt[0]->type(FL_TOGGLE_BUTTON);
      geo_butt[0]->down_box(TOGGLE_BOX);
      geo_butt[0]->selection_color(TOGGLE_COLOR);

      geo_butt[1] = new Fl_Check_Button(2 * WB, 2 * WB + 2 * BH, BW / 2 - WB, BH, "Lines");
      geo_butt[1]->tooltip("(Alt+l)");
      geo_butt[1]->type(FL_TOGGLE_BUTTON);
      geo_butt[1]->down_box(TOGGLE_BOX);
      geo_butt[1]->selection_color(TOGGLE_COLOR);

      geo_butt[2] = new Fl_Check_Button(2 * WB, 2 * WB + 3 * BH, BW / 2 - WB, BH, "Surfaces");
      geo_butt[2]->tooltip("(Alt+s)");
      geo_butt[2]->type(FL_TOGGLE_BUTTON);
      geo_butt[2]->down_box(TOGGLE_BOX);
      geo_butt[2]->selection_color(TOGGLE_COLOR);

      geo_butt[3] = new Fl_Check_Button(2 * WB, 2 * WB + 4 * BH, BW / 2 - WB, BH, "Volumes");
      geo_butt[3]->tooltip("(Alt+v)");
      geo_butt[3]->type(FL_TOGGLE_BUTTON);
      geo_butt[3]->down_box(TOGGLE_BOX);
      geo_butt[3]->selection_color(TOGGLE_COLOR);

      geo_butt[4] = new Fl_Check_Button(width / 2, 2 * WB + 1 * BH, BW / 2 - WB, BH, "Point numbers");
      geo_butt[4]->type(FL_TOGGLE_BUTTON);
      geo_butt[4]->down_box(TOGGLE_BOX);
      geo_butt[4]->selection_color(TOGGLE_COLOR);

      geo_butt[5] = new Fl_Check_Button(width / 2, 2 * WB + 2 * BH, BW / 2 - WB, BH, "Line numbers");
      geo_butt[5]->type(FL_TOGGLE_BUTTON);
      geo_butt[5]->down_box(TOGGLE_BOX);
      geo_butt[5]->selection_color(TOGGLE_COLOR);

      geo_butt[6] = new Fl_Check_Button(width / 2, 2 * WB + 3 * BH, BW / 2 - WB, BH, "Surface numbers");
      geo_butt[6]->type(FL_TOGGLE_BUTTON);
      geo_butt[6]->down_box(TOGGLE_BOX);
      geo_butt[6]->selection_color(TOGGLE_COLOR);

      geo_butt[7] = new Fl_Check_Button(width / 2, 2 * WB + 4 * BH, BW / 2 - WB, BH, "Volume numbers");
      geo_butt[7]->type(FL_TOGGLE_BUTTON);
      geo_butt[7]->down_box(TOGGLE_BOX);
      geo_butt[7]->selection_color(TOGGLE_COLOR);

      geo_value[0] = new Fl_Value_Input(2 * WB, 2 * WB + 5 * BH, IW, BH, "Normals");
      geo_value[0]->minimum(0);
      geo_value[0]->maximum(500);
      geo_value[0]->step(1);
      geo_value[0]->align(FL_ALIGN_RIGHT);

      geo_value[1] = new Fl_Value_Input(2 * WB, 2 * WB + 6 * BH, IW, BH, "Tangents");
      geo_value[1]->minimum(0);
      geo_value[1]->maximum(500);
      geo_value[1]->step(1);
      geo_value[1]->align(FL_ALIGN_RIGHT);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Aspect");
      o->hide();

      geo_choice[0] = new Fl_Choice(2 * WB, 2 * WB + 1 * BH, IW, BH, "Point display");
      GMSH_Solve_Plugin *sp = GMSH_PluginManager::instance()->findSolverPlugin();   
      if (sp)
	{
	  geo_choice[0]->menu(menu_point_display_with_plugin);
	}
      else
	{
	  geo_choice[0]->menu(menu_point_display);
	}
      geo_choice[0]->align(FL_ALIGN_RIGHT);

      geo_value[3] = new Fl_Value_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "Point size");
      geo_value[3]->minimum(0.1);
      geo_value[3]->maximum(50);
      geo_value[3]->step(0.1);
      geo_value[3]->align(FL_ALIGN_RIGHT);

      geo_value[5] = new Fl_Value_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "Selected point size");
      geo_value[5]->minimum(0.1);
      geo_value[5]->maximum(50);
      geo_value[5]->step(0.1);
      geo_value[5]->align(FL_ALIGN_RIGHT);


      {
	geo_choice[1] = new Fl_Choice(2 * WB, 2 * WB + 4 * BH, IW, BH, "Line display");
	GMSH_Solve_Plugin *sp = GMSH_PluginManager::instance()->findSolverPlugin();   
	if (sp)
	  {
	    geo_choice[1]->menu(menu_line_display_with_plugin);
	  }
	else
	  {
	    geo_choice[1]->menu(menu_line_display);
	  }
	geo_choice[1]->align(FL_ALIGN_RIGHT);	
      }

      geo_value[4] = new Fl_Value_Input(2 * WB, 2 * WB + 5 * BH, IW, BH, "Line width");
      geo_value[4]->minimum(0.1);
      geo_value[4]->maximum(50);
      geo_value[4]->step(0.1);
      geo_value[4]->align(FL_ALIGN_RIGHT);

      geo_value[6] = new Fl_Value_Input(2 * WB, 2 * WB + 6 * BH, IW, BH, "Selected line width");
      geo_value[6]->minimum(0.1);
      geo_value[6]->maximum(50);
      geo_value[6]->step(0.1);
      geo_value[6]->align(FL_ALIGN_RIGHT);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Light");
      o->hide();

      geo_butt[9] = new Fl_Check_Button(2 * WB, 2 * WB + 1 * BH, BW, BH, "Enable lighting");
      geo_butt[9]->type(FL_TOGGLE_BUTTON);
      geo_butt[9]->down_box(TOGGLE_BOX);
      geo_butt[9]->selection_color(TOGGLE_COLOR);
      geo_butt[9]->tooltip("(Alt+w)");

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Color");
      o->hide();

      Fl_Scroll *s = new Fl_Scroll(2 * WB, 2 * WB + 1 * BH, IW + 20, height - 4 * WB - 1 * BH);
      int i = 0;
      while(GeometryOptions_Color[i].str) {
        geo_col[i] = new Fl_Button(2 * WB, 2 * WB + (1 + i) * BH, IW, BH, GeometryOptions_Color[i].str);
        geo_col[i]->callback(color_cb, (void *)GeometryOptions_Color[i].function);
        i++;
      }
      s->end();

      o->end();
    }
    o->end();
  }
  geo_window->end();

  // Mesh options

  mesh_window = new Fl_Window(BROWSERW, 0, width, height, "Mesh options");
  mesh_window->hide();
  {
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width - 2 * WB, height - 2 * WB);
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "General");
      o->hide();

      static Fl_Menu_Item menu_2d_algo[] = {
        {"Isotropic", 0, 0, 0},
        {"Triangle", 0, 0, 0},
        {"Anisotropic", 0, 0, 0},
        {0}
      };
      static Fl_Menu_Item menu_3d_algo[] = {
        {"Isotropic", 0, 0, 0},
        {"Netgen", 0, 0, 0},
        {0}
      };

      mesh_choice[2] = new Fl_Choice(2 * WB, 2 * WB + 1 * BH, IW, BH, "2D algorithm");
      mesh_choice[2]->menu(menu_2d_algo);
      mesh_choice[2]->align(FL_ALIGN_RIGHT);

      mesh_choice[3] = new Fl_Choice(2 * WB, 2 * WB + 2 * BH, IW, BH, "3D algorithm");
      mesh_choice[3]->menu(menu_3d_algo);
      mesh_choice[3]->align(FL_ALIGN_RIGHT);

      mesh_butt[4] = new Fl_Check_Button(2 * WB, 2 * WB + 3 * BH, BW, BH, "Show interactive anisotropic mesh construction");
      mesh_butt[4]->type(FL_TOGGLE_BUTTON);
      mesh_butt[4]->down_box(TOGGLE_BOX);
      mesh_butt[4]->selection_color(TOGGLE_COLOR);

      mesh_value[0] = new Fl_Value_Input(2 * WB, 2 * WB + 4 * BH, IW, BH, "Number of smoothing steps");
      mesh_value[0]->minimum(0);
      mesh_value[0]->maximum(100);
      mesh_value[0]->step(1);
      mesh_value[0]->align(FL_ALIGN_RIGHT);

      mesh_value[1] = new Fl_Value_Input(2 * WB, 2 * WB + 5 * BH, IW, BH, "Mesh scaling factor");
      mesh_value[1]->minimum(0.001);
      mesh_value[1]->maximum(1000);
      mesh_value[1]->step(0.001);
      mesh_value[1]->align(FL_ALIGN_RIGHT);

      mesh_value[2] = new Fl_Value_Input(2 * WB, 2 * WB + 6 * BH, IW, BH, "Characteristic length factor");
      mesh_value[2]->minimum(0.001);
      mesh_value[2]->maximum(1000);
      mesh_value[2]->step(0.001);
      mesh_value[2]->align(FL_ALIGN_RIGHT);

      mesh_value[3] = new Fl_Value_Input(2 * WB, 2 * WB + 7 * BH, IW, BH, "Random perturbation factor");
      mesh_value[3]->minimum(1.e-6);
      mesh_value[3]->maximum(1.e-1);
      mesh_value[3]->step(1.e-6);
      mesh_value[3]->align(FL_ALIGN_RIGHT);

      mesh_butt[2] = new Fl_Check_Button(2 * WB, 2 * WB + 8 * BH, BW, BH, "Optimize quality of tetrahedral elements");
      mesh_butt[2]->type(FL_TOGGLE_BUTTON);
      mesh_butt[2]->down_box(TOGGLE_BOX);
      mesh_butt[2]->selection_color(TOGGLE_COLOR);
#if !defined(HAVE_NETGEN)
      mesh_butt[2]->deactivate();
#endif

      mesh_butt[3] = new Fl_Check_Button(2 * WB, 2 * WB + 9 * BH, BW, BH, "Generate second order elements");
      mesh_butt[3]->type(FL_TOGGLE_BUTTON);
      mesh_butt[3]->down_box(TOGGLE_BOX);
      mesh_butt[3]->selection_color(TOGGLE_COLOR);

      mesh_butt[5] = new Fl_Check_Button(2 * WB, 2 * WB + 10 * BH, BW, BH, "Constrain background mesh with characteristic length field");
      mesh_butt[5]->type(FL_TOGGLE_BUTTON);
      mesh_butt[5]->down_box(TOGGLE_BOX);
      mesh_butt[5]->selection_color(TOGGLE_COLOR);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Visibility");

      mesh_butt[6] = new Fl_Check_Button(2 * WB, 2 * WB + 1 * BH, BW / 2 - WB, BH, "Nodes");
      mesh_butt[6]->tooltip("(Alt+Shift+p)");
      mesh_butt[6]->type(FL_TOGGLE_BUTTON);
      mesh_butt[6]->down_box(TOGGLE_BOX);
      mesh_butt[6]->selection_color(TOGGLE_COLOR);

      mesh_butt[7] = new Fl_Check_Button(2 * WB, 2 * WB + 2 * BH, BW / 2 - WB, BH, "Line elements");
      mesh_butt[7]->tooltip("(Alt+Shift+l)");
      mesh_butt[7]->type(FL_TOGGLE_BUTTON);
      mesh_butt[7]->down_box(TOGGLE_BOX);
      mesh_butt[7]->selection_color(TOGGLE_COLOR);

      mesh_butt[8] = new Fl_Check_Button(2 * WB, 2 * WB + 3 * BH, BW / 2 - WB, BH, "Surface element edges");
      mesh_butt[8]->tooltip("(Alt+Shift+s)");
      mesh_butt[8]->type(FL_TOGGLE_BUTTON);
      mesh_butt[8]->down_box(TOGGLE_BOX);
      mesh_butt[8]->selection_color(TOGGLE_COLOR);

      mesh_butt[9] = new Fl_Check_Button(2 * WB, 2 * WB + 4 * BH, BW / 2 - WB, BH, "Surface element faces");
      mesh_butt[9]->tooltip("(Alt+Shift+s, Alt+Shift+d)");
      mesh_butt[9]->type(FL_TOGGLE_BUTTON);
      mesh_butt[9]->down_box(TOGGLE_BOX);
      mesh_butt[9]->selection_color(TOGGLE_COLOR);

      mesh_butt[10] = new Fl_Check_Button(2 * WB, 2 * WB + 5 * BH, BW / 2 - WB, BH, "Volume element edges");
      mesh_butt[10]->tooltip("(Alt+Shift+v)");
      mesh_butt[10]->type(FL_TOGGLE_BUTTON);
      mesh_butt[10]->down_box(TOGGLE_BOX);
      mesh_butt[10]->selection_color(TOGGLE_COLOR);

      mesh_butt[11] = new Fl_Check_Button(2 * WB, 2 * WB + 6 * BH, BW / 2 - WB, BH, "Volume element faces");
      mesh_butt[11]->tooltip("(Alt+Shift+v)");
      mesh_butt[11]->type(FL_TOGGLE_BUTTON);
      mesh_butt[11]->down_box(TOGGLE_BOX);
      mesh_butt[11]->selection_color(TOGGLE_COLOR);

      mesh_butt[12] = new Fl_Check_Button(width / 2, 2 * WB + 1 * BH, BW / 2 - WB, BH, "Node numbers");
      mesh_butt[12]->type(FL_TOGGLE_BUTTON);
      mesh_butt[12]->down_box(TOGGLE_BOX);
      mesh_butt[12]->selection_color(TOGGLE_COLOR);

      mesh_butt[13] = new Fl_Check_Button(width / 2, 2 * WB + 2 * BH, BW / 2 - WB, BH, "Line element numbers");
      mesh_butt[13]->type(FL_TOGGLE_BUTTON);
      mesh_butt[13]->down_box(TOGGLE_BOX);
      mesh_butt[13]->selection_color(TOGGLE_COLOR);

      mesh_butt[14] = new Fl_Check_Button(width / 2, 2 * WB + 3 * BH, BW / 2 - WB, BH, "Surface element numbers");
      mesh_butt[14]->type(FL_TOGGLE_BUTTON);
      mesh_butt[14]->down_box(TOGGLE_BOX);
      mesh_butt[14]->selection_color(TOGGLE_COLOR);

      mesh_butt[15] = new Fl_Check_Button(width / 2, 2 * WB + 4 * BH, BW / 2 - WB, BH, "Volume element numbers");
      mesh_butt[15]->type(FL_TOGGLE_BUTTON);
      mesh_butt[15]->down_box(TOGGLE_BOX);
      mesh_butt[15]->selection_color(TOGGLE_COLOR);

      mesh_value[4] = new Fl_Value_Input(2 * WB, 2 * WB + 7 * BH, IW / 2, BH);
      mesh_value[4]->minimum(0);
      mesh_value[4]->maximum(1);
      mesh_value[4]->step(0.001);
      mesh_value[4]->align(FL_ALIGN_RIGHT);

      mesh_value[5] = new Fl_Value_Input(2 * WB + IW / 2, 2 * WB + 7 * BH, IW / 2, BH, "Tetrahedra quality range");
      mesh_value[5]->minimum(0);
      mesh_value[5]->maximum(1);
      mesh_value[5]->step(0.001);
      mesh_value[5]->align(FL_ALIGN_RIGHT);

      mesh_value[6] = new Fl_Value_Input(2 * WB, 2 * WB + 8 * BH, IW / 2, BH);
      mesh_value[6]->align(FL_ALIGN_RIGHT);

      mesh_value[7] = new Fl_Value_Input(2 * WB + IW / 2, 2 * WB + 8 * BH, IW / 2, BH, "Tetrahedra size range");
      mesh_value[7]->align(FL_ALIGN_RIGHT);

      mesh_value[8] = new Fl_Value_Input(2 * WB, 2 * WB + 9 * BH, IW, BH, "Normals");
      mesh_value[8]->minimum(0);
      mesh_value[8]->maximum(500);
      mesh_value[8]->step(1);
      mesh_value[8]->align(FL_ALIGN_RIGHT);

      mesh_value[13] = new Fl_Value_Input(2 * WB, 2 * WB + 10 * BH, IW, BH, "Tangents");
      mesh_value[13]->minimum(0);
      mesh_value[13]->maximum(200);
      mesh_value[13]->step(1.0);
      mesh_value[13]->align(FL_ALIGN_RIGHT);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Cut plane");
      o->hide();

      mesh_butt[16] = new Fl_Check_Button(2 * WB, 2 * WB + 1 * BH, BW, BH, "Enable");
      mesh_butt[16]->type(FL_TOGGLE_BUTTON);
      mesh_butt[16]->down_box(TOGGLE_BOX);
      mesh_butt[16]->selection_color(TOGGLE_COLOR);

      mesh_value[14] = new Fl_Value_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "A");
      mesh_value[14]->align(FL_ALIGN_RIGHT);
      mesh_value[14]->callback(mesh_cut_plane_cb);
      mesh_value[14]->step(0.01);
      mesh_value[14]->minimum(-1.0);
      mesh_value[14]->maximum(1.0);

      mesh_value[15] = new Fl_Value_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "B");
      mesh_value[15]->align(FL_ALIGN_RIGHT);
      mesh_value[15]->callback(mesh_cut_plane_cb);
      mesh_value[15]->step(0.01);
      mesh_value[15]->minimum(-1.0);
      mesh_value[15]->maximum(1.0);

      mesh_value[16] = new Fl_Value_Input(2 * WB, 2 * WB + 4 * BH, IW, BH, "C");
      mesh_value[16]->align(FL_ALIGN_RIGHT);
      mesh_value[16]->callback(mesh_cut_plane_cb);
      mesh_value[16]->step(0.01);
      mesh_value[16]->minimum(-1.0);
      mesh_value[16]->maximum(1.0);

      mesh_value[17] = new Fl_Value_Input(2 * WB, 2 * WB + 5 * BH, IW, BH, "D");
      mesh_value[17]->align(FL_ALIGN_RIGHT);
      mesh_value[17]->callback(mesh_cut_plane_cb);
      mesh_value[17]->step(0.01);
      mesh_value[17]->minimum(-1.0);
      mesh_value[17]->maximum(1.0);

      mesh_butt[22] = new Fl_Check_Button(2 * WB, 2 * WB + 6 * BH, BW, BH, "Draw intersecting volume layer as surface");
      mesh_butt[22]->type(FL_TOGGLE_BUTTON);
      mesh_butt[22]->down_box(TOGGLE_BOX);
      mesh_butt[22]->selection_color(TOGGLE_COLOR);

      mesh_butt[23] = new Fl_Check_Button(2 * WB, 2 * WB + 7 * BH, BW, BH, "Cut only volume elements");
      mesh_butt[23]->type(FL_TOGGLE_BUTTON);
      mesh_butt[23]->down_box(TOGGLE_BOX);
      mesh_butt[23]->selection_color(TOGGLE_COLOR);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Aspect");
      o->hide();

      mesh_value[9] = new Fl_Value_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "Explosion factor");
      mesh_value[9]->minimum(0);
      mesh_value[9]->maximum(1);
      mesh_value[9]->step(0.01);
      mesh_value[9]->align(FL_ALIGN_RIGHT);

      mesh_choice[0] = new Fl_Choice(2 * WB, 2 * WB + 2 * BH, IW, BH, "Point display");
      mesh_choice[0]->menu(menu_point_display);
      mesh_choice[0]->align(FL_ALIGN_RIGHT);

      mesh_value[10] = new Fl_Value_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "Point size");
      mesh_value[10]->minimum(0.1);
      mesh_value[10]->maximum(50);
      mesh_value[10]->step(0.1);
      mesh_value[10]->align(FL_ALIGN_RIGHT);

      mesh_choice[1] = new Fl_Choice(2 * WB, 2 * WB + 4 * BH, IW, BH, "Line display");
      mesh_choice[1]->menu(menu_line_display);
      mesh_choice[1]->align(FL_ALIGN_RIGHT);
      mesh_choice[1]->deactivate(); // don't give false hopes, as it's not used anywhere right now

      mesh_value[11] = new Fl_Value_Input(2 * WB, 2 * WB + 5 * BH, IW, BH, "Line width");
      mesh_value[11]->minimum(0.1);
      mesh_value[11]->maximum(50);
      mesh_value[11]->step(0.1);
      mesh_value[11]->align(FL_ALIGN_RIGHT);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Light");
      o->hide();

      mesh_butt[17] = new Fl_Check_Button(2 * WB, 2 * WB + 1 * BH, BW, BH, "Enable lighting");
      mesh_butt[17]->tooltip("(Alt+w)");
      mesh_butt[17]->type(FL_TOGGLE_BUTTON);
      mesh_butt[17]->down_box(TOGGLE_BOX);
      mesh_butt[17]->selection_color(TOGGLE_COLOR);

      mesh_butt[18] = new Fl_Check_Button(2 * WB, 2 * WB + 2 * BH, BW, BH, "Use two-side lighting");
      mesh_butt[18]->type(FL_TOGGLE_BUTTON);
      mesh_butt[18]->down_box(TOGGLE_BOX);
      mesh_butt[18]->selection_color(TOGGLE_COLOR);

      mesh_butt[19] = new Fl_Check_Button(2 * WB, 2 * WB + 3 * BH, BW, BH, "Smooth normals");
      mesh_butt[19]->type(FL_TOGGLE_BUTTON);
      mesh_butt[19]->down_box(TOGGLE_BOX);
      mesh_butt[19]->selection_color(TOGGLE_COLOR);

      mesh_value[18] = new Fl_Value_Input(2 * WB, 2 * WB + 4 * BH, IW, BH, "Smoothing threshold angle");
      mesh_value[18]->minimum(0.);
      mesh_value[18]->maximum(180.);
      mesh_value[18]->step(1.);
      mesh_value[18]->align(FL_ALIGN_RIGHT);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Color");
      o->hide();

      static Fl_Menu_Item menu_mesh_color[] = {
        {"By element type", 0, 0, 0},
        {"By elementary entity", 0, 0, 0},
        {"By physical entity", 0, 0, 0},
        {"By partition", 0, 0, 0},
        {0}
      };
      mesh_choice[4] = new Fl_Choice(2 * WB, 2 * WB + 1 * BH, IW, BH, "Coloring mode");
      mesh_choice[4]->menu(menu_mesh_color);
      mesh_choice[4]->align(FL_ALIGN_RIGHT);

      Fl_Scroll *s = new Fl_Scroll(2 * WB, 3 * WB + 2 * BH, IW + 20, height - 5 * WB - 2 * BH);
      int i = 0;
      while(MeshOptions_Color[i].str) {
        mesh_col[i] = new Fl_Button(2 * WB, 3 * WB + (2 + i) * BH, IW, BH, MeshOptions_Color[i].str);
        mesh_col[i]->callback(color_cb, (void *)MeshOptions_Color[i].function);
        i++;
      }
      s->end();

      o->end();
    }
    o->end();
  }
  mesh_window->end();

  // Solver options

  solver_window = new Fl_Window(BROWSERW, 0, width, height, "Solver options");
  solver_window->hide();
  {
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width - 2 * WB, height - 2 * WB);
    {
      {
	Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "General");
	
	Fl_Box *text = new Fl_Box(FL_NO_BOX, 2 * WB, 3 * WB + 1 * BH, width - 4 * WB, 2 * BH,
				  "There are no global solver options available yet.\n\n"
				  "To define your own solver interface, you have to edit the option file.");
	text->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
	
	o->end();
      }
    }
    o->end();
  }

  solver_window->end();

  // Post-processing options

  post_window = new Fl_Window(BROWSERW, 0, width, height, "Post-processing options");
  post_window->hide();
  {
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width - 2 * WB, height - 2 * WB);
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "General");

      static Fl_Menu_Item menu_links[] = {
        {"None", 0, 0, 0},
        {"Apply next changes to all visible views", 0, 0, 0},
        {"Apply next changes to all views", 0, 0, 0},
        {"Force same options for all visible views", 0, 0, 0},
        {"Force same options for all views", 0, 0, 0},
        {0}
      };

      post_choice[0] = new Fl_Choice(2 * WB, 2 * WB + 1 * BH, IW, BH, "View links");
      post_choice[0]->menu(menu_links);
      post_choice[0]->align(FL_ALIGN_RIGHT);

      post_value[0] = new Fl_Value_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "Animation delay");
      post_value[0]->minimum(0);
      post_value[0]->maximum(10);
      post_value[0]->step(0.01);
      post_value[0]->align(FL_ALIGN_RIGHT);

      post_butt[0] = new Fl_Check_Button(2 * WB, 2 * WB + 3 * BH, BW, BH, "Cycle through views instead of time steps");
      post_butt[0]->type(FL_TOGGLE_BUTTON);
      post_butt[0]->down_box(TOGGLE_BOX);
      post_butt[0]->selection_color(TOGGLE_COLOR);

      o->end();
    }
    o->end();
  }

  post_window->end();

  // View options

  view_number = -1;

  view_window = new Fl_Window(BROWSERW, 0, width, height, "View options");
  view_window->hide();
  {
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width - 2 * WB, height - 2 * WB);
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "General");

      view_butt[1] = new Fl_Check_Button(2 * WB, 2 * WB + 1 * BH, BW / 2 - WB, BH, "3D view");
      view_butt[1]->type(FL_RADIO_BUTTON);
      view_butt[1]->down_box(RADIO_BOX);
      view_butt[1]->selection_color(RADIO_COLOR);

      view_butt[2] = new Fl_Check_Button(2 * WB, 2 * WB + 2 * BH, BW / 2 - WB, BH, "2D space table");
      view_butt[2]->type(FL_RADIO_BUTTON);
      view_butt[2]->down_box(RADIO_BOX);
      view_butt[2]->selection_color(RADIO_COLOR);

      view_butt[3] = new Fl_Check_Button(2 * WB, 2 * WB + 3 * BH, BW / 2 - WB, BH, "2D time table");
      view_butt[3]->type(FL_RADIO_BUTTON);
      view_butt[3]->down_box(RADIO_BOX);
      view_butt[3]->selection_color(RADIO_COLOR);

      int sw = (int)(1.5 * fontsize);
      view_butt_rep[0] = new Fl_Repeat_Button(2 * WB, 2 * WB + 4 * BH, sw, BH, "-");
      view_butt_rep[1] = new Fl_Repeat_Button(2 * WB + IW - sw, 2 * WB + 4 * BH, sw, BH, "+");
      view_value[50] = new Fl_Value_Input(2 * WB + sw, 2 * WB + 4 * BH, IW - 2 * sw, BH);
      view_value[50]->align(FL_ALIGN_RIGHT);
      view_value[50]->minimum(0);
      view_value[50]->maximum(0);
      view_value[50]->step(1);
      Fl_Box *a = new Fl_Box(2 * WB + IW, 2 * WB + 4 * BH, IW / 2, BH, "Step");
      a->box(FL_NO_BOX);
      a->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

      view_input[0] = new Fl_Input(2 * WB, 2 * WB + 5 * BH, IW, BH, "Name");
      view_input[0]->align(FL_ALIGN_RIGHT);

      view_input[1] = new Fl_Input(2 * WB, 2 * WB + 6 * BH, IW, BH, "Format");
      view_input[1]->align(FL_ALIGN_RIGHT);

      {
	view_2d = new Fl_Group(2 * WB, 2 * WB + 7 * BH, width - 2 * WB, 4 * BH, 0);
	
	view_input[2] = new Fl_Input(2 * WB, 2 * WB + 7 * BH, IW, BH, "Abscissa name");
	view_input[2]->align(FL_ALIGN_RIGHT);
	
	view_input[3] = new Fl_Input(2 * WB, 2 * WB + 8 * BH, IW, BH, "Abscissa format");
	view_input[3]->align(FL_ALIGN_RIGHT);
	
	view_value[25] = new Fl_Value_Input(2 * WB, 2 * WB + 9 * BH, IW, BH, "Number of abscissa points");
	view_value[25]->minimum(0.);
	view_value[25]->step(1);
	view_value[25]->maximum(256);
	view_value[25]->align(FL_ALIGN_RIGHT);
	
	static Fl_Menu_Item menu_grid_mode[] = {
	  {"None", 0, 0, 0},
	  {"Axes", 0, 0, 0},
	  {"Box", 0, 0, 0},
	  {"Full", 0, 0, 0},
	  {0}
	};
	view_choice[8] = new Fl_Choice(2 * WB, 2 * WB + 10 * BH, IW, BH, "Grid mode");
	view_choice[8]->menu(menu_grid_mode);
	view_choice[8]->align(FL_ALIGN_RIGHT);
	
	view_2d->end();
      }

      view_butt[7] = new Fl_Check_Button(width / 2, 2 * WB + 1 * BH, BW / 2 - WB, BH, "Set position automatically");
      view_butt[7]->type(FL_TOGGLE_BUTTON);
      view_butt[7]->down_box(TOGGLE_BOX);
      view_butt[7]->selection_color(TOGGLE_COLOR);
      
      view_value[20] = new Fl_Value_Input(width /2, 2 * WB + 2 * BH, IW / 2, BH);
      view_value[20]->align(FL_ALIGN_RIGHT);
      view_value[20]->minimum(0);
      view_value[20]->maximum(1024);
      view_value[20]->step(1);
      view_value[21] = new Fl_Value_Input(width / 2 + IW / 2, 2 * WB + 2 * BH, IW / 2, BH, "Position");
      view_value[21]->align(FL_ALIGN_RIGHT);
      view_value[21]->minimum(0);
      view_value[21]->maximum(1024);
      view_value[21]->step(1);

      view_value[22] = new Fl_Value_Input(width /2, 2 * WB + 3 * BH, IW / 2, BH);
      view_value[22]->align(FL_ALIGN_RIGHT);
      view_value[22]->minimum(0);
      view_value[22]->maximum(1024);
      view_value[22]->step(1);
      view_value[23] = new Fl_Value_Input(width / 2 + IW / 2, 2 * WB + 3 * BH, IW / 2, BH, "Size");
      view_value[23]->align(FL_ALIGN_RIGHT);
      view_value[23]->minimum(0);
      view_value[23]->maximum(1024);
      view_value[23]->step(1);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Visibility");
      o->hide();

      view_butt[13] = new Fl_Check_Button(2 * WB, 2 * WB + 1 * BH, BW / 2 - WB, BH, "Points");
      view_butt[13]->type(FL_TOGGLE_BUTTON);
      view_butt[13]->down_box(TOGGLE_BOX);
      view_butt[13]->selection_color(TOGGLE_COLOR);

      view_butt[14] = new Fl_Check_Button(2 * WB, 2 * WB + 2 * BH, BW / 2 - WB, BH, "Lines");
      view_butt[14]->type(FL_TOGGLE_BUTTON);
      view_butt[14]->down_box(TOGGLE_BOX);
      view_butt[14]->selection_color(TOGGLE_COLOR);

      view_butt[15] = new Fl_Check_Button(2 * WB, 2 * WB + 3 * BH, BW / 2 - WB, BH, "Triangles");
      view_butt[15]->type(FL_TOGGLE_BUTTON);
      view_butt[15]->down_box(TOGGLE_BOX);
      view_butt[15]->selection_color(TOGGLE_COLOR);

      view_butt[16] = new Fl_Check_Button(2 * WB, 2 * WB + 4 * BH, BW / 2 - WB, BH, "Quadrangles");
      view_butt[16]->type(FL_TOGGLE_BUTTON);
      view_butt[16]->down_box(TOGGLE_BOX);
      view_butt[16]->selection_color(TOGGLE_COLOR);

      view_butt[17] = new Fl_Check_Button(2 * WB, 2 * WB + 5 * BH, BW / 2 - WB, BH, "Tetrahedra");
      view_butt[17]->type(FL_TOGGLE_BUTTON);
      view_butt[17]->down_box(TOGGLE_BOX);
      view_butt[17]->selection_color(TOGGLE_COLOR);

      view_butt[18] = new Fl_Check_Button(2 * WB, 2 * WB + 6 * BH, BW / 2 - WB, BH, "Hexahedra");
      view_butt[18]->type(FL_TOGGLE_BUTTON);
      view_butt[18]->down_box(TOGGLE_BOX);
      view_butt[18]->selection_color(TOGGLE_COLOR);

      view_butt[19] = new Fl_Check_Button(2 * WB, 2 * WB + 7 * BH, BW / 2 - WB, BH, "Prisms");
      view_butt[19]->type(FL_TOGGLE_BUTTON);
      view_butt[19]->down_box(TOGGLE_BOX);
      view_butt[19]->selection_color(TOGGLE_COLOR);

      view_butt[20] = new Fl_Check_Button(2 * WB, 2 * WB + 8 * BH, BW / 2 - WB, BH, "Pyramids");
      view_butt[20]->type(FL_TOGGLE_BUTTON);
      view_butt[20]->down_box(TOGGLE_BOX);
      view_butt[20]->selection_color(TOGGLE_COLOR);

      view_butt[4] = new Fl_Check_Button(width / 2, 2 * WB + 1 * BH, BW / 2 - WB, BH, "Scale");
      view_butt[4]->tooltip("(Alt+i)");
      view_butt[4]->type(FL_TOGGLE_BUTTON);
      view_butt[4]->down_box(TOGGLE_BOX);
      view_butt[4]->selection_color(TOGGLE_COLOR);

      view_butt[8] = new Fl_Check_Button(width / 2, 2 * WB + 2 * BH, BW / 2 - WB, BH, "Step value");
      view_butt[8]->type(FL_TOGGLE_BUTTON);
      view_butt[8]->down_box(TOGGLE_BOX);
      view_butt[8]->selection_color(TOGGLE_COLOR);

      view_butt[5] = new Fl_Check_Button(width / 2, 2 * WB + 3 * BH, BW / 2 - WB, BH, "Annotations");
      view_butt[5]->tooltip("(Alt+n)");
      view_butt[5]->type(FL_TOGGLE_BUTTON);
      view_butt[5]->down_box(TOGGLE_BOX);
      view_butt[5]->selection_color(TOGGLE_COLOR);

      view_butt[10] = new Fl_Check_Button(width / 2, 2 * WB + 4 * BH, BW / 2 - WB, BH, "Element edges");
      view_butt[10]->type(FL_TOGGLE_BUTTON);
      view_butt[10]->down_box(TOGGLE_BOX);
      view_butt[10]->selection_color(TOGGLE_COLOR);

      static Fl_Menu_Item menu_boundary[] = {
	{"None", 0, 0, 0},
	{"Dimension - 1", 0, 0, 0},
	{"Dimension - 2", 0, 0, 0},
	{"Dimension - 3", 0, 0, 0},
	{0}
      };
      view_choice[9] = new Fl_Choice(width / 2, 2 * WB + 5 * BH, IW, BH, "Boundary");
      view_choice[9]->menu(menu_boundary);
      view_choice[9]->align(FL_ALIGN_RIGHT);

      view_butt[21] = new Fl_Check_Button(width / 2, 2 * WB + 6 * BH, BW / 2 - WB, BH, "Scalar values");
      view_butt[21]->type(FL_TOGGLE_BUTTON);
      view_butt[21]->down_box(TOGGLE_BOX);
      view_butt[21]->selection_color(TOGGLE_COLOR);

      view_butt[22] = new Fl_Check_Button(width / 2, 2 * WB + 7 * BH, BW / 2 - WB, BH, "Vector values");
      view_butt[22]->type(FL_TOGGLE_BUTTON);
      view_butt[22]->down_box(TOGGLE_BOX);
      view_butt[22]->selection_color(TOGGLE_COLOR);

      view_butt[23] = new Fl_Check_Button(width / 2, 2 * WB + 8 * BH, BW / 2 - WB, BH, "Tensor values");
      view_butt[23]->type(FL_TOGGLE_BUTTON);
      view_butt[23]->down_box(TOGGLE_BOX);
      view_butt[23]->selection_color(TOGGLE_COLOR);
      
      o->end();
    }
    {
      view_range = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Range");
      view_range->hide();

      view_value[30] = new Fl_Value_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "Number of intervals");
      view_value[30]->align(FL_ALIGN_RIGHT);
      view_value[30]->minimum(1);
      view_value[30]->maximum(256);
      view_value[30]->step(1);

      static Fl_Menu_Item menu_iso[] = {
        {"Iso-values", 0, 0, 0},
        {"Filled iso-values", 0, 0, 0},
        {"Continuous map", 0, 0, 0},
        {"Numeric values", 0, 0, 0},
        {0}
      };
      view_choice[0] = new Fl_Choice(2 * WB, 2 * WB + 2 * BH, IW, BH, "Intervals type");
      view_choice[0]->menu(menu_iso);
      view_choice[0]->align(FL_ALIGN_RIGHT);
      view_choice[0]->tooltip("(Alt+t)");

      static Fl_Menu_Item menu_range[] = {
        {"Default", 0, (Fl_Callback *)view_options_custom_cb, (void*)0},
        {"Per time step", 0, (Fl_Callback *)view_options_custom_cb, (void*)0},
        {"Custom", 0, (Fl_Callback *)view_options_custom_cb, (void*)1},
        {0}
      };
      view_choice[7] = new Fl_Choice(2 * WB, 2 * WB + 3 * BH, IW, BH, "Range type");
      view_choice[7]->menu(menu_range);
      view_choice[7]->align(FL_ALIGN_RIGHT);

      view_value[31] = new Fl_Value_Input(2 * WB, 2 * WB + 4 * BH, IW, BH, "Custom minimum");
      view_value[31]->align(FL_ALIGN_RIGHT);

      view_value[32] = new Fl_Value_Input(2 * WB, 2 * WB + 5 * BH, IW, BH, "Custom maximum");
      view_value[32]->align(FL_ALIGN_RIGHT);

      static Fl_Menu_Item menu_scale[] = {
        {"Linear", 0, 0, 0},
        {"Logarithmic", 0, 0, 0},
        {"Double logarithmic", 0, 0, 0},
        {0}
      };
      view_choice[1] = new Fl_Choice(2 * WB, 2 * WB + 6 * BH, IW, BH, "Scale");
      view_choice[1]->menu(menu_scale);
      view_choice[1]->align(FL_ALIGN_RIGHT);

      view_butt[38] = new Fl_Check_Button(2 * WB, 2 * WB + 7 * BH, BW, BH, "Saturate values");
      view_butt[38]->type(FL_TOGGLE_BUTTON);
      view_butt[38]->down_box(TOGGLE_BOX);
      view_butt[38]->selection_color(TOGGLE_COLOR);

      view_range->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Offset");
      o->hide();

      view_value[40] = new Fl_Value_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "X offset");
      view_value[40]->align(FL_ALIGN_RIGHT);

      view_value[41] = new Fl_Value_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "Y offset");
      view_value[41]->align(FL_ALIGN_RIGHT);

      view_value[42] = new Fl_Value_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "Z offset");
      view_value[42]->align(FL_ALIGN_RIGHT);

      view_value[43] = new Fl_Value_Input(width / 2, 2 * WB + 1 * BH, IW, BH, "X raise");
      view_value[43]->align(FL_ALIGN_RIGHT);

      view_value[44] = new Fl_Value_Input(width / 2, 2 * WB + 2 * BH, IW, BH, "Y raise");
      view_value[44]->align(FL_ALIGN_RIGHT);

      view_value[45] = new Fl_Value_Input(width / 2, 2 * WB + 3 * BH, IW, BH, "Z raise");
      view_value[45]->align(FL_ALIGN_RIGHT);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Aspect");
      o->hide();

      view_value[12] = new Fl_Value_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "Explosion factor");
      view_value[12]->minimum(0.);
      view_value[12]->step(0.01);
      view_value[12]->maximum(1.);
      view_value[12]->align(FL_ALIGN_RIGHT);

      view_choice[5] = new Fl_Choice(2 * WB, 2 * WB + 2 * BH, IW, BH, "Point display");
      view_choice[5]->menu(menu_point_display);
      view_choice[5]->align(FL_ALIGN_RIGHT);

      view_value[61] = new Fl_Value_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "Point size");
      view_value[61]->minimum(0.1);
      view_value[61]->maximum(50);
      view_value[61]->step(0.1);
      view_value[61]->align(FL_ALIGN_RIGHT);

      view_choice[6] = new Fl_Choice(2 * WB, 2 * WB + 4 * BH, IW, BH, "Line display");
      view_choice[6]->menu(menu_line_display);
      view_choice[6]->align(FL_ALIGN_RIGHT);

      view_value[62] = new Fl_Value_Input(2 * WB, 2 * WB + 5 * BH, IW, BH, "Line width");
      view_value[62]->minimum(0.1);
      view_value[62]->maximum(50);
      view_value[62]->step(0.1);
      view_value[62]->align(FL_ALIGN_RIGHT);

      {
        view_vector = new Fl_Group(2 * WB, 2 * WB + 6 * BH, width - 2 * WB, 5 * BH, 0);

        static Fl_Menu_Item menu_vectype[] = {
          {"Line", 0, 0, 0},
          {"Arrow", 0, 0, 0},
          {"Pyramid", 0, 0, 0},
          {"3D arrow", 0, 0, 0},
          {"Displacement", 0, 0, 0},
          {0}
        };
        view_choice[2] = new Fl_Choice(2 * WB, 2 * WB + 6 * BH, IW, BH, "Vector display");
        view_choice[2]->menu(menu_vectype);
        view_choice[2]->align(FL_ALIGN_RIGHT);

	view_push_butt[0] = new Fl_Button(2 * IW - 2 * WB, 2 * WB + 6 * BH, (int)(1.5*BB), BH, "Edit arrow shape");
      
        view_value[60] = new Fl_Value_Input(2 * WB, 2 * WB + 7 * BH, IW, BH, "Arrow size");
        view_value[60]->minimum(0);
        view_value[60]->maximum(500);
        view_value[60]->step(1);
        view_value[60]->align(FL_ALIGN_RIGHT);

        view_value[63] = new Fl_Value_Input(2 * WB, 2 * WB + 8 * BH, IW, BH, "Displacement factor");
        view_value[63]->align(FL_ALIGN_RIGHT);

        static Fl_Menu_Item menu_vecloc[] = {
          {"Cell centered", 0, 0, 0},
          {"Vertex centered", 0, 0, 0},
          {0}
        };
        view_choice[3] = new Fl_Choice(2 * WB, 2 * WB + 9 * BH, IW, BH, "Arrow location");
        view_choice[3]->menu(menu_vecloc);
        view_choice[3]->align(FL_ALIGN_RIGHT);

        static Fl_Menu_Item menu_tensor[] = {
          {"Von-Mises", 0, 0, 0},
          //{"Eigenvectors", 0, 0, 0}, //not implemented yet
          {0}
        };
        view_choice[4] = new Fl_Choice(2 * WB, 2 * WB + 10 * BH, IW, BH, "Tensor display");
        view_choice[4]->menu(menu_tensor);
        view_choice[4]->align(FL_ALIGN_RIGHT);

        view_vector->end();
      }

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Light");
      o->hide();

      view_butt[11] = new Fl_Check_Button(2 * WB, 2 * WB + 1 * BH, BW, BH, "Enable lighting");
      view_butt[11]->tooltip("(Alt+w)");
      view_butt[11]->type(FL_TOGGLE_BUTTON);
      view_butt[11]->down_box(TOGGLE_BOX);
      view_butt[11]->selection_color(TOGGLE_COLOR);

      view_butt[9]  = new Fl_Check_Button(2 * WB, 2 * WB + 2 * BH, BW, BH, "Use two-side lighting");
      view_butt[9]->type(FL_TOGGLE_BUTTON);
      view_butt[9]->down_box(TOGGLE_BOX);
      view_butt[9]->selection_color(TOGGLE_COLOR);

      view_butt[12] = new Fl_Check_Button(2 * WB, 2 * WB + 3 * BH, BW, BH, "Smooth normals");
      view_butt[12]->type(FL_TOGGLE_BUTTON);
      view_butt[12]->down_box(TOGGLE_BOX);
      view_butt[12]->selection_color(TOGGLE_COLOR);

      view_value[10] = new Fl_Value_Input(2 * WB, 2 * WB + 4 * BH, IW, BH, "Smoothing threshold angle");
      view_value[10]->minimum(0.);
      view_value[10]->step(1.);
      view_value[10]->maximum(180.);
      view_value[10]->align(FL_ALIGN_RIGHT);
      
      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 2 * WB - BH, "Color");
      o->hide();

      view_colorbar_window = new Colorbar_Window(2 * WB, 2 * WB + BH, width - 4 * WB, height - 4 * WB - BH);
      view_colorbar_window->end();

      // this exposes the MacOSX clipping bug with a regular subwindow:
      //view_colorbar_window->hide();
      //Fl_Window *ww = new Fl_Window(2 * WB, 2 * WB + BH, width - 4 * WB, height - 4 * WB - BH);
      //ww->box(FL_DOWN_BOX);
      //ww->end();

      o->end();
    }
    o->end();
  }

  view_window->end();

  opt_window->position(CTX.opt_position[0], CTX.opt_position[1]);
  opt_window->end();
}

void GUI::update_view_window(int num)
{
  if(num < 0 || num >= List_Nbr(CTX.post.list))
    return;

  view_number = num;
  Post_View *v = (Post_View *) List_Pointer(CTX.post.list, num);

  opt_view_name(num, GMSH_GUI, NULL);
  opt_view_format(num, GMSH_GUI, NULL);
  opt_view_type(num, GMSH_GUI, 0);
  opt_view_show_scale(num, GMSH_GUI, 0);
  opt_view_draw_strings(num, GMSH_GUI, 0);
  opt_view_auto_position(num, GMSH_GUI, 0);
  if(v->NbSP) {
    view_butt[2]->activate();
    view_butt[3]->activate();
  }
  else {
    view_butt[2]->deactivate();
    view_butt[3]->deactivate();
  }
  opt_view_position0(num, GMSH_GUI, 0);
  opt_view_position1(num, GMSH_GUI, 0);
  opt_view_size0(num, GMSH_GUI, 0);
  opt_view_size1(num, GMSH_GUI, 0);

  if(v->TextOnly) {
    view_range->deactivate();
    view_butt[1]->deactivate();
  }
  else {
    view_range->activate();
    view_butt[1]->activate();
  }
  opt_view_show_element(num, GMSH_GUI, 0);
  opt_view_light(num, GMSH_GUI, 0);
  opt_view_light_two_side(num, GMSH_GUI, 0);
  opt_view_smooth_normals(num, GMSH_GUI, 0);
  opt_view_angle_smooth_normals(num, GMSH_GUI, 0);
  opt_view_boundary(num, GMSH_GUI, 0);
  opt_view_explode(num, GMSH_GUI, 0);
  opt_view_draw_points(num, GMSH_GUI, 0);
  opt_view_draw_lines(num, GMSH_GUI, 0);
  opt_view_draw_triangles(num, GMSH_GUI, 0);
  opt_view_draw_quadrangles(num, GMSH_GUI, 0);
  opt_view_draw_tetrahedra(num, GMSH_GUI, 0);
  opt_view_draw_hexahedra(num, GMSH_GUI, 0);
  opt_view_draw_prisms(num, GMSH_GUI, 0);
  opt_view_draw_pyramids(num, GMSH_GUI, 0);
  opt_view_draw_scalars(num, GMSH_GUI, 0);
  opt_view_draw_vectors(num, GMSH_GUI, 0);
  opt_view_draw_tensors(num, GMSH_GUI, 0);

  if(v->NbSP)
    view_2d->activate();
  else
    view_2d->deactivate();
  opt_view_abscissa_name(num, GMSH_GUI, NULL);
  opt_view_abscissa_format(num, GMSH_GUI, NULL);
  opt_view_nb_abscissa(num, GMSH_GUI, 0);
  opt_view_grid(num, GMSH_GUI, 0);

  opt_view_nb_iso(num, GMSH_GUI, 0);
  opt_view_intervals_type(num, GMSH_GUI, 0);
  int range_type = (int)opt_view_range_type(num, GMSH_GUI, 0);
  opt_view_custom_min(num, GMSH_GUI, 0);
  opt_view_custom_max(num, GMSH_GUI, 0);
  for(int i = 31; i <= 32; i++) {
    view_value[i]->minimum(v->CustomMin);
    view_value[i]->maximum(v->CustomMax);
    if(range_type == DRAW_POST_RANGE_CUSTOM)
      view_options_custom_cb(NULL, (void*)1);
    else
      view_options_custom_cb(NULL, (void*)0);
  }
  opt_view_scale_type(num, GMSH_GUI, 0);
  opt_view_saturate_values(num, GMSH_GUI, 0);

  opt_view_offset0(num, GMSH_GUI, 0);
  opt_view_offset1(num, GMSH_GUI, 0);
  opt_view_offset2(num, GMSH_GUI, 0);
  double val1 = 10. * CTX.lc;
  for(int i = 40; i <= 42; i++) {
    view_value[i]->step(val1/100.);
    view_value[i]->minimum(-val1);
    view_value[i]->maximum(val1);
  }
  opt_view_raise0(num, GMSH_GUI, 0);
  opt_view_raise1(num, GMSH_GUI, 0);
  opt_view_raise2(num, GMSH_GUI, 0);
  double maxval = MAX(fabs(v->Min), fabs(v->Max));
  if(!maxval) maxval = 1.;
  double val2 = 2. * CTX.lc / maxval;
  for(int i = 43; i <= 45; i++) {
    view_value[i]->step(val2/100.);
    view_value[i]->minimum(-val2);
    view_value[i]->maximum(val2);
  }

  if(v->NbTimeStep == 1) {
    view_value[50]->deactivate();
    view_butt_rep[0]->deactivate();
    view_butt_rep[1]->deactivate();
    view_butt[8]->deactivate();
  }
  else {
    view_value[50]->activate();
    view_butt_rep[0]->activate();
    view_butt_rep[1]->activate();
    view_butt[8]->activate();
  }
  view_value[50]->callback(view_options_timestep_cb, (void *)num);
  view_value[50]->maximum(v->NbTimeStep - 1);
  view_butt_rep[0]->callback(view_options_timestep_decr_cb, (void *)num);
  view_butt_rep[1]->callback(view_options_timestep_incr_cb, (void *)num);
  opt_view_timestep(num, GMSH_GUI, 0);
  opt_view_show_time(num, GMSH_GUI, 0);

  if(v->ScalarOnly)
    view_vector->deactivate();
  else
    view_vector->activate();
  opt_view_point_size(num, GMSH_GUI, 0);
  opt_view_point_type(num, GMSH_GUI, 0);
  opt_view_line_width(num, GMSH_GUI, 0);
  opt_view_line_type(num, GMSH_GUI, 0);
  opt_view_vector_type(num, GMSH_GUI, 0);
  opt_view_arrow_size(num, GMSH_GUI, 0);
  opt_view_displacement_factor(num, GMSH_GUI, 0);
  opt_view_arrow_location(num, GMSH_GUI, 0);
  opt_view_tensor_type(num, GMSH_GUI, 0);
  view_push_butt[0]->callback(view_arrow_param_cb, (void*)num);

  view_colorbar_window->update(v->Name, v->Min, v->Max, &v->CT, &v->Changed);
}

// Create the window for the statistics

void GUI::create_statistics_window()
{
  int i, num = 0;

  if(stat_window) {
    if(!stat_window->shown())
      set_statistics();
    stat_window->show();
    return;
  }

  int width = 26 * fontsize;
  int height = 5 * WB + 18 * BH;

  stat_window = new Fl_Window(width, height, "Statistics");
  stat_window->box(WINDOW_BOX);
  {
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width - 2 * WB, height - 3 * WB - BH);
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Geometry");
      o->hide();
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 1 * BH, IW, BH, "Points");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 2 * BH, IW, BH, "Lines");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 3 * BH, IW, BH, "Surfaces");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 4 * BH, IW, BH, "Volumes");
      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Mesh");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 1 * BH, IW, BH, "Nodes on Lines");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 2 * BH, IW, BH, "Nodes on surfaces");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 3 * BH, IW, BH, "Nodes in volumes");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 4 * BH, IW, BH, "Second order nodes");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 5 * BH, IW, BH, "Triangles");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 6 * BH, IW, BH, "Quadrangles");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 7 * BH, IW, BH, "Tetrahedra");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 8 * BH, IW, BH, "Hexahedra");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 9 * BH, IW, BH, "Prisms");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 10 * BH, IW, BH, "Pyramids");

      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 11 * BH, IW, BH, "Time for 1D mesh");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 12 * BH, IW, BH, "Time for 2D mesh");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 13 * BH, IW, BH, "Time for 3D mesh");

      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 14 * BH, IW, BH, "Gamma factor");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 15 * BH, IW, BH, "Eta factor");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 16 * BH, IW, BH, "Rho factor");

      Fl_Button *b0 = new Fl_Button(width - BB - 2 * WB, 2 * WB + 14 * BH, BB, BH, "Graph");
      b0->callback(statistics_histogram_cb, (void *)0);
      Fl_Button *b1 = new Fl_Button(width - BB - 2 * WB, 2 * WB + 15 * BH, BB, BH, "Graph");
      b1->callback(statistics_histogram_cb, (void *)1);
      Fl_Button *b2 = new Fl_Button(width - BB - 2 * WB, 2 * WB + 16 * BH, BB, BH, "Graph");
      b2->callback(statistics_histogram_cb, (void *)2);

      o->end();
    }
    {
      Fl_Group *o = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Post-processing");
      o->hide();
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 1 * BH, IW, BH, "Views");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 2 * BH, IW, BH, "Visible points");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 3 * BH, IW, BH, "Visible lines");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 4 * BH, IW, BH, "Visible triangles");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 5 * BH, IW, BH, "Visible quadrangles");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 6 * BH, IW, BH, "Visible tetrahedra");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 7 * BH, IW, BH, "Visible hexahedra");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 8 * BH, IW, BH, "Visible prisms");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 9 * BH, IW, BH, "Visible pyramids");
      stat_value[num++] = new Fl_Output(2 * WB, 2 * WB + 10 * BH, IW, BH, "Visible strings");
      o->end();
    }
    o->end();
  }

  for(i = 0; i < num; i++) {
    stat_value[i]->align(FL_ALIGN_RIGHT);
    stat_value[i]->value(0);
  }

  {
    Fl_Return_Button *o = new Fl_Return_Button(width - 2 * BB - 2 * WB, height - BH - WB, BB, BH, "Update");
    o->callback(statistics_update_cb);
  }
  {
    Fl_Button *o = new Fl_Button(width - BB - WB, height - BH - WB, BB, BH, "Cancel");
    o->callback(cancel_cb, (void *)stat_window);
  }


  stat_window->position(CTX.stat_position[0], CTX.stat_position[1]);
  stat_window->end();
}

void GUI::set_statistics()
{

  int num = 0;
  static double s[50];
  static char label[50][256];

  extern Mesh *THEM;

  Mesh_Quality(THEM);
  GetStatistics(s);

  // geom
  sprintf(label[num], "%g", s[0]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[1]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[2]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[3]); stat_value[num]->value(label[num]); num++;

  // mesh
  sprintf(label[num], "%g", s[4]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[5]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[6]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[16]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[7]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[8]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[9]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[10]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[11]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[12]); stat_value[num]->value(label[num]); num++;

  sprintf(label[num], "%g", s[13]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[14]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[15]); stat_value[num]->value(label[num]); num++;

  sprintf(label[num], "%.4g (%.4g->%.4g)", s[17], s[18], s[19]);
  stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%.4g (%.4g->%.4g)", s[20], s[21], s[22]);
  stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%.4g (%.4g->%.4g)", s[23], s[24], s[25]);
  stat_value[num]->value(label[num]); num++;

  // post
  sprintf(label[num], "%g", s[26]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", s[36], s[27]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", s[37], s[28]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", s[38], s[29]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", s[39], s[30]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", s[40], s[31]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", s[41], s[32]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", s[42], s[33]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", s[43], s[34]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", s[44], s[35]); stat_value[num]->value(label[num]); num++;
}


// Create the window for the plugins

void GUI::add_multiline_in_browser(Fl_Browser * o, char *prefix, char *str)
{
  int start = 0, len;
  char *buff;
  if(!str || !strlen(str) || !strcmp(str, "\n")) {
    o->add(" ");
    return;
  }
  for(unsigned int i = 0; i < strlen(str); i++) {
    if(i == strlen(str) - 1 || str[i] == '\n') {
      len = i - start + (str[i] == '\n' ? 0 : 1);
      buff = new char[len + strlen(prefix) + 2];
      strcpy(buff, prefix);
      strncat(buff, &str[start], len);
      buff[len + strlen(prefix)] = '\0';
      o->add(buff);
      start = i + 1;
    }
  }
}

PluginDialogBox *GUI::create_plugin_window(GMSH_Plugin * p)
{
  char buffer[1024], namep[1024], copyright[256], author[256], help[1024];

  // get plugin info

  int m = p->getNbOptionsStr();
  int n = p->getNbOptions();
  p->getName(namep);
  p->getInfos(author, copyright, help);

  // create window

  int width = 27 * fontsize;
  int height = ((n+m > 8 ? n+m : 8) + 2) * BH + 5 * WB;

  PluginDialogBox *pdb = new PluginDialogBox;
  pdb->main_window = new Fl_Window(width, height);
  pdb->main_window->box(WINDOW_BOX);
  sprintf(buffer, "%s Plugin", namep);
  char *nbuffer = new char[strlen(buffer) + 1];
  strcpy(nbuffer, buffer);
  pdb->main_window->label(nbuffer);

  {
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width - 2 * WB, height - 3 * WB - 1 * BH);
    {
      Fl_Group *g = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Options");

      if(m > 20) m = 20;
      if(n > 20) n = 20;

      int k = 0;
      for(int i = 0; i < m; i++) {
        StringXString *sxs = p->getOptionStr(i);
        pdb->input[i] = new Fl_Input(2 * WB, 2 * WB + (k + 1) * BH, IW, BH, sxs->str);
        pdb->input[i]->align(FL_ALIGN_RIGHT);
        pdb->input[i]->value(sxs->def);
	k++;
      }
      for(int i = 0; i < n; i++) {
        StringXNumber *sxn = p->getOption(i);
        pdb->value[i] = new Fl_Value_Input(2 * WB, 2 * WB + (k + 1) * BH, IW, BH, sxn->str);
        pdb->value[i]->align(FL_ALIGN_RIGHT);
        pdb->value[i]->value(sxn->def);
	k++;
      }

      g->end();
    }
    {
      Fl_Group *g = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "About");

      Fl_Browser *o = new Fl_Browser(2 * WB, 2 * WB + 1 * BH, width - 4 * WB, height - 5 * WB - 2 * BH);

      o->add(" ");
      add_multiline_in_browser(o, "@c@b@.", namep);
      o->add(" ");
      add_multiline_in_browser(o, "", help);
      o->add(" ");
      add_multiline_in_browser(o, "Author: ", author);
      add_multiline_in_browser(o, "Copyright (C) ", copyright);
      o->add(" ");

      g->end();
    }
    o->end();
  }

  pdb->run_button = new Fl_Return_Button(width - 2 * BB - 2 * WB, height - BH - WB, BB, BH, "Run");

  Fl_Button *cancel = new Fl_Button(width - BB - WB, height - BH - WB, BB, BH, "Cancel");
  cancel->callback(cancel_cb, (void *)pdb->main_window);

  //pdb->main_window->resizable(new Fl_Box(2 * WB, 2 * WB + BH, 10, 10));
  
  pdb->main_window->position(m_window->x() + m_window->w() / 2 - width / 2,
			     m_window->y() + 6 * BH);
  pdb->main_window->end();

  return pdb;
}

// Create the window for the messages

void GUI::create_message_window()
{

  if(msg_window) {
    if(msg_window->shown())
      msg_window->redraw();
    else
      msg_window->show();
    return;
  }

  int width = CTX.msg_size[0];
  int height = CTX.msg_size[1];

  msg_window = new Fl_Window(width, height, "Messages");
  msg_window->box(WINDOW_BOX);

  msg_browser = new Fl_Browser(WB, WB, width - 2 * WB, height - 3 * WB - BH);
  msg_browser->textfont(FL_COURIER);
  msg_browser->type(FL_MULTI_BROWSER);
  msg_browser->callback(message_copy_cb);

  {
    Fl_Return_Button *o = new Fl_Return_Button(width - 3 * BB - 3 * WB, height - BH - WB, BB, BH, "Save");
    o->callback(message_save_cb);
  }
  {
    Fl_Button *o = new Fl_Button(width - 2 * BB - 2 * WB, height - BH - WB, BB, BH, "Clear");
    o->callback(message_clear_cb);
  }
  {
    Fl_Button *o = new Fl_Button(width - BB - WB, height - BH - WB, BB, BH, "Cancel");
    o->callback(cancel_cb, (void *)msg_window);
  }

  msg_window->resizable(new Fl_Box(WB, WB, 100, 10));
  msg_window->size_range(WB + 100 + 3 * BB + 4 * WB, 100);

  msg_window->position(CTX.msg_position[0], CTX.msg_position[1]);
  msg_window->end();
}

void GUI::add_message(char *msg)
{
  for(int i = 0; i < (int)strlen(msg); i++)
    if(msg[i] == '\n')
      msg[i] = ' ';
  msg_browser->add(msg, 0);
  msg_browser->bottomline(msg_browser->size());
}

void GUI::save_message(char *filename)
{
  FILE *fp;

  if(!(fp = fopen(filename, "w"))) {
    Msg(GERROR, "Unable to open file '%s'", filename);
    return;
  }

  Msg(INFO, "Writing log file '%s'", filename);
  for(int i = 1; i <= msg_browser->size(); i++) {
    const char *c = msg_browser->text(i);
    if(c[0] == '@')
      fprintf(fp, "%s\n", &c[5]);
    else
      fprintf(fp, "%s\n", c);
  }

  Msg(INFO, "Wrote log file '%s'", filename);
  Msg(STATUS2N, "Wrote '%s'", filename);
  fclose(fp);
}

void GUI::fatal_error(char *filename)
{
  fl_alert("A fatal error has occurred, which will force Gmsh to exit "
           "(all messages have been saved in the error log file '%s')",
           filename);
}

// Create the visibility window

void GUI::reset_visibility()
{
  if(vis_window) {
    vis_browser->clear();
    if(vis_window->shown())
      visibility_cb(NULL, NULL);
  }
}

void GUI::create_visibility_window()
{

  if(vis_window) {
    vis_window->show();
    return;
  }

  static int cols[5] = { 15, 95, 95, 180, 0 };
  static Fl_Menu_Item type_table[] = {
    {"Elementary", 0, (Fl_Callback *) visibility_cb},
    {"Physical", 0, (Fl_Callback *) visibility_cb},
    {"Partitions", 0, (Fl_Callback *) visibility_cb},
    {0}
  };
  static Fl_Menu_Item browser_mode_table[] = {
    {"Geometry+Mesh", 0, (Fl_Callback *) visibility_cb},
    {"Geometry", 0, (Fl_Callback *) visibility_cb},
    {"Mesh", 0, (Fl_Callback *) visibility_cb},
    {0}
  };
  static Fl_Menu_Item input_mode_table[] = {
    {"Node", 0, 0},
    {"Element", 0, 0},
    {"Point", 0, 0},
    {"Line", 0, 0},
    {"Surface", 0, 0},
    {"Volume", 0, 0},
    {0}
  };

  int width = cols[0] + cols[1] + cols[2] + cols[3] + 2 * WB;
  int height = 15 * BH;

  vis_window = new Fl_Window(width, height, "Visibility");
  vis_window->box(WINDOW_BOX);

  int brw = width - 2 * WB;

  vis_type = new Fl_Choice(1 * WB, 1 * WB + 0 * BH, brw / 3, BH);
  vis_type->menu(type_table);

  vis_browser_mode = new Fl_Choice(2 * WB + (brw - 2 * WB) / 3, 1 * WB + 0 * BH, (brw - 2 * WB) / 3, BH);
  vis_browser_mode->menu(browser_mode_table);

  vis_butt[0] = new Fl_Check_Button(3 * WB + 2 * (brw - 2 * WB) / 3, 1 * WB + 0 * BH, (brw - 2 * WB) / 3, BH, "Recursive");
  vis_butt[0]->type(FL_TOGGLE_BUTTON);
  vis_butt[0]->down_box(TOGGLE_BOX);
  vis_butt[0]->selection_color(TOGGLE_COLOR);
  vis_butt[0]->value(1);

  Fl_Button *o0 = new Fl_Button(1 * WB, 2 * WB + 1 * BH, cols[0], BH, "*");
  o0->callback(visibility_sort_cb, (void *)0);

  Fl_Button *o1 = new Fl_Button(1 * WB + cols[0], 2 * WB + 1 * BH, cols[1], BH, "Type");
  o1->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  o1->callback(visibility_sort_cb, (void *)1);

  Fl_Button *o2 = new Fl_Button(1 * WB + cols[0] + cols[1], 2 * WB + 1 * BH, cols[2], BH, "Number");
  o2->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  o2->callback(visibility_sort_cb, (void *)2);

  Fl_Button *o3 = new Fl_Button(1 * WB + cols[0] + cols[1] + cols[2], 2 * WB + 1 * BH, cols[3], BH, "Name");
  o3->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  o3->callback(visibility_sort_cb, (void *)3);

  vis_browser = new Fl_Browser(1 * WB, 2 * WB + 2 * BH, brw, height - 5 * WB - 4 * BH);
  vis_browser->textfont(FL_COURIER);
  vis_browser->type(FL_MULTI_BROWSER);
  vis_browser->column_widths(cols);

  vis_window->resizable(new Fl_Box(width - 3 * WB - 2 * BB - 10, 3 * WB + 2 * BH + 10, 10, 10));
  vis_window->size_range(width, 5 * BH + 5 * WB);

  {
    int ww = (((brw - 2 * WB) / 3) - WB) / 2;

    Fl_Button *o1 = new Fl_Button(1 * WB, height - 2 * WB - 2 * BH, ww, BH, "Show");
    o1->callback(visibility_number_cb, (void *)1);
    Fl_Button *o2 = new Fl_Button(2 * WB + ww, height - 2 * WB - 2 * BH, ww, BH, "Hide");
    o2->callback(visibility_number_cb, (void *)0);

    vis_input_mode = new Fl_Choice(2 * WB + (brw - 2 * WB) / 3, height - 2 * WB - 2 * BH,
				   (brw - 2 * WB) / 3, BH);
    vis_input_mode->menu(input_mode_table);
    
    vis_input = new Fl_Input(3 * WB + 2 * (brw - 2 * WB) / 3, height - 2 * WB - 2 * BH,
			     (brw - 2 * WB) / 3, BH);
    vis_input->value("*");
    vis_input->tooltip("Enter an entity number, or *");
  }

  {
    Fl_Return_Button *o = new Fl_Return_Button(width - 2 * BB - 2 * WB, height - BH - WB, BB, BH, "Apply");
    o->callback(visibility_ok_cb);
  }
  {
    Fl_Button *o = new Fl_Button(width - BB - WB, height - BH - WB, BB, BH, "Cancel");
    o->callback(cancel_cb, (void *)vis_window);
  }

  vis_window->position(CTX.vis_position[0], CTX.vis_position[1]);
  vis_window->end();
}

// Create the clipping planes window

void GUI::reset_clip_browser()
{
  char str[128];
  clip_browser->clear();
  clip_browser->add("Geometry");
  clip_browser->add("Mesh");
  for(int i = 0; i < List_Nbr(CTX.post.list); i++) {
    if(i == NB_BUTT_MAX)
      break;
    sprintf(str, "View [%d]", i);
    clip_browser->add(str);
  }
  int idx = clip_choice->value();
  clip_browser->deselect();
  for(int i = 0; i < clip_browser->size(); i++)
    if(CTX.clip[idx] & (1<<i))
      clip_browser->select(i+1);
  for(int i = 0; i < 4; i++)
    clip_value[i]->value(CTX.clip_plane[idx][i]);

  for(int i = 0; i < 3; i++) {
    clip_value[i]->step(0.01);
    clip_value[i]->minimum(-1.0);
    clip_value[i]->maximum(1.0);
  }
  double val1 = CTX.lc;
  clip_value[3]->step(val1/200.);
  clip_value[3]->minimum(-val1);
  clip_value[3]->maximum(val1);
}

void GUI::create_clip_window()
{
  if(clip_window) {
    reset_clip_browser();
    clip_window->show();
    return;
  }

  static Fl_Menu_Item plane_number[] = {
    {"Plane 0", 0, 0},
    {"Plane 1", 0, 0},
    {"Plane 2", 0, 0},
    {"Plane 3", 0, 0},
    {"Plane 4", 0, 0},
    {"Plane 5", 0, 0},
    {0}
  };

  int width = 3 * BB + 4 * WB;
  int height = 6 * BH + 3 * WB;
  int brw = 105;
  int BW = width - brw - 3 * WB - 2 * fontsize;

  clip_window = new Fl_Window(width, height, "Clipping Planes");
  clip_window->box(WINDOW_BOX);

  clip_browser = new Fl_Multi_Browser(1 * WB, 1 * WB, brw, 5 * BH);
  clip_browser->callback(clip_update_cb);

  clip_choice = new Fl_Choice(2 * WB + brw, 1 * WB + 0 * BH, BW, BH);
  clip_choice->menu(plane_number);
  clip_choice->callback(clip_num_cb);

  clip_value[0] = new Fl_Value_Input(2 * WB + brw, 1 * WB + 1 * BH, BW, BH, "A");
  clip_value[1] = new Fl_Value_Input(2 * WB + brw, 1 * WB + 2 * BH, BW, BH, "B");
  clip_value[2] = new Fl_Value_Input(2 * WB + brw, 1 * WB + 3 * BH, BW, BH, "C");
  clip_value[3] = new Fl_Value_Input(2 * WB + brw, 1 * WB + 4 * BH, BW, BH, "D");
  for(int i = 0; i < 4; i++){
    clip_value[i]->align(FL_ALIGN_RIGHT);
    clip_value[i]->callback(clip_update_cb);
  }

  reset_clip_browser();

  {
    Fl_Return_Button *o = new Fl_Return_Button(width - 3 * BB - 3 * WB, height - BH - WB, BB, BH, "Redraw");
    o->callback(redraw_cb);
  }
  {
    Fl_Button *o = new Fl_Button(width - 2 * BB - 2 * WB, height - BH - WB, BB, BH, "Reset");
    o->callback(clip_reset_cb);
  }
  {
    Fl_Button *o = new Fl_Button(width - BB - WB, height - BH - WB, BB, BH, "Cancel");
    o->callback(cancel_cb, (void *)clip_window);
  }

  clip_window->position(CTX.clip_position[0], CTX.clip_position[1]);
  clip_window->end();
}

// Create the about window

void GUI::create_about_window()
{
  char buffer[1024];

  if(about_window) {
    about_window->show();
    return;
  }

  int width = 40 * fontsize;
  int height = 10 * BH;

  about_window = new Fl_Window(width, height, "About Gmsh");
  about_window->box(WINDOW_BOX);

  {
    Fl_Box *o = new Fl_Box(2 * WB, WB, about_width, height - 3 * WB - BH);
    about_bmp = new Fl_Bitmap(about_bits, about_width, about_height);
    about_bmp->label(o);
  }

  {
    Fl_Browser *o = new Fl_Browser(WB + 80, WB, width - 2 * WB - 80, height - 3 * WB - BH);
    o->add(" ");
    o->add("@c@b@.Gmsh");
    o->add("@c@.A three-dimensional finite element mesh generator");
    o->add("@c@.with built-in pre- and post-processing facilities");
    o->add(" ");
    o->add("@c@.Copyright (C) 1997-2004");
#if defined(__APPLE__)
    o->add("@c@.Christophe Geuzaine and Jean-Francois Remacle");
#else
    o->add("@c@.Christophe Geuzaine and Jean-Fran�ois Remacle");
#endif
    o->add(" ");
    o->add("@c@.Please send all questions and bug reports to");
    o->add("@c@b@.gmsh@geuz.org");
    o->add(" ");
    sprintf(buffer, "@c@.Version: %s", GMSH_VERSION);
    o->add(buffer);
    sprintf(buffer, "@c@.License: %s", GMSH_SHORT_LICENSE);
    o->add(buffer);
    sprintf(buffer, "@c@.Graphical user interface toolkit: FLTK %d.%d.%d", 
	    FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION);
    o->add(buffer);
    sprintf(buffer, "@c@.Build OS: %s", GMSH_OS);
    o->add(buffer);
    sprintf(buffer, "@c@.Build options: %s", Get_BuildOptions());
    o->add(buffer);
    sprintf(buffer, "@c@.Build date: %s", GMSH_DATE);
    o->add(buffer);
    sprintf(buffer, "@c@.Build host: %s", GMSH_HOST);
    o->add(buffer);
    sprintf(buffer, "@c@.Packaged by: %s", GMSH_PACKAGER);
    o->add(buffer);
    o->add(" ");
    o->add("@c@.Visit http://www.geuz.org/gmsh/ for more information");
    o->add(" ");
  }

  {
    Fl_Button *o = new Fl_Button(width - 2*BB - 2*WB, height - BH - WB, BB, BH, "License");
    o->callback(help_license_cb);
  }

  {
    Fl_Return_Button *o = new Fl_Return_Button(width - BB - WB, height - BH - WB, BB, BH, "OK");
    o->callback(cancel_cb, (void *)about_window);
  }

  about_window->position(m_window->x() + m_window->w() / 2 - width / 2,
			 m_window->y());

  about_window->end();
}


// Create the window for geometry context dependant definitions

void GUI::create_geometry_context_window(int num)
{
  static Fl_Group *g[10];
  int i;

  if(context_geometry_window) {
    for(i = 0; i < 6; i++)
      g[i]->hide();
    g[num]->show();
    context_geometry_window->show();
    return;
  }

  int width = 31 * fontsize;
  int height = 5 * WB + 9 * BH;

  context_geometry_window = new Fl_Window(width, height, "Contextual geometry definitions");
  context_geometry_window->box(WINDOW_BOX);
  {
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width - 2 * WB, height - 3 * WB - BH);
    // 0: Parameter
    {
      g[0] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Parameter");
      context_geometry_input[0] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "Name");
      context_geometry_input[0]->value("lc");
      context_geometry_input[1] = new Fl_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "Value");
      context_geometry_input[1]->value("1.0");
      for(i = 0; i < 2; i++) {
        context_geometry_input[i]->align(FL_ALIGN_RIGHT);
      }
      {
        Fl_Return_Button *o = new Fl_Return_Button(width - BB - 2 * WB, 2 * WB + 7 * BH, BB, BH, "Add");
        o->callback(con_geometry_define_parameter_cb);
      }
      g[0]->end();
    }
    // 1: Point
    {
      g[1] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Point");
      context_geometry_input[2] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "X coordinate");
      context_geometry_input[2]->value("0.0");
      context_geometry_input[3] = new Fl_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "Y coordinate");
      context_geometry_input[3]->value("0.0");
      context_geometry_input[4] = new Fl_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "Z coordinate");
      context_geometry_input[4]->value("0.0");
      context_geometry_input[5] = new Fl_Input(2 * WB, 2 * WB + 4 * BH, IW, BH, "Characteristic length");
      context_geometry_input[5]->value("1.0");
      for(i = 2; i < 6; i++) {
        context_geometry_input[i]->align(FL_ALIGN_RIGHT);
      }
      {
        Fl_Return_Button *o = new Fl_Return_Button(width - BB - 2 * WB, 2 * WB + 7 * BH, BB, BH, "Add");
        o->callback(con_geometry_define_point_cb);
      }
      g[1]->end();
    }
    // 2: Translation
    {
      g[2] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Translation");
      context_geometry_input[6] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "X component");
      context_geometry_input[6]->value("0.0");
      context_geometry_input[7] = new Fl_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "Y component");
      context_geometry_input[7]->value("0.0");
      context_geometry_input[8] = new Fl_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "Z component");
      context_geometry_input[8]->value("1.0");
      for(i = 6; i < 9; i++) {
        context_geometry_input[i]->align(FL_ALIGN_RIGHT);
      }
      g[2]->end();
    }
    // 3: Rotation
    {
      g[3] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Rotation");
      context_geometry_input[9] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "X coordinate of an axis point");
      context_geometry_input[9]->value("0.0");
      context_geometry_input[10] = new Fl_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "Y coordinate of an axis point");
      context_geometry_input[10]->value("0.0");
      context_geometry_input[11] = new Fl_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "Z coordinate of an axis point");
      context_geometry_input[11]->value("0.0");
      context_geometry_input[12] = new Fl_Input(2 * WB, 2 * WB + 4 * BH, IW, BH, "X component of direction");
      context_geometry_input[12]->value("0.0");
      context_geometry_input[13] = new Fl_Input(2 * WB, 2 * WB + 5 * BH, IW, BH, "Y component of direction");
      context_geometry_input[13]->value("1.0");
      context_geometry_input[14] = new Fl_Input(2 * WB, 2 * WB + 6 * BH, IW, BH, "Z component of direction");
      context_geometry_input[14]->value("0.0");
      context_geometry_input[15] = new Fl_Input(2 * WB, 2 * WB + 7 * BH, IW, BH, "Angle in radians");
      context_geometry_input[15]->value("Pi/4");
      for(i = 9; i < 16; i++) {
        context_geometry_input[i]->align(FL_ALIGN_RIGHT);
      }
      g[3]->end();
    }
    // 4: Scale
    {
      g[4] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Scale");
      context_geometry_input[16] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "X component of direction");
      context_geometry_input[16]->value("1.0");
      context_geometry_input[17] = new Fl_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "Y component of direction");
      context_geometry_input[17]->value("0.0");
      context_geometry_input[18] = new Fl_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "Z component of direction");
      context_geometry_input[18]->value("0.0");
      context_geometry_input[19] = new Fl_Input(2 * WB, 2 * WB + 4 * BH, IW, BH, "Factor");
      context_geometry_input[19]->value("2.0");
      for(i = 16; i < 20; i++) {
        context_geometry_input[i]->align(FL_ALIGN_RIGHT);
      }
      g[4]->end();
    }
    // 5: Symmetry
    {
      g[5] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Symmetry");
      context_geometry_input[20] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "1st plane equation coefficient");
      context_geometry_input[20]->value("1.0");
      context_geometry_input[21] = new Fl_Input(2 * WB, 2 * WB + 2 * BH, IW, BH, "2nd plane equation coefficient");
      context_geometry_input[21]->value("0.0");
      context_geometry_input[22] = new Fl_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "3rd plane equation coefficient");
      context_geometry_input[22]->value("0.0");
      context_geometry_input[23] = new Fl_Input(2 * WB, 2 * WB + 4 * BH, IW, BH, "4th plane equation coefficient");
      context_geometry_input[23]->value("1.0");
      for(i = 20; i < 24; i++) {
        context_geometry_input[i]->align(FL_ALIGN_RIGHT);
      }
      g[5]->end();
    }
    o->end();
  }

  {
    Fl_Button *o = new Fl_Button(width - BB - WB, height - BH - WB, BB, BH, "Cancel");
    o->callback(cancel_cb, (void *)context_geometry_window);
  }

  context_geometry_window->position(CTX.ctx_position[0], CTX.ctx_position[1]);
  context_geometry_window->end();
}

// Create the window for physical context dependant definitions

void GUI::call_for_solver_plugin (int dim)
{ 
   GMSH_Solve_Plugin *sp = GMSH_PluginManager::instance()->findSolverPlugin();   
   if (sp)
     {
       sp->popupPropertiesForPhysicalEntity(dim);
     }
}



// Create the window for mesh context dependant definitions

void GUI::create_mesh_context_window(int num)
{
  static Fl_Group *g[10];
  int i;

  if(context_mesh_window) {
    for(i = 0; i < 3; i++)
      g[i]->hide();
    g[num]->show();
    context_mesh_window->show();
    return;
  }

  int width = 31 * fontsize;
  int height = 5 * WB + 5 * BH;

  context_mesh_window = new Fl_Window(width, height, "Contextual mesh definitions");
  context_mesh_window->box(WINDOW_BOX);
  {
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width - 2 * WB, height - 3 * WB - BH);
    // 0: Characteristic length
    {
      g[0] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Characteristic length");
      context_mesh_input[0] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "Value");
      context_mesh_input[0]->value("1.0");
      context_mesh_input[0]->align(FL_ALIGN_RIGHT);
      g[0]->end();
    }
    // 1: Transfinite line
    {
      g[1] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Transfinite line");
      context_mesh_input[1] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "Number of points");
      context_mesh_input[1]->value("10");
      context_mesh_input[2] = new Fl_Input(2 * WB, 2 * WB + 3 * BH, IW, BH, "Parameter");
      context_mesh_input[2]->value("1.0");
      for(i = 1; i < 3; i++) {
        context_mesh_input[i]->align(FL_ALIGN_RIGHT);
      }
      static Fl_Menu_Item menu_trsf_mesh[] = {
        {"Progression", 0, 0, 0},
        {"Bump", 0, 0, 0},
        {0}
      };
      context_mesh_choice[0] = new Fl_Choice(2 * WB, 2 * WB + 2 * BH, IW, BH, "Type");
      context_mesh_choice[0]->menu(menu_trsf_mesh);
      context_mesh_choice[0]->align(FL_ALIGN_RIGHT);
      g[1]->end();
    }
    // 2: Transfinite volume
    {
      g[2] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - 3 * WB - 2 * BH, "Transfinite volume");
      context_mesh_input[3] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, IW, BH, "Volume number");
      context_mesh_input[3]->value("1");
      context_mesh_input[3]->align(FL_ALIGN_RIGHT);
      g[2]->end();
    }
    o->end();
  }

  {
    Fl_Button *o = new Fl_Button(width - BB - WB, height - BH - WB, BB, BH, "Cancel");
    o->callback(cancel_cb, (void *)context_mesh_window);
  }

  context_mesh_window->position(CTX.ctx_position[0], CTX.ctx_position[1]);
  context_mesh_window->end();
}


// Create the windows for the solvers

void GUI::create_solver_window(int num)
{
  int i, nbbutts = 0, newrow = 0;
  static Fl_Group *g[10];

  int LL = (int)(1.75 * IW);
  int BBS = (5 * fontsize + 1); // smaller width of a button with internal label

  if(solver[num].window) {
    solver[num].window->show();
    return;
  }

  for(i = 0; i < MAXSOLVERS; i++)
    if(strlen(SINFO[num].option_name[i]))
      SINFO[num].nboptions = i + 1;

  for(i = 0; i < MAXSOLVERS; i++)
    if(strlen(SINFO[num].button_name[i]))
      nbbutts++;
  if(nbbutts > 3)
    newrow = 1;

  int width = 5 * BBS + 6 * WB;
  int height = (8 + SINFO[num].nboptions + newrow) * WB + (6 + SINFO[num].nboptions + newrow) * BH;
  if(height < 7 * WB + 7 * BH)
    height = 7 * WB + 7 * BH;   //minimum height required by Options tab

  solver[num].window = new Fl_Window(width, height);
  solver[num].window->box(WINDOW_BOX);
  {
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width - 2 * WB, height - (3 + newrow) * WB - (1 + newrow) * BH);
    {
      g[0] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - (3 + newrow) * WB - (2 + newrow) * BH, "General");

      solver[num].input[0] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, LL, BH, "Problem");
      Fl_Button *b1 = new Fl_Button(2 * WB, 3 * WB + 2 * BH, BB, BH, "Choose");
      b1->callback(solver_file_open_cb, (void *)num);
      Fl_Button *b2 = new Fl_Button(3 * WB + BB, 3 * WB + 2 * BH, BB, BH, "Edit");
      b2->callback(solver_file_edit_cb, (void *)num);

      solver[num].input[1] = new Fl_Input(2 * WB, 4 * WB + 3 * BH, LL, BH, "Mesh");
      Fl_Button *b3 = new Fl_Button(2 * WB, 5 * WB + 4 * BH, BB, BH, "Choose");
      b3->callback(solver_choose_mesh_cb, (void *)num);

      for(i = 0; i < 2; i++) {
        solver[num].input[i]->align(FL_ALIGN_RIGHT);
      }

      for(i = 0; i < SINFO[num].nboptions; i++) {
        solver[num].choice[i] = new Fl_Choice(2 * WB, (6 + i) * WB + (5 + i) * BH, LL, BH, SINFO[num].option_name[i]);
        solver[num].choice[i]->align(FL_ALIGN_RIGHT);
      }

      g[0]->end();
    }
    {
      g[1] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - (3 + newrow) * WB - (2 + newrow) * BH, "Options");

      solver[num].input[2] = new Fl_Input(2 * WB, 2 * WB + 1 * BH, LL, BH, "Executable");
      solver[num].input[2]->align(FL_ALIGN_RIGHT);
      Fl_Button *b = new Fl_Button(2 * WB, 3 * WB + 2 * BH, BB, BH, "Choose");
      b->callback(solver_choose_executable_cb, (void *)num);

      solver[num].butt[2] = new Fl_Check_Button(2 * WB, 4 * WB + 3 * BH, LL, BH, "Enable client-server connection");
      solver[num].butt[0] = new Fl_Check_Button(2 * WB, 4 * WB + 4 * BH, LL, BH, "Display client messages");
      solver[num].butt[1] = new Fl_Check_Button(2 * WB, 4 * WB + 5 * BH, LL, BH, "Merge views automatically");
      for(i = 0; i < 3; i++) {
        solver[num].butt[i]->type(FL_TOGGLE_BUTTON);
        solver[num].butt[i]->down_box(TOGGLE_BOX);
        solver[num].butt[i]->selection_color(TOGGLE_COLOR);
      }

      {
        Fl_Return_Button *o = new Fl_Return_Button(width - 2 * BB - 3 * WB, 
						   height - (3 + newrow) * WB - (2 + newrow) * BH, 
						   BB, BH, "Apply");
        o->callback(solver_ok_cb, (void *)num);
      }
      {
        Fl_Button *o = new Fl_Button(width - BB - 2 * WB,
                                     height - (3 + newrow) * WB - (2 + newrow) * BH,
				     BB, BH, "Save");
        o->callback(options_save_cb);
      }
      g[1]->end();
    }
    {
      g[2] = new Fl_Group(WB, WB + BH, width - 2 * WB, height - (3 + newrow) * WB - (2 + newrow) * BH, "About");

      Fl_Browser *o = new Fl_Browser(2 * WB, 2 * WB + 1 * BH, width - 4 * WB,
                                     height - (5 + newrow) * WB - (2 + newrow) * BH);
      o->add(" ");
      add_multiline_in_browser(o, "@c@b@.", SINFO[num].name);
      o->add(" ");
      add_multiline_in_browser(o, "@c@. ", SINFO[num].help);

      g[2]->end();
    }
    o->end();
  }

  static int arg[MAXSOLVERS][5][2];
  int nb = 0;
  for(i = 4; i >= 0; i--) {
    if(strlen(SINFO[num].button_name[i])) {
      arg[num][i][0] = num;
      arg[num][i][1] = i;
      solver[num].command[nb] = new Fl_Button(width - (1 + nb + 2 * !newrow) * BBS - (1 + nb + 2 * !newrow) * WB,
					      height - (1 + newrow) * BH - (1 + newrow) * WB, BBS, BH,
					      SINFO[num].button_name[i]);
      solver[num].command[nb]->callback(solver_command_cb, (void *)arg[num][i]);
      nb++;
    }
  }

  {
    Fl_Button *o = new Fl_Button(width - 2 * BBS - 2 * WB, height - BH - WB, BBS, BH, "Kill");
    o->callback(solver_kill_cb, (void *)num);
  }
  {
    Fl_Button *o = new Fl_Button(width - BBS - WB, height - BH - WB, BBS, BH, "Cancel");
    o->callback(cancel_cb, (void *)solver[num].window);
  }

  solver[num].window->position(CTX.solver_position[0], CTX.solver_position[1]);
  solver[num].window->end();
}

