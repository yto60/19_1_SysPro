#include <dlfcn.h>	/* dladdr */
#include <fcntl.h>	/* open */
#include <stdarg.h> /* va_list */
#include <stdio.h>	/* printf */
#include <stdlib.h> /* atexit, getenv */
#include <string.h>
#include <unistd.h> /* write */

#define MAX_DEPTH 32
#define MAX_CALLS 1024
#define OUTFILE "cg.dot"

FILE *fout;

void print_last_line() {
	fprintf(fout, "}\n");
}

__attribute__((no_instrument_function)) int
log_to_stderr(const char *file, int line, const char *func, const char *format,
							...) {
	char message[4096];
	va_list va;
	va_start(va, format);
	vsprintf(message, format, va);
	va_end(va);
	return fprintf(stderr, "%s:%d(%s): %s\n", file, line, func, message);
}
#define LOG(...) log_to_stderr(__FILE__, __LINE__, __func__, __VA_ARGS__)

__attribute__((no_instrument_function)) const char *addr2name(void *address) {
	Dl_info dli;
	if (dladdr(address, &dli) != 0) {
		return dli.dli_sname;
	} else {
		return NULL;
	}
}

__attribute__((no_instrument_function)) void
__cyg_profile_func_enter(void *addr, void *call_site) {
	/* Not Yet Implemented */
	LOG("IN addr: %s call_site: %s", addr2name(addr), addr2name(call_site));
	if (strcmp(addr2name(addr), "main") == 0) {
		fout = fopen(OUTFILE, "w");
		fprintf(fout, "strict digraph G {\n");
		atexit(print_last_line);
	} else if (strcmp(addr2name(addr), "print_last_line") != 0) {
		fprintf(fout, "\t%s -> %s;\n", addr2name(call_site), addr2name(addr));
	}
}

__attribute__((no_instrument_function)) void
__cyg_profile_func_exit(void *addr, void *call_site) {
	/* Not Yet Implemented */
	LOG("OUT addr: %s call_site: %s", addr2name(addr), addr2name(call_site));
}