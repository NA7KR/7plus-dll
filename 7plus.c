/* Version */
#define VERSION "2.25"
#define PL7_DATE "20000320"

#include "7plus.h"
#include <conio.h>

FILE* o;
uint crctab[256];
byte decode[256];
byte code[216];
char range[257];
byte _extended = '*'; /* Allow long filenames */
size_t buflen;
char _drive[MAXDRIVE], _dir[MAXDIR], _file[MAXFILE], _ext[MAXEXT];
char spaces[] = "                                                   ";
char* endstr;
char* sendstr;
char* pathstr;
char genpath[MAXPATH];
char altname[MAXPATH];
char delimit[] = "\n";
char def_format[] = "format.def";
const char cant[] = "\007\n'%s': Can't open. Break.\n";
const char notsame[] = "\007Filesize in %s differs from the original file!\n" "Break.\n";
const char nomem[] = "\007Argh error: Not enough memory present! " "Can't continue.....\n";
int noquery = 0;
int force = 0;
int fls = 0;
int autokill = 0;
int simulate = 0;
int sysop = 0;
int no_tty = 0;
int twolinesend = 0;
struct m_index* idxptr;
#define _LFN "/LFN"

const char s_logon[] = "\n[7+ v"VERSION""_LFN" ("PL7_DATE"), (C) DG1BBQ]\n";

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


// The real DLL entry point
//int __export CALLBACK Do_7plus(char *cmd_line)
__declspec(dllexport) int  Do_7plus(char *cmd_line)
{
	char *p1, *p2;
	char **argv;
	int argc = 0;
	int i, l;
	int ret;

	/*
		* Count the args. Long Windows 9x file names may contain spaces,
		* a long file name could look like this... "This is a long win9x file called fumph.zip" Note the " " surrounding the file name.
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
		//	* Replace ' ' with '\0' unless surrounded with quotes to indicate the spaces are inside a long file name.
		if (cmd_line[i] == ' ')
		{
			cmd_line[i] = 0;
			argc++;
		}
	}

	// The number of args should be one more than the spaces.
	argc++;

	// Allocate the pointers.

	argv = (char **)calloc(argc, sizeof(char *));

	// Process cmd_line again setting up argv.

	p1 = cmd_line;

	for (i = 0; i < argc; i++)
	{
		argv[i] = p1;
		p2 = strchr(p1, 0);
		if (p2) p1 = p2 + 1;
	}

	// Remove any quotes

	for (i = 0; i < argc; i++)
		if (argv[i][0] == '"')
			for (l = 0; argv[i][l]; l++)
			{
				argv[i][l] = argv[i][l + 1];
				if (argv[i][l] == '"')
					argv[i][l] = 0;
			}

	// Call real program entry point

	ret = go_at_it(argc, argv);
	fclose(o);
	return ret;
}



/* This is the real main() */
int go_at_it(int argc, char** argv)
{
	char *p, *r, *s, *t;
	char argname[MAXPATH];
	int ret, i, extract, genflag, join, cor;
	long blocksize;

	extract = genflag = join = cor = twolinesend = 0;
	p = r = s = t = endstr = sendstr = NULLCP;
	*genpath = *argname = *altname = EOS;

	i = 0;
	o = stdout;

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
				p = argv[i]; /* Name of file to de/encode/correct */
				continue;
			}
			if (!r)
			{
				r = argv[i]; /* Searchpath for non-coded file. Needed for */
				continue; /* generating correction file */
			}
		}

		if (!_stricmp(argv[i], "-S")) /* Split option */
		{
			i++;
			if (i == argc)
			{
				blocksize = 512 * 62; /* No parameter, set max blocksize */
				i--;
			}
			else if (sscanf(argv[i], "%li", &blocksize) == 1 && *argv[i] != '-')
				blocksize *= 62L; /* Set blocksize to parameter */
			else
				blocksize = 512 * 62; /* Next arg is not a parm. Set max blocksize */
		}

		if (!_stricmp(argv[i], "-SP")) /* Split into equal parts */
		{
			i++;
			if (i == argc)
			{
				blocksize = 0; /* No parameter, no user defined split */
				i--;
			}
			else if (sscanf(argv[i], "%li", &blocksize) == 1)
				blocksize += 50000L; /* Number of parts to encode (50000 used as
											 indicator) */
		}

		if (!_stricmp(argv[i], "-SB")) /* Split in parts of n bytes */
		{
			i++;
			if (i == argc)
				i--;
			else if (sscanf(argv[i], "%li", &blocksize) == 1)
				blocksize = (blocksize / 71 - 2) * 62;
		}

		if (!_stricmp(argv[i], "-R")) /* Only re-encode specified part(s) */
		{
			i++;
			if (i == argc)
				i--;
			else
				get_range(argv[i]);
		}

		if (!_stricmp(argv[i], "-TB")) /* File to get head and foot lines from */
		{
			i++;
			if (i == argc)
			{
				t = def_format;
				i--;
			}
			else if (*argv[i] != '-')
				t = argv[i];
			else
				t = def_format;
		}

		if (!_stricmp(argv[i], "-T")) /* Define BBS's termination string, */
		{ /* e.g. "/ex" */
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
		// Save KRR
		if (!_strnicmp(argv[i], "-SAVE", 5))
		{
			i++;
			if (i == argc)
				i--;
			else
			{
				if (t != def_format)
				{
					pathstr = (char *)malloc((int)strlen(argv[i]) + 1);
					strcpy(pathstr, argv[i]);
				}
			}

		}
		//end save
		if (!_strnicmp(argv[i], "-SEND", 5)) /* Define send string, */
		{ /* e.g. "sp dg1bbq @db0ver.#nds.deu.eu" */
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

		if (!_stricmp(argv[i], "-U")) /* Set alternative filename */
		{
			i++;
			if (i == argc)
				i--;
			else
				strcpy(altname, argv[i]);
		}

		if (!_stricmp(argv[i], "-#")) /* Create 7PLUS.FLS. Contents e.g.:     */
			fls = 1; /* 10 TEST */
		/* for TEST.EXE encoded into 10 parts   */

		if (!_stricmp(argv[i], "-C")) /* Use 7PLUS-file as a correction file  */
			cor = 1;

		if (!_stricmp(argv[i], "-K")) /* Kill obsolete files, stop if gap */
			autokill = 1; /* greater than 10 files (faster)   */

		if (!_stricmp(argv[i], "-KA"))/* Kill all obsolete files        */
			autokill = 2; /* (slow, but better for servers) */

		if (!_stricmp(argv[i], "-F")) /* Force usage of correction file */
			force = 1;

		if (!_stricmp(argv[i], "-G")) /* Write to same dir as input file */
			genflag = 1;

		if (!_stricmp(argv[i], "-J")) /* Join two error reports / Produce single */
			join = 1; /* output file when encoding               */


		if (!_stricmp(argv[i], "-P")) /* Write encoded files in Packet format */
			sprintf(delimit, "\r"); /* for direct binary upload. */



		if (!_stricmp(argv[i], "-SIM")) /* Simulate encoding and report */
			simulate = 1; /* number of parts and parts */
		/* filename in 7plus.fls */

		if (!_stricmp(argv[i], "-SYSOP")) /* SYSOP mode. Decode, even if parts */
			sysop = 1; /* are missing. */

		if (!_stricmp(argv[i], "-X")) /* Extract 7plus-files from log-file    */
			extract = 1;

		if (!_stricmp(argv[i], "-Y")) /* Always assume YES on queries.*/
			noquery = 1;
	}

	if (!_isatty(_fileno(o)))
		no_tty = noquery = 1;

	if (no_tty)
		fprintf(o, "%s", s_logon);
	else if (!p) /* No File specified, show help */
	{
		int scrlines;
		int n = 5;

		i = 0;

		/* How many lines fit on screen? */
		scrlines = 40;


		ret = 0;
		if (o != stdout)
			fclose(o);
		free(idxptr);
		return (ret);
	}

	if ((s = (char *)malloc((size_t)4000UL)) == NULLCP)
	{
		fprintf(o, nomem);
		if (o != stdout)
			fclose(o);

		exit(21);

	}
	free(s);

	if ((idxptr = (struct m_index *)malloc(sizeof(struct m_index))) == NULL)
	{
		fprintf(o, nomem);
		if (o != stdout)
			fclose(o);

		exit(21);

	}

	buflen = 16384;

	init_crctab(); /* Initialize table for CRC-calculation */
	init_decodetab(); /* decoding-table */
	init_codetab(); /* encoding-table */

	strcpy(argname, p);
	// KRR
	if (pathstr == NULL)
	{
		fnsplit(argname, _drive, _dir, _file, _ext);
	}
	else
	{
		fnsplit(argname, _drive, _dir, _file, _ext);

		char *c = malloc(strlen(pathstr) + strlen(_file) + strlen(_ext) + 1);
		if (c != NULL)
		{
			strcpy(c, pathstr);
			strcat(c, _file);
			strcat(c, _ext);
			strcpy(argname, c);
			strcpy(genpath, pathstr);

			free(c);
		}

	}
	//KRR 
	// fnsplit(argname, _drive, _dir, _file, _ext);
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
		if (o != stdout)
			fclose(o);
		free(idxptr);
		return (ret);
	}
	/* Does the filename contain an extension? */
	if (*_ext)
	{
		if (cor)
		{
			ret = correct_meta(argname, 0, 0);
			if (o != stdout)
				fclose(o);
			free(idxptr);
			return (ret);
		}

		if (join)
			if (!_strnicmp(".err", _ext, 4) ||
				(toupper(*(_ext + 1)) == 'E' &&
				isxdigit(*(_ext + 2)) &&
				isxdigit(*(_ext + 3))))
			{
				ret = join_control(argname, r);
				if (o != stdout)
					fclose(o);
				free(idxptr);
				return (ret);
			}

		if (!_strnicmp(".cor", _ext, 4) ||
			(toupper(*(_ext + 1)) == 'C' &&
			isxdigit(*(_ext + 2)) &&
			isxdigit(*(_ext + 3))))
		{
			ret = correct_meta(argname, 1, 0);
			if (o != stdout)
				fclose(o);
			free(idxptr);
			return (ret);
		}

		if (sysop)
		{
			ret = control_decode(argname);
			if (o != stdout)
				fclose(o);
			free(idxptr);
			return (ret);
		}

		/* Call decode_file() if ext ist 7PL, P01, else encode_file() */
		if (!_strnicmp(".7pl", _ext, 4) || !_strnicmp(".p01", _ext, 4))
		{
			ret = control_decode(argname);
			if (o != stdout)
				fclose(o);
			free(idxptr);
			return (ret);
		}
#ifdef _HAVE_CHSIZE
		if (!_strnicmp(".7mf", _ext, 4))
#else
		if (!_strnicmp(".7ix", _ext, 4))
#endif
		{
			ret = make_new_err(argname);
			if (o != stdout)
				fclose(o);
			free(idxptr);
			return (ret);
		}

		if (!_strnicmp(".x", _ext, 3))
		{
			ret = extract_files(argname, r);
			if (o != stdout)
				fclose(o);
			free(idxptr);
			return (ret);
		}
		ret = encode_file(argname, blocksize, r, join, t, genpath);
	}
	else
	{
		if (!test_exist(argname)) /* no EXT, but file exists on disk, then encode */
			ret = encode_file(argname, blocksize, r, join, t, genpath);
		else
			ret = control_decode(argname);
	}

	if (o != stdout)
		fclose(o);
	free(idxptr);
	return (ret);
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