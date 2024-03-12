#include <bits/stdc++.h>
using namespace std;

constexpr int kNumQuestion = 5;
constexpr int kMaxSubmission = 12;
constexpr char kTrue = 't';
constexpr char kFalse = 'f';
const string kRejectedFeedback = "rejected";
const string kCompletedFeedback = "completed";

void ac() {
  cerr << "AC" << endl;
  exit(0); 
}

void wa() {
  cerr << "WA" << endl;
  exit(0);
}

void wrong_answer(const char *err, ...) {
  (void)err;  // error message is not used for now.
  wa();
}

void judge_error(const char *err, ...) {
  (void)err;  // error message is not used for now.
  assert(!"Judge Error");
}

int main(int argc, char* argv[]) {
  ifstream tc_in(argv[1]);
  // out = ofstream(argv[2]);
  
  string S;
  tc_in >> S;
  if (S.size() != kNumQuestion) {
    judge_error("Input is invalid: length expected=%d but found=%d",
                kNumQuestion, S.size());
  }
  for (char c : S) {
    if (c != kTrue && c != kFalse) {
      judge_error("Input is invalid: invalid character found=%c", c);
    }
  }

  for (int i = 0; i < kMaxSubmission; ++i) {
    string sub;
    if (!(cin >> sub)) {
      wrong_answer("Contestant did not make any submission");
    }
    if (sub.size() != kNumQuestion) {
      wrong_answer("Submission is invalid: length expected=%d but found=%d",
                   kNumQuestion, S.size());
    }
    for (char c : S) {
      if (c != kTrue && c != kFalse) {
        wrong_answer("Submission is invalid: invalid character found=%c", c);
      }
    }
    int correct = 0;
    for (int j = 0; j < kNumQuestion; ++j) {
      if (S[j] == sub[j]) {
        ++correct;
      }
    }
    if (correct == kNumQuestion) {
      cout << kCompletedFeedback << endl;
      string buffer;
      if (cin >> buffer) {
        wrong_answer("Found extraneous output: %s", buffer.c_str());
      }
      ac();
    } else {
      cout << kRejectedFeedback << " " << correct << endl;
    }
  }

  wrong_answer(
      "Contestant did not make correct submission after %d submissions",
      kMaxSubmission);
}
