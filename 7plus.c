/* Version */
#define VERSION "2.25"
#define PL7_DATE "20000320"
#define AMIGADATE "20.03.00"

/**********************************************************
*** 7PLUS ASCII- Encoder/Decoder, (c) Axel Bauda, DG1BBQ ***
***********************************************************
***
*** Compile:
*** --------
***
*** -------
*** | IBM |  : SMALL memory model
*** -------
***            Turbo C++ and Turbo C 2.0:
***            MAKE -FTC_DOS.MAK
***
***      also: Turbo C++:
***            Compile in IDE using 7PL_TCPP.PRJ
***
***      or  : Turbo C 2.0:
***            Rename TC2.PRJ to 7PLUS.PRJ and compile in IDE
***
*** ----------
*** |IBM OS/2|           EMX/GCC 0.8d : make -femxgcc.mak
*** ---------- IBM C Set/2 & Toolkit/2: nmake -f ibmc2.mak
***                    Borland C++ 1.0: make -fbcos2.mak (or use the IDE)
***
*** ---------
*** | ATARI |  Pure C 1.0/Turbo C 2.0 : Use 7PLST.PRJ
*** ---------
*** --------
*** | UNIX |   support by Torsten H. Bischoff, DF6NL @ DB0BOX
*** --------
***            Supported systems are:  Interactive UNIX 386
***                                    SCO XENIX 386
***                                    VAX BSD 4.3/ Ultrix 4.1
***
***            Compile: make -funix.mak
***
*** ---------  Adjustments made by Mario DL5MLO @OK0PKL.TCH.EU
*** | LINUX |  Tested only on an I486.
*** ---------  Compile: make -flinux.mak
***
*** ---------  Adjustments made by Berndt VK5ABN
*** | NETBSD | Compile: make -fnetbsd.mak
*** ---------
***
*** ---------
*** | AMIGA |  egcs or gcc
*** ---------  Compile: make -f amiga.mak
***
*** ------------
*** | OS-9/68K |    GNU C 1.42
*** ------------
***            copy os9/stat.h /dd/defs/stat.h
***            Compile : make -f=os9_68k.mak
***
*** ----------
*** | Mac OS | Metrowerks CodeWarrior
*** ----------
***
***
*** Other systems: Find out for yourself :->  Good luck!
***
*** All systems:
*** Signed or unsigned char per default is: don't care.
*** No floating point lib required.
***
*** TABSIZE when editing: 2; don't insert real TABs (^I), use spaces instead.
***
*** When porting or modifying this source, make SURE it can still be compiled
*** on all systems! Do this by using #ifdef directives! Please let me know
*** about the modifications or portations, so I can include them in the origi-
*** nal 7PLUS source.
***
**********************************************************
*** 7PLUS ASCII-Encoder/Decoder, (c) Axel Bauda, DG1BBQ ***
**********************************************************
***
*** File converter for transfer of arbitrary binary data
*** via store & forward.
***
*** 7PLUS is HAMWARE. No commercial use. No Sale. Pass on only in it's
*** entirety! There is no warranty for the proper functioning. Use at own
*** risk.
***
*/

#include "7plus.h"

/** globals **/

FILE    *o;
uint    crctab[256];
byte    decode[256];
byte    code[216];
char    range[257];
#ifdef LFN
byte    _extended = '*';  /* Allow long filenames */
#else
byte    _extended = 0xdb; /* Stick to 8.3 */
#endif
size_t  buflen;
char    _drive[MAXDRIVE], _dir[MAXDIR], _file[MAXFILE], _ext[MAXEXT];
char    spaces[] = "                                                   ";
char    *endstr;
char    *sendstr;
char    genpath[MAXPATH];
char    altname[MAXPATH];
char    delimit[] = "\n";
char def_format[] = "format.def";
const char cant[] = "\007\n'%s': Can't open. Break.\n";
const char notsame[] = "\007Filesize in %s differs from the original file!\n"
"Break.\n";
const char nomem[] = "\007Argh error: Not enough memory present! "
"Can't continue.....\n";
int     noquery = 0;
int     force = 0;
int     fls = 0;
int     autokill = 0;
int     simulate = 0;
int     sysop = 0;
int     no_tty = 0;
int     twolinesend = 0;
struct  m_index *idxptr;





#ifdef __TOS__
int   nowait  = 0;
#endif

const char logon_ctrl[] =


"\n"
BKG"浜様様様様様様様様様様様様様様様様様様様様様様様様様融"DFT"\n"
BKG"�"CHR"%s"BKG"�"DFT"\n"
BKG"�"CHR"%s"BKG"�"DFT"\n"
BKG"�"CHR"%s"BKG"�"DFT"\n"
BKG"藩様様様様様様様様様様様様様様様様様様様様様様様様様夕"DFT"\n";
#define LOGON_OK
#endif

#ifdef __TOS__
"\033p\033v\n"
"%s\n"
"%s\n"
"%s\n"
"\033q\n";
#define LOGON_OK
#endif




#ifndef LOGON_OK /* default logon */
"\n"
"[]--------------------------------------------------[]\n"
"|%s|\n"
"|%s|\n"
"|%s|\n"
"[]--------------------------------------------------[]\n";
#endif

#ifdef LFN
#define _LFN "/LFN"
#else
#define _LFN "/8.3"
#endif

const char *logon[] = { "     7PLUS - file converter for store & forward     ",
" * no commercial use * no sale * circulate freely * ",
" version "VERSION""_LFN" ("PL7_DATE") (C) Axel Bauda, DG1BBQ " };

const char s_logon[] = "\n[7+ v"VERSION""_LFN" ("PL7_DATE"), (C) DG1BBQ]\n";

const char *help[] = {
	"\n",
	"Commands (more exact descriptions, see manual): \n",
	"\n",
	"7plus file   <-- 'file' must not carry an extension (.7pl/.p01/.cor etc.)\n",
	"  Automagic mode. 7PLUS will look for the right files and try to decode\n",
	"  and correct, if possible. If a non-7PLUS file named 'file' exists, 7PLUS\n",
	"  will encode it.\n",
	"\n",
	"7plus file.txt\n",
	"  Encode 'file.txt' (automatically split into 10K chunks).\n",
	"\n",
	"  Valid options for encoding:\n",
	"\n",
	"  -s 30      30 lines/part (max 512 lines/part).\n",
	"  -sp 3      3 parts of roughly equal size (max 255 parts).\n",
	"  -sb 3000   Parts of roughly 3000 bytes (max 36000).\n",
	"  -r 5-10,1  When encoding, only create part 5 through 10 and part 1.\n",
	"             Be sure to split the same way as for the first upload!\n",
	"  -t /ex     Append string '/ex' to encoded files (BBS file termination).\n",
	"  -send \"sp dg1bbq @db0ver\" Add send command for BBS (-send2 = 2 line send).\n",
	"  -tb file   Get head and footlines from format file 'file' when encoding.\n",
	"             Produces ready-for-upload files. See manual.\n",
	"  -j         Join all parts into a single output file 'file.upl'.\n",
	"\n",

#define EXMPL "c:\\pr\\"






	"7plus file.err "EXMPL"\n",
	"  Create correction file. Look for original unencoded file in '"EXMPL"'.\n",
	"  Omit path of original unencoded file, if it's in the current directory.\n",
	"\n",
	"7plus file.err file2.err -j\n",
	"  Add contents of error report 'file2.err' to error report 'file.err'.\n",
	"  If second filename is omitted, 7PLUS will look for 'file.e01', 'file.e02'\n",
	"  etc and add their contents to 'file.err' (automatic multiple join).\n"
	"  Joining of err-files is necessary for producing collective cor-files.\n",
	"\n",

#ifdef _HAVE_CHSIZE
#define MF "7mf"
#define MG "meta"
#else
#define MF "7ix"
#define MG "index"
#endif

	"7plus file."MF"\n",
	"  Create new error report from "MG"file, if it's been accidentally erased.\n",
	"\n",
	"7plus logfile -x text\n",
	"  Extract 7PLUS files from 'logfile'. Only extract a file, if its name\n",
	"  contains 'text'. Omit 'text' to extract all files in 'logfile'.\n",
	"\n",
	"Other Options (not all options listed):\n",
	"\n",
	"-k    Automatically kill all obsolete files.\n",
#ifdef __TOS__
	"-n    Don't wait for a keystroke at termination.\n",
#endif
	"-p    Use Packet line separator CR for encoded files. Should be used,\n",
	"      when uploading files to the BBS in binary mode! See manual.\n",
	"-y    Assume YES on all queries.\n",
	"\n",
	"And do me a great favor: READ THE MANUAL, PLEASE! 73s, Axel.\n",
	"\n",
	NULLCP
};

#ifdef __DLL__

/*
 * DLL initialisation
 */

#pragma warn -par

/*
 * 32 bit DLL initialisation.
 */
HANDLE hDLLInst = 0;

BOOL WINAPI DllMain(HANDLE hModule, DWORD dwFunction, LPVOID lpNot)
{
	hDLLInst = hModule;

	switch (dwFunction)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_PROCESS_DETACH:
	default:
		break;
	}
	return TRUE;
}

#pragma warn +par

/*
 * The real DLL entry point
 */
int __export CALLBACK Do_7plus(char *cmd_line)
{
	char *p1, *p2;
	char **argv;
	int argc = 0;
	int i, l;
	int ret;

	/*
	 * Count the args.
	 *
	 * Long Windows 9x file names may contain spaces,
	 * a long file name could look like this...
	 *
	 * "This is a long win9x file called fumph.zip"
	 *
	 * Note the " " surrounding the file name.
	 */
	l = strlen(cmd_line);

	for (i = 0; i <= l; i++)
	{
		if (cmd_line[i] == '"')
		{
			i++;
			while (cmd_line[i] != '"') i++;
			i++;
		}
		/*
		 * Replace ' ' with '\0' unless surrounded with quotes
		 * to indicate the spaces are inside a long file name.
		 */
		if (cmd_line[i] == ' ')
		{
			cmd_line[i] = 0;
			argc++;
		}
	}

	/*
	 * The number of args should be one more than the spaces.
	 */
	argc++;

	/*
	 * Allocate the pointers.
	 */
	argv = (char **) calloc (argc, sizeof (char *));

	/*
	 * Process cmd_line again setting up argv.
	 */
	p1 = cmd_line;

	for (i = 0; i < argc; i++)
	{
		argv[i] = p1;
		p2 = strchr(p1,0);
		if (p2) p1 = p2+1;
	}

	/*
	 * Remove any quotes
	 */
	for (i = 0; i < argc; i++)
		if (argv[i] [0] == '"')
			for (l = 0; argv[i] [l]; l++)
			{
				argv[i] [l] = argv[i] [l + 1];
				if (argv[i] [l] == '"')
					argv[i] [l] = 0;
			}

	/*
	 * Call real program entry point
	 */
	ret = go_at_it(argc,argv);

	fclose(o);
	return ret;
}

#else /* #ifdef __DLL__ #else */

/* Depending on the system, it may be nessesary to prompt the user for a
   keystroke, before terminating, because user wouldn't be able to read
   the outputs to the screen, when the window closes at termination.
   However, the '-n' option overrides this. */
int main(int argc, char **argv)
{
#ifdef __TOS__
	int ret;

	ret = go_at_it (argc, argv);
	if ((!nowait || !noquery) && !system(NULLCP))
	{
		printf("\n\033p Hit any key \033q");
		getch();
	}
	return (ret);
#else





	return (go_at_it(argc, argv));

#endif
}
#endif /* #ifdef __DLL__ #else */

/* This is the real main() */
int go_at_it(int argc, char **argv)
{

	char *p, *r, *s, *t;
	char argname[MAXPATH];
	int  ret, i, extract, genflag, join, cor;
	long blocksize;


	extract = genflag = join = cor = twolinesend = 0;
	p = r = s = t = endstr = sendstr = NULLCP;
	*genpath = *argname = *altname = EOS;

#ifndef __DLL__
	i = 0;
	o = stdout;
#else
	o = fopen("\\7plus.out","w");
	noquery = 1;
	i = -1; /* Args start at 0 with DLL */
#endif

	/* initialize range array */
	get_range("1-");

	/* Default blocksize (abt 10000 bytes) */
	blocksize = 138 * 62;



	while (++i < argc)
	{
		if (*argv[i] != '-')
		{
			if (!p)
			{
				p = argv[i];  /* Name of file to de/encode/correct */
				continue;
			}
			if (!r)
			{
				r = argv[i];  /* Searchpath for non-coded file. Needed for */
				continue;     /* generating correction file */
			}
		}

		if (!stricmp(argv[i], "-S")) /* Split option */
		{
			i++;
			if (i == argc)
			{
				blocksize = 512 * 62;  /* No parameter, set max blocksize */
				i--;
			}
			else
				if (sscanf(argv[i], "%li", &blocksize) == 1 && *argv[i] != '-')
					blocksize *= 62L; /* Set blocksize to parameter */
				else
					blocksize = 512 * 62; /* Next arg is not a parm. Set max blocksize */

		}

		if (!stricmp(argv[i], "-SP")) /* Split into equal parts */
		{
			i++;
			if (i == argc)
			{
				blocksize = 0; /* No parameter, no user defined split */
				i--;
			}
			else
				if (sscanf(argv[i], "%li", &blocksize) == 1)
					blocksize += 50000L; /* Number of parts to encode (50000 used as
											 indicator) */
		}

		if (!stricmp(argv[i], "-SB")) /* Split in parts of n bytes */
		{
			i++;
			if (i == argc)
				i--;
			else
				if (sscanf(argv[i], "%li", &blocksize) == 1)
					blocksize = (blocksize / 71 - 2) * 62;
		}

		if (!stricmp(argv[i], "-R")) /* Only re-encode specified part(s) */
		{
			i++;
			if (i == argc)
				i--;
			else
				get_range(argv[i]);
		}

		if (!stricmp(argv[i], "-TB")) /* File to get head and foot lines from */
		{
			i++;
			if (i == argc)
			{
				t = def_format;
				i--;
			}
			else
				if (*argv[i] != '-')
					t = argv[i];
				else
					t = def_format;
		}

		if (!stricmp(argv[i], "-T")) /* Define BBS's termination string, */
		{                             /* e.g. "/ex" */
			i++;
			if (i == argc)
				i--;
			else
			{
				if (t != def_format)
				{
					endstr = (char *)malloc((int)strlen(argv[i]) + 1);
					strcpy(endstr, argv[i]);
				}
			}
		}

		if (!strnicmp(argv[i], "-SEND", 5)) /* Define send string, */
		{                                /* e.g. "sp dg1bbq @db0ver.#nds.deu.eu" */
			if (argv[i][5] == '2')
				twolinesend = 1;
			i++;
			if (i == argc)
				i--;
			else
			{
				if (t != def_format)
				{
					sendstr = (char *)malloc((int)strlen(argv[i]) + 1);
					strcpy(sendstr, argv[i]);
				}
			}
		}

		if (!stricmp(argv[i], "-U")) /* Set alternative filename */
		{
			i++;
			if (i == argc)
				i--;
			else
				strcpy(altname, argv[i]);
		}

		if (!stricmp(argv[i], "-#")) /* Create 7PLUS.FLS. Contents e.g.:     */
			fls = 1;                    /* 10 TEST */
		/* for TEST.EXE encoded into 10 parts   */

		if (!stricmp(argv[i], "-C")) /* Use 7PLUS-file as a correction file  */
			cor = 1;

		if (!stricmp(argv[i], "-K")) /* Kill obsolete files, stop if gap */
			autokill = 1;               /* greater than 10 files (faster)   */

		if (!stricmp(argv[i], "-KA"))/* Kill all obsolete files        */
			autokill = 2;               /* (slow, but better for servers) */

		if (!stricmp(argv[i], "-F")) /* Force usage of correction file */
			force = 1;

		if (!stricmp(argv[i], "-G")) /* Write to same dir as input file */
			genflag = 1;

		if (!stricmp(argv[i], "-J")) /* Join two error reports / Produce single */
			join = 1;                   /* output file when encoding               */

#ifdef __TOS__
		if (!stricmp (argv[i], "-N")) /* Don't wait for a key at termination  */
			nowait = 1;
#endif

		if (!stricmp(argv[i], "-P")) /* Write encoded files in Packet format */
			sprintf(delimit, "\r");    /* for direct binary upload. */

		if (!stricmp(argv[i], "-Q")) /* Quiet mode. Absolutely no screen output */
		{

			o = fopen("7plus.out", OPEN_WRITE_TEXT);

			noquery = 1;
		}

		if (!stricmp(argv[i], "-SIM")) /* Simulate encoding and report */
			simulate = 1;                 /* number of parts and parts */
		/* filename in 7plus.fls */

		if (!stricmp(argv[i], "-SYSOP")) /* SYSOP mode. Decode, even if parts */
			sysop = 1;                      /* are missing. */

		if (!stricmp(argv[i], "-X")) /* Extract 7plus-files from log-file    */
			extract = 1;

		if (!stricmp(argv[i], "-Y")) /* Always assume YES on queries.*/
			noquery = 1;

	}

	if (!isatty(fileno(o)))
		no_tty = noquery = 1;

	if (no_tty)
		fprintf(o, "%s", s_logon);
	else



		if (!p) /* No File specified, show help */
		{
			int scrlines;
			int n = 5;

			i = 0;

			/* How many lines fit on screen? */
			scrlines = screenlength() - 1;

			while (help[i])
			{
				if (++n == scrlines && !noquery)
				{
					set_autolf(0);

					fprintf(o, "Press RETURN to continue....\r");

					fflush(stdout);
					while (!getch());
					fflush(stdin);
					n = 0;

					set_autolf(1);
				}
				fprintf(o, help[i++]);
			}
			ret = 0;
			goto end;
		}

	if ((s = (char *)malloc((size_t)4000UL)) == NULLCP)
	{
		fprintf(o, nomem);
		if (o != stdout)
			fclose(o);
#ifndef __DLL__
		exit(21);
#else
		return 21;
#endif
	}
	free(s);

	if ((idxptr = (struct m_index *)malloc(sizeof(struct m_index))) == NULL)
	{
		fprintf(o, nomem);
		if (o != stdout)
			fclose(o);
#ifndef __DLL__
		exit(21);
#else
		return 21;
#endif
	}

	buflen = 16384;

	init_crctab(); /* Initialize table for CRC-calculation */
	init_decodetab(); /* decoding-table */
	init_codetab(); /* encoding-table */

	strcpy(argname, p);


	{ /* Since Win32 does not distinguish the case in filenames, findfirst
	  ** is used to determine how the filename is really spelt casewise.
	  ** It's usefull for DOS also.
	  */
		struct ffblk ffblk; /* only needed locally */
		if (findfirst (p, &ffblk, 0) == 0)
		{
			fnsplit (p, _drive, _dir, NULL, NULL);
			sprintf (argname, "%s%s%s", _drive, _dir, ffblk.ff_name);
		}
	}


	fnsplit(argname, _drive, _dir, _file, _ext);

	if (genflag)
		sprintf(genpath, "%s%s", _drive, _dir);

	if (extract)
	{
		if (p)
			ret = extract_files(argname, r);
		else
		{
			fprintf(o, "\007File to extract from not specified. Break.\n");
			ret = 6;
		}
		goto end;
	}
	/* Does the filename contain an extension? */
	if (*_ext)
	{
		if (cor)
		{
			ret = correct_meta(argname, 0, 0);
			goto end;
		}

		if (join)
			if (!strnicmp(".err", _ext, 4) ||
				(toupper(*(_ext + 1)) == 'E' &&
				isxdigit(*(_ext + 2)) &&
				isxdigit(*(_ext + 3))))
			{
				ret = join_control(argname, r);
				goto end;
			}

		if (!strnicmp(".cor", _ext, 4) ||
			(toupper(*(_ext + 1)) == 'C' &&
			isxdigit(*(_ext + 2)) &&
			isxdigit(*(_ext + 3))))
		{
			ret = correct_meta(argname, 1, 0);
			goto end;
		}

		if (sysop)
		{
			ret = control_decode(argname);
			goto end;
		}

		/* Call decode_file() if ext ist 7PL, P01, else encode_file() */
		if (!strnicmp(".7pl", _ext, 4) || !strnicmp(".p01", _ext, 4))
		{
			ret = control_decode(argname);
			goto end;
		}
#ifdef _HAVE_CHSIZE
		if (!strnicmp (".7mf", _ext, 4))
#else
		if (!strnicmp(".7ix", _ext, 4))
#endif
		{
			ret = make_new_err(argname);
			goto end;
		}

		if (!strnicmp(".x", _ext, 3))
		{
			ret = extract_files(argname, r);
			goto end;
		}
		ret = encode_file(argname, blocksize, r, join, t);
	}
	else
	{
		if (!test_exist(argname)) /* no EXT, but file exists on disk, then encode */
			ret = encode_file(argname, blocksize, r, join, t);
		else
			ret = control_decode(argname);
	}

end:
	if (o != stdout)
		fclose(o);
	free(idxptr);
	return (ret);
}

/*
*** How many lines fit on screen?
***
***
*/
int screenlength(void)
{
	int scrlines;


#ifdef __EMX__
	/* Find out, how many lines fit on screen.
	   _scrsize() probably only available with GNU_C/EMX */
	{
		int dst[2];
		_scrsize (dst);
		scrlines = dst[1];
	}
#endif

#ifdef __IBMC__
	/* IBM C++ Set/2 & Developer's Toolkit/2 , Borland C++ */
#define INCL_VIO
#include <os2.h>
	{
		VIOMODEINFO VioModeInfo;
		HVIO hvio = 0;
		VioModeInfo.cb = sizeof (VioModeInfo);
		VioGetMode (&VioModeInfo, hvio);
		scrlines = VioModeInfo.row;
	}
#endif


	
	{
		struct text_info t;
		gettextinfo (&t);
		scrlines = t.screenheight;
	}


	return (scrlines);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
 *+ Possible return codes:                                                 +*
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*

 0 No errors detected.
 1 Write error.
 2 File not found.
 3 7PLUS header not found.
 4 File does not contain expected part.
 5 7PLUS header corrrupted.
 6 No filename for extracting defined.
 7 invalid error report / correction / index file.
 8 Max number of parts exceeded.
 9 Bit 8 stripped.
 10 User break in test_file();
 11 Error report generated.
 12 Only one or no error report to join
 13 Error report/cor-file does not refer to the same original file
 14 Couldn't write 7plus.fls
 15 Filesize of original file and the size reported in err/cor-file not equal
 16 Correction not successful.
 17 No CRC found in err/cor-file.
 18 Timestamp in metafile differs from that in the correction file.
 19 Metafile already exists.
 20 Can't encode files with 0 filelength.
 21 Not enough memory available

 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
