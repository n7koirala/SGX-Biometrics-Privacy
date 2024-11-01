/*******************************************************************************

License: 
This software and/or related materials was developed at the National Institute
of Standards and Technology (NIST) by employees of the Federal Government
in the course of their official duties. Pursuant to title 17 Section 105
of the United States Code, this software is not subject to copyright
protection and is in the public domain. 

This software and/or related materials have been determined to be not subject
to the EAR (see Part 734.3 of the EAR for exact details) because it is
a publicly available technology and software, and is freely distributed
to any interested party with no licensing requirements.  Therefore, it is 
permissible to distribute this software as a free download from the internet.

Disclaimer: 
This software and/or related materials was developed to promote biometric
standards and biometric technology testing for the Federal Government
in accordance with the USA PATRIOT Act and the Enhanced Border Security
and Visa Entry Reform Act. Specific hardware and software products identified
in this software were used in order to perform the software development.
In no case does such identification imply recommendation or endorsement
by the National Institute of Standards and Technology, nor does it imply that
the products and equipment identified are necessarily the best available
for the purpose.

This software and/or related materials are provided "AS-IS" without warranty
of any kind including NO WARRANTY OF PERFORMANCE, MERCHANTABILITY,
NO WARRANTY OF NON-INFRINGEMENT OF ANY 3RD PARTY INTELLECTUAL PROPERTY
or FITNESS FOR A PARTICULAR PURPOSE or for any purpose whatsoever, for the
licensed product, however used. In no event shall NIST be liable for any
damages and/or costs, including but not limited to incidental or consequential
damages of any kind, including economic damage or injury to property and lost
profits, regardless of whether NIST shall be advised, have reason to know,
or in fact shall know of the possibility.

By using this software, you agree to bear all risk relating to quality,
use and performance of the software and/or related materials.  You agree
to hold the Government harmless from any claim arising from your use
of the software.

*******************************************************************************/

/***********************************************************************
      LIBRARY: FING - NIST Fingerprint Systems Utilities

      FILE:           BOZORTH3.C
      ALGORITHM:      Allan S. Bozorth (FBI)
      MODIFICATIONS:  Michael D. Garris (NIST)
                      Stan Janet (NIST)
      DATE:           09/21/2004

      Contains the "core" routines responsible for supporting the
      Bozorth3 fingerprint matching algorithm.

***********************************************************************

      ROUTINES:
#cat: bz_comp -  takes a set of minutiae (probe or gallery) and
#cat:            compares/measures  each minutia's {x,y,t} with every
#cat:            other minutia's {x,y,t} in the set creating a table
#cat:            of pairwise comparison entries
#cat: bz_find -  trims sorted table of pairwise minutia comparisons to
#cat:            a max distance of 75^2
#cat: bz_match - takes the two pairwise minutia comparison tables (a probe
#cat:            table and a gallery table) and compiles a list of
#cat:            all relatively "compatible" entries between the two
#cat:            tables generating a match table
#cat: bz_match_score - takes a match table and traverses it looking for
#cat:            a sufficiently long path (or a cluster of compatible paths)
#cat:            of "linked" match table entries
#cat:            the accumulation of which results in a match "score"
#cat: bz_sift -  main routine handling the path linking and match table
#cat:            traversal
#cat: bz_final_loop - (declared static) a final postprocess after
#cat:            the main match table traversal which looks to combine
#cat:            clusters of compatible paths

***********************************************************************/

#include <stdio.h>
#include <bozorth.h>

int global_xcol[ MAX_BOZORTH_MINUTIAE ];
int global_ycol[ MAX_BOZORTH_MINUTIAE ];

int server_xcol[ MAX_BOZORTH_MINUTIAE ];
int server_ycol[ MAX_BOZORTH_MINUTIAE ];
int PASSED=0;

/***********************************************************************/
void bz_comp(
	int npoints,				/* INPUT: # of points */
	int xcol[     MAX_BOZORTH_MINUTIAE ],	/* INPUT: x cordinates */
	int ycol[     MAX_BOZORTH_MINUTIAE ],	/* INPUT: y cordinates */
	int thetacol[ MAX_BOZORTH_MINUTIAE ],	/* INPUT: theta values */

	int * ncomparisons,			/* OUTPUT: number of pointwise comparisons */
	int cols[][ COLS_SIZE_2 ],		/* OUTPUT: pointwise comparison table */
	int * colptrs[]				/* INPUT and OUTPUT: sorted list of pointers to rows in cols[] */
	)
{
int i, j, k;

int b;
int t;
int n;
int l;

int table_index;

int dx;
int dy;
int distance;

int theta_kj;
int beta_j;
int beta_k;

int * c;

for (i = 0; i < MAX_BOZORTH_MINUTIAE; i++) {
	if (PASSED == 0) {
		global_xcol[i] = xcol[i];
		global_ycol[i] = ycol[i];
	} else {
		server_xcol[i] = xcol[i];
		server_ycol[i] = ycol[i];
	}
}

PASSED++;

c = &cols[0][0];

table_index = 0;
for ( k = 0; k < npoints - 1; k++ ) {
	for ( j = k + 1; j < npoints; j++ ) {


		if ( thetacol[j] > 0 ) {

			if ( thetacol[k] == thetacol[j] - 180 )
				continue;
		} else {

			if ( thetacol[k] == thetacol[j] + 180 )
				continue;
		}


		dx = xcol[j] - xcol[k];
		dy = ycol[j] - ycol[k];
		distance = SQUARED(dx) + SQUARED(dy);
		if ( distance > SQUARED(DM) ) {
			if ( dx > DM )
				break;
			else
				continue;

		}

					/* The distance is in the range [ 0, 125^2 ] */
		if ( dx == 0 )
			theta_kj = 90;
		else {
			double dz;

			if ( m1_xyt )
				dz = ( 180.0F / PI_SINGLE ) * atanf( (float) -dy / (float) dx );
			else
				dz = ( 180.0F / PI_SINGLE ) * atanf( (float) dy / (float) dx );
			if ( dz < 0.0F )
				dz -= 0.5F;
			else
				dz += 0.5F;
			theta_kj = (int) dz;
		}


		beta_k = theta_kj - thetacol[k];
		beta_k = IANGLE180(beta_k);

		beta_j = theta_kj - thetacol[j] + 180;
		beta_j = IANGLE180(beta_j);


		if ( beta_k < beta_j ) {
			*c++ = distance;
			*c++ = beta_k;
			*c++ = beta_j;
			*c++ = k+1;
			*c++ = j+1;
			*c++ = theta_kj;
		} else {
			*c++ = distance;
			*c++ = beta_j;
			*c++ = beta_k;
			*c++ = k+1;
			*c++ = j+1;
			*c++ = theta_kj + 400;

		}






		b = 0;
		t = table_index + 1;
		l = 1;
		n = -1;			/* Init binary search state ... */




		while ( t - b > 1 ) {
			int * midpoint;

			l = ( b + t ) / 2;
			midpoint = colptrs[l-1];




			for ( i=0; i < 3; i++ ) {
				int dd, ff;

				dd = cols[table_index][i];

				ff = midpoint[i];


				n = SENSE(dd,ff);


				if ( n < 0 ) {
					t = l;
					break;
				}
				if ( n > 0 ) {
					b = l;
					break;
				}
			}

			if ( n == 0 ) {
				n = 1;
				b = l;
			}
		} /* END while */

		if ( n == 1 )
			++l;




		for ( i = table_index; i >= l; --i )
			colptrs[i] = colptrs[i-1];


		colptrs[l-1] = &cols[table_index][0];
		++table_index;


		if ( table_index == 19999 ) {
#ifndef NOVERBOSE
			if ( verbose_bozorth )
				printf( "bz_comp(): breaking loop to avoid table overflow\n" );
#endif
			goto COMP_END;
		}

	} /* END for j */

} /* END for k */

COMP_END:
	*ncomparisons = table_index;

}

/***********************************************************************/
void bz_find(
	int * xlim,		/* INPUT:  number of pointwise comparisons in table */
				/* OUTPUT: determined insertion location (NOT ALWAYS SET) */
	int * colpt[]		/* INOUT:  sorted list of pointers to rows in the pointwise comparison table */
	)
{
int midpoint;
int top;
int bottom;
int state;
int distance;



/* binary search to locate the insertion location of a predefined distance in list of sorted distances */


bottom   = 0;
top      = *xlim + 1;
midpoint = 1;
state    = -1;

while ( top - bottom > 1 ) {
	midpoint = ( bottom + top ) / 2;
	distance = *colpt[ midpoint-1 ];
	state = SENSE_NEG_POS(FD,distance);
	if ( state < 0 )
		top = midpoint;
	else {
		bottom = midpoint;
	}
}

if ( state > -1 )
	++midpoint;

if ( midpoint < *xlim )
	*xlim = midpoint;

/* limit the size to MAX_PAIR_TABLE_SIZE */
/* this will limit the midpoint to 500, limits  */
/* the pair table size to FDD=500 in ./bz_drvrs/bozorth_probe_init()  */
#define MAX_PAIR_TABLE_SIZE 32
if ( *xlim > MAX_PAIR_TABLE_SIZE )
	*xlim = MAX_PAIR_TABLE_SIZE;

}

/***********************************************************************/
/* Make room in RTP list at insertion point by shifting contents down the
   list.  Then insert the address of the current ROT row into desired
   location */
/***********************************************************************/
static

void rtp_insert( int * rtp[], int l, int idx, int * ptr )
{
int shiftcount;
int ** r1;
int ** r2;


r1 = &rtp[idx];
r2 = r1 - 1;

shiftcount = ( idx - l ) + 1;
while ( shiftcount-- > 0 ) {
	*r1-- = *r2--;
}
*r1 = ptr;
}

/***********************************************************************/
/* Builds list of compatible edge pairs between the 2 Webs. */
/* The Edge pair DeltaThetaKJs and endpoints are sorted     */
/*	first on Subject's K,                               */
/*	then On-File's J or K (depending),                  */
/*	and lastly on Subject's J point index.              */
/* Return value is the # of compatible edge pairs           */
/***********************************************************************/
int bz_match(
    int probe_ptrlist_len,      /* INPUT:  pruned length of Subject's pointer list */
    int gallery_ptrlist_len     /* INPUT:  pruned length of On-File Record's pointer list */
)
{
    int i;                      /* Temp index */
    int ii;                     /* Temp index */
    int edge_pair_index;        /* Compatible edge pair index */
    float dz;                   /* Delta difference and delta angle stats */
    float fi;                   /* Distance limit based on factor TK */
    int *ss;                    /* Subject's comparison stats row */
    int *ff;                    /* On-File Record's comparison stats row */
    int j;                      /* On-File Record's row index */
    int k;                      /* Subject's row index */
    int st;                     /* Starting On-File Record's row index */
    int p1;                     /* Adjusted Subject's ThetaKJ, DeltaThetaKJs, K or J point index */
    int p2;                     /* Adjusted On-File's ThetaKJ, RTP point index */
    int n;                      /* ThetaKJ and binary search state variable */
    int l;                      /* Midpoint of binary search */
    int b;                      /* ThetaKJ state variable, and bottom of search range */
    int t;                      /* Top of search range */

    register int *rotptr;

    #define ROT_SIZE_1 20000
    #define ROT_SIZE_2 5

    static int rot[ROT_SIZE_1][ROT_SIZE_2];
    static int *rtp[ROT_SIZE_1];

    /* These are now externally defined in bozorth.h */
    /* extern int *scolpt[SCOLPT_SIZE]; */           /* INPUT */
    /* extern int *fcolpt[FCOLPT_SIZE]; */           /* INPUT */
    /* extern int colp[COLP_SIZE_1][COLP_SIZE_2]; */ /* OUTPUT */
    /* extern int verbose_bozorth; */
    /* extern FILE *errorfp; */
    /* extern char *get_progname(void); */
    /* extern char *get_probe_filename(void); */
    /* extern char *get_gallery_filename(void); */

    printf("probe_ptrlist_len: %d\n", probe_ptrlist_len);
    printf("gallery_ptrlist_len: %d\n", gallery_ptrlist_len);

    st = 1;
    edge_pair_index = 0;
    rotptr = &rot[0][0];

    /* Define data structures to store the data previously written to files */
    #define MAX_PAIRS 20000

    typedef struct {
        int k;
        int ss3;
        int ss4;
        int x1;
        int y1;
        int x2;
        int y2;
    } MinutiaPair;

    MinutiaPair *minutia_pairs = (MinutiaPair *)malloc(sizeof(MinutiaPair) * MAX_PAIRS);
    int minutia_pairs_count = 0;

    typedef struct {
        int p1;
        int x1;
        int y1;
        int x2;
        int y2;
        int x3;
        int y3;
        int x4;
        int y4;
    } SimilarityPair;

    SimilarityPair *similarity_pairs = (SimilarityPair *)malloc(sizeof(SimilarityPair) * MAX_PAIRS);
    int similarity_pairs_count = 0;

    if (!minutia_pairs || !similarity_pairs) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return -1;
    }

    /* Foreach sorted edge in Subject's Web ... */
    for (k = 1; k < probe_ptrlist_len; k++) {
        ss = scolpt[k - 1];

        /* Store data in minutia_pairs array instead of writing to file */
        if (minutia_pairs_count < MAX_PAIRS) {
            MinutiaPair *pair = &minutia_pairs[minutia_pairs_count++];
            pair->k = k;
            pair->ss3 = ss[3];
            pair->ss4 = ss[4];
            pair->x1 = global_xcol[ss[3]];
            pair->y1 = global_ycol[ss[3]];
            pair->x2 = global_xcol[ss[4]];
            pair->y2 = global_ycol[ss[4]];
        } else {
            fprintf(stderr, "Error: Exceeded maximum number of minutia pairs.\n");
            break;
        }

        /* Foreach sorted edge in On-File Record's Web ... */
        for (j = st; j <= gallery_ptrlist_len; j++) {
            ff = fcolpt[j - 1];
            dz = *ff - *ss;

            fi = (2.0F * TK) * (*ff + *ss);

            if (SQUARED(dz) > SQUARED(fi)) {
                if (dz < 0) {
                    st = j + 1;
                    continue;
                } else {
                    break;
                }
            }

            for (i = 1; i < 3; i++) {
                float dz_squared;

                dz = *(ss + i) - *(ff + i);
                dz_squared = SQUARED(dz);

                if (dz_squared > TXS && dz_squared < CTXS)
                    break;
            }

            if (i < 3)
                continue;

            if (*(ss + 5) >= 220) {
                p1 = *(ss + 5) - 580;
                n = 1;
            } else {
                p1 = *(ss + 5);
                n = 0;
            }

            if (*(ff + 5) >= 220) {
                p2 = *(ff + 5) - 580;
                b = 1;
            } else {
                p2 = *(ff + 5);
                b = 0;
            }

            p1 -= p2;
            p1 = IANGLE180(p1);

            if (n != b) {
                *rotptr++ = p1;
                *rotptr++ = *(ss + 3);
                *rotptr++ = *(ss + 4);
                *rotptr++ = *(ff + 4);
                *rotptr++ = *(ff + 3);

                /* Store data in similarity_pairs array instead of writing to file */
                if (similarity_pairs_count < MAX_PAIRS) {
                    SimilarityPair *sim_pair = &similarity_pairs[similarity_pairs_count++];
                    sim_pair->p1 = p1;
                    sim_pair->x1 = global_xcol[ss[3]];
                    sim_pair->y1 = global_ycol[ss[3]];
                    sim_pair->x2 = global_xcol[ss[4]];
                    sim_pair->y2 = global_ycol[ss[4]];
                    sim_pair->x3 = server_xcol[ff[4]];
                    sim_pair->y3 = server_ycol[ff[4]];
                    sim_pair->x4 = server_xcol[ff[3]];
                    sim_pair->y4 = server_ycol[ff[3]];
                } else {
                    fprintf(stderr, "Error: Exceeded maximum number of similarity pairs.\n");
                    break;
                }
            } else {
                *rotptr++ = p1;
                *rotptr++ = *(ss + 3);
                *rotptr++ = *(ss + 4);
                *rotptr++ = *(ff + 3);
                *rotptr++ = *(ff + 4);

                /* Store data in similarity_pairs array instead of writing to file */
                if (similarity_pairs_count < MAX_PAIRS) {
                    SimilarityPair *sim_pair = &similarity_pairs[similarity_pairs_count++];
                    sim_pair->p1 = p1;
                    sim_pair->x1 = global_xcol[ss[3]];
                    sim_pair->y1 = global_ycol[ss[3]];
                    sim_pair->x2 = global_xcol[ss[4]];
                    sim_pair->y2 = global_ycol[ss[4]];
                    sim_pair->x3 = server_xcol[ff[3]];
                    sim_pair->y3 = server_ycol[ff[3]];
                    sim_pair->x4 = server_xcol[ff[4]];
                    sim_pair->y4 = server_ycol[ff[4]];
                } else {
                    fprintf(stderr, "Error: Exceeded maximum number of similarity pairs.\n");
                    break;
                }
            }

            n = -1;
            l = 1;
            b = 0;
            t = edge_pair_index + 1;
            while (t - b > 1) {
                l = (b + t) / 2;

                for (i = 0; i < 3; i++) {
                    static int ii_table[] = {1, 3, 2};

                    /* 1 = Subject's Kth, */
                    /* 3 = On-File's Jth or Kth (depending), */
                    /* 2 = Subject's Jth */

                    ii = ii_table[i];
                    p1 = rot[edge_pair_index][ii];
                    p2 = *(rtp[l - 1] + ii);

                    n = SENSE(p1, p2);

                    if (n < 0) {
                        t = l;
                        break;
                    }
                    if (n > 0) {
                        b = l;
                        break;
                    }
                }

                if (n == 0) {
                    n = 1;
                    b = l;
                }
            } /* END while() for binary search */

            if (n == 1)
                ++l;

            rtp_insert(rtp, l, edge_pair_index, &rot[edge_pair_index][0]);
            ++edge_pair_index;

            if (edge_pair_index == ROT_SIZE_1 - 1) {
#ifndef NOVERBOSE
                if (verbose_bozorth)
                    fprintf(errorfp, "%s: bz_match(): WARNING: list is full, breaking loop early [p=%s; g=%s]\n",
                            get_progname(), get_probe_filename(), get_gallery_filename());
#endif
                goto END;      /* break out if list exceeded */
            }

        } /* END FOR On-File (edge) distance */

    } /* END FOR Subject (edge) distance */

END:
    {
        int *colp_ptr = &colp[0][0];

        for (i = 0; i < edge_pair_index; i++) {
            INT_COPY(colp_ptr, rtp[i], COLP_SIZE_2);
        }
    }

    /* Process or use the data stored in minutia_pairs and similarity_pairs as needed */
    /* For example, you can iterate over them or pass them to other functions */

    /* Free allocated memory */
    free(minutia_pairs);
    free(similarity_pairs);

    return edge_pair_index;         /* Return the number of compatible edge pairs stored into colp[][] */
}

/**************************************************************************/
/* These global arrays are declared "static" as they are only used        */
/* between bz_match_score() & bz_final_loop()                             */
/**************************************************************************/
static int ct[ CT_SIZE ];
static int gct[ GCT_SIZE ];
static int ctt[ CTT_SIZE ];
static int ctp[ CTP_SIZE_1 ][ CTP_SIZE_2 ];
static int yy[ YY_SIZE_1 ][ YY_SIZE_2 ][ YY_SIZE_3 ];

static int    bz_final_loop( int );

/**************************************************************************/
int bz_match_score(
	int np,
	struct xyt_struct * pstruct,
	struct xyt_struct * gstruct
	)
{
int kx, kq;
int ftt;
int tot;
int qh;
int tp;
int ll, jj, kk, n, t, b;
int k, i, j, ii, z;
int kz, l;
int p1, p2;
int dw, ww;
int match_score;
int qq_overflow = 0;
float fi;

/* These next 3 arrays originally declared global, but moved here */
/* locally because they are only used herein */
int rr[ RR_SIZE ];
int avn[ AVN_SIZE ];
int avv[ AVV_SIZE_1 ][ AVV_SIZE_2 ];

/* These now externally defined in bozorth.h */
/* extern FILE * errorfp; */
/* extern char * get_progname( void ); */
/* extern char * get_probe_filename( void ); */
/* extern char * get_gallery_filename( void ); */






if ( pstruct->nrows < min_computable_minutiae ) {
#ifndef NOVERBOSE
	if ( gstruct->nrows < min_computable_minutiae ) {
		if ( verbose_bozorth )
			fprintf( errorfp, "%s: bz_match_score(): both probe and gallery file have too few minutiae (%d,%d) to compute a real Bozorth match score; min. is %d [p=%s; g=%s]\n",
						get_progname(),
						pstruct->nrows, gstruct->nrows, min_computable_minutiae,
						get_probe_filename(), get_gallery_filename() );
	} else {
		if ( verbose_bozorth )
			fprintf( errorfp, "%s: bz_match_score(): probe file has too few minutiae (%d) to compute a real Bozorth match score; min. is %d [p=%s; g=%s]\n",
						get_progname(),
						pstruct->nrows, min_computable_minutiae,
						get_probe_filename(), get_gallery_filename() );
	}
#endif
	return ZERO_MATCH_SCORE;
}



if ( gstruct->nrows < min_computable_minutiae ) {
#ifndef NOVERBOSE
	if ( verbose_bozorth )
		fprintf( errorfp, "%s: bz_match_score(): gallery file has too few minutiae (%d) to compute a real Bozorth match score; min. is %d [p=%s; g=%s]\n",
						get_progname(),
						gstruct->nrows, min_computable_minutiae,
						get_probe_filename(), get_gallery_filename() );
#endif
	return ZERO_MATCH_SCORE;
}









								/* initialize tables to 0's */
INT_SET( (int *) &yl, YL_SIZE_1 * YL_SIZE_2, 0 );



INT_SET( (int *) &sc, SC_SIZE, 0 );
INT_SET( (int *) &cp, CP_SIZE, 0 );
INT_SET( (int *) &rp, RP_SIZE, 0 );
INT_SET( (int *) &tq, TQ_SIZE, 0 );
INT_SET( (int *) &rq, RQ_SIZE, 0 );
INT_SET( (int *) &zz, ZZ_SIZE, 1000 );				/* zz[] initialized to 1000's */

INT_SET( (int *) &avn, AVN_SIZE, 0 );				/* avn[0...4] <== 0; */





tp  = 0;
p1  = 0;
tot = 0;
ftt = 0;
kx  = 0;
match_score = 0;

for ( k = 0; k < np - 1; k++ ) {
					/* printf( "compute(): looping with k=%d\n", k ); */

	if ( sc[k] )			/* If SC counter for current pair already incremented ... */
		continue;		/*		Skip to next pair */


	i = colp[k][1];
	t = colp[k][3];




	qq[0]   = i;
	rq[t-1] = i;
	tq[i-1] = t;


	ww = 0;
	dw = 0;

	do {
		ftt++;
		tot = 0;
		qh  = 1;
		kx  = k;




		do {









			kz = colp[kx][2];
			l  = colp[kx][4];
			kx++;
			bz_sift( &ww, kz, &qh, l, kx, ftt, &tot, &qq_overflow );
			if ( qq_overflow ) {
				fprintf( errorfp, "%s: WARNING: bz_match_score(): qq[] overflow from bz_sift() #1 [p=%s; g=%s]\n",
							get_progname(), get_probe_filename(), get_gallery_filename() );
				return QQ_OVERFLOW_SCORE;
			}

#ifndef NOVERBOSE
			if ( verbose_bozorth )
				printf( "x1 %d %d %d %d %d %d\n", kx, colp[kx][0], colp[kx][1], colp[kx][2], colp[kx][3], colp[kx][4] );
#endif

		} while ( colp[kx][3] == colp[k][3] && colp[kx][1] == colp[k][1] );
			/* While the startpoints of lookahead edge pairs are the same as the starting points of the */
			/* current pair, set KQ to lookahead edge pair index where above bz_sift() loop left off */

		kq = kx;

		for ( j = 1; j < qh; j++ ) {
			for ( i = kq; i < np; i++ ) {

				for ( z = 1; z < 3; z++ ) {
					if ( z == 1 ) {
						if ( (j+1) > QQ_SIZE ) {
							fprintf( errorfp, "%s: WARNING: bz_match_score(): qq[] overflow #1 in bozorth3(); j-1 is %d [p=%s; g=%s]\n",
								get_progname(), j-1, get_probe_filename(), get_gallery_filename() );
							return QQ_OVERFLOW_SCORE;
						}
						p1 = qq[j];
					} else {
						p1 = tq[p1-1];

					}






					if ( colp[i][2*z] != p1 )
						break;
				}


				if ( z == 3 ) {
					z = colp[i][1];
					l = colp[i][3];



					if ( z != colp[k][1] && l != colp[k][3] ) {
						kx = i + 1;
						bz_sift( &ww, z, &qh, l, kx, ftt, &tot, &qq_overflow );
						if ( qq_overflow ) {
							fprintf( errorfp, "%s: WARNING: bz_match_score(): qq[] overflow from bz_sift() #2 [p=%s; g=%s]\n",
								get_progname(), get_probe_filename(), get_gallery_filename() );
							return QQ_OVERFLOW_SCORE;
						}
					}
				}
			} /* END for i */



			/* Done looking ahead for current j */





			l = 1;
			t = np + 1;
			b = kq;

			while ( t - b > 1 ) {
				l = ( b + t ) / 2;

				for ( i = 1; i < 3; i++ ) {

					if ( i == 1 ) {
						if ( (j+1) > QQ_SIZE ) {
							fprintf( errorfp, "%s: WARNING: bz_match_score(): qq[] overflow #2 in bozorth3(); j-1 is %d [p=%s; g=%s]\n",
								get_progname(), j-1, get_probe_filename(), get_gallery_filename() );
							return QQ_OVERFLOW_SCORE;
						}
						p1 = qq[j];
					} else {
						p1 = tq[p1-1];
					}



					p2 = colp[l-1][i*2-1];

					n = SENSE(p1,p2);

					if ( n < 0 ) {
						t = l;
						break;
					}
					if ( n > 0 ) {
						b = l;
						break;
					}
				}

				if ( n == 0 ) {






					/* Locates the head of consecutive sequence of edge pairs all having the same starting Subject and On-File edgepoints */
					while ( colp[l-2][3] == p2 && colp[l-2][1] == colp[l-1][1] )
						l--;

					kx = l - 1;


					do {
						kz = colp[kx][2];
						l  = colp[kx][4];
						kx++;
						bz_sift( &ww, kz, &qh, l, kx, ftt, &tot, &qq_overflow );
						if ( qq_overflow ) {
							fprintf( errorfp, "%s: WARNING: bz_match_score(): qq[] overflow from bz_sift() #3 [p=%s; g=%s]\n",
								get_progname(), get_probe_filename(), get_gallery_filename() );
							return QQ_OVERFLOW_SCORE;
						}
					} while ( colp[kx][3] == p2 && colp[kx][1] == colp[kx-1][1] );

					break;
				} /* END if ( n == 0 ) */

			} /* END while */

		} /* END for j */




		if ( tot >= MSTR ) {
			jj = 0;
			kk = 0;
			n  = 0;
			l  = 0;

			for ( i = 0; i < tot; i++ ) {


				int colp_value = colp[ y[i]-1 ][0];
				if ( colp_value < 0 ) {
					kk += colp_value;
					n++;
				} else {
					jj += colp_value;
					l++;
				}
			}


			if ( n == 0 ) {
				n = 1;
			} else if ( l == 0 ) {
				l = 1;
			}



			fi = (float) jj / (float) l - (float) kk / (float) n;

			if ( fi > 180.0F ) {
				fi = ( jj + kk + n * 360 ) / (float) tot;
				if ( fi > 180.0F )
					fi -= 360.0F;
			} else {
				fi = ( jj + kk ) / (float) tot;
			}

			jj = ROUND(fi);
			if ( jj <= -180 )
				jj += 360;



			kk = 0;
			for ( i = 0; i < tot; i++ ) {
				int diff = colp[ y[i]-1 ][0] - jj;
				j = SQUARED( diff );




				if ( j > TXS && j < CTXS )
					kk++;
				else
					y[i-kk] = y[i];
			} /* END FOR i */

			tot -= kk;				/* Adjust the total edge pairs TOT based on # of edge pairs skipped */

		} /* END if ( tot >= MSTR ) */




		if ( tot < MSTR ) {




			for ( i = tot-1 ; i >= 0; i-- ) {
				int idx = y[i] - 1;
				if ( rk[idx] == 0 ) {
					sc[idx] = -1;
				} else {
					sc[idx] = rk[idx];
				}
			}
			ftt--;

		} else {		/* tot >= MSTR */
					/* Otherwise size of TOT group (seq. of TOT indices stored in Y) is large enough to analyze */

			int pa = 0;
			int pb = 0;
			int pc = 0;
			int pd = 0;

			for ( i = 0; i < tot; i++ ) {
				int idx = y[i] - 1;
				for ( ii = 1; ii < 4; ii++ ) {




					kk = ( SQUARED(ii) - ii + 2 ) / 2 - 1;




					jj = colp[idx][kk];

					switch ( ii ) {
					  case 1:
						if ( colp[idx][0] < 0 ) {
							pd += colp[idx][0];
							pb++;
						} else {
							pa += colp[idx][0];
							pc++;
						}
						break;
					  case 2:
						avn[ii-1] += pstruct->xcol[jj-1];
						avn[ii] += pstruct->ycol[jj-1];
						break;
					  default:
						avn[ii] += gstruct->xcol[jj-1];
						avn[ii+1] += gstruct->ycol[jj-1];
						break;
					} /* switch */
				} /* END for ii = [1..3] */

				for ( ii = 0; ii < 2; ii++ ) {
					n = -1;
					l = 1;

					for ( jj = 1; jj < 3; jj++ ) {










						p1 = colp[idx][ 2 * ii + jj ];


						b = 0;
						t = yl[ii][tp] + 1;

						while ( t - b > 1 ) {
							l  = ( b + t ) / 2;
							p2 = yy[l-1][ii][tp];
							n  = SENSE(p1,p2);

							if ( n < 0 ) {
								t = l;
							} else {
								if ( n > 0 ) {
									b = l;
								} else {
									break;
								}
							}
						} /* END WHILE */

						if ( n != 0 ) {
							if ( n == 1 )
								++l;

							for ( kk = yl[ii][tp]; kk >= l; --kk ) {
								yy[kk][ii][tp] = yy[kk-1][ii][tp];
							}

							++yl[ii][tp];
							yy[l-1][ii][tp] = p1;


						} /* END if ( n != 0 ) */

						/* Otherwise, edgepoint already stored in YY */

					} /* END FOR jj in [1,2] */
				} /* END FOR ii in [0,1] */
			} /* END FOR i */

			if ( pb == 0 ) {
				pb = 1;
			} else if ( pc == 0 ) {
				pc = 1;
			}



			fi = (float) pa / (float) pc - (float) pd / (float) pb;
			if ( fi > 180.0F ) {

				fi = ( pa + pd + pb * 360 ) / (float) tot;
				if ( fi > 180.0F )
					fi -= 360.0F;
			} else {
				fi = ( pa + pd ) / (float) tot;
			}

			pa = ROUND(fi);
			if ( pa <= -180 )
				pa += 360;



			avv[tp][0] = pa;

			for ( ii = 1; ii < 5; ii++ ) {
				avv[tp][ii] = avn[ii] / tot;
				avn[ii] = 0;
			}

			ct[tp]  = tot;
			gct[tp] = tot;

			if ( tot > match_score )		/* If current TOT > match_score ... */
				match_score = tot;		/*	Keep track of max TOT in match_score */

			ctt[tp]    = 0;		/* Init CTT[TP] to 0 */
			ctp[tp][0] = tp;	/* Store TP into CTP */

			for ( ii = 0; ii < tp; ii++ ) {
				int found;
				int diff;

				int * avv_tp_ptr = &avv[tp][0];
				int * avv_ii_ptr = &avv[ii][0];
				diff = *avv_tp_ptr++ - *avv_ii_ptr++;
				j = SQUARED( diff );






				if ( j > TXS && j < CTXS )
					continue;









				ll = *avv_tp_ptr++ - *avv_ii_ptr++;
				jj = *avv_tp_ptr++ - *avv_ii_ptr++;
				kk = *avv_tp_ptr++ - *avv_ii_ptr++;
				j  = *avv_tp_ptr++ - *avv_ii_ptr++;

				{
				float tt, ai, dz;

				tt = (float) (SQUARED(ll) + SQUARED(jj));
				ai = (float) (SQUARED(j)  + SQUARED(kk));

				fi = ( 2.0F * TK ) * ( tt + ai );
				dz = tt - ai;


				if ( SQUARED(dz) > SQUARED(fi) )
					continue;
				}



				if ( ll ) {

					if ( m1_xyt )
						fi = ( 180.0F / PI_SINGLE ) * atanf( (float) -jj / (float) ll );
					else
						fi = ( 180.0F / PI_SINGLE ) * atanf( (float) jj / (float) ll );
					if ( fi < 0.0F ) {
						if ( ll < 0 )
							fi += 180.5F;
						else
							fi -= 0.5F;
					} else {
						if ( ll < 0 )
							fi -= 180.5F;
						else
							fi += 0.5F;
					}
					jj = (int) fi;
					if ( jj <= -180 )
						jj += 360;
				} else {

					if ( m1_xyt ) {
						if ( jj > 0 )
							jj = -90;
						else
							jj = 90;
					} else {
						if ( jj > 0 )
							jj = 90;
						else
							jj = -90;
					}
				}



				if ( kk ) {

					if ( m1_xyt )
						fi = ( 180.0F / PI_SINGLE ) * atanf( (float) -j / (float) kk );
					else
						fi = ( 180.0F / PI_SINGLE ) * atanf( (float) j / (float) kk );
					if ( fi < 0.0F ) {
						if ( kk < 0 )
							fi += 180.5F;
						else
							fi -= 0.5F;
					} else {
						if ( kk < 0 )
							fi -= 180.5F;
						else
							fi += 0.5F;
					}
					j = (int) fi;
					if ( j <= -180 )
						j += 360;
				} else {

					if ( m1_xyt ) {
						if ( j > 0 )
							j = -90;
						else
							j = 90;
					} else {
						if ( j > 0 )
							j = 90;
						else
							j = -90;
					}
				}





				pa = 0;
				pb = 0;
				pc = 0;
				pd = 0;

				if ( avv[tp][0] < 0 ) {
					pd += avv[tp][0];
					pb++;
				} else {
					pa += avv[tp][0];
					pc++;
				}

				if ( avv[ii][0] < 0 ) {
					pd += avv[ii][0];
					pb++;
				} else {
					pa += avv[ii][0];
					pc++;
				}

				if ( pb == 0 ) {
					pb = 1;
				} else if ( pc == 0 ) {
					pc = 1;
				}



				fi = (float) pa / (float) pc - (float) pd / (float) pb;

				if ( fi > 180.0F ) {
					fi = ( pa + pd + pb * 360 ) / 2.0F;
					if ( fi > 180.0F )
						fi -= 360.0F;
				} else {
					fi = ( pa + pd ) / 2.0F;
				}

				pb = ROUND(fi);
				if ( pb <= -180 )
					pb += 360;





				pa = jj - j;
				pa = IANGLE180(pa);
				kk = SQUARED(pb-pa);




				/* Was: if ( SQUARED(kk) > TXS && kk < CTXS ) : assume typo */
				if ( kk > TXS && kk < CTXS )
					continue;


				found = 0;
				for ( kk = 0; kk < 2; kk++ ) {
					jj = 0;
					ll = 0;

					do {
						while ( yy[jj][kk][ii] < yy[ll][kk][tp] && jj < yl[kk][ii] ) {

							jj++;
						}




						while ( yy[jj][kk][ii] > yy[ll][kk][tp] && ll < yl[kk][tp] ) {

							ll++;
						}




						if ( yy[jj][kk][ii] == yy[ll][kk][tp] && jj < yl[kk][ii] && ll < yl[kk][tp] ) {
							found = 1;
							break;
						}


					} while ( jj < yl[kk][ii] && ll < yl[kk][tp] );
					if ( found )
						break;
				} /* END for kk */

				if ( ! found ) {			/* If we didn't find what we were searching for ... */
					gct[ii] += ct[tp];
					if ( gct[ii] > match_score )
						match_score = gct[ii];
					++ctt[ii];
					ctp[ii][ctt[ii]] = tp;
				}

			} /* END for ii in [0,TP-1] prior TP group */

			tp++;			/* Bump TP counter */


		} /* END ELSE if ( tot == MSTR ) */



		if ( qh > QQ_SIZE ) {
			fprintf( errorfp, "%s: WARNING: bz_match_score(): qq[] overflow #3 in bozorth3(); qh-1 is %d [p=%s; g=%s]\n",
					get_progname(), qh-1, get_probe_filename(), get_gallery_filename() );
			return QQ_OVERFLOW_SCORE;
		}
		for ( i = qh - 1; i > 0; i-- ) {
			n = qq[i] - 1;
			if ( ( tq[n] - 1 ) >= 0 ) {
				rq[tq[n]-1] = 0;
				tq[n]       = 0;
				zz[n]       = 1000;
			}
		}

		for ( i = dw - 1; i >= 0; i-- ) {
			n = rr[i] - 1;
			if ( tq[n] ) {
				rq[tq[n]-1] = 0;
				tq[n]       = 0;
			}
		}

		i = 0;
		j = ww - 1;
		while ( i >= 0 && j >= 0 ) {
			if ( nn[j] < mm[j] ) {
				++nn[j];

				for ( i = ww - 1; i >= 0; i-- ) {
					int rt = rx[i];
					if ( rt < 0 ) {
						rt = - rt;
						rt--;
						z  = rf[i][nn[i]-1]-1;



						if (( tq[z] != (rt+1) && tq[z] ) || ( rq[rt] != (z+1) && rq[rt] ))
							break;


						tq[z]  = rt+1;
						rq[rt] = z+1;
						rr[i]  = z+1;
					} else {
						rt--;
						z = cf[i][nn[i]-1]-1;


						if (( tq[rt] != (z+1) && tq[rt] ) || ( rq[z] != (rt+1) && rq[z] ))
							break;


						tq[rt] = z+1;
						rq[z]  = rt+1;
						rr[i]  = rt+1;
					}
				} /* END for i */

				if ( i >= 0 ) {
					for ( z = i + 1; z < ww; z++) {
						n = rr[z] - 1;
						if ( tq[n] - 1 >= 0 ) {
							rq[tq[n]-1] = 0;
							tq[n]       = 0;
						}
					}
					j = ww - 1;
				}

			} else {
				nn[j] = 1;
				j--;
			}

		}

		if ( tp > 1999 )
			break;

		dw = ww;


	} while ( j >= 0 ); /* END while endpoint group remain ... */


	if ( tp > 1999 )
		break;




	n = qq[0] - 1;
	if ( tq[n] - 1 >= 0 ) {
		rq[tq[n]-1] = 0;
		tq[n]       = 0;
	}

	for ( i = ww-1; i >= 0; i-- ) {
		n = rx[i];
		if ( n < 0 ) {
			n = - n;
			rp[n-1] = 0;
		} else {
			cp[n-1] = 0;
		}

	}

} /* END FOR each edge pair */



if ( match_score < MMSTR ) {
	return match_score;
}

match_score = bz_final_loop( tp );
return match_score;
}


/***********************************************************************/
/* These globals signficantly used by bz_sift () */
/* Now externally defined in bozorth.h */
/* extern int sc[ SC_SIZE ]; */
/* extern int rq[ RQ_SIZE ]; */
/* extern int tq[ TQ_SIZE ]; */
/* extern int rf[ RF_SIZE_1 ][ RF_SIZE_2 ]; */
/* extern int cf[ CF_SIZE_1 ][ CF_SIZE_2 ]; */
/* extern int zz[ ZZ_SIZE ]; */
/* extern int rx[ RX_SIZE ]; */
/* extern int mm[ MM_SIZE ]; */
/* extern int nn[ NN_SIZE ]; */
/* extern int qq[ QQ_SIZE ]; */
/* extern int rk[ RK_SIZE ]; */
/* extern int cp[ CP_SIZE ]; */
/* extern int rp[ RP_SIZE ]; */
/* extern int y[ Y_SIZE ]; */

void bz_sift(
	int * ww,		/* INPUT and OUTPUT; endpoint groups index; *ww may be bumped by one or by two */
	int   kz,		/* INPUT only;       endpoint of lookahead Subject edge */
	int * qh,		/* INPUT and OUTPUT; the value is an index into qq[] and is stored in zz[]; *qh may be bumped by one */
	int   l,		/* INPUT only;       endpoint of lookahead On-File edge */
	int   kx,		/* INPUT only -- index */
	int   ftt,		/* INPUT only */
	int * tot,		/* OUTPUT -- counter is incremented by one, sometimes */
	int * qq_overflow	/* OUTPUT -- flag is set only if qq[] overflows */
	)
{
int n;
int t;

/* These now externally defined in bozorth.h */
/* extern FILE * errorfp; */
/* extern char * get_progname( void ); */
/* extern char * get_probe_filename( void ); */
/* extern char * get_gallery_filename( void ); */



n = tq[ kz - 1];	/* Lookup On-File edgepoint stored in TQ at index of endpoint of lookahead Subject edge */
t = rq[ l  - 1];	/* Lookup Subject edgepoint stored in RQ at index of endpoint of lookahead On-File edge */

if ( n == 0 && t == 0 ) {


	if ( sc[kx-1] != ftt ) {
		y[ (*tot)++ ] = kx;
		rk[kx-1] = sc[kx-1];
		sc[kx-1] = ftt;
	}

	if ( *qh >= QQ_SIZE ) {
		fprintf( errorfp, "%s: ERROR: bz_sift(): qq[] overflow #1; the index [*qh] is %d [p=%s; g=%s]\n",
						get_progname(),
						*qh, get_probe_filename(), get_gallery_filename() );
		*qq_overflow = 1;
		return;
	}
	qq[ *qh ]  = kz;
	zz[ kz-1 ] = (*qh)++;


				/* The TQ and RQ locations are set, so set them ... */
	tq[ kz-1 ] = l;
	rq[ l-1 ] = kz;

	return;
} /* END if ( n == 0 && t == 0 ) */









if ( n == l ) {

	if ( sc[kx-1] != ftt ) {
		if ( zz[kx-1] == 1000 ) {
			if ( *qh >= QQ_SIZE ) {
				fprintf( errorfp, "%s: ERROR: bz_sift(): qq[] overflow #2; the index [*qh] is %d [p=%s; g=%s]\n",
							get_progname(),
							*qh,
							get_probe_filename(), get_gallery_filename() );
				*qq_overflow = 1;
				return;
			}
			qq[*qh]  = kz;
			zz[kz-1] = (*qh)++;
		}
		y[(*tot)++] = kx;
		rk[kx-1] = sc[kx-1];
		sc[kx-1] = ftt;
	}

	return;
} /* END if ( n == l ) */





if ( *ww >= WWIM )	/* This limits the number of endpoint groups that can be constructed */
	return;


{
int b;
int b_index;
register int i;
int notfound;
int lim;
register int * lptr;

/* If lookahead Subject endpoint previously assigned to TQ but not paired with lookahead On-File endpoint ... */

if ( n ) {
	b = cp[ kz - 1 ];
	if ( b == 0 ) {
		b              = ++*ww;
		b_index        = b - 1;
		cp[kz-1]       = b;
		cf[b_index][0] = n;
		mm[b_index]    = 1;
		nn[b_index]    = 1;
		rx[b_index]    = kz;

	} else {
		b_index = b - 1;
	}

	lim = mm[b_index];
	lptr = &cf[b_index][0];
	notfound = 1;

#ifndef NOVERBOSE
	if ( verbose_bozorth ) {
		int * llptr = lptr;
		printf( "bz_sift(): n: looking for l=%d in [", l );
		for ( i = 0; i < lim; i++ ) {
			printf( " %d", *llptr++ );
		}
		printf( " ]\n" );
	}
#endif

	for ( i = 0; i < lim; i++ ) {
		if ( *lptr++ == l ) {
			notfound = 0;
			break;
		}
	}
	if ( notfound ) {		/* If lookahead On-File endpoint not in list ... */
		cf[b_index][i] = l;
		++mm[b_index];
	}
} /* END if ( n ) */


/* If lookahead On-File endpoint previously assigned to RQ but not paired with lookahead Subject endpoint... */

if ( t ) {
	b = rp[ l - 1 ];
	if ( b == 0 ) {
		b              = ++*ww;
		b_index        = b - 1;
		rp[l-1]        = b;
		rf[b_index][0] = t;
		mm[b_index]    = 1;
		nn[b_index]    = 1;
		rx[b_index]    = -l;


	} else {
		b_index = b - 1;
	}

	lim = mm[b_index];
	lptr = &rf[b_index][0];
	notfound = 1;

#ifndef NOVERBOSE
	if ( verbose_bozorth ) {
		int * llptr = lptr;
		printf( "bz_sift(): t: looking for kz=%d in [", kz );
		for ( i = 0; i < lim; i++ ) {
			printf( " %d", *llptr++ );
		}
		printf( " ]\n" );
	}
#endif

	for ( i = 0; i < lim; i++ ) {
		if ( *lptr++ == kz ) {
			notfound = 0;
			break;
		}
	}
	if ( notfound ) {		/* If lookahead Subject endpoint not in list ... */
		rf[b_index][i] = kz;
		++mm[b_index];
	}
} /* END if ( t ) */

}

}

/**************************************************************************/

static int bz_final_loop( int tp )
{
int ii, i, t, b, n, k, j, kk, jj;
int lim;
int match_score;

/* This array originally declared global, but moved here */
/* locally because it is only used herein.  The use of   */
/* "static" is required as the array will exceed the     */
/* stack allocation on our local systems otherwise.      */
static int sct[ SCT_SIZE_1 ][ SCT_SIZE_2 ];

match_score = 0;
for ( ii = 0; ii < tp; ii++ ) {				/* For each index up to the current value of TP ... */

		if ( match_score >= gct[ii] )		/* if next group total not bigger than current match_score.. */
			continue;			/*		skip to next TP index */

		lim = ctt[ii] + 1;
		for ( i = 0; i < lim; i++ ) {
			sct[i][0] = ctp[ii][i];
		}

		t     = 0;
		y[0]  = lim;
		cp[0] = 1;
		b     = 0;
		n     = 1;
		do {					/* looping until T < 0 ... */
			if ( y[t] - cp[t] > 1 ) {
				k = sct[cp[t]][t];
				j = ctt[k] + 1;
				for ( i = 0; i < j; i++ ) {
					rp[i] = ctp[k][i];
				}
				k  = 0;
				kk = cp[t];
				jj = 0;

				do {
					while ( rp[jj] < sct[kk][t] && jj < j )
						jj++;
					while ( rp[jj] > sct[kk][t] && kk < y[t] )
						kk++;
					while ( rp[jj] == sct[kk][t] && kk < y[t] && jj < j ) {
						sct[k][t+1] = sct[kk][t];
						k++;
						kk++;
						jj++;
					}
				} while ( kk < y[t] && jj < j );

				t++;
				cp[t] = 1;
				y[t]  = k;
				b     = t;
				n     = 1;
			} else {
				int tot = 0;

				lim = y[t];
				for ( i = n-1; i < lim; i++ ) {
					tot += ct[ sct[i][t] ];
				}

				for ( i = 0; i < b; i++ ) {
					tot += ct[ sct[0][i] ];
				}

				if ( tot > match_score ) {		/* If the current total is larger than the running total ... */
					match_score = tot;		/*	then set match_score to the new total */
					for ( i = 0; i < b; i++ ) {
						rk[i] = sct[0][i];
					}

					{
					int rk_index = b;
					lim = y[t];
					for ( i = n-1; i < lim; ) {
						rk[ rk_index++ ] = sct[ i++ ][ t ];
					}
					}
				}
				b = t;
				t--;
				if ( t >= 0 ) {
					++cp[t];
					n = y[t];
				}
			} /* END IF */

		} while ( t >= 0 );

} /* END FOR ii */

return match_score;

} /* END bz_final_loop() */



//  FHE ENABLED (only for the 2nd step)
// /*******************************************************************************

// License:
// This software and/or related materials was developed at the National Institute
// of Standards and Technology (NIST) by employees of the Federal Government
// in the course of their official duties. Pursuant to title 17 Section 105
// of the United States Code, this software is not subject to copyright
// protection and is in the public domain.

// This software and/or related materials have been determined to be not subject
// to the EAR (see Part 734.3 of the EAR for exact details) because it is
// a publicly available technology and software, and is freely distributed
// to any interested party with no licensing requirements.  Therefore, it is
// permissible to distribute this software as a free download from the internet.

// Disclaimer:
// This software and/or related materials was developed to promote biometric
// standards and biometric technology testing for the Federal Government
// in accordance with the USA PATRIOT Act and the Enhanced Border Security
// and Visa Entry Reform Act. Specific hardware and software products identified
// in this software were used in order to perform the software development.
// In no case does such identification imply recommendation or endorsement
// by the National Institute of Standards and Technology, nor does it imply that
// the products and equipment identified are necessarily the best available
// for the purpose.

// This software and/or related materials are provided "AS-IS" without warranty
// of any kind including NO WARRANTY OF PERFORMANCE, MERCHANTABILITY,
// NO WARRANTY OF NON-INFRINGEMENT OF ANY 3RD PARTY INTELLECTUAL PROPERTY
// or FITNESS FOR A PARTICULAR PURPOSE or for any purpose whatsoever, for the
// licensed product, however used. In no event shall NIST be liable for any
// damages and/or costs, including but not limited to incidental or consequential
// damages of any kind, including economic damage or injury to property and lost
// profits, regardless of whether NIST shall be advised, have reason to know,
// or in fact shall know of the possibility.

// By using this software, you agree to bear all risk relating to quality,
// use and performance of the software and/or related materials.  You agree
// to hold the Government harmless from any claim arising from your use
// of the software.

// *******************************************************************************/

// /***********************************************************************
//       LIBRARY: FING - NIST Fingerprint Systems Utilities

//       FILE:           BOZORTH3.C
//       ALGORITHM:      Allan S. Bozorth (FBI)
//       MODIFICATIONS:  Michael D. Garris (NIST)
//                       Stan Janet (NIST)
//       DATE:           09/21/2004

//       Contains the "core" routines responsible for supporting the
//       Bozorth3 fingerprint matching algorithm.

// ***********************************************************************

//       ROUTINES:
// #cat: bz_comp -  takes a set of minutiae (probe or gallery) and
// #cat:            compares/measures  each minutia's {x,y,t} with every
// #cat:            other minutia's {x,y,t} in the set creating a table
// #cat:            of pairwise comparison entries
// #cat: bz_find -  trims sorted table of pairwise minutia comparisons to
// #cat:            a max distance of 75^2
// #cat: bz_match - takes the two pairwise minutia comparison tables (a probe
// #cat:            table and a gallery table) and compiles a list of
// #cat:            all relatively "compatible" entries between the two
// #cat:            tables generating a match table
// #cat: bz_match_score - takes a match table and traverses it looking for
// #cat:            a sufficiently long path (or a cluster of compatible paths)
// #cat:            of "linked" match table entries
// #cat:            the accumulation of which results in a match "score"
// #cat: bz_sift -  main routine handling the path linking and match table
// #cat:            traversal
// #cat: bz_final_loop - (declared static) a final postprocess after
// #cat:            the main match table traversal which looks to combine
// #cat:            clusters of compatible paths

// ***********************************************************************/ 


// #include <bozorth.h>
// #include <stdio.h>
// #include <chrono>

// #include <openfhe.h>
// #include <vector>
// using namespace lbcrypto;
// using namespace std;

// /***********************************************************************/
// void bz_comp(int npoints,                        /* INPUT: # of points */
//              int xcol[MAX_BOZORTH_MINUTIAE],     /* INPUT: x cordinates */
//              int ycol[MAX_BOZORTH_MINUTIAE],     /* INPUT: y cordinates */
//              int thetacol[MAX_BOZORTH_MINUTIAE], /* INPUT: theta values */

//              int *ncomparisons, /* OUTPUT: number of pointwise comparisons */
//              int cols[][COLS_SIZE_2], /* OUTPUT: pointwise comparison table */
//              int *colptrs[] /* INPUT and OUTPUT: sorted list of pointers to rows
//                                in cols[] */
// ) {
//   int i, j, k;

//   int b;
//   int t;
//   int n;
//   int l;

//   int table_index;

//   int dx;
//   int dy;
//   int distance;

//   int theta_kj;
//   int beta_j;
//   int beta_k;

//   int *c;

//   c = &cols[0][0];

//   table_index = 0;
//   for (k = 0; k < npoints - 1; k++) {
//     for (j = k + 1; j < npoints; j++) {

//       if (thetacol[j] > 0) {
//         if (thetacol[k] == thetacol[j] - 180)
//           continue;
//       } else {
//         if (thetacol[k] == thetacol[j] + 180)
//           continue;
//       }

//       dx = xcol[j] - xcol[k];
//       dy = ycol[j] - ycol[k];
//       distance = SQUARED(dx) + SQUARED(dy);
//       if (distance > SQUARED(DM)) {
//         if (dx > DM)
//           break;
//         else
//           continue;
//       }

//       /* The distance is in the range [ 0, 125^2 ] */
//       if (dx == 0)
//         theta_kj = 90;
//       else {
//         double dz;

//         if (m1_xyt)
//           dz = (180.0F / PI_SINGLE) * atanf((float)-dy / (float)dx);
//         else
//           dz = (180.0F / PI_SINGLE) * atanf((float)dy / (float)dx);
//         if (dz < 0.0F)
//           dz -= 0.5F;
//         else
//           dz += 0.5F;
//         theta_kj = (int)dz;
//       }

//       beta_k = theta_kj - thetacol[k];
//       beta_k = IANGLE180(beta_k);

//       beta_j = theta_kj - thetacol[j] + 180;
//       beta_j = IANGLE180(beta_j);

//       if (beta_k < beta_j) {
//         *c++ = distance;
//         *c++ = beta_k;
//         *c++ = beta_j;
//         *c++ = k + 1;
//         *c++ = j + 1;
//         *c++ = theta_kj;
//       } else {
//         *c++ = distance;
//         *c++ = beta_j;
//         *c++ = beta_k;
//         *c++ = k + 1;
//         *c++ = j + 1;
//         *c++ = theta_kj + 400;
//       }

//       b = 0;
//       t = table_index + 1;
//       l = 1;
//       n = -1; /* Init binary search state ... */

//       while (t - b > 1) {
//         int *midpoint;

//         l = (b + t) / 2;
//         midpoint = colptrs[l - 1];

//         for (i = 0; i < 3; i++) {
//           int dd, ff;

//           dd = cols[table_index][i];

//           ff = midpoint[i];

//           n = SENSE(dd, ff);

//           if (n < 0) {
//             t = l;
//             break;
//           }
//           if (n > 0) {
//             b = l;
//             break;
//           }
//         }

//         if (n == 0) {
//           n = 1;
//           b = l;
//         }
//       } /* END while */

//       if (n == 1)
//         ++l;

//       for (i = table_index; i >= l; --i)
//         colptrs[i] = colptrs[i - 1];

//       colptrs[l - 1] = &cols[table_index][0];
//       ++table_index;

//       if (table_index == 19999) {
// #ifndef NOVERBOSE
//         if (verbose_bozorth)
//           printf("bz_comp(): breaking loop to avoid table overflow\n");
// #endif
//         goto COMP_END;
//       }

//     } /* END for j */

//   } /* END for k */

// COMP_END:
//   *ncomparisons = table_index;
//   /*
//   fprintf(stdout, "in bz_comp: \n");
//   fprintf(stdout, "\tbefore trimming table_index = %d\n", table_index);
//   */
// }

// /***********************************************************************/
// void bz_find(int *xlim, /* INPUT:  number of pointwise comparisons in table */
//              /* OUTPUT: determined insertion location (NOT ALWAYS SET) */
//              int *colpt[] /* INOUT:  sorted list of pointers to rows in the
//                              pointwise comparison table */
// ) {
//   int midpoint;
//   int top;
//   int bottom;
//   int state;
//   int distance;

//   /* binary search to locate the insertion location of a predefined distance in
//    * list of sorted distances */

//   bottom = 0;
//   top = *xlim + 1;
//   midpoint = 1;
//   state = -1;

//   while (top - bottom > 1) {
//     midpoint = (bottom + top) / 2;
//     distance = *colpt[midpoint - 1];
//     state = SENSE_NEG_POS(FD, distance);
//     if (state < 0)
//       top = midpoint;
//     else {
//       bottom = midpoint;
//     }
//   }

//   if (state > -1)
//     ++midpoint;

//   if (midpoint < *xlim)
//     *xlim = midpoint;

// /* limit the size to MAX_PAIR_TABLE_SIZE */
// /* this will limit the midpoint to 500, limits  */
// /* the pair table size to FDD=500 in ./bz_drvrs/bozorth_probe_init()  */
// #define MAX_PAIR_TABLE_SIZE 128
//   if (*xlim > MAX_PAIR_TABLE_SIZE)
//     *xlim = MAX_PAIR_TABLE_SIZE;
// }

// /***********************************************************************/
// /* Make room in RTP list at insertion point by shifting contents down the
//    list.  Then insert the address of the current ROT row into desired
//    location */
// /***********************************************************************/
// static

//     void
//     rtp_insert(int *rtp[], int l, int idx, int *ptr) {
//   int shiftcount;
//   int **r1;
//   int **r2;

//   r1 = &rtp[idx];
//   r2 = r1 - 1;

//   shiftcount = (idx - l) + 1;
//   while (shiftcount-- > 0) {
//     *r1-- = *r2--;
//   }
//   *r1 = ptr;
// }

// /***********************************************************************/
// /* Builds list of compatible edge pairs between the 2 Webs. */
// /* The Edge pair DeltaThetaKJs and endpoints are sorted     */
// /*	first on Subject's K,                               */
// /*	then On-File's J or K (depending),                  */
// /*	and lastly on Subject's J point index.              */
// /* Return value is the # of compatible edge pairs           */
// /***********************************************************************/
// int bz_match(
//     int probe_ptrlist_len, /* INPUT:  pruned length of Subject's pointer list */
//     int gallery_ptrlist_len /* INPUT:  pruned length of On-File Record's pointer
//                                list */
// ) {
//   int i;               /* Temp index */
//   int ii;              /* Temp index */
//   int edge_pair_index; /* Compatible edge pair index */
//   float dz;            /* Delta difference and delta angle stats */
//   float fi;            /* Distance limit based on factor TK */
//   int *ss;             /* Subject's comparison stats row */
//   int *ff;             /* On-File Record's comparison stats row */
//   int j;               /* On-File Record's row index */
//   int k;               /* Subject's row index */
//   int st;              /* Starting On-File Record's row index */
//   int p1; /* Adjusted Subject's ThetaKJ, DeltaThetaKJs, K or J point index */
//   int p2; /* Adjusted On-File's ThetaKJ, RTP point index */
//   int n;  /* ThetaKJ and binary search state variable */
//   int l;  /* Midpoint of binary search */
//   int b;  /* ThetaKJ state variable, and bottom of search range */
//   int t;  /* Top of search range */

//   register int *rotptr;

//   /*
//           int probe_ptrlist_len,		INPUT:  pruned length of
//      Subject's
//      pointer list int gallery_ptrlist_len		INPUT:  pruned length of
//      On-File Record's pointer list Print out the probe and gallery sizes to see
//      how big they are typically. (Assumed 200)
//   */
//   /* fprintf(stdout, "In bz_match(): \n"); */
//   /* fprintf(stdout, "\tprob_ptrlist_len: %d\n", probe_ptrlist_len); */
//   /* fprintf(stdout, "\tgallery_ptrlist_len: %d\n", gallery_ptrlist_len); */
//   /* The table sizes are limited to be 500 - changed bz_find (and bz_probe_init)
//    */

// #define ROT_SIZE_1 20000
// #define ROT_SIZE_2 5

//   static int rot[ROT_SIZE_1][ROT_SIZE_2];

//   static int *rtp[ROT_SIZE_1];

//   /* These now externally defined in bozorth.h */
//   /* extern int * scolpt[ SCOLPT_SIZE ];			 INPUT */
//   /* extern int * fcolpt[ FCOLPT_SIZE ];			 INPUT */
//   /* extern int   colp[ COLP_SIZE_1 ][ COLP_SIZE_2 ];	 OUTPUT */
//   /* extern int verbose_bozorth; */
//   /* extern FILE * errorfp; */
//   /* extern char * get_progname( void ); */
//   /* extern char * get_probe_filename( void ); */
//   /* extern char * get_gallery_filename( void ); */

//   std::chrono::high_resolution_clock::time_point start, end;
//   std::chrono::duration<double> duration;

//   /* Create OpenFHE variables */
//   start = std::chrono::high_resolution_clock::now();

//   CCParams<CryptoContextCKKSRNS> parameters;
//   uint32_t multDepth = 15;
//   lbcrypto::SecurityLevel securityLevel = lbcrypto::HEStd_128_classic;

//   // ScalingTechnique rescaleTech = FIXEDAUTO;

//   // parameters.SetSecurityLevel(HEStd_128_classic);
//   parameters.SetSecurityLevel(HEStd_128_classic);
//   parameters.SetMultiplicativeDepth(multDepth);
//   parameters.SetScalingModSize(45);
//   parameters.SetBatchSize(32768);
//   // parameters.SetScalingTechnique(rescaleTech);

//   CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
//   cc->Enable(PKE);
//   cc->Enable(KEYSWITCH);
//   cc->Enable(LEVELEDSHE);
//   cc->Enable(ADVANCEDSHE);

//   auto keyPair = cc->KeyGen();
//   cc->EvalMultKeyGen(keyPair.secretKey);
//   auto pk = keyPair.publicKey;

//   end = std::chrono::high_resolution_clock::now();
//   duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
 
//   cout << "Time taken for crypto-context initilization: "
//          << duration.count() << " milliseconds" << endl;

//   unsigned int batchSize = cc->GetEncodingParams()->GetBatchSize();


// #if false
//   std::cout << "CKKS default parameters: " << parameters << std::endl;
//   std::cout << std::endl;
//   std::cout << std::endl;
// #endif

// #define CT_SIZE 16384 /* 128*128 since that'll be max size for pair tables */
//   static int he_cmp[CT_SIZE] = {0};
//   int s_d_max = *scolpt[0];
//   int f_d_max = *fcolpt[0];

// #if false
// 	std::cout << "probe_ptrlist_len: " << probe_ptrlist_len << std::endl;
// 	std::cout << "gallery_ptrlist_len: " << gallery_ptrlist_len << std::endl;
// #endif

//   /* Create vectors for probe and gallery fingerprints */
//   // static float ss_dist[CT_SIZE] = {0};
//   std::vector<double> ss_dist(CT_SIZE, 0);
//   std::vector<double> ss_b1(CT_SIZE, 0);
//   std::vector<double> ss_b2(CT_SIZE, 0);
//   std::vector<double> ss_global(CT_SIZE, 0);
//   std::vector<double> ss_n(CT_SIZE, 0);

//   std::vector<double> ff_dist(CT_SIZE, 0);
//   std::vector<double> ff_b1(CT_SIZE, 0);
//   std::vector<double> ff_b2(CT_SIZE, 0);
//   std::vector<double> ff_global(CT_SIZE, 0);
//   std::vector<double> ff_b(CT_SIZE, 0);

//   /* Create vectors to store results */

//   std::vector<double> rr_dist(CT_SIZE, 0);
//   std::vector<double> rr_b1(CT_SIZE, 0);
//   std::vector<double> rr_b2(CT_SIZE, 0);
//   std::vector<double> rr_global(CT_SIZE, 0);

//   std::vector<double> test_print(probe_ptrlist_len, 0);

//   int start_index, end_index;
//   int window_length = 8;

//   /* pre-process vectors for subjet and On-File */
//   for (k = 1; k < probe_ptrlist_len; k++) {
//     ss = scolpt[k - 1];

//     /* limit table to be 128*16 instead of 128*128 */
//     start_index = k - (window_length >> 1);
//     end_index = k + (window_length >> 1);

//     if (start_index < 1)
//       start_index = 1;
//     if (end_index > gallery_ptrlist_len)
//       end_index = gallery_ptrlist_len;

//     for (j = start_index; j < end_index; j++) {
//       ff = fcolpt[j - 1];

//       test_print[j - 1] = (double)ff[0];

//       if (*ff > f_d_max) {
//         f_d_max = ff[3];
//       }

//       if (*ss > s_d_max) {
//         s_d_max = ss[3];
//       }

//       window_length = 8;

//       ss_dist[((k - 1) * window_length) + j - 1] =
//           (float)ss[0]; /* subject distance */
//       ss_b1[((k - 1) * window_length) + j - 1] = (float)ss[1]; /* subject b1 */
//       ss_b2[((k - 1) * window_length) + j - 1] = (float)ss[2]; /* subject b2 */

//       if (ss[5] >= 220) {
//         ss_global[((k - 1) * window_length) + j - 1] = ss[5] - 580;
//         ss_n[((k - 1) * window_length) + j - 1] = 1;
//       } else {
//         ss_global[((k - 1) * window_length) + j - 1] = ss[5];
//         ss_n[((k - 1) * window_length) + j - 1] = 0;
//       }

//       ff_dist[((k - 1) * window_length) + j - 1] =
//           (float)ff[0]; /* on file distance */
//       ff_b1[((k - 1) * window_length) + j - 1] = (float)ff[1]; /* on file b1 */
//       ff_b2[((k - 1) * window_length) + j - 1] = (float)ff[2]; /* on file b2 */

//       if (ff[5] >= 220) {
//         ff_global[((k - 1) * window_length) + j - 1] = ff[5] - 580;
//         ff_b[((k - 1) * window_length) + j - 1] = 1;
//       } else {
//         ff_global[((k - 1) * window_length) + j - 1] = ff[5];
//         ff_b[((k - 1) * window_length) + j - 1] = 0;
//       }
//       window_length = 8;
//     }
//   }

// #if false
//   std::vector<double> ss_print(ss_dist.begin(), ss_dist.begin()+(128*10));
//   std::vector<double> ff_print(ff_dist.begin(), ff_dist.begin()+(128*10));
  
//   std::cout << ss_print << std::endl << std::endl;
//   std::cout << ff_print << std::endl << std::endl;
//   std::cout << test_print << std::endl << std::endl;
// #endif

//   float x;
//   float y;

//   start = std::chrono::high_resolution_clock::now();

//   /* make packed plaintexts */
//   Plaintext ps_dist = cc->MakeCKKSPackedPlaintext(ss_dist);
//   Plaintext pf_dist = cc->MakeCKKSPackedPlaintext(ff_dist);

//   Plaintext ps_b1 = cc->MakeCKKSPackedPlaintext(ss_b1);
//   Plaintext ps_b2 = cc->MakeCKKSPackedPlaintext(ss_b2);
//   Plaintext pf_b1 = cc->MakeCKKSPackedPlaintext(ff_b1);
//   Plaintext pf_b2 = cc->MakeCKKSPackedPlaintext(ff_b2);

//   /* Encrypt distance and distance squared */
//   auto cs_dist = cc->Encrypt(pk, ps_dist);
//   auto cs_dsqr = cc->EvalSquare(cs_dist);
//   auto cf_dist = cc->Encrypt(pk, pf_dist);
//   auto cf_dsqr = cc->EvalSquare(cf_dist);

//   auto cs_b1 = cc->Encrypt(pk, ps_b1);
//   auto cs_b2 = cc->Encrypt(pk, ps_b2);
//   auto cf_b1 = cc->Encrypt(pk, pf_b1);
//   auto cf_b2 = cc->Encrypt(pk, pf_b2);

//   end = std::chrono::high_resolution_clock::now();

//   duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
 
//   cout << "Time taken to make plaintexts and encrypt them: "
//          << duration.count() << " milliseconds" << endl;


//   /* compute homomophic ops */
  
//   /* dist */
//   start = std::chrono::high_resolution_clock::now();

//   auto c1 = cc->EvalMult(cs_dist, cf_dist);
//   auto c1Scaled = cc->EvalMult(c1, 2.02);
//   auto c2 = cc->EvalAdd(cs_dsqr, cf_dsqr);
//   auto c2Scaled = cc->EvalMult(c2, 0.99);
//   auto cdist = cc->EvalSub(c2Scaled, c1Scaled);

//   /* angles */
//   c1 = cc->EvalSub(cs_b1, cf_b1);
//   c1Scaled = cc->EvalMult(c1, 0.091);
//   c2 = cc->EvalMult(c1Scaled, c1Scaled);
//   c2Scaled = cc->EvalMult(c2, 1.5);
//   c1 = cc->EvalSub(c2Scaled, 0.75);
//   // c2 = cc->EvalMult(c1, c1);
//   auto cb1 = cc->EvalMult(c1, c1);

//   c1 = cc->EvalSub(cs_b2, cf_b2);
//   c1Scaled = cc->EvalMult(c1, 0.091);
//   c2 = cc->EvalMult(c1Scaled, c1Scaled);
//   c2Scaled = cc->EvalMult(c2, 1.5);
//   c1 = cc->EvalSub(c2Scaled, 0.75);
//   // c2 = cc->EvalMult(c1, c1);
//   auto cb2 = cc->EvalMult(c1, c1);

//   end = std::chrono::high_resolution_clock::now();

//   duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
 
//   cout << "Time taken for FHE evaluations: "
//          << duration.count() << " milliseconds" << endl;

//   /* Decrypt results */
//   start = std::chrono::high_resolution_clock::now();

//   Plaintext dist_result;
//   cc->Decrypt(keyPair.secretKey, cdist, &dist_result);
//   dist_result->SetLength(CT_SIZE);
//   rr_dist = dist_result->GetRealPackedValue();

//   Plaintext b1_result;
//   cc->Decrypt(keyPair.secretKey, cb1, &b1_result);
//   b1_result->SetLength(CT_SIZE);
//   rr_b1 = b1_result->GetRealPackedValue();

//   Plaintext b2_result;
//   cc->Decrypt(keyPair.secretKey, cb2, &b2_result);
//   b2_result->SetLength(CT_SIZE);
//   rr_b2 = b2_result->GetRealPackedValue();

//   end = std::chrono::high_resolution_clock::now();

//   duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
 
//   cout << "Time taken for decryption: "
//          << duration.count() << " milliseconds" << endl;

//   std::cout.precision(8);

//   for (k = 0; k < CT_SIZE; k++) {

//     /* Calculate distance homomorphically */
// #if false
//     x = 2.02F * (ss_dist[k] * ff_dist[k]); /* Depth 1 */
//     y = 0.99F * (ss_dsqr[k] + ff_dsqr[k]);


// 		if (abs(rr_dist[k] -y + x) > 100) {
// 			std::cout << "rr_dist[k]: " << rr_dist[k] << std::endl;
// 			std::cout << "y: " << y << std::endl;
// 			std::cout << "x: " << x << std::endl;
// 			std::cout << std::endl;
// 		}
// 		// std::cout << rr_dist[k] - y + x << std::endl;

//     rr_dist[k] = y - x;

// #endif
//     /* Calculate angles difference homomorphically */
//     x = ss_b1[k] - ff_b1[k];
//     x = x * 0.091; /* normalize by 1/11 */

//     y = SQUARED(x); /* depth 1 */
//     y = 1.5F * y - 0.75;

//     x = SQUARED(y); /* depth 2 */
//     y = SQUARED(x); /* depth 3 */

//     // rr_b1[k] = y;
// #if false
// 		if ((rr_b1[k] > 1) != (y > 1)) {
// 			std::cout << "rr_b1: " << rr_b1[k] << std::endl;
// 			std::cout << "y: " << y << std::endl;
// 			std::cout << std::endl;
// 		}
// #endif

//     /* Calculate angles difference homomorphically */
//     x = ss_b2[k] - ff_b2[k];
//     x = x * 0.091; /* normalize by 1/11 */

//     y = SQUARED(x); /* depth 1 */
//     y = 1.5F * y - 0.75;

//     x = SQUARED(y); /* depth 2 */
//     y = SQUARED(x); /* depth 3 */
//                     // rr_b2[k] = y;

//     /* Calculate global angles */
//     x = ss_global[k] - ff_global[k];
//     rr_global[k] = IANGLE180(x);

//     /* check conditionals after decryption */
//     if (ss_dist[k] == 0) {
//       continue;
//     }

//     if (rr_dist[k] > 0) {
//       continue;
//     }
//     if (rr_b1[k] > 1) {
//       continue;
//     }
//     if (rr_b2[k] > 1) {
//       continue;
//     }

//     he_cmp[k] = 1;
//   }

//   st = 1;
//   edge_pair_index = 0;
//   rotptr = &rot[0][0];

//   /* Foreach sorted edge in Subject's Web ... */

//   for (k = 1; k < probe_ptrlist_len; k++) {
//     /* get subject's stats pointer */
//     ss = scolpt[k - 1];

//     /* Foreach sorted edge in On-File Record's Web ... */

//     for (j = st; j <= gallery_ptrlist_len; j++) {
//       /* get On-File's stats pointer */
//       ff = fcolpt[j - 1];
//       /* calculate the difference between distance_s and distance_f */
//       /* dz = *ff - *ss;  */
//       /* compute scale to see if dist is in acceptable range */
//       /* fi = ( 2.0F * TK ) * ( *ff + *ss );  */

//       /* check tolerance of the distances */
//       /* (dx-dy)^2 > ((dx+dy)*0.1)^2 */
//       /* if ( SQUARED(dz) > SQUARED(fi) ) {  */
//       /* if this is true, then the two are not compatible */

//       /* check to see if the subject is larger than the on file, */
//       /* if true, then every other subject will also be larger so  */
//       /* update the start of the on file and skip loop */
//       /* otherwise break and continue with the curren subject */
//       /* if ( dz < 0 ) {  */
//       /* st = j + 1; / TODO: Uncomment this to improve performance/ */
//       /* continue; */
//       /* } else */
//       /* break;  */
//       /* } */
//       /* passed the distance test */

// #if false
//       /* get the angles of the subject and on file stats */
//       for ( i = 1; i < 3; i++ ) { 
// 				float dz_squared; 

// 				dz = *(ss+i) - *(ff+i); 
// 				dz_squared = SQUARED(dz);  

// 				/* TXS = 121 = 11^2; CTXS = 121801 = 349^2 */
// 				if ( dz_squared > TXS && dz_squared < CTXS )  
// 					break; 
// 				/* only accept pairs with angles that are less that 11 degrees apart  */
// 				/* angle < 11 or angle > 349 passes the check */
//       } 

//       /* skip if either angle is out of the range */
//       if ( i < 3 ) 
//       	continue;
// #endif

//       /* Everything above this comment gives the code for comparison */

//       /* implement skipping when the conditions are not met */
//       if (he_cmp[((k - 1) * window_length) + j - 1] == 0)
//         continue;

// #if false

// 		/* 580 = 360 + 220 */
// 		/* normalize angle to [-360, 220) */
// 		if ( *(ss+5) >= 220 ) {
// 			p1 = *(ss+5) - 580; 
// 			n  = 1;
// 		} else {
// 			p1 = *(ss+5);
// 			n  = 0;
// 		} 

// 		/* normalize angle to [-360, 220) */
// 		if ( *(ff+5) >= 220 ) {
// 			p2 = *(ff+5) - 580;
// 			b  = 1;
// 		} else {
// 			p2 = *(ff+5);
// 			b  = 0;
// 		}

// 		/* IANGLE180 normalizes range to (-180, 180] */
// 		p1 -= p2;
// 		p1 = IANGLE180(p1);

// 		if ( n != b ) {
// #endif

//       if (ss_n[((k - 1) * window_length) + j - 1] !=
//           ff_b[((k - 1) * window_length) + j - 1]) {
//         p1 = rr_global[((k - 1) * window_length) + j - 1];

//         *rotptr++ = p1;
//         *rotptr++ = *(ss + 3);
//         *rotptr++ = *(ss + 4);

//         *rotptr++ = *(ff + 4);
//         *rotptr++ = *(ff + 3);
//       } else {
//         p1 = rr_global[((k - 1) * window_length) + j - 1];

//         *rotptr++ = p1;
//         *rotptr++ = *(ss + 3);
//         *rotptr++ = *(ss + 4);

//         *rotptr++ = *(ff + 3);
//         *rotptr++ = *(ff + 4);
//       }

//       n = -1;
//       l = 1;
//       b = 0;
//       t = edge_pair_index + 1;
//       while (t - b > 1) {
//         l = (b + t) / 2;

//         for (i = 0; i < 3; i++) {
//           static int ii_table[] = {1, 3, 2};

//           /*	1 = Subject's Kth, */
//           /*	3 = On-File's Jth or Kth (depending), */
//           /*	2 = Subject's Jth */

//           ii = ii_table[i];
//           p1 = rot[edge_pair_index][ii];
//           p2 = *(rtp[l - 1] + ii);

//           n = SENSE(
//               p1,
//               p2); /* n = -1 if p1 < p2, n = 1 if p1 > p2, n = 0 if p1 == p2 */

//           if (n < 0) {
//             t = l;
//             break;
//           }
//           if (n > 0) {
//             b = l;
//             break;
//           }
//         }

//         if (n == 0) {
//           n = 1;
//           b = l;
//         }
//       } /* END while() for binary search */

//       if (n == 1)
//         ++l;

//       rtp_insert(rtp, l, edge_pair_index, &rot[edge_pair_index][0]);
//       ++edge_pair_index;

//       if (edge_pair_index == 19999) {
// #ifndef NOVERBOSE
//         if (verbose_bozorth)
//           fprintf(errorfp,
//                   "%s: bz_match(): WARNING: list is full, breaking loop early "
//                   "[p=%s; g=%s]\n",
//                   get_progname(), get_probe_filename(), get_gallery_filename());
// #endif
//         goto END; /* break out if list exceeded */
//       }

//     } /* END FOR On-File (edge) distance */

//   } /* END FOR Subject (edge) distance */

// END : {
//   int *colp_ptr = &colp[0][0];

//   for (i = 0; i < edge_pair_index; i++) {
//     INT_COPY(colp_ptr, rtp[i], COLP_SIZE_2);
//   }
// }

//   return edge_pair_index; /* Return the number of compatible edge pairs stored
//                              into colp[][] */
// }

// /**************************************************************************/
// /* These global arrays are declared "static" as they are only used        */
// /* between bz_match_score() & bz_final_loop()                             */
// /**************************************************************************/
// static int ct[CT_SIZE];
// static int gct[GCT_SIZE];
// static int ctt[CTT_SIZE];
// static int ctp[CTP_SIZE_1][CTP_SIZE_2];
// static int yy[YY_SIZE_1][YY_SIZE_2][YY_SIZE_3];

// static int bz_final_loop(int);

// /**************************************************************************/
// int bz_match_score(int np, struct xyt_struct *pstruct,
//                    struct xyt_struct *gstruct) {
//   int kx, kq;
//   int ftt;
//   int tot;
//   int qh;
//   int tp;
//   int ll, jj, kk, n, t, b;
//   int k, i, j, ii, z;
//   int kz, l;
//   int p1, p2;
//   int dw, ww;
//   int match_score;
//   int qq_overflow = 0;
//   float fi;

//   /* These next 3 arrays originally declared global, but moved here */
//   /* locally because they are only used herein */
//   int rr[RR_SIZE];
//   int avn[AVN_SIZE];
//   int avv[AVV_SIZE_1][AVV_SIZE_2];

//   /* These now externally defined in bozorth.h */
//   /* extern FILE * errorfp; */
//   /* extern char * get_progname( void ); */
//   /* extern char * get_probe_filename( void ); */
//   /* extern char * get_gallery_filename( void ); */

//   if (pstruct->nrows < min_computable_minutiae) {
// #ifndef NOVERBOSE
//     if (gstruct->nrows < min_computable_minutiae) {
//       if (verbose_bozorth)
//         fprintf(errorfp,
//                 "%s: bz_match_score(): both probe and gallery file have too "
//                 "few minutiae (%d,%d) to compute a real Bozorth match score; "
//                 "min. is %d [p=%s; g=%s]\n",
//                 get_progname(), pstruct->nrows, gstruct->nrows,
//                 min_computable_minutiae, get_probe_filename(),
//                 get_gallery_filename());
//     } else {
//       if (verbose_bozorth)
//         fprintf(errorfp,
//                 "%s: bz_match_score(): probe file has too few minutiae (%d) to "
//                 "compute a real Bozorth match score; min. is %d [p=%s; g=%s]\n",
//                 get_progname(), pstruct->nrows, min_computable_minutiae,
//                 get_probe_filename(), get_gallery_filename());
//     }
// #endif
//     return ZERO_MATCH_SCORE;
//   }

//   if (gstruct->nrows < min_computable_minutiae) {
// #ifndef NOVERBOSE
//     if (verbose_bozorth)
//       fprintf(errorfp,
//               "%s: bz_match_score(): gallery file has too few minutiae (%d) to "
//               "compute a real Bozorth match score; min. is %d [p=%s; g=%s]\n",
//               get_progname(), gstruct->nrows, min_computable_minutiae,
//               get_probe_filename(), get_gallery_filename());
// #endif
//     return ZERO_MATCH_SCORE;
//   }

//   /* initialize tables to 0's */
//   INT_SET((int *)&yl, YL_SIZE_1 * YL_SIZE_2, 0);

//   INT_SET((int *)&sc, SC_SIZE, 0);
//   INT_SET((int *)&cp, CP_SIZE, 0);
//   INT_SET((int *)&rp, RP_SIZE, 0);
//   INT_SET((int *)&tq, TQ_SIZE, 0);
//   INT_SET((int *)&rq, RQ_SIZE, 0);
//   INT_SET((int *)&zz, ZZ_SIZE, 1000); /* zz[] initialized to 1000's */

//   INT_SET((int *)&avn, AVN_SIZE, 0); /* avn[0...4] <== 0; */

//   tp = 0;
//   p1 = 0;
//   tot = 0;
//   ftt = 0;
//   kx = 0;
//   match_score = 0;

//   for (k = 0; k < np - 1; k++) {
//     /* printf( "compute(): looping with k=%d\n", k ); */

//     if (sc[k])  /* If SC counter for current pair already incremented ... */
//       continue; /*		Skip to next pair */

//     i = colp[k][1];
//     t = colp[k][3];

//     qq[0] = i;
//     rq[t - 1] = i;
//     tq[i - 1] = t;

//     ww = 0;
//     dw = 0;

//     do {
//       ftt++;
//       tot = 0;
//       qh = 1;
//       kx = k;

//       do {

//         kz = colp[kx][2];
//         l = colp[kx][4];
//         kx++;
//         bz_sift(&ww, kz, &qh, l, kx, ftt, &tot, &qq_overflow);
//         if (qq_overflow) {
//           fprintf(errorfp,
//                   "%s: WARNING: bz_match_score(): qq[] overflow from bz_sift() "
//                   "#1 [p=%s; g=%s]\n",
//                   get_progname(), get_probe_filename(), get_gallery_filename());
//           return QQ_OVERFLOW_SCORE;
//         }

// #ifndef NOVERBOSE
//         if (verbose_bozorth)
//           printf("x1 %d %d %d %d %d %d\n", kx, colp[kx][0], colp[kx][1],
//                  colp[kx][2], colp[kx][3], colp[kx][4]);
// #endif

//       } while (colp[kx][3] == colp[k][3] && colp[kx][1] == colp[k][1]);
//       /* While the startpoints of lookahead edge pairs are the same as the
//        * starting points of the */
//       /* current pair, set KQ to lookahead edge pair index where above bz_sift()
//        * loop left off */

//       kq = kx;

//       for (j = 1; j < qh; j++) {
//         for (i = kq; i < np; i++) {

//           for (z = 1; z < 3; z++) {
//             if (z == 1) {
//               if ((j + 1) > QQ_SIZE) {
//                 fprintf(errorfp,
//                         "%s: WARNING: bz_match_score(): qq[] overflow #1 in "
//                         "bozorth3(); j-1 is %d [p=%s; g=%s]\n",
//                         get_progname(), j - 1, get_probe_filename(),
//                         get_gallery_filename());
//                 return QQ_OVERFLOW_SCORE;
//               }
//               p1 = qq[j];
//             } else {
//               p1 = tq[p1 - 1];
//             }

//             if (colp[i][2 * z] != p1)
//               break;
//           }

//           if (z == 3) {
//             z = colp[i][1];
//             l = colp[i][3];

//             if (z != colp[k][1] && l != colp[k][3]) {
//               kx = i + 1;
//               bz_sift(&ww, z, &qh, l, kx, ftt, &tot, &qq_overflow);
//               if (qq_overflow) {
//                 fprintf(errorfp,
//                         "%s: WARNING: bz_match_score(): qq[] overflow from "
//                         "bz_sift() #2 [p=%s; g=%s]\n",
//                         get_progname(), get_probe_filename(),
//                         get_gallery_filename());
//                 return QQ_OVERFLOW_SCORE;
//               }
//             }
//           }
//         } /* END for i */

//         /* Done looking ahead for current j */

//         l = 1;
//         t = np + 1;
//         b = kq;

//         while (t - b > 1) {
//           l = (b + t) / 2;

//           for (i = 1; i < 3; i++) {

//             if (i == 1) {
//               if ((j + 1) > QQ_SIZE) {
//                 fprintf(errorfp,
//                         "%s: WARNING: bz_match_score(): qq[] overflow #2 in "
//                         "bozorth3(); j-1 is %d [p=%s; g=%s]\n",
//                         get_progname(), j - 1, get_probe_filename(),
//                         get_gallery_filename());
//                 return QQ_OVERFLOW_SCORE;
//               }
//               p1 = qq[j];
//             } else {
//               p1 = tq[p1 - 1];
//             }

//             p2 = colp[l - 1][i * 2 - 1];

//             n = SENSE(p1, p2);

//             if (n < 0) {
//               t = l;
//               break;
//             }
//             if (n > 0) {
//               b = l;
//               break;
//             }
//           }

//           if (n == 0) {

//             /* Locates the head of consecutive sequence of edge pairs all having
//              * the same starting Subject and On-File edgepoints */
//             while (colp[l - 2][3] == p2 && colp[l - 2][1] == colp[l - 1][1])
//               l--;

//             kx = l - 1;

//             do {
//               kz = colp[kx][2];
//               l = colp[kx][4];
//               kx++;
//               bz_sift(&ww, kz, &qh, l, kx, ftt, &tot, &qq_overflow);
//               if (qq_overflow) {
//                 fprintf(errorfp,
//                         "%s: WARNING: bz_match_score(): qq[] overflow from "
//                         "bz_sift() #3 [p=%s; g=%s]\n",
//                         get_progname(), get_probe_filename(),
//                         get_gallery_filename());
//                 return QQ_OVERFLOW_SCORE;
//               }
//             } while (colp[kx][3] == p2 && colp[kx][1] == colp[kx - 1][1]);

//             break;
//           } /* END if ( n == 0 ) */

//         } /* END while */

//       } /* END for j */

//       if (tot >= MSTR) {
//         jj = 0;
//         kk = 0;
//         n = 0;
//         l = 0;

//         for (i = 0; i < tot; i++) {

//           int colp_value = colp[y[i] - 1][0];
//           if (colp_value < 0) {
//             kk += colp_value;
//             n++;
//           } else {
//             jj += colp_value;
//             l++;
//           }
//         }

//         if (n == 0) {
//           n = 1;
//         } else if (l == 0) {
//           l = 1;
//         }

//         fi = (float)jj / (float)l - (float)kk / (float)n;

//         if (fi > 180.0F) {
//           fi = (jj + kk + n * 360) / (float)tot;
//           if (fi > 180.0F)
//             fi -= 360.0F;
//         } else {
//           fi = (jj + kk) / (float)tot;
//         }

//         jj = ROUND(fi);
//         if (jj <= -180)
//           jj += 360;

//         kk = 0;
//         for (i = 0; i < tot; i++) {
//           int diff = colp[y[i] - 1][0] - jj;
//           j = SQUARED(diff);

//           if (j > TXS && j < CTXS)
//             kk++;
//           else
//             y[i - kk] = y[i];
//         } /* END FOR i */

//         tot -= kk; /* Adjust the total edge pairs TOT based on # of edge pairs
//                       skipped */

//       } /* END if ( tot >= MSTR ) */

//       if (tot < MSTR) {

//         for (i = tot - 1; i >= 0; i--) {
//           int idx = y[i] - 1;
//           if (rk[idx] == 0) {
//             sc[idx] = -1;
//           } else {
//             sc[idx] = rk[idx];
//           }
//         }
//         ftt--;

//       } else { /* tot >= MSTR */
//         /* Otherwise size of TOT group (seq. of TOT indices stored in Y) is
//          * large enough to analyze */

//         int pa = 0;
//         int pb = 0;
//         int pc = 0;
//         int pd = 0;

//         for (i = 0; i < tot; i++) {
//           int idx = y[i] - 1;
//           for (ii = 1; ii < 4; ii++) {

//             kk = (SQUARED(ii) - ii + 2) / 2 - 1;

//             jj = colp[idx][kk];

//             switch (ii) {
//             case 1:
//               if (colp[idx][0] < 0) {
//                 pd += colp[idx][0];
//                 pb++;
//               } else {
//                 pa += colp[idx][0];
//                 pc++;
//               }
//               break;
//             case 2:
//               avn[ii - 1] += pstruct->xcol[jj - 1];
//               avn[ii] += pstruct->ycol[jj - 1];
//               break;
//             default:
//               avn[ii] += gstruct->xcol[jj - 1];
//               avn[ii + 1] += gstruct->ycol[jj - 1];
//               break;
//             } /* switch */
//           }   /* END for ii = [1..3] */

//           for (ii = 0; ii < 2; ii++) {
//             n = -1;
//             l = 1;

//             for (jj = 1; jj < 3; jj++) {

//               p1 = colp[idx][2 * ii + jj];

//               b = 0;
//               t = yl[ii][tp] + 1;

//               while (t - b > 1) {
//                 l = (b + t) / 2;
//                 p2 = yy[l - 1][ii][tp];
//                 n = SENSE(p1, p2);

//                 if (n < 0) {
//                   t = l;
//                 } else {
//                   if (n > 0) {
//                     b = l;
//                   } else {
//                     break;
//                   }
//                 }
//               } /* END WHILE */

//               if (n != 0) {
//                 if (n == 1)
//                   ++l;

//                 for (kk = yl[ii][tp]; kk >= l; --kk) {
//                   yy[kk][ii][tp] = yy[kk - 1][ii][tp];
//                 }

//                 ++yl[ii][tp];
//                 yy[l - 1][ii][tp] = p1;

//               } /* END if ( n != 0 ) */

//               /* Otherwise, edgepoint already stored in YY */

//             } /* END FOR jj in [1,2] */
//           }   /* END FOR ii in [0,1] */
//         }     /* END FOR i */

//         if (pb == 0) {
//           pb = 1;
//         } else if (pc == 0) {
//           pc = 1;
//         }

//         fi = (float)pa / (float)pc - (float)pd / (float)pb;
//         if (fi > 180.0F) {

//           fi = (pa + pd + pb * 360) / (float)tot;
//           if (fi > 180.0F)
//             fi -= 360.0F;
//         } else {
//           fi = (pa + pd) / (float)tot;
//         }

//         pa = ROUND(fi);
//         if (pa <= -180)
//           pa += 360;

//         avv[tp][0] = pa;

//         for (ii = 1; ii < 5; ii++) {
//           avv[tp][ii] = avn[ii] / tot;
//           avn[ii] = 0;
//         }

//         ct[tp] = tot;
//         gct[tp] = tot;

//         if (tot > match_score) /* If current TOT > match_score ... */
//           match_score = tot;   /*	Keep track of max TOT in match_score */

//         ctt[tp] = 0;     /* Init CTT[TP] to 0 */
//         ctp[tp][0] = tp; /* Store TP into CTP */

//         for (ii = 0; ii < tp; ii++) {
//           int found;
//           int diff;

//           int *avv_tp_ptr = &avv[tp][0];
//           int *avv_ii_ptr = &avv[ii][0];
//           diff = *avv_tp_ptr++ - *avv_ii_ptr++;
//           j = SQUARED(diff);

//           if (j > TXS && j < CTXS)
//             continue;

//           ll = *avv_tp_ptr++ - *avv_ii_ptr++;
//           jj = *avv_tp_ptr++ - *avv_ii_ptr++;
//           kk = *avv_tp_ptr++ - *avv_ii_ptr++;
//           j = *avv_tp_ptr++ - *avv_ii_ptr++;

//           {
//             float tt, ai, dz;

//             tt = (float)(SQUARED(ll) + SQUARED(jj));
//             ai = (float)(SQUARED(j) + SQUARED(kk));

//             fi = (2.0F * TK) * (tt + ai);
//             dz = tt - ai;

//             if (SQUARED(dz) > SQUARED(fi))
//               continue;
//           }

//           if (ll) {

//             if (m1_xyt)
//               fi = (180.0F / PI_SINGLE) * atanf((float)-jj / (float)ll);
//             else
//               fi = (180.0F / PI_SINGLE) * atanf((float)jj / (float)ll);
//             if (fi < 0.0F) {
//               if (ll < 0)
//                 fi += 180.5F;
//               else
//                 fi -= 0.5F;
//             } else {
//               if (ll < 0)
//                 fi -= 180.5F;
//               else
//                 fi += 0.5F;
//             }
//             jj = (int)fi;
//             if (jj <= -180)
//               jj += 360;
//           } else {

//             if (m1_xyt) {
//               if (jj > 0)
//                 jj = -90;
//               else
//                 jj = 90;
//             } else {
//               if (jj > 0)
//                 jj = 90;
//               else
//                 jj = -90;
//             }
//           }

//           if (kk) {

//             if (m1_xyt)
//               fi = (180.0F / PI_SINGLE) * atanf((float)-j / (float)kk);
//             else
//               fi = (180.0F / PI_SINGLE) * atanf((float)j / (float)kk);
//             if (fi < 0.0F) {
//               if (kk < 0)
//                 fi += 180.5F;
//               else
//                 fi -= 0.5F;
//             } else {
//               if (kk < 0)
//                 fi -= 180.5F;
//               else
//                 fi += 0.5F;
//             }
//             j = (int)fi;
//             if (j <= -180)
//               j += 360;
//           } else {

//             if (m1_xyt) {
//               if (j > 0)
//                 j = -90;
//               else
//                 j = 90;
//             } else {
//               if (j > 0)
//                 j = 90;
//               else
//                 j = -90;
//             }
//           }

//           pa = 0;
//           pb = 0;
//           pc = 0;
//           pd = 0;

//           if (avv[tp][0] < 0) {
//             pd += avv[tp][0];
//             pb++;
//           } else {
//             pa += avv[tp][0];
//             pc++;
//           }

//           if (avv[ii][0] < 0) {
//             pd += avv[ii][0];
//             pb++;
//           } else {
//             pa += avv[ii][0];
//             pc++;
//           }

//           if (pb == 0) {
//             pb = 1;
//           } else if (pc == 0) {
//             pc = 1;
//           }

//           fi = (float)pa / (float)pc - (float)pd / (float)pb;

//           if (fi > 180.0F) {
//             fi = (pa + pd + pb * 360) / 2.0F;
//             if (fi > 180.0F)
//               fi -= 360.0F;
//           } else {
//             fi = (pa + pd) / 2.0F;
//           }

//           pb = ROUND(fi);
//           if (pb <= -180)
//             pb += 360;

//           pa = jj - j;
//           pa = IANGLE180(pa);
//           kk = SQUARED(pb - pa);

//           /* Was: if ( SQUARED(kk) > TXS && kk < CTXS ) : assume typo */
//           if (kk > TXS && kk < CTXS)
//             continue;

//           found = 0;
//           for (kk = 0; kk < 2; kk++) {
//             jj = 0;
//             ll = 0;

//             do {
//               while (yy[jj][kk][ii] < yy[ll][kk][tp] && jj < yl[kk][ii]) {

//                 jj++;
//               }

//               while (yy[jj][kk][ii] > yy[ll][kk][tp] && ll < yl[kk][tp]) {

//                 ll++;
//               }

//               if (yy[jj][kk][ii] == yy[ll][kk][tp] && jj < yl[kk][ii] &&
//                   ll < yl[kk][tp]) {
//                 found = 1;
//                 break;
//               }

//             } while (jj < yl[kk][ii] && ll < yl[kk][tp]);
//             if (found)
//               break;
//           } /* END for kk */

//           if (!found) { /* If we didn't find what we were searching for ... */
//             gct[ii] += ct[tp];
//             if (gct[ii] > match_score)
//               match_score = gct[ii];
//             ++ctt[ii];
//             ctp[ii][ctt[ii]] = tp;
//           }

//         } /* END for ii in [0,TP-1] prior TP group */

//         tp++; /* Bump TP counter */

//       } /* END ELSE if ( tot == MSTR ) */

//       if (qh > QQ_SIZE) {
//         fprintf(errorfp,
//                 "%s: WARNING: bz_match_score(): qq[] overflow #3 in "
//                 "bozorth3(); qh-1 is %d [p=%s; g=%s]\n",
//                 get_progname(), qh - 1, get_probe_filename(),
//                 get_gallery_filename());
//         return QQ_OVERFLOW_SCORE;
//       }
//       for (i = qh - 1; i > 0; i--) {
//         n = qq[i] - 1;
//         if ((tq[n] - 1) >= 0) {
//           rq[tq[n] - 1] = 0;
//           tq[n] = 0;
//           zz[n] = 1000;
//         }
//       }

//       for (i = dw - 1; i >= 0; i--) {
//         n = rr[i] - 1;
//         if (tq[n]) {
//           rq[tq[n] - 1] = 0;
//           tq[n] = 0;
//         }
//       }

//       i = 0;
//       j = ww - 1;
//       while (i >= 0 && j >= 0) {
//         if (nn[j] < mm[j]) {
//           ++nn[j];

//           for (i = ww - 1; i >= 0; i--) {
//             int rt = rx[i];
//             if (rt < 0) {
//               rt = -rt;
//               rt--;
//               z = rf[i][nn[i] - 1] - 1;

//               if ((tq[z] != (rt + 1) && tq[z]) || (rq[rt] != (z + 1) && rq[rt]))
//                 break;

//               tq[z] = rt + 1;
//               rq[rt] = z + 1;
//               rr[i] = z + 1;
//             } else {
//               rt--;
//               z = cf[i][nn[i] - 1] - 1;

//               if ((tq[rt] != (z + 1) && tq[rt]) || (rq[z] != (rt + 1) && rq[z]))
//                 break;

//               tq[rt] = z + 1;
//               rq[z] = rt + 1;
//               rr[i] = rt + 1;
//             }
//           } /* END for i */

//           if (i >= 0) {
//             for (z = i + 1; z < ww; z++) {
//               n = rr[z] - 1;
//               if (tq[n] - 1 >= 0) {
//                 rq[tq[n] - 1] = 0;
//                 tq[n] = 0;
//               }
//             }
//             j = ww - 1;
//           }

//         } else {
//           nn[j] = 1;
//           j--;
//         }
//       }

//       if (tp > 1999)
//         break;

//       dw = ww;

//     } while (j >= 0); /* END while endpoint group remain ... */

//     if (tp > 1999)
//       break;

//     n = qq[0] - 1;
//     if (tq[n] - 1 >= 0) {
//       rq[tq[n] - 1] = 0;
//       tq[n] = 0;
//     }

//     for (i = ww - 1; i >= 0; i--) {
//       n = rx[i];
//       if (n < 0) {
//         n = -n;
//         rp[n - 1] = 0;
//       } else {
//         cp[n - 1] = 0;
//       }
//     }

//   } /* END FOR each edge pair */

//   if (match_score < MMSTR) {
//     return match_score;
//   }

//   match_score = bz_final_loop(tp);
//   return match_score;
// }

// /***********************************************************************/
// /* These globals signficantly used by bz_sift () */
// /* Now externally defined in bozorth.h */
// /* extern int sc[ SC_SIZE ]; */
// /* extern int rq[ RQ_SIZE ]; */
// /* extern int tq[ TQ_SIZE ]; */
// /* extern int rf[ RF_SIZE_1 ][ RF_SIZE_2 ]; */
// /* extern int cf[ CF_SIZE_1 ][ CF_SIZE_2 ]; */
// /* extern int zz[ ZZ_SIZE ]; */
// /* extern int rx[ RX_SIZE ]; */
// /* extern int mm[ MM_SIZE ]; */
// /* extern int nn[ NN_SIZE ]; */
// /* extern int qq[ QQ_SIZE ]; */
// /* extern int rk[ RK_SIZE ]; */
// /* extern int cp[ CP_SIZE ]; */
// /* extern int rp[ RP_SIZE ]; */
// /* extern int y[ Y_SIZE ]; */

// void bz_sift(int *ww, /* INPUT and OUTPUT; endpoint groups index; *ww may be
//                          bumped by one or by two */
//              int kz,  /* INPUT only;       endpoint of lookahead Subject edge */
//              int *qh, /* INPUT and OUTPUT; the value is an index into qq[] and
//                          is stored in zz[]; *qh may be bumped by one */
//              int l,   /* INPUT only;       endpoint of lookahead On-File edge */
//              int kx,  /* INPUT only -- index */
//              int ftt, /* INPUT only */
//              int *tot, /* OUTPUT -- counter is incremented by one, sometimes */
//              int *qq_overflow /* OUTPUT -- flag is set only if qq[] overflows */
// ) {
//   int n;
//   int t;

//   /* These now externally defined in bozorth.h */
//   /* extern FILE * errorfp; */
//   /* extern char * get_progname( void ); */
//   /* extern char * get_probe_filename( void ); */
//   /* extern char * get_gallery_filename( void ); */

//   n = tq[kz - 1]; /* Lookup On-File edgepoint stored in TQ at index of endpoint
//                      of lookahead Subject edge */
//   t = rq[l - 1];  /* Lookup Subject edgepoint stored in RQ at index of endpoint
//                      of lookahead On-File edge */

//   if (n == 0 && t == 0) {

//     if (sc[kx - 1] != ftt) {
//       y[(*tot)++] = kx;
//       rk[kx - 1] = sc[kx - 1];
//       sc[kx - 1] = ftt;
//     }

//     if (*qh >= QQ_SIZE) {
//       fprintf(errorfp,
//               "%s: ERROR: bz_sift(): qq[] overflow #1; the index [*qh] is %d "
//               "[p=%s; g=%s]\n",
//               get_progname(), *qh, get_probe_filename(),
//               get_gallery_filename());
//       *qq_overflow = 1;
//       return;
//     }
//     qq[*qh] = kz;
//     zz[kz - 1] = (*qh)++;

//     /* The TQ and RQ locations are set, so set them ... */
//     tq[kz - 1] = l;
//     rq[l - 1] = kz;

//     return;
//   } /* END if ( n == 0 && t == 0 ) */

//   if (n == l) {

//     if (sc[kx - 1] != ftt) {
//       if (zz[kx - 1] == 1000) {
//         if (*qh >= QQ_SIZE) {
//           fprintf(errorfp,
//                   "%s: ERROR: bz_sift(): qq[] overflow #2; the index [*qh] is "
//                   "%d [p=%s; g=%s]\n",
//                   get_progname(), *qh, get_probe_filename(),
//                   get_gallery_filename());
//           *qq_overflow = 1;
//           return;
//         }
//         qq[*qh] = kz;
//         zz[kz - 1] = (*qh)++;
//       }
//       y[(*tot)++] = kx;
//       rk[kx - 1] = sc[kx - 1];
//       sc[kx - 1] = ftt;
//     }

//     return;
//   } /* END if ( n == l ) */

//   if (*ww >= WWIM) /* This limits the number of endpoint groups that can be
//                       constructed */
//     return;

//   {
//     int b;
//     int b_index;
//     register int i;
//     int notfound;
//     int lim;
//     register int *lptr;

//     /* If lookahead Subject endpoint previously assigned to TQ but not paired
//      * with lookahead On-File endpoint ... */

//     if (n) {
//       b = cp[kz - 1];
//       if (b == 0) {
//         b = ++*ww;
//         b_index = b - 1;
//         cp[kz - 1] = b;
//         cf[b_index][0] = n;
//         mm[b_index] = 1;
//         nn[b_index] = 1;
//         rx[b_index] = kz;

//       } else {
//         b_index = b - 1;
//       }

//       lim = mm[b_index];
//       lptr = &cf[b_index][0];
//       notfound = 1;

// #ifndef NOVERBOSE
//       if (verbose_bozorth) {
//         int *llptr = lptr;
//         printf("bz_sift(): n: looking for l=%d in [", l);
//         for (i = 0; i < lim; i++) {
//           printf(" %d", *llptr++);
//         }
//         printf(" ]\n");
//       }
// #endif

//       for (i = 0; i < lim; i++) {
//         if (*lptr++ == l) {
//           notfound = 0;
//           break;
//         }
//       }
//       if (notfound) { /* If lookahead On-File endpoint not in list ... */
//         cf[b_index][i] = l;
//         ++mm[b_index];
//       }
//     } /* END if ( n ) */

//     /* If lookahead On-File endpoint previously assigned to RQ but not paired
//      * with lookahead Subject endpoint... */

//     if (t) {
//       b = rp[l - 1];
//       if (b == 0) {
//         b = ++*ww;
//         b_index = b - 1;
//         rp[l - 1] = b;
//         rf[b_index][0] = t;
//         mm[b_index] = 1;
//         nn[b_index] = 1;
//         rx[b_index] = -l;

//       } else {
//         b_index = b - 1;
//       }

//       lim = mm[b_index];
//       lptr = &rf[b_index][0];
//       notfound = 1;

// #ifndef NOVERBOSE
//       if (verbose_bozorth) {
//         int *llptr = lptr;
//         printf("bz_sift(): t: looking for kz=%d in [", kz);
//         for (i = 0; i < lim; i++) {
//           printf(" %d", *llptr++);
//         }
//         printf(" ]\n");
//       }
// #endif

//       for (i = 0; i < lim; i++) {
//         if (*lptr++ == kz) {
//           notfound = 0;
//           break;
//         }
//       }
//       if (notfound) { /* If lookahead Subject endpoint not in list ... */
//         rf[b_index][i] = kz;
//         ++mm[b_index];
//       }
//     } /* END if ( t ) */
//   }
// }

// /**************************************************************************/

// static int bz_final_loop(int tp) {
//   int ii, i, t, b, n, k, j, kk, jj;
//   int lim;
//   int match_score;

//   /* This array originally declared global, but moved here */
//   /* locally because it is only used herein.  The use of   */
//   /* "static" is required as the array will exceed the     */
//   /* stack allocation on our local systems otherwise.      */
//   static int sct[SCT_SIZE_1][SCT_SIZE_2];

//   match_score = 0;
//   for (ii = 0; ii < tp;
//        ii++) { /* For each index up to the current value of TP ... */

//     if (match_score >=
//         gct[ii]) /* if next group total not bigger than current match_score.. */
//       continue;  /*		skip to next TP index */

//     lim = ctt[ii] + 1;
//     for (i = 0; i < lim; i++) {
//       sct[i][0] = ctp[ii][i];
//     }

//     t = 0;
//     y[0] = lim;
//     cp[0] = 1;
//     b = 0;
//     n = 1;
//     do { /* looping until T < 0 ... */
//       if (y[t] - cp[t] > 1) {
//         k = sct[cp[t]][t];
//         j = ctt[k] + 1;
//         for (i = 0; i < j; i++) {
//           rp[i] = ctp[k][i];
//         }
//         k = 0;
//         kk = cp[t];
//         jj = 0;

//         do {
//           while (rp[jj] < sct[kk][t] && jj < j)
//             jj++;
//           while (rp[jj] > sct[kk][t] && kk < y[t])
//             kk++;
//           while (rp[jj] == sct[kk][t] && kk < y[t] && jj < j) {
//             sct[k][t + 1] = sct[kk][t];
//             k++;
//             kk++;
//             jj++;
//           }
//         } while (kk < y[t] && jj < j);

//         t++;
//         cp[t] = 1;
//         y[t] = k;
//         b = t;
//         n = 1;
//       } else {
//         int tot = 0;

//         lim = y[t];
//         for (i = n - 1; i < lim; i++) {
//           tot += ct[sct[i][t]];
//         }

//         for (i = 0; i < b; i++) {
//           tot += ct[sct[0][i]];
//         }

//         if (tot > match_score) { /* If the current total is larger than the
//                                     running total ... */
//           match_score = tot;     /*	then set match_score to the new total */
//           for (i = 0; i < b; i++) {
//             rk[i] = sct[0][i];
//           }

//           {
//             int rk_index = b;
//             lim = y[t];
//             for (i = n - 1; i < lim;) {
//               rk[rk_index++] = sct[i++][t];
//             }
//           }
//         }
//         b = t;
//         t--;
//         if (t >= 0) {
//           ++cp[t];
//           n = y[t];
//         }
//       } /* END IF */

//     } while (t >= 0);

//   } /* END FOR ii */

//   return match_score;

// } /* END bz_final_loop() */
