#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

// consider chars from [FIRST_CHAR, LAST_CHAR)
#define FIRST_CHAR 33
#define LAST_CHAR 127
#define MAX_CHARS (LAST_CHAR - FIRST_CHAR)
#define MAX_DIGRAMS (LAST_CHAR - FIRST_CHAR) * (LAST_CHAR - FIRST_CHAR)

#define NEWLINE '\n'
#define IN_WORD 1

#define IN_LINE_COMMENT 1
#define IN_BLOCK_COMMENT 2

#define TEST 0   // 1 dla testowania, 0 dla automatycznej oceny

int count[MAX_DIGRAMS] = { 0 };

// function to be used as comparator to qsort (compares counters and thes sorts
// alphabetically when equal)
int cmp (const void *a, const void *b) {
	int va = *(int*)a;
	int vb = *(int*)b;
	if (count[va] == count[vb]) return va - vb; // sort alphabetically if counts equal
	return count[vb] - count[va];
}

// function to be used as comparator to qsort for digrams (compares counters and
// thes sorts alphabetically when equal)
int cmp_di (const void *a, const void *b) {
	int va = *(int*)a;
	int vb = *(int*)b;
	// sort according to second char if counts and the first char equal
	if (count[va] == count[vb] && va / MAX_CHARS == vb / MAX_CHARS) return va % MAX_CHARS - vb % MAX_CHARS;
	// sort according to first char if counts equal
	if (count[va] == count[vb]) return va / MAX_CHARS - vb / MAX_CHARS;
	return count[vb] - count[va];
}

// count number of lines (nl), number of words (nw) and number of characters
// (nc) in the text read from stream
void wc(int *nl, int *nw, int *nc, FILE *stream) {
    int c;
    *nl = *nw = *nc = 0;
    int inWord = 0;
     while ((c = getc(stream)) != EOF) {
        switch (c) {
            case NEWLINE:
                ++(*nl);
            case ' ':
            case '\t':
                if (inWord) {
                    ++(*nw);
                    inWord = 0;
                }
                ++(*nc);
                break;
            default:
                ++(*nc);
                inWord = 1;
                break;
        }
    }
}

typedef struct {
    int first;
    int second;
} pair1;

int comp1(const void* a, const void* b) {
    return ((pair1*) a)->second < ((pair1*) b)->second;
}

// count how many times each character from [FIRST_CHAR, LAST_CHAR) occurs
// in the text read from stream. Sort chars according to their respective
// cardinalities (decreasing order). Set n_char and cnt to the char_no - th char
// in the sorted list and its cardinality respectively
void char_count(int char_no, int *n_char, int *cnt, FILE *stream) {
    int c;
    #define SIZE LAST_CHAR - FIRST_CHAR
    pair1 counters[SIZE];
    for (int i = 0; i < SIZE; i++) {
        counters[i].first = FIRST_CHAR + i;
        counters[i].second = 0;
    }
    while ((c = getc(stream)) != EOF) {
        ++counters[c - FIRST_CHAR].second;
    }
    qsort(counters, SIZE, sizeof(pair1), comp1);
    *n_char = counters[char_no - 1].first;
    *cnt = counters[char_no - 1].second;
    #undef SIZE
}


typedef struct {
    int first[2];
    int second;
} pair2;

int comp2(const void* a, const void* b) {
    return ((pair2*) a)->second < ((pair2*) b)->second;
}

// count how many times each digram (a pair of characters, from [FIRST_CHAR,
// LAST_CHAR) each) occurs in the text read from stream. Sort digrams according
// to their respective cardinalities (decreasing order). Set digram[0] and
// digram[1] to the first and the second char in the digram_no - th digram_char
// in the sorted list. Set digram[2] to its cardinality.
void digram_count(int digram_no, int digram[], FILE *stream){
    int digrams[MAX_DIGRAMS];
    for(int i = 0; i < MAX_CHARS; i++)
        for(int j = 0; j < MAX_CHARS; j++)
            digrams[i*MAX_CHARS + j] = i*MAX_CHARS + j;

    char c_1 = fgetc(stream);
    char c_2 = fgetc(stream);
    while(!feof(stream)){

        if(c_1 != ' ' && c_1 != '\n' && c_1 != '\t' && c_2 != ' ' && c_2 != '\n' && c_2 != '\t')
            count[((int)c_1 - FIRST_CHAR)*MAX_CHARS + (int)c_2 - FIRST_CHAR]++;

        c_1 = c_2;
        c_2 = fgetc(stream);
    }

    qsort(digrams, MAX_DIGRAMS ,sizeof(int), cmp_di);

    digram[0] = digrams[digram_no - 1] / MAX_CHARS + FIRST_CHAR;
    digram[1] = digrams[digram_no - 1] % MAX_CHARS + FIRST_CHAR;
    digram[2] = count[digrams[digram_no - 1]];
}

// Count block and line comments in the text read from stream. Set
// line_comment_counter and block_comment_counter accordingly
void find_comments(int *line_comment_counter, int *block_comment_counter, FILE *stream){
    int inInline = 0;
    int inBlock = 0;
    int c1 = getc(stream);
    int c2 = getc(stream);
    while (c2 != EOF) {
        if (!inInline && !inBlock) {
            if (c1 == '/') {
                switch (c2) {
                    case '*':
                        inBlock = 1;
                        ++(*block_comment_counter);
                        c1 = c2;
                        c2 = getc(stream);
                        break;
                    case '/':
                        inInline = 1;
                        ++(*line_comment_counter);
                        break;
                    default:
                        break;
                }
            }
        } else if (inInline && c1 == '\n') {
            inInline = 0;
        } else if (inBlock && c1 == '*' && c2 == '/') {
            inBlock = 0;
        }
        c1 = c2;
        c2 = getc(stream);
    }
}

#define MAX_LINE 128

int read_line() {
	char line[MAX_LINE];
	int n;

	fgets (line, MAX_LINE, stdin); // to get the whole line
	sscanf (line, "%d", &n);
	return n;
}

int main(void) {
	int to_do;
	int nl, nw, nc, char_no, n_char, cnt;
	int line_comment_counter, block_comment_counter;
	int digram[3];

	char file_name[40];
	FILE *stream;

	if(TEST) printf("Wpisz nr zadania ");
    to_do = read_line();      //    scanf ("%d", &to_do);
    if(TEST)  stream = stdin;
    else {
        scanf("%s",file_name);  read_line();
        stream = fopen(file_name,"r");
        if(stream == NULL) {
            printf("fopen failed\n");
            return -1;
        }
    }

//	to_do = read_line();
	switch (to_do) {
		case 1: // wc()
			wc (&nl, &nw, &nc, stream);
			printf("%d %d %d\n", nl, nw, nc);
			break;
		case 2: // char_count()
      if(TEST) printf("Wpisz numer znaku ");
      char_no = read_line();  //    scanf("%d",&char_no);
			char_count(char_no, &n_char, &cnt, stream);
			printf("%c %d\n", n_char, cnt);
			break;
		case 3: // digram_count()
      if(TEST) printf("Wpisz numer digramu ");
      char_no = read_line();   //  scanf("%d",&char_no);
			digram_count(char_no, digram, stream);
			printf("%c%c %d\n", digram[0], digram[1], digram[2]);
			break;
		case 4:
			find_comments(&line_comment_counter, &block_comment_counter, stream);
			printf("%d %d\n", block_comment_counter, line_comment_counter);
			break;
		default:
			printf("NOTHING TO DO FOR %d\n", to_do);
			break;
	}
	return 0;
}
