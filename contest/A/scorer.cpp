/*
 * Compile with -D_DEBUG_DYNAMIC_STRING to debug the dynamic string implementation
 */

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
#include <functional>
#include <map>

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

const char *USAGE = "Usage: %s judge_in judge_ans contestant_ans";


/* checks whether v is in [low, high] */
template <typename VType, typename LType, typename HType>
bool in_interval(const VType &v, const LType &low, const HType &high) {
  return low <= v and v <= high;
}



#ifndef _THRESH
  #define _THRESH 350
#endif

constexpr int THRESH = _THRESH;

#undef _THRESH

/* The string represented is the concatenation
 * total[i0, j0) + total[i1, j1) + total[i2, j2) + ...
 *
 * where pieces = [(i0, j0), (i1, j1), (i2, j2), ...]
 * 
 * From time to time, when 'pieces' gets too long (determined by "THRESH"),
 * the whole string is "collapsed" into a single piece.
 *
 * _size represents the total length, while _rec is the number of times
 * the whole string has been "collapsed".
 */
class dynamic_string {
  int _size, _rec = 0;
  std::string total;
  std::vector<std::pair<int,int>> pieces;

  void try_collapse() {
    if (pieces.size() > THRESH) collapse();
  }

  void collapse() {
    // rebuild the whole string as a single piece.
    std::string ntotal;
    ntotal.reserve(_size);
    for (auto [I, J] : pieces) {
      for (int t = I; t < J; t++) ntotal.push_back(total[t]);
    }
    total.swap(ntotal);
    pieces = {{0, total.size()}};
    _rec++;
  }

  template <typename Container>
  void extract(int i, int j, Container &target) {
    _size -= j - i;
    int found = 0;
    for (int p = 0, l = 0; j > l and p < pieces.size(); p++) {
      auto [I, J] = pieces[p];
      l -= I;
      int ii = std::max(I, i - l);
      int jj = std::min(J, j - l);
      // [ii, jj) represents the actual range in the string 'total'
      // containing the intersection of [i, j) and piece p
      if (ii < jj) {
        found += jj - ii;
        target.insert(target.end(), total.begin() + ii, total.begin() + jj);

        // note that [I, J) = [I, ii) + [ii, jj) + [jj, J)
        if (I < ii and jj < J) {
          // if [ii, jj) is strictly contained in [I, J), then
          // we need to split the piece into two: [I, ii) and [jj, J)
          pieces[p] = {I, ii};
          pieces.insert(pieces.begin() + p + 1, std::make_pair(jj, J));
          break;
        } else if (jj == J) {
          // only [I, ii) remains
          pieces[p] = {I, ii};
        } else {
          // only [J, jj) remains
          assert(ii == I);
          pieces[p] = {jj, J};
        }
      }
      l += J;
    }
    assert(found == j - i);
  }

  template <typename Container>
  void insert(int i, const Container &source) {
    _size += source.size();
    // [ni, nj) will be the interval representing the inserted string
    int ni = total.size(), nj = ni + source.size();
    total.insert(total.end(), source.begin(), source.end());
    for (int p = 0, l = 0; p < pieces.size(); p++) {
      auto [I, J] = pieces[p];
      l -= I;
      int ii = i - l;
      // ii represents the actual position in 'total' where piece p
      // will potentially be "split" into two, assuming ii is in [I, J).
      if (I <= ii and ii <= J) {
        // note that [I, J) = [I, ii) + [ii, J)
        // we then insert [ni, nj) between [I, ii) and [ii, J)
        // but we skip the empty pieces
        static std::vector<std::pair<int, int>> to_insert;  // reduces allocation
        to_insert.clear();

        if (I < ii) to_insert.emplace_back(I, ii);
        to_insert.emplace_back(ni, nj);
        if (ii < J) to_insert.emplace_back(ii, J);

        // put the first piece in position p, then insert the rest after
        pieces[p] = to_insert.front();
        pieces.insert(pieces.begin() + p + 1, to_insert.begin() + 1, to_insert.end());
        return;
      }
      l += J;
    }
    assert(false);
  }

public:
  dynamic_string(const std::string &start): total(start), pieces{{0, start.size()}}, _size(start.size()) {}

  /* replace the substring in [i, j) with the string "target" */
  std::string replace(int i, int j, const std::string& target) {
    try_collapse();
    std::string source;
    extract(i, j, source);
    insert(i, target);
    try_collapse();
    return source;
  }

  /* clear the whole string (and return it) */
  std::string pop() { return replace(0, size(), ""); }

  bool empty() const { return size() == 0; }
  int size() const { return _size; }

  int mem() const { return pieces.size(); }
  int rebuild_count() const { return _rec; }
};


constexpr int  TMIN = 1,  TMAX = 10;
constexpr int APMIN = 5, APMAX = 20;
constexpr int  SMIN = 1,  SMAX = 50;
constexpr int  KMIN = 1,  KMAX = 35'000;

bool is_valid_state(const std::string &S) {
  if (not in_interval(S.size(), SMIN, SMAX))
    return false;
  for (char ch : S)
    if (not (ch == 'A' or ch == 'P'))
      return false;
  return true;
}

std::pair<int,int> count_ap(const std::string &S) {
  int a = 0, p = 0;
  for (char ch : S) {
    switch (ch) {
    case 'A': a++; break;
    case 'P': p++; break;
    default: judge_error("judging broken! invalid string! %s", S.c_str());
    }
  }
  return {a, p};
}


#ifdef _DEBUG_DYNAMIC_STRING

  // for dynamic string debugging only. perform string replacements given in stdin
  
  using namespace std;
  int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    auto get = [&]() {
      string s; cin >> s;
      assert(s[0] == ':');
      s.erase(s.begin());
      return s;
    };
    dynamic_string s(get());

    auto _debug_print = [&]() {
      int size = s.size();
      int mem = s.mem();
      int rebuilds = s.rebuild_count();
      fprintf(stderr, "thresh = %d, size = %d,  mem = %d,  rebuilds = %d\n", THRESH, size, mem, rebuilds);
      assert(s.mem() <= THRESH);
    };

    #ifdef VERIFY
      auto verify = [&]() {
        _debug_print();
        assert(s.mem() <= THRESH);
      };
      auto finish = [&]() {
        _debug_print();
      };
    #else
      auto verify = [&]() {
        assert(s.mem() <= THRESH);
      };
      auto finish = [&]() {
        _debug_print();
      };
    #endif

    verify();
    while (true) {
      int i, l; cin >> i >> l;
      if (i == -1) break;
      int j = i + l;
      assert(0 <= i and i <= j and j <= s.size());
      cout << s.replace(i, j, get()) << '\n';
      verify();
    }
    cout << s.pop() << '\n';
    verify();
    assert(s.empty());
    finish();
  }


#else
  // the actual checker

  void verify_single_case(const int cas) {
    // read input
    int a, p;
    std::string S, E;
    if (not (judgein >> a))
      judge_error("[Case %d] input broken! a cannot be read", cas);

    if (not (judgein >> p))
      judge_error("[Case %d] input broken! p cannot be read", cas);

    if (not (judgein >> S))
      judge_error("[Case %d] input broken! S cannot be read", cas);

    if (not (judgein >> E))
      judge_error("[Case %d] input broken! E cannot be read", cas);


    // check some level of validity
    if (not in_interval(a, APMIN, APMAX))
      judge_error("[Case %d] input broken! invalid a: %d", cas, a);

    if (not in_interval(p, APMIN, APMAX))
      judge_error("[Case %d] input broken! invalid p: %d", cas, p);

    if (not is_valid_state(S))
      judge_error("[Case %d] input broken! S=%s", cas, S.c_str());

    if (not is_valid_state(E))
      judge_error("[Case %d] input broken! E=%s", cas, E.c_str());


    // read judge's solution
    int judge_k;
    if (not (judgeans >> judge_k))
      judge_error("[Case %d] model solution broken! k cannot be read", cas);

    if (not (judge_k == -1 or in_interval(judge_k, KMIN, KMAX)))
      judge_error("[Case %d] model solution broken! invalid k: %d", cas, judge_k);

    if (judge_k != -1) {
      for (int opidx = 1; opidx <= judge_k; opidx++) {
        std::string type;
        int index;
        if (not (judgeans >> type))
          judge_error("[Case %d] model solution broken! cannot read operation type", cas);

        if (not (judgeans >> index))
          judge_error("[Case %d] model solution broken! cannot read operation index", cas);
      }
    }


    // read contestant's solution
    int conte_k;
    if (not (conans >> conte_k))
      wrong_answer("[Case %d] k cannot be read", cas);

    if (not (conte_k == -1 or in_interval(conte_k, KMIN, KMAX)))
      wrong_answer("[Case %d] invalid k=%d", cas, conte_k);

    // verify that the contestant gives a sequence if the judge does
    if (judge_k != -1 and conte_k == -1)
      wrong_answer("[Case %d] contestant said there's no answer but there is", cas);

    // verify that the contestant's solution transforms S to E
    // this contains the bulk of the scorer's running time
    if (conte_k != -1) {

      // make string replacement map
      std::map<std::string,std::pair<std::string,std::string>> replacements = {
        {"+P", {"P", "APA"}},
        {"+A", {"A", "PAP"}},
        {"-A", {std::string(a, 'A'), ""}},
        {"-P", {std::string(p, 'P'), ""}},
      };

      // get operations
      assert(conte_k >= 0);
      std::vector<std::pair<std::string,int>> operations(conte_k);
      for (auto &[type, index] : operations) {
        if (not (conans >> type))
          wrong_answer("[Case %d] operation invalid: cannot read type", cas);

        if (not (conans >> index))
          wrong_answer("[Case %d] operation invalid: cannot read index", cas);
      }

      auto check_partial = [&]() {
        // checks that the operations are valid, within bounds,
        // and produce the correct number of A's and P's.

        // precompute replacement string A and P counts
        std::map<std::string,std::pair<int,int>> delta_ap;
        for (const auto &[type, replacement] : replacements) {
          const auto &[source, target] = replacement;
          auto [sacount, spcount] = count_ap(source);
          auto [tacount, tpcount] = count_ap(target);
          delta_ap[type] = {tacount - sacount, tpcount - spcount};
        }

        // initial A and P count
        auto [acount, pcount] = count_ap(S);

        for (const auto &[type, index] : operations) {

          // check that the operation type is valid
          if (not replacements.count(type))
            wrong_answer("[Case %d] operation invalid: type %s invalid", cas, type.c_str());

          const auto &[source, target] = replacements[type];
          int lindex = index - 1; // make 0-indexed
          int rindex = lindex + source.size();
          assert(lindex < rindex);

          // check that the replacement range is valid
          if (not in_interval(lindex, 0, acount + pcount))
            wrong_answer("[Case %d] operation invalid: index %d out of bounds (left)", cas, index+1);

          if (not in_interval(rindex, 0, acount + pcount))
            wrong_answer("[Case %d] operation invalid: index %d out of bounds (right)", cas, index+1);

          // update A and P count
          const auto [adelta, pdelta] = delta_ap[type];
          acount += adelta;
          pcount += pdelta;

          // we should not get negative counts
          if (not (acount >= 0 and pcount >= 0))
            wrong_answer("[Case %d] operation invalid", cas);
        }

        // check that they match the target's A and P counts
        const auto [tacount, tpcount] = count_ap(E);
        if (not (acount == tacount and pcount == tpcount))
          wrong_answer("[Case %d] transformation sequence invalid", cas);
      };

      auto check_full = [&]() {
        // perform the transformation and check that it matches the target string

        dynamic_string curr(S);

        for (const auto &[type, index] : operations) {

          const auto &[source, target] = replacements[type];
          int lindex = index - 1; // make 0-indexed
          int rindex = lindex + source.size();
          assert(lindex < rindex);

          if (source != curr.replace(lindex, rindex, target))
            wrong_answer("[Case %d] operation invalid", cas);
        }

        if (curr.pop() != E)
          wrong_answer("[Case %d] transformation sequence invalid", cas);

        assert(curr.empty());
      };

      // perform rough checks first, so we don't have to do the slow procedure if it doesn't match
      check_partial();

      // now, actually check
      check_full();
    }

    // verify that the judge gives a sequence if the contestant does
    if (conte_k != -1 and judge_k == -1)
      judge_error("[Case %d] model solution broken! judge said there's no answer but contestant found one", cas);
  }

  int main(int argc, char **argv) {
    if(argc < 4) judge_error(USAGE, argv[0]);
    openfile(judgein,  argv[1], argv[0]);
    openfile(judgeans, argv[2], argv[0]);
    openfile(conans,   argv[3], argv[0]);

    // get test case count
    int t;
    if (not (judgein >> t))
      judge_error("input broken! t cannot be read");

    if (not in_interval(t, TMIN, TMAX))
      judge_error("input broken! invalid t: %d", t);

    // process t test cases
    for (int cas = 1; cas <= t; cas++) {
      verify_single_case(cas);
    }

    std::string garbage;
    if (conans >> garbage) {
      wrong_answer("Trailing output:\n%s", garbage.c_str());
    }

    ac();
  }


#endif
