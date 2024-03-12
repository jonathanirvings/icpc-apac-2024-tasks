#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>
#include <cstdarg>
#include <vector>
#include <set>
#include <algorithm>

std::ifstream judgein, judgeans, conans;
FILE *judgemessage = NULL;
FILE *diffpos = NULL;
int judgeans_pos, con_pos;
int judgeans_line, con_line;

void ac() {
	puts("AC");
	exit(0); 
}

void wa() {
	puts("WA");
	exit(0);
}

void wrong_answer(const char *err, ...) {
	va_list pvar;
	va_start(pvar, err);
	vfprintf(stderr, err, pvar);
	fprintf(stderr, "\n");
	wa();
}

void judge_error(const char *err, ...) {
	va_list pvar;
	va_start(pvar, err);
	// If judgemessage hasn't been set up yet, write error to stderr
	if (!judgemessage) judgemessage = stderr;
	vfprintf(judgemessage, err, pvar);
	fprintf(judgemessage, "\n");
	assert(!"Judge Error");
}

bool isfloat(const char *s, double &val) {
	char trash[20];
	double v;
	if (sscanf(s, "%lf%10s", &v, trash) != 1) return false;
	val = v;
	return true;
}

template <typename Stream>
void openfile(Stream &stream, const char *file, const char *whoami) {
	stream.open(file);
	if (stream.fail()) {
		judge_error("%s: failed to open %s\n", whoami, file);
	}
}

FILE *openfeedback(const char *feedbackdir, const char *feedback, const char *whoami) {
	std::string path = std::string(feedbackdir) + "/" + std::string(feedback);
	FILE *res = fopen(path.c_str(), "w");
	if (!res) {
		judge_error("%s: failed to open %s for writing", whoami, path.c_str());
	}
	return res;
}

int N, ANS;
int A[57], B[57], CON[57];

const char *USAGE = "Usage: %s judge_in judge_ans contestant_ans";

int main(int argc, char **argv) {
	if(argc < 4) {
		judge_error(USAGE, argv[0]);
	}
	openfile(judgein, argv[1], argv[0]);
	openfile(judgeans, argv[2], argv[0]);
	openfile(conans, argv[3], argv[0]);

	if (!(judgein >> N)) judge_error("input broken! N not found\n");
	if (!(2 <= N && N <= 50)) judge_error("input broken! N=%d\n", N);
	
	for (int i = 0; i < N; ++i) {
		if (!(judgein >> A[i])) judge_error("input broken! A[i] not found\n");
		if (!(judgein >> B[i])) judge_error("input broken! B[i] not found\n");
		if (!(1 <= A[i] && A[i] <= B[i] && B[i] <= 50)) judge_error("input broken! A[%d]=%d and B[%d]=%d\n", i, A[i], i, B[i]);
	}

	if (!(judgeans >> ANS)) judge_error("model solution broken! ANS cannot be read");
	if (!(conans >> CON[0])) wrong_answer("CON[0] cannot be read");

	if (CON[0] == -1) {
		if (ANS != -1) {
			wrong_answer("contestant cannot find a solution when there is a solution");
		}
	} else {
		for (int i = 1; i < N; ++i) {
			if (!(conans >> CON[i])) wrong_answer("CON[%d] cannot be read", i);
		}
		
		for (int i = 0; i < N; ++i) {
			if (!(A[i] <= CON[i] && CON[i] <= B[i])) {
				wrong_answer("CON[%d]=%d is invalid as A[%d]=%d and B[%d]=%d", i, CON[i], i, A[i], i, B[i]);
			}
			for (int j = i+1; j < N; ++j) {
				if (std::__gcd(CON[i],CON[j]) != 1) {
					wrong_answer("CON[%d]=%d and CON[%d]=%d are not coprime", i, CON[i], j, CON[j]);
				}
			}
		}

		if (ANS == -1) {
			judge_error("contestant's output is valid while we don't find any!? that cannot be happening!");
		}
	}

	std::string j;
	if (judgein >> j) {
		judge_error("Unread token:\n%s", j.c_str());
	}

	std::string team;
	if (conans >> team) {
		wrong_answer("Trailing output:\n%s", team.c_str());
	}

	ac();
}
