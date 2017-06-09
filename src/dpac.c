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
#include "ip.h"
#include "alog.h"

/*
 * Globals
 */
char *g_app_basename;		/* invocation name */

static char *
app_init(const char *app_basename);

static int
parse_conf(char *conf_file, const char *client_ip);

static int
output_pac_file(const char *fname);

/*
 * main
 */
int
main(int argc, char *argv[])
{

    char *conf_file;		/* conf file contents */
    char *rem_addr;			/* client's ip address */

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
        alog(LL_NORM, "unable to get client's address from environment variable \"REMOTE_ADDR\"");
        exit(EXIT_FAILURE);
    }

    if (parse_conf(conf_file, rem_addr) != 0)
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
        alog(LL_NORM, "unable to open conf file \"%s\" (%s)", conf_fname, strerror(errno));
        return NULL;
    }
    /* get file size for alloc and read entire file in */
    if (fstat(fileno(fs), &sbuf) != 0) {
        alog(LL_NORM, "fstat failed for conf file \"%s\" (%s)", conf_fname, strerror(errno));
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
 * parse conf (networks) file
 *
 * XXX - Note this function overwrites/modifies the passed-in buffer.
 *       It also handles "\r\n" text files (DOS/Windows).
 *
 * File format:
 * 
 *		- lines should be of the form:
 *		  <network> <pac filename>
 *		  the network and filename delimiter may be one or more spaces or tabs
 * 		- comment lines and empty lines are ignored; comment lines must
 * 		  start with "#" as first character in line (column 1)
 * 		- example line:
 * 		  10.4.5.0/24 proxy1.js
 *
 * Returns:
 *      0: on success
 *     >0: if error
 */
static int
parse_conf(char *conf_file, const char *client_ip)
{

    int linenum = 0;
    char *bol;  /* begin of current line */
    char *eol;  /* end of current line */
    char *nxl;  /* pointer to start of next line (or EOF) */
    char *pn;   /* intra-line ptr to network */
    char *pc;   /* intra-line ptr to cidr */
    char *pf;   /* intra-line ptr to filename */
    char *tmp;

    struct in_addr inaddr_clientip;
    struct in_addr inaddr_network;
    int cidr;

    if (conf_file == NULL)
        return 2;

    if (inet_aton(client_ip, &inaddr_clientip) != 1) {
        alog(LL_NORM, "inet_aton failed for client ip \"%s\"", client_ip);
        return 3;
    }

    /* set initial line prior to loop */
    bol = conf_file;

    /* parse each line */
    while (*bol != '\0') {

        linenum++;

        /* find end of line */
        if ((eol = strchr(bol, '\n')) != NULL) {
            *eol = '\0';
            if (*(eol - 1) == '\r')
                *(eol - 1) = '\0';
            nxl = eol + 1;
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
         * setup intra-line pointers to network and filename
         */

        /* network */
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
            alog(LL_NORM, "syntax error: no delimiter found (line %d)", linenum);
            goto next_loop;
        }

        /*
         * check for special case network "*" (match all / default)
         */
        if (*pn == '*') {
            if (output_pac_file(pf) != 0)
                return 4;
            else
                return 0;
        }

        /* terminate cidr */
        tmp = pf - 1;
        while (*tmp == ' ' || *tmp == '\t')
            tmp--;
        *(tmp + 1) = '\0';

        /* terminate network */
        while (*tmp != '/')
            tmp--;
        *tmp = '\0';
        pc = tmp + 1;

        /*
         * do it
         */

        if (inet_aton(pn, &inaddr_network) != 1) {
            alog(LL_NORM, "inet_aton failed for network \"%s\" (line %d)", pn, linenum);
            goto next_loop;
        }

        if ((cidr = atoi(pc)) == 0) {
            alog(LL_NORM, "cidr conversion failed for network \"%s\", cidr \"%s\" (line %d)", pn, pc, linenum);
            goto next_loop;
        }

        /* check address */
        if (cidr4_match(&inaddr_clientip, &inaddr_network, cidr)) {
            /*
             * found a match; output pac file
             */
            if (output_pac_file(pf) != 0)
                return 5;
            else
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
        alog(LL_NORM, "unable to open pac file \"%s\" (%s)", fname, strerror(errno));
        return 1;
    }
    /* get file size for alloc and read entire file in */
    if (fstat(fileno(fs), &sbuf) != 0) {
        alog(LL_NORM, "fstat failed for pac file \"%s\" (%s)", fname, strerror(errno));
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
    fprintf(stdout, "%s", pac_file);

    return 0;
}
