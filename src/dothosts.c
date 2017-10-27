/*
 * dothosts.c
 * Copyright (C) 2017 Andres J. Diaz <ajdiaz@ajdiaz.me>
 *
 * Distributed under terms of the MIT license.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>

#define _errexit(_s) {perror((_s)); exit(1);}


static char *
parse_host_line(char *hosts_line, const char *hname)
{
  const char *delim = " \t\n\r";
	char *r = NULL;
	char *ret = NULL;
	char *token = NULL;

  /* Replace comment by 0x00 */
	if ((r = index(hosts_line, '#')))
		*r = '\0';

	ret = strtok_r(hosts_line, delim, &r); /* ip addr */
	while ((token = strtok_r(NULL, delim, &r))) {
		if (!strcmp(hname, token))
			return ret;
  }
	return NULL;
}


static char *
lookup(const char *hname)
{
  FILE *fp = NULL;
  char *env_file = NULL;
  char *line = NULL;
  char *ret = NULL;
  size_t size;

  if (hname == NULL)
    return NULL;

  if ((env_file = getenv("DOTHOSTS")) == NULL)
  {
    /* No DOTHOSTS variable, composing from HOME */
    if ((env_file = getenv("HOME")) == NULL)
      env_file = strdup("/.hosts");
    else
    {
      if ((env_file = strdup(env_file)) == NULL)
        _errexit("Not enough memory");

      if ((env_file = (char *) realloc(env_file, strlen(env_file) + 8)) == NULL)
        _errexit("Not enough memory");

      (void) strcat(env_file, "/.hosts");
    }
  }
  else
    env_file = strdup(env_file);


  if ((fp = fopen(env_file, "r")) == NULL)
  {
    free(env_file);
    _errexit("File not found");
  }

	while (getline(&line, &size, fp) >= 0)
	{
	  if (line == NULL || *line == '\0')
	    continue;

		if ((ret = parse_host_line(line, hname)) != NULL)
			goto finish;
	}

finish:
	fclose(fp);
	free(env_file);

  if (ret != NULL)
    ret = strdup(ret);

  free(line); /* free after strdup */
	return ret;
}

/* Override library calls (note that some struct points are casting to
 * void pointers to avoid includes */

#ifndef USE_TEST
#define _lcall_override(_r, _f, _n, ...) {        \
  _r ret = (_r)0; char *n = NULL;                 \
  char *p = (char *)(_n);                         \
  _r (*orig)() = dlsym(RTLD_NEXT, #_f);           \
  if ((n = lookup(_n)) != NULL) p = n;            \
  ret = (_r) orig(p,  ## __VA_ARGS__);            \
  if (n != NULL) free(n);                         \
  return ret; }


int
getaddrinfo(const char *node, const char *service,
            const void *hints, void *res)
{
  _lcall_override(int, getaddrinfo, node, service, hints, res);
}

void *
gethostbyname(const char *name)
{
  _lcall_override(void *, gethostbyname, name);
}

void *
gethostbyname2(const char *name, int af)
{
  _lcall_override(void *, gethostbyname2, name, af);
}

int
gethostbyname_r(const char *name, void *ret, char *buf,
                size_t buflen, void *result, int *h_errnop)
{
  _lcall_override(int, gethostbyname_r, name, ret, buf,
                  buflen, result, h_errnop);
}

int
gethostbyname2_r(const char *name, int af,
                 void *ret, char *buf, size_t buflen,
                 void *result, int *h_errnop)
{
  _lcall_override(int, gethostbyname2_r, name, af, ret, buf, buflen,
                  result, h_errnop);
}
#endif
