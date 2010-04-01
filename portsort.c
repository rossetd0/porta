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
 
  NAME: portsort.c
  
  LAST MODIFICATION DATE: 17.01.1994
  
  AUTHOR: Thomas Christof
  
  REVISED BY ANDREAS LOEBEL & MECHTHILD STOER
             KONRAD-ZUSE-ZENTRUM FUER
             INFORMATIONSTECHNIK BERLIN
             HEILBRONNER STR. 10
             D-10711 BERLIN-WILMERSDORF

 *****************************************************************/
/*****************************************************************/


#include "porta.h"


/* 17.01.1994: include logging on file porta.log */
extern FILE *logfile;
extern int log( );


int comp,delay,same_vals=0,(*syscompare)(),rowlen;


/*****************************************************************/

int int_syscompare(i,j)
	 listp *i,*j;

/*****************************************************************/

{
	return(((*i)->sys+comp)->num - ((*j)->sys+comp)->num > 0);
}

/*****************************************************************/

int rat_syscompare(i,j)
	 listp *i,*j;
	 
/*****************************************************************/

{
	
    (*RAT_sub)(*((*i)->sys+comp),*((*j)->sys+comp),var);
    return(var[0].num > 0);
}

/*****************************************************************/

int ptrcompare(i,j)
	 listp *i,*j;
	 
/*****************************************************************/

{
	int *statptr1,*statptr2;
	
	statptr1 = (int *) (*i)->ptr;
	statptr2 = (int *) (*j)->ptr;
	return(*(statptr1+comp) - *(statptr2+comp) > 0);
}

/*****************************************************************/

sortrekurs(first,last,whatcomp)
	 int first,last,whatcomp;

/*****************************************************************/

{
	int i,compint,*statptr1,*statptr2;
	
	if (whatcomp == 0 || whatcomp > 10) 
	{
		
		if (whatcomp == 0)  
		{
			comp = rowlen-1;
		}
		else 
		{
			comp = whatcomp - 11;
		}
		
		if (whatcomp == 11) 
		{
			same_vals++; 
			for (i = first; i <= last; i++)
				*((int *) list[i]->ptr) = same_vals;
		}
		
		compint = comp;
		qsort(CP &list[first],last-first+1,sizeof(list[0]),*syscompare);
		
		for (; first < last; first = i+1) 
		{
			(*RAT_sub)(*(list[first]->sys+compint),*(list[first+1]->sys+compint),var+1);
			for (i = first; i < last && var[1].num == 0; i++)
				if (i+1 < last) 
					(*RAT_sub)(*(list[i+1]->sys+compint),*(list[i+2]->sys+compint),var+1);
			if (i != first && whatcomp < rowlen+10)
				sortrekurs(first,i,whatcomp+1);
		}
	}
	else  
	{
		
		compint = comp = whatcomp-6;
		if (comp >= 0)
			compint = ++comp;
		
		qsort(CP (list+first),last-first+1,sizeof(list[0]),ptrcompare);
		
		for (; first < last; first = i+1) 
		{
			statptr1 = (int *) list[first]->ptr;
			statptr2 = (int *) list[first+1]->ptr;
			for (i = first; 
				 i < last && 
				 (*(statptr1+compint) == *(statptr2+compint)); i++) 
			{
				statptr1 = (int *) list[i+1]->ptr;
				if (i+1 < last)
					statptr2 = (int *) list[i+2]->ptr;
			}  
			if (i != first)
				sortrekurs(first,i,whatcomp+1);
		}
	}
}


/*****************************************************************/

sort(int_val,rl,first,last)
	 int int_val,rl,first,last;
	 
/*****************************************************************/

{
	int i,j,val,*statptr,*statistik;
	
	fprintf(prt,"sorting system ");

	/* 17.01.1994: include logging on file porta.log */
	log( "sorting system ");
	
	rowlen = rl;
	syscompare = (int_val) ? int_syscompare : rat_syscompare;
	
	statistik = (int *) allo(CP 0,0,U (last-first+1)*11*sizeof(int));
	for (j = 0; j < (last-first+1)*11; j++)
		*(statistik+j) = 0;
	
	statptr = statistik + 5;
	
	for (i = first; i < last; i++,statptr += 11) 
	{
		
		list[i]->ptr =  (RAT *) statptr;
		
		for ( j = 0; j != rl-1; j++) 
		{
			
			val = ((list[i]->sys+j)->den.i == 1) ? (list[i]->sys+j)->num : -100;
			if ( val < 0 && val > -6)
				*(statptr+val) += 1;
			else if (val > 0 && val < 6 ) 
				*(statptr+val) += 1;
			
		}
	}
	
	sortrekurs(first,last-1,0);
	
	if (MP_realised)
		return_from_mp();
	
	fprintf(prt,"\n");

	/* 17.01.1994: include logging on file porta.log */
	log( "\n");
}

