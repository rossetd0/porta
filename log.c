/*****************************************************************/
/*****************************************************************

    Copyright (C) 1997 Thomas Christof, Andreas Loebel
 
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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 

  NAME: log.c
  
  LAST MODIFICATION DATE: 17.01.1994
  
  AUTHOR: Andreas Loebel
  
  KONRAD-ZUSE-ZENTRUM FUER
  INFORMATIONSTECHNIK BERLIN
  HEILBRONNER STR. 10
  D-10711 BERLIN-WILMERSDORF

 *****************************************************************/
/*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>



extern FILE *logfile;


int log( char *fmt, ... )
{
	va_list argp;
	int ret;
		
	if( logfile )
	{
		va_start( argp, fmt );
		ret = vfprintf( logfile, fmt, argp );
		va_end( argp );
		return ret;
	}
	else
		return 0;
}
