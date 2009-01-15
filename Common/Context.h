// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <vector>
#include <string>
#include "CGNSOptions.h"
#include "meshPartitionOptions.h"

// The interface-independent context.

class Context_T {
 public :
  // general options
  std::vector<std::string> files; // all the files on the command line
  std::string bgm_filename; // background mesh
  std::string output_filename; // output file specified with command line option '-o'
  const char *default_filename;
  const char *tmp_filename;
  const char *session_filename;
  const char *options_filename;
  const char *error_filename;
  char statreport[256]; // mesh stat output file
  int create_append_statreport; // do nothing 0 create 1 append 2 
  int session_save, options_save; // save session/option file on exit
  int confirm_overwrite; // confirm overwrite when file->save as
  const char *display; // forced display host:0.0 under X11 
  int terminal; // show we print to the terminal console?
  int num_windows, num_tiles; // number of graphical windows/tiles to create
  const char *editor; // text editor command (with included '%s')
  const char *web_browser; // web browser command (with included '%s')
  std::string home_dir; // the home directory
  const char *gui_theme; // FLTK GUI theme
  int tooltips; // show tootips in GUI?
  int position[2]; // position of the menu window on the screen
  int gl_position[2]; // position of the graphic window on the screen
  int gl_size[2]; // size of the graphic window on the screen
  int msg_position[2]; // position of the message window on the screen
  int msg_size[2]; // size of the message window on the screen
  int msg_auto_scroll; // scroll automatically to last message
  int opt_position[2]; // position of the option window on the screen
  int vis_position[2]; // position of the visibility window on the screen
  int clip_position[2]; // position of the clipping planes window on the screen
  int manip_position[2]; // position of the manipulator window on the screen
  int stat_position[2]; // position of the statistics window on the screen
  int ctx_position[2]; // position of the geo/mesh context windows on the screen
  int solver_position[2]; // position of the solver windows on the screen
  int plugin_position[2]; // position of the plugin window on the screen
  int plugin_size[2]; // size of the plugin window on the screen
  int field_position[2]; // position of the field window on the screen
  int field_size[2]; // size of the field window on the screen
  int file_chooser_position[2]; // position of the file chooser window on the screen
  int system_menu_bar; // use the system menu bar on MacOS?
  int batch; // -3=server daemon; -2=check coherence; -1=write geo; 0=full gfx;
             // 1=1D mesh; 2=2D mesh; 3=3D mesh; 4=adapt mesh; 5=refine mesh;
  int batch_after_mesh; // 1=partition mesh
  int initial_context; // 0=automatic; 1=geom; 2=mesh; 3=solver; 4=post 
  int nopopup; // never popup dialogs in scripts (use default values instead)
  int non_modal_windows; // make all windows "non modal"
  double clip_factor; // clipping plane distance factor
  int useTrackball; // do or do not use the trackball for rotations 
  double rotation_center[3]; // point around which to rotate the scene
  int rotation_center_cg; // rotate around the center of mass instead of rotation_center[]
  double min[3], max[3]; // "overall" x, y and z min used for drawing and lc computation
  double cg[3]; // "center of mass" of the current geometry, used for graphics only
  double lc; // characteristic length for the whole problem (never
             // used in mesh generation ->only for geo/post)
  int db, antialiasing; // double buffer? antialiasing?
  int ortho; // orthogonal projection? 
  int draw_bbox, draw_rotation_center; // draw the bounding boxes and the rot center?
  int fast_redraw; // draw simplified model during user interaction
  int small_axes, small_axes_size, small_axes_pos[2]; // small axes
  int axes, axes_auto_position, axes_tics[3],axes_mikado; // large axes
  double axes_position[6];
  char axes_label[3][256], axes_format[3][256];
  int threads, threads_lock; // threads?, lock (should be a mutex...) 
  int alpha; // enable alpha blending 
  double zoom_factor; // mouse2 zoom coefficient
  int bg_gradient; // draw background gradient?
  int fontsize, deltafontsize; // fltk font size (and delta for palettes)
  const char *gl_font; // font for opengl graphics
  int gl_font_enum; // font for opengl graphics (fltk enum value)
  int gl_fontsize; // font size for opengl graphics
  double point_size, line_width; // point/line widths
  int light[6]; // status of lights
  double light_position[6][4]; // light sources positions 
  double shine, shine_exponent; // material specular reflection parameters
  double clip_plane[6][4]; // clipping planes 
  int clip_whole_elements, clip_only_draw_intersecting_volume; // clipping options
  int clip_only_volume;
  int polygon_offset, polygon_offset_always; // polygon offset control
  double polygon_offset_factor, polygon_offset_units; // params for glPolygonOffset
  int color_scheme; // general color scheme
  int quadric_subdivisions; // nb of subdivisions for gluQuadrics (spheres/cylinders)
  int vector_type; // default vector display type (for normals, etc.)
  double arrow_rel_head_radius; // relative radius of arrow head
  double arrow_rel_stem_radius; // relative radius of arrow stem
  double arrow_rel_stem_length; // relative length of arrow stem
  double mesh_timer[3]; // records cpu times for 1-D, 2-D and 3-D mesh generation
  int forced_bbox; // dynamic variable tracking if the bbox is currently imposed
  int mouse_selection; // enable selection using the mouse
  int mouse_hover_meshes; // enable mouse hover on meshes
  int pick_elements; // allow individual element picking
  int expert_mode; // to disable some warnings for beginners
  int printing; // dynamic: equal to 1 while gmsh is printing
  int hide_unselected; // hide all unselected entities

  // these are used as temp vars until the GUI is ready
  double tmp_r[3]; // current Euler angles (in degrees!) 
  double tmp_t[3], tmp_s[3]; // current translation and scale 
  double tmp_quaternion[4]; // current quaternion used for "trackball" rotation

  // geometry options 
  struct{
    int points, lines, surfaces, volumes;
    int points_num, lines_num, surfaces_num, volumes_num;
    double point_size, line_width, point_sel_size, line_sel_width;
    int point_type, line_type, surface_type;
    int draw, light, light_two_side;
    int old_circle, num_sub_edges;
    int extrude_spline_points, extrude_return_lateral, old_newreg;
    double normals, tangents;
    double scaling_factor;
    int auto_coherence, highlight_orphans;
    double tolerance;
    double snap[3];
    int occ_fix_small_edges, occ_fix_small_faces, occ_sew_faces;
    int clip; // status of clip planes (bit array)
    int use_transform; // transform display coordinates? (0=no, 1=scale, 2=...)
    double transform[3][3], offset[3]; // transformation matrix for geo/mesh drawing
  } geom;

  // mesh options 
  struct {
    double msh_file_version;
    int changed, draw;
    int points, lines, triangles, quadrangles, tetrahedra, hexahedra, prisms, pyramids;
    int surfaces_edges, surfaces_faces, volumes_edges, volumes_faces;
    int points_num, lines_num, surfaces_num, volumes_num;
    int num_sub_edges;
    double label_frequency;
    int point_type; // flat or 3D
    double point_size, line_width;
    int optimize, optimize_netgen, refine_steps;
    int quality_type, label_type;
    double quality_inf, quality_sup, radius_inf, radius_sup;
    double scaling_factor, lc_factor, rand_factor, lc_integration_precision;
    double lc_min, lc_max, tolerance_edge_length;
    int lc_from_points, lc_from_curvature, lc_extend_from_boundary;
    int dual, voronoi, draw_skin_only;
    int light, light_two_side, light_lines;
    int format, nb_smoothing, algo2d, algo3d, algo_subdivide;
    int order, second_order_linear, second_order_incomplete;
    int second_order_experimental, mesh_only_visible;
    int smooth_internal_edges, c1_continuity;
    int min_circ_points, min_curv_points;
    double normals, tangents, explode;
    int color_carousel;
    int save_all, save_groups_of_nodes, binary, bdf_field_format, save_parametric;
    int smooth_normals, reverse_all_normals;
    double angle_smooth_normals;
    double allow_swap_edge_angle;
    int zone_definition;
    meshPartitionOptions partition_options;
    CGNSOptions cgns_options;
    int clip; // status of clip planes (bit array)
  } mesh;

  // post processing options 
  struct{
    int draw, link, horizontal_scales ;
    int smooth, anim_cycle, combine_time, combine_remove_orig ;
    int file_format, plugins;
    double anim_delay ;
    void (*plugin_draw_function)(void *context) ;
  }post;

  // solver options 
  struct{
    int max_delay, plugins, listen ;
    const char *socket_name ;
  }solver;

  // print options 
  struct{
    int format;
    int eps_quality, eps_background, eps_compress, eps_ps3shading;
    int eps_occlusion_culling, eps_best_root;
    double eps_line_width_factor, eps_point_size_factor;
    int jpeg_quality, jpeg_smoothing;
    int gif_dither, gif_sort, gif_interlace, gif_transparent;
    int geo_labels;
    int pos_elementary, pos_element, pos_gamma, pos_eta, pos_rho,pos_disto;
    int text, tex_as_equation;
  } print;

  // color options
  struct{
    unsigned int bg, bg_grad, fg, text, axes, small_axes;
    unsigned int ambient_light[6], diffuse_light[6], specular_light[6];
    struct{
      unsigned int point, line, surface, volume;
      unsigned int selection, highlight[3], projection;
      unsigned int tangents, normals;
    } geom;
    struct{
      unsigned int vertex, vertex_sup, line, triangle, quadrangle;
      unsigned int tetrahedron, hexahedron, prism, pyramid;
      unsigned int carousel[20];
      unsigned int tangents, normals;
    } mesh;
  } color;
  
  int big_endian; // is the machine big-endian?

  // how RGBA values are packed and unpacked into/from an unsigned
  // integer to be fed to glColor4ubv (depends on machine byte
  // ordering!):
  inline unsigned int PACK_COLOR(int R, int G, int B, int A){
    if(big_endian) return ( (unsigned int)((R)<<24 | (G)<<16 | (B)<<8 | (A)) );
    else           return ( (unsigned int)((A)<<24 | (B)<<16 | (G)<<8 | (R)) );
  }
  inline int UNPACK_RED(unsigned int X){
    if(big_endian) return ( ( (X) >> 24 ) & 0xff );
    else           return ( (X) & 0xff );
  }
  inline int UNPACK_GREEN(unsigned int X){
    if(big_endian) return ( ( (X) >> 16 ) & 0xff );
    else           return ( ( (X) >> 8 ) & 0xff );
  }
  inline int UNPACK_BLUE(unsigned int X){
    if(big_endian) return ( ( (X) >> 8 ) & 0xff );
    else           return ( ( (X) >> 16 ) & 0xff );
  }
  inline int UNPACK_ALPHA(unsigned int X){
    if(big_endian) return ( (X) & 0xff );
    else           return ( ( (X) >> 24 ) & 0xff );
  }
};

#endif
