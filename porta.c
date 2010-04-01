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

  NAME: porta.c
  
  LAST MODIFICATION DATE: 21.02.1997
  
  AUTHOR: Thomas Christof
          Universitaet Heidelberg      
          Im Neuenheimer Feld 294  
          D-69120 Heidelberg       
 

  REVISED BY ANDREAS LOEBEL & MECHTHILD STOER
             KONRAD-ZUSE-ZENTRUM FUER
             INFORMATIONSTECHNIK BERLIN
             HEILBRONNER STR. 10
             D-10711 BERLIN-WILMERSDORF

 *****************************************************************/
/*****************************************************************/

#include "porta.h"

/* 17.01.1994: include logging to file porta.log */
FILE *logfile;
extern int log( );


void init_total_time();


/*****************************************************************/

main(argc,argv)
	 int argc;
	 char *argv[];

/*****************************************************************/

{
	extern char *allo();
	int	i, ieq_file, start;
	char outfname[20];
	char fname[20];
	int 	sysrow;	    /* length of list[]->sys rows */
	int   poi_file;
	int   rowl_inar, ierl;
	int  *indx = (int *)0;      
	int equa_in,ineq_in, ineq_out;
	FILE *outfp;
	void 	fourier_motzkin(), gentableau(), red_test(), origin_add(),
  	resubst(), sort(), writelistsys(), polarformat(),
  	get_options(),arith_overflow_func(), reorder_var();
	int * check_and_reorder_elim_ord(), read_input_file();
	FILE * wfopen();  

	
	/* 21.01.1994 include version and author informations */
	fprintf( stderr, "\n\nPORTA Vers. %s, written by T. Christof ", VERSION );
	fprintf( stderr, "(Uni Heidelberg),\n" );
	fprintf( stderr, "                   revised by A. Loebel and M. Stoer " );
	fprintf( stderr, "(ZIB Berlin)\n\n" );
	if( argc <= 2 )
	{
		fprintf( stderr,
				"For more information read the manpages about porta.\n\n");
		exit(-1);
	}
			
	/* 17.01.1994: include logging on file porta.log */
	if( !(logfile = fopen( "porta.log", "a" )) )
		fprintf( stderr, "can't open logfile porta.log\n" );
	else
	{
		log( "\n\n\nlog for " );
		for( i = 0; i < argc; i++ )
			log( "%s ", argv[i] );
		log( "\n\n" );
	}
			
	init_total_time();
	
	initialize();

	prt = stdout;
	get_options(&argc,&argv);
	
	if (option & Protocol_to_file) 
	{
		strcat(*argv,".prt");
		prt = fopen(*argv,"w");
		(*argv)[strlen(*argv)-4] = '\0';
	}
	setbuf(prt,CP 0);
	
	set_I_functions();
	SET_MP_not_ready;
	ieq_file = !strcmp(*argv+strlen(*argv)-4,".ieq");
	poi_file = !strcmp(*argv+strlen(*argv)-4,".poi");
	
	if (!poi_file && !ieq_file)
		msg("invalid format of command line");
    
	/*
	 * change by M.S. 5.6.92:
	 * read_input_file writes to the output file, if is_set(Sort).
	 */
	outfp = 0;
	strcpy(outfname,*argv);
	if (is_set(Sort) && poi_file) 
	{
		strcat(outfname,".poi");
		outfp = wfopen(outfname);
	}
	if (is_set(Sort) && ieq_file) 
	{
		strcat(outfname,".ieq");
		fprintf(prt,"outfname = %s\n",outfname);
		fflush(stdout);

		/* 17.01.1994: include logging on file porta.log */
		log( "outfname = %s\n",outfname );
		fflush(logfile);

		outfp = wfopen(outfname);
	}

	if (is_set(Fmel) && ieq_file) 
	{  
		/* ONLY FM-ELIMINATON */
		
		int *elim_ord,nel;
	        char *cp1;
 		elim_ord = 0;
		cp1=strdup("ELIMINATION_ORDER");
		if(!cp1)
                        msg("allocation of new space failed");   
		ineq = read_input_file(*argv,outfp,&dim,&ar1,&nel_ar1,
						
cp1,&elim_ord,"\0",&i,"\0",&i);
		free(cp1);
		sort_eqie_cvce(ar1,ineq,dim+2,&equa_in,&ineq_in);
		ineq = ineq_in+equa_in;
		/*     elim_ord = check_and_reorder_elim_ord(elim_ord,&nel); */
		reorder_var(ineq,ar1,&ar2,&nel_ar2,&nel,&elim_ord,&indx);
		/*     indx = elim_ord+nel; */
		/* 
		 * Transform all inequalities into "<= 1" or "<=-1" inequalities,
		 * if possible.
		 * If the right-hand side is 0, divide the numerators by their gcd
		 * and the denominators by their gcd.
		 * (This is not really necessary).
		 */
		/*
		   for (i = 0; i < ineq; i++) 
		   (* RAT_row_prim)(ar2+(dim+2)*i,ar2+(dim+1)*i,ar2+(dim+2)*i+dim,dim+1);
		   */
               if(is_set(Long_arithmetic)) {
		  SET_MP_ready;
	          arith_overflow_func(0,0,0,0,0);
                }
		for (i = 0; i < ineq; i++) 
			(* RAT_row_prim)(ar2+(dim+1)*i,ar2+(dim+1)*i,ar2+(dim+1)*i+dim,dim+1);
		/*     nel_ar2 = ineq*(dim+1);  
		 */
		equa = 0;
		ineq_out = ineq;
		gauss(0, dim+1, dim+1, dim-nel, equa_in, &ineq_out, &equa, indx);
		for (; (*indx) < 0; indx++);
		ierl = dim-nel-equa +1;
		/* row-length of inequalities after fourier_motzkin
		 * =  number of noneliminated variables+1 */
		nel = nel - (ineq - ineq_out);	/* number of variables to be elim. */
		ineq = ineq_out;
		fourier_motzkin(0,ineq-equa,dim+1-equa_in,nel,poi_file,indx,elim_ord);
		if ((MP_realised && return_from_mp()) || !MP_realised) 
			sort(no_denom(ierl, 0, ineq,1), ierl, 0, ineq);
		write_ieq_file(*argv,outfp,equa,ineq,dim+1,0,
					   ineq,0,ierl,indx);
		
	}
	else if (is_set(Sort)) 
	{
		points = read_input_file(*argv,outfp,&dim,&ar1,&nel_ar1,"\0",&i,
								 "\0",&i,"\0",&i);
		if (ieq_file)
			sort_eqie_cvce(ar1,points,dim+2,&equa,&ineq);
		listptoar(ar1,points,ieq_file?dim+2:dim+1,0); 
		if (ieq_file) 
		{ 
			if (equa) sort(1,dim+1,0,equa);
			if (ineq) sort(1,dim+1,equa,points); 
			write_ieq_file(*argv,outfp,equa,0,dim+1,0,ineq,equa,dim+1,0);
		}
		else 
		{
			sort(1,dim+1,0,points);
			for (cone = 0; !(list[cone]->sys[dim].num); cone++);
			write_poi_file(*argv,outfp,dim,0,0,cone,0,points-cone,cone);
		}
	}  
	else if ((is_set(Traf) || is_set(Dim)) && poi_file) 
	{
		points = read_input_file(*argv,outfp,&dim,&ar1,
								 &nel_ar1,"\0",&i,"\0",&i,"\0",&i);
		gentableau(ar1,1,&rowl_inar,&indx); 
                if(is_set(Long_arithmetic)) {
		  SET_MP_ready;
	          arith_overflow_func(0,0,0,0,0);
                }
		ineq = (cone == points) ? dim : dim + 1;
		ineq_out = ineq;  /*not used further */
		gauss(1, points+dim+1,dim+1,dim,ineq,&ineq_out, &equa, indx);
		/* make indx point to the system-variable section */
		for (; (*indx) < 0; indx++);
		if (is_set(Dim)) 
		{
			char str[100];
			fprintf (prt,"\nDIMENSION OF THE POLYHEDRON : %i\n\n",dim-equa);
			
			/* 17.01.1994: include logging on file porta.log */
			log ( "\nDIMENSION OF THE POLYHEDRON : %i\n\n",dim-equa );
			
			sprintf (str,"echo 'DIMENSION OF THE POLYHEDRON : %i' | cat >> %s",
					 dim-equa,argv[0]);
			system(str);
			if (equa) 
			{
				fprintf(prt,"equations :\n");

				/* 17.01.1994: include logging on file porta.log */
				log( "equations :\n");
				
				listptoar(ar4,equa,dim+1,0); 
				if ((MP_realised && return_from_mp()) || !MP_realised) 
					sort(no_denom(dim+1,0,equa,1), dim+1,0,equa);
				start = 1;
				/* last argument of writesys was lost? 
				   writesys(prt,0,equa,dim+1,0,0,'=');
				   */
				writesys(prt,0,equa,dim+1,0,0,'=', &start);

				/* log equation system */
				start = 1;
				writesys(logfile,0,equa,dim+1,0,0,'=', &start);
			}
		}
		else  
		{
			/* POINTS TO INEQUALITIES */
			sprintf(fname,"%s.ieq",*argv);
			fourier_motzkin(fname,ineq-equa,points+dim+1-ineq,
							points-ineq+equa,poi_file,indx,0);
			if (is_set(Validity_table_out)) 
				red_test(indx,ar1,&rowl_inar);
			if ((MP_realised && return_from_mp()) || !MP_realised) 
			{
				if (equa) sort(no_denom(dim+1, ineq, ineq+equa,1), 
							   dim+1, ineq, ineq+equa);
				sort(no_denom(dim+1-equa, 0, ineq,1), 
					 dim+1-equa, 0, ineq);
			}
			write_ieq_file(*argv,outfp,equa,ineq,
						   dim+1,0,ineq,0,dim+1-equa,indx);
		}
	}
	else if (is_set(Traf) && ieq_file) 
	{
		/* INEQUALITIES TO POINTS */
		RAT *inner,*iep;
		char *cp1;
		cp1=strdup("VALID");
		inner = 0;
		if(!cp1)
                        msg("allocation of new space failed");   
			
		points = read_input_file(*argv,outfp,&dim,&ar1,
								 &nel_ar1,"\0",&i,"\0",&i,cp1,&inner);
		free(cp1);
		ar6 = inner; if (inner) nel_ar6 = dim;
		sort_eqie_cvce(ar1,points,dim+2,&equa_in,&ineq_in);
		iep = ar1+equa_in*(dim+2);
		/* first equations then inequalities */
		points = ineq_in;
		polarformat(iep,&equa_in,ineq_in,inner);
		gentableau(iep,0,&rowl_inar,&indx);
                if(is_set(Long_arithmetic)) {
		  SET_MP_ready;
	          arith_overflow_func(0,0,0,0,0);
                }
		ineq = (cone == points) ? dim : dim + 1;
		ineq_out = ineq; /* not used further */
		gauss(1, points+dim+1,dim+1,dim,ineq,&ineq_out, &equa, indx);
		/* make indx point to the x-variable section */
		for (; (*indx) < 0; indx++);
		fourier_motzkin(0,ineq-equa,points+dim+1-ineq,
						points-ineq+equa,poi_file,indx,0);
		if (is_set(Validity_table_out)) 
			red_test(indx,iep,&rowl_inar);
		if (cone >= dim-equa)
			origin_add(rowl_inar,iep); 
		resubst(inner,equa_in,indx);
		if ((MP_realised && return_from_mp()) || !MP_realised) 
		{
			if (equa)
				sort(no_denom(dim+1, ineq, ineq+equa,1), 
					 dim+1,ineq,ineq+equa);
			sort(0, dim+1, 0, ineq);
		}
		for (cone = 0; !(list[cone]->sys[dim].num); cone++);
		conv = ineq - cone;
		if (!MP_realised) no_denom(dim+1, 0, cone,1);
		write_poi_file(*argv,outfp,dim,equa,ineq,cone,0,conv,cone);
	}
	else 
		msg("invalid format of command line");

	/* 17.01.1994: include logging on file porta.log */
	fclose( logfile );
	exit(0);
}


			
/*****************************************************************/

int * check_and_reorder_elim_ord(eo,nel)
	 int *eo,*nel;

/*****************************************************************/
/*
 * This routine is not needed any more.
 *
 * invert the elimination order:
 * The "invertion" h satisfies:
 * h[5-1] = 3-1		if 3 is the 5th variable to be eliminated
 * etc. up to h[*nel-1], where *nel is the number of variables to be eliminated
 * h[*nel]   = number of first variable not to be elim.
 * h[*nel+1] = number of second variable not to be eliminated,
 * etc.
 */

{
	int *h,i,j;
	char *allo();
	
	if (!eo)
		msg ("Need 'ELIMINATION_ORDER' to eliminate variables");
	
	h  = (int *) allo(CP 0,0,(dim+1)*sizeof(int));
	
	j = h[dim] = dim;
	for (i = dim-1; i >= 0; i--) 
		if (!(eo)[i])
			h[--j] = i;
	
	*nel = 0;
	do 
	{
		for (i = 0; i < dim; i++)
			if ((eo)[i] == *nel+1) 
			{
				h[(*nel)++] = i;
				(eo)[i] = 0;
				break;
			}
		if (i == dim) 
			break;
	} 
	while (1);
	
	for (i = 0; i < dim; i++)
		if ((eo)[i])
			msg ("Invalid format of 'ELIMINATION_ORDER' line");
	
	allo(CP eo, dim*sizeof(int),0);
	return(h);
	
}


/*****************************************************************/

int intcompare(i,j)
	 int *i,*j;

/*****************************************************************/

{
	
	return(*i - *j);
	
}

int *elim_in;

/*****************************************************************/

void polarformat(inieq,equa_in,ineq_in,inner)
	 int *equa_in,ineq_in;
	 RAT *inner,*inieq;

/*****************************************************************/
	 
{
	
	int i,j,col,row,itr,pivcol,sysrow,  rowcomp();
	RAT *o,*n,*pivot;
	void vecpr();
	char *allo();
	
	sysrow = dim + 2;
	
	if (inner != 0) 
	{
		for (i = 0; i < *equa_in; i++) 
		{
			vecpr(inner,ar1+i*sysrow,var+3,dim); 
			(*RAT_sub)(*(ar1+(i+1)*sysrow-2),var[3],var+3);
			if (var[3].num)
				msg("%sinput point not valid (equalnum : %i)","",i+1);
		}
		for (i = 0; i < ineq_in; i++) 
		{
			vecpr(inner,inieq+i*sysrow,var+3,dim); 
			(*RAT_sub)(*(inieq+(i+1)*sysrow-2),var[3],var+3);
			if (var[3].num < 0)
				msg("%sinput point not valid (inequalnum : %i)","",i+1);
		}
		
	}
	
	if (*equa_in) 
	{  
		
		fprintf(prt,"Gauss - elimination (input - equations) :\n" );
		
		/* 17.01.1994: include logging on file porta.log */
		log( "Gauss - elimination (input - equations) :\n" );
		
		for (i = 0; i < *equa_in+ineq_in; i++) 
		{
			allo_list(i,0,0);
			list[i]->sys = ar1+i*(dim+2);
		}
		
		/* ELIMINATE INPUT - EQUALITIES */
		
		elim_in = (int *) allo(CP elim_in,1,U (dim+1)*sizeof(int));
		for (col = 0; col < dim; col++)
			*(elim_in+col) =  col ;
		
		for (itr = 0; itr < *equa_in; itr++) 
		{
			
			pivot = list[itr]->sys;
			for(pivcol = 0; pivcol < dim && !pivot->num; pivcol++,pivot++);
			
			if (pivcol == dim) 
				if (!(pivot->num)) 
				{
					
					(*RAT_row_prim)(list[*equa_in-1]->sys,list[itr]->sys, pivot,dim+1);
					(*equa_in)--;
					itr--;
					continue;
				}
				else
					msg("input equality system has no solution");
			
			(*RAT_row_prim)(list[itr]->sys,list[itr]->sys, pivot,dim+1);
			
			for (row = itr+1; row < *equa_in+ineq_in; row++)  
				if (row != itr) {
				gauss_calcnewrow(list[itr]->sys,list[row]->sys,pivcol,list[row]->sys,0,sysrow-1);
				(*RAT_row_prim)(list[row]->sys,list[row]->sys,list[row]->sys+dim,dim+1);
			}
			
			fprintf(prt," elimination of variable %d\n", pivcol+1);

			/* 17.01.1994: include logging on file porta.log */
			log( " elimination of variable %d\n", pivcol+1);
			
			for (col = 0; *(elim_in+col) != pivcol ; col++);
			for (; col < dim-itr-1 ; col++)
				*(elim_in+col) = *(elim_in+col+1);
			for (j = dim-itr-1; j < dim-1  ; j++)
				*(elim_in+j) = *(elim_in+j+1);
			*(elim_in+dim-1) = pivcol;
			
		}
		fprintf(prt,"\n" );

		/* 17.01.1994: include logging on file porta.log */
		log( "\n" );
	} 
	
	if (inner) 
		for (i = 1; i <= ineq_in; i++) 
		{
			vecpr(inner,inieq+(i-1)*sysrow,var+3,dim);
			(*RAT_sub)(*(inieq+i*sysrow-2),var[3],inieq+i*sysrow-2);
		}
	
	if (*equa_in > 0) 
	{
		dim -= *equa_in;
		n = o = inieq;
		for (row = 0; row < ineq_in; row++) 
		{
			for (col = 0; col < dim; col++)
				(*RAT_assign)(n++,inieq+row*sysrow+*(elim_in +col));  
			(*RAT_assign)(n++,inieq+(row+1)*sysrow-2);  
			(*RAT_assign)(n++,inieq+(row+1)*sysrow-1);  
		}
		sysrow -= *equa_in;
	}
	
	if (!inner) 
	{
		for (i = 1; i <= ineq_in; i++)
			if ((inieq+i*sysrow-2)->num < 0) 
				
				msg("no valid point known ");
		
	}
	
	
    /* SCALE LEFT SIDE TO ONE */
	
    for (o = n = inieq,i = 0; i < ineq_in; o += dim+2, n += dim+1, i++) 
		(*RAT_row_prim)(o,n,o+dim,dim+1);
	
}


/*****************************************************************/

void resubst(inner,equa_in,indx)
	 RAT *inner;
	 int equa_in;
	 int indx[];
	 
/*****************************************************************/
	 
{
    int sysrow,ie,j;
	RAT *new_sys;
	static void backwsubst(),writelistsys(),blow_up();
	
    new_sys = ar3 + (equa+ineq)*(dim+equa_in+1);
    while (new_sys > ar3+nel_ar3-1) 
	{
		reallocate(ineq, &new_sys);
        new_sys = ar3 + (equa+ineq)*(dim+equa_in+1);
    }
	
    if (equa) 
	{
		/* inequality dimension to dim */
		new_sys = ar3 + ineq*(dim+1);
		for (ie = ineq-1; ie >= 0; ie--, new_sys -= dim+1) 
			blow_up(new_sys,ie,indx,dim-equa,dim);
    } 
	
    if (equa_in) 
	{
		/* append equations on sys */
		new_sys = ar3 + (ineq+equa)*(dim+equa_in+1);
		for (ie = equa+ineq-1; ie >= 0; ie--, new_sys -= (equa_in+dim+1)) 
			blow_up(new_sys,ie,elim_in,dim,dim+equa_in);
    } 
	
	if (inner != 0) 
	{   
		for (ie = 0; ie < ineq; ie++)
			if ((list[ie]->sys+dim+equa_in)->num)
				for (j = 0; j < dim+equa_in; j++)
					(*RAT_add)(*(list[ie]->sys+j),*(inner+j),list[ie]->sys+j);
	} 
	
	if (equa_in > 0) 
	{ 
		
		fprintf(prt,"solving linear equality system ");
		fflush(prt);
				
		/* 17.01.1994: include logging on file porta.log */
		log( "solving linear equality system ");
		fflush(logfile);
		
		sysrow = equa_in + dim + 2;
		for (ie = 0; ie < ineq+equa; ie++) 
		{
			if (ie%50 == 0) 
			{
				fprintf(prt,".");
				fflush(prt);
		   
				/* 17.01.1994: include logging on file porta.log */
				log( ".");
				fflush(logfile);
			}
			backwsubst(list[ie]->sys,sysrow,equa_in);
		}
		
		dim += equa_in;
		fprintf(prt,"\n");

		/* 17.01.1994: include logging on file porta.log */
		log( "\n");
	}
	
}

/*****************************************************************/

void blow_up(nptr,ie,el,eldim,fdim)
	 RAT *nptr;
	 int ie,*el,eldim,fdim;
	 
/* blows up to get space for the eliminated components */
/*****************************************************************/

{
	int elp,not_elp,col,i;
	
	elp = fdim-1;
	not_elp = eldim-1;
	
	(*RAT_assign)(nptr,(list[ie]->sys+eldim));
	nptr--;  
	for (col = fdim-1; col >= 0 ; col--) 
	{
		for (i = eldim; i < fdim; i++)
			if (el[i] == col)
				break;
		if (i < fdim) 
		{
			(*RAT_assign)(nptr,RAT_const);
			nptr--;
			if (elp > eldim) 
				elp--;
		}
		else  
		{
			(*RAT_assign)(nptr,list[ie]->sys+not_elp);
			nptr--;
			not_elp--;
		}
	}
	list[ie]->sys = nptr+1;
	
}
/*****************************************************************/

void backwsubst(ieptr,sysrow,equa_in)
	 RAT *ieptr;
	 int sysrow,equa_in;

/* computed the values of the components eliminated by the input equals
*/
/*****************************************************************/

{
	int j,col,i,ind;
	RAT *rptr;
	
    for (i = 0; i < equa_in; i++) 
	{
		rptr = ar1+i*sysrow;
		(*RAT_assign)(rptr+sysrow-1,RAT_const);
		for (j = 0; j < dim; j++) 
		{ 
			col = *(elim_in+j);
			(*RAT_mul)(*(ieptr+col), *(rptr+col), var+3);
			(*RAT_add)(rptr[sysrow-1], var[3], rptr+sysrow-1);
		}
		if (ieptr[dim+equa_in].num)
			(*RAT_sub)(rptr[sysrow-2],rptr[sysrow-1], rptr+sysrow-1);
		else
			(rptr+sysrow-1)->num *= -1;
    }
	
    for (i = equa_in-1; i >= 0; i--) 
	{
		rptr = ar1+i*sysrow;
		(*RAT_assign)(var+2,RAT_const);
		for (j = i+1; j < equa_in; j++) 
		{ 
			col = *(elim_in+dim+j);
			(*RAT_mul)( rptr[col],*(ieptr+col), var+3);
			(*RAT_add)( var[2], var[3], var+2);
		}
		ind = *(elim_in+dim+i);
		(*RAT_sub)(rptr[sysrow-1],var[2],ieptr+ind );
		(*RAT_mul)(*(ieptr+ind),*(rptr+ind),(ieptr+ind));
    }
	
}

writemat(ptr,row,col)
	 int row,col;
	 RAT *ptr;
	 
{ 
	int i,j;
	
	for(i = 0; i<row; i++) 
	{
		for(j=0; j<col; j++,ptr++)
		{
			fprintf(prt,"%i/%i ",ptr->num,ptr->den);
			
			/* 17.01.1994: include logging on file porta.log */
			log( "%i/%i ",ptr->num,ptr->den);
		}
		fprintf(prt,"\n");

		/* 17.01.1994: include logging on file porta.log */
		log( "\n");
	}
	fprintf(prt,"\n");
	
	/* 17.01.1994: include logging on file porta.log */
	log( "\n");
}


/*****************************************************************/

int no_denom(sysrow,first,last,outmsg)
	 int sysrow,first,last;
	 
/*****************************************************************/
/*
 * Make a fractional point integer by multiplying it with a positive number
 * that is as small as possible.
 */
{
	char *allo();
	int ret=1,scm,old_scm,i,j,ie,*denom,gcdrow();
	
	denom = (int *) allo(CP 0,0,U sysrow*sizeof(int));
	
	if(outmsg)
	{ 
		fprintf(prt,"transformation to integer values ");

		/* 17.01.1994: include logging on file porta.log */
		log( "transformation to integer values ");
	}
	
	for (ie = first; ie < last; ie++) 
	{
		
		for (i = 0; i < sysrow; i++)
			denom[i] = (list[ie]->sys+i)->den.i;
		
		qsort(CP denom,sysrow,sizeof(int),intcompare);
		old_scm = scm = denom[0];
		for (i = 0,j = 0; i < sysrow; i++)
			if (i > 0 && denom[i] != denom[i-1]) 
			{
				scm = old_scm*denom[i];
				if (scm/denom[i] != old_scm) 
				{
					ret = 0;
					break;
				}
				else
					old_scm = scm;
				denom[j++] = denom[i];
			}
		if (i != sysrow)
			continue;
		for (i = 0; i < sysrow; i++)
			denom[i] = scm/denom[i];
		scm = scm/gcdrow(denom,j);
		
		(list[ie]->sys+sysrow-1)->num *= scm;
		for (i = 0; i < sysrow-1; i++) 
		{
			(list[ie]->sys+i)->num = (scm/(list[ie]->sys+i)->den.i)*(list[ie]->sys+i)->num;
			(list[ie]->sys+i)->den.i = 1;
		}
		
	}
	
	if(outmsg)
	{
		fprintf(prt,"\n");

		/* 17.01.1994: include logging on file porta.log */
		log( "\n");
	}	
	
	cfree(denom);
	return(ret);
	
}


/*****************************************************************/

void origin_add(rowl_inar,inieq)
	 int rowl_inar;
	 RAT *inieq;

/*****************************************************************/

{
	int i,j;
	unsigned *m;
	listp lp;
	
	m = 0;
	for (i = 0; i < ineq; i++) 
	{
		for (j = 0; j < dim-equa; j++)
			if (( (list[i]->sys)+j )->num)
				break;
		if (j == dim-equa && ( (list[i]->sys)+j )->num) 
		{     
			m = list[i]->mark;
			break;
		}
	}
	
	if (!m && cone >= dim-equa) 
	{
		m = (unsigned *) 1;
		allo_list(ineq+equa,&m,points/32+2);
		
		lp = list[ineq+equa];
		for (j = ineq+equa; j != ineq; j--)
			list[j] = list[j-1];
		list[ineq] = lp;
		/* 
		 * Change by M.S. 5.6.92:
		 * A bus error occurred in the following line with the .ieq file:
		 * DIM=1
		 * INEQUALITIES_SECTION
		 * x1=0
		 * with or without VALID points.
		 * ineq is 0 in this case.
		 if (list[ineq-1]->sys+2*(dim+1-equa) > ar3+nel_ar3-1)
		 reallocate(ineq, &(RAT *) i);
		 
		 list[ineq]->sys = list[ineq-1]->sys+dim+1-equa;
		 */
		if (ineq) 
		{
			if (list[ineq-1]->sys+2*(dim+1-equa) > ar3+nel_ar3-1)
				reallocate(ineq, &i);
			list[ineq]->sys = list[ineq-1]->sys+dim+1-equa;
		}
		else 
		{
			if (!ar3) 
			{
				nel_ar3 = FIRST_SYS_EL+ dim+1-equa;
				ar3 = (RAT *) RATallo(CP ar3,0,U nel_ar3);
			}
			else if (ar3+dim+1-equa > ar3+nel_ar3-1)
				reallocate(ineq, &i);
			list[ineq]->sys = ar3;
		}

		for (j = 0; j < dim-equa; j++) 
			(*RAT_assign)(list[ineq]->sys+j,RAT_const);
		(*RAT_assign)(list[ineq]->sys+j,RAT_const+1);
		ineq++;
	}
	
	if (m) 
	{
		for (i=0; i< points/32+2; i++)
			m[i] = 0;
		for (i=1; i <= points; i++)
			if (!inieq[i*rowl_inar-1].num)
				domark(m,i-1);
	}
	
	
} 


/*****************************************************************/

void gentableau(ar1p,poi_to_ieq,rowl_inar,indx)
	 int poi_to_ieq;
	 RAT *ar1p;
	 int *rowl_inar;
	 int **indx;
	 
/*****************************************************************/
/*
 * Copy the (points * dim+1) matrix ar1 (where the rows are the points) 
 * to the (dim+1 * points+dim+1) matrix ar2 
 * transposed (so that the points are columns), 
 * append a negative identity matrix of dimension "dim",
 * and a right-hand side vector of (0,1) of size: (dim,1).
 * The last row of ar2 corresponds to an equation bx=1,
 * where b_p = 1 of the corresponding point is a conv-point,
 * and b_p = 0, if it is a cone_point.
 * ar2 actually contains space for dim+1 more rows 
 * for temporary usage in gauss.
 * Compute cone and conv, set indx[] and list[]->sys.
 */
{
	int sysrow,i,j;
	RAT *r_sp;
	char *allo();

	if (poi_to_ieq == 0) 
	{
		/* KH(0 (!) ,....) */
		/* Append a conv-point 0 to array ar1 */
		for (r_sp = ar1p+(dim+1)*points, i = 0; i < dim; r_sp++, i++) 
			(*RAT_assign)(r_sp,RAT_const);
		(*RAT_assign)(r_sp,RAT_const+1);
		points++;
	}
	
    sysrow = dim+points+1;
    *rowl_inar = dim+1;

    nel_ar2 = 2*(dim+1)*sysrow;
    ar2 = (RAT *) RATallo(CP ar2,1, nel_ar2);
	
    for (j = 1; j <= points; j++) 
	{
		for (i = 1; i<= dim; i++)
			ar2[(i-1)*sysrow+j-1] = ar1p[(j-1)*(dim+1)+i-1];
		ar2[dim*sysrow+j-1] = ar1p[(j-1)*(dim+1)+dim];
		if (ar1p[(j-1)*(dim+1)+dim].num) conv++; else cone++;
    }
    for (j = points+1; j <= sysrow; j++) 
		for (i = 1; i<= dim+1; i++) 
		{
			ar2[(i-1)*sysrow+j-1].num = (j-points == i) ? -1 : 0;
			ar2[(i-1)*sysrow+j-1].den.i = 1;
		}
    ar2[sysrow*(dim+1)-1].num = 1;
    ar2[sysrow*(dim+1)-1].den.i = 1;
	
    for (r_sp = ar2,i = 0; i <= 2*dim+1; r_sp += sysrow,i++) 
	{
		allo_list(i,0,0);
		list[i]->sys =  r_sp;
	}
	
	*indx = (int *) allo(CP (*indx),0,U (points+dim+1)*sizeof(int));
	for (i = 0; i < points; i++)
		(*indx)[i] =  -i-1 ;
	for (i = 0; i < dim; i++)
		(*indx)[points+i] =  i ;
	(*indx)[points+dim] = 0;
	
	equa=0;
	ineq=0;
}


/*****************************************************************/

void reorder_var(ineq,ar1,ar2,nel_ar2,nel,elim_ord,indx)
	 RAT *ar1, **ar2;
	 int *nel_ar2, *nel;
	 int **elim_ord, **indx;
	 
/*****************************************************************/
/*
 * Copy the (ineq * dim+1) matrix ar1 into the (2ineq * dim+1) matrix ar2,
 * so that the first variable to be eliminated is the first column of ar2,
 * the second variable is the second column, etc.
 * The extra "ineq" rows in ar2 are used for temporary storage in gauss.
 * Upon input, elim_ord[j-1] = i > 0 means that variable j
 * is the i-th variable to be eliminated.
 * elim_ord[j-1] = 0 means that variable j is not eliminated.
 * Upon output, elim_ord[0,1,...,dim] is set to 0,1,...,dim.
 * indx[] gives the original names of the variables (minus 1),
 *    which are made negative for the variables to be eliminated
 *    and nonnegative for the others.
 *    The negativity is used in gauss().
 * nel is the number of variables to be eliminated.
 */
{
    int sysrow,i,j,col,max;
    RAT *r1ptr, *r2ptr;
    char *allo();
	
    if (!(*elim_ord))
		msg ("Need 'ELIMINATION_ORDER' to eliminate variables");
	
    sysrow = dim+1;
	
    *nel_ar2 = 2*ineq*sysrow;
    *ar2 = (RAT *) RATallo( CP (*ar2), 0, *nel_ar2);
    
    /* copy columns belonging to variables that will be eliminated */
	
    *nel = 0;	/* number of variables to be eliminated		     */
    max = 0;    /* highest number occurring in the elimination order */
    for (i = 0; i < dim; i++) 
	{
		col = (*elim_ord)[i];
		if (!col) 
			continue;
		if (col > max) 
			max = col;
		if (col < 0) 
			msg ("Invalid format of 'ELIMINATION_ORDER' line");
		col = col-1;
		r1ptr = ar1+i;
		r2ptr = *ar2+col;
		for (j = 0; j < ineq; j++ ) 
		{
			(*RAT_assign)(r2ptr,r1ptr);
			r1ptr += dim + 2;
			r2ptr += dim + 1;
		}
		(*nel)++;
    }
    if (max != *nel) 
		msg ("Invalid format of 'ELIMINATION_ORDER' line");
	
    /* copy columns belonging to variables that will not be eliminated */
    col = *nel;		/* number of variables to eliminate */
    for (i = 0; i < dim; i++) 
	{
		if ((*elim_ord)[i]) 
			continue;
		r1ptr = ar1+i;
		r2ptr = *ar2+col;
		for (j = 0; j < ineq; j++ ) 
		{
			(*RAT_assign)(r2ptr,r1ptr);
			r1ptr += dim + 2;
			r2ptr += dim + 1;
		}
		col++;
    }
	
    /* Copy the right-hand side vector */
    r1ptr = ar1+dim;
    r2ptr = *ar2+dim;
    for (j = 0; j < ineq; j++ ) 
	{
		(*RAT_assign)(r2ptr,r1ptr);
		r1ptr += dim + 2;
		r2ptr += dim + 1;
    }
    
	/* Assign list[]->sys */
    r2ptr = *ar2;
    for (i = 0; i <= 2*ineq; i++) 
	{
		allo_list(i,0,0);
		list[i]->sys = r2ptr;
		r2ptr += sysrow;
    }

    /* indx */
    *indx = (int *) allo(CP (*indx),1,U (dim+1)*sizeof(int));
    for (i = 0; i < dim+1; i++) 
		(*indx)[i] = 0;
    j = *nel;
    for (i = 0; i < dim; i++) 
	{
        col = (*elim_ord)[i];
		if (col) 
			(*indx)[col-1] = -i-1;
		else 
			(*indx)[j++] = i;
    }
    (*indx)[dim] = 0;
    /* no "holes" in indx for the first "nel" elements? */
    for (i = 0; i < *nel; i++)
		if ((*indx)[i] == 0)
			msg ("Invalid format of 'ELIMINATION_ORDER' line");
	    
    /* elim_ord */
    *elim_ord = (int *) allo( CP (*elim_ord),U (dim*sizeof(int)),
							 U ((dim+1)*sizeof(int)));
    for (i = 0; i < dim+1; i++)
		(*elim_ord)[i] = i;
}


/*****************************************************************/

reallocate(ie,sysptr)
	 int ie;
	 RAT **sysptr;
	 
/*****************************************************************/
/*
 * enlarge array "ar3",
 * and make the pointers list[]->sys, list[]->ptr, and sysptr
 * point to their respective new positions.
 * "ie" is the current number of inequalities in Fourier_Motzkin,
 * "sysptr" contains a pointer to some place in "ar3".
 */
{
	int i,*save_sys,*save_ptr,save_sp;
	char *allo();
	
	save_sys = (int *) allo(CP 0,0,U ie*sizeof(int));
	save_ptr = (int *) allo(CP 0,0,U ie*sizeof(int));
	
	save_sp = (*sysptr == 0) ? -1 : (*sysptr - ar3);
	for (i = 0; i < ie; i++) 
	{
		if( list[i]->sys == (RAT *)0 )
			save_sys[i] = -1;
		else
			save_sys[i] = list[i]->sys - ar3;
		
		if( list[i]->ptr == (RAT *)0 )
			save_ptr[i] = -1;
		else
			save_ptr[i] = list[i]->ptr - ar3;
	}
	
	nel_ar3 += INCR_SYS_EL;
	ar3 = (RAT *) RATallo(CP ar3,nel_ar3-INCR_SYS_EL,U nel_ar3);
	/* fprintf(prt,"New space allocated \n"); */
	
	/* 
	 * "ar3" has maybe changed its value, 
	 * therefore sysptr has to be recomputed as ar3[save_sp],
	 * and list[i]->sys and list[i]->ptr also have to be recomputed,
	 * so they have the same offset in ar3 as before.
	 */
	*sysptr = (save_sp == -1) ? 0 : (ar3 + save_sp);
	for (i = 0; i < ie; i++) 
	{
		list[i]->sys =  (save_sys[i] == -1) ? 0 : (ar3 + save_sys[i]);
		list[i]->ptr =  (save_ptr[i] == -1) ? 0 : (ar3 + save_ptr[i]);
	}
	
	/*
	 * Change by M.S. 3.6.1992:
	 * Free save_sys and save_ptr by using allo(), not cfree().
	 *
	 cfree(save_sys);
	 cfree(save_ptr);
	 */
	allo(CP save_sys,U ie*sizeof(int), 0);
	allo(CP save_ptr,U ie*sizeof(int), 0);
}


#include <sys/times.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>



/*****************************************************************/

float time_used()
	 
/*****************************************************************/

{
	struct tms now;
	int sec, hun;
	float total;

#ifndef CLK_TCK
	int clocks_per_second = 0;
	
	clocks_per_second = sysconf( _SC_CLK_TCK );
	times( &now );
	
	hun  = ( ( now.tms_utime % clocks_per_second ) * 100 ) / clocks_per_second;
	sec = ( now.tms_utime / clocks_per_second );
#else
	times( &now );

	hun  = ( ( now.tms_utime % CLK_TCK ) * 100 ) / CLK_TCK;
	sec = ( now.tms_utime / CLK_TCK );
#endif

	total = (float)sec + ( (float)(hun) / 100.0 );
	
	return( total );
}

     
static double initial_time;
/*****************************************************************/

void init_total_time()
	 
/*****************************************************************/

{
	struct timeval tp;
	struct timezone tzp;
	
	gettimeofday( &tp, &tzp );
	initial_time = (double)(tp.tv_sec) 
		+ 0.000001 * (double)(tp.tv_usec);
}
/*****************************************************************/

float total_time()
	 
/*****************************************************************/

{
	struct timeval tp;
	struct timezone tzp;
	
	gettimeofday( &tp, &tzp );
	return (double)(tp.tv_sec) 
		+ 0.000001 * (double)(tp.tv_usec)
		- initial_time;
}
