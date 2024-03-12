#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>
#include <cstdarg>

using ld = long double;

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

bool isfloat(const char *s, ld &val) {
  char trash[20];
  ld v;
  if (sscanf(s, "%Lf%10s", &v, trash) != 1) return false;
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

const char *USAGE = "Usage: %s judge_in judge_ans contestant_ans";

int main(int argc, char **argv) {
  if(argc < 4) {
    judge_error(USAGE, argv[0]);
  }
  openfile(judgein, argv[1], argv[0]);
  openfile(judgeans, argv[2], argv[0]);
  openfile(conans, argv[3], argv[0]);

  constexpr ld float_rel_tol = 1e-9;

  ld jval;
  if (!(judgeans >> jval)) {
    judge_error("No answer given from judge!\n");
  }
  std::string team;
  if (!(conans >> team)) {
    wrong_answer("No answer given.\n");
  }

  if (jval < 0) {
    if (team != "-1") {
      wrong_answer("-1 is expected. output: %s\n", team.c_str());
    }
  } else {
    ld tval;
    if (!isfloat(team.c_str(), tval)) {
      wrong_answer("Not a float. output: %s\n", team.c_str());
    }
    constexpr ld eps = 1e-6;
    if (!(fabs(jval - tval) <= (1 + eps)*float_rel_tol*fabs(jval))) {
      wrong_answer("Too large difference.\n Judge: %.14Lf\n Team: %s\n Difference: %.14Lf\n", 
          jval, team.c_str(), jval - tval);
    }
  }

  if (conans >> team) {
    wrong_answer("Trailing output:\n%s", team.c_str());
  }

  ac();
}
