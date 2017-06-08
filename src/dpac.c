/*
 * dpac.c
 *
 * Dynamic proxy auto configuration CGI application.
 *
 * Returns Javascript proxy configuration dynamically
 * based on client's IP address.
 *
 * Client's IP address is in environment variable:
 * REMOTE_ADDR
 *
 */

#include "app.h"

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/* network types */
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#include "inet_aton.h"
#endif

#include "dpac.h"
#include "ipcheck.h"
#include "alog.h"

/*
 * Globals
 */
char *g_app_basename;		/* invocation name */

static char *
app_init(const char *app_basename);

static int
parse_client_conf(char *conf_file, const char *client_ip);

static int
output_pac_file(const char *fname);

static int
ip_in_acl(const char *client_ip, CIDRNetwork *cidr_net);

/*
 * main
 */
int
main(int argc, char *argv[])
{

char *conf_file;			/* conf file contents */
char *rem_addr;				/* client's ip address */

	/* get app basename from invocation name */
	if ((g_app_basename = strrchr(argv[0], PATH_SEP)) != NULL)
		g_app_basename++;
	else
		g_app_basename = argv[0];

	/* do app initialization */
	if ((conf_file = app_init(g_app_basename)) == NULL) {
		exit(EXIT_FAILURE);
	}

	/* get client ip address from environment */
	rem_addr = getenv("REMOTE_ADDR");
	if (rem_addr == NULL) {
		alog(LL_NORM, "unable to get client's remote address via environment");
		exit(EXIT_FAILURE);
	}

	if (parse_client_conf(conf_file, rem_addr) != 0)
		exit(EXIT_FAILURE);

return 0;
}

/*
 * app startup initialization
 *
 * Returns:
 * 		- pointer to conf file contents if successful
 * 		- NULL if error
 */
static char *
app_init(const char *app_basename)
{

char *conf_file;
char conf_fname[1024] = { '\0' };
FILE *fs;
struct stat sbuf;

	snprintf(conf_fname, 1023, "%s.conf", app_basename);

	/* read conf file into buffer */
	if ((fs = fopen(conf_fname, "r")) == NULL) {
		alog(LL_NORM, "unable to open conf file \"%s\" (%s)", conf_fname, 
			strerror(errno));
		return NULL;
	}
	/* get file size for alloc and read entire file in */
	if (fstat(fileno(fs), &sbuf) != 0) {
		alog(LL_NORM, "fstat failed for conf file \"%s\" (%s)", conf_fname, 
			strerror(errno));
		return NULL;
	}
	conf_file = malloc(sbuf.st_size + 1);
	if (conf_file == NULL) {
		alog(LL_NORM, "malloc failed for conf file \"%s\"", conf_fname);
		return NULL;
	}
	*(conf_file + sbuf.st_size) = '\0';
	fread(conf_file, sbuf.st_size, 1, fs);
	/* check for successful read */
	if (ferror(fs) != 0) {
		alog(LL_NORM, "error reading conf file \"%s\"", conf_fname);
		return NULL;
	}
	fclose(fs);

return conf_file;
}

/*
 * parse client conf (networks) file
 *
 * XXX - Note this function overwrites/modifies the passed-in buffer.
 *       It also handles "\r\n" text files (DOS/Windows).
 *
 * File format:
 * 
 *		o lines should be of the basic form:
 *		      <network list> <pac filename>
 *		  the list and filename delimiter may be one or more spaces or tabs
 * 		o comment lines and empty lines are ignored; comment lines must 
 * 		  start with "#" as first character in line (column 1)
 * 		o networks may be semi-colon delimited on one line:
 * 		      10.2.3.0/24;10.3.4.0/24 proxy.pac
 * 		  or simply one per line:
 * 		      10.4.5.0/24 proxy.pac
 *
 * Return values:
 *      0:  on success
 *     >0:  if error
 */
static int
parse_client_conf(char *conf_file, const char *client_ip)
{

int     linenum = 0;
char	*bol;	/* begin of current line */
char	*eol;	/* end   of current line */
char	*nxl;	/* pointer to start of next line (or EOF) */
char    *pn;	/* intra-line ptr to net list */
char    *pf;	/* intra-line ptr to filename */
char	sav;	/* save 1st char of filename */
char	*tmp;

CIDRNetwork *cidr_net;		/* parsed CIDR network list */

	if (conf_file == NULL)
		return 2;

	/* set initial line prior to loop */
	bol = conf_file;

    /* parse each line */
	while (*bol != '\0') {

		linenum++;

		/* find end of line */
		if ((eol = strchr(bol, '\n')) != NULL) {
			*eol = '\0';
			if (*(eol-1) == '\r')
				*(eol-1) = '\0';
			nxl = eol+1;
		} else {
			nxl = bol + strlen(bol);
		}

		/* get rid of start-of-line whitespace */
		while (*bol == ' ' || *bol == '\t')
			bol++;

		/* skip empty line */
        if (*bol == '\0')
			goto next_loop;

		/* skip comment line */
        if (*bol == '#')
			goto next_loop;

        /*
		 * setup intra-line pointers to net list and filename
		 */

		/* net list */
		pn = bol;
		/* filename, find 1st space or tab delimiter */
		pf = bol;
		while (*pf != '\0' && *pf != ' ' && *pf != '\t') {
			pf++;
		}
		/* skip multiple delimiters */
		while (*pf == ' ' || *pf == '\t')
			pf++;
		/* syntax error: no delimiter found */
		if (*pf == '\0') {
			alog(LL_NORM, "syntax error: no delimiter found (line %d)", 
				linenum);
			goto next_loop;
		}
		
		/*
		 * check for special case network "*" (match all / default)
		 */
		if (*pn == '*') {
			if (output_pac_file(pf) != 0)
				return 3;
			else
				return 0;
		}

		/* save 1st char in filename since we probably overwrite with 
		 * null terminator if there's only one delimiter */
		sav = *pf;
		*pf = '\0';
		/* make sure net list has trailing semi-colon;
		   could have multiple spaces or tabs as delimiters */
		tmp = pf - 1;
		while (*(tmp-1) == ' ' || *(tmp-1) == '\t')
			tmp--;
		if (*(tmp-1) != ';') {
			*tmp = ';';
			/* terminate after ';' (may already be there) */
			*(tmp+1) = '\0';
		}

		/*
		 * do it
		 */

		if ((cidr_net = parseRoutes(pn)) == NULL) {
			alog(LL_NORM, "unable to parse client network list \"%s\" "
				"(line %d)", pn, linenum);
			goto next_loop;
		}
		/* check address */
		if (ip_in_acl(client_ip, cidr_net)) {
			/*
			 * found a match; output pac file
			 */
			/* restore 1st char to filename */
			*pf = sav;
			if (output_pac_file(pf) != 0)
				return 4;
			/* done */
			return 0;
		}

		/*
		 * setup next loop iteration
		 */
next_loop:
		bol = nxl;
		continue;
    }

return 1;
}

static int
output_pac_file(const char *fname)
{

char *pac_file;
FILE *fs;
struct stat sbuf;

	/* get pac file contents */
	if ((fs = fopen(fname, "r")) == NULL) {
		alog(LL_NORM, "unable to open pac file \"%s\" (%s)", fname,
			strerror(errno));
		return 1;
	}
	/* get file size for alloc and read entire file in */
	if (fstat(fileno(fs), &sbuf) != 0) {
		alog(LL_NORM, "fstat failed for pac file \"%s\" (%s)", fname,
			strerror(errno));
		return 2;
	}
	pac_file = malloc(sbuf.st_size + 1);
	if (pac_file == NULL) {
		alog(LL_NORM, "malloc failed for pac file \"%s\"", fname);
		return 3;
	}
	*(pac_file + sbuf.st_size) = '\0';
	fread(pac_file, sbuf.st_size, 1, fs);
	/* check for successful read */
	if (ferror(fs) != 0) {
		alog(LL_NORM, "error reading pac file \"%s\"", fname);
		return 4;
	}
	fclose(fs);

	fprintf(stdout, "Content-Length: %llu\n", sbuf.st_size);
	fprintf(stdout, "Content-Type: application/x-ns-proxy-autoconfig\n\n");
	fprintf(stdout, pac_file);

return 0;
}

/*
 * check if ip is in acl
 *
 * Returns:
 *	1: if allowed
 *	0: if not allowed or error
 */
static int
ip_in_acl(const char *client_ip, CIDRNetwork *cidr_net)
{

IPaddr ipa;
struct in_addr inaddr_ip;			/* in_addr for IPaddr type */

	/* create IPaddr type from client ip */
    if (inet_aton(client_ip, &inaddr_ip) != 1) {
        alog(LL_NORM, "inet_aton failed for client ip \"%s\"", client_ip);
	    return 0;
    }
    ipa.addr = inaddr_ip.s_addr;

    if (checkIPListed(cidr_net, ipa) == YES)
		return 1;
	else
		return 0;
}
