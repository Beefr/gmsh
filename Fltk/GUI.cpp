// $Id: GUI.cpp,v 1.117 2001-10-10 19:28:09 geuzaine Exp $

// To make the interface as visually consistent as possible, please:
// - use the BH, BW, WB, IW values for button heights/widths, window borders, etc.
// - use CTX.fontsize for font sizes
// - examine what's already done before adding something new...

#include "PluginManager.h"
#include "Plugin.h"

#include "Gmsh.h"
#include "GmshUI.h"
#include "Numeric.h"
#include "GmshVersion.h"
#include "Context.h"
#include "Options.h"
#include "Geo.h"
#include "Mesh.h"
#include "Draw.h"
#include "GUI.h"
#include "Callbacks.h"
#include "Bitmaps.h"
#include "Icon.h"
#include "OpenFile.h"
#include "GetOptions.h"

#define WINDOW_BOX FL_FLAT_BOX

#define IW  (10*CTX.fontsize) // input field width
#define BW  (3*IW/2) // width of a button with external label
#define BB  (5*CTX.fontsize-2) // width of a button with internal label
#define BH  (2*CTX.fontsize+1) // button height
#define WB  (5) // window border

extern Context_T  CTX;

//******************* Definition of the static menus ***********************************

Fl_Menu_Item m_menubar_table[] = {
  {"File", 0, 0, 0, FL_SUBMENU},
    {"Open...",          FL_CTRL+'o', (Fl_Callback *)file_open_cb, 0},
    {"Merge...",         FL_CTRL+'m', (Fl_Callback *)file_merge_cb, 0},
    {"Save as",          0, 0, 0, FL_MENU_DIVIDER|FL_SUBMENU},
      {"By extension...",        FL_CTRL+'p', (Fl_Callback *)file_save_as_auto_cb, 0, FL_MENU_DIVIDER},
      {"MSH native mesh format...",       0, (Fl_Callback *)file_save_as_msh_cb, 0},
      {"MSH all elements...",             0, (Fl_Callback *)file_save_as_msh_all_cb, 0},
      {"UNV universal mesh format...",    0, (Fl_Callback *)file_save_as_unv_cb, 0},
      {"GREF gref mesh format...",        0, (Fl_Callback *)file_save_as_gref_cb, 0},
      {"GEO flattened geometry...",       0, (Fl_Callback *)file_save_as_geo_cb, 0},
      {"GEO complete options...",         0, (Fl_Callback *)file_save_as_geo_options_cb, 0},
      {"EPS simple sort postscript...",   0, (Fl_Callback *)file_save_as_eps_simple_cb, 0},
      {"EPS accurate sort postscript...", 0, (Fl_Callback *)file_save_as_eps_accurate_cb, 0},
      {"JPEG...",                0, (Fl_Callback *)file_save_as_jpeg_cb, 0},
      {"GIF...",                 0, (Fl_Callback *)file_save_as_gif_cb, 0},
      {"GIF dithered...",        0, (Fl_Callback *)file_save_as_gif_dithered_cb, 0},
      {"GIF transparent...",     0, (Fl_Callback *)file_save_as_gif_transparent_cb, 0},
      {"PPM...",                 0, (Fl_Callback *)file_save_as_ppm_cb, 0},
      {"UCB YUV...",             0, (Fl_Callback *)file_save_as_yuv_cb, 0},
      {0},
    {"Messages...",      FL_SHIFT+'l', (Fl_Callback *)opt_message_cb, 0},
    {"Statistics...",    FL_SHIFT+'i', (Fl_Callback *)opt_statistics_cb, 0, FL_MENU_DIVIDER},
    {"Quit",             FL_CTRL+'q', (Fl_Callback *)file_quit_cb, 0},
    {0},
  {"Options",0,0,0,FL_SUBMENU},
    {"General...",         FL_SHIFT+'o', (Fl_Callback *)opt_general_cb, 0, FL_MENU_DIVIDER},
    {"Geometry...",        FL_SHIFT+'g', (Fl_Callback *)opt_geometry_cb, 0},
    {"Mesh...",            FL_SHIFT+'m', (Fl_Callback *)opt_mesh_cb, 0},
    {"Solver...",          FL_SHIFT+'s', (Fl_Callback *)opt_solver_cb, 0},
    {"Post-processing...", FL_SHIFT+'p', (Fl_Callback *)opt_post_cb, 0, FL_MENU_DIVIDER},
    {"Save options now",   0,            (Fl_Callback *)opt_save_cb, 0},
    {0},
  {"Help",0,0,0,FL_SUBMENU},
    {"Current options...",       0, (Fl_Callback *)status_xyz1p_cb, (void*)4},
    {"Shortcuts...",             0, (Fl_Callback *)help_short_cb, 0},
    {"Command line options...",  0, (Fl_Callback *)help_command_line_cb, 0, FL_MENU_DIVIDER},
    {"About...",                 0, (Fl_Callback *)help_about_cb, 0},
    {0},
  {0}
};

Fl_Menu_Item m_module_table[] = {
  {"Geometry",        'g', (Fl_Callback *)mod_geometry_cb, 0},
  {"Mesh",            'm', (Fl_Callback *)mod_mesh_cb, 0},
  {"Solver",          's', (Fl_Callback *)mod_solver_cb, 0},
  {"Post-processing", 'p', (Fl_Callback *)mod_post_cb, 0},
  {0}
};

//********************* Definition of the dynamic contexts *****************************

Context_Item menu_geometry[] = 
{ { "0Geometry", NULL } ,
  { "Elementary", (Fl_Callback *)geometry_elementary_cb } ,
  { "Physical",   (Fl_Callback *)geometry_physical_cb } ,
  { "Edit",       (Fl_Callback *)geometry_edit_cb } , 
  { "Reload",     (Fl_Callback *)geometry_reload_cb } , 
  { NULL }
};  
    Context_Item menu_geometry_elementary[] = 
    { { "0Geometry Elementary", NULL } ,
      { "Add",       (Fl_Callback *)geometry_elementary_add_cb } ,
      { "Translate", (Fl_Callback *)geometry_elementary_translate_cb } ,
      { "Rotate",    (Fl_Callback *)geometry_elementary_rotate_cb } ,
      { "Scale",     (Fl_Callback *)geometry_elementary_scale_cb } ,
      { "Symmetry",  (Fl_Callback *)geometry_elementary_symmetry_cb } ,
      { "Extrude",   (Fl_Callback *)geometry_elementary_extrude_cb } ,
      { "Delete",    (Fl_Callback *)geometry_elementary_delete_cb } ,
      { NULL } 
    };  
        Context_Item menu_geometry_elementary_add[] = 
	{ { "0Geometry Elementary Add", NULL } ,
          { "New",       (Fl_Callback *)geometry_elementary_add_new_cb } ,
	  { "Translate", (Fl_Callback *)geometry_elementary_add_translate_cb } ,
	  { "Rotate",    (Fl_Callback *)geometry_elementary_add_rotate_cb } ,
	  { "Scale",     (Fl_Callback *)geometry_elementary_add_scale_cb } ,
	  { "Symmetry",  (Fl_Callback *)geometry_elementary_add_symmetry_cb } ,
	  { NULL } 
	};  
            Context_Item menu_geometry_elementary_add_new[] = 
	    { { "0Geometry Elementary Add New", NULL } ,
              { "Parameter",     (Fl_Callback *)geometry_elementary_add_new_parameter_cb } ,
	      { "Point",         (Fl_Callback *)geometry_elementary_add_new_point_cb } ,
	      { "Line",          (Fl_Callback *)geometry_elementary_add_new_line_cb } ,
	      { "Spline",        (Fl_Callback *)geometry_elementary_add_new_spline_cb } ,
	      { "B-Spline",      (Fl_Callback *)geometry_elementary_add_new_bspline_cb } ,
	      { "Circle",        (Fl_Callback *)geometry_elementary_add_new_circle_cb } ,
	      { "Ellipsis",      (Fl_Callback *)geometry_elementary_add_new_ellipsis_cb } ,
	      { "Plane surface", (Fl_Callback *)geometry_elementary_add_new_planesurface_cb } ,
	      { "Ruled surface", (Fl_Callback *)geometry_elementary_add_new_ruledsurface_cb } ,
	      { "Volume",        (Fl_Callback *)geometry_elementary_add_new_volume_cb } ,
	      { NULL } 
	    };  
            Context_Item menu_geometry_elementary_add_translate[] = 
	    { { "0Geometry Elementary Add Translate", NULL } ,
              { "Point",   (Fl_Callback *)geometry_elementary_add_translate_point_cb } ,
	      { "Curve",   (Fl_Callback *)geometry_elementary_add_translate_curve_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_add_translate_surface_cb } ,
	      { NULL } 
	    };  
            Context_Item menu_geometry_elementary_add_rotate[] = 
	    { { "0Geometry Elementary Add Rotate", NULL } ,
              { "Point",   (Fl_Callback *)geometry_elementary_add_rotate_point_cb } ,
	      { "Curve",   (Fl_Callback *)geometry_elementary_add_rotate_curve_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_add_rotate_surface_cb } ,
	      { NULL } 
	    };  
            Context_Item menu_geometry_elementary_add_scale[] = 
	    { { "0Geometry Elementary Add Scale", NULL } ,
	      { "Point",   (Fl_Callback *)geometry_elementary_add_scale_point_cb } ,
	      { "Curve",   (Fl_Callback *)geometry_elementary_add_scale_curve_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_add_scale_surface_cb } ,
	      { NULL } 
	    };  
            Context_Item menu_geometry_elementary_add_symmetry[] = 
	    { { "0Geometry Elementary Add Symmetry", NULL } ,
	      { "Point",   (Fl_Callback *)geometry_elementary_add_symmetry_point_cb } ,
	      { "Curve",   (Fl_Callback *)geometry_elementary_add_symmetry_curve_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_add_symmetry_surface_cb } ,
	      { NULL } 
	    };  
        Context_Item menu_geometry_elementary_translate[] = 
	{ { "0Geometry Elementary Translate", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_elementary_translate_point_cb } ,
	  { "Curve",   (Fl_Callback *)geometry_elementary_translate_curve_cb } ,
	  { "Surface", (Fl_Callback *)geometry_elementary_translate_surface_cb } ,
	  { NULL } 
	};  
        Context_Item menu_geometry_elementary_rotate[] = 
	{ { "0Geometry Elementary Rotate", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_elementary_rotate_point_cb } ,
	  { "Curve",   (Fl_Callback *)geometry_elementary_rotate_curve_cb } ,
	  { "Surface", (Fl_Callback *)geometry_elementary_rotate_surface_cb } ,
	  { NULL } 
	};  
        Context_Item menu_geometry_elementary_scale[] = 
	{ { "0Geometry Elementary Scale", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_elementary_scale_point_cb } ,
	  { "Curve",   (Fl_Callback *)geometry_elementary_scale_curve_cb } ,
	  { "Surface", (Fl_Callback *)geometry_elementary_scale_surface_cb } ,
	  { NULL } 
	};  
        Context_Item menu_geometry_elementary_symmetry[] = 
	{ { "0Geometry Elementary Symmetry", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_elementary_symmetry_point_cb } ,
	  { "Curve",   (Fl_Callback *)geometry_elementary_symmetry_curve_cb } ,
	  { "Surface", (Fl_Callback *)geometry_elementary_symmetry_surface_cb } ,
	  { NULL } 
	};  
        Context_Item menu_geometry_elementary_extrude[] = 
	{ { "0Geometry Elementary Extrude", NULL } ,
	  { "Translate",   (Fl_Callback *)geometry_elementary_extrude_translate_cb } ,
	  { "Rotate",   (Fl_Callback *)geometry_elementary_extrude_rotate_cb } ,
	  { NULL } 
 	};  
            Context_Item menu_geometry_elementary_extrude_translate[] = 
	    { { "0Geometry Elementary Extrude Translate", NULL } ,
	      { "Point",   (Fl_Callback *)geometry_elementary_extrude_translate_point_cb } ,
	      { "Curve",   (Fl_Callback *)geometry_elementary_extrude_translate_curve_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_extrude_translate_surface_cb } ,
	      { NULL } 
	    };  
            Context_Item menu_geometry_elementary_extrude_rotate[] = 
	    { { "0Geometry Elementary Extrude Rotate", NULL } ,
	      { "Point",   (Fl_Callback *)geometry_elementary_extrude_rotate_point_cb } ,
	      { "Curve",   (Fl_Callback *)geometry_elementary_extrude_rotate_curve_cb } ,
	      { "Surface", (Fl_Callback *)geometry_elementary_extrude_rotate_surface_cb } ,
	      { NULL } 
	    };  
        Context_Item menu_geometry_elementary_delete[] = 
	{ { "0Geometry Elementary Delete", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_elementary_delete_point_cb } ,
	  { "Curve",   (Fl_Callback *)geometry_elementary_delete_curve_cb } ,
	  { "Surface", (Fl_Callback *)geometry_elementary_delete_surface_cb } ,
	  { NULL } 
	};  
    Context_Item menu_geometry_physical[] = 
    { { "0Geometry Physical", NULL } ,
      { "Add",    (Fl_Callback *)geometry_physical_add_cb } ,
      { NULL } 
    };  
        Context_Item menu_geometry_physical_add[] = 
	{ { "0Geometry Physical Add", NULL } ,
	  { "Point",   (Fl_Callback *)geometry_physical_add_point_cb  } ,
	  { "Curve",   (Fl_Callback *)geometry_physical_add_curve_cb  } ,
	  { "Surface", (Fl_Callback *)geometry_physical_add_surface_cb  } ,
	  { "Volume",  (Fl_Callback *)geometry_physical_add_volume_cb  } ,
	  { NULL } 
	};  

Context_Item menu_mesh[] = 
{ { "1Mesh", NULL } ,
  { "Define", (Fl_Callback *)mesh_define_cb } ,
  { "1D",     (Fl_Callback *)mesh_1d_cb } ,
  { "2D",     (Fl_Callback *)mesh_2d_cb } , 
  { "3D",     (Fl_Callback *)mesh_3d_cb } , 
  { "Save",   (Fl_Callback *)mesh_save_cb } ,
  { "Save all", (Fl_Callback *)mesh_save_all_cb } ,
  { NULL } 
};  
    Context_Item menu_mesh_define[] = 
    { { "1Mesh Define", NULL } ,
      { "Length",      (Fl_Callback *)mesh_define_length_cb  } ,
      { "Recombine",   (Fl_Callback *)mesh_define_recombine_cb  } ,
      { "Transfinite", (Fl_Callback *)mesh_define_transfinite_cb  } , 
      { NULL } 
    };  
        Context_Item menu_mesh_define_transfinite[] = 
	{ { "1Mesh Define Transfinite", NULL } ,
	  { "Line",    (Fl_Callback *)mesh_define_transfinite_line_cb } ,
	  { "Surface", (Fl_Callback *)mesh_define_transfinite_surface_cb } ,
	  { "Volume",  (Fl_Callback *)mesh_define_transfinite_volume_cb } , 
	  { NULL } 
	};  

Context_Item menu_solver[] = 
{ { "2Solver", NULL } ,
  { "GetDP", (Fl_Callback *)getdp_cb } ,
  { NULL } };

Context_Item menu_post[] = 
{ { "3Post-processing", NULL } ,
  { NULL } };

//********************** Definition of global shortcuts ********************************

int GUI::global_shortcuts(int event){
  int i, j;

  // we only handle shortcuts here
  if(event != FL_SHORTCUT) return 0 ;


  if(Fl::test_shortcut(FL_SHIFT+FL_Escape) ||
     Fl::test_shortcut(FL_CTRL+FL_Escape) ||
     Fl::test_shortcut(FL_ALT+FL_Escape)){
    return 1;
  }
  if(Fl::test_shortcut('0') || Fl::test_shortcut(FL_Escape)){
    geometry_reload_cb(0,0);
    mod_geometry_cb(0,0);
    return 1;
  }
  else if(Fl::test_shortcut('1') || Fl::test_shortcut(FL_F+1)){
    mesh_1d_cb(0,0);
    mod_mesh_cb(0,0);
    return 1;
  }
  else if(Fl::test_shortcut('2') || Fl::test_shortcut(FL_F+2)){
    mesh_2d_cb(0,0);
    mod_mesh_cb(0,0);
    return 1;
  }
  else if(Fl::test_shortcut('3') || Fl::test_shortcut(FL_F+3)){
    mesh_3d_cb(0,0);
    mod_mesh_cb(0,0);
    return 1;
  }
  else if(Fl::test_shortcut('g')){
    mod_geometry_cb(0,0);
    return 1;
  }
  else if(Fl::test_shortcut('m')){
    mod_mesh_cb(0,0);
    return 1;
  }
  else if(Fl::test_shortcut('s')){
    mod_solver_cb(0,0);
    return 1;
  }
  else if(Fl::test_shortcut('p')){
    mod_post_cb(0,0);
    return 1;
  }
  else if(Fl::test_shortcut('b')){
    mod_back_cb(0,0);
    return 1;
  }
  else if(Fl::test_shortcut('f')){
    mod_forward_cb(0,0);
    return 1;
  }
  else if(Fl::test_shortcut('e')){
    end_selection = 1;
    return 1;
  }
  else if(Fl::test_shortcut('q')){
    quit_selection = 1;
    return 1;
  }
  else if(Fl::test_shortcut(FL_CTRL+'s')){
    mesh_save_cb(0,0);
    return 1;
  }
  else if(Fl::test_shortcut(FL_CTRL+FL_SHIFT+'d')){
    opt_post_anim_delay(0,GMSH_SET|GMSH_GUI,opt_post_anim_delay(0,GMSH_GET,0) + 0.01);
    return 1;
  }
  else if(Fl::test_shortcut(FL_SHIFT+'d')){
    opt_post_anim_delay(0,GMSH_SET|GMSH_GUI,opt_post_anim_delay(0,GMSH_GET,0) - 0.01);
    return 1;
  }
  else if(Fl::test_shortcut(FL_CTRL+'z')){
    g_window->iconize() ;
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'f')){
    opt_general_fast_redraw(0,GMSH_SET|GMSH_GUI,!opt_general_fast_redraw(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'b')){
    opt_post_scales(0,GMSH_SET|GMSH_GUI,!opt_post_scales(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'o')){
    opt_general_orthographic(0,GMSH_SET|GMSH_GUI,!opt_general_orthographic(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'c')){
    opt_general_color_scheme(0,GMSH_SET|GMSH_GUI,opt_general_color_scheme(0,GMSH_GET,0)+1);
    opt_geometry_color_scheme(0,GMSH_SET|GMSH_GUI,opt_geometry_color_scheme(0,GMSH_GET,0)+1);
    opt_mesh_color_scheme(0,GMSH_SET|GMSH_GUI,opt_mesh_color_scheme(0,GMSH_GET,0)+1);
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'d')){
    opt_mesh_aspect(0,GMSH_SET|GMSH_GUI,opt_mesh_aspect(0,GMSH_GET,0)+1);
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'x')){
    status_xyz1p_cb(0,(void*)0);
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'y')){
    status_xyz1p_cb(0,(void*)1);
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'z')){
    status_xyz1p_cb(0,(void*)2);
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+FL_SHIFT+'a')){
    opt_general_axes(0,GMSH_SET|GMSH_GUI,!opt_general_axes(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'a')){
    opt_general_small_axes(0,GMSH_SET|GMSH_GUI,!opt_general_small_axes(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'p')){
    opt_geometry_points(0,GMSH_SET|GMSH_GUI,!opt_geometry_points(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'l')){
    opt_geometry_lines(0,GMSH_SET|GMSH_GUI,!opt_geometry_lines(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'s')){
    opt_geometry_surfaces(0,GMSH_SET|GMSH_GUI,!opt_geometry_surfaces(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'v')){
    opt_geometry_volumes(0,GMSH_SET|GMSH_GUI,!opt_geometry_volumes(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+FL_SHIFT+'p')){
    opt_mesh_points(0,GMSH_SET|GMSH_GUI,!opt_mesh_points(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+FL_SHIFT+'l')){
    opt_mesh_lines(0,GMSH_SET|GMSH_GUI,!opt_mesh_lines(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+FL_SHIFT+'s')){
    opt_mesh_surfaces(0,GMSH_SET|GMSH_GUI,!opt_mesh_surfaces(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+FL_SHIFT+'v')){
    opt_mesh_volumes(0,GMSH_SET|GMSH_GUI,!opt_mesh_volumes(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'m')){
    opt_mesh_points(0,GMSH_SET|GMSH_GUI,!opt_mesh_points(0,GMSH_GET,0));
    opt_mesh_lines(0,GMSH_SET|GMSH_GUI,!opt_mesh_lines(0,GMSH_GET,0));
    opt_mesh_surfaces(0,GMSH_SET|GMSH_GUI,!opt_mesh_surfaces(0,GMSH_GET,0));
    opt_mesh_volumes(0,GMSH_SET|GMSH_GUI,!opt_mesh_volumes(0,GMSH_GET,0));
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'t')){
    for(i=0 ; i<List_Nbr(Post_ViewList) ; i++){
      if(opt_view_visible(i,GMSH_GET,0)){
	j = (int)opt_view_intervals_type(i, GMSH_GET, 0);
	opt_view_intervals_type(i, GMSH_SET|GMSH_GUI, 
				(j == DRAW_POST_ISO) ? DRAW_POST_DISCRETE :
				(j == DRAW_POST_DISCRETE) ? DRAW_POST_CONTINUOUS :
				DRAW_POST_ISO);
      }
    }
    redraw_opengl();
    return 1;
  }
  else if(Fl::test_shortcut(FL_ALT+'h')){
    static int show = 0;
    for(i=0 ; i<List_Nbr(Post_ViewList) ; i++)
      opt_view_visible(i, GMSH_SET|GMSH_GUI, show);
    redraw_opengl();
    show = !show;
    return 1;
  }
  

  return 0;
}


//***************************** The GUI constructor ************************************

GUI::GUI(int argc, char **argv) {
  
  init_menu_window = 0;
  init_graphic_window = 0;
  init_general_options_window = 0;
  init_geometry_options_window = 0;
  init_mesh_options_window = 0;
  init_solver_options_window = 0;
  init_post_options_window = 0;
  init_statistics_window = 0;
  init_message_window = 0;
  init_about_window = 0;
  init_view_window = 0;
  init_geometry_context_window = 0;
  init_mesh_context_window = 0;
  init_getdp_window = 0;

  if(strlen(CTX.display)) Fl::display(CTX.display);

  Fl::add_handler(SetGlobalShortcut);

  // All static windows are contructed (even if some are not
  // displayed) since the shortcuts should be valid even for hidden
  // windows, and we don't want to test for widget existence every time

  create_menu_window(argc, argv);
  create_graphic_window(argc, argv);

#ifdef WIN32
  m_window->icon((char *)LoadImage(fl_display, MAKEINTRESOURCE(IDI_ICON),
  				   IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
#else
  fl_open_display();
  Pixmap p1 = XCreateBitmapFromData(fl_display, DefaultRootWindow(fl_display),
				    g1_bits, g1_width, g1_height);
  Pixmap p2 = XCreateBitmapFromData(fl_display, DefaultRootWindow(fl_display),
				    g2_bits, g2_width, g2_height);
  m_window->icon((char *)p1); 
  g_window->icon((char *)p2);
#endif

  m_window->show(1, argv);
  g_window->show(1, argv);

  create_general_options_window();
  create_geometry_options_window();
  create_mesh_options_window();
  create_solver_options_window();
  create_post_options_window();
  create_view_options_window(-1);
  create_message_window();
  create_about_window();
  create_getdp_window();

  // Draw the scene
  g_opengl_window->redraw();

}

// Run the GUI until no window is left

void GUI::run(){
  Fl::run();
}

// Check (now) if any pending events and run them

void GUI::check(){
  Fl::check();
}

// Wait for any events and run them

void GUI::wait(){
  Fl::wait();
}

//********************************* Create the menu window *****************************


void GUI::add_post_plugins (Fl_Menu_Button *button , int iView){
  char name[256],menuname[256];
  for(GMSH_PluginManager::iter it = GMSH_PluginManager::Instance()->begin();
      it != GMSH_PluginManager::Instance()->end();
      ++it){
    GMSH_Plugin *p = (*it).second;
    if(p->getType() == GMSH_Plugin::GMSH_POST_PLUGIN){
      p->getName(name);
      std::pair<int,GMSH_Plugin*> *pair = new std::pair<int,GMSH_Plugin*>(iView,p);
      sprintf(menuname,"Plugins/%s...",name);
      button->add(menuname, 0,(Fl_Callback *)view_options_plugin_cb, (void*)(pair), 0);
      p->dialogBox = 0;
    }
  }
}

void GUI::create_menu_window(int argc, char **argv){
  int i, y;
  
  if(!init_menu_window){
    init_menu_window = 1 ;
    
    int width = 13*CTX.fontsize-CTX.fontsize/2-2 ;
    MH = BH + BH+6 ; // this is the initial height: no dynamic button is shown!
    
    m_window = new Fl_Window(width,MH);
    m_window->box(WINDOW_BOX);
    m_window->label("Gmsh");
    m_window->callback(file_quit_cb);
    
    m_menu_bar = new Fl_Menu_Bar(0,0,width,BH); 
    m_menu_bar->menu(m_menubar_table);
    m_menu_bar->textsize(CTX.fontsize);
    m_menu_bar->box(FL_UP_BOX);
    m_menu_bar->global();
    
    Fl_Box *o = new Fl_Box(0,BH,width,BH+6);
    o->box(FL_UP_BOX);
    
    y = BH + 3;
    
    m_navig_butt[0] = new Fl_Button(1,y,18,BH/2,"@<");
    m_navig_butt[0]->labeltype(FL_SYMBOL_LABEL);
    m_navig_butt[0]->labelsize(11);
    m_navig_butt[0]->box(FL_FLAT_BOX);
    m_navig_butt[0]->selection_color(FL_WHITE);
    m_navig_butt[0]->callback(mod_back_cb);

    m_navig_butt[1] = new Fl_Button(1,y+BH/2,18,BH/2,"@>");
    m_navig_butt[1]->labeltype(FL_SYMBOL_LABEL);
    m_navig_butt[1]->labelsize(11);
    m_navig_butt[1]->box(FL_FLAT_BOX);
    m_navig_butt[1]->selection_color(FL_WHITE);
    m_navig_butt[1]->callback(mod_forward_cb);
    
    m_module_butt = new Fl_Choice(19,y,width-24,BH);
    m_module_butt->menu(m_module_table);
    m_module_butt->textsize(CTX.fontsize);
    m_module_butt->box(FL_THIN_DOWN_BOX);
    
    y = MH ;
    
    for(i=0; i<NB_BUTT_MAX; i++){
      m_push_butt[i] = new Fl_Button(0,y+i*BH,width,BH); 
      m_push_butt[i]->labelsize(CTX.fontsize);
      m_push_butt[i]->hide();
      m_toggle_butt[i] = new Fl_Light_Button(0,y+i*BH,width,BH); 
      m_toggle_butt[i]->labelsize(CTX.fontsize); 
      m_toggle_butt[i]->callback(view_toggle_cb, (void*)i);
      m_toggle_butt[i]->hide();
      m_popup_butt[i] = new Fl_Menu_Button(0,y+i*BH,width,BH);
      m_popup_butt[i]->type(Fl_Menu_Button::POPUP3);
      m_popup_butt[i]->add("Reload/View", 0, 
			   (Fl_Callback *)view_reload_cb, (void*)i, 0);
      m_popup_butt[i]->add("Reload/All views", 0, 
			   (Fl_Callback *)view_reload_all_cb, (void*)i, 0);
      m_popup_butt[i]->add("Reload/All visible views", 0, 
			   (Fl_Callback *)view_reload_visible_cb, (void*)i, 0);
      m_popup_butt[i]->add("Remove/View", 0, 
			   (Fl_Callback *)view_remove_cb, (void*)i, 0);
      m_popup_butt[i]->add("Remove/All views", 0, 
			   (Fl_Callback *)view_remove_all_cb, (void*)i, 0);
      m_popup_butt[i]->add("Remove/All visible views", 0, 
			   (Fl_Callback *)view_remove_visible_cb, (void*)i, 0);
      m_popup_butt[i]->add("Duplicate/View without options", 0,
			   (Fl_Callback *)view_duplicate_cb, (void*)i, 0) ;
      m_popup_butt[i]->add("Duplicate/View with options", 0,
			   (Fl_Callback *)view_duplicate_with_options_cb, (void*)i, 0) ;
      m_popup_butt[i]->add("Save as/ASCII view...", 0,
			   (Fl_Callback *)view_save_ascii_cb, (void*)i, 0) ;
      m_popup_butt[i]->add("Save as/Binary view...", 0,
			   (Fl_Callback *)view_save_binary_cb, (void*)i, 0) ;
      add_post_plugins ( m_popup_butt[i] , i);
      m_popup_butt[i]->add("Apply as background mesh", 0,
			   (Fl_Callback *)view_applybgmesh_cb, (void*)i, FL_MENU_DIVIDER);
      m_popup_butt[i]->add("Options...", 0,
			   (Fl_Callback *)view_options_cb, (void*)i, 0);
      m_popup_butt[i]->textsize(CTX.fontsize);
      m_popup_butt[i]->hide();
    }
    
    m_window->position(CTX.position[0],CTX.position[1]);
    m_window->end();
  }
  else{
    if(m_window->shown())
      m_window->redraw();
    else
      m_window->show(1, argv);
    
  }
  
}

// Dynamically set the height of the menu window

void GUI::set_menu_size(int nb_butt){
  m_window->size(m_window->w(), MH + nb_butt*BH);
}

// Dynamically set the context

void GUI::set_context(Context_Item *menu_asked, int flag){
  static int nb_back = 0, nb_forward = 0, init_context=0;
  static Context_Item *menu_history[NB_HISTORY_MAX];
  Context_Item *menu;
  Post_View *v;
  int i;
  
  if(!init_context){
    init_context = 1;
    for(i=0 ; i<NB_HISTORY_MAX ; i++){
      menu_history[i] = NULL ;
    }
  }
  
  if(nb_back > NB_HISTORY_MAX-2) nb_back = 1; // we should do a circular list
  
  if(flag == -1){
    if(nb_back > 1){
      nb_back--;
      nb_forward++;
      menu = menu_history[nb_back-1];
    }
    else return;
  }
  else if(flag == 1){
    if(nb_forward > 0){
      nb_back++;
      nb_forward--;
      menu = menu_history[nb_back-1];
    }
    else return;
  }
  else{
    menu = menu_asked;
    if(!nb_back || menu_history[nb_back-1] != menu){
      menu_history[nb_back++] = menu;
    }
    nb_forward = 0;
  }
  
  int nb = 0;
  
  if(menu[0].label[0] == '0')      m_module_butt->value(0);
  else if(menu[0].label[0] == '1') m_module_butt->value(1);
  else if(menu[0].label[0] == '2') m_module_butt->value(2);
  else if(menu[0].label[0] == '3') m_module_butt->value(3);
  else {
    Msg(WARNING, "Something is wrong in your dynamic context definition");
    return;
  }
  
  Msg(STATUS2, menu[0].label+1);
  
  switch(m_module_butt->value()){
  case 3 : // post-processing contexts
    for(i = 0 ; i < List_Nbr(Post_ViewList) ; i++) {
      if(i == NB_BUTT_MAX) break;
      nb++ ;
      v = (Post_View*)List_Pointer(Post_ViewList,i);
      m_push_butt[i]->hide();
      m_toggle_butt[i]->show();
      m_toggle_butt[i]->value(v->Visible);
      m_toggle_butt[i]->label(v->Name);
      m_popup_butt[i]->show();
    }
    for(i = List_Nbr(Post_ViewList) ; i < NB_BUTT_MAX ; i++) {
      m_push_butt[i]->hide();
      m_toggle_butt[i]->hide();
      m_popup_butt[i]->hide();
    }
    break;
  default : // geometry, mesh, solver contexts
    for(i=0 ; i < NB_BUTT_MAX ; i++){
      m_toggle_butt[i]->hide();
      m_popup_butt[i]->hide();
      if(menu[i+1].label){
	m_push_butt[i]->label(menu[i+1].label);
	m_push_butt[i]->callback(menu[i+1].callback);
	m_push_butt[i]->redraw();
	m_push_butt[i]->show();
	nb++;
      }
      else
	break;
    }
    for(i=nb ; i<NB_BUTT_MAX ; i++){
      m_toggle_butt[i]->hide();
      m_popup_butt[i]->hide();
      m_push_butt[i]->hide();
    }
    break ;
  }
  
  set_menu_size(nb);
  
}

int GUI::get_context(){
  return m_module_butt->value();
}


//******************************** Create the graphic window ***************************

void GUI::create_graphic_window(int argc, char **argv){
  int i, x;
  
  if(!init_graphic_window){
    init_graphic_window = 1 ;
    
    int sh = 2*CTX.fontsize-4; // status bar height
    int sw = CTX.fontsize+4; //status button width
    int width = CTX.viewport[2]-CTX.viewport[0];
    int glheight = CTX.viewport[3]-CTX.viewport[1];
    int height = glheight + sh;
    
    g_window = new Fl_Window(width, height);
    g_window->callback(file_quit_cb);
    
    g_opengl_window = new Opengl_Window(0,0,width,glheight);
    if(!opt_general_double_buffer(0,GMSH_GET,0)){
      Msg(INFO, "Setting OpenGL visual to single buffered");
      g_opengl_window->mode(FL_RGB | FL_DEPTH | FL_SINGLE);
    }
    g_opengl_window->end();
    
    {
      Fl_Group *o = new Fl_Group(0,glheight,width,sh);
      o->box(FL_THIN_UP_BOX);
      
      x = 2;
      
      g_status_butt[0] = new Fl_Button(x,glheight+2,sw,sh-4,"X"); x+=sw;
      g_status_butt[0]->callback(status_xyz1p_cb, (void*)0);
      //g_status_butt[0]->tooltip("Set X view");
      g_status_butt[1] = new Fl_Button(x,glheight+2,sw,sh-4,"Y"); x+=sw;
      g_status_butt[1]->callback(status_xyz1p_cb, (void*)1);
      g_status_butt[2] = new Fl_Button(x,glheight+2,sw,sh-4,"Z"); x+=sw;
      g_status_butt[2]->callback(status_xyz1p_cb, (void*)2);
      g_status_butt[3] = new Fl_Button(x,glheight+2,2*CTX.fontsize,sh-4,"1:1"); x+=2*CTX.fontsize;
      g_status_butt[3]->callback(status_xyz1p_cb, (void*)3);
      g_status_butt[4] = new Fl_Button(x,glheight+2,sw,sh-4,"?"); x+=sw;
      g_status_butt[4]->callback(status_xyz1p_cb, (void*)4);
      g_status_butt[5] = new Fl_Button(x,glheight+2,sw,sh-4); x+=sw;
      g_status_butt[5]->callback(status_play_cb);
      start_bmp = new Fl_Bitmap(start_bits,start_width,start_height);
      start_bmp->label(g_status_butt[5]);
      stop_bmp = new Fl_Bitmap(stop_bits,stop_width,stop_height);
      g_status_butt[5]->deactivate();
      g_status_butt[6] = new Fl_Button(x,glheight+2,sw,sh-4); x+=sw;
      g_status_butt[6]->callback(status_cancel_cb);
      abort_bmp = new Fl_Bitmap(abort_bits,abort_width,abort_height);
      abort_bmp->label(g_status_butt[6]);
      g_status_butt[6]->deactivate();
      for(i = 0 ; i<7 ; i++){
	g_status_butt[i]->box(FL_FLAT_BOX);
	g_status_butt[i]->selection_color(FL_WHITE);
	g_status_butt[i]->labelsize(CTX.fontsize);
	g_status_butt[i]->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
      }
      
      g_status_label[0] = new Fl_Box(x,glheight+2,(width-x)/3,sh-4);
      g_status_label[1] = new Fl_Box(x+(width-x)/3,glheight+2,(width-x)/3,sh-4);
      g_status_label[2] = new Fl_Box(x+2*(width-x)/3,glheight+2,(width-x)/3-2,sh-4);
      for(i = 0 ; i<3 ; i++){
	g_status_label[i]->box(FL_FLAT_BOX);
	g_status_label[i]->labelsize(CTX.fontsize);
	g_status_label[i]->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE|FL_ALIGN_CLIP);
      }
      
      o->end();
    }
    
    g_window->resizable(g_opengl_window);
    g_window->position(CTX.gl_position[0],CTX.gl_position[1]);
    g_window->end();   
  }
  else{
    if(g_window->shown())
      g_window->redraw();
    else
      g_window->show(1, argv);
    
  }
}

// Set the size of the graphical window

void GUI::set_size(int new_w, int new_h){
  g_window->size(new_w,new_h+g_window->h()-g_opengl_window->h());
}

// Set graphic window title

void GUI::set_title(char *str){
  g_window->label(str);
}

// Set animation button

void GUI::set_anim(int mode){
  if(mode){
    g_status_butt[5]->callback(status_play_cb);
    start_bmp->label(g_status_butt[5]);
  }
  else{
    g_status_butt[5]->callback(status_pause_cb);
    stop_bmp->label(g_status_butt[5]);
  }
}

// Set the status messages

void GUI::set_status(char *msg, int num){
  g_status_label[num]->label(msg);
  g_status_label[num]->redraw();
}

// set the current drawing context 

void GUI::make_opengl_current(){
  g_opengl_window->make_current();
}

void GUI::make_overlay_current(){
  g_opengl_window->make_overlay_current();
}

// Draw the opengl window

void GUI::redraw_opengl(){
  g_opengl_window->redraw();
}

// Draw the opengl overlay window

void GUI::redraw_overlay(){
  g_opengl_window->redraw_overlay();
}

//************************ Create the window for general options ***********************

void GUI::create_general_options_window(){
  int i;
  
  if(!init_general_options_window){
    init_general_options_window = 1 ;
    
    int width = 25*CTX.fontsize;
    int height = 5*WB+11*BH ;
    
    gen_window = new Fl_Window(width,height);
    gen_window->box(WINDOW_BOX);
    gen_window->label("General options");
    { 
      Fl_Tabs* o = new Fl_Tabs(WB, WB, width-2*WB, height-3*WB-BH);
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Display");
	o->labelsize(CTX.fontsize);
	gen_butt[0] = new Fl_Check_Button(2*WB, 2*WB+1*BH, BW, BH, "Show moving axes");
	gen_butt[1] = new Fl_Check_Button(2*WB, 2*WB+2*BH, BW, BH, "Show small axes");
	gen_butt[2] = new Fl_Check_Button(2*WB, 2*WB+3*BH, BW, BH, "Enable fast redraw");
	gen_butt[3] = new Fl_Check_Button(2*WB, 2*WB+4*BH, BW, BH, "Enable double buffering");
	gen_butt[4] = new Fl_Check_Button(2*WB, 2*WB+5*BH, BW, BH, "Use display lists");
	gen_butt[5] = new Fl_Check_Button(2*WB, 2*WB+6*BH, BW, BH, "Enable alpha blending");
	gen_butt[6] = new Fl_Check_Button(2*WB, 2*WB+7*BH, BW, BH, "Use trackball rotation mode");
	for(i=0 ; i<7 ; i++){
	  gen_butt[i]->type(FL_TOGGLE_BUTTON);
	  gen_butt[i]->down_box(FL_DOWN_BOX);
	  gen_butt[i]->labelsize(CTX.fontsize);
	  gen_butt[i]->selection_color(FL_YELLOW);
	}
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Output");
	o->labelsize(CTX.fontsize);
	gen_butt[7] = new Fl_Check_Button(2*WB, 2*WB+1*BH, BW, BH, "Print messages on terminal");
	gen_butt[8] = new Fl_Check_Button(2*WB, 2*WB+2*BH, BW, BH, "Save session information on exit");
	gen_butt[9] = new Fl_Check_Button(2*WB, 2*WB+3*BH, BW, BH, "Save options on exit");
	for(i=7 ; i<10 ; i++){
	  gen_butt[i]->type(FL_TOGGLE_BUTTON);
	  gen_butt[i]->down_box(FL_DOWN_BOX);
	  gen_butt[i]->labelsize(CTX.fontsize);
	  gen_butt[i]->selection_color(FL_YELLOW);
	}
	gen_value[5] = new Fl_Value_Input(2*WB, 2*WB+4*BH, IW, BH, "Message verbosity");
	gen_value[5]->minimum(0); 
	gen_value[5]->maximum(10); 
	gen_value[5]->step(1);
	gen_value[5]->labelsize(CTX.fontsize);
	gen_value[5]->textsize(CTX.fontsize);
	gen_value[5]->type(FL_HORIZONTAL);
	gen_value[5]->align(FL_ALIGN_RIGHT);
	gen_input[0] = new Fl_Input(2*WB, 2*WB+5*BH, IW, BH, "Default file name");
	gen_input[1] = new Fl_Input(2*WB, 2*WB+6*BH, IW, BH, "Temporary file");
	gen_input[2] = new Fl_Input(2*WB, 2*WB+7*BH, IW, BH, "Error file");
	gen_input[3] = new Fl_Input(2*WB, 2*WB+8*BH, IW, BH, "Option file");
	gen_input[4] = new Fl_Input(2*WB, 2*WB+9*BH, IW, BH, "Text editor command");
	for(i=0 ; i<5 ; i++){
	  gen_input[i]->labelsize(CTX.fontsize);
	  gen_input[i]->textsize(CTX.fontsize);
	  gen_input[i]->align(FL_ALIGN_RIGHT);
	}
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Projection");
	o->labelsize(CTX.fontsize);
	o->hide();
	gen_butt[10] = new Fl_Check_Button(2*WB, 2*WB+1*BH, BW, BH, "Orthographic");
	gen_butt[11] = new Fl_Check_Button(2*WB, 2*WB+2*BH, BW, BH, "Perspective");
	for(i=10 ; i<12 ; i++){
	  gen_butt[i]->type(FL_RADIO_BUTTON);
	  gen_butt[i]->labelsize(CTX.fontsize);
	  gen_butt[i]->selection_color(FL_YELLOW);
	}
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Colors");
	o->labelsize(CTX.fontsize);
	o->hide();
	gen_value[0] = new Fl_Value_Input(2*WB, 2*WB+1*BH, IW, BH, "Predefined color scheme");
	gen_value[0]->minimum(0); 
	gen_value[0]->maximum(2); 
	gen_value[0]->step(1);
	gen_value[0]->labelsize(CTX.fontsize);
	gen_value[0]->textsize(CTX.fontsize);
	gen_value[0]->type(FL_HORIZONTAL);
	gen_value[0]->align(FL_ALIGN_RIGHT);
	gen_value[0]->callback(opt_general_color_scheme_cb);
	
	Fl_Scroll* s = new Fl_Scroll(2*WB, 3*WB+2*BH, IW+20, height-3*WB-4*BH);
	i = 0;
	while(GeneralOptions_Color[i].str){
	  gen_col[i] = new Fl_Button(2*WB, 3*WB+(2+i)*BH, IW, BH, GeneralOptions_Color[i].str);
	  gen_col[i]->callback(color_cb, (void*)GeneralOptions_Color[i].function) ;
	  gen_col[i]->labelsize(CTX.fontsize);
	  i++;
	}
	s->end();
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Light");
	o->labelsize(CTX.fontsize);
	o->hide();
	gen_value[1] = new Fl_Value_Input(2*WB, 2*WB+1*BH, IW, BH, "Material shininess");
	gen_value[1]->minimum(0); 
	gen_value[1]->maximum(10);
	gen_value[1]->step(0.1);
	gen_butt[12] = new Fl_Check_Button(2*WB, 2*WB+2*BH, BW, BH, "Moving light");
	gen_butt[12]->type(FL_TOGGLE_BUTTON);
	gen_butt[12]->down_box(FL_DOWN_BOX);
	gen_butt[12]->labelsize(CTX.fontsize);
	gen_butt[12]->selection_color(FL_YELLOW);
	gen_value[2] = new Fl_Value_Input(2*WB, 2*WB+3*BH, IW, BH, "Light position X");
	gen_value[2]->minimum(-1); 
	gen_value[2]->maximum(1);
	gen_value[2]->step(0.01);
	gen_value[3] = new Fl_Value_Input(2*WB, 2*WB+4*BH, IW, BH, "Light position Y");
	gen_value[3]->minimum(-1); 
	gen_value[3]->maximum(1); 
	gen_value[3]->step(0.01);
	gen_value[4] = new Fl_Value_Input(2*WB, 2*WB+5*BH, IW, BH, "Light position Z");
	gen_value[4]->minimum(-1); 
	gen_value[4]->maximum(1); 
	gen_value[4]->step(0.01);
	for(i=1 ; i<5 ; i++){
	  gen_value[i]->labelsize(CTX.fontsize);
	  gen_value[i]->textsize(CTX.fontsize);
	  gen_value[i]->type(FL_HORIZONTAL);
	  gen_value[i]->align(FL_ALIGN_RIGHT);
	}
	o->end();
      }
      o->end();
    }
    
    { 
      Fl_Return_Button* o = new Fl_Return_Button(width-2*BB-2*WB, height-BH-WB, BB, BH, "OK");
      o->labelsize(CTX.fontsize);
      o->callback(opt_general_ok_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Cancel");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)gen_window);
    }
    
    if(CTX.center_windows)
      gen_window->position(m_window->x()+m_window->w()/2-width/2,
			   m_window->y()+9*BH-height/2);
    gen_window->end();
  }
  else{
    if(gen_window->shown())
      gen_window->redraw();
    else
      gen_window->show();
    
  }
  
}

//************************ Create the window for geometry options **********************

void GUI::create_geometry_options_window(){
  int i;
  
  if(!init_geometry_options_window){
    init_geometry_options_window = 1 ;
    
    int width = 25*CTX.fontsize;
    int height = 5*WB+9*BH ;
    
    geo_window = new Fl_Window(width,height);
    geo_window->box(WINDOW_BOX);
    geo_window->label("Geometry options");
    { 
      Fl_Tabs* o = new Fl_Tabs(WB, WB, width-2*WB, height-3*WB-BH);
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Algorithm");
	o->labelsize(CTX.fontsize);
	o->hide();
	geo_butt[8] = new Fl_Check_Button(2*WB, 2*WB+1*BH, IW, BH, "Auto coherence (suppress duplicates)");
	geo_butt[8]->type(FL_TOGGLE_BUTTON);
	geo_butt[8]->down_box(FL_DOWN_BOX);
	geo_butt[8]->labelsize(CTX.fontsize);
	geo_butt[8]->selection_color(FL_YELLOW);
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Visibility");
	o->labelsize(CTX.fontsize);
	geo_butt[0] = new Fl_Check_Button(2*WB, 2*WB+1*BH, IW, BH, "Points");
	geo_butt[1] = new Fl_Check_Button(2*WB, 2*WB+2*BH, IW, BH, "Curves");
	geo_butt[2] = new Fl_Check_Button(2*WB, 2*WB+3*BH, IW, BH, "Surfaces");
	geo_butt[3] = new Fl_Check_Button(2*WB, 2*WB+4*BH, IW, BH, "Volumes");
	geo_butt[4] = new Fl_Check_Button(width/2, 2*WB+1*BH, IW, BH, "Point numbers");
	geo_butt[5] = new Fl_Check_Button(width/2, 2*WB+2*BH, IW, BH, "Curve numbers");
	geo_butt[6] = new Fl_Check_Button(width/2, 2*WB+3*BH, IW, BH, "Surface numbers");
	geo_butt[7] = new Fl_Check_Button(width/2, 2*WB+4*BH, IW, BH, "Volume numbers");
	for(i=0 ; i<8 ; i++){
	  geo_butt[i]->type(FL_TOGGLE_BUTTON);
	  geo_butt[i]->down_box(FL_DOWN_BOX);
	  geo_butt[i]->labelsize(CTX.fontsize);
	  geo_butt[i]->selection_color(FL_YELLOW);
	}
	
	geo_input = new Fl_Input(2*WB, 2*WB+5*BH, IW, BH, "Show by entity number");
	geo_input->labelsize(CTX.fontsize);
	geo_input->textsize(CTX.fontsize);
	geo_input->align(FL_ALIGN_RIGHT);
	geo_input->callback(opt_geometry_show_by_entity_num_cb);
	geo_input->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);
	
	geo_value[0] = new Fl_Value_Input(2*WB, 2*WB+6*BH, IW, BH, "Normals");
	geo_value[0]->minimum(0); 
	geo_value[0]->maximum(100);
	geo_value[0]->step(0.1);
	geo_value[1] = new Fl_Value_Input(2*WB, 2*WB+7*BH, IW, BH, "Tangents");
	geo_value[1]->minimum(0);
	geo_value[1]->maximum(100);
	geo_value[1]->step(0.1);
	for(i=0 ; i<2 ; i++){
	  geo_value[i]->labelsize(CTX.fontsize);
	  geo_value[i]->textsize(CTX.fontsize);
	  geo_value[i]->type(FL_HORIZONTAL);
	  geo_value[i]->align(FL_ALIGN_RIGHT);
	}
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Colors");
	o->labelsize(CTX.fontsize);
	o->hide();
	geo_value[2] = new Fl_Value_Input(2*WB, 2*WB+1*BH, IW, BH, "Predefined color scheme");
	geo_value[2]->minimum(0); 
	geo_value[2]->maximum(2); 
	geo_value[2]->step(1);
	geo_value[2]->labelsize(CTX.fontsize);
	geo_value[2]->textsize(CTX.fontsize);
	geo_value[2]->type(FL_HORIZONTAL);
	geo_value[2]->align(FL_ALIGN_RIGHT);
	geo_value[2]->callback(opt_geometry_color_scheme_cb);
	
	Fl_Scroll* s = new Fl_Scroll(2*WB, 3*WB+2*BH, IW+20, height-3*WB-4*BH);
	i = 0;
	while(GeometryOptions_Color[i].str){
	  geo_col[i] = new Fl_Button(2*WB, 3*WB+(2+i)*BH, IW, BH, GeometryOptions_Color[i].str);
	  geo_col[i]->callback(color_cb, (void*)GeometryOptions_Color[i].function) ;
	  geo_col[i]->labelsize(CTX.fontsize);
	  i++;
	}
	s->end();
	o->end();
      }
      o->end();
    }
    
    { 
      Fl_Return_Button* o = new Fl_Return_Button(width-2*BB-2*WB, height-BH-WB, BB, BH, "OK");
      o->labelsize(CTX.fontsize);
      o->callback(opt_geometry_ok_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Cancel");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)geo_window);
    }
    
    if(CTX.center_windows)
      geo_window->position(m_window->x()+m_window->w()/2-width/2,
			   m_window->y()+9*BH-height/2);
    geo_window->end();
  }
  else{
    if(geo_window->shown())
      geo_window->redraw();
    else
      geo_window->show();
    
  }
  
}

//****************************** Create the window for mesh options ********************

void GUI::create_mesh_options_window(){
  int i;
  
  if(!init_mesh_options_window){
    init_mesh_options_window = 1 ;
    
    int width = 25*CTX.fontsize;
    int height = 5*WB+12*BH ;
    
    mesh_window = new Fl_Window(width,height);
    mesh_window->box(WINDOW_BOX);
    mesh_window->label("Mesh options");
    { 
      Fl_Tabs* o = new Fl_Tabs(WB, WB, width-2*WB, height-3*WB-BH);
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Algorithm");
	o->labelsize(CTX.fontsize);
	o->hide();

	mesh_butt[0] = new Fl_Check_Button(2*WB, 2*WB+1*BH, BW, BH, "Isotropic");
	mesh_butt[1] = new Fl_Check_Button(2*WB, 2*WB+2*BH, BW, BH, "Isotropic (Triangle)");
	mesh_butt[2] = new Fl_Check_Button(2*WB, 2*WB+3*BH, BW, BH, "Anisotropic");
	for(i=0 ; i<3 ; i++){
	  mesh_butt[i]->type(FL_RADIO_BUTTON);
	  mesh_butt[i]->labelsize(CTX.fontsize);
	  mesh_butt[i]->selection_color(FL_YELLOW);
	}

	mesh_value[0] = new Fl_Value_Input(2*WB, 2*WB+4*BH, IW, BH, "Number of smoothing steps");
	mesh_value[0]->minimum(0);
	mesh_value[0]->maximum(100); 
	mesh_value[0]->step(1);
	mesh_value[1] = new Fl_Value_Input(2*WB, 2*WB+5*BH, IW, BH, "Mesh scaling factor");
	mesh_value[1]->minimum(0.001);
	mesh_value[1]->maximum(1000); 
	mesh_value[1]->step(0.001);
	mesh_value[2] = new Fl_Value_Input(2*WB, 2*WB+6*BH, IW, BH, "Characteristic length factor");
	mesh_value[2]->minimum(0.001);
	mesh_value[2]->maximum(1000); 
	mesh_value[2]->step(0.001);
	mesh_value[3] = new Fl_Value_Input(2*WB, 2*WB+7*BH, IW, BH, "Random perturbation factor");
	mesh_value[3]->minimum(1.e-6);
	mesh_value[3]->maximum(1.e-1); 
	mesh_value[3]->step(1.e-6);
	for(i = 0 ; i<4 ; i++){
	  mesh_value[i]->labelsize(CTX.fontsize);
	  mesh_value[i]->textsize(CTX.fontsize);
	  mesh_value[i]->type(FL_HORIZONTAL);
	  mesh_value[i]->align(FL_ALIGN_RIGHT);
	}

	mesh_butt[3] = new Fl_Check_Button(2*WB, 2*WB+8*BH, BW, BH, "Second order elements");
	mesh_butt[3]->deactivate();//2nd order elements do not work. Disable the graphical option.
	mesh_butt[4] = new Fl_Check_Button(2*WB, 2*WB+9*BH, BW, BH, "Interactive");
	mesh_butt[5] = new Fl_Check_Button(2*WB, 2*WB+10*BH, BW, BH, "Constrained background mesh");
	for(i=3 ; i<6 ; i++){
	  mesh_butt[i]->type(FL_TOGGLE_BUTTON);
	  mesh_butt[i]->down_box(FL_DOWN_BOX);
	  mesh_butt[i]->labelsize(CTX.fontsize);
	  mesh_butt[i]->selection_color(FL_YELLOW);
	}
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Visibility");
	o->labelsize(CTX.fontsize);
	mesh_butt[6] = new Fl_Check_Button(2*WB, 2*WB+1*BH, IW, BH, "Points");
	mesh_butt[7] = new Fl_Check_Button(2*WB, 2*WB+2*BH, IW, BH, "Lines");
	mesh_butt[8] = new Fl_Check_Button(2*WB, 2*WB+3*BH, IW, BH, "Surfaces");
	mesh_butt[9] = new Fl_Check_Button(2*WB, 2*WB+4*BH, IW, BH, "Volumes");
	mesh_butt[10] = new Fl_Check_Button(width/2, 2*WB+1*BH, IW, BH, "Point numbers");
	mesh_butt[11] = new Fl_Check_Button(width/2, 2*WB+2*BH, IW, BH, "Line numbers");
	mesh_butt[12] = new Fl_Check_Button(width/2, 2*WB+3*BH, IW, BH, "Surface numbers");
	mesh_butt[13] = new Fl_Check_Button(width/2, 2*WB+4*BH, IW, BH, "Volume numbers");
	for(i=6 ; i<14 ; i++){
	  mesh_butt[i]->type(FL_TOGGLE_BUTTON);
	  mesh_butt[i]->down_box(FL_DOWN_BOX);
	  mesh_butt[i]->labelsize(CTX.fontsize);
	  mesh_butt[i]->selection_color(FL_YELLOW);
	}
	mesh_input = new Fl_Input(2*WB, 2*WB+5*BH, IW, BH, "Show by entity Number");
	mesh_input->labelsize(CTX.fontsize);
	mesh_input->textsize(CTX.fontsize);
	mesh_input->align(FL_ALIGN_RIGHT);
	mesh_input->callback(opt_mesh_show_by_entity_num_cb);
	mesh_input->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);

	mesh_value[4] = new Fl_Value_Input(2*WB, 2*WB+6*BH, IW/2, BH);
	mesh_value[4]->minimum(0); 
	mesh_value[4]->maximum(1);
	mesh_value[4]->step(0.001);
	mesh_value[5] = new Fl_Value_Input(2*WB+IW/2, 2*WB+6*BH, IW/2, BH, "Quality range");
	mesh_value[5]->minimum(0); 
	mesh_value[5]->maximum(1);
	mesh_value[5]->step(0.001);

	mesh_value[6] = new Fl_Value_Input(2*WB, 2*WB+7*BH, IW/2, BH);
	mesh_value[7] = new Fl_Value_Input(2*WB+IW/2, 2*WB+7*BH, IW/2, BH, "Size range");

	mesh_value[8] = new Fl_Value_Input(2*WB, 2*WB+8*BH, IW, BH, "Normals");
	mesh_value[8]->minimum(0); 
	mesh_value[8]->maximum(100);
	mesh_value[8]->step(1);
	for(i=4 ; i<9 ; i++){
	  mesh_value[i]->labelsize(CTX.fontsize);
	  mesh_value[i]->textsize(CTX.fontsize);
	  mesh_value[i]->type(FL_HORIZONTAL);
	  mesh_value[i]->align(FL_ALIGN_RIGHT);
	}
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Aspect");
	o->labelsize(CTX.fontsize);
	o->hide();
	mesh_butt[14] = new Fl_Check_Button(2*WB, 2*WB+1*BH, BW, BH, "Wireframe");
	mesh_butt[15] = new Fl_Check_Button(2*WB, 2*WB+2*BH, BW, BH, "Hidden lines");
	mesh_butt[16] = new Fl_Check_Button(2*WB, 2*WB+3*BH, BW, BH, "Solid");
	for(i=14 ; i<17 ; i++){
	  mesh_butt[i]->type(FL_RADIO_BUTTON);
	  mesh_butt[i]->labelsize(CTX.fontsize);
	  mesh_butt[i]->selection_color(FL_YELLOW);
	}
	mesh_value[9] = new Fl_Value_Input(2*WB, 2*WB+4*BH, IW, BH, "Explode elements");
	mesh_value[9]->minimum(0);
	mesh_value[9]->maximum(1);
	mesh_value[9]->step(0.01);
	mesh_value[9]->labelsize(CTX.fontsize);
	mesh_value[9]->textsize(CTX.fontsize);
	mesh_value[9]->type(FL_HORIZONTAL);
	mesh_value[9]->align(FL_ALIGN_RIGHT);
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Colors");
	o->labelsize(CTX.fontsize);
	o->hide();
	mesh_butt[17] = new Fl_Check_Button(2*WB, 2*WB+1*BH, IW, BH, "Switch color by entity");
	mesh_butt[17]->type(FL_TOGGLE_BUTTON);
	mesh_butt[17]->down_box(FL_DOWN_BOX);
	mesh_butt[17]->labelsize(CTX.fontsize);
	mesh_butt[17]->selection_color(FL_YELLOW);

	mesh_value[10] = new Fl_Value_Input(2*WB, 2*WB+2*BH, IW, BH, "Predefined color scheme");
	mesh_value[10]->minimum(0); 
	mesh_value[10]->maximum(2); 
	mesh_value[10]->step(1);
	mesh_value[10]->labelsize(CTX.fontsize);
	mesh_value[10]->textsize(CTX.fontsize);
	mesh_value[10]->type(FL_HORIZONTAL);
	mesh_value[10]->align(FL_ALIGN_RIGHT);
	mesh_value[10]->callback(opt_mesh_color_scheme_cb);

	Fl_Scroll* s = new Fl_Scroll(2*WB, 3*WB+3*BH, IW+20, height-3*WB-5*BH);
	i = 0;
	while(MeshOptions_Color[i].str){
	  mesh_col[i] = new Fl_Button(2*WB, 3*WB+(3+i)*BH, IW, BH, MeshOptions_Color[i].str);
	  mesh_col[i]->callback(color_cb, (void*)MeshOptions_Color[i].function) ;
	  mesh_col[i]->labelsize(CTX.fontsize);
	  i++;
	}
	s->end();
	o->end();
      }
      o->end();
    }

    { 
      Fl_Return_Button* o = new Fl_Return_Button(width-2*BB-2*WB, height-BH-WB, BB, BH, "OK");
      o->labelsize(CTX.fontsize);
      o->callback(opt_mesh_ok_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Cancel");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)mesh_window);
    }

    if(CTX.center_windows)
      mesh_window->position(m_window->x()+m_window->w()/2-width/2,
			    m_window->y()+9*BH-height/2);
    mesh_window->end();
  }
  else{
    if(mesh_window->shown())
      mesh_window->redraw();
    else
      mesh_window->show();

  }

}

//******************** Create the window for solver options *******************

void GUI::create_solver_options_window(){

  if(!init_solver_options_window){
    init_solver_options_window = 1 ;

    int width = 20*CTX.fontsize;
    int height = 5*WB+8*BH ;

    solver_window = new Fl_Window(width,height);
    solver_window->box(WINDOW_BOX);
    solver_window->label("Solver options");
    { 
      Fl_Tabs* o = new Fl_Tabs(WB, WB, width-2*WB, height-3*WB-BH);
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Solvers");
	o->labelsize(CTX.fontsize);
	Fl_Box *text =  new Fl_Box(FL_NO_BOX, 2*WB, 3*WB+1*BH, width-4*WB, 2*BH,
				   "There are no global solver options available yet");
	text->align(FL_ALIGN_LEFT|FL_ALIGN_TOP|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
	text->labelsize(CTX.fontsize);
	o->end();
      }
      o->end();
    }

    { 
      Fl_Return_Button* o = new Fl_Return_Button(width-2*BB-2*WB, height-BH-WB, BB, BH, "OK");
      o->labelsize(CTX.fontsize);
      o->callback(opt_solver_ok_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Cancel");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)solver_window);
    }

    if(CTX.center_windows)
      solver_window->position(m_window->x()+m_window->w()/2-width/2,
			      m_window->y()+9*BH-height/2);
    solver_window->end();
  }
  else{
    if(solver_window->shown())
      solver_window->redraw();
    else
      solver_window->show();

  }

}


//******************** Create the window for post-processing options *******************

void GUI::create_post_options_window(){
  int i;

  if(!init_post_options_window){
    init_post_options_window = 1 ;

    int width = 24*CTX.fontsize;
    int height = 5*WB+10*BH ;

    post_window = new Fl_Window(width,height);
    post_window->box(WINDOW_BOX);
    post_window->label("Post-processing options");
    { 
      Fl_Tabs* o = new Fl_Tabs(WB, WB, width-2*WB, height-3*WB-BH);
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Views");
	o->labelsize(CTX.fontsize);
	post_butt[0] = new Fl_Check_Button(2*WB, 2*WB+1*BH, BW, BH, "Independent views");
	post_butt[1] = new Fl_Check_Button(2*WB, 2*WB+2*BH, BW, BH, "Apply next changes to all visible views");
	post_butt[2] = new Fl_Check_Button(2*WB, 2*WB+3*BH, BW, BH, "Apply next changes to all views");
	post_butt[3] = new Fl_Check_Button(2*WB, 2*WB+4*BH, BW, BH, "Force same options for all visible views");
	post_butt[4] = new Fl_Check_Button(2*WB, 2*WB+5*BH, BW, BH, "Force same options for all views");
	for(i=0 ; i<5 ; i++){
	  post_butt[i]->type(FL_RADIO_BUTTON);
	  post_butt[i]->labelsize(CTX.fontsize);
	  post_butt[i]->selection_color(FL_YELLOW);
	}
	Fl_Box *text =  new Fl_Box(FL_NO_BOX, 2*WB, 3*WB+6*BH, width-4*WB, 2*BH,
				   "Individual view options are available "
				   "by right-clicking on each view button "
				   "in the post-processing menu");
	text->align(FL_ALIGN_LEFT|FL_ALIGN_TOP|FL_ALIGN_INSIDE|FL_ALIGN_WRAP);
	text->labelsize(CTX.fontsize);
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Smoothing");
	o->labelsize(CTX.fontsize);
	post_butt[3] = new Fl_Check_Button(2*WB, 2*WB+1*BH, BW, BH, "Smooth views during merge");
	post_butt[3]->type(FL_TOGGLE_BUTTON);
	post_butt[3]->down_box(FL_DOWN_BOX);
	post_butt[3]->labelsize(CTX.fontsize);
	post_butt[3]->selection_color(FL_YELLOW);
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Animation");
	o->labelsize(CTX.fontsize);
	o->hide();
	post_value[0] = new Fl_Value_Input(2*WB, 2*WB+1*BH, IW, BH, "Delay");
	post_value[0]->minimum(0);
	post_value[0]->maximum(10); 
	post_value[0]->step(0.01);
	post_value[0]->labelsize(CTX.fontsize);
	post_value[0]->textsize(CTX.fontsize);
	post_value[0]->type(FL_HORIZONTAL);
	post_value[0]->align(FL_ALIGN_RIGHT);
	o->end();
      }
      o->end();
    }

    { 
      Fl_Return_Button* o = new Fl_Return_Button(width-2*BB-2*WB, height-BH-WB, BB, BH, "OK");
      o->labelsize(CTX.fontsize);
      o->callback(opt_post_ok_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Cancel");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)post_window);
    }

    if(CTX.center_windows)
      post_window->position(m_window->x()+m_window->w()/2-width/2,
			    m_window->y()+9*BH-height/2);
    post_window->end();
  }
  else{
    if(post_window->shown())
      post_window->redraw();
    else
      post_window->show();

  }

}

//*********************** Create the window for the statistics *************************

void GUI::create_statistics_window(){
  int i, num=0;

  if(!init_statistics_window){
    init_statistics_window = 1 ;

    int width = 24*CTX.fontsize;
    int height = 5*WB+17*BH ;

    stat_window = new Fl_Window(width,height);
    stat_window->box(WINDOW_BOX);
    stat_window->label("Statistics");
    {
      Fl_Tabs* o = new Fl_Tabs(WB, WB, width-2*WB, height-3*WB-BH);
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Geometry");
	o->labelsize(CTX.fontsize);
	o->hide();
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+1*BH, IW, BH, "Points");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+2*BH, IW, BH, "Curves");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+3*BH, IW, BH, "Surfaces");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+4*BH, IW, BH, "Volumes");
	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Mesh");
	o->labelsize(CTX.fontsize);
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+1*BH, IW, BH, "Nodes on curves");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+2*BH, IW, BH, "Nodes on surfaces");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+3*BH, IW, BH, "Nodes in volumes");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+4*BH, IW, BH, "Triangles");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+5*BH, IW, BH, "Quadrangles");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+6*BH, IW, BH, "Tetrahedra");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+7*BH, IW, BH, "Hexahedra");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+8*BH, IW, BH, "Prisms");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+9*BH, IW, BH, "Pyramids");

	stat_value[num++] = new Fl_Output(2*WB, 2*WB+10*BH, IW, BH, "Time for 1D mesh");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+11*BH, IW, BH, "Time for 2D mesh");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+12*BH, IW, BH, "Time for 3D mesh");

	stat_value[num++] = new Fl_Output(2*WB, 2*WB+13*BH, IW, BH, "Gamma factor");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+14*BH, IW, BH, "Eta factor");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+15*BH, IW, BH, "Rho factor");

	Fl_Button* b0 = new Fl_Button(width-BB-2*WB, 2*WB+13*BH, BB, BH, "List");
	b0->labelsize(CTX.fontsize);
	b0->callback(opt_statistics_histogram_cb, (void*)0);
	Fl_Button* b1 = new Fl_Button(width-BB-2*WB, 2*WB+14*BH, BB, BH, "List");
	b1->labelsize(CTX.fontsize);
	b1->callback(opt_statistics_histogram_cb, (void*)1);
	Fl_Button* b2 = new Fl_Button(width-BB-2*WB, 2*WB+15*BH, BB, BH, "List");
	b2->labelsize(CTX.fontsize);
	b2->callback(opt_statistics_histogram_cb, (void*)2);

	o->end();
      }
      { 
	Fl_Group* o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Post-processing");
	o->labelsize(CTX.fontsize);
	o->hide();
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+1*BH, IW, BH, "Views");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+2*BH, IW, BH, "Visible points");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+3*BH, IW, BH, "Visible lines");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+4*BH, IW, BH, "Visible triangles");
	stat_value[num++] = new Fl_Output(2*WB, 2*WB+5*BH, IW, BH, "Visible tetrahedra");
	o->end();
      }
      o->end();
    }

    for(i=0 ; i<num ; i++){
      stat_value[i]->labelsize(CTX.fontsize);
      stat_value[i]->textsize(CTX.fontsize);
      stat_value[i]->type(FL_HORIZONTAL);
      stat_value[i]->align(FL_ALIGN_RIGHT);
      stat_value[i]->value(0);
    }

    { 
      Fl_Button* o = new Fl_Button(width-2*BB-2*WB, height-BH-WB, BB, BH, "Update");
      o->labelsize(CTX.fontsize);
      o->callback(opt_statistics_update_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Cancel");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)stat_window);
    }

    if(CTX.center_windows)
      stat_window->position(m_window->x()+m_window->w()/2-width/2,
			    m_window->y()+9*BH-height/2);
    stat_window->end();
    set_statistics();
    stat_window->show();
  }
  else{
    if(stat_window->shown())
      stat_window->redraw();
    else{
      set_statistics();
      stat_window->show();     
    }
  }

}

void GUI::set_statistics(){

  int i,num=0;
  static double  s[50], p[20];
  static char    label[50][256];

  GetStatistics(s);

  // geom
  sprintf(label[num], "%g", s[0]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[1]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[2]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[3]); stat_value[num]->value(label[num]); num++;

  // mesh
  sprintf(label[num], "%g", s[4]);  stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[5]);  stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[6]);  stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[7]-s[8]); 
                                    stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[8]);  stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[9]);  stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[10]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[11]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[12]); stat_value[num]->value(label[num]); num++;

  sprintf(label[num], "%g", s[13]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[14]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g", s[15]); stat_value[num]->value(label[num]); num++;

  sprintf(label[num], "%.4g (%.4g->%.4g)", s[17], s[19], s[18]); 
                                    stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%.4g (%.4g->%.4g)", s[20], s[22], s[21]); 
                                    stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%.4g (%.4g->%.4g)", s[23], s[25], s[24]);
                                    stat_value[num]->value(label[num]); num++;

  // post
  p[0] = List_Nbr(Post_ViewList) ;
  sprintf(label[num], "%g", p[0]); stat_value[num]->value(label[num]); num++;
  p[1] = p[2] = p[3] = p[4] = p[5] = p[6] = p[7] = p[8] = 0 ;
  for(i=0 ; i<List_Nbr(Post_ViewList) ; i++){
    Post_View *v = (Post_View*)List_Pointer(Post_ViewList, i);
    p[1] += v->NbSP + v->NbVP + v->NbTP;
    p[2] += v->NbSL + v->NbVL + v->NbTL;
    p[3] += v->NbST + v->NbVT + v->NbTT;
    p[4] += v->NbSS + v->NbVS + v->NbTS;
    if(v->Visible){
      if(v->DrawPoints)	p[5] += (v->DrawScalars ? v->NbSP : 0) + 
			        (v->DrawVectors ? v->NbVP : 0) + 
			        (v->DrawTensors ? v->NbTP : 0) ;
      if(v->DrawLines) p[6] += (v->DrawScalars ? v->NbSL : 0) + 
			       (v->DrawVectors ? v->NbVL : 0) + 
			       (v->DrawTensors ? v->NbTL : 0) ;
      if(v->DrawTriangles) p[7] += (v->DrawScalars ? v->NbST : 0) + 
			           (v->DrawVectors ? v->NbVT : 0) + 
			           (v->DrawTensors ? v->NbTT : 0) ;
      if(v->DrawTetrahedra) p[8] += (v->DrawScalars ? v->NbSS : 0) + 
			            (v->DrawVectors ? v->NbVS : 0) + 
   			            (v->DrawTensors ? v->NbTS : 0) ;
    }
  }
  sprintf(label[num], "%g/%g", p[5],p[1]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", p[6],p[2]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", p[7],p[3]); stat_value[num]->value(label[num]); num++;
  sprintf(label[num], "%g/%g", p[8],p[4]); stat_value[num]->value(label[num]); num++;
}


//*********************** Create the window for the plugins *************************

void GUI::add_multiline_in_browser(Fl_Browser *o, char* prefix, char *str){
  int start = 0, len;
  char *buff;
  if(!strlen(str) || !strcmp(str, "\n")){
    o->add("");
    return;
  }
  for(unsigned int i=0 ; i<strlen(str) ; i++){
    if(i==strlen(str)-1 || str[i]=='\n'){
      len = i-start+(str[i]=='\n'?0:1);
      buff = new char[len+strlen(prefix)+2];
      strcpy(buff, prefix);
      strncat(buff, &str[start], len);
      buff[len+strlen(prefix)]='\0';
      o->add(buff);
      start = i+1;
    }
  }
}

PluginDialogBox * GUI::create_plugin_window(GMSH_Plugin *p){
  char buffer[1024],namep[1024],copyright[256],author[256],help[1024];

  // get plugin info

  int n = p->getNbOptions();
  p->getName(namep);
  p->getInfos(author,copyright,help);

  // create window

  int width = 20*CTX.fontsize;
  int height = ((n>5?n:5)+2)*BH + 5*WB;

  PluginDialogBox *pdb = new PluginDialogBox;
  pdb->main_window = new Fl_Window(width,height);
  pdb->main_window->box(WINDOW_BOX);
  sprintf(buffer,"%s Plugin",namep);
  char *nbuffer = new char[strlen(buffer)+1];
  strcpy(nbuffer,buffer);
  pdb->main_window->label(nbuffer);

  { 
    Fl_Tabs *o = new Fl_Tabs(WB, WB, width-2*WB, height-3*WB-1*BH);
    { 
      Fl_Group *g = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Options");
      g->labelsize(CTX.fontsize);

      if(n > 20)Msg(GERROR, "Plugin has too many parameters");
      
      for(int i=0;i<n;i++){
	StringXNumber *sxn;
	sxn = p->GetOption(i);
	pdb->view_value[i] = new Fl_Value_Input(2*WB, 2*WB+(i+1)*BH, IW, BH, sxn->str);
	pdb->view_value[i]->labelsize(CTX.fontsize);
	pdb->view_value[i]->textsize(CTX.fontsize);
	pdb->view_value[i]->type(FL_HORIZONTAL);
	pdb->view_value[i]->align(FL_ALIGN_RIGHT);
	pdb->view_value[i]->value(sxn->def);
      }

      g->end();
    }
    { 
      Fl_Group *g = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "About");
      g->labelsize(CTX.fontsize);

      Fl_Browser *o = new Fl_Browser(2*WB, 2*WB+1*BH, width-4*WB, height-5*WB-2*BH);

      o->add("");
      add_multiline_in_browser(o, "@c@b@.", namep);
      o->add("");
      add_multiline_in_browser(o, "", help);
      o->add("");
      add_multiline_in_browser(o, "Author(s): ", author);
      add_multiline_in_browser(o, "Copyright: ", copyright);
      o->textsize(CTX.fontsize);
      
      pdb->main_window->resizable(o);

      g->end();
    }
    o->end();
  }

  Fl_Button* cancel = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Close");
  cancel->labelsize(CTX.fontsize);
  cancel->callback(cancel_cb, (void*)pdb->main_window);

  pdb->run_button = new Fl_Return_Button(width-2*BB-2*WB, height-BH-WB, BB, BH, "Run");
  pdb->run_button->labelsize(CTX.fontsize);

  if(CTX.center_windows)
    pdb->main_window->position(m_window->x()+m_window->w()/2-width/2,
			       m_window->y()+9*BH-height/2);

  pdb->main_window->end();  

  return pdb;
}

//********************** Create the window for the messages ****************************

void GUI::create_message_window(){

  if(!init_message_window){
    init_message_window = 1 ;

    int width = CTX.msg_size[0];
    int height = CTX.msg_size[1];
    
    msg_window = new Fl_Window(width,height);
    msg_window->box(WINDOW_BOX);
    msg_window->label("Messages");
    
    msg_browser = new Fl_Browser(WB, WB, width-2*WB, height-3*WB-BH);
    msg_browser->textfont(FL_COURIER);
    msg_browser->textsize(CTX.fontsize);

    { 
      Fl_Return_Button* o = new Fl_Return_Button(width-3*BB-3*WB, height-BH-WB, BB, BH, "Save");
      o->labelsize(CTX.fontsize);
      o->callback(opt_message_save_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-2*BB-2*WB, height-BH-WB, BB, BH, "Clear");
      o->labelsize(CTX.fontsize);
      o->callback(opt_message_clear_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Cancel");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)msg_window);
    }

    msg_window->resizable(msg_browser);

    msg_window->position(CTX.msg_position[0], CTX.msg_position[1]);
    msg_window->end();
  }
  else{
    if(msg_window->shown())
      msg_window->redraw();
    else
      msg_window->show();
  }

}

void GUI::add_message(char *msg){
  msg_browser->add(msg,0);
  msg_browser->bottomline(msg_browser->size());
}

void GUI::save_message(char *filename){
  FILE *fp;

  if(!(fp = fopen(filename,"w"))) {
    Msg(WARNING, "Unable to open file '%s'", filename); 
    return;
  }
  for(int i = 1 ; i<=msg_browser->size() ; i++){
    const char *c=msg_browser->text(i);
    if(c[0]=='@') fprintf(fp, "%s\n", &c[3]);
    else fprintf(fp, "%s\n", c);
  }

  Msg(INFO, "Log creation complete '%s'", filename);
  Msg(STATUS2, "Wrote '%s'", filename);
  fclose(fp);
}

void GUI::fatal_error(char *filename){
  fl_alert("A fatal error has occurred, which will force Gmsh to exit "
	   "(all messages have been saved in the error log file '%s')", filename);
}

//******************************* Create the about window ******************************

void GUI::create_about_window(){
  char buffer[1024];

  if(!init_about_window){
    init_about_window = 1 ;

    int width = 40*CTX.fontsize;
    int height = 10*BH ;

    about_window = new Fl_Window(width,height);
    about_window->box(WINDOW_BOX);
    about_window->label("About Gmsh");

    {
      Fl_Box *o = new Fl_Box(2*WB, WB, about_width, height-3*WB-BH);
      about_bmp = new Fl_Bitmap(about_bits,about_width,about_height);
      about_bmp->label(o);
    }

    {
      Fl_Browser *o = new Fl_Browser(WB+80, WB, width-2*WB-80, height-3*WB-BH);
      o->add("");
      o->add("@c@b@.Gmsh");
      o->add("@c@.A three-dimensional finite element mesh generator");
      o->add("@c@.with built-in pre- and post-processing facilities");
      o->add("");
      o->add("@c@.Copyright (c) 1997-2001");
      o->add("@c@.Christophe Geuzaine and Jean-Fran�ois Remacle");
      o->add("");
      o->add("@c@.Please send all questions and bug reports to");
#if (FL_MAJOR_VERSION == 1) && (FL_MINOR_VERSION == 1)
      o->add("@c@b@.gmsh@@geuz.org");
#else
      o->add("@c@b@.gmsh@geuz.org");
#endif
      o->add("");
      sprintf(buffer, "@c@.Version: %.2f", GMSH_VERSION); o->add(buffer);
      sprintf(buffer, "@c@.Build date: %s", GMSH_DATE); o->add(buffer);
      sprintf(buffer, "@c@.Build OS: %s", GMSH_OS); o->add(buffer);
      sprintf(buffer, "@c@.Graphical user interface toolkit: FLTK %d.%d.%d",
	      FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION); o->add(buffer);
      sprintf(buffer, "@c@.Build host: %s", GMSH_HOST); o->add(buffer);
      sprintf(buffer, "@c@.Packaged by: %s", GMSH_PACKAGER); o->add(buffer);
      o->add("");
      o->add("@c@.Visit http://www.geuz.org/gmsh/ for more information");
      o->textsize(CTX.fontsize);
    }

    { 
      Fl_Return_Button* o = new Fl_Return_Button(width-BB-WB, height-BH-WB, BB, BH, "OK");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)about_window);
    }

    if(CTX.center_windows)
      about_window->position(m_window->x()+m_window->w()/2-width/2,
			     m_window->y()+9*BH-height/2);
    about_window->end();
  }
  else{
    if(about_window->shown())
      about_window->redraw();
    else
      about_window->show();
  }

}

//************************* Create the window for view options *************************

// WARNING! Don't forget to add the set_changed_cb() callback to any new widget!

void GUI::create_view_options_window(int num){
  int i;

  if(!init_view_window){
    init_view_window = 1 ;

    // initialise all buttons to NULL (see the clear_changed() in opt_view_options_bd)
    for(i=0; i<VIEW_OPT_BUTT; i++){
      view_butt[i] = NULL;
      view_value[i] = NULL;
      view_input[i] = NULL;
    }

    int width = 32*CTX.fontsize;
    int height = 5*WB+11*BH;
    
    view_window = new Fl_Window(width,height);
    view_window->box(WINDOW_BOX);

    { 
      Fl_Tabs* o = new Fl_Tabs(WB, WB, width-2*WB, height-3*WB-BH);
      // General
      { 
	Fl_Group *o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "General");
	o->labelsize(CTX.fontsize);
        o->hide();

	view_input[0] = new Fl_Input(2*WB, 2*WB+1*BH, IW, BH, "Name");
	view_input[1] = new Fl_Input(2*WB, 2*WB+2*BH, IW, BH, "Format");
	for(i=0 ; i<2 ; i++){
	  view_input[i]->labelsize(CTX.fontsize);
	  view_input[i]->textsize(CTX.fontsize);
	  view_input[i]->align(FL_ALIGN_RIGHT);
	  view_input[i]->callback(set_changed_cb, 0);
	}

        view_butt[13] = new Fl_Check_Button(2*WB, 2*WB+3*BH, BW, BH, "Show elements");
        view_butt[14] = new Fl_Check_Button(2*WB, 2*WB+4*BH, BW, BH, "Show color bar");
        view_butt[15] = new Fl_Check_Button(2*WB, 2*WB+5*BH, BW, BH, "Display time");
        view_butt[16] = new Fl_Check_Button(2*WB, 2*WB+6*BH, BW, BH, "Transparent color bar");
	view_butt[17] = new Fl_Check_Button(2*WB, 2*WB+7*BH, BW, BH, "Enable Lighting");

	view_butt[27] = new Fl_Check_Button(2*WB, 2*WB+8*BH, BW, BH, "Smooth normals");	
	view_butt[27]->type(FL_TOGGLE_BUTTON);
	view_butt[27]->down_box(FL_DOWN_BOX);
	view_butt[27]->labelsize(CTX.fontsize);
	view_butt[27]->selection_color(FL_YELLOW);
	view_butt[27]->callback(set_changed_cb, 0);

	view_value[13] = new Fl_Value_Input(2*WB, 2*WB+9*BH, IW, BH, "Angle");
	view_value[13]->labelsize(CTX.fontsize);
	view_value[13]->textsize(CTX.fontsize);
	view_value[13]->type(FL_HORIZONTAL);
	view_value[13]->align(FL_ALIGN_RIGHT);
	view_value[13]->minimum(0.); 
	view_value[13]->step(1.); 
	view_value[13]->maximum(180.); 
	view_value[13]->callback(set_changed_cb, 0);

	view_value[11] = new Fl_Value_Input(width/2, 2*WB+ 1*BH, IW, BH, "Boundary");
	view_value[11]->labelsize(CTX.fontsize);
	view_value[11]->textsize(CTX.fontsize);
	view_value[11]->type(FL_HORIZONTAL);
	view_value[11]->align(FL_ALIGN_RIGHT);
	view_value[11]->minimum(0); 
	view_value[11]->step(1); 
	view_value[11]->maximum(3); 
	view_value[11]->callback(set_changed_cb, 0);

	view_value[12] = new Fl_Value_Input(width/2, 2*WB+ 2*BH, IW, BH, "Explode");
	view_value[12]->labelsize(CTX.fontsize);
	view_value[12]->textsize(CTX.fontsize);
	view_value[12]->type(FL_HORIZONTAL);
	view_value[12]->align(FL_ALIGN_RIGHT);
	view_value[12]->minimum(0.); 
	view_value[12]->step(0.01); 
	view_value[12]->maximum(1.); 
	view_value[12]->callback(set_changed_cb, 0);

        view_butt[18] = new Fl_Check_Button(width/2, 2*WB+3*BH, BW, BH, "Draw points");
        view_butt[19] = new Fl_Check_Button(width/2, 2*WB+4*BH, BW, BH, "Draw lines");
        view_butt[20] = new Fl_Check_Button(width/2, 2*WB+5*BH, BW, BH, "Draw triangles");
        view_butt[21] = new Fl_Check_Button(width/2, 2*WB+6*BH, BW, BH, "Draw tetrahedra");
        view_butt[22] = new Fl_Check_Button(width/2, 2*WB+7*BH, BW, BH, "Draw scalar values");
        view_butt[23] = new Fl_Check_Button(width/2, 2*WB+8*BH, BW, BH, "Draw vector values");
        view_butt[24] = new Fl_Check_Button(width/2, 2*WB+9*BH, BW, BH, "Draw tensor values");
	for(i=13 ; i<25 ; i++){
	  view_butt[i]->type(FL_TOGGLE_BUTTON);
	  view_butt[i]->down_box(FL_DOWN_BOX);
	  view_butt[i]->labelsize(CTX.fontsize);
	  view_butt[i]->selection_color(FL_YELLOW);
	  view_butt[i]->callback(set_changed_cb, 0);
	}

        o->end();
      }
      // Range
      { 
	Fl_Group *o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Range");
	o->labelsize(CTX.fontsize);
	o->hide();
        view_butt[0] = new Fl_Check_Button(2*WB, 2*WB+1*BH, BW, BH, "Custom range");
	view_butt[0]->type(FL_TOGGLE_BUTTON);
	view_butt[0]->down_box(FL_DOWN_BOX);
	view_butt[0]->labelsize(CTX.fontsize);
	view_butt[0]->selection_color(FL_YELLOW);
	//no set_changed since customrange has its own callback

        view_value[0] = new Fl_Value_Input(2*WB, 2*WB+2*BH, IW, BH, "Minimum");
        view_value[1] = new Fl_Value_Input(2*WB, 2*WB+3*BH, IW, BH, "Maximum");
	for(i=0 ; i<2 ; i++){
	  view_value[i]->labelsize(CTX.fontsize);
	  view_value[i]->textsize(CTX.fontsize);
	  view_value[i]->type(FL_HORIZONTAL);
	  view_value[i]->align(FL_ALIGN_RIGHT);
	  view_value[i]->callback(set_changed_cb, 0);
	}

	view_butt[1] = new Fl_Check_Button(2*WB, 2*WB+4*BH, BW, BH, "Linear");
	view_butt[2] = new Fl_Check_Button(2*WB, 2*WB+5*BH, BW, BH, "Logarithmic");
	for(i=1 ; i<3 ; i++){
	  view_butt[i]->type(FL_RADIO_BUTTON);
	  view_butt[i]->labelsize(CTX.fontsize);
	  view_butt[i]->selection_color(FL_YELLOW);
	  view_butt[i]->callback(set_changed_cb, 0);
	}

	view_butt[26] = new Fl_Check_Button(2*WB, 2*WB+6*BH, BW, BH, "Double logarithmic");
	view_butt[26]->type(FL_RADIO_BUTTON);
	view_butt[26]->labelsize(CTX.fontsize);
	view_butt[26]->selection_color(FL_YELLOW);
	view_butt[26]->callback(set_changed_cb, 0);

        view_butt[25] = new Fl_Check_Button(2*WB, 2*WB+7*BH, BW, BH, "Saturate values");
	view_butt[25]->type(FL_TOGGLE_BUTTON);
	view_butt[25]->down_box(FL_DOWN_BOX);
	view_butt[25]->labelsize(CTX.fontsize);
	view_butt[25]->selection_color(FL_YELLOW);
	view_butt[25]->callback(set_changed_cb, 0);

	o->end();
      }
      // Intervals
      {
	Fl_Group *o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Intervals");
	o->labelsize(CTX.fontsize);
	//view_intervals->hide();
	view_value[2] = new Fl_Value_Input(2*WB, 2*WB+1*BH, IW, BH, "Number of intervals");
	view_value[2]->labelsize(CTX.fontsize);
	view_value[2]->textsize(CTX.fontsize);
	view_value[2]->type(FL_HORIZONTAL);
	view_value[2]->align(FL_ALIGN_RIGHT);
	view_value[2]->minimum(1); 
	view_value[2]->maximum(256); 
	view_value[2]->step(1);
	view_value[2]->callback(set_changed_cb, 0);

	view_butt[3] = new Fl_Check_Button(2*WB, 2*WB+2*BH, BW, BH, "Iso-values");
	view_butt[4] = new Fl_Check_Button(2*WB, 2*WB+3*BH, BW, BH, "Filled iso-values");
	view_butt[5] = new Fl_Check_Button(2*WB, 2*WB+4*BH, BW, BH, "Continuous map");
	view_butt[6] = new Fl_Check_Button(2*WB, 2*WB+5*BH, BW, BH, "Numeric values");
	for(i=3 ; i<7 ; i++){
	  view_butt[i]->type(FL_RADIO_BUTTON);
	  view_butt[i]->labelsize(CTX.fontsize);
	  view_butt[i]->selection_color(FL_YELLOW);
	  view_butt[i]->callback(set_changed_cb, 0);
	}
        o->end();
      }
      // Offset and Raise
      { 
	Fl_Group *o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Offset");
	o->labelsize(CTX.fontsize);
        o->hide();
	view_value[3] = new Fl_Value_Input(2*WB, 2*WB+1*BH, IW, BH, "X offset");
        view_value[4] = new Fl_Value_Input(2*WB, 2*WB+2*BH, IW, BH, "Y offset");
	view_value[5] = new Fl_Value_Input(2*WB, 2*WB+3*BH, IW, BH, "Z offset");
	view_value[6] = new Fl_Value_Input(width/2, 2*WB+1*BH, IW, BH, "X raise");
        view_value[7] = new Fl_Value_Input(width/2, 2*WB+2*BH, IW, BH, "Y raise");
	view_value[8] = new Fl_Value_Input(width/2, 2*WB+3*BH, IW, BH, "Z raise");
	for(i=3 ; i<9 ; i++){
	  view_value[i]->labelsize(CTX.fontsize);
	  view_value[i]->textsize(CTX.fontsize);
	  view_value[i]->type(FL_HORIZONTAL);
	  view_value[i]->align(FL_ALIGN_RIGHT);
	  view_value[i]->callback(set_changed_cb, 0);
	}	
	o->end();
      }
      // Time step
      { 
	view_timestep = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Time step");
	view_timestep->labelsize(CTX.fontsize);
        view_timestep->hide();
	view_value[9] = new Fl_Value_Input(2*WB, 2*WB+BH, IW, BH, "Time step number");
	view_value[9]->labelsize(CTX.fontsize);
	view_value[9]->textsize(CTX.fontsize);
	view_value[9]->type(FL_HORIZONTAL);
	view_value[9]->align(FL_ALIGN_RIGHT);
	view_value[9]->minimum(0); 
	view_value[9]->maximum(0); 
	view_value[9]->step(1);
	view_timestep->end();
	//no set_changed since timestep has its own callback
      }
      // Vector display
      { 
	view_vector = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Vector");
	view_vector->labelsize(CTX.fontsize);
        view_vector->hide();

	{
	  Fl_Group *o = new Fl_Group(2*WB, WB+BH, width-4*WB, 2*BH, 0);
	  view_butt[7] = new Fl_Check_Button(2*WB, 2*WB+1*BH, IW, BH, "Line");
	  view_butt[8] = new Fl_Check_Button(2*WB, 2*WB+2*BH, IW, BH, "Arrow");
	  view_butt[9] = new Fl_Check_Button(width/2, 2*WB+1*BH, IW, BH, "Cone");
	  view_butt[10] = new Fl_Check_Button(width/2, 2*WB+2*BH, IW, BH, "Displacement");
	  for(i=7 ; i<11 ; i++){
	    view_butt[i]->type(FL_RADIO_BUTTON);
	    view_butt[i]->labelsize(CTX.fontsize);
	    view_butt[i]->selection_color(FL_YELLOW);
	    view_butt[i]->callback(set_changed_cb, 0);
	  }
	  o->end();
	}
	{
	  Fl_Group *o = new Fl_Group(2*WB, WB+3*BH, width-4*WB, 2*BH, 0);
	  view_butt[11] = new Fl_Check_Button(2*WB, 2*WB+3*BH, IW, BH, "Cell centered");
	  view_butt[12] = new Fl_Check_Button(2*WB, 2*WB+4*BH, IW, BH, "Vertex centered");
	  for(i=11 ; i<13 ; i++){
	    view_butt[i]->type(FL_RADIO_BUTTON);
	    view_butt[i]->labelsize(CTX.fontsize);
	    view_butt[i]->selection_color(FL_YELLOW);
	    view_butt[i]->callback(set_changed_cb, 0);
	  }
	  o->end();
	}
	view_value[10] = new Fl_Value_Input(2*WB, 2*WB+ 5*BH, IW, BH, "Vector scale");
	view_value[10]->labelsize(CTX.fontsize);
	view_value[10]->textsize(CTX.fontsize);
	view_value[10]->type(FL_HORIZONTAL);
	view_value[10]->align(FL_ALIGN_RIGHT);
	view_value[10]->minimum(0); 
	view_value[10]->callback(set_changed_cb, 0);
	view_vector->end();
      }
      // Colors
      { 
	Fl_Group *o = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Colors");
	o->labelsize(CTX.fontsize);
        o->hide();
	view_colorbar_window = new Colorbar_Window(2*WB, 2*WB+1*BH,
						   width-4*WB, height-5*WB-2*BH);
	view_colorbar_window->end();
	//no set_changed since colorbarwindow has its own callbacks
        o->end();
      }
      o->end();
    }

    { view_ok = new Fl_Return_Button(width-2*BB-2*WB, height-BH-WB, BB, BH, "OK");
      view_ok->labelsize(CTX.fontsize);
    }
    { 
      Fl_Button* o = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Cancel");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)view_window);
    }

    if(CTX.center_windows)
      view_window->position(m_window->x()+m_window->w()/2-width/2,
			    m_window->y()+9*BH-height/2);

    //view_window->resizable(view_colorbar_window);
    view_window->end();
  }
  else{
    update_view_window(num);
    if(view_window->shown())
      view_window->redraw();
    else
      view_window->show();
  }

}

void GUI::update_view_window(int num){
  int i;
  double val;

  view_number = num ;
  Post_View *v = (Post_View*)List_Pointer(Post_ViewList, num);

  static char buffer[1024];
  sprintf(buffer, "Options for \"%s\" (\"%s\")", v->Name, v->FileName);
  view_window->label(buffer);

  // general
  opt_view_show_element(num, GMSH_GUI, 0);
  opt_view_show_scale(num, GMSH_GUI, 0);
  opt_view_show_time(num, GMSH_GUI, 0);
  opt_view_transparent_scale(num, GMSH_GUI, 0);
  opt_view_name(num, GMSH_GUI, NULL);
  opt_view_format(num, GMSH_GUI, NULL);
  opt_view_draw_points(num, GMSH_GUI, 0);
  opt_view_draw_lines(num, GMSH_GUI, 0);
  opt_view_draw_triangles(num, GMSH_GUI, 0);
  opt_view_draw_tetrahedra(num, GMSH_GUI, 0);
  opt_view_draw_scalars(num, GMSH_GUI, 0);
  opt_view_draw_vectors(num, GMSH_GUI, 0);
  opt_view_draw_tensors(num, GMSH_GUI, 0);

  // range
  opt_view_range_type(num, GMSH_GUI, 0);
  view_butt[0]->callback(view_options_custom_cb, (void*)num);
  view_options_custom_cb(0,0);
  view_butt[0]->clear_changed();
  opt_view_custom_min(num, GMSH_GUI, 0);
  opt_view_custom_max(num, GMSH_GUI, 0);
  for(i=0 ; i<2 ; i++){
    view_value[i]->minimum(v->CustomMin); 
    view_value[i]->maximum(v->CustomMax); 
  }
  opt_view_scale_type(num, GMSH_GUI, 0);
  opt_view_saturate_values(num, GMSH_GUI, 0);

  // intervals
  opt_view_nb_iso(num, GMSH_GUI, 0);
  opt_view_intervals_type(num, GMSH_GUI, 0);
  opt_view_boundary(num, GMSH_GUI, 0);
  opt_view_explode(num, GMSH_GUI, 0);

  // offset/raise
  opt_view_offset0(num, GMSH_GUI, 0);
  opt_view_offset1(num, GMSH_GUI, 0);
  opt_view_offset2(num, GMSH_GUI, 0);
  opt_view_raise0(num, GMSH_GUI, 0);
  opt_view_raise1(num, GMSH_GUI, 0);
  opt_view_raise2(num, GMSH_GUI, 0);
  val = 10.*CTX.lc ;
  for(i=3 ; i<9 ; i++){
    view_value[i]->step(val,1000); 
    view_value[i]->minimum(-val); 
    view_value[i]->maximum(val); 
  }

  // timestep
  if(v->NbTimeStep==1) view_timestep->deactivate();
  else view_timestep->activate();
  view_value[9]->callback(view_options_timestep_cb, (void*)num);
  view_value[9]->maximum(v->NbTimeStep-1); 
  opt_view_timestep(num, GMSH_GUI, 0);

  // vector
  if(v->ScalarOnly) view_vector->deactivate();
  else view_vector->activate();
  opt_view_arrow_type(num, GMSH_GUI, 0);
  opt_view_arrow_scale(num, GMSH_GUI, 0);
  opt_view_arrow_location(num, GMSH_GUI, 0);

  // colors
  view_colorbar_window->update(v->Name, v->Min, v->Max, &v->CT, &v->Changed);

  // light
  opt_view_light(num, GMSH_GUI, 0);
  opt_view_smooth_normals(num, GMSH_GUI, 0);
  opt_view_angle_smooth_normals(num, GMSH_GUI, 0);

  // OK
  view_ok->callback(view_options_ok_cb, (void*)num);

}

//*************** Create the window for geometry context dependant definitions *********

void GUI::create_geometry_context_window(int num){
  static Fl_Group *g[10];
  int i;

  if(!init_geometry_context_window){
    init_geometry_context_window = 1 ;

    int width = 31*CTX.fontsize;
    int height = 5*WB+9*BH ;
    
    context_geometry_window = new Fl_Window(width,height);
    context_geometry_window->box(WINDOW_BOX);
    context_geometry_window->label("Contextual geometry definitions");
    { 
      Fl_Tabs* o = new Fl_Tabs(WB, WB, width-2*WB, height-3*WB-BH);
      // 0: Parameter
      { 
	g[0] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Parameter");
	g[0]->labelsize(CTX.fontsize);
	context_geometry_input[0] = new Fl_Input (2*WB, 2*WB+1*BH, IW, BH, "Name");
	context_geometry_input[1] = new Fl_Input (2*WB, 2*WB+2*BH, IW, BH, "Value");
	for(i=0 ; i<2 ; i++){
	  context_geometry_input[i]->labelsize(CTX.fontsize);
	  context_geometry_input[i]->textsize(CTX.fontsize);
	  context_geometry_input[i]->align(FL_ALIGN_RIGHT);
	}
	{ 
	  Fl_Return_Button* o = new Fl_Return_Button(width-BB-2*WB, 2*WB+7*BH, BB, BH, "Add");
	  o->labelsize(CTX.fontsize);
	  o->callback(con_geometry_define_parameter_cb);
	}
        g[0]->end();
      }
      // 1: Point
      { 
	g[1] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Point");
	g[1]->labelsize(CTX.fontsize);
	context_geometry_input[2] = new Fl_Input (2*WB, 2*WB+1*BH, IW, BH, "X coordinate");
	context_geometry_input[3] = new Fl_Input (2*WB, 2*WB+2*BH, IW, BH, "Y coordinate");
	context_geometry_input[4] = new Fl_Input (2*WB, 2*WB+3*BH, IW, BH, "Z coordinate");
	context_geometry_input[5] = new Fl_Input (2*WB, 2*WB+4*BH, IW, BH, "Characteristic length");
	for(i=2 ; i<6 ; i++){
	  context_geometry_input[i]->labelsize(CTX.fontsize);
	  context_geometry_input[i]->textsize(CTX.fontsize);
	  context_geometry_input[i]->align(FL_ALIGN_RIGHT);
	}
	{ 
	  Fl_Return_Button* o = new Fl_Return_Button(width-BB-2*WB, 2*WB+7*BH, BB, BH, "Add");
	  o->labelsize(CTX.fontsize);
	  o->callback(con_geometry_define_point_cb);
	}
        g[1]->end();
      }
      // 2: Translation
      { 
	g[2] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Translation");
	g[2]->labelsize(CTX.fontsize);
	context_geometry_input[6] = new Fl_Input (2*WB, 2*WB+1*BH, IW, BH, "X component");
	context_geometry_input[7] = new Fl_Input (2*WB, 2*WB+2*BH, IW, BH, "Y component");
	context_geometry_input[8] = new Fl_Input (2*WB, 2*WB+3*BH, IW, BH, "Z component");
	for(i=6 ; i<9 ; i++){
	  context_geometry_input[i]->labelsize(CTX.fontsize);
	  context_geometry_input[i]->textsize(CTX.fontsize);
	  context_geometry_input[i]->align(FL_ALIGN_RIGHT);
	}
	{ 
	  Fl_Return_Button* o = new Fl_Return_Button(width-BB-2*WB, 2*WB+7*BH, BB, BH, "Set");
	  o->labelsize(CTX.fontsize);
	  o->callback(con_geometry_define_translation_cb);
	}
        g[2]->end();
      }
      // 3: Rotation
      { 
	g[3] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Rotation");
	g[3]->labelsize(CTX.fontsize);
	context_geometry_input[9]  = new Fl_Input (2*WB, 2*WB+1*BH, IW, BH, "X coordinate of an axis point");
	context_geometry_input[10] = new Fl_Input (2*WB, 2*WB+2*BH, IW, BH, "Y coordinate of an axis point");
	context_geometry_input[11] = new Fl_Input (2*WB, 2*WB+3*BH, IW, BH, "Z coordinate of an axis point");
	context_geometry_input[12] = new Fl_Input (2*WB, 2*WB+4*BH, IW, BH, "X component of direction");
	context_geometry_input[13] = new Fl_Input (2*WB, 2*WB+5*BH, IW, BH, "Y component of direction");
	context_geometry_input[14] = new Fl_Input (2*WB, 2*WB+6*BH, IW, BH, "Z component of direction");
	context_geometry_input[15] = new Fl_Input (2*WB, 2*WB+7*BH, IW, BH, "Angle in radians");
	for(i=9 ; i<16 ; i++){
	  context_geometry_input[i]->labelsize(CTX.fontsize);
	  context_geometry_input[i]->textsize(CTX.fontsize);
	  context_geometry_input[i]->align(FL_ALIGN_RIGHT);
	}
	{ 
	  Fl_Return_Button* o = new Fl_Return_Button(width-BB-2*WB, 2*WB+7*BH, BB, BH, "Set");
	  o->labelsize(CTX.fontsize);
	  o->callback(con_geometry_define_rotation_cb);
	}
        g[3]->end();
      }
      // 4: Scale
      { 
	g[4] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Scale");
	g[4]->labelsize(CTX.fontsize);
	context_geometry_input[16] = new Fl_Input (2*WB, 2*WB+1*BH, IW, BH, "X component of direction");
	context_geometry_input[17] = new Fl_Input (2*WB, 2*WB+2*BH, IW, BH, "Y component of direction");
	context_geometry_input[18] = new Fl_Input (2*WB, 2*WB+3*BH, IW, BH, "Z component of direction");
	context_geometry_input[19] = new Fl_Input (2*WB, 2*WB+4*BH, IW, BH, "Factor");
	for(i=16 ; i<20 ; i++){
	  context_geometry_input[i]->labelsize(CTX.fontsize);
	  context_geometry_input[i]->textsize(CTX.fontsize);
	  context_geometry_input[i]->align(FL_ALIGN_RIGHT);
	}
	{ 
	  Fl_Return_Button* o = new Fl_Return_Button(width-BB-2*WB, 2*WB+7*BH, BB, BH, "Set");
	  o->labelsize(CTX.fontsize);
	  o->callback(con_geometry_define_scale_cb);
	}
        g[4]->end();
      }
      // 5: Symmetry
      { 
	g[5] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Symmetry");
	g[5]->labelsize(CTX.fontsize);
	context_geometry_input[20] = new Fl_Input (2*WB, 2*WB+1*BH, IW, BH, "1st plane equation coefficient");
	context_geometry_input[21] = new Fl_Input (2*WB, 2*WB+2*BH, IW, BH, "2nd plane equation coefficient");
	context_geometry_input[22] = new Fl_Input (2*WB, 2*WB+3*BH, IW, BH, "3rd plane equation coefficient");
	context_geometry_input[23] = new Fl_Input (2*WB, 2*WB+4*BH, IW, BH, "4th plane equation coefficient");
	for(i=20 ; i<24 ; i++){
	  context_geometry_input[i]->labelsize(CTX.fontsize);
	  context_geometry_input[i]->textsize(CTX.fontsize);
	  context_geometry_input[i]->align(FL_ALIGN_RIGHT);
	}
	{ 
	  Fl_Return_Button* o = new Fl_Return_Button(width-BB-2*WB, 2*WB+7*BH, BB, BH, "Set");
	  o->labelsize(CTX.fontsize);
	  o->callback(con_geometry_define_symmetry_cb);
	}
        g[5]->end();
      }
      o->end();
    }

    { 
      Fl_Button* o = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Cancel");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)context_geometry_window);
    }

    for(i=0 ; i<6 ; i++) g[i]->hide();
    g[num]->show();

    if(CTX.center_windows)
      context_geometry_window->position(m_window->x()+m_window->w()/2-width/2,
					m_window->y()+9*BH-height/2);
    context_geometry_window->end();
    context_geometry_window->show();
  }
  else{
    if(context_geometry_window->shown()){
      for(i=0 ; i<6 ; i++) g[i]->hide();
      g[num]->show();
    }
    else{
      for(i=0 ; i<6 ; i++) g[i]->hide();
      g[num]->show();
      context_geometry_window->show();
    }
    
  }

}

//************** Create the window for mesh context dependant definitions **************

void GUI::create_mesh_context_window(int num){
  static Fl_Group *g[10];
  int i;

  if(!init_mesh_context_window){
    init_mesh_context_window = 1 ;

    int width = 31*CTX.fontsize;
    int height = 5*WB+5*BH ;
    
    context_mesh_window = new Fl_Window(width,height);
    context_mesh_window->box(WINDOW_BOX);
    context_mesh_window->label("Contextual mesh definitions");
    { 
      Fl_Tabs* o = new Fl_Tabs(WB, WB, width-2*WB, height-3*WB-BH);
      // 0: Characteristic length
      { 
	g[0] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Characteristic length");
	g[0]->labelsize(CTX.fontsize);
	context_mesh_input[0] = new Fl_Input (2*WB, 2*WB+1*BH, IW, BH, "Value");
	context_mesh_input[0]->labelsize(CTX.fontsize);
	context_mesh_input[0]->textsize(CTX.fontsize);
	context_mesh_input[0]->align(FL_ALIGN_RIGHT);
	{ 
	  Fl_Return_Button* o = new Fl_Return_Button(width-BB-2*WB, 2*WB+3*BH, BB, BH, "Set");
	  o->labelsize(CTX.fontsize);
	  o->callback(con_mesh_define_length_cb);
	}
        g[0]->end();
      }
      // 1: Transfinite line
      { 
	g[1] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Transfinite line");
	g[1]->labelsize(CTX.fontsize);
	context_mesh_input[1] = new Fl_Input (2*WB, 2*WB+1*BH, IW, BH, "Number of points");
	context_mesh_input[2] = new Fl_Input (2*WB, 2*WB+2*BH, IW, BH, "Distribution");
	for(i=1 ; i<3 ; i++){
	  context_mesh_input[i]->labelsize(CTX.fontsize);
	  context_mesh_input[i]->textsize(CTX.fontsize);
	  context_mesh_input[i]->align(FL_ALIGN_RIGHT);
	}
	{ 
	  Fl_Return_Button* o = new Fl_Return_Button(width-BB-2*WB, 2*WB+3*BH, BB, BH, "Set");
	  o->labelsize(CTX.fontsize);
	  o->callback(con_mesh_define_transfinite_line_cb);
	}
        g[1]->end();
      }
      // 2: Transfinite volume
      { 
	g[2] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Transfinite volume");
	g[2]->labelsize(CTX.fontsize);
	context_mesh_input[3] = new Fl_Input (2*WB, 2*WB+1*BH, IW, BH, "Volume number");
	context_mesh_input[3]->labelsize(CTX.fontsize);
	context_mesh_input[3]->textsize(CTX.fontsize);
	context_mesh_input[3]->align(FL_ALIGN_RIGHT);
	{ 
	  Fl_Return_Button* o = new Fl_Return_Button(width-BB-2*WB, 2*WB+3*BH, BB, BH, "Set");
	  o->labelsize(CTX.fontsize);
	  o->callback(con_mesh_define_transfinite_line_cb);
	}
        g[2]->end();
      }
      o->end();
    }

    { 
      Fl_Button* o = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Cancel");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)context_mesh_window);
    }

    for(i=0 ; i<3 ; i++) g[i]->hide();
    g[num]->show();

    if(CTX.center_windows)
      context_mesh_window->position(m_window->x()+m_window->w()/2-width/2,
				    m_window->y()+9*BH-height/2);
    context_mesh_window->end();
    context_mesh_window->show();
  }
  else{
    if(context_mesh_window->shown()){
      for(i=0 ; i<3 ; i++) g[i]->hide();
      g[num]->show();
    }
    else{
      for(i=0 ; i<3 ; i++) g[i]->hide();
      g[num]->show();
      context_mesh_window->show();
    }
    
  }
}


//************** Create the window for the getdp solver **************

void GUI::create_getdp_window(){
  int i;
  static Fl_Group *g[10];

  int LL = (int)(1.75*IW);

  if(!init_getdp_window){
    init_getdp_window = 1 ;

    int width = 5*BB+6*WB;
    int height = 10*WB+ 8*BH  ;
    
    getdp_window = new Fl_Window(width,height);
    getdp_window->box(WINDOW_BOX);
    getdp_window->label("GetDP solver");
    { 
      Fl_Tabs* o = new Fl_Tabs(WB, WB, width-2*WB, height-3*WB-1*BH);
      { 
	g[0] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "General");
	g[0]->labelsize(CTX.fontsize);

	getdp_input[0] = new Fl_Input(2*WB, 2*WB+1*BH, LL, BH, "Problem");
	Fl_Button *b1 = new Fl_Button(2*WB, 3*WB+2*BH, BB, BH, "Choose");
	b1->callback(getdp_file_open_cb);
	b1->labelsize(CTX.fontsize);
	Fl_Button *b2 = new Fl_Button(3*WB+BB, 3*WB+2*BH, BB, BH, "Edit");
	b2->callback(getdp_file_edit_cb);
	b2->labelsize(CTX.fontsize);

	getdp_choice[0] = new Fl_Choice(2*WB, 4*WB+3*BH, LL, BH,"Resolution");
	getdp_choice[1] = new Fl_Choice(2*WB, 5*WB+4*BH, LL, BH,"Post operation");

	getdp_input[1] = new Fl_Input(2*WB, 6*WB+5*BH, LL, BH, "Mesh");
	Fl_Button *b3 = new Fl_Button(2*WB, 7*WB+6*BH, BB, BH, "Choose");
	b3->callback(getdp_choose_mesh_cb);
	b3->labelsize(CTX.fontsize);

	for(i=0 ; i<2 ; i++){
	  getdp_input[i]->labelsize(CTX.fontsize);
	  getdp_input[i]->textsize(CTX.fontsize);
	  getdp_input[i]->align(FL_ALIGN_RIGHT);
	}
	for(i=0 ; i<2 ; i++){
	  getdp_choice[i]->textsize(CTX.fontsize);
	  getdp_choice[i]->labelsize(CTX.fontsize);
	  getdp_choice[i]->align(FL_ALIGN_RIGHT);
	}

        g[0]->end();
      }
      { 
	g[1] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "Options");
	g[1]->labelsize(CTX.fontsize);

	getdp_input[2] = new Fl_Input(2*WB, 2*WB+1*BH, LL, BH, "Command");
	Fl_Button *b = new Fl_Button(2*WB, 3*WB+2*BH, BB, BH, "Choose");
	b->callback(getdp_choose_command_cb);
	b->labelsize(CTX.fontsize);
	
	getdp_butt[0] = new Fl_Check_Button(2*WB, 4*WB+3*BH, LL, BH, 
					    "Automatic message display");
	getdp_butt[1] = new Fl_Check_Button(2*WB, 4*WB+4*BH, LL, BH, 
					    "Automatic view merge");

	getdp_input[2]->labelsize(CTX.fontsize);
	getdp_input[2]->textsize(CTX.fontsize);
	getdp_input[2]->align(FL_ALIGN_RIGHT);
	for(i=0 ; i<2 ; i++){
	  getdp_butt[i]->type(FL_TOGGLE_BUTTON);
	  getdp_butt[i]->down_box(FL_DOWN_BOX);
	  getdp_butt[i]->labelsize(CTX.fontsize);
	  getdp_butt[i]->selection_color(FL_YELLOW);
	}

	Fl_Return_Button* o = new Fl_Return_Button(width-BB-2*WB, 2*WB+7*BH, BB, BH, "OK");
	o->labelsize(CTX.fontsize);
	o->callback(getdp_ok_cb);

        g[1]->end();
      }
      { 
	g[2] = new Fl_Group(WB, WB+BH, width-2*WB, height-3*WB-2*BH, "About");
	g[2]->labelsize(CTX.fontsize);

	Fl_Browser *o = new Fl_Browser(2*WB, 2*WB+1*BH, width-4*WB, height-5*WB-2*BH);
	o->add("");
	o->add("@c@b@.GetDP");
	o->add("@c@.A General environment for the treatment");
	o->add("@c@.of Discrete Problems");
	o->add("");
	o->add("@c@.Experimental solver plugin for Gmsh");
	o->add("");
	o->add("@c@.Visit http://www.geuz.org/getdp/ for more info");
	o->textsize(CTX.fontsize);

        g[2]->end();
      }
      o->end();
    }

    { 
      Fl_Button* o = new Fl_Button(width-5*BB-5*WB, height-BH-WB, BB, BH, "Pre");
      o->labelsize(CTX.fontsize);
      o->callback(getdp_pre_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-4*BB-4*WB, height-BH-WB, BB, BH, "Cal");
      o->labelsize(CTX.fontsize);
      o->callback(getdp_cal_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-3*BB-3*WB, height-BH-WB, BB, BH, "Post");
      o->labelsize(CTX.fontsize);
      o->callback(getdp_post_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-2*BB-2*WB, height-BH-WB, BB, BH, "Kill");
      o->labelsize(CTX.fontsize);
      o->callback(getdp_kill_cb);
    }
    { 
      Fl_Button* o = new Fl_Button(width-BB-WB, height-BH-WB, BB, BH, "Cancel");
      o->labelsize(CTX.fontsize);
      o->callback(cancel_cb, (void*)getdp_window);
    }


    if(CTX.center_windows)
      getdp_window->position(m_window->x()+m_window->w()/2-width/2,
			     m_window->y()+9*BH-height/2);
    getdp_window->end();
  }
  else{
    if(getdp_window->shown())
      getdp_window->redraw();
    else
      getdp_window->show();
  }
}

