#ifndef _PLUGIN_H_
#define _PLUGIN_H_
/*
  The one who intend to create a plugin for gmsh have to 
    -) Create a dynamin lib (.so) containing 1 symbols
       GMSH_Plugin * GMSH_RegisterPlugin ();
    -) When there is an unacceptable error in the plugin,
    just throw this, the plugin manager will be able to 
    catch the exception.
*/

class PluginContainer;
class Post_View;

class GMSH_Plugin
{
public :
  /*this is there for internal use, this variable will be
   used by the PluginManager, just forget it*/
  void *hlib;
  /* 3 kind of plugins, one for cad, one for mesh, one for postpro*/
  typedef enum GMSH_PLUGIN_TYPE {GMSH_CAD_PLUGIN, GMSH_MESH_PLUGIN, GMSH_POST_PLUGIN};
  /* returns the type of plugin for downcasting GMSH_Plugin into
     GMSH_CAD_Plugin, GMSH_Mesh_Plugin and GMSH_Post_Plugin */
  virtual GMSH_PLUGIN_TYPE getType() const = 0;
  virtual void getName (char *name) const = 0;
  virtual void getInfos (char *author, 
			 char *copyright,
			 char *help_text) const = 0;
  /* When an error is thrown by the plugin, the plugin manager
     will show the message and hopefully continue */
  virtual void CatchErrorMessage (char *errorMessage) const = 0;
  /* gmsh style option, ca be loaded, saved and set*/
  virtual void SetOption (char *optionName, void *optionValue) = 0;
  virtual int getNbOptions() const = 0;
  virtual void GetOption (int iopt, char *optionName, void *optionValue) const = 0;  
};

/* Base class for Post-Processing Plugins
   The user can either modify or duplicate 
   a Post_View */
class GMSH_Post_Plugin : public GMSH_Plugin
{
public:
  GMSH_PLUGIN_TYPE getType() const {return GMSH_Plugin::GMSH_POST_PLUGIN;}
  /* If returned pointer is the same as the argument, then view is simply modified,
    else, a new view is added in the view list */
  virtual Post_View *execute (Post_View *) = 0;
};

class GMSH_PluginManager
{
  GMSH_PluginManager();
  ~GMSH_PluginManager();
  static GMSH_PluginManager *instance;
  PluginContainer* allPlugins;
public :
/**
  Registering all default plugins that are in $(GMSHPLUGINSHOME)
  In fact, we will load all .so files in dir $(GMSHPLUGINSHOME)
*/
  void RegisterDefaultPlugins();
  static GMSH_PluginManager *Instance();
  /** Dynamically add a plugin pluginName.so in dirName*/
  void AddPlugin(char *dirName, char *pluginName);
  void CallPlugin (char *name);
  void DestroyPlugin (char *name);
  void SetPluginOption (char *pluginName, char *option, void *value);
};
#endif




