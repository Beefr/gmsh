# $Id: Makefile,v 1.50 2001-02-06 07:52:51 geuzaine Exp $
# ----------------------------------------------------------------------
#  Makefile for Gmsh  
# ----------------------------------------------------------------------

         GMSH_RELEASE = 1.11

                 MAKE = make
                   CC = c++
                FLAGS = -O
                   RM = rm
              RMFLAGS = -f 

           OPENGL_INC = -I/usr/include/X11/GLw\
                        -I$(HOME)/SOURCES/Mesa-3.1/include\
                        -I$(HOME)/SOURCES/Mesa-3.1/include/GL
            MOTIF_INC = -I/usr/X11R6/LessTif/Motif1.2/include
            FLTK_INC   = -I$(HOME)/SOURCES/fltk
	    FLTK_INC_SCOREC = -I/users/develop/develop/visual/fltk/1.0/include

           OPENGL_LIB = -lGLU -lGL
     OPENGL_MOTIF_LIB = -lGLw
             MESA_LIB = -L$(HOME)/SOURCES/Mesa-3.1/lib -lGLU -lGL
       MESA_MOTIF_LIB = -L$(HOME)/SOURCES/Mesa-3.1/lib -lGLw
      MESA_STATIC_LIB = $(HOME)/SOURCES/Mesa-static/lib/libGLU.a\
                        $(HOME)/SOURCES/Mesa-static/lib/libGL.a
MESA_MOTIF_STATIC_LIB = $(HOME)/SOURCES/Mesa-static/lib/libGLw.a
          XMOTIF_LIB = /usr/local/lib/libXm.so.2 -L/usr/X11R6/lib -lXt -lX11 -lXext
#           XMOTIF_LIB = -L/usr/local/lib -L/usr/X11R6/LessTif/Motif1.2/lib -lXm\
                        -L/usr/X11R6/lib -lXt -lX11 -lXext 
             FLTK_LIB = -L$(HOME)/SOURCES/fltk/lib -lfltk\
                        -L/usr/X11R6/lib -lXext -lX11
             FLTK_LIB_SOLARIS_SCOREC = /users/develop/develop/visual/fltk/1.0/lib/sun4_5/libfltk-gcc.a\
                        -L/usr/X11R6/lib -lXext -lX11
             FLTK_LIB_LINUX_SCOREC = /users/develop/develop/visual/fltk/1.0/lib/x86_linux/libfltk.a\
                        -L/usr/X11R6/lib -lXext -lX11

           THREAD_LIB = -L/usr/lib -lpthread
             JPEG_LIB = jpeg/libjpeg.a

             GMSH_DIR = Adapt Common DataStr Geo Graphics Mesh Parser Motif Fltk\
                        jpeg utils
      GMSH_XMOTIF_DIR = Adapt Common DataStr Geo Graphics Mesh Parser Motif jpeg
        GMSH_FLTK_DIR = Adapt Common DataStr Geo Graphics Mesh Parser Fltk jpeg
     GMSH_FLTKWIN_DIR = Adapt Common DataStr Geo Graphics Mesh Parser Fltk
         GMSH_BOX_DIR = Adapt Box Common DataStr Geo Mesh Parser
         GMSH_BIN_DIR = bin
         GMSH_LIB_DIR = lib
         GMSH_DOC_DIR = doc
        GMSH_DEMO_DIR = demos
       GMSH_TUTOR_DIR = tutorial
     GMSH_ARCHIVE_DIR = archives
      GMSH_XMOTIF_LIB = -L$(GMSH_LIB_DIR) -lMotif -lGraphics -lParser -lMesh -lGeo\
                                          -lAdapt -lCommon -lDataStr $(JPEG_LIB)
        GMSH_FLTK_LIB = -L$(GMSH_LIB_DIR) -lFltk -lParser -lGraphics -lMesh -lGeo\
                                          -lAdapt -lCommon -lDataStr $(JPEG_LIB)
     GMSH_FLTKWIN_LIB = -L$(GMSH_LIB_DIR) -lFltk -lParser -lGraphics -lMesh -lGeo\
                                          -lAdapt -lCommon -lDataStr
         GMSH_BOX_LIB = -L$(GMSH_LIB_DIR) -lBox -lParser -lMesh -lGeo\
                                          -lAdapt -lCommon -lDataStr
         GMSH_ARCHIVE = $(GMSH_ARCHIVE_DIR)/gmsh-`date "+%Y.%m.%d"`
          GMSH_SRCRPM = gmsh-$(GMSH_RELEASE)
         GMSH_SOURCES = `find . \( ! -name "*.tar*" -a ! -name "*.tgz" \
                                -a ! -name "*.o"    -a ! -name "lib*.a"   \
                                -a ! -name "*.msh"  -a ! -name "*.bak" \
                                -a ! -name "gmsh"   -a ! -name "gmsh-*"\
                                -a ! -type d \)`
           GMSH_UNAME = `uname`

# ----------------------------------------------------------------------
# Rules for developpers
# ----------------------------------------------------------------------

default: initialtag
	@for i in $(GMSH_FLTK_DIR); do (cd $$i && $(MAKE) \
           "CC=$(CC)" \
           "C_FLAGS=$(FLAGS)" \
           "OS_FLAGS=-D_LITTLE_ENDIAN" \
           "VERSION_FLAGS=-D_FLTK" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(FLTK_INC)" \
        ); done

motif: initialtag
	@for i in $(GMSH_XMOTIF_DIR); do (cd $$i && $(MAKE) \
           "CC=$(CC)" \
           "C_FLAGS=$(FLAGS)" \
           "OS_FLAGS=-D_LITTLE_ENDIAN" \
           "VERSION_FLAGS=-D_XMOTIF" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(MOTIF_INC)" \
        ); done

gmsh:
	$(CC) -o $(GMSH_BIN_DIR)/gmsh $(GMSH_FLTK_LIB) $(MESA_LIB) \
                 $(FLTK_LIB) -lm

gmsh2:
	$(CC) -o $(GMSH_BIN_DIR)/gmsh $(GMSH_FLTK_LIB) $(MESA_LIB) \
                 $(FLTK_LIB) -lfltk_gl $(THREAD_LIB) -lm

motifgl:
	$(CC) -o $(GMSH_BIN_DIR)/gmsh $(GMSH_XMOTIF_LIB)\
              $(OPENGL_MOTIF_LIB) $(OPENGL_LIB) $(XMOTIF_LIB) $(THREAD_LIB) -lm

motifmesa:
	$(CC) -o $(GMSH_BIN_DIR)/gmsh $(GMSH_XMOTIF_LIB)\
              $(MESA_MOTIF_LIB) $(MESA_LIB) $(XMOTIF_LIB) $(THREAD_LIB) -lm

motifmesastatic:
	$(CC) -o $(GMSH_BIN_DIR)/gmshm $(GMSH_XMOTIF_LIB)\
              $(MESA_MOTIF_STATIC_LIB) $(MESA_STATIC_LIB)\
              $(XMOTIF_LIB) $(THREAD_LIB) -lm


# ----------------------------------------------------------------------
# Utilities
# ----------------------------------------------------------------------

parser:
	cd Parser && $(MAKE) parser

utilities:
	cd utils && $(MAKE)

purge:
	for i in "." $(GMSH_DIR) $(GMSH_LIB_DIR) $(GMSH_ARCHIVE_DIR)\
                     $(GMSH_DEMO_DIR) $(GMSH_TUTOR_DIR) $(GMSH_DOC_DIR) $(GMSH_BOX_DIR); \
        do (cd $$i && $(RM) $(RMFLAGS) *~ *~~ .gmshrc .gmshtmp .gmshlog gmon.out); \
        done

clean:
	for i in $(GMSH_DIR) $(GMSH_DOC_DIR) $(GMSH_LIB_DIR) ; \
        do (cd $$i && $(MAKE) clean); \
        done

depend:
	for i in $(GMSH_DIR); \
        do (cd $$i && $(MAKE) depend \
           "CC=$(CC)" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(MOTIF_INC) $(FLTK_INC)" \
        ); done

nodepend:
	for i in $(GMSH_DIR) ; do \
          (cd $$i && (sed '/^# DO NOT DELETE THIS LINE/q' Makefile) > Makefile.new \
          && cp Makefile Makefile.bak \
          && cp Makefile.new Makefile \
          && $(RM) $(RMFLAGS) Makefile.new); \
        done 

tag:
	$(RM) $(RMFLAGS) Common/GmshVersion.h
	echo "#define GMSH_VERSION  $(GMSH_RELEASE)" >  Common/GmshVersion.h
	echo "#define GMSH_DATE     \"`date`\""      >> Common/GmshVersion.h
	echo "#define GMSH_HOST     \"`hostname`\""  >> Common/GmshVersion.h
	echo "#define GMSH_PACKAGER \"`whoami`\""    >> Common/GmshVersion.h
	echo "#define GMSH_OS       \"`uname -sr`\"" >> Common/GmshVersion.h

initialtag:
	@if [ ! -r Common/GmshVersion.h ]; then \
        $(MAKE) tag ; \
        fi

tgz:
	if (test -f $(GMSH_ARCHIVE).tar.gz); \
	then mv -f $(GMSH_ARCHIVE).tar.gz $(GMSH_ARCHIVE).tar.gz~; \
	fi
	tar cvf $(GMSH_ARCHIVE).tar $(GMSH_SOURCES)
	gzip $(GMSH_ARCHIVE).tar
	chmod 640 $(GMSH_ARCHIVE).tar.gz

src:
	tar cvf $(GMSH_SRCRPM).tar $(GMSH_SOURCES)
	gzip $(GMSH_SRCRPM).tar

compress_bin:
	cd $(GMSH_BIN_DIR) && tar cvf gmsh-$(GMSH_UNAME).tar gmsh
	gzip $(GMSH_BIN_DIR)/gmsh-$(GMSH_UNAME).tar
	mv $(GMSH_BIN_DIR)/gmsh-$(GMSH_UNAME).tar.gz gmsh-$(GMSH_UNAME).tgz

strip_bin:
	strip $(GMSH_BIN_DIR)/gmsh

# ----------------------------------------------------------------------
# Demos
# ----------------------------------------------------------------------

dem:
	gtar zcvf gmsh-demos.tgz $(GMSH_DEMO_DIR)
	gtar zcvf gmsh-tutorial.tgz $(GMSH_TUTOR_DIR)

# ----------------------------------------------------------------------
# Black Box
# ----------------------------------------------------------------------

bb: tag
	@for i in $(GMSH_BOX_DIR); do (cd $$i && $(MAKE) \
           "CC=$(CC)" \
           "C_FLAGS=-O3" \
           "OS_FLAGS=" \
           "VERSION_FLAGS=-D_BLACKBOX" \
           "GL_INCLUDE=" \
           "GUI_INCLUDE=" \
        ); done
	$(CC) -o $(GMSH_BIN_DIR)/gmsh-box $(GMSH_BOX_LIB) -lm

bbn: tag
	@for i in $(GMSH_BOX_DIR) ; do (cd $$i && $(MAKE) \
           "CC=g++ -mno-cygwin -I/mingw/include" \
           "C_FLAGS=-O3" \
           "OS_FLAGS=" \
           "VERSION_FLAGS=-D_BLACKBOX" \
           "GL_INCLUDE=" \
           "GUI_INCLUDE=" \
        ); done
	g++ -o $(GMSH_BIN_DIR)/gmsh.exe -mno-cygwin -L/mingw/lib $(GMSH_BOX_LIB) -lm

# ----------------------------------------------------------------------
# Ready to compile for somes platforms with MOTIF
# ----------------------------------------------------------------------

motif_dec: tag motif_compile_little_endian motif_link_opengl strip_bin compress_bin

motif_linux: tag motif_compile_little_endian motif_link_mesa strip_bin compress_bin

motif_linux_2952: tag motif_compile_little_endian_2952 motif_link_mesa_2952 strip_bin compress_bin

motif_ibm: tag motif_compile_big_endian motif_link_mesa strip_bin compress_bin

motif_sun: tag motif_compile_big_endian motif_link_opengl strip_bin compress_bin

motif_hp: tag motif_compile_big_endian motif_link_hp strip_bin compress_bin

motif_sgi: tag motif_compile_sgi motif_link_sgi strip_bin compress_bin

motif_rpm: src
	mv $(GMSH_SRCRPM).tar.gz /usr/src/redhat/SOURCES
	rpm -bb utils/gmsh_motif.spec
	cp /usr/src/redhat/RPMS/i386/$(GMSH_SRCRPM)-1.i386.rpm .
	cp /usr/src/redhat/BUILD/$(GMSH_SRCRPM)/bin/gmsh .
	gtar zcvf gmsh-$(GMSH_UNAME).tgz gmsh
	rm -f gmsh

motif_compile_little_endian:
	@for i in $(GMSH_XMOTIF_DIR); do (cd $$i && $(MAKE) \
           "CC=g++" \
           "C_FLAGS=-O3" \
           "OS_FLAGS=-D_LITTLE_ENDIAN" \
           "VERSION_FLAGS=-D_XMOTIF" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(MOTIF_INC)" \
        ); done

motif_compile_little_endian_2952:
	@for i in $(GMSH_XMOTIF_DIR); do (cd $$i && $(MAKE) \
           "CC=$(HOME)/gcc-2.95.2/bin/g++" \
           "C_FLAGS=-O3" \
           "OS_FLAGS=-D_LITTLE_ENDIAN" \
           "VERSION_FLAGS=-D_XMOTIF" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(MOTIF_INC)" \
        ); done

motif_compile_little_endian_threads:
	@for i in $(GMSH_XMOTIF_DIR); do (cd $$i && $(MAKE) \
           "CC=g++" \
           "C_FLAGS=-D_REENTRANT -O3" \
           "OS_FLAGS=-D_LITTLE_ENDIAN" \
           "VERSION_FLAGS=-D_XMOTIF -D_USETHREADS" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(MOTIF_INC)" \
        ); done

motif_compile_big_endian:
	@for i in $(GMSH_XMOTIF_DIR); do (cd $$i && $(MAKE) \
           "CC=g++" \
           "C_FLAGS=-O3" \
           "OS_FLAGS=" \
           "VERSION_FLAGS=-D_XMOTIF" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(MOTIF_INC)" \
        ); done

# special car -O2 merde dans 3d_smesh.c sur TransfiniteHex()
motif_compile_sgi:
	@for i in $(GMSH_XMOTIF_DIR); do (cd $$i && $(MAKE) \
           "CC=CC" \
           "C_FLAGS=-O2 -o32 -Olimit 3000" \
           "RANLIB=true"\
           "AR=CC -o32 -ar -o"\
           "OS_FLAGS=" \
           "VERSION_FLAGS=-D_XMOTIF" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(MOTIF_INC)" \
        ); done
	@for i in Mesh; do (cd $$i && $(MAKE) \
           "CC=CC" \
           "C_FLAGS=-O1 -o32" \
           "AR=CC -o32 -ar -o"\
           "RANLIB=true"\
           "OS_FLAGS=" \
           "VERSION_FLAGS=-D_XMOTIF" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(MOTIF_INC)" \
        ); done

motif_link_sgi:
	CC -O2 -o32 -o $(GMSH_BIN_DIR)/gmsh $(GMSH_XMOTIF_LIB)\
           $(OPENGL_MOTIF_LIB) $(OPENGL_LIB) $(XMOTIF_LIB) -lm

motif_link_opengl:
	g++ -o $(GMSH_BIN_DIR)/gmsh $(GMSH_XMOTIF_LIB)\
            $(OPENGL_MOTIF_LIB) $(OPENGL_LIB) $(XMOTIF_LIB) -lm

motif_link_mesa:
	g++ -o $(GMSH_BIN_DIR)/gmsh $(GMSH_XMOTIF_LIB)\
            $(MESA_MOTIF_LIB) $(MESA_LIB) $(XMOTIF_LIB) -lm

motif_link_mesa_2952:
	$(HOME)/gcc-2.95.2/bin/g++ -o $(GMSH_BIN_DIR)/gmsh $(GMSH_XMOTIF_LIB)\
               $(MESA_MOTIF_LIB) $(MESA_LIB) $(XMOTIF_LIB) -lm

motif_link_mesa_threads:
	g++ -o $(GMSH_BIN_DIR)/gmsh $(GMSH_XMOTIF_LIB)\
               $(MESA_MOTIF_LIB) $(MESA_LIB) $(XMOTIF_LIB) $(THREAD_LIB) -lm

# special car +s necessaire pour shared libs avec SHLIB_PATH variable.
motif_link_hp:
	g++ -Wl,+s -o $(GMSH_BIN_DIR)/gmsh $(GMSH_XMOTIF_LIB)\
                      $(MESA_MOTIF_LIB) $(MESA_LIB) $(XMOTIF_LIB) -lm


# ----------------------------------------------------------------------
# Ready to compile for some platforms with FLTK
# ----------------------------------------------------------------------

fltk_compile_little_endian:
	@for i in $(GMSH_FLTK_DIR); do (cd $$i && $(MAKE) \
           "CC=$(CC)" \
           "C_FLAGS=-O3" \
           "OS_FLAGS=-D_LITTLE_ENDIAN" \
           "VERSION_FLAGS=-D_FLTK" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(FLTK_INC)" \
        ); done

fltk_compile_solaris_scorec :
	@for i in $(GMSH_FLTK_DIR); do (cd $$i && $(MAKE) \
           "CC=$(CC)" \
           "C_FLAGS=-O3" \
           "VERSION_FLAGS=-D_FLTK" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(FLTK_INC_SCOREC)" \
        ); done

fltk_compile_linux_scorec :
	@for i in $(GMSH_FLTK_DIR); do (cd $$i && $(MAKE) \
           "CC=$(CC)" \
           "C_FLAGS=-O3" \
           "VERSION_FLAGS=-D_FLTK" \
           "OS_FLAGS=-D_LITTLE_ENDIAN" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(FLTK_INC_SCOREC)" \
        ); done

fltk_link_solaris_scorec:
	$(CC) -o $(GMSH_BIN_DIR)/gmsh-sun $(GMSH_FLTK_LIB) $(OPENGL_LIB) \
                 $(FLTK_LIB_SOLARIS_SCOREC) -lm
fltk_link_linux_scorec:
	$(CC) -o $(GMSH_BIN_DIR)/gmsh-linux $(GMSH_FLTK_LIB) $(OPENGL_LIB) \
                 $(FLTK_LIB_LINUX_SCOREC) -lm
fltk_link_mesa:
	$(CC) -o $(GMSH_BIN_DIR)/gmsh $(GMSH_FLTK_LIB) $(MESA_LIB) \
                 $(FLTK_LIB) -lm

fltk_compile_little_endian_2952:
	@for i in $(GMSH_FLTK_DIR); do (cd $$i && $(MAKE) \
           "CC=$(HOME)/gcc-2.95.2/bin/g++" \
           "C_FLAGS=-O3" \
           "OS_FLAGS=-D_LITTLE_ENDIAN" \
           "VERSION_FLAGS=-D_FLTK" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(FLTK_INC)" \
        ); done

fltk_link_mesa_2952:
	$(HOME)/gcc-2.95.2/bin/g++ -o $(GMSH_BIN_DIR)/gmsh $(GMSH_FLTK_LIB) $(MESA_LIB) \
                 $(FLTK_LIB) -lm


fltk_linux: tag fltk_compile_little_endian fltk_link_mesa strip_bin compress_bin

fltk_linux_2952: tag fltk_compile_little_endian_2952 fltk_link_mesa_2952 strip_bin compress_bin

fltk_solaris_scorec : tag fltk_compile_solaris_scorec fltk_link_solaris_scorec strip_bin 
fltk_linux_scorec : tag fltk_compile_linux_scorec fltk_link_linux_scorec strip_bin 

fltk_rpm: src
	mv $(GMSH_SRCRPM).tar.gz /usr/src/redhat/SOURCES
	rpm -bb utils/gmsh_fltk.spec
	cp /usr/src/redhat/RPMS/i386/$(GMSH_SRCRPM)-1.i386.rpm .
	cp /usr/src/redhat/BUILD/$(GMSH_SRCRPM)/bin/gmsh .
	gtar zcvf gmsh-$(GMSH_UNAME).tgz gmsh
	rm -f gmsh

fltk_mingw: tag
	@for i in $(GMSH_FLTKWIN_DIR); do (cd $$i && $(MAKE) \
           "CC=g++ -mno-cygwin -I/mingw/include" \
           "C_FLAGS=-O2 -DWIN32" \
           "OS_FLAGS=-D_LITTLE_ENDIAN" \
           "VERSION_FLAGS=-D_FLTK" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(FLTK_INC)" \
        ); done
	g++ -mno-cygwin -L/mingw/lib -o $(GMSH_BIN_DIR)/gmsh.exe $(GMSH_FLTKWIN_LIB) \
                 $(HOME)/SOURCES/fltk/lib/libfltk.a -lglu32 -lopengl32 -lgdi32 -lwsock32 -lm

fltk_cygwin: tag
	@for i in $(GMSH_FLTKWIN_DIR); do (cd $$i && $(MAKE) \
           "CC=g++" \
           "C_FLAGS=-O2 -DWIN32" \
           "OS_FLAGS=-D_LITTLE_ENDIAN" \
           "VERSION_FLAGS=-D_FLTK" \
           "GL_INCLUDE=$(OPENGL_INC)" \
           "GUI_INCLUDE=$(FLTK_INC)" \
        ); done
	g++ -Wl,--subsystem,windows -o $(GMSH_BIN_DIR)/gmsh.exe $(GMSH_FLTKWIN_LIB) \
                 $(HOME)/SOURCES/fltk/lib/libfltk.a -lglu32 -lopengl32 -lgdi32 -lwsock32 -lm
	strip $(GMSH_BIN_DIR)/gmsh.exe
