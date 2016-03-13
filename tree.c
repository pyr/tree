/* $Id: tree.c,v 1.6 2006/06/21 18:47:54 panda Exp $ */
/*
 * Copyright (c) 2006 Pierre-Yves Ritschard <pyr@spootnik.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/stat.h>
#include <sys/time.h>

#include <err.h>
#include <fts.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int 		 aflag = 0;
static int 		 dflag = 0;
static int 		 fflag = 0;
static int 		 Fflag = 0;
static int 		 iflag = 0;
static int		 rflag = 0;
static int		 sflag = 0;
static int		 tflag = 0;
static int		 maxdepth = 0;

int		 	 has_child(FTSENT *);
int			 sort_entries(const FTSENT **, const FTSENT **);
char 			*format_entry(FTSENT *);
void		 	 walk(char *, int);

extern char		*__progname;


int
has_child(FTSENT *entry)
{
	FTSENT *e;

	if (aflag == 1) {
		if (dflag == 1) {
			for (e = entry->fts_link; e != NULL; e = e->fts_link)
				if (e->fts_info == FTS_D)
					return (1);
			return (0);
		}
		return (entry->fts_link != NULL);
	}

	for (e = entry->fts_link; e != NULL; e = e->fts_link) {
		if (e->fts_name[0] != '.') {
			if (dflag == 0)
				return (1);
			if (e->fts_info == FTS_D)
				return (1);
		}
	}
	return (0);
}

int
sort_entries(const FTSENT **e1, const FTSENT **e2)
{
	struct timeval	 t1;
	struct timeval	 t2;

	if (tflag == 1) {
		TIMESPEC_TO_TIMEVAL(&t1, &((*e1)->fts_statp->st_mtimespec));
		TIMESPEC_TO_TIMEVAL(&t2, &((*e2)->fts_statp->st_mtimespec));
		if (timercmp(&t1, &t2, ==))
			return (0);
		if (rflag == 0) {
			if (timercmp(&t1, &t2, >))
				return (-1);
			else
				return (1);
		} else {
			if (timercmp(&t1, &t2, <))
				return (-1);
			else
				return (1);
		}
	}

	if (rflag == 1)
		return (strcmp((*e2)->fts_name, (*e1)->fts_name));
	return (strcmp((*e1)->fts_name, (*e2)->fts_name));
}

char *
format_entry(FTSENT *entry)
{
	char		 buf[PATH_MAX + 1];
	char		*str;
	size_t		 sz;

	if (fflag == 0)
		str = strdup(entry->fts_name);
	else 
		str = strdup(entry->fts_path);

	if (str == NULL)
		err(1, "memory exhausted");

	sz = strlen(str);
	if (Fflag == 1) {
		sz += 2;
		if ((str = realloc(str, sz)) == NULL)
			err(1, "memory exhausted");

		switch (entry->fts_statp->st_mode & S_IFMT) {
			case S_IFDIR:
				(void)strlcat(str, "/", sz);
				break;
			case S_IFIFO:
				(void)strlcat(str, "|", sz);
				break;
			case S_IFLNK:
				(void)strlcat(str, "@", sz);
				break;
			case S_IFSOCK:
				(void)strlcat(str, "=", sz);
				break;
		}
		if (entry->fts_statp->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			(void)strlcat(str, "*", sz);
	}

	if (entry->fts_info == FTS_SL) {
		bzero(buf, PATH_MAX + 1);
		if (readlink(entry->fts_accpath, buf, PATH_MAX) == -1)
			err(1, "readlink: %s", entry->fts_path);
		sz += strlen(buf) + 5;
		if ((str = realloc(str, sz)) == NULL)
			err(1, "memory exhausted");
		(void)strlcat(str, " -> ", sz);
		(void)strlcat(str, buf, sz);
	}
	return (str);
}

void
walk(char *path, int ftsoptions)
{
	int		 depth = 0;
	int		 i;
	size_t		 padlen;
	int		*show_bar;
	FTS		*tree;
	FTSENT		*entry;
	char		**paths;
	char		*pad;
	char		*ename;
	int		 nfiles = 0;
	int		 ndir = 0;

	paths = calloc(2, sizeof(char *));
	paths[0] = path;
	paths[1] = NULL;

	if ((show_bar = calloc(1, sizeof(int))) == NULL)
		err(1, "memory exhausted");

	if ((tree = fts_open(paths, ftsoptions, sort_entries)) == NULL)
		err(1, "fts_open");

	entry = fts_read(tree);
	if (entry == NULL || entry->fts_info != FTS_D)
		errx(1, "invalid root node: %s", path);

	(void)printf("%s\n", path);
	for (entry = fts_read(tree); entry != NULL; entry = fts_read(tree)) {

		if (entry->fts_info == FTS_ERR || entry->fts_info == FTS_NS)
			errx(1, "fts_read: %s", strerror(entry->fts_errno));
		
		padlen =  (depth * 4) + 1;
		switch (entry->fts_info) {
			case FTS_D:
				if (aflag == 0 && entry->fts_name[0] == '.') {
					(void)fts_set(tree, entry, FTS_SKIP);
					continue;
				}
				show_bar[depth] = has_child(entry);
				depth++;
				show_bar = realloc(show_bar,
						   (depth + 1) * sizeof(int));
				if (show_bar == NULL)
					err(1, "memory exhausted");
				show_bar[depth] = 1;
				if (maxdepth > 0 && depth > maxdepth) {
					(void)fts_set(tree, entry, FTS_SKIP);
					continue;
				}
				ndir++;
				break;
			case FTS_DNR: /* depth goes down anyway */
			case FTS_DP:
				if (aflag == 0 && entry->fts_name[0] == '.')
					continue;
				if (depth > 0)
					depth--;
				show_bar = realloc(show_bar,
						  (depth + 1) * sizeof(int));
				if (show_bar == NULL)
					err(1, "memory exhausted");
				continue;
			default:
				if (dflag == 1)
					continue;
				if (aflag == 0 && entry->fts_name[0] == '.')
					continue;
				if (maxdepth > 0 && depth >= maxdepth)
					continue;
				nfiles++;
				break;
		}

		/* construct line */
		if (iflag == 0) {
			if ((pad = calloc(1, padlen)) == NULL)
				err(1, "memory exhausted");

			for (i = 0; i < depth; i++) {
				if (show_bar[i] == 1)
					(void)strlcat(pad, "|   ", padlen);
				else
					(void)strlcat(pad, "    ", padlen);
			}
			(void)printf("%s%c-- ", pad, has_child(entry)?'|':'`');
			free(pad);
		}

		ename = format_entry(entry);
		(void)printf("%s\n", ename);
		free(ename);
	}
	if (sflag == 1)
		return;
	(void)printf("\n%d director%s", ndir, (ndir > 1) ? "ies" : "y");
	if (dflag == 0)
		(void)printf(", %d file%s", nfiles, (nfiles > 1) ? "s" : "");
	(void)printf("\n");
}


int
main(int argc, char *argv[])
{
	int 		 c;
	int		 i;
	int		 ftsoptions;
	const char	*estr;

	ftsoptions = FTS_PHYSICAL;

	while ((c = getopt(argc, argv, "adfFilL:srtx")) != -1) {
		switch (c) {
			case 'a':
				aflag = 1;
				break;
			case 'd':
				dflag = 1;
				break;
			case 'f':
				fflag = 1;
				break;
			case 'F':
				Fflag = 1;
				break;
			case 'i':
				/*
				 * this is stupid, shouldn't you be
				 * using find ?
				 */
				iflag = 1;
				break;
			case 'l':
				ftsoptions &= ~FTS_PHYSICAL;
				ftsoptions |= FTS_LOGICAL;
				break;
			case 'L':
				estr = NULL;
				maxdepth = (int)strtonum(optarg, 1,
						         INT_MAX, &estr);
				if (estr != NULL) {
					errx(1, "invalid depth %s: %s", optarg,
					     estr);
				}
				break;
			case 'r':
				rflag = 1;
				break;
			case 's':
				sflag = 1;
				break;
			case 't':
				tflag = 1;
				break;
			case 'x':
				ftsoptions |= FTS_XDEV;
				break;
			default:
				(void)fprintf(stderr,
					      "usage: %s [-adfFilsrtx] [-L depth] [path ...]\n",
					      __progname);
				return 1;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0)
		walk(".", ftsoptions);
	for (i = 0; i < argc; i++)
		walk(argv[i], ftsoptions);

	return (0);
}
