#!/usr/bin/env python3

"""
Local testing tool for 'Online Assignment'.

Disclaimer: This is *not* the same code used to test your solution when it is
submitted. The purpose of this tool is to help with debugging the interactive
problem and it has no ambitions to extensively test all possibilities that
are allowed by the problem statement. While the tool tries to yield the same
results as the real judging system, this is not guaranteed and the result
may differ if the tested program does not use the correct formatting or
exhibits other incorrect behavior. It also *does not* apply the time and
memory limits that are applied to submitted solutions.

The behavior is controlled by an input data file with a single line containing
five characters representing the correct answers to the questions, where the
i-th character is `t` if the statement of the i-th question is true, or `f` if
the statement of the i-th question is false.

To run the testing tool, run::

    pypy3 testing_tool.py <input file> <program> <arguments>

where `arguments` are optional arguments to the program to run. The following
show examples for different languages::

    pypy3 testing_tool.py test.in ./myprogram
    pypy3 testing_tool.py test.in java -cp . MyProgram
    pypy3 testing_tool.py test.in pypy3 myprogram.py

One can also pass `--verbose` (before the input file name) to see a log of the
complete interaction.
"""

import argparse
import subprocess
import re
import sys
from typing import TextIO


NUM_QUESTIONS = 5
MAX_SUBMISSIONS = 12


class WrongAnswer(RuntimeError):
    """Raised whenever an incorrect answer is received."""
    pass


def vprint(*args, verbose: bool, file: TextIO, **kwargs) -> None:
    """Print to `file`, and also to stdout if `verbose is true."""
    if verbose:
        print('< ', end='')
        print(*args, **kwargs)
        sys.stdout.flush()
    print(*args, file=file, flush=True, **kwargs)


def vreadline(data: TextIO, *, verbose: bool) -> str:
    """Read a line from `data`, and also log it to stdout if `verbose` is true."""
    line = data.readline()
    if verbose and line:
        print('>', line.rstrip('\n'))
    return line


def interact(process: subprocess.Popen, answer: str, *, verbose: bool) -> int:
    submissions = 0
    try:
        while True:
            line = vreadline(process.stdout, verbose=verbose)
            if line == '':
                raise WrongAnswer('End of file received from team program')
            submission = line.strip()
            if len(submission) != NUM_QUESTIONS:
                raise WrongAnswer(f'submission string must have length {NUM_QUESTIONS}')
            if not re.fullmatch('[tf]+', submission):
                raise WrongAnswer('submission string must contain only `t` or `f`')
            correct_answers = 0
            for i in range(NUM_QUESTIONS):
                if answer[i] == submission[i]:
                    correct_answers += 1
            if correct_answers == NUM_QUESTIONS:
                vprint('completed', file=process.stdin, verbose=verbose)
                break
            else:
                vprint(f'rejected {correct_answers}', file=process.stdin, verbose=verbose)
            submissions += 1
            if submissions == MAX_SUBMISSIONS:
                raise WrongAnswer(f'{MAX_SUBMISSIONS}-th submission is still rejected')
    except BrokenPipeError:
        raise WrongAnswer('Error when sending response to team program - possibly exited')
    line = vreadline(process.stdout, verbose=verbose)
    if line.strip() != '':
        raise WrongAnswer('Found extraneous output from team program')
    return submissions


def main() -> int:
    parser = argparse.ArgumentParser(usage='%(prog)s [--verbose] data.in program [args...]')
    parser.add_argument('--verbose', '-v', action='store_true', help='Show interactions')
    parser.add_argument('data')
    parser.add_argument('program', nargs=argparse.REMAINDER)

    args = parser.parse_args()
    if not args.program:
        parser.error('Must specify program to run')

    answer = None
    with open(args.data, 'r') as data:
        answer = data.readline().strip()
        if len(answer) != NUM_QUESTIONS:
            raise ValueError(f'answer string must have length {NUM_QUESTIONS}')
        if not re.fullmatch('[tf]+', answer):
            raise ValueError('answer string must contain only `t` or `f`')

    # max_queries = 0
    submissions = 0
    process = subprocess.Popen(args.program, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                               encoding='utf-8', errors='surrogateescape')
    try:
        try:
            submissions = interact(process, answer, verbose=args.verbose)
        except WrongAnswer as exc:
            print('Wrong answer ({})'.format(exc))
            return 1
        process.wait()
    finally:
        if process.poll() is None:
            try:
                process.terminate()
            except ProcessLookupError:  # Should be impossible, but just to be safe
                pass
        process.wait()
    if process.returncode < 0:
        print(f'Run-time error (process exited with signal {-process.returncode})')
        return 1
    elif process.returncode > 0:
        print(f'Run-time error (process exited with status {process.returncode})')
        return 1
    else:
        print(f'Accepted (submissions = {submissions})')
        return 0


if __name__ == '__main__':
    sys.exit(main())
