#!/usr/bin/env python3
"""
diffwatch.py — Unified diff of two files, line-number annotated.

Usage:
    python3 diffwatch.py OLD NEW [--context N]
    python3 diffwatch.py compiler_passes.c.pre_patch compiler_passes.c
    python3 diffwatch.py compiler_passes.c.pre_patch compiler_passes.c --context 10

Also useful for comparing AST vs IR assembly:
    python3 diffwatch.py zcc_ast_ref.s zcc2.s --context 5 --section is_digit

Options:
    --context N      Lines of context around each change (default: 3)
    --section NAME   Only show diffs within the assembly function NAME:
                     (extracts from 'NAME:' label to next bare word label)
"""

import sys
import difflib
import re


def extract_section(lines, name):
    """Extract lines from 'name:' label to next bare function label."""
    out = []
    inside = False
    label_re = re.compile(r'^[a-zA-Z_][a-zA-Z0-9_]*:')
    for line in lines:
        if line.rstrip() == f'{name}:':
            inside = True
        elif inside and label_re.match(line) and line.rstrip() != f'{name}:':
            break
        if inside:
            out.append(line)
    return out


def main():
    args = sys.argv[1:]
    if len(args) < 2:
        print(__doc__)
        sys.exit(1)

    old_path = args[0]
    new_path = args[1]
    context = 3
    section = None

    i = 2
    while i < len(args):
        if args[i] == '--context' and i + 1 < len(args):
            context = int(args[i + 1])
            i += 2
        elif args[i] == '--section' and i + 1 < len(args):
            section = args[i + 1]
            i += 2
        else:
            i += 1

    old_lines = open(old_path, 'r').readlines()
    new_lines = open(new_path, 'r').readlines()

    if section:
        old_lines = extract_section(old_lines, section)
        new_lines = extract_section(new_lines, section)
        if not old_lines and not new_lines:
            print(f"Section '{section}' not found in either file.")
            sys.exit(1)

    diff = list(difflib.unified_diff(
        old_lines, new_lines,
        fromfile=f'BEFORE  {old_path}',
        tofile=f'AFTER   {new_path}',
        n=context,
        lineterm=''
    ))

    if not diff:
        print(f"No differences between {old_path} and {new_path}")
        if section:
            print(f"(section: {section})")
        sys.exit(0)

    # Colorize if terminal supports it
    use_color = sys.stdout.isatty()
    RED = '\033[31m' if use_color else ''
    GREEN = '\033[32m' if use_color else ''
    CYAN = '\033[36m' if use_color else ''
    RESET = '\033[0m' if use_color else ''

    for line in diff:
        if line.startswith('---') or line.startswith('+++'):
            print(f"{CYAN}{line}{RESET}")
        elif line.startswith('@@'):
            print(f"{CYAN}{line}{RESET}")
        elif line.startswith('-'):
            print(f"{RED}{line}{RESET}")
        elif line.startswith('+'):
            print(f"{GREEN}{line}{RESET}")
        else:
            print(line)

    # Stats
    adds = sum(1 for l in diff if l.startswith('+') and not l.startswith('+++'))
    dels = sum(1 for l in diff if l.startswith('-') and not l.startswith('---'))
    print(f"\n{GREEN}+{adds}{RESET} / {RED}-{dels}{RESET} lines changed")


if __name__ == '__main__':
    main()
