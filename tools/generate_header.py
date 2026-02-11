#!/usr/bin/env python3
import argparse
import os
import sys

dirs = []
included_files = set()
output = "#pragma once\n"

def find_file(file, from_dir):
    def rel_dirs():
        if from_dir is None:
            return dirs
        else:
            return [from_dir] + dirs

    if os.path.isabs(file):
        if os.path.isfile(file):
            return os.path.normpath(file) 
    else:
        for d in rel_dirs():
            path = os.path.join(d, file)
            if os.path.isfile(path):
                return os.path.normpath(path)

    if from_dir is None:
        print(f"Warning: File <{file}> not found.", file=sys.stderr)
        return None

    raise FileNotFoundError(f"File {file} not found")

def include_file(file, from_dir):
    global output
    global included_files

    abs_file = find_file(file, from_dir)

    if abs_file is None:
        output += f"#include <{file}>\n"
        return

    if abs_file in included_files:
        return
    included_files.add(abs_file)

    with open(abs_file, "r", encoding="utf-8") as f:
        for line in f:
            stripped = line.strip()
            if stripped.startswith('#pragma once'):
                continue
            if stripped.startswith('#include'):
                start = stripped.find('"')
                end = stripped.rfind('"')
                relative = True
                if start == -1 or end == -1:
                    start = stripped.find('<')
                    end = stripped.rfind('>')
                    relative = False

                if start != -1 and end != -1 and end > start:
                    inc_name = stripped[start + 1 : end]
                    if relative:
                        include_file(inc_name, os.path.dirname(abs_file))
                    else:
                        include_file(inc_name, None)
                else:
                    output += line
            else:
                output += line

def main():
    global output
    global dirs

    p = argparse.ArgumentParser()
    p.add_argument('-o', '--output')
    p.add_argument('-I', dest='dirs', action='append', default=[])
    p.add_argument('headers', nargs='+')
    args = p.parse_args()

    dirs = [os.path.abspath(d) for d in args.dirs]

    for hdr in args.headers:
        include_file(hdr, os.path.abspath("."))

    if args.output:
        with open(args.output, "w", encoding="utf-8") as out_f:
            out_f.write(output)
    else:
        print(output)

if __name__ == '__main__':
    main()

