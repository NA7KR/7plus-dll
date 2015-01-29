/*---------*\
>  7plus.h  <
\*---------*/



#define YES "yes"
#define ALWAYS "always"
#define NO  "no"
#define EOS '\0'
#define ON 0
#define OFF 1
#define LSEP 0x0a
#define LSEPS "\x0a"
#define _7PLUS_FLS "7plus.fls"

/* Some compilers are very strict abt the type of NULL-pointers */
#define NULLFP ((FILE *) 0)
#define NULLCP ((char *) 0)

/* Some compilers have difficulties using setvbuf().
   Uncomment next line, if so (or add it to your definition block). */
/* #define setvbuf(a,b,c,d) */

/** these includes should work anywhere **/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#ifdef __DLL__
 #include <windows.h>
#endif

/* Microsoft's Quick C has some different makros and function names **/
#ifdef _QC
 #define __MSDOS__
 #define MAXDRIVE _MAX_DRIVE
 #define MAXDIR   _MAX_DIR
 #define MAXFILE  _MAX_FNAME
 #define MAXEXT   _MAX_EXT
 #define MAXPATH  _MAX_PATH
 #define fnsplit  _splitpath
#endif /* _QC */

#ifdef __MSDOS__
 #ifdef __TURBOC__
  #include <dir.h>
  #include <io.h>
 #endif
 #include <conio.h>
 #include <stdlib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #define PATHSEP "\\"
 #define PATHCHAR '\\'
 #define TWO_CHAR_SEP
 #define MAXFNAME MAXFILE+MAXEXT-1
 #define _HAVE_FNSPLIT
 #define _HAVE_CHSIZE
 #define _HAVE_ICMP
 #define _HAVE_GMTIME
 #define _HAVE_MKTIME
 #define _HAVE_GETCH
#endif /* __MSDOS__ */

#if defined (__BORLANDC__) && (__WIN32__)
 #include <dir.h>
 #include <io.h>
 #include <conio.h>
 #include <stdlib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #define PATHSEP "\\"
 #define PATHCHAR '\\'
 #define LFN  /* Allow long filenames */
 #define TWO_CHAR_SEP
 #define MAXFNAME MAXPATH
 #define _HAVE_FNSPLIT
 #define _HAVE_CHSIZE
 #define _HAVE_ICMP
 #define _HAVE_GMTIME
 #define _HAVE_MKTIME
 #define _HAVE_GETCH
 #define __MSDOS__
#endif /* __BORLANDC__ && __WIN32__*/





#ifdef __TOS__
 #define MAXPATH   119
 #define MAXDRIVE  3
 #define MAXDIR    102
 #define MAXFILE   9
 #define MAXEXT    5
 #define PATHSEP "\\"
 #define PATHCHAR '\\'
 #define TWO_CHAR_SEP
 #define MAXFNAME MAXFILE+MAXEXT-1
 #define _HAVE_GETCH
 #include <stdlib.h>
 #include <ext.h>
 /* quick & dirty, swaps upper and lower word                      */
 unsigned long swapl(unsigned long l)0x4840; /* opcode for SWAP D0 */
 /* needed for timestamp-functions    (Odo,DL1XAO)                 */
#endif /* __TOS__ */






 #include <sys/types.h>
 #include <sys/stat.h>






#ifdef OSK
 /* Assumed limits */
 #include <types.h>
 #include <stat.h>
 #define _680X0_
 #define _IOFBF 0
 #undef YES
 #undef NO
 #undef ALWAYS
 #define YES "es"
 #define NO  "o"
 #define ALWAYS "lways"
 #undef LSEP
 #undef LSEPS
 #define LSEP 0x0d
 #define LSEPS "\x0d"
 #define MAXPATH 256
 #define MAXDRIVE 16
 #define MAXDIR 256
 #define MAXFILE 32
 #define MAXEXT 256
 #define PATHSEP "/"
 #define PATHCHAR '/'
 #define LFN
 #define MAXFNAME MAXFILE
 #define _HAVE_GETCH
 #define getch()  getc(stdin)
 char *strsave(const char *);
 #define strdup  strsave
 #define SEEK_SET   0
 #define SEEK_CUR   1
 #define SEEK_END   2
 #include <termcap.h>
 #include <stdlib.h>
#endif /* OSK */

#define MAXFPATH (MAXDRIVE+MAXDIR-1)

/* flags for fopen() */
#define OPEN_READ_TEXT "r"
#define OPEN_WRITE_TEXT "w"
#define OPEN_APPEND_TEXT "a"
#define OPEN_RANDOM_TEXT "r+"
#ifdef TWO_CHAR_SEP
 /* This is for systems that convert LF to CR/LF in textmode */
 #define OPEN_READ_BINARY "rb"
 #define OPEN_WRITE_BINARY "wb"
 #define OPEN_APPEND_BINARY "ab"
 #define OPEN_RANDOM_BINARY "r+b"
#else
 /* And this, if there is no conversion */
 #define OPEN_READ_BINARY "r"
 #define OPEN_WRITE_BINARY "w"
 #define OPEN_APPEND_BINARY "a"
 #define OPEN_RANDOM_BINARY "r+"
#endif

/** shorthands for unsigned types **/
typedef unsigned char byte;  /* 8bit unsigned char */

 typedef unsigned int  uint;  /* 16 or 32bit unsigned int */
 typedef unsigned long ulong; /* 32bit unsigned long      */


struct  m_index
{
  char  filename[14];  /*12  chars +2*/
  char  full_name[258];/*256 chars +2*/
  long length;
  ulong timestamp;
  int  splitsize;
  ulong lines_ok[4090];
  long  lines_left;
};

/*********************** macros *************************/

#define crc_calc(x,y) (x)=crctab[(x)>>8]^((((x)&255)<<8)|(byte)(y))

/***************** function prototypes ******************/

/** 7plus.c **/
int   go_at_it       (int argc, char **argv);
int   screenlength   (void);

/** encode.c **/
int   encode_file    (char *name, long blocksize, char *searchbin,
		      int join, char *head_foot);
void  get_range      (char *rangestring);
int   read_tb        (char *name, char *go_top, char *go_bottom);
int   top_bottom     (FILE *wfile, char *top_bot, char *orgname, char *type,
                                                          int part, int parts);
/** decode.c **/
int   control_decode (char *name);
int   decode_file    (char *name, int flag);
void  decode_n_write (FILE *raus, char *p, int length);
void  w_index_err    (struct m_index *idxptr, const char *localname, int flag);
int   make_new_err   (const char *name);
void  progress       (const char *filename, int part, int of_parts,
                                long errors, long rebuilt, const char *status);
/* correct.c */
int   correct_meta   (char *name, int itsacor, int quietmode);

/** util.c **/
char  *my_fgets      (char *string, register int n, FILE *rein);
int   my_putc        (int  outchar, FILE *out);
void  crc_n_lnum     (uint *crc, int *linenumber, char *line);
void  crc2           (uint *crc, char *line);
void  add_crc2       (char *line);
int   mcrc           (char *line, int flag);
int   read_index     (FILE *ifile, struct m_index *idxptr);
int   write_index    (FILE *ifile, struct m_index *idxptr, int flag);
ulong read_ulong     (FILE *in);
uint  read_uint      (FILE *in);
void  write_ulong    (FILE *out, ulong val);
void  write_uint     (FILE *out, uint val);
int   crc_file       (const char *file, const char *s1, const char *s2,
                                                                     int flag);
int   copy_file      (const char *to,  const char *from, ulong timestamp);
void  replace        (const char *oldfil, const char *newfil,  ulong timestamp);



void  kill_em        (const char *name, const char *inpath, const char *one,
                      const char *two,  const char *three,  const char *four,
		      const char *five, int _one, int no_lf);
void  kill_dest      (FILE *in, FILE *out, const char *name);
int   test_exist     (const char *filename);
int   test_file      (FILE *in, char *destnam, int flag, int namsize);
void  init_decodetab (void);
void  init_codetab   (void);
void  init_crctab    (void);
void  build_DOS_name (char *name, char *ext);
void  strip          (char *string);
#if defined (__MSDOS__) || (__TOS__)
 ulong get_filetime   (FILE *_file);
 void  set_filetime   (FILE *_file, ulong ftimestamp);
#else
 #ifndef _HAVE_GMTIME
  struct tm *__offtime (const time_t *t, long int offset);
  struct tm *gmtime    (const time_t *t);
 #endif
 #ifndef _HAVE_MKTIME
  time_t    mktime     (register struct tm *tp);
 #endif
 ulong get_filetime (const char *filename);
 void  set_filetime (const char *filename, ulong ftimestamp);
#endif
uint  get_hex        (char *hex);
#ifndef _HAVE_FNSPLIT
  void  fnsplit      (char *pth, char *dr, char *pa, char *fn, char *ft);
#endif
#ifndef _HAVE_ICMP
  char  *strupr      (char *string);
  char  *strlwr      (char *string);
  char  *strcnvt     (char *string, int flag);
  int   stricmp      (const char *s1, const char *s2);
  int   strnicmp     (const char *s1, const char *s2, size_t n);
#endif
#ifndef _HAVE_GETCH
 int    getch        (void);
#endif

/** rebuild.c **/
int   rebuild        (char *line, int flag);

/** extract.c **/
int   extract_files  (char *name, char *search);

/** join.c **/
int   join_control   (char *file1, char *file2);
int   join_err       (char *file1, char *file2);





#ifdef OSK
 int  rename        (const char *s1, const char *s2);
 int  my_getch      (void);
 int  setvbuf       (FILE * stream, char *buf, int bufmode, size_t size);
 #ifndef _HAVE_STRSTR
  const char *strstr (const char *src, const char *sub);
 #endif
 void check_fn      (register char *fn);
 void set_autolf    (int flag);
#else
 #define set_autolf(x)
 #define check_fn(x)
#endif
