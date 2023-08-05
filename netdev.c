/*
#define DEBUG 1
*/

static char *version = "@(!--#) @(#) netdev.c, sversion 0.1.0, fversion 003, 05-august-2023";

/*
 *  px2emul
 *
 *  raritan PX2 emulator
 *
 */

/********************************************************************************/

/*
 *  includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>

/********************************************************************************/

/*
 *  defines
 */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MAX_LINE_LENGTH 1024

/********************************************************************************/

/*
 *  local includes
 */

#include "settings.h"

/********************************************************************************/

/*
 *  globals
 */

char *progname;

int   term;

/********************************************************************************/

/*
 *  rstrip
 *
 *  trim whitespace fron right hand side of a string
 *
 */

void rstrip(s)
  char *s;
{
  int   lenstring;
  int   c;

  lenstring = strlen(s);

  while (lenstring > 0) {
    lenstring--;

    c = s[lenstring];

    if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r')) {
      s[lenstring] = '\0';
    } else {
      break;
    }
  }

  return;
}

/********************************************************************************/

/*
 *  copyuntil
 *
 *  copy a string until the end of the string, a maximum number of characters
 *  has been copied or you get to a specified character
 *
 */

void copyuntil(dest, src, maxlength, stopchar)
  char *dest;
  char *src;
  int   maxlength;
  char  stopchar;
{
  char  c;

  while (maxlength > 0) {
    c = *src;

    if ((c == stopchar) || (c == '\0')) {
      break;
    }

    *dest = c;

    src++;
    dest++;
    maxlength--;
  }

  *dest = '\0';

  return;
}

/********************************************************************************/

/*
 *  beginswith
 *
 *  return TRUE is a string begins with another string
 *
 */

int beginswith(string, with)
  char *string;
  char *with;
{
  int   lenwith;

  lenwith = strlen(with);

  if (strncmp(string, with, lenwith) == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/********************************************************************************/

/*
 *  writeterm
 *
 *  write a string to the terminal
 *
 */

void writeterm(s)
  char *s;
{
  int   lens;
  int   n;

  lens = strlen(s);

  n = write(term, s, lens);

  if (n != lens) {
    fprintf(stderr, "\n%s: write error (n=%d)\n", progname, n);
  }

  tcdrain(term);

  return;
}

/********************************************************************************/

/*
 *  reverselookup
 *
 *  take an IP address and attempt a reverse lookup to get the associated name
 *
 */

void reverselookup(ipaddress, hostname, maxhostnamelength)
  char *ipaddress;
  char *hostname;
  int   maxhostnamelength;
{
  struct addrinfo *result;
  int error;

  *hostname = '\0';
 
  error = getaddrinfo(ipaddress, NULL, NULL, &result);

  if (error == 0) {   
    if (result != NULL) {
      error = getnameinfo(result->ai_addr, result->ai_addrlen, hostname, maxhostnamelength, NULL, 0, 0); 
    }
  }

  freeaddrinfo(result);

  return;
}

/********************************************************************************/

/*
 *  getinboundip
 *
 *  assuming a ssh login get the inbound IP address from SSH_CONNECTION
 *  i.e. the IP address (or name) used by the client to connect and login
 *
 */

void getinboundip(inboundip, maxinboundiplength)
  char *inboundip;
  int   maxinboundiplength;
{
  int   rval;
  char *ssh_conn;
  char  ssh_connection[MAX_LINE_LENGTH + sizeof(char)];
  char *token;

  *inboundip = '\0';

  ssh_conn = getenv("SSH_CONNECTION");

  if (ssh_conn == NULL) {
    fprintf(stderr, "%s: Warning: unable to get value of SSH_CONNECTION environment variable\n", progname);
    return;
  }

  if (strlen(ssh_conn) < 8) {
    fprintf(stderr, "%s: Warning: too little content in SSH_CONNECTION environment\n", progname);
    return;
  }

  if (strlen(ssh_conn) >= (MAX_LINE_LENGTH - sizeof(char))) {
    fprintf(stderr, "%s: Warning: too much content in SSH_CONNECTION environment\n", progname);
    return;
  }

  strncpy(ssh_connection, ssh_conn, MAX_LINE_LENGTH);

  if ((token = strtok(ssh_connection, " \t\n\r")) == NULL) {
    fprintf(stderr, "%s: failed to get first token in SSH_CONNECTION - \"%s\"\n", progname, ssh_conn);
    return;
  }

  if ((token = strtok(NULL, " \t\n\r")) == NULL) {
    fprintf(stderr, "%s: failed to get second token in SSH_CONNECTION - \"%s\"\n", progname, ssh_conn);
    return;
  }

  if ((token = strtok(NULL, " \t\n\r")) == NULL) {
    fprintf(stderr, "%s: failed to get third token in SSH_CONNECTION - \"%s\"\n", progname, ssh_conn);
    return;
  }

  if (strlen(token) > maxinboundiplength) {
    fprintf(stderr, "%s: inbound IP address is too long - \"%s\"\n", progname, token);
    return;
  }

  strncpy(inboundip, token, maxinboundiplength);

  return;
}

/*****************************************************************************/

/*
 *  get the size of a file
 */

int filesize(filename)
  char *filename;
{
  FILE *f;
  int   c;
  int   bytecount;

  bytecount = -1;

  if ((f = fopen(filename, "r")) != NULL) {
    bytecount = 0;

    while ((c = getc(f)) != EOF) {
      bytecount++;
    }

    fclose(f);
  }

  return(bytecount);
}

/*****************************************************************************/

/*
 *  send file content to terminal
 */

void showfile(filename)
  char *filename;
{
  FILE *f;
  char  line[MAX_LINE_LENGTH + sizeof(char)];

  if ((f = fopen(filename, "r")) != NULL) {
    while (fgets(line, MAX_LINE_LENGTH, f) != NULL) {
      writeterm(line);
    }

    fclose(f);
  }

  return;
}

/********************************************************************************/

void define_setting(inboundip, settingpair)
	char	*inboundip;
	char	*settingpair;
{
	char	*equals;
	char	*value;
	int	retcode;

	equals = strstr(settingpair, "=");

	if (equals == NULL) {
		writeterm("No equals sign (=) following the define keyword\n");
	} else {
		value = equals + 1;

		*equals = '\0';

		if (! definesetting(inboundip, settingpair, value)) {
			writeterm("The define command has failed\n");
		}
	}

	return;
}

/********************************************************************************/

void show_boot(inboundip)
	char	*inboundip;
{
	char	value[MAX_SETTINGS_LINE_LENGTH];

	if (! lookupsetting(inboundip, "BOOTMODE", value)) {
		strcpy(value, "");
	}

	writeterm("Boot mode: ");
	writeterm(value);
	writeterm("\n");

	if (! lookupsetting(inboundip, "BOOTHOST", value)) {
		strcpy(value, "");
	}

	writeterm("Boot host: ");
	writeterm(value);
	writeterm("\n");

	if (! lookupsetting(inboundip, "BOOTFILE", value)) {
		strcpy(value, "");
	}

	writeterm("Boot file: ");
	writeterm(value);
	writeterm("\n");

	return;
}

/********************************************************************************/

void show_startup_config(inboundip)
	char	*inboundip;
{
	char	configfilename[MAX_LINE_LENGTH];
	FILE	*configfile;
	char	line[MAX_LINE_LENGTH];

	strcpy(configfilename, "startup-config-");
	strcat(configfilename, inboundip);
	strcat(configfilename, ".txt");

	if ((configfile = fopen(configfilename, "r")) == NULL) {
		writeterm("No startup config loaded\n");
	} else {
		while (fgets(line, MAX_LINE_LENGTH - sizeof(char), configfile) != NULL) {
			writeterm(line);
		}

		fclose(configfile);
	}

	return;
}

/********************************************************************************/

void reboot(inboundip)
        char    *inboundip;
{
        char    configfilename[MAX_LINE_LENGTH];
        FILE    *configfile;
        char    line[MAX_LINE_LENGTH];

        strcpy(configfilename, "startup-config-");
        strcat(configfilename, inboundip);
        strcat(configfilename, ".txt");

	unlink(configfilename);

	system("/usr/bin/sleep 3");

	return;
}

/********************************************************************************/

/*
 *  main
 */

int main(argc, argv)
	int	argc;
 	char	*argv[];
{
	char	inboundip[MAX_LINE_LENGTH];
	struct	termios termoptions;
	char	line[MAX_LINE_LENGTH];
	int	lenline;
	char	devname[MAX_SETTINGS_LINE_LENGTH];

	progname = argv[0];

	getinboundip(inboundip, MAX_LINE_LENGTH);

	if (inboundip[0] == '\0') {
		fprintf(stderr, "%s: unable to determine inbound IPv4 address", progname);
		exit(2);
	}

	term = open("/dev/tty", O_RDWR | O_NOCTTY);

	if (term == -1) {
		fprintf(stderr, "%s: cannot open /dev/tty\n", progname);
		exit(2);
	}

	tcgetattr(term, &termoptions);
	termoptions.c_lflag |= (ICANON | ECHO | ECHOE);
	tcsetattr(term, TCSANOW, &termoptions);

	if (! lookupsetting(inboundip, "DEVNAME", devname)) {
		strcpy(devname, "generic");
	}

	writeterm("Logged into: ");
	writeterm(inboundip);
	writeterm("\n");

	while (TRUE) {
		writeterm(devname);
		writeterm("> ");

		lenline = read(term, line, MAX_LINE_LENGTH - sizeof(char));

		if (lenline < 0) {
			break;
		}

		line[lenline] = '\0';

		rstrip(line);

		if (line[0] == '\0') {
			continue;
		} else if (strncmp(line, "define ", 7) == 0) {
			define_setting(inboundip, line + 7);
		} else if (strcmp(line, "show boot") == 0) {
			show_boot(inboundip);
		} else if (strcmp(line, "show startup-config") == 0) {
			show_startup_config(inboundip);
		} else if (strcmp(line, "reboot") == 0) {
			reboot(inboundip);
			break;
		} else if (strcmp(line, "exit") == 0) {
			break;
		} else {
			writeterm("Badly formed command line - please try again\n");
		}
	}

	exit(0);
}

/* end of file */
