#!/usr/bin/env python

import os
import sys
import timeit

def mix_the_stuffs(data):
    code_result = data
    print(f'Length: {len(code_result)}')
    code_positions = [(e,i) for i, e in enumerate(data)]
    for e, d in enumerate(code_positions):
        new_pos = (d[1] + d[0]) % len(code_result)
        code_positions[e] = (d[0],new_pos)

        for e2, d2 in enumerate(code_positions):
            if d2[0] != d[0] and d2[1] != new_pos and d2[1] >= new_pos:
                code_positions[e2] = (d2[0], d2[1] + 1)
            
        print(d, e, new_pos)
        print(new_pos)

    # for idx, e in enumerate(data):
    #     print(e,idx, e % len(code_result))
    #     print(code_result)
    #     code_fixed = code_result[:idx] + code_result[idx + 1:]
    #     print(code_fixed)
    #     code_result = code_fixed[:e % len(code_result)] + [e] + code_fixed[e % len(code_result):]
    #     print(code_result)
    #     input()
    print(code_positions)
    return code_result


def part1(data: list[str]):
    code_wheel = list(int(x.strip()) for x in data)
    print(code_wheel)
    next_code_wheel = mix_the_stuffs(code_wheel)
    print(next_code_wheel)

def main():
    input_file = "input_test.txt"
    with open(input_file, 'r') as f:
        start = timeit.default_timer()
        part1(f.readlines())
        end = timeit.default_timer()
        print(f'Part1 took {end - start} sec')


if __name__ == "__main__":
    main()
