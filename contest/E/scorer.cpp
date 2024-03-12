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

int X[120][120];

struct Mod {
  int i, j, v;
};

const char *USAGE = "Usage: %s judge_in judge_ans contestant_ans";

int main(int argc, char **argv) {
  if(argc < 4) {
    judge_error(USAGE, argv[0]);
  }
  openfile(judgein, argv[1], argv[0]);
  openfile(judgeans, argv[2], argv[0]);
  openfile(conans, argv[3], argv[0]);

  int t;
  if (!(judgein >> t)) judge_error("input broken! t not found\n");
  if (t < 1 || t > 1000) judge_error("input broken! t=%d\n", t);
  int nsum2 = 0;
  while (t--) {
    int n;
    if (!(judgein >> n)) judge_error("input broken! n not found\n");
    if (n < 3 || n > 100) judge_error("input broken! n=%d\n", n);
    nsum2 += n * n;
    if (nsum2 > 10000) judge_error("input broken! nsum2=%d\n", nsum2);

    for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      if (!(judgein >> X[i][j])) judge_error("input broken! Xij not found\n");
      if (X[i][j] < 1 || X[i][j] > n) judge_error("input broken! X[%d][%d]=%d\n", i, j, X[i][j]);
    }

    int m_ans;
    if (!(judgeans >> m_ans)) judge_error("model solution broken! m_ans cannot be read");
    for (int i = 0; i < m_ans; ++i) {
      int nop1, nop2, nop3;
      if (!(judgeans >> nop1 >> nop2 >> nop3)) judge_error("model solution broken! mod values cannot be read");
    }

    int m_con;
    if (!(conans >> m_con)) wrong_answer("m cannot be read");
    if (m_con > m_ans) {
      wrong_answer("contestant's output has more modifications: expected=%d, received=%d", m_ans, m_con);
    }

    std::vector<Mod> mod_con;
    for (int it = 0; it < m_con; ++it) {
      int i = 0, j = 0, v = 0;
      if (!(conans >> i >> j >> v)) {
        wrong_answer("unable to read the %d-th modification", it + 1);
      }
      if (i < 1 || i > n) wrong_answer("i is violating range");
      if (j < 1 || j > n) wrong_answer("j is violating range");
      if (v < 1 || v > n) wrong_answer("v is violating range");

      mod_con.push_back({i, j, v});
    }

    for (auto mod : mod_con) {
      X[mod.i - 1][mod.j - 1] = mod.v;
    }

    for (int i = 0; i < n; ++i) {
      std::set<int> st;
      for (int j = 0; j < n; ++j) st.insert(X[i][j]);
      if ((int)st.size() == n) {
        wrong_answer("row %d does not contain duplicate", i + 1);
      }
    }

    for (int j = 0; j < n; ++j) {
      std::set<int> st;
      for (int i = 0; i < n; ++i) st.insert(X[i][j]);
      if ((int)st.size() == n) {
        wrong_answer("column %d does not contain duplicate", j + 1);
      }
    }

    if (m_con < m_ans) {
      judge_error("contestant's output is better than us!? that cannot be happening! expected=%d, received=%d", m_ans, m_con);
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
