/*
 * kenny -- a command-line kennyfier
 * Copyright 2002 Kohan Ikin <syneryder@namesuppressed.com>
 * Ported to C by Colin Leroy <colin@colino.net>
 * To compile use `gcc -o kenny kenny.c`
 * To run use `kenny "my string here"`
 *
 * Thanks to:
 *	Peter Palmreuther <pit@zentrumderarbeit.org> for a 
 *	reverse-translation bugfix.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

//#include <stdio.h>
#include "EVEServerPCH.h"

#define NORMAL 		0
#define KENNYSPEAK 	1
#define FALSE		0
#define TRUE		1

/*
void usage (void)
{
	printf("kenny STRING\n");
}
*/

int is_kennyspeak (char *str) 
{
	int i=0;
	for (i=0; i < strlen(str); i++) {
		if ((*str >= 'a' && *str <= 'z' && *str != 'm' && *str != 'f' && *str != 'p')
		 || (*str >= 'A' && *str <= 'Z' && *str != 'M' && *str != 'F' && *str != 'P')) 
			return FALSE;
		*str++;
	}
	return TRUE;
}

void normal_to_kennyspeak (std::string str, std::string &kenny_text)
{
	std::string add;
	kenny_text = "";
    unsigned long index = 0;
	while (index < str.size()) {
		if (str[index] < 'A' || (str[index] > 'Z' && str[index] < 'a') || str[index] > 'z') {
			kenny_text += str[index];
		} else {
			add = "";
			switch (str[index]) {
				case 'e': add = "mpp"; break;
				case 't': add = "fmp"; break;
				case 'a': add = "mmm"; break;
				case 'o': add = "ppf"; break;
				case 'i': add = "mff"; break;
				case 'n': add = "ppp"; break;
				case 's': add = "fmm"; break;
				case 'h': add = "mfp"; break;
				case 'r': add = "pff"; break;
				case 'd': add = "mpm"; break;
				case 'l': add = "pmf"; break;
				case 'c': add = "mmf"; break;
				case 'u': add = "fmf"; break;
				case 'm': add = "ppm"; break;
				case 'w': add = "fpp"; break;
				case 'f': add = "mpf"; break;
				case 'g': add = "mfm"; break;
				case 'y': add = "ffm"; break;
				case 'p': add = "pfm"; break;
				case 'b': add = "mmp"; break;
				case 'v': add = "fpm"; break;
				case 'k': add = "pmp"; break;
				case 'j': add = "pmm"; break;
				case 'x': add = "fpf"; break;
				case 'q': add = "pfp"; break;
				case 'z': add = "ffp"; break;
				case 'E': add = "Mpp"; break;
				case 'T': add = "Fmp"; break;
				case 'A': add = "Mmm"; break;
				case 'O': add = "Ppf"; break;
				case 'I': add = "Mff"; break;
				case 'N': add = "Ppp"; break;
				case 'S': add = "Fmm"; break;
				case 'H': add = "Mfp"; break;
				case 'R': add = "Pff"; break;
				case 'D': add = "Mpm"; break;
				case 'L': add = "Pmf"; break;
				case 'C': add = "Mmf"; break;
				case 'U': add = "Fmf"; break;
				case 'M': add = "Ppm"; break;
				case 'W': add = "Fpp"; break;
				case 'F': add = "Mpf"; break;
				case 'G': add = "Mfm"; break;
				case 'Y': add = "Ffm"; break;
				case 'P': add = "Pfm"; break;
				case 'B': add = "Mmp"; break;
				case 'V': add = "Fpm"; break;
				case 'K': add = "Pmp"; break;
				case 'J': add = "Pmm"; break;
				case 'X': add = "Fpf"; break;
				case 'Q': add = "Pfp"; break;
				case 'Z': add = "Ffp"; break;
			}
			kenny_text += add;
		}
		index++;
	}
}

void kennyspeak_to_normal (std::string str, std::string &normal_text)
{
	std::string temp;
	std::string add;
	normal_text = "";
	int i = 0;
	int str_length = str.size();

	while (i < str_length) {
		if (i < str_length - 2)
		{
			temp = str[i+1];
			temp += str[i+2];
			//sprintf(temp,"%c%c",*(str+1),*(str+2));
		}
		if (str[i] < 'A' || (str[i] > 'Z' && str[i] < 'a') || str[i] > 'z') {
			normal_text += str[i];
			//sprintf(normal_text, "%s%c", normal_text, *str);
			i++;
			continue;
		}
		else if (str[i] == 'm')
		{
                 if (!temp.compare("pp")) add = 'e';
			else if (!temp.compare("mm")) add = 'a';
			else if (!temp.compare("ff")) add = 'i';
			else if (!temp.compare("fp")) add = 'h';
			else if (!temp.compare("pm")) add = 'd';
			else if (!temp.compare("mf")) add = 'c';
			else if (!temp.compare("pf")) add = 'f';
			else if (!temp.compare("fm")) add = 'g';
			else if (!temp.compare("mp")) add = 'b';
		}
		else if (str[i] == 'p')
		{
			     if (!temp.compare("pf")) add = 'o';
			else if (!temp.compare("pp")) add = 'n';
			else if (!temp.compare("ff")) add = 'r';
			else if (!temp.compare("mf")) add = 'l';
			else if (!temp.compare("pm")) add = 'm';
			else if (!temp.compare("fm")) add = 'p';
			else if (!temp.compare("mp")) add = 'k';
			else if (!temp.compare("mm")) add = 'j';
			else if (!temp.compare("fp")) add = 'q';
		}
		else if (str[i] == 'f')
		{
			     if (!temp.compare("mp")) add = 't';
			else if (!temp.compare("mm")) add = 's';
			else if (!temp.compare("mf")) add = 'u';
			else if (!temp.compare("pp")) add = 'w';
			else if (!temp.compare("fm")) add = 'y';
			else if (!temp.compare("pm")) add = 'v';
			else if (!temp.compare("pf")) add = 'x';
			else if (!temp.compare("fp")) add = 'z';
		}
		else if (str[i] == 'M')
		{
			     if (!temp.compare("pp")) add = 'E';
			else if (!temp.compare("mm")) add = 'A';
			else if (!temp.compare("ff")) add = 'I';
			else if (!temp.compare("fp")) add = 'H';
			else if (!temp.compare("pm")) add = 'D';
			else if (!temp.compare("mf")) add = 'C';
			else if (!temp.compare("pf")) add = 'F';
			else if (!temp.compare("fm")) add = 'G';
			else if (!temp.compare("mp")) add = 'B';
		}
		else if (str[i] == 'P')
		{
			     if (!temp.compare("pf")) add = 'O';
			else if (!temp.compare("pp")) add = 'N';
			else if (!temp.compare("ff")) add = 'R';
			else if (!temp.compare("mf")) add = 'L';
			else if (!temp.compare("pm")) add = 'M';
			else if (!temp.compare("fm")) add = 'P';
			else if (!temp.compare("mp")) add = 'K';
			else if (!temp.compare("mm")) add = 'J';
			else if (!temp.compare("fp")) add = 'Q';
		}
		else if (str[i] == 'F')
		{
			     if (!temp.compare("mp")) add = 'T';
			else if (!temp.compare("mm")) add = 'S';
			else if (!temp.compare("mf")) add = 'U';
			else if (!temp.compare("pp")) add = 'W';
			else if (!temp.compare("fm")) add = 'Y';
			else if (!temp.compare("pm")) add = 'V';
			else if (!temp.compare("pf")) add = 'X';
			else if (!temp.compare("fp")) add = 'Z';
		}
		//sprintf(normal_text,"%s%c",normal_text,add);
		normal_text += add;
		i+=3;
	}
}

/*
int main (int argc, char *argv[])
{
	if (argc < 2 || !strncmp(argv[1],"--help",6)) {
		usage();
	} else if (!strncmp(argv[1],"greggles is a steggles",22)) {
		printf("oh yes he is\n");
	} else if (!strncmp(argv[1],"hellokohan",10)) {
		printf("fittzuzfa!\n");
	} else if (!strncmp(argv[1],"ik!",3)) {
		printf("\"ik!\" is not an easter egg.\n");
	} else if (!strncmp(argv[1],"ik",2)) {
		printf("\"ik\" was Simon's idea, not mine!\n");
	} else if (!strncmp(argv[1],"ingy",4)) {
		printf("\"Sad eggs....\"\n\n- Ingy\nwww.ingy.net\n");
	} else if (!strncmp(argv[1],"CS302",5)) {
		printf("Pat Quincy Upbottom\n");
	} else if (!strncmp(argv[1],"Aeolas",6)) {
		printf("Aeolas thinks KennySpeak sounds like "
                       "someone getting head....\n\n"
                       "http://pub28.ezboard.com/ftweetysnestofr"
                       "eaksfrm1.showMessage?topicID=701.topic\n");
	} else if (is_kennyspeak(argv[1])) {
		printf("%s\n", to_normal(argv[1]));
	} else {
		printf("%s\n", to_kennyspeak(argv[1]));
	}
	return 0;
}
*/
