#include "hw6.h"

int main(int argc, char *argv[]) {
	
	extern char *optarg;
	extern int optind;

	char *s = NULL;
	char *r = NULL;

	bool w = false;
	long start = 0;
	long end = 0;

	if(argc < 7) {
		return MISSING_ARGUMENT;
	}

	char *in = argv[argc - 2];
	char *out = argv[argc - 1];

	FILE *infile = fopen(in, "r");
	FILE *outfile = fopen(out, "w");

	if(infile == NULL) {
		return INPUT_FILE_MISSING;
	}

	if(outfile == NULL) {
		return OUTPUT_FILE_UNWRITABLE;
	}

	char c;
	while((c = getopt(argc, argv, "s:r:wl:")) != -1) {
		switch(c) {
			case 's': {
				if(s != NULL) {
					return DUPLICATE_ARGUMENT;
				}

				if(optarg == NULL || optarg[0] == '-') {
					return S_ARGUMENT_MISSING;
				}

				s = optarg;
				break;
			}
			case 'r': {
				if(r != NULL) {
					return DUPLICATE_ARGUMENT;
				}

				if(optarg == NULL || optarg[0] == '-') {
					return R_ARGUMENT_MISSING;
				}

				r = optarg;
				break;
			}
			case 'w': {
				if(w) {
					return DUPLICATE_ARGUMENT;
				}

				w = true;
				break;
			}
			case 'l': {
				if(start * end != 0) {
					return DUPLICATE_ARGUMENT;
				}

				if(optarg == NULL || optarg[0] == '-') {
					return L_ARGUMENT_INVALID;
				}

				char *text = strtok(optarg, ",");

				if(text == NULL) {
					return L_ARGUMENT_INVALID;
				}

				start = strtol(text, NULL, 10);

				text = strtok(NULL, ",");

				if(text == NULL) {
					return L_ARGUMENT_INVALID;
				}

				end = strtol(text, NULL, 10);

				if(start <= 0 || end <= 0 || start > end) {
					return L_ARGUMENT_INVALID;
				}

				break;
			}
		}
	}

	if(s == NULL) {
		return S_ARGUMENT_MISSING;
	}

	if(r == NULL) {
		return R_ARGUMENT_MISSING;
	}

	int sLen = strlen(s);
	int rLen = strlen(r);

	bool w_pre;
	if(w) {
		if(s[0] == '*' && s[sLen - 1] != '*') {
			s++;
			w_pre = false;
		} else if(s[sLen - 1] == '*' && s[0] != '*') {
			s[sLen - 1] = '\0';
			w_pre = true;
		} else {
			return WILDCARD_INVALID;
		}

		sLen--;
	}

	char buffer[MAX_LINE + 1]; 
	for(int line = 1; fgets(buffer, MAX_LINE + 1, infile) != NULL; line++) {

		if(start * end != 0 && (start > line || line > end)) {
			fputs(buffer, outfile);
			continue;
		}

		char *remaining = &buffer[0];
		char *sLoc;
		while((sLoc = strstr(remaining, s)) != NULL) {
			char buf[MAX_LINE + 1];

			if(w) {
				if(w_pre) {

					if(sLoc != &buffer[0] && !isspace(sLoc[-1]) && !ispunct(sLoc[-1])) {
						remaining = sLoc + sLen;
						continue;
					}

					while(sLoc[sLen] != '\0' && sLoc[sLen] != '\n' && !isspace(sLoc[sLen]) && !ispunct(sLoc[sLen])) {
						sLen++;
					}

				} else {

					if(sLoc[sLen] != '\0' && sLoc[sLen] != '\n' && !isspace(sLoc[sLen]) && !ispunct(sLoc[sLen])) {
						remaining = sLoc + sLen;
						continue;
					}

					while(sLoc != &buffer[0] && !isspace(sLoc[-1]) && !ispunct(sLoc[-1])) {
						sLoc--;
						sLen++;
					}
				}
			}
			
			strcpy(buf, sLoc + sLen);

			strcpy(sLoc, r);

			strcpy(sLoc + rLen, buf);

			if(w) {
				sLen = strlen(s);
			}

			remaining = sLoc + rLen;

		}

		fputs(buffer, outfile);
	}

    return 0;
}
