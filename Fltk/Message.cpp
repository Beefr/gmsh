// $Id: Message.cpp,v 1.55 2004-06-08 00:30:21 geuzaine Exp $
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

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

#if defined(__APPLE__)
#define RUSAGE_SELF      0
#define RUSAGE_CHILDREN -1
#endif

#include "Gmsh.h"
#include "GmshUI.h"
#include "GmshVersion.h"
#include "Context.h"
#include "Options.h"
#include "GUI.h"

extern GUI *WID;
extern Context_T CTX;

// Handle signals. It is a crime to call stdio functions in a signal
// handler. But who cares? ;-)

void Signal(int sig_num)
{
  switch (sig_num) {
  case SIGSEGV:
    Msg(FATAL1, "Segmentation violation (invalid memory reference)");
    Msg(FATAL2, "------------------------------------------------------");
    Msg(FATAL2, "You have discovered a bug in Gmsh! You may report it");
    Msg(FATAL2, "by e-mail (together with any helpful data permitting to");
    Msg(FATAL3, "reproduce it) to <gmsh@geuz.org>");
    break;
  case SIGFPE:
    Msg(FATAL, "Floating point exception (division by zero?)");
    break;
  case SIGINT:
    Msg(INFO, "Interrupt (generated from terminal special character)");
    Exit(1);
    break;
  default:
    Msg(FATAL, "Unknown signal");
    break;
  }
}

// General purpose message routine

void Debug()
{
  ;
}

void Msg(int level, char *fmt, ...)
{
  va_list args;
  int abort = 0, verb = 0, window = -1, log = 1, color = 0;
  char *str = NULL;

  // *INDENT-OFF*
  switch(level){
  case DIRECT   : color = 5; verb = 2; break ;
  case SOLVER   : color = 4; verb = 3; break ;
  case SOLVERR  : color = 1; verb = 3; break ;

  case STATUS1N : log = 0; //fallthrough
  case STATUS1  : str = INFO_STR; verb = 1; window = 0; break ;
  case STATUS2N : log = 0; //fallthrough
  case STATUS2  : str = INFO_STR; verb = 1; window = 1; break ;
  case STATUS3N : log = 0; //fallthrough
  case STATUS3  : str = INFO_STR; verb = 1; window = 2; break ;
  case ONSCREEN : log = 0; verb = 1; window = 3; break ;

  case FATAL    : str = FATAL_STR; abort = 1; break ;
  case FATAL1   : str = FATAL_STR; break ;
  case FATAL2   : str = WHITE_STR; break ;
  case FATAL3   : str = WHITE_STR; abort = 1; break ;
				     		  
  case GERROR   : 		     		  
  case GERROR1  : str = ERROR_STR; break ;
  case GERROR2  : 		     
  case GERROR3  : str = WHITE_STR; break ;
				     	  
  case WARNING  : 		     	  
  case WARNING1 : str = WARNING_STR; verb = 1; break ;
  case WARNING2 : 		     	  
  case WARNING3 : str = WHITE_STR; verb = 1; break ;
				     	  
  case INFO     :		     	  
  case INFO1    : str = INFO_STR; verb = 3; break ;
  case INFO2    :		     	  
  case INFO3    : str = WHITE_STR; verb = 3; break ;
				     	  
  case DEBUG    :		     	  
  case DEBUG1   : str = DEBUG_STR; verb = 4; break ;
  case DEBUG2   :		     	  
  case DEBUG3   : str = WHITE_STR; verb = 4; break ;

  default : return;
  }
  // *INDENT-ON*

  if(verb < 2)
    color = 1;

#define BUFFSIZE 1024

  static char buff1[BUFFSIZE], buff2[BUFFSIZE], buff[5][BUFFSIZE];

  if(CTX.verbosity >= verb) {

    if(!WID){
      window = -1;
    }
    else{
      // This is pretty costly, but permits to keep the app
      // responsive. The downside is that it can cause race
      // conditions: everytime we output a Msg, a pending callback can
      // be executed! We fix this by encapsulating critical sections
      // (mai3d(), CreateFile(), etc.) with 'CTX.threads_lock', but
      // this is far from perfect...
      if(level != DEBUG &&
	 level != DEBUG1 &&
	 level != DEBUG2 &&
	 level != DEBUG3 &&
	 level != STATUS1N &&
	 level != STATUS2N &&
	 level != STATUS3N){
	WID->check();
      }
    }

    va_start(args, fmt);

    if(window >= 0) {
#if defined(HAVE_NO_VSNPRINTF)
      vsprintf(buff[window], fmt, args);
#else
      vsnprintf(buff[window], BUFFSIZE, fmt, args);
#endif
      WID->set_status(buff[window], window);
      if(log && strlen(buff[window])){
	strcpy(buff1, str ? str : "");
	strncat(buff1, buff[window], BUFFSIZE-strlen(buff1));
        WID->add_message(buff1);
      }
    }
    else if(log) {
      sprintf(buff1, "@C%d@.", color);
      if(str)
        strncat(buff1, str, BUFFSIZE-strlen(buff1));

#if defined(HAVE_NO_VSNPRINTF)
      vsprintf(buff2, fmt, args);
#else
      vsnprintf(buff2, BUFFSIZE, fmt, args);
#endif
      strncat(buff1, buff2, BUFFSIZE-strlen(buff1));
      if(CTX.terminal)
        fprintf(stderr, "%s\n", &buff1[5]);
      if(WID) {
        if(color)
          WID->add_message(buff1);
        else
          WID->add_message(&buff1[5]);
        if(!verb)
          WID->create_message_window();
      }
    }
    va_end(args);

    if(CTX.terminal)
      fflush(stderr);
  }

  if(abort) {
    Debug();
    if(WID) {
      WID->save_message(CTX.errorrc_filename);
      WID->fatal_error(CTX.errorrc_filename);
    }
    Exit(1);
  }
}


// Clean exit

void Exit(int level)
{
  // delete the temp file
  unlink(CTX.tmprc_filename);

  if(level){
    // in case of an abnormal exit, force the abort directly
    // (bypassing any post main stuff, e.g. destructors for static
    // variables). This still guarantees that any open streams are
    // flushed and closed, but can prevent nasty infinite loops.
    abort();
  }

  // if we exit cleanly (level==0) and we are in full GUI mode, save
  // the persistent info to disk
  if(WID && !CTX.batch) {
    if(CTX.session_save) {
      CTX.position[0] = WID->m_window->x();
      CTX.position[1] = WID->m_window->y();
      CTX.gl_position[0] = WID->g_window->x();
      CTX.gl_position[1] = WID->g_window->y();
      CTX.msg_position[0] = WID->msg_window->x();
      CTX.msg_position[1] = WID->msg_window->y();
      CTX.msg_size[0] = WID->msg_window->w();
      CTX.msg_size[1] = WID->msg_window->h();
      CTX.opt_position[0] = WID->opt_window->x();
      CTX.opt_position[1] = WID->opt_window->y();
      CTX.stat_position[0] = WID->stat_window->x();
      CTX.stat_position[1] = WID->stat_window->y();
      CTX.vis_position[0] = WID->vis_window->x();
      CTX.vis_position[1] = WID->vis_window->y();
      CTX.ctx_position[0] = WID->context_geometry_window->x();
      CTX.ctx_position[1] = WID->context_geometry_window->y();
      CTX.solver_position[0] = WID->solver[0].window->x();
      CTX.solver_position[1] = WID->solver[0].window->y();
      Print_Options(0, GMSH_SESSIONRC, false, CTX.sessionrc_filename);
    }
    if(CTX.options_save)
      Print_Options(0, GMSH_OPTIONSRC, true, CTX.optionsrc_filename);
  }

  exit(0);
}

// CPU time computation, etc.

void GetResources(long *s, long *us, long *mem)
{
  static struct rusage r;

  getrusage(RUSAGE_SELF, &r);
  *s = (long)r.ru_utime.tv_sec;
  *us = (long)r.ru_utime.tv_usec;
  *mem = (long)r.ru_maxrss;
}

void PrintResources(long s, long us, long mem)
{
  Msg(INFO, "cpu %ld.%ld s / mem %ld kb\n", s, us, mem);
}

double Cpu(void)
{
  long s, us, mem;
  GetResources(&s, &us, &mem);
  return (double)s + (double)us / 1.e6;
}
