/*muc: tiny markup subset
  (c) 5qr1 WTFPL 2026- */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "muc.h"

static void
eprint(const char *format, int err, ...) {
	va_list ap;

	va_start(ap, err);
	vfprintf(stderr, format, ap);
	va_end(ap);

	exit(err);
}

static void 
*emalloc(int n) {
	void *x = malloc(n);
	if(!x)
		eprint("malloc failed\n", EXIT_FAILURE);
	return x;
}

static int
paragraphs(char *section, char *end) {
	char *p = section;

	while(p < end) {
		if(p[0] == '\n' && p[1] == '\n')
			break;
		p++;
	}
	
	if(!p || p >= end) 
		return 0;

	hprint("<p>");
	process(section, p, 0);
	hprint("</p>\n");
	
	return (p - section) + 1;
} 

static int
underlines(char *section, char *end) {
	if(section[0] != '\n')
		return 0;
	if(section[1] == '\n')
		return 0;
	
	int tog=0, i=0;
	char buf[256] = "",  *p = section + 1;

	while(p < end) {
		if(p[0] == '\n') {
			tog = 1;
			p++;
			buf[i] = '\0';
			continue;
		}
		if(!tog) {
			buf[i] = p[0];
			p++;
			i++;
			continue;
		}

		if(p[0] != '=' && p[0] != '-')
			return 0;	
		

		while(p < end) {
			p++;
			if(p[0] != '\n') 
				continue;

			if(section[p - 1 - section] == '=') {
				hprint("<h1>%s</h1>\n", buf);
				return (p - section) + 1;
			} else {
				hprint("<h2>%s</h2>\n", buf);
				return (p - section) + 1;
			}
		}
	}
	return 0;
}

static int
links(char *section, char *end) {
	if(section[0] != '<')
		return 0;

	int i = 0, ia = 0, ib = 0;
	char bufa[256] = "", bufb[256] = "", *p = section + 1;

	while(p < end) {
		if(p[0] == '\n')
			return 1;
		if(p[0] == '|') {
			i = 1;
			p++;
			continue;
		}
		if(p[0] == '>')
			break;

		if(i) {
			bufb[ib] = p[0];
			ib++;
		} else {
			bufa[ia] = p[0];
			ia++;
		}

		p++;
	}
	
	bufa[ia] = '\0';
	bufb[ib] = '\0';

	for(i = 0; ((unsigned long)i) < LENGTH(fmts); i++) { 
		if(strstr(bufa, fmts[i])) {
			if(ib)
				hprint("<a href=\"%s\"><img src=\"%s\"</a>", bufb, bufa);
			else
				hprint("<img src=\"%s\">", bufa);
			return (p - section) + 1;
		}
	}

	hprint("<a href=\"%s\">%s</a>", bufa, bufb);
	return (p - section) + 1;
}

static void
hprint(const char *format, ...) {
	int len;
	char *newbuf;
	va_list ap, apc;
	size_t u, n, s;

	va_start(ap, format);
	va_copy(apc, ap);

	len = vsnprintf(NULL, 0, format, ap);
	va_end(ap);

	u = strlen(hbuf);
	n = u + len + 1;

	if(n > ((size_t)hbufs)) {
		if(hbufs)
			s = hbufs;
		else 
			s = BUFFERSIZE;
		while (s < n)
			s *= 2;

		if(!(newbuf = realloc(hbuf, s)))
			eprint("malloc failed\n", EXIT_FAILURE);

		hbuf = newbuf;
		hbufs = s;
	}

	vsnprintf(hbuf + u, hbufs - u, format, apc);
	va_end(apc);
}

char
*mkbuf(FILE *in) {
	int len=0, bsize = 2 * BUFFERSIZE;
	size_t s;
	char *buffer = emalloc(BUFFERSIZE);
	buffer[0] = '\n';

	while((s = fread(buffer + 1 + len, 1, BUFFERSIZE, in))) {
		len += s;
		if(BUFFERSIZE + len + 3 > bsize) {
			bsize += BUFFERSIZE;
			if(!(buffer = realloc(buffer, bsize))) 
				eprint("malloc failed\n", EXIT_FAILURE);
		}
	}
	
	strcpy(buffer + len + 1, "\n\0");

	return buffer;
}

static void
process(char *section, char *end, Parser pparser) {
	int pc=0, i=0, pi=0, len = end - section;
	while(i < len) {
		pc = 0;
		if(pparser)
			pc = pparser(section + i, end);
		else
			for(pi = 0; ((size_t)pi) < LENGTH(parsers) && !pc; pi++)
				pc = parsers[pi](section + i, end);
		if(pc)
			i += pc;
		else {
			hprint("%c", section[i]);
			i++;
		}
	}
}

extern char
*muc_htmlmd(char *in) {
	hbuf = emalloc(BUFFERSIZE);
	hbufs = BUFFERSIZE;

	process(in, &in[strlen(in)], NULL);
	return hbuf;
}

int
main(int argc, char **argv) {
	FILE *source = stdin;
	if(argv[1]) {
		if(argv[1][1] != 'v') {
			if(!(source = fopen(argv[1], "r")))
				eprint("bad file\n", EXIT_FAILURE);
		} else
			eprint("%s\n", 0, VERSION);
	}

	char *buffer = mkbuf(source);
	hbuf = emalloc(BUFFERSIZE);
	hbufs = BUFFERSIZE;
	process(buffer, &buffer[strlen(buffer)], 0);

	printf("%s", hbuf);

	fclose(source);
	free(buffer);
}
