/*--------------------------------*\
| Additions for UNIX-compatibility |
\*--------------------------------*/
#include "7plus.h"





#ifdef OSK

 #include "7plus.h"


int setvbuf(FILE *stream, char *buf, int bufmode, size_t size)
{
  /* derzeit eine Dummyfunktion */

  return (0); /* Success */
}

char *strsave (const char *s1)
{ 
  register unsigned l = strlen(s1) + 1;
  register char *s = (char*)malloc (l);
  if(s ) memcpy (s , s1, l);
  return (s);
}

const char *strstr( const char *src, const char *sub )
{
  register int gefundene_position = findstr(1,src,sub);

  if(gefundene_position) 
    return (src+(gefundene_position-1));
  else
    return ( (const char *)0);	
}


/* Pruefen ob der angebotene Filename den Betriebssytem-Konventionen
   entspricht, gegebenenfalls entsprechend umbauen!! 
   Nicht zugelassene Zeichen werden gegen '_' ersetzt.
*/
void check_fn(register char * fn)
{
  register char tz;
  while((tz=*fn)!=0)
  {
    if (isalnum(tz) 
        || (tz == '.') 
        || (tz == '/')
        || (tz == '_') 
        || (tz == '$'))
    {;}
    else	
      *fn = '_';

    fn++;
  }
}

#include <sgstat.h>

static struct sgbuf original_buffer;
static struct sgbuf aktueller_zustand;

typedef enum { Unbekannt, Besetzt } Zustand;
static Zustand org_buff_zustand = Unbekannt;
#define SCF_CLASS 0
/*
       flag =0 == kein auto lf auf dem device
             1 ==      auto lf auf dem device
*/
void set_autolf(int flag)
{
  if(org_buff_zustand == Unbekannt )
  {
    if(_gs_opt(0,&original_buffer) != -1)
    {
      if( original_buffer.sg_class == SCF_CLASS)
      {
        memcpy(&aktueller_zustand,&original_buffer,sizeof(struct sgbuf));
        org_buff_zustand = Besetzt;
      }
    }
  }

  if( org_buff_zustand == Besetzt )  /* aktueller Zustand ebenfalls */
  {
    if( flag)
    {
      _ss_opt(0,&original_buffer);
    }
    else
    {
      aktueller_zustand.sg_alf   = 0;
      aktueller_zustand.sg_pause = 0;  /* switch page pause off */
      _ss_opt(0,&aktueller_zustand);
    }
  }
}  
#endif /*OSK*/
