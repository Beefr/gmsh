#ifndef _SKIN_H_
#define _SKIN_H

extern "C"
{
  GMSH_Plugin *GMSH_RegisterSkinPlugin ();
}

class GMSH_SkinPlugin : public GMSH_Post_Plugin
{
  int iView;
public:
  GMSH_SkinPlugin(int IVIEW);
  virtual void Run();
  virtual void Save();
  virtual void getName  (char *name) const;
  virtual void getInfos (char *author, 
			 char *copyright,
			 char *help_text) const;
  virtual void CatchErrorMessage (char *errorMessage) const;
  virtual int getNbOptions() const;
  virtual StringXNumber* GetOption (int iopt);  
  virtual Post_View *execute (Post_View *);
};
#endif
