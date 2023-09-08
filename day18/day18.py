#!/usr/bin/env python

import timeit
from collections.abc import Generator

CubeEntity = tuple[int, int, int]
CubeContainer = set[CubeEntity]

to_check: CubeContainer = set()
checked: CubeContainer = set()


def apply_offset(cube: CubeEntity) -> Generator[CubeEntity | tuple[int, ...], None, None]:
    offsets = [(1, 0, 0),
               (-1, 0, 0),
               (0, 1,  0),
               (0, -1, 0),
               (0, 0,  1),
               (0, 0, -1)]
    for off in offsets:
        yield tuple((sum(x) for x in zip(cube, off)))


def part1(cubes: set[tuple[int, int, int]]) -> None:
    total_sides = sum((sum(map(lambda c: 0 if c in cubes else 1, apply_offset(cube))) for cube in cubes))

    print(f'part1: {total_sides}')


def is_cube(cube, cubes):
    return cube in cubes


def expand_all(cubes, airs):
    while len(to_check) >= 1:
        check_position = to_check.pop()
        if check_position in checked:
            continue
        checked.add(check_position)
        if is_cube(check_position, cubes):
            continue
        if check_position not in airs:
            airs.add(check_position)
        for positions in apply_offset(check_position):
            if positions not in cubes and positions not in airs and positions not in checked:
                to_check.add(positions)


def count_sides_next_to_air(cube, airs):
    return sum(map(lambda c: 1 if c in airs else 0, apply_offset(cube)))


def part2(cubes: set[tuple[int, int, int]]) -> None:
    max_x = 0
    min_x = 10000
    max_y = 0
    min_y = 10000
    max_z = 0
    min_z = 10000

    for c in cubes:
        max_x = max(c[0] + 1, max_x)
        min_x = min(c[0] - 1, min_x)
        max_y = max(c[1] + 1, max_y)
        min_y = min(c[1] - 1, min_y)
        max_z = max(c[2] + 1, max_z)
        min_z = min(c[2] - 1, min_z)

    air: CubeContainer = set()

    def track_outlining_air(position):
        # if not is_cube(position, cubes):
        air.add(position)
        checked.add(position)

    for y in range(min_y, max_y + 1):
        for z in range(min_z, max_z + 1):
            track_outlining_air((min_x, y, z))
            track_outlining_air((max_x, y, z))
        for x in range(min_x, max_x + 1):
            track_outlining_air((x, y, min_z))
            track_outlining_air((x, y, max_z))

    for x in range(min_x, max_x + 1):
        for z in range(min_z, max_z + 1):
            track_outlining_air((x, min_y, z))
            track_outlining_air((x, max_y, z))

    def reached_cube_or_untracked_air(position):
        if is_cube(position, cubes):
            return True

        if position not in air and position not in checked:
            to_check.add(position)
            return True

        return False

    for x in range(min_x + 1, max_x - 1):
        for y in range(min_y + 1, max_y - 1):
            for z in range(min_z + 1, max_z - 1):
                position = (x, y, z)
                if reached_cube_or_untracked_air(position):
                    break
            for z in range(max_z - 1, min_z, -1):
                position = (x, y, z)
                if reached_cube_or_untracked_air(position):
                    break

        for z in range(min_z + 1, max_z - 1):
            for y in range(min_y + 1, max_y - 1):
                position = (x, y, z)
                if reached_cube_or_untracked_air(position):
                    break
            for y in range(max_y - 1, min_y, -1):
                position = (x, y, z)
                if reached_cube_or_untracked_air(position):
                    break

    for y in range(min_y + 1, max_y - 1):
        for z in range(min_z + 1, max_z - 1):
            for x in range(min_x + 1, max_x - 1):
                position = (x, y, z)
                if reached_cube_or_untracked_air(position):
                    break
            for x in range(max_x - 1, min_x, -1):
                position = (x, y, z)
                if reached_cube_or_untracked_air(position):
                    break

    expand_all(cubes, air)

    print("Part2:", sum((count_sides_next_to_air(x, air) for x in cubes)))


def main():
    input_file = "input.txt"
    cubes: set[tuple[int, int, int]] = set()

    with open(input_file, 'r') as f:
        cubes = set(((int(z[0]), int(z[1]), int(z[2])) for z in map(lambda v: v.split(','), [x.strip() for x in f.readlines()])))

    start = timeit.default_timer()
    part1(cubes)
    end = timeit.default_timer()
    print(f'Part1 took {end - start} sec')

    start = timeit.default_timer()
    part2(cubes)
    end = timeit.default_timer()
    print(f'Part2 took {end - start} sec')
    # with open(input_file, 'r') as f:
    #     start = timeit.default_timer()
    #     part2(f.readlines())
    #     end = timeit.default_timer()
    #     print(f'Part2 took {end - start} sec')


if __name__ == "__main__":
    main()
