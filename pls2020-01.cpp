// pls2020-01.cpp : définit le point d'entrée pour l'application console.
//

// #include "stdafx.h"


// PLS01-2020.cpp : définit le point d'entrée pour l'application console.
//

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define strcpy strcpy_s


// todo : eviter les motifs identiques a une rotation / symetrie pret. 
// todo : limiter iloopMax : Ex 10011 11111 01000 ne sert a rien car on est deja passe par 00010 11111 11001
//    00010 11111 11001
//    10011 11111 01000 unusefull
//    00001 11110 10000
//    00001 01111 10000 unusefull
// done : limiter iloopMin : ne pas commencer a 1 mais a 00001 00001 00001 pour eviter des lignes vides
// todo : adapter w et h a wMotif et hMotif
// todo : eviter les motifs avec une colonne de zeros (car identique a motif(wMotif-1,hMotif)) 

clock_t  t1, t2;

#define uchar unsigned char
#define uint unsigned int

bool symV = false;

// static const int w = 21 + 2, h = 15 + 2, n = 4;
int w = 23 + 2, h = 17 + 2, n = 4;
// static const int w = 46 + 2, h = 34 + 2, n = 4;

int wMotif = 11, hMotif = 5;

char bits[256]; // = "100000000001100110011001111111111";
typedef struct {
	char bits[256];
}BITS;
BITS *bitsDone = (BITS*)NULL;
int nbBitsAlloc = 0, nbBitsDone = 0;

unsigned int loops, nbSol, nbSolTot;

#define TYP unsigned long long
void strreverse(const char *src, char *dst) {
	int l = strlen(src);
	dst[l] = 0;
	for (int i = l - 1; i >= 0; i--) dst[l - i] = src[i];
}
typedef struct {
	int x, y, r;
}SOL;
SOL sol[16];
class MOTIF {
public:
	int w, h;
	uchar *t;
	MOTIF(const int w, const int h) {
		this->w = w;
		this->h = h;
		this->t = (uchar*)malloc(this->w*this->h * sizeof(uchar));
		this->ini();
	}
	void ini() {
		for (int i = 0; i < this->w*this->h; i++) this->t[i] = 0;
	}
	void setc(const int y, const char *str) {
		int l = (int)strlen(str);
		for (int i = 0; i < this->w; i++) if (i < l && str[i] == '1') this->t[y*this->w + i] = 1; else this->t[y*this->w + i] = 0;
	}
	void setcAll(const int y, const char *str) {
		int l = (int)strlen(str);
		for (int i = 0; i < this->w*this->h; i++) if (i < l && str[i] == '1') this->t[y*this->w + i] = 1; else this->t[y*this->w + i] = 0;
	}
	void symv() {	// copie les lignes du haut a l'envers en bas (symetrie verticale)
		for (int j = 0; j < this->h / 2; j++) {
			for (int i = 0; i < this->w; i++) {
				this->t[(this->h - j - 1)*this->w + i] = this->t[j*this->w + i];
			}
		}
	}
	void symc() {	// copie les lignes du haut a l'envers en bas, et gauche droite (symetrie centrale)
		for (int j = 0; j < this->h / 2; j++) {
			for (int i = 0; i < this->w; i++) {
				this->t[(this->h - j - 1)*this->w + this->w - i - 1] = this->t[j*this->w + i];
			}
		}
	}
	void mirrorv() {	// inverse le haut et le bas
		for (int j = 0; j < this->h / 2; j++) {
			for (int i = 0; i < this->w; i++) {
				uchar tmp = this->t[(this->h - j - 1)*this->w + i]; this->t[(this->h - j - 1)*this->w + i] = this->t[j*this->w + i]; this->t[j*this->w + i] = tmp;
			}
		}
	}
	void dispMotif(FILE *f = stdout, bool sample = false) {
		if (sample) {
			int hmax = this->h; //  this->h & 1 ? this->h / 2 + 1 : this->h / 2;
			for (int j = 0; j < hmax; j++) {
				for (int i = 0; i < this->w; i++) {
					fprintf(f, "%d", (int)this->t[j*this->w + i]);
				}
				// fprintf(f, " ");
			}
			// printf("\n");
			return;
		}
		int nbOne = 0;
		for (int j = 0; j < this->h; j++) {
			for (int i = 0; i < this->w; i++) {
				nbOne += (this->t[j*this->w + i] == 1);
				fprintf(f, "%d", (int)this->t[j*this->w + i]);
			}
			fprintf(f, "\n");
		}
		fprintf(f, "nb 1 = %d\n", nbOne);
	}
	void copyFrom(MOTIF *other) {
		if (other->w > this->w || other->h > this->h)
			this->t = (uchar*)realloc(this->t, other->w*other->h * sizeof(uchar));
		this->w = other->w;
		this->h = other->h;
		// this->t = (uchar*)malloc(this->w*this->h*sizeof(uchar));
		memcpy(this->t, other->t, this->w*this->h * sizeof(uchar));
	}
	void rotate90() {
		uchar *tmp=(uchar*)malloc(::w*::h);
		memcpy(tmp, t, w*h);
		for (int j = 0; j < this->h; j++) {
			for (int i = 0; i < this->w; i++) {
				int y = j, x = i;
				int Y = i, X = this->h - j - 1;
				t[Y*this->h + X] = tmp[y*this->w + x];
			}
		}
		int itmp = this->w;
		this->w = this->h;
		this->h = itmp;
		free(tmp);
	}

};

MOTIF *motif[4];
uchar *t = (uchar*)NULL;
uchar *buf = (uchar*)NULL;

void fillBuff(const int x, const int y, const int ww, const int hh, char *buf) {
	// printf("fill(%d,%d)", x, y);
	buf[y*ww + x] = 'x';
	if (x > 0 && buf[y*ww + x - 1] == '1') fillBuff(x - 1, y, ww, hh, buf);
	if (x < ww - 1 && buf[y*ww + x + 1] == '1') fillBuff(x + 1, y, ww, hh, buf);
	if (y > 0 && buf[(y - 1)*ww + x] == '1') fillBuff(x, y - 1, ww, hh, buf);
	if (y < hh - 1 && buf[(y + 1)*ww + x] == '1') fillBuff(x, y + 1, ww, hh, buf);
}
bool isConnected(const int ww, const int hh, const char *bits) {
	char buf[512];
	int hhmax = hh;	// because of vertical symetry
	if (symV) {
		hhmax = hh / 2 + 1;	// because of vertical symetry
		if (hh & 1) hhmax++;
	}
	for (int j = 0; j < hhmax; j++) {
		bool isEmpty = true;
		for (int i = 0; i < ww; i++) {
			if (bits[j*ww + i] == '1') {
				isEmpty = false;
				memcpy(buf, bits, ww*hh);
				fillBuff(i, j, ww, hh, buf);
				for (int ij = ww * hh - 1; ij >= 0; ij--) if (buf[ij] == '1') return false;
				return true;
			}
		}
		// printf("ww=%d hh=%d hhmax=%d isEmpty=%d\n", ww, hh, hhmax, isEmpty ? 1 : 0);
		if (isEmpty) return false;
	}
	return true;
}
void draw(const int x, const int y, const uchar prof, MOTIF *m) {
	for (int j = 0; j < m->h; j++) {
		uchar *ptr = &t[(y + j)*w + x];
		for (int i = 0; i < m->w; i++) {
			if (m->t[j*m->w + i]) ptr[i] = prof;
		}
	}
}
void undraw(const int x, const int y, MOTIF *m) {
	for (int j = 0; j < m->h; j++) {
		uchar *ptr = &t[(y + j)*w + x];
		for (int i = 0; i < m->w; i++) {
			if (m->t[j*m->w + i]) ptr[i] = 0;
		}
	}
}
bool can(const int x, const int y, MOTIF *m) {
	for (int j = 0; j < m->h; j++) {
		uchar *ptr = &t[(y + j)*w + x];
		for (int i = 0; i < m->w; i++) {
			if (m->t[j*m->w + i] && ptr[i]) return false;
		}
	}
	return true;
}
void disp(FILE *f = stdout) {
	nbSol++;
	fprintf(f, "sol[]="); for (int i = 1; i <= n; i++) fprintf(f, "[%d,%d,%d] ", sol[i].x, sol[i].y, sol[i].r); fprintf(f, " t[] = \n");
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			fprintf(f, "%d", (int)t[j*w + i]);
		}
		fprintf(f, "\n");
	}
	fflush(f);
}

void dispMotif(MOTIF *m, bool sample = false) {
	if (sample) {
		for (int j = 0; j < m->h / 2; j++) {
			for (int i = 0; i < m->w; i++) {
				printf("%d", (int)m->t[j*m->w + i]);
			}
			printf(" ");
		}
		printf("\n");
		return;
	}
	int nbOne = 0;
	for (int j = 0; j < m->h; j++) {
		for (int i = 0; i < m->w; i++) {
			nbOne += m->t[j*m->w + i] == 1;
			printf("%d", (int)m->t[j*m->w + i]);
		}
		printf("\n");
	}
	printf("nb 1 = %d\n", nbOne);
}
bool contact(const int i, const int j) {
	// verifie que les i et les j se touchent
	bool contactOn = false;
	for (int y = 0; y < h; y++) {
		if (contactOn) break;
		for (int x = 0; x < w; x++) {
			if (t[y*w + x] != i) continue;
			// left ?
			if (x > 0) {
				if (t[y*w + x - 1] == j) { contactOn = true; break; }
			}
			// right ?
			if (x < w - 1) {
				if (t[y*w + x + 1] == j) { contactOn = true; break; }
			}
			// top ?
			if (y > 0) {
				if (t[(y - 1)*w + x] == j) { contactOn = true; break; }
			}
			// bottom ?
			if (y < h - 1) {
				if (t[(y + 1)*w + x] == j) { contactOn = true; break; }
			}
		}
	}
	return contactOn;
}
bool contact4() {
	// verifie que le ieme est en contact avec tous les autres
	for (int i = 1; i < n; i++) {
		for (int j = i + 1; j <= n; j++) {
			uchar ok_i_contact_j = false;
			for (int y = 0; y < h; y++) {
				if (ok_i_contact_j) break;
				for (int x = 0; x < w; x++) {
					if (t[y*w + x] != i) continue;
					// left ?
					if (x > 0) {
						if (t[y*w + x - 1] == j) { ok_i_contact_j = 1; break; }
					}
					// right ?
					if (x < w - 1) {
						if (t[y*w + x + 1] == j) { ok_i_contact_j = 1; break; }
					}
					// top ?
					if (y > 0) {
						if (t[(y - 1)*w + x] == j) { ok_i_contact_j = 1; break; }
					}
					// bottom ?
					if (y < h - 1) {
						if (t[(y + 1)*w + x] == j) { ok_i_contact_j = 1; break; }
					}
				}
			}
			if (!ok_i_contact_j) return false;
		}
	}
	return true;
}
void dispBuf() {
	printf("buf[]=\n");
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			printf("%d", (int)buf[j*w + i]);
		}
		printf("\n");
	}
}
void fill(const int x, const int y) {
	// printf("fill(%d,%d)", x, y);
	buf[y*w + x] = 9;
	if (x > 0 && buf[y*w + x - 1] == 0) fill(x - 1, y);
	if (x < w - 1 && buf[y*w + x + 1] == 0) fill(x + 1, y);
	if (y > 0 && buf[(y - 1)*w + x] == 0) fill(x, y - 1);
	if (y < h - 1 && buf[(y + 1)*w + x] == 0) fill(x, y + 1);
}
bool noHoles() {		// returns true if no holes in t
						// fill 1st pix having 0 value with 9, then check there is no more pix having 0 value
	for (int j = h - 2; j < h; j++) {
		for (int i = 0; i < w; i++) {
			if (t[j*w + i] == 0) {
				memcpy(buf, t, w*h * sizeof(uchar));
				// dispBuf();
				fill(i, j); // fill buf
							// dispBuf(); exit(1);
				for (int ij = 0; ij < w*h; ij++) {
					if (buf[ij] == 0) {
						return false;
					}
				}
				return true;
			}
		}
	}
	return true;  // not any pix with 0 value
}
bool parseFound = false;
void parse(const uchar prof, int caseDepart = w + 1) {
	loops++;
	// printf("parse(%d, %d [%d,%d]) loops=%d\n", (int)prof, caseDepart, caseDepart / w, caseDepart%w, loops);
	// horiz
	for (int r = 0; r < 4; r++) {
		int xmax = w - motif[r]->w, ymax = h - motif[r]->h;
		int ymin = caseDepart / w;
		int xmin = caseDepart % w;
		for (int x = xmin, y = ymin; !parseFound; x++) {
			if (x >= xmax) {
				x = 1;
				y++;
				if (y >= ymax) break;
			}
			if (can(x, y, motif[r])) {
				sol[prof].x = x;
				sol[prof].y = y;
				sol[prof].r = r;
				draw(x, y, prof, motif[r]);
				bool contactOk = true;
				for (int i = 1; i < prof; i++) if (!contact(prof, i)) contactOk = false;
				if (contactOk) {
					if (prof == 4) {
						bool deja = false;
						char bitsRev[256];
						strreverse(bits, bitsRev);
						for (int i = 0; i < nbBitsDone; i++) {
							if (!strcmp(bits, bitsDone[i].bits)) {
								deja = true; break;
							}
							if (!strcmp(bitsRev, bitsDone[i].bits)) {
								deja = true; break;
							}
						}
						// fprintf(stderr, "parse(%d) x=%d y=%d\n", prof, x, y);
						if (!deja) {
							// fprintf(stderr, "!deja\n");
							// exit(1);
							printf("new bits=%s not found in %d solutions.\n", bits, nbBitsDone);
							if (nbBitsDone >= nbBitsAlloc) {
								nbBitsAlloc += 256;
								bitsDone = (BITS*)realloc(bitsDone, nbBitsAlloc * 256 * sizeof(char));
							}
							strcpy(bitsDone[nbBitsDone++].bits, bits);
							printf("Solution %d trouvee !\n", nbSolTot + nbSol + 1);
							printf("motif %dx%d: \n", motif[0]->w, motif[0]->h); motif[0]->dispMotif();
							printf("isConnected(%d,%d,", wMotif, hMotif);
							// printf(",%s", bits);
							motif[0]->dispMotif(stdout, true);
							printf(")=%s\n", isConnected(wMotif, hMotif, bits) ? "yes" : "no");
							disp();
							FILE *f;
							char fout[80];
							sprintf_s(fout, "sol_%d.txt", wMotif);
							int ok = fopen_s(&f, fout, "a");
							fprintf(f, "Solution trouvee !\n");
							fprintf(f, "motif %dx%d: \n", motif[0]->w, motif[0]->h); motif[0]->dispMotif(f);
							fprintf(f, "isConnected(%d,%d,", wMotif, hMotif);
							// fprintf(f, ",%s", bits);
							motif[0]->dispMotif(f, true);
							fprintf(f, ")=%s\n", isConnected(wMotif, hMotif, bits) ? "yes" : "no");
							disp(f);
							fclose(f);
							parseFound = true;
						}
					}
					else parse(prof + 1, y*w + x);
				}
				undraw(x, y, motif[r]);
			}
		}
	}
}
void int2bin(const uint v, const int nbBits, char *dst) {
	unsigned int msk = 0x00000001;
	if (nbBits > 1) msk <<= (nbBits - 1);	// ex : nbBits=4 => msk=0001, 0010, 0100, 1000 : mask = 1000 = 0x80
	for (int i = 0; i < nbBits; i++) {
		dst[i] = v & msk ? '1' : '0';
		msk >>= 1;
	}
	dst[nbBits] = '\0';
}
void int2bin(const unsigned long long v, const int nbBits, char *dst) {
	TYP msk = 0x00000001;
	if (nbBits > 1) msk <<= (nbBits - 1);	// ex : nbBits=4 => msk=0001, 0010, 0100, 1000 : mask = 1000 = 0x80
	for (int i = 0; i < nbBits; i++) {
		dst[i] = v & msk ? '1' : '0';
		msk >>= 1;
	}
	dst[nbBits] = '\0';
}
int nbBitsOnes(const uint v) {
	int nbBits = 32;
	uint msk = 1 << (nbBits - 1);
	int ret = 0;
	for (int i = 0; i < nbBits; i++) {
		if (v&msk) ret++;
		msk >>= 1;
	}
	return ret;
}
int nbBitsOnes(const unsigned long long v) {
	int nbBits = 64;
	unsigned long long msk = 1LL << (nbBits - 1);
	int ret = 0;
	for (int i = 0; i < nbBits; i++) {
		if (v&msk) ret++;
		msk >>= 1;
	}
	return ret;
}

void printBits(uint *ptr, char *str = (char*)NULL)
{
	size_t size = sizeof(ptr);
	printf("printBits::size=%d\n", size);
	unsigned char *b = (unsigned char*)ptr;
	unsigned char byte;
	int i, j;

	for (i = size - 1; i >= 0; i--)
	{
		for (j = 7; j >= 0; j--)
		{
			byte = (b[i] >> j) & 1;
			printf("%u", byte);
		}
	}
	if (str) printf("%s", str);
}
void printBits(unsigned long long *ptr, char *str = (char*)NULL)
{
	size_t size = sizeof(*ptr);
	// printf("printBits::size=%d\n", size);
	unsigned char *b = (unsigned char*)ptr;
	unsigned char byte;
	int i, j;

	for (i = size - 1; i >= 0; i--)
	{
		for (j = 7; j >= 0; j--)
		{
			byte = (b[i] >> j) & 1;
			printf("%u", byte);
		}
		printf(" ");
	}
	if (str) printf("%s", str);
}
int main(int argc, char* argv[])
{
#if SINGLE
	//reset t
	for (int i = 0; i < w*h; i++) t[i] = 0;
	// create 4 motifs
	wMotif = 7; hMotif = 5;
	for (int i = 0; i < 4; i++) motif[i] = new MOTIF(wMotif, hMotif);
	char mbits[256];
#define xTEST_c 1
#if TEST_c
	strcpy(mbits, "0000001");	motif[0]->setc(0, mbits);
	strcpy(mbits, "0100001");	motif[0]->setc(1, mbits);
	strcpy(mbits, "1111111");	motif[0]->setc(2, mbits);
	// motif[0]->dispMotif();
	motif[0]->symc();
	motif[1]->copyFrom(motif[0]);
	motif[1]->rotate90();
	motif[2]->copyFrom(motif[0]);
	motif[2]->mirrorv();
	motif[3]->copyFrom(motif[2]);
	motif[3]->rotate90();
#else
	strcpy(mbits, "1000000");	motif[0]->setc(0, mbits);
	strcpy(mbits, "1000001");	motif[0]->setc(1, mbits);
	strcpy(mbits, "1111111");	motif[0]->setc(2, mbits);
	motif[0]->symv();
	for (int i = 1; i < 4; i++) {
		motif[i]->copyFrom(motif[i - 1]);
		motif[i]->rotate90();
	}
#endif
	motif[0]->dispMotif();

	for (int i = 1; i < 4; i++) {
		printf("motif motif[%d] : \n", i);
		motif[i]->dispMotif();
	}

	int x = w / 2 - motif[0]->w / 2;
	int y = h / 2 - motif[0]->h / 2;
	int r = 0;
	sol[1].x = x;
	sol[1].y = y;
	sol[1].r = r;
	draw(x, y, 1, motif[r]);
	disp();
	parse((uchar)2);

	printf("Finito.\n");
	exit(1);
	// strcat(mbits, "1010010");
	// strcat(mbits, "1010010");
	// strcat(mbits, "1000000");
#endif
	int wMin = 5, wMax = 32, hFixed=0;
	if (argc > 1) {
		sscanf_s(argv[1], "%d", &wMin);
		wMax = wMin;
	}
	if (argc > 2) {
		sscanf_s(argv[2], "%d", &hFixed);
	}
	if (argc > 3 && !strcmp(argv[3], "symv")) {
		symV = true;
	}
	nbSolTot = 0;
	for (wMotif = wMin; wMotif <= wMax; wMotif++) {
		int hMin = 3, hMax = wMotif;
		if (hFixed) {
			hMin = hMax = hFixed;
		}
		// if (wMin == 6) { hMin = 6; hMax = 6; }
		for (hMotif = hMin; hMotif <= hMax; hMotif++) {

			// adapt grid size to shape size
			w = wMotif * 3 + 2;
			h = hMotif * 3 + 2;
			t = (uchar*)realloc(t, w * h * sizeof(uchar));
			buf = (uchar*)realloc(buf, w * h * sizeof(uchar));

			int hHalf = hMotif;
			if (symV) {
				hHalf = hMotif / 2;
				if (hMotif & 1) hHalf++;
			}

			int nbBitsNeeded = wMotif * hHalf;		// ex : 8x4 => (8*4)/2 = 16 bits, 8x5 => 16 + 8 = 24 bits
			if (nbBitsNeeded > 64) continue;

			printf("\n***** motif %dx%d symv=%s (nbBitsNeeded=%d) ****\n", wMotif, hMotif, (symV?"yes":"no"), nbBitsNeeded);
			//reset t
			for (int i = 0; i < w*h; i++) t[i] = 0;
			nbSol = 0;
			// create 4 motifs
			for (int i = 0; i < 4; i++) motif[i] = new MOTIF(wMotif, hMotif);
			t1 = clock();

			// loop on combinations
			loops = 0;
			TYP maskOneLine = (1 << wMotif) - 1;	// ex: wMotif=8 => 0x00000100 -1 = 0x000000ff
			TYP iloopMax = (uint)(1 << nbBitsNeeded) - 1;
			TYP iloopMin = 1; //  (uint)(1 << ((wMotif)*(hHalf - 1)));
			for (int i = 1; i < hHalf; i++) {	// statrs with 100001000001000001 (to prevent empty lines)
				iloopMin |= (unsigned long long)(1LL << (wMotif*i));
			}
			// printf(" wMotif=%d hHalf=%d iloopMin=", wMotif, hHalf); printBits(&iloopMin); printf("\n");
			for (TYP iloop = iloopMin; iloop != iloopMax; iloop++) {
				// if (nbBitsOnes(iloop) > 17) continue;
				bool emptyLines = false;
				TYP maskOneLineCopy = maskOneLine;
				for (int i = 0; i < hHalf && !emptyLines; i++) {
					// printf(" maskOneLineCopy="); printBits(&maskOneLineCopy); printf("\n");
					if ((iloop & maskOneLineCopy) == 0) emptyLines = true;
					maskOneLineCopy <<= wMotif;
				}
				if (emptyLines) {
					//printf(" iloop="); printBits(&iloop); printf(" : empty lines\n");
					continue;
				}

				bool emptyColumn;
				TYP maskOneColumn;
				for (int i = 0; i < wMotif; i++) {
					maskOneColumn = 1; if (i > 0) maskOneColumn <<= i;
					emptyColumn = true;
					for (int j = 0; j < hHalf && emptyColumn; j++) {
						if (iloop & maskOneColumn) {
							emptyColumn = false;
						}
						maskOneColumn <<= wMotif;
					}
					if (emptyColumn) {
						break;
					}
					maskOneColumn <<= 1LL;
					// printf("j=[0,%d[ col %d checked\n", hHalf, i);
				}
				if (emptyColumn) continue;

				// printf("wMotif=%d hMotif=%d iloop=%llu iloop=0x%llx", wMotif, hMotif, iloop, iloop);
				// printf(" bin="); printBits(&iloop);

				int2bin(iloop, nbBitsNeeded, bits);
				// for (int y = 0; y < hHalf; y++) motif[0]->setc(y, bits + wMotif * y /*+32 -3*wMotif-1*/);

				// printf("iloop=%llu motif=", iloop); motif[0]->dispMotif(stdout);
				// exit(1);

				if (!isConnected(wMotif, hMotif, bits)) {
					//printf("not connected (%d,%d,%s)\n",wMotif, hMotif, bits);
					continue;
				}
				//exit(1);

				// printf("motif=%dx%d iloop=%12u=%08x bits=(l=%d) %s", wMotif, hMotif, iloop, iloop, strlen(bits), bits);
				// printf(" bin="); printBits(4, &iloop);

				for (int y = 0; y < hHalf; y++) motif[0]->setc(y, bits + wMotif * y /*+32 -3*wMotif-1*/);

				// printf(" "); motif[0]->dispMotif(stdout, true);

				// printf("   %c", 13);	// printf("\n");

				if (symV) motif[0]->symv();

				for (int i = 1; i < 4; i++) {
					motif[i]->copyFrom(motif[i - 1]);
					motif[i]->rotate90();
					// printf("motif motif[%d] : \n", i); motif[i]->dispMotif();
				}

				// center 1st motif in grid
				int x = w / 2 - motif[0]->w / 2;
				int y = h / 2 - motif[0]->h / 2;
				int r = 0;
				sol[1].x = x;
				sol[1].y = y;
				sol[1].r = r;
				draw(x, y, 1, motif[r]);
				// disp();
				parseFound = false;
				parse((uchar)2);
				// parse((uchar)1);
				undraw(x, y, motif[r]);
				//exit(1);
			}
			nbSolTot += nbSol;
			t2 = clock();
			printf("finished : %d sol cpu:%.3lf\n", nbSol, ((double)t2 - (double)t1) / CLOCKS_PER_SEC);
		}
	}
#define TEST 0
#if TEST
	int x;
	x = 5; y = 2;
	if (can(x, y, motif[0])) draw(x, y, 1, motif[0]); else printf("!!! cant %d,%d\n", x, y);
	disp();
	x = 0; y = 0;
	if (can(x, y, motif[1])) draw(0, 0, 2, motif[1]); else printf("!!! cant %d,%d\n", x, y);
	disp();
	return 1;
#endif
	printf("Finished after %u loops : %d sol found.\n", loops, nbSolTot);
	return 0;
}

