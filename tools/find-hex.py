#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Tommy McLeroy. Part of cps2recomp.
#
# Finds the lines of the given files that match a pattern, the way the audit needs it. The audit uses
# this instead of grep for the patterns that need the {n,} repetition: the grep of a Windows shell
# does not know it, and takes the pattern for the name of a file (so the audit would find nothing, and
# say so that it did).
#
#   tools/find-hex.py <pattern> <file>...
#   tools/find-hex.py --names <pattern> <file>...
#   tools/find-hex.py --pattern-file <file> <listfile>
#
# The pattern is a Python regular expression, so {n} and {n,} are written as they are elsewhere.
# With --names, the pattern is matched against each name, and the name is printed when it matches
# (the audit looks for files of a kind that may not be in the tree).
# With --pattern-file, the pattern is read from the given file and the files to search are read from
# the other, one name to a line: a shell's xargs may change a pattern that has the characters it
# treats as its own.
import re
import sys


def main() -> int:
    args = sys.argv[1:]
    names = False
    pattern_file = None
    if args and args[0] == "--names":
        names = True
        args = args[1:]
    elif args and args[0] == "--pattern-file":
        pattern_file = args[1]
        args = args[2:]
    if len(args) < 2 and pattern_file is None:
        print("usage: find-hex.py [--names|--pattern-file <file>] <pattern> <file>...", file=sys.stderr)
        return 2
    text = ""
    if pattern_file is not None:
        try:
            with open(pattern_file, encoding="utf-8") as f:
                text = f.read().strip()
        except OSError as error:
            print(f"find-hex.py: cannot read the pattern: {error}", file=sys.stderr)
            return 2
    else:
        text = args[0]
        args = args[1:]
    try:
        pattern = re.compile(text)
    except re.error as error:
        print(f"find-hex.py: the pattern is not a regular expression: {error}", file=sys.stderr)
        return 2
    # (the console of Windows may not have the characters of a file: they are put as they are)
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except AttributeError:
        pass
    if names:
        for name in args:
            if pattern.search(name):
                print(name)
        return 0
    if pattern_file is not None:
        try:
            with open(args[0], encoding="utf-8") as f:
                args = [line.strip() for line in f if line.strip()]
        except OSError as error:
            print(f"find-hex.py: cannot read the list of files: {error}", file=sys.stderr)
            return 2
    for name in args:
        try:
            with open(name, "rb") as f:
                data = f.read()
        except OSError:
            continue
        # (a picture or a sound is not a text: the bytes in it would match by chance)
        if b"\0" in data[:8192]:
            continue
        text = data.decode("utf-8", "replace")
        for number, line in enumerate(text.splitlines(), 1):
            if pattern.search(line):
                print(f"{name}:{number}:{line}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
