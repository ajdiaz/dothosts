/*
 * test.c
 * Copyright (C) 2017 Andres J. Diaz <ajdiaz@ajdiaz.me>
 *
 * Distributed under terms of the MIT license.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include "dothosts.c"


/* Test main code */

#define assert_str(_x, _y) { \
  assert(_x != NULL); \
  assert(_y != NULL); \
  assert(strcmp(_x, _y) == 0); }


static char *
parse_host_line_test(char *hosts_line, const char *hname)
{
	static char buf[128];
	strncpy(buf, hosts_line, sizeof(buf)-1);
	return parse_host_line(buf, hname);
}


static char *
lookup_test(const char *hname)
{
	static char buf[128];
	char *p = lookup(hname);

	if (p != NULL) {
		strncpy(buf, p, sizeof(buf)-1);
		free(p);
		p = buf;
	}
	return p;
}


static void
test_hosts_lines()
{

	assert(parse_host_line_test("", "sometest") == NULL);
	assert(parse_host_line_test("", "") == NULL);

	assert(parse_host_line_test("#sometestsometest sometest sometest sometest",
	                            "sometest") == NULL);

	assert(parse_host_line_test("#abbb bbb bb #aaabbb bbb, bbb", "bbb") == NULL);

	assert(parse_host_line_test("10.0.0.1 sometest # github.com",
	                            "github.com") == NULL);

	assert(parse_host_line_test("10.0.0.1 sometest#github.com",
	                            "github.com") == NULL);

	assert_str(parse_host_line_test("10.0.0.1 sometest#github.com", "sometest"),
	           "10.0.0.1");

	assert_str(parse_host_line_test("10.0.0.1 github.com", "github.com"),
	           "10.0.0.1");

	assert_str(parse_host_line_test("10.0.0.1 github.com sometest", "sometest"),
	           "10.0.0.1");
}


static void
test_hosts_file()
{
  FILE *hosts = NULL;

	char test_case[] = (
    "10.0.0.1 sometest\n"
    "10.0.0.2 sometest2 sometest3 sometest4\n"
    "10.0.0.3 sometesttest # sometest\n"
    "10.0.0.4 \x00 some_evilness \n"
    "10.0.0.5 shouldwork\n" "\n" "\n" "\n"
    "10.0.0.6 thistoo#end\n"
	);

  /* save host file */
	assert(setenv("HOME", "/tmp", 1) == 0);
	assert(chdir("/tmp") == 0);
	hosts = fopen(".hosts", "w");
	assert(hosts != NULL);
	assert(fwrite(test_case, 1, sizeof(test_case), hosts) == sizeof(test_case));
	fclose(hosts);

  /* test host file */
	assert(lookup_test("xyz") == NULL);
	assert(lookup_test("some_evilness") == NULL);
	assert(lookup_test(NULL) == NULL);

  assert_str(lookup_test("sometest2"), "10.0.0.2");
	assert_str(lookup_test("sometest3"), "10.0.0.2");
	assert_str(lookup_test("sometest4"), "10.0.0.2");
  assert_str(lookup_test("sometesttest"), "10.0.0.3");
	assert_str(lookup_test("shouldwork"), "10.0.0.5");
	assert_str(lookup_test("thistoo"), "10.0.0.6");

	unlink("/tmp/.hosts");
}


int
main()
{
	test_hosts_lines();
	test_hosts_file();
	return 0;
}
