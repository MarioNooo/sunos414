/*      Copyright (c) 1988 Sun Microsystems Inc. */
/*        All Rights Reserved   		 */

#if !defined(lint) && defined(SCCSIDS)
static char *sccsid = "@(#)strxfrm.c 1.1 94/10/31 SMI";
#endif

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <locale.h>
#include <malloc.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAXSTR		256
#define TRUE		1
#define FALSE		0

#define IGNORE_KEY	01	/* magic number defined by colldef */


/*   Default is machine code sort order, not including non printing 
 *   characters
 */

struct prim_sec {
	unsigned char p;
	unsigned char s;
};

struct one_to_m {
	unsigned char one;
	struct prim_sec *many;
};
static struct one_to_m *one_to_m;

struct two_to_one {
	unsigned char one;
	unsigned char two;
	struct prim_sec prim_sec;
};
static struct two_to_one *two_to_one;

static char saved_locale[MAXLOCALENAME] = 0;
static char *str = NULL;
static char *primary_sort = NULL;
static char *secondary_sort = NULL;
static struct prim_sec *prim_secp;

extern char _locales[MAXLOCALE - 1][MAXLOCALENAME + 1]; 
static struct one_to_m  	*start1tom;
static struct two_to_one  	*start2to1, *two_to_onep ;

/* openlocale declared in setlocale.c */

int openlocale();

/*
 * so that strcoll works correctly, we use our own version of strcmp,
 * that deals with unsigned characters
 */
static strcmp(s1, s2)
register unsigned char *s1, *s2;
{
	while (*s1 == *s2++)
		if (*s1++=='\0')
			return (0);
	return (*s1 - *--s2);
}

/* Strcoll calls up strxfrm to do the work */

int
strcoll(s1, s2)
char *s1;
char *s2;
{
static	char *sv1, *sv2;
static	int	sv1sz = 0; /* size of space allocated to sv1 */
static  int	sv2sz = 0; /* size of space allocated to sv2 */
	int	i;

#define XFRMFACTOR	2.3 /* Ratio of max length of transformed
				string and the original string. */
	if (sv1sz==0){	/* initially, sv1 points to NULL */
		sv1=malloc(sv1sz=strlen(s1)*XFRMFACTOR + 4);
		if(sv1==NULL){	/* out of memory */
			sv1sz=0;
			fprintf(stderr, "strcoll: string %s is too long", s1);
			return 0;
			}
		}
	else {	/* sv1 is allocted some space now */
		i=strlen(s1)*XFRMFACTOR + 4;
		if(sv1sz < i){	/* space alloctedt to sv1 is not enough */
			sv1=realloc(sv1, i);
			if(sv1==NULL)  {
			  sv1sz=0;
			  fprintf(stderr, "strcoll: string %s is too long", s1);
			  return 0;
			  }
			sv1sz = i;	
			}
		}

	if (i=strxfrm(sv1, s1, sv1sz) >= sv1sz) {	
		/* space allocated to sv1sz is not enough for strxfrm() */
		/* no need to recalculate if strxfrm is correct */
		/* i = strxfrm(NULL, s1, 0); */
		sv1sz = i+1;
		sv1=realloc(sv1, sv1sz);
		if(sv1==NULL)  {
		   sv1sz=0;
		   fprintf(stderr, "strcoll: string %s is too long", s1);
		   return 0;
		   }
		strxfrm(sv1, s1, sv1sz);
		}

	/* Now for sv2, repeat the above steps */
	if (sv2sz==0){	
		sv2=malloc(sv2sz=strlen(s2)*XFRMFACTOR + 1);
		if(sv2==NULL){
			sv2sz=0;
			fprintf(stderr, "strcoll: string %s is too long", s2);
			return 0;
			}
		}
	else	{
		i=strlen(s2)*XFRMFACTOR + 1;
		if(sv2sz < i){
			sv2=realloc(sv2, i);
			if(sv2==NULL)  {
			  sv2sz=0;
			  fprintf(stderr, "strcoll: string %s is too long", s2);
			  return 0;
			  }
			sv2sz = i;
			}
		}

	if (strxfrm(sv2, s2, sv2sz) >= sv2sz) {
		i = strxfrm(NULL, s1, 0);
		sv2sz = i+1;
		sv2=realloc(sv2, sv2sz);
		if(sv2==NULL)  {
		   sv2sz=0;
		   fprintf(stderr, "strcoll: string %s is too long", s2);
		   return 0;
		   }
		strxfrm(sv2, s2, sv2sz);
		}
	/* after we strxfrm s1 and s2, we can simply use strcmp */
	return strcmp(sv1, sv2);
}

/* strxfrm is driven by setlocale() and picks up tables that
 * were generated by the colldef utility.
 */ 

int strxfrm(outputstr,  inputstr,  maxlen)
	char	*outputstr;
	char	*inputstr;
	size_t 	maxlen;

{
static	unsigned char *sec;
static 	int		secsz = 0;	/* size of space allocated to sec */
	unsigned char	*secondary;
	register   count;	/* length of primary weights in outputstr after
					strxfrm */
	register   seccount;	/* like count, but for the secondary weights */
	register   i;
	unsigned char ch;	

	if (outputstr != NULL) {
	    if (secsz == 0) {	/* initially, sec points to NULL */
				/* need to allocate space to it */
		sec = (unsigned char *) malloc(maxlen);
		secsz = maxlen;
		}
	    else if (secsz < maxlen) { /* need to expand space of sec */
		free(sec);
		if ((sec = (unsigned char *) malloc(maxlen)) == NULL) {
			printf("strxfrm: string %s too long\n", inputstr);
			secsz = 0;
			return 0;
			}
		secsz = maxlen;
		}
	}

	if (!start2to1) {
		two_to_one = (struct two_to_one *) malloc
			(sizeof(struct two_to_one) * MAXSUBS);
		one_to_m = (struct one_to_m *) malloc
			(sizeof (struct one_to_m) * MAXSUBS);
		start1tom = one_to_m;
		start2to1 = two_to_one;
	}
	count = 0;
	seccount = 0;
	secondary = sec;
	if (inputstr == NULL) {
		return 0; 	/* No work to do */
		}


	/* If locale value has changed since last we used this
	 * function (or if it has never been called and is now not == C)
	 * then we need to load a new collation table, also we assign 
	 * at runtime so we can xstr this module
	 */

	if (saved_locale[0] == 0)
		strcpy(saved_locale, "C");
	if (strcmp(_locales[LC_COLLATE-1] ,saved_locale)) 
		setupcolldef();

	/* If we are set to "C" or have not even called setlocale
	 * it means "machine order sort" 
  	 */
	if ( (strcmp(saved_locale, "C") == 0) ) {
		if (outputstr != NULL)
			return strlen(strcpy(outputstr, inputstr));
		else
			return strlen(inputstr);
		}

	
	

	/* step through input string */
	while ( (ch = *inputstr++) != '\0') {
		/* Search for 1-to-many mapping */

		for (i = 0; one_to_m[i].one != 0; i++) {
			if (one_to_m[i].one == ch) {
               			prim_secp = one_to_m[i].many;
				while (prim_secp->p) {
				  if ((outputstr != NULL && count < maxlen) 
					&& (prim_secp->p != IGNORE_KEY)) {
					*outputstr++ = prim_secp->p; 
					}
				  if (prim_secp->p != IGNORE_KEY)
				  	count++;
					/* 
					 * If there is a secondary value in the 
					 * substitution, then add this too
					 */
				  if (prim_secp->s) {
					   if (seccount++ < maxlen)
						*secondary++ = prim_secp->s;
						}
					prim_secp++;
				}
			break;
			}
		}

		/* search for first character of potential 2-to-1 mapping */

		for (two_to_onep = start2to1; two_to_onep->one != 0; two_to_onep++) {
			if ((two_to_onep->one == ch) && (two_to_onep->two == *inputstr)) {
				if (outputstr != NULL && count < maxlen) {
					*outputstr++ = two_to_onep->prim_sec.p;
					}
				count++;
				inputstr++; 
					
				if (two_to_onep->prim_sec.s) 
				    if (seccount++ < maxlen)
					*secondary++ = two_to_onep->prim_sec.s;
				break;
			}
		}
		
	
		/* Test to see if we got a match from previous
		 * two "for" loops
		 */
		if (two_to_onep -> one != 0 || one_to_m[i].one != 0)
			continue; 


		/* Now assign real primary and secondary values */

		if ((outputstr != NULL && count < maxlen)
			&& (primary_sort[ch] != IGNORE_KEY)) {
			*outputstr++ = primary_sort[ch] ;
			}
		count++;
		if (secondary_sort[ch]) {
		    if (seccount++ < maxlen)
			*secondary++ = secondary_sort[ch];
		    }

	}


	/*
         * Put the delimiter between primary and secondary
         *      weight level strings
         */
        if ((outputstr != NULL) && count < maxlen)
                *outputstr++ = IGNORE_KEY;
        count++;


	/* 	Now compose primary and secondary lists and output 	
	 *	string as necessary
	 */

	if (outputstr != NULL) {
		*outputstr = '\0'; 
		if (count >= maxlen) {
			return (count+seccount);  /* copy no more characters! */
						  /* fix to bugid 1151444 */
			}

		*secondary = '\0'; 
		if (count + seccount > maxlen)
			strncat(outputstr, sec, maxlen - count);
		else
			strcat(outputstr, sec) ;
		return (count+seccount); /* fix to bugid 1151444 */
	}
	else {
		
		return (count+seccount);
	}
	
}


/*
 * setupcolldef() sets up coll definitions.  It is only called when the
 * current locale is different from the saved_locale by strxfrm().
 * (To force the call to setupcolldef() before setlocale() is called,
 * strxfrm() faked saved_locale to be "C" if saved_locale is NULL.)
 */

setupcolldef()
{
	register int fd;
	register char *memp;
	unsigned short i;
	struct stat buf;
	char locale[MAXLOCALENAME];

	/*
         * If _locale[LC_COLLATE-1] is not set yet, "C" should be used as the
         * default locale, not the environment variable LANG or LC_COLLATE.
         *
         * openlocale(category, cat_id, locale, newlocale) compares
         * locale with _locale[LC_COLLATE-1] and returns 0 (fd)
         * if they are equal.  So we force _locale[LC_COLLATE-1] to NULL
         * before calling openlocale().
         * We will copy the new locale to _locale[LC_COLLATE-1] from
         * saved_locale a few lines later.
        */
 
        if (_locales[LC_COLLATE-1][0] == 0) {
                strcpy(locale, "C");
        } else {
                 strcpy(locale, _locales[LC_COLLATE-1]);
                 _locales[LC_COLLATE-1][0] = 0;
        }
 
        fd = openlocale("LC_COLLATE", LC_COLLATE, locale, saved_locale);

	if (saved_locale)
		strcpy(_locales[LC_COLLATE-1], saved_locale);
	
	if (fd < 0)
		return;
	if ((fstat(fd, &buf)) != 0)
		return;

	/* Now we have got a valid collation table, 
	 * Free previously allocated collation table 
	 * In all error situations return without any change
	 * Note that most error conditions should have been caught
	 * at setlocale() time.
	 */
	
	if (buf.st_size < (2*MAXSTR+2)) {
		(void) close(fd);
		return;
	}
	if (str != NULL)
		free(str);
	if ((str = malloc(buf.st_size +2 )) == NULL ) {
		(void) close(fd);
		return;
	}
	if ((read(fd, str, buf.st_size)) != buf.st_size) {
		free(str);
		(void) close(fd);
		return;
	}

	/* Now step through str, casting pieces to the internal data structures */
		
	primary_sort = str;
	secondary_sort = str+MAXSTR;
	
	memp = str+(2*MAXSTR); i=0; 
	while (*memp != '\0' && i < MAXSUBS) {
                one_to_m[i].one = *memp++;
               	prim_secp = (struct prim_sec *) memp;
                one_to_m[i].many = prim_secp;
		while (prim_secp->p != 0 || prim_secp->s != 0)
			prim_secp++;
		memp = (char *) ++prim_secp;
                ++i;
	}

	memp += sizeof (struct prim_sec) + 1; /* last entry is always empty */
	one_to_m[i].one = 0; 
 
	i=0; 
	while (*memp != '\0' && i < MAXSUBS) {
		two_to_one[i].one = *memp++;	
		two_to_one[i].two = *memp++;	
		two_to_one[i].prim_sec.p = *memp++;	
		two_to_one[i].prim_sec.s = *memp++;	
		++i;
	}
	two_to_one[i].one  = 0;
	strcpy(saved_locale,_locales[LC_COLLATE-1]);
	(void) close(fd);
}
