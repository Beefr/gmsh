// $Id: Message.cpp,v 1.8 2001-01-12 13:28:55 geuzaine Exp $

#include <signal.h>
#ifndef WIN32
#include <sys/resource.h>
#endif

#include "Gmsh.h"
#include "GmshUI.h"
#include "Version.h"
#include "Context.h"
#include "GUI.h"

extern GUI       *WID;
extern Context_T  CTX;

/* ------------------------------------------------------------------------ */
/*  S i g n a l                                                             */
/* ------------------------------------------------------------------------ */

void Signal (int sig_num){

  switch (sig_num){
  case SIGSEGV : 
    Msg(FATAL1, "Segmentation Violation (Invalid Memory Reference)");
    Msg(FATAL2, "------------------------------------------------------");
    Msg(FATAL2, "You have discovered a bug in Gmsh. You may e-mail the");
    Msg(FATAL2, "context in which it occurred to one of the authors:");
    Msg(FATAL3, "type 'gmsh -info' to get feedback information"); 
    break;
  case SIGFPE : 
    Msg(FATAL, "Floating Point Exception (Division by Zero?)"); 
    break;
  case SIGINT :
    Msg(FATAL, "Interrupt (Generated from Terminal Special Character)"); 
    break;
  default :
    Msg(FATAL, "Unknown Signal");
    break;
  }
}


/* ------------------------------------------------------------------------ */
/*  M s g                                                                   */
/* ------------------------------------------------------------------------ */

#define PUT_IN_COMMAND_WIN			\
    vfprintf(stderr, fmt, args); 		\
    fprintf(stderr, "\n");

void Msg(int level, char *fmt, ...){
  va_list  args;
  int      abort = 0, verb = 0, window = -1, log = 1;
  char     *str = NULL;

  switch(level){
  case STATUS1N : log = 0; //fallthrough
  case STATUS1  : window = 0; break ;
  case STATUS2N : log = 0; //fallthrough
  case STATUS2  : window = 1; break ;
  case STATUS3N : log = 0; //fallthrough
  case STATUS3  : window = 2; break ;

  case FATAL    : str = FATAL_STR; abort = 1; break ;
  case FATAL1   : str = FATAL_STR; break ;
  case FATAL2   : str = FATAL_NIL; break ;
  case FATAL3   : str = FATAL_NIL; abort = 1; break ;
				     		  
  case GERROR   : 		     		  
  case GERROR1  : str = ERROR_STR; break ;
  case GERROR2  : 		     
  case GERROR3  : str = ERROR_NIL; break ;
				     	  
  case WARNING  : 		     	  
  case WARNING1 : str = WARNING_STR; verb = 1; break ;
  case WARNING2 : 		     	  
  case WARNING3 : str = WARNING_NIL; verb = 1; break ;
				     	  
  case INFO     :		     	  
  case INFO1    : str = INFO_STR; verb = 2; break ;
  case INFO2    :		     	  
  case INFO3    : str = INFO_NIL; verb = 2; break ;
				     	  
  case DEBUG    :		     	  
  case DEBUG1   : str = DEBUG_STR; verb = 3; break ;
  case DEBUG2   :		     	  
  case DEBUG3   : str = DEBUG_NIL; verb = 3; break ;

  case PARSER_ERROR : str = PARSER_ERROR_STR ; break ;

  case PARSER_INFO : str = PARSER_INFO_STR ; verb = 2; break ;

  case LOG_INFO : verb = 2 ; window = 3; break ;

  default : return;
  }

  static char buff1[1024], buff2[1024], buff[4][1024];

  if(CTX.interactive){
    if(verb) return;
    window = -1;
  }
  else 
    WID->check();

  if(CTX.verbosity >= verb){
    va_start (args, fmt);

    if(window >= 0){
      vsprintf(buff[window], fmt, args); 
      if(window <= 2) WID->set_status(buff[window], window);
      if(log && strlen(buff[window])) WID->add_message(buff[window]);
    }
    else{
      strcpy(buff1, "@C1");
      if(str && window<0) strcat(buff1, str);
      vsprintf(buff2, fmt, args); 
      strcat(buff1,buff2);
      fprintf(stderr, "%s\n", &buff1[3]);
      if(!CTX.interactive){
	if(verb<2)
	  WID->add_message(buff1);
	else
	  WID->add_message(&buff1[3]);
	if(!verb) WID->create_message_window();
      }
    }
    va_end (args);
  }

  if(abort){
    WID->save_message("error.log");
    exit(1);
  }
}


/* ------------------------------------------------------------------------ */
/*  C p u                                                                   */
/* ------------------------------------------------------------------------ */

void GetResources(long *s, long *us, long *mem){
#ifndef WIN32
  static struct rusage r;

  getrusage(RUSAGE_SELF,&r);
  *s   = (long)r.ru_utime.tv_sec ;
  *us  = (long)r.ru_utime.tv_usec ;
  *mem = (long)r.ru_maxrss ;
#else
  *s = *us = *mem = 0;
#endif

}

void PrintResources(FILE *stream, char *fmt, long s, long us, long mem){
  fprintf(stream, "Resources = %scpu %ld.%ld s / mem %ld kb\n", fmt, s, us, mem);
}

double Cpu(void){
  long s, us, mem;
  GetResources(&s, &us, &mem);
  return (double)s + (double)us/1.e6 ;
}

/* ------------------------------------------------------------------------ */
/*  P r o g r e s s                                                         */
/* ------------------------------------------------------------------------ */

void Progress(int i){
}

/* ------------------------------------------------------------------------ */
/*  E d i t G e o m e t r y                                                 */
/* ------------------------------------------------------------------------ */

void AddALineInTheEditGeometryForm (char* line){
}
