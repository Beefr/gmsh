// $Id: Plugin.cpp,v 1.23 2002-03-26 22:11:33 geuzaine Exp $

#include <map>
#ifndef _NODLL
#include <dlfcn.h>
#endif

#include <map>

#ifdef _FLTK
#include <FL/Fl.H>
#include <FL/filename.H>
#endif

#include "Plugin.h"
#include "PluginManager.h"

#include "CutMap.h"
#include "CutPlane.h"
#include "CutSphere.h"
#include "Skin.h"
#include "Smooth.h"
#include "Transform.h"
#include "Triangulate.h"

using namespace std;

const char *GMSH_PluginEntry = "GMSH_RegisterPlugin";

#if defined(WIN32) && !defined(__CYGWIN__)
#define SLASH "\\"
#else
#define SLASH "/"
#endif


GMSH_PluginManager *GMSH_PluginManager::instance = 0;

GMSH_PluginManager::GMSH_PluginManager(){
}

GMSH_PluginManager::~GMSH_PluginManager(){
  for(iter it = allPlugins.begin();
      it != allPlugins.end();
      ++it) delete (*it).second;
}

GMSH_Plugin* GMSH_PluginManager::find (char *pluginName){
  iter it = allPlugins.find(pluginName);
  if (it == allPlugins.end()) return 0;
  return (*it).second;
}

void GMSH_PluginManager::Action( char *pluginName, char *action, void *data){
  GMSH_Plugin * plugin = find(pluginName);
  if(!plugin){
    throw 1;
  }
  if(!strcmp(action,"Run")){      
    plugin->Run();
  }
  else if(!strcmp(action,"Save")){
    plugin->Save();
  }
  else{
    throw 1;
  }
}

void GMSH_PluginManager::SetPluginOption (char *pluginName, char *option, char *value){
  GMSH_Plugin *plugin = find(pluginName);

  if(!plugin) throw "Unknown plugin name" ;

  if(!strcmp(option,"OutputFileName"))
    strcpy(plugin->OutputFileName,value);
  else if(!strcmp(option,"InputFileName"))
    strcpy(plugin->InputFileName,value);
  else
    throw "Unknown plugin option name";
}

void GMSH_PluginManager::SetPluginOption (char *pluginName, char *option, double value){
  GMSH_Plugin *plugin = find(pluginName);

  if(!plugin) throw "Unknown plugin name";

  for(int i=0 ; i<plugin->getNbOptions();i++){
    StringXNumber *sxn;
    // get the ith option of the plugin
    sxn = plugin->GetOption(i);
    // look if it's the good option name
    if(!strcmp (sxn->str,option)){
      sxn->def = value;
      return;
    }
  }
  throw "Unknown plugin option name";
}

GMSH_PluginManager* GMSH_PluginManager::Instance(){
  if(!instance){
    instance = new GMSH_PluginManager;
  }
  return instance;
}

void GMSH_PluginManager::RegisterDefaultPlugins(){
  struct dirent **list;
  char ext[6];

  allPlugins.insert(std::pair<char*,GMSH_Plugin*>("CutMap",
						  GMSH_RegisterCutMapPlugin()));
  allPlugins.insert(std::pair<char*,GMSH_Plugin*>("CutPlane",
						  GMSH_RegisterCutPlanePlugin()));
  allPlugins.insert(std::pair<char*,GMSH_Plugin*>("CutSphere",
						  GMSH_RegisterCutSpherePlugin()));
  allPlugins.insert(std::pair<char*,GMSH_Plugin*>("Skin",
						  GMSH_RegisterSkinPlugin()));
  allPlugins.insert(std::pair<char*,GMSH_Plugin*>("Smooth",
						  GMSH_RegisterSmoothPlugin()));
  allPlugins.insert(std::pair<char*,GMSH_Plugin*>("Transform",
						  GMSH_RegisterTransformPlugin()));
  allPlugins.insert(std::pair<char*,GMSH_Plugin*>("Triangulate",
						  GMSH_RegisterTriangulatePlugin()));

#ifdef _FLTK
  char *homeplugins = getenv ("GMSHPLUGINSHOME");
  if(!homeplugins)return;
#if (FL_MAJOR_VERSION == 1) && (FL_MINOR_VERSION == 1)
  int nbFiles = fl_filename_list(homeplugins,&list);
#else
  int nbFiles = filename_list(homeplugins,&list);
#endif
  if(nbFiles <= 0)  return;
  for(int i=0;i<nbFiles;i++){
    char *name = list[i]->d_name;
    if(strlen(name) > 3){
      strcpy(ext,name+(strlen(name)-3));
      if(!strcmp(ext,".so") || !strcmp(ext,"dll")){
	AddPlugin(homeplugins,name);
      }
    }
  }
  for(int i=0;i<nbFiles;i++)free(list[i]);
  free (list);
#endif
}

void GMSH_PluginManager::AddPlugin( char *dirName, char *pluginName){

#if ( defined(WIN32) && !defined(__CYGWIN__) ) || defined(_NODLL) || !defined(_FLTK)
  Msg(WARNING,"No dynamic plugin loading on this platform");
  return;
#else
  char dynamic_lib[1024];
  char plugin_name[256];
  char plugin_author[256];
  char plugin_copyright[256];
  char plugin_help[256];
  class GMSH_Plugin* (*RegisterPlugin)(void);
  sprintf(dynamic_lib,"%s%s%s",dirName,SLASH,pluginName);
  Msg(INFO,"Opening Plugin '%s'",dynamic_lib);
  void *hlib = dlopen (dynamic_lib,RTLD_NOW);
  char *err = dlerror();
  if(hlib == NULL){
    Msg(WARNING,"Error in opening %s (dlerror = %s)",dynamic_lib,err);
    return;
  }
  RegisterPlugin = (class GMSH_Plugin* (*)(void)) dlsym(hlib,GMSH_PluginEntry);
  err = dlerror();
  if(err != NULL){
    Msg(WARNING,"Symbol '%s' missing in plugin '%s' (dlerror = %s)",
	GMSH_PluginEntry,pluginName,err);
    return;
  }

  GMSH_Plugin *p = RegisterPlugin();
  p->hlib = hlib;
  p->getName(plugin_name);
  p->getInfos(plugin_author,plugin_copyright,plugin_help);
  if(allPlugins.find(plugin_name) != allPlugins.end()){
    Msg(WARNING,"Plugin '%s' multiply defined",pluginName);
    return;
  }
  allPlugins.insert(std::pair<char*,GMSH_Plugin*>(plugin_name,p));
  Msg(INFO,"Plugin name      : %s",plugin_name);
  Msg(INFO,"Plugin author    : %s",plugin_author);
  Msg(INFO,"Plugin copyright : %s",plugin_copyright);
  Msg(INFO,"Plugin help      : %s",plugin_help);
#endif
}



