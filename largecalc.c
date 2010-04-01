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
 

  NAME: largecalc.c 
  
  LAST MODIFICATION DATE: 02.12.1993
  
  AUTHOR: Thomas Christof
  
  REVISED BY ANDREAS LOEBEL & MECHTHILD STOER
             KONRAD-ZUSE-ZENTRUM FUER
             INFORMATIONSTECHNIK BERLIN
             HEILBRONNER STR. 10
             D-10711 BERLIN-WILMERSDORF

 *****************************************************************/
/*****************************************************************/

#include "porta.h"

#define bits	12
#define B	4096  /* 2^bits */
#define m0_11 	4095
#define mask 	m0_11
#define m0_9 	1023
#define m10_19 	1047552
#define m0_3 	15
#define m0_7 	255
#define m_31 	(1 << 31)

#define lord(la,lb) (lorder(la.val,lb.val,la.len,lb.len))

/*
   void msg()
   {printf("exit\n");
   exit(0);}*/

/*****************************************************************/

void lsub(la,lb,lc)
	 loint la,lb,*lc;
	 
/*****************************************************************/

{
	int ord;
	void lsubber(),ladder();
	
	if (!la.len) 
	{
		/* lc = -lb */
		*lc = lb;
		lc->neg = !lb.neg;
	}
	else if (!lb.len)
		/* lc = la */
		*lc = la;
	else if (la.neg ^ lb.neg) 
	{
		/* sign(la) <> sign(lb) */	
		ladder(la.val,lb.val,lc->val,la.len,lb.len,&lc->len);
		lc->neg = la.neg;
	}
	else 
	{
		ord = lord(la,lb);
		lc->neg = (la.neg) ?  (ord > 0) : (ord < 0);
		if (ord < 0)
			lsubber(lb.val,la.val,lc->val,lb.len,la.len,&lc->len);
		else if (ord > 0)
			lsubber(la.val,lb.val,lc->val,la.len,lb.len,&lc->len);
		else
			lc->len = 0;
	}
}
    

/*****************************************************************/

void lsubber(pla,plb,plc,alen,blen,clen)
	 int alen,blen,*clen;
	 unsigned *pla,*plb,*plc;
	 
/*****************************************************************/

{
	int i;
	unsigned c,nextc;
	unsigned lai;
	
	i = 0;
	c = nextc = 0;
	while (i < blen)
	{
        lai = *pla;
        if (lai < (*plb + c))
        {
			lai += B;
			nextc  = 1;
        }
        *plc = lai - (*plb + c);
        c = nextc;
        nextc = 0;
        pla++;
        plb++;
        plc++;
        i++;
	}
	while ((c == 1) && (i < alen))
	{
        lai  = *pla;
        if (lai == 0)
        {
			lai = B;
			nextc = 1;
        }
        *plc = lai - c;
        c = nextc;
        nextc = 0;
        pla++;
        plc++;
        i++;
	}
	while (i < alen)
	{
        *plc = *pla;
        plc++;
        pla++;
        i++;
	}
	do 
	{
        i--;
        plc--;
	}
	while ((i >= 0) && (*plc == 0));
	
	*clen = i+1;
}

/*****************************************************************/

void ladd(la,lb,lc)	
	 loint la,lb,*lc;

/*****************************************************************/

{
	int ord;
	void lsubber(),ladder();
	
	if (!la.len)
		/* lc = lb */
		*lc = lb;
	else if (!lb.len)
		/* lc = la */
		*lc = la;
	else if (!(la.neg ^ lb.neg)) 
	{
		/* sign(la) = sign(lb) */	
		lc->neg = la.neg;
		if (la.len == MAX_LEN_LINT || lb.len == MAX_LEN_LINT)
			msg("Arithmetic overflow !");
		ladder(la.val,lb.val,lc->val,la.len,lb.len,&lc->len);
	}
	else 
	{
		ord = lord(la,lb);
		lc->neg = (la.neg) ?  (ord > 0) : (ord < 0);
		if (ord < 0)
			lsubber(lb.val,la.val,lc->val,lb.len,la.len,&lc->len);
		else if (ord > 0)
			lsubber(la.val,lb.val,lc->val,la.len,lb.len,&lc->len);
		else
			lc->len = 0;
	}
}

/*****************************************************************/

void ladder(pla,plb,plc,alen,blen,clen)
	 int alen,blen,*clen;
	 unsigned *pla,*plb,*plc;

/*****************************************************************/

{
	unsigned c;
	unsigned sum;
	int i;
	
	i = 1;
	c = 0;
	while (i <= alen && i <= blen)
	{
        sum  = *pla + *plb + c;
        c    = sum >> bits;
        sum  = sum & mask;
        *plc = sum;
        i++;
        pla++;
        plb++;
        plc++;
	}
	while (i <= alen)
	{
        sum  = *pla + c;
        c    = sum >> bits;
        sum  = sum & mask;
        *plc = sum;
        i++;
        pla++;
        plc++;
	}
	while (i <= blen)
	{
        sum  = *plb + c;
        c    = sum >> bits;
        sum  = sum & mask;
        *plc = sum;
        i++;
        plb++;
        plc++;
	}
	if (c == 1)
	{
        *plc = 1;
        i++;
	}
	*clen = i - 1;
	
}

/*****************************************************************/

lmul(la,lb,lc)
	 loint la,lb,*lc;
	 
/*****************************************************************/
	 
{ 
	void lmuller();
	
	if (!la.len || !lb.len)
		lc->len = 0;
	else
	{
		lc->neg = (la.neg ^ lb.neg);
		if ((la.len+lb.len) > MAX_LEN_LINT-2)
			msg("Arithmetic overflow !");
		lmuller(la.val,lb.val,lc->val,la.len,lb.len,&lc->len);
	}
}

/*****************************************************************/

void lmuller(pa,pb,erg,alen,blen,erglen)
	 unsigned *pa,*pb,*erg;
	 int alen,blen,*erglen;

/*****************************************************************/
	 
{
	int i,j,clen;
	unsigned *perg,*ppb;
	int c;
	
    clen = alen+blen;
    for (i=0;i <= clen;i++)
		erg[i] = 0;
    for (i = 0; i < alen; i++)
    {
		perg = erg+i;
		ppb = pb;
		for (j=1; j <= blen; j++)
		{
			*perg += (*pa) * (*ppb);
			perg++;
			ppb++;
		}   /* for j */
      pa++;
    }   /* for i */
    perg = erg;
    c    = 0;
    for (i=1;i<=clen;i++)
    {
		*perg += c;
		c      = (*perg) >> bits;
		*perg = (*perg) & mask;
		perg++;
    }
    while ((clen >= 0) && (erg[clen] == 0))
		clen--;
    *erglen = clen+1;
	
}


/* 02.12.1993: I have changed the identifier ldiv to porta_ldiv,
               because there exists a stdlib-function ldiv on
			   system-V-platforms */
/*****************************************************************/

void porta_ldiv(la,lb,lc,lr)
	 loint la,lb,*lr,*lc;

/*****************************************************************/

{
	int i,blen,rlen,clen,xlen;
	unsigned *plr,*plb,*plc,bval,r,x[MAX_LEN_LINT];
	
	lc->len = 0;
	*lr = la;
	if ((i = lord(la,lb)) < 0)
		return;
	else if (!i) 
	{
		/* la = lb */
		lc->len = 1;
		lc->val[0] = 1;
		lr->len = 0;
		return;
	}
	
	blen = rlen = lb.len;
	plr = lr->val+lr->len-blen;
	plb = lb.val;
	plc = lc->val+la.len;
	clen = 0;
	
	if (blen == 1) 
	{
		bval = *plb;
		r = 0;
		for (; plr >= lr->val; plr--, clen++) 
		{
			r |= *plr;
			*(--plc) = r/bval;
			r = ((r%bval) << bits);
		}
		lr->val[0] = (r >> bits);
	}
	else 
	{
		rlen--;
		bval = plb[blen-1] + 1;
		for ( ;plr >= lr->val; plr--,clen++) 
		{
			rlen++;
			if (lorder(plr,plb,rlen,blen) < 0)
				*(--plc) = 0;
			else 
			{
				r = plr[rlen-1];
				if (r < bval && rlen > blen)
					r = (r << bits) | plr[rlen-2];
				/* printf("0:%i %i \n",r,bval);*/
				*(--plc) = r/bval;
				/* printf("1:%i %i %i %i \n",r/bval,*plc,blen,rlen); */
				lmuller(plc,plb,x,1,blen,&xlen);
				lsubber(plr,x,plr,rlen,xlen,&rlen);
				while (!(lorder(plr,plb,rlen,blen) < 0)) 
				{
					lsubber(plr,plb,plr,rlen,blen,&rlen);
					(*plc)++;
				}
			}
		}
	}
	
	for (i = 0; i < clen; i++)
		lc->val[i] = plc[i];
	lc->len = clen;
	
	for (;rlen && (lr->val[rlen-1] == 0); rlen--);
	for (;clen && (lc->val[clen-1] == 0); clen--);
	lr->len = rlen;  lc->len = clen;
}

/*****************************************************************/

int lorder(x,y,xlen,ylen)	
	 unsigned *x,*y;
	 int xlen,ylen;

/*****************************************************************/

{
	int d,i;
	
	if ((d = xlen - ylen) > 0)
		return(1);
	if (d < 0)
		return(-1);
	for (i = xlen-1; i >= 0; i--) 
	{
		if ((d = x[i] - y[i]) > 0)
			return (1);
		if (d < 0)
			return (-1);
	}
	return (0);
}

/*****************************************************************/

void lgcd(a,b,c)
	 loint a,b,*c;

/*****************************************************************/

{ 
	
	loint r;
	if (a.len == 0) 
	{
		*c = b;
		return;
	}
	
  if (lord(a,b) == -1) 
  {
	  r = a;
	  a = b;
	  b = r;
  }
	
	porta_ldiv(a,b,&a,&r);
	while (r.len > 0) 
	{
		a = b;
		b = r;
		porta_ldiv(a,b,&a,&r);
	}
	*c = b;
}







