/* $Id: GmshServer.cpp,v 1.9 2003-01-24 23:13:34 geuzaine Exp $ */
/*
  Copyright (C) 1997 - 2003 C. Geuzaine, J.-F. Remacle

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
 
  Please report all bugs and problems to "gmsh@geuz.org".
*/

// This is a hacked version using the Gmsh function SystemCall()
// instead system()
void SystemCall(char *str);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _AIX
#include <strings.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/time.h>
#include <unistd.h>

/* private functions */

static int Socket_ReceiveData(int socket, void *buffer, int bytes){
  int sofar, remaining, len;
  char *buf;

  buf = (char*)buffer;
  sofar = 0;
  remaining = bytes;
  do {
    len = read(socket, buf + sofar, remaining);
    if (len<=0) return 0;
    sofar += len;
    remaining -= len;
  } while (remaining>0);
  return bytes;
}

static int Socket_UnlinkName(char *name){
#ifdef _AIX
   char name2[1000];
   strcpy(name2,name);
   name2[strlen(name2)-1] = '\0';
   return unlink(name2);
#else
   return unlink(name);
#endif
}


/* public interface */

int Gmsh_StartClient(char *command, char *sockname){
  int s, sock;
#if defined(linux) || defined(_AIX)
  socklen_t len ;
#else
  int len;
#endif
  struct sockaddr_un addr, from;
  fd_set rfds;
  struct timeval tv;
  int retval;

  /* no socket? launch the command! */
  if(!sockname){
    //system(command);
    SystemCall(command);
    return 1;
  }

  /* first delete the socket's name if it exists */
  Socket_UnlinkName(sockname);
  
  /* make the socket */
  s = socket(PF_UNIX, SOCK_STREAM, 0);
  if (s<0) return -1; /* Error: Couldn't create socket */
  
  /* bind the socket to its name */
  strcpy(addr.sun_path, sockname);
  addr.sun_family = AF_UNIX;
  if(bind(s, (struct sockaddr *)&addr, 
	  strlen(addr.sun_path)+sizeof(addr.sun_family)) < 0)
    return -2; /* Error: Couldn't bind socket to name */
  
  /* change permissions on the socket name in case it has to be rm'd later */
  chmod(sockname, 0666);
  
  /* Start the external function via system() call */
  //system(command);
  SystemCall(command);

  /* wait for external function to connect */
  if(listen(s, 20)) return -3; /* Error: Socket listen failed */
  
  /* Watch s to see when it has input. */
  /* Wait up to 2 seconds */
  tv.tv_sec = 2;
  tv.tv_usec = 0;
  FD_ZERO(&rfds);
  FD_SET(s, &rfds);
  retval = select(s+1, &rfds, NULL, NULL, &tv);
  
  if(!retval) return -4; /* Error: Socket listening timeout */
  
  len = sizeof(from);
  if ((sock = accept(s, (struct sockaddr *)&from, &len)) < 0)
    return -5; /* Error: Socket accept failed */
  
  close(s); /* don't need this socket anymore */
  
  return sock;
}

int Gmsh_ReceiveString(int socket, int *type, char str[]){
   int len;

   Socket_ReceiveData(socket, type, sizeof(int));
   if (Socket_ReceiveData(socket, &len, sizeof(int))) {
     if (Socket_ReceiveData(socket, str, len)==len) {
       str[len] = '\0';
       return 1;
     }
   }
   return 0;
}

int Gmsh_StopClient(char *sockname, int sock){
  if(Socket_UnlinkName(sockname)==-1)
    return -1; /* Impossible to unlink the socket */
  close(sock);
  return 0;
}

