/*muth: markup to html
 (c) 5qr1 WTFPL 2026 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define LENGTH(x) sizeof(x)/sizeof(x[0])
#define BUFSIZE 2048

typedef int (*Parser)(char *, char *);
static void eprint(int err, const char *fmt, ...);
static void *emalloc(void *in, size_t n);
static char *lfiletobuf(FILE *in);
static void process(char *st, char *en, Parser parser);
static int code(char *st, char *en);
static int underlines(char *st, char *en);
static int paragraphs(char *st, char *en);
static int inlinecode(char *st, char *en);
static int links(char *st, char *en);
static int replace(char *st, char *en);
Parser parsers[] = {code, underlines, paragraphs, inlinecode, links, replace};
char *fmts[] = {".jpg", ".jpeg", ".png", ".webp", ".gif"};

int
main(int argc, char **argv) {
	FILE *s = stdin;
	if(argc > 1) {
	if(!(strcmp(argv[1], "-v")))
		eprint(0, "muth v%s\n", VERSION);
	if(!(s = fopen(argv[1], "r")))
		eprint(EXIT_FAILURE, "bad file\n");
	}
	
	char *buf = lfiletobuf(s);
	process(buf, &buf[strlen(buf)], 0);

	fclose(s);
	free(buf);
	exit(0);
}

static void
eprint(int err, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(err);
}

static void
*emalloc(void *in, size_t n) {
	void *out;
	if(n <= 0)
		n = 1;
	if(in)
		out = realloc(in, n);
	else
		out = malloc(n);
	if(!out)
		eprint(EXIT_FAILURE, "emalloc() failed\n");

	return out;
}

static char
*lfiletobuf(FILE *in) {
	if(!in)
		return 0;

	char *buf = emalloc(NULL, BUFSIZE);
	buf[0] = '\n';

	size_t s, len = 0, bsize = 2 * BUFSIZE;
	while((s = fread(buf + len + 1, 1, BUFSIZE, in))) {
		len += s;
		if(BUFSIZE + len + 1 > bsize) {
			bsize += BUFSIZE;
			buf = emalloc(buf, bsize);
		}
	}
	strcpy(buf + len + 1, "\n\0");
	return buf;
}

static void
process(char *st, char *en, Parser parser) {
	if(!(st) || !(en))
		return;
	
	for(char *p = st; p < en; p++) {
		int ch = 0;
		if(!parser)
			for(size_t i = 0; i < LENGTH(parsers) && !ch; i++)
				ch = parsers[i](p, en);
		else
			ch = parser(p, en);
		if(ch)
			p += ch - 1;
		else
			putc(p[0], stdout);
	}
}

static int
code(char *st, char *en) {
	if(!(st) || st[0] != '\n' || st[1] != '`')
		return 0;
	
	char *p = st + 2;
	for(;p < en && p[0] != '`'; p++);
	if(p >= en)
		return 0;

	printf("<pre><code>");
	process(st + 2, p, replace);
	printf("</code></pre>");
	return (p - st) + 1;
}

static int
underlines(char *st, char *en) {
	if(!(st) || st[0] != '\n')
		return 0;
	
	char *p = st + 1;
	for(;p < en && p[0] != '\n'; p++);
	if(p >= en)
		return 0;

	p++;
	char c = p[0];
	if(c != '=' && c != '-') 
		return 0;
	int l = p - (st + 1);

	for(; p < en && p[0] == c; p++);
	if(p[0] != '\n')
		return 0;

	if(p - (st + l) != l)
		return 0;

	if(c == '=') {
		printf("<h1>");
		process(st + 1, st + l, 0);
		printf("</h1>\n");
	} else if(c == '-') {
		printf("<h2>");
		process(st + 1, st + l, 0);
		printf("</h2>\n");
	}

	if((en - p) > 2 && p[1] == '\n')
		return (p - st) + 1;
	else
		return(p - st);
}

static int
paragraphs(char *st, char *en) {
	if(!(st) || st[0] != '\n' || st[1] == '\n')
		return 0;
	
	char *p = st + 1;
	for(;p < en && p[0] != '\n'; p++);
	if(p[0] != '\n')
		return 0;
	
	printf("<p>");
	process(st + 1, p, 0);
	printf("</p>\n");
	return (p - st);
}

static int
inlinecode(char *st, char *en) {
	if(!(st) || st[0] != '`')
		return 0;
	
	char *p = st + 1;
	for(;p < en && p[0] != '`'; p++);
	if(p >= en)
		return 0;

	printf("<code>");
	process(st + 1, p, replace);
	printf("</code>");
	return (p - st) + 1;
}

static int
links(char *st, char *en) {
	if(!(st) || st[0] != '<')
		return 0;
	
	char *p = st + 1;
	for(;p < en && p[0] != '>'; p++);
	if(p >= en) 
		return 0;
	
	char *buf = emalloc(NULL, (p - st)), *h;
	memcpy(buf, st + 1, (p - st - 1));
	buf[p - st - 1] = '\0';

	strtok(buf, "|");
	h = strtok(NULL, "|");
	
	char *img = 0;
	for(size_t i = 0; i < LENGTH(fmts) && !img; i++)
		img = strstr(buf, fmts[i]);

	if(img) {
		if(h)
			printf("<a href=\"%s\"><img src=\"%s\"></a>", h, buf);
		else
			printf("<img src=\"%s\">", buf);
	} else {
		if(h)
			printf("<a href=\"%s\">%s</a>", buf, h);
		else	
			printf("<a href=\"%s\">%s</a>", buf, buf);
	}

	free(buf);
	return (p - st) + 1;
}

static int
replace(char *st, char *en) {
	if(!(st))
		return 0;
	switch(st[0]) {
		case '&':
			printf("&amp;");
			return 1;
		case '<':
			printf("&lt;");
			return 1;
		case '>':
			printf("&gt;");
			return 1;
		case '\"':
			printf("&quot;");
			return 1;
		case '\'':
			printf("&#39;");
			return 1;
		default:
			return 0;
	}
}
