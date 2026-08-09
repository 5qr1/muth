#ifndef MUC_H
#define MUC_H

#ifndef VERSION
	#define VERSION "?"
#endif

#define BUFFERSIZE 2048
#define LENGTH(x)  sizeof(x)/sizeof(x[0])

typedef int (*Parser)(char *, char *);

static void eprint(const char *format, int err, ...);
static void *emalloc(int n);
static int paragraphs(char *section, char *end);
static int underlines(char *section, char *end);
static int links(char *section, char *end);
static void hprint(const char *format, ...);
static char *mkbuf(FILE *in);
static void process(char *section, char *end, Parser pparser);
extern char *muc_htmlmd(char *in);

static char *hbuf;
static int hbufs;
Parser parsers[] = {underlines, paragraphs, links};
const char *fmts[] = {
	".jpg",
	".jpeg",
	".png",
	".gif",
	".webp",
	".bmp",
	".tiff",
	".svg",
	".heic",
	".heif",
	".avif"
};

#endif
