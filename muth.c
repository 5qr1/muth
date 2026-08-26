/*muth: markup to html
 (c) 5qr1 WTFPL 2026 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define LENGTH(x) sizeof(x)/sizeof(x[0])

typedef int (*Parser)(char **, char *, char *);
static void eprint(int err, const char *fmt, ...);
static void *emalloc(void *in, size_t n);
static char *apsprintf(char *in, const char *fmt, ...);
static char *filetobuf(FILE *in);
static char *process(char **ret, char *st, char *en);
static int underlines(char **ret, char *st, char *en);
static int paragraphs(char **ret, char *st, char *en);
static int links(char **ret, char *st, char *en);
static int replace(char **ret, char *st, char *en);
Parser parsers[] = {underlines, paragraphs, links, replace};
char *fmts[] = {".jpg", ".jpeg", ".png", ".webp", ".gif"};

int
main(int argc, char **argv) {
	FILE *s;
	if(argc < 2)
		eprint(0, "muth [file]\n");
	if(!(strcmp(argv[1], "-v")))
		eprint(0, "muth v%s\n", VERSION);
	if(!(s = fopen(argv[1], "r")))
		eprint(EXIT_FAILURE, "bad file\n");
	
	char *buf = filetobuf(s), *html = emalloc(NULL, 1024);
	html[0] = '\0';

	printf("%s", process(&html, buf, &buf[strlen(buf)]));
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
*apsprintf(char *in, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	int l = vsnprintf(NULL, 0, fmt, ap), pl = strlen(in);
	va_end(ap);

	if(!l)
		return in;
	
	in = emalloc(in, pl + l + 1);
	va_start(ap, fmt);
	vsnprintf(in + pl, l + 1, fmt, ap);
	va_end(ap);

	return in;
}

static char
*filetobuf(FILE *in) {
	if(!in)
		return NULL;

	fseek(in, 0, SEEK_END);
	long bufsiz = ftell(in);
	char *buf = emalloc(NULL, bufsiz + 3);
	fseek(in, 0, SEEK_SET);
	
	buf[0] = '\n';
	size_t len = fread(buf + 1, 1, bufsiz, in);
	buf[len + 1] = '\0';
	return buf;
}

static char
*process(char **ret, char *st, char *en) {
	if(!(st) || !(en))
		return 0;
	if(!(*ret))
		*ret = emalloc(NULL, 1024);
	
	for(char *p = st; p < en; p++) {
		int ch = 0;
		for(size_t i = 0; i < LENGTH(parsers) && !ch; i++)
			ch = parsers[i](ret, p, en);
		if(ch)
			p += ch - 1;
		else
			*ret = apsprintf(*ret, "%c", p[0]);
	}
	return *ret;
}

static int
underlines(char **ret, char *st, char *en) {
	if(!(st) || st[0] != '\n' || st[1] == '\n')
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
		*ret = apsprintf(*ret, "<h1>");
		process(ret, st + 1, st + l);
		*ret = apsprintf(*ret, "</h1>\n");
	} else {
		*ret = apsprintf(*ret, "<h2>");
		process(ret, st + 1, st + l);
		*ret = apsprintf(*ret, "</h2>\n");
	}
	
	return (p - st) + 1;
}

static int
paragraphs(char **ret, char *st, char *en) {
	if(!(st) || st[0] != '\n' || st[1] == '\n')
		return 0;
	
	char *p = st + 1;
	for(;p < en && p[0] != '\n'; p++);
	if(p[0] != '\n')
		return 0;
	
	*ret = apsprintf(*ret, "<p>");
	process(ret, st + 1, p);
	*ret = apsprintf(*ret, "</p>\n");
	return (p - st);
}

static int
links(char **ret, char *st, char *en) {
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
	
	if(h)
		*ret = apsprintf(*ret, "<a href=\"%s\">", h);
	else
		*ret = apsprintf(*ret, "<a>");
	if(img)
		*ret = apsprintf(*ret, "<img src=\"%s\"></a>", buf);
	else
		*ret = apsprintf(*ret, "%s</a>", buf);

	free(buf);
	return (p - st) + 1;
}

static int
replace(char **ret, char *st, char *en) {
	if(!(st))
		return 0;
	switch(st[0]) {
		case '&':
			*ret = apsprintf(*ret, "&amp;");
			return 1;
		case '<':
			*ret = apsprintf(*ret, "&lt;");
			return 1;
		case '>':
			*ret = apsprintf(*ret, "&gt;");
			return 1;
		case '\"':
			*ret = apsprintf(*ret, "&quot;");
			return 1;
		case '\'':
			*ret = apsprintf(*ret, "&#39;");
			return 1;
		case '\n':
			*ret = apsprintf(*ret, "<br>\n");
			return 1;
		default:
			return 0;
	}
}
