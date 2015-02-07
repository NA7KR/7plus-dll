/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                                   TEST.C                                   */
/*                                                                            */
/*       Demonstrates how to load and link into a dynamic link library        */
/*                                                                            */
/*       This simple demonstration loads the 7plus dynamic link library       */
/*               and passes on to it the command line arguements              */
/*                      it is only a demo, use with care                      */
/*                                                                            */
/*                Example ...   TEST -SB 5000 "Long Name.zip"                 */
/*                    c:\temp\7plus.zip -SAVE "c:\temp\"                      */
/*                   c:\temp\7plus.p01 - SAVE "c:\temp\"                      */
/*       Long file names are supported in the 32 bit DLL, and should be       */
/*         surrounded with quotes, short file names don't need quotes         */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include <windows.h>

HINSTANCE hInst;

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
	HINSTANCE hinstDo7plus;
	int (FAR *lpfnDo7plus) (char FAR*);
	const char help[] = "!-!p 		/* Name of file to de/encode/correct */\n!-!r 		/* Searchpath for non-coded file. Needed for */\n\n-S 		/* Split option */\n-SP 		/* Split into equal parts */	\n-SB 		/* Split in parts of n bytes */		\n-R 		/* Only re-encode specified part(s) */		\n-TB 		/* File to get head and foot lines from */		\n-T 		/* Define BBS's termination string, */\n-SEND 		/* Define send string, */\n-U 		/* Set alternative filename */\n-# 		/* Create 7PLUS.FLS. Contents e.g.:     */\n		/* for TEST.EXE encoded into 10 parts   */\n-C 		/* Use 7PLUS-file as a correction file  */\n-K 		/* Kill obsolete files, stop if gap */\n-KA		/* Kill all obsolete files        */	\n-F 		/* Force usage of correction file */	\n-G 		/* Write to same dir as input file */\n-J 		/* Join two error reports / Produce single */\n-P 		/* Write encoded files in Packet format */\n-Q 		/* Quiet mode. Absolutely no screen output */	\n-SIM 		/* Simulate encoding and report */\n-SYSOP 		/* SYSOP mode. Decode, even if parts */\n-X 		/* Extract 7plus-files from log-file    */\n-Y 		/* Always assume YES on queries.*/\n";
	if (lpszArgs != NULL && lpszArgs[0] == '\0')
	{
		MessageBox(NULL, help , "Error", MB_OK);
		return 10;
	}
	if (lpszArgs[0] == '?')
	{
		MessageBox(NULL, help, "Error", MB_OK);
		return 10;
	}

	if (lpszArgs[0] == '\\')
	{
		if (lpszArgs[1] == '?')
		{
			MessageBox(NULL, help, "Error", MB_OK);
			return 10;
		}
	}

	if (lpszArgs[0] == '/')
	{
		if (lpszArgs[1] == '?')
		{
			MessageBox(NULL, help, "Error", MB_OK);
			return 10;
		}
	}
	/*
	* The DLL in this example is called 7PLUSDLL.DLL
	*/
	hinstDo7plus = LoadLibrary("7plus.dll");

	/*
	* Load DLL
	*/
	if (hinstDo7plus == NULL)
	{
		MessageBox(0, "Cannot load\n7PLUSDLL.DLL", "Error", MB_ICONSTOP | MB_OK);
		goto end;
	}

	/*
	* The entry point into the DLL is in this example called Do_7plus
	*/
	(FARPROC)lpfnDo7plus = GetProcAddress(hinstDo7plus, "Do_7plus");

	if (lpfnDo7plus == NULL)
	{
		MessageBox(0, "Cannot Link DLL", "Error", MB_ICONSTOP | MB_OK);
	goto end;
	}

	/*
	* Execute the DLL passing it the command line arguements
	*/
	lpfnDo7plus((char FAR*) lpszArgs);

end:
	FreeLibrary(hinstDo7plus);
	return 0;
}
