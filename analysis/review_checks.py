import argparse
import heapq
import os
import subprocess
from pathlib import Path
from tempfile import TemporaryDirectory

from verify import check_output


def run(binary, commands, verify=True):
    with TemporaryDirectory() as directory:
        path = Path(directory) / "case.txt"
        path.write_text(commands.strip() + "\n", encoding="ascii")
        proc = subprocess.run([str(binary), str(path)], capture_output=True, text=True, check=True)
        assert not proc.stderr, proc.stderr
        if verify:
            check_output(path, proc.stdout)
        return proc.stdout.rstrip()


def validate(commands, output):
    with TemporaryDirectory() as directory:
        file = Path(directory) / "case.txt"
        file.write_text(commands.lstrip(), encoding="ascii")
        check_output(file, output)


MAP = """N 6 0
B 0 1 3
B 3 4 8
regions
hops 0 4
B 1 3 2
regions
hops 0 4
B 4 5 2
regions
hops 0 5
"""
FOREST = """N 5 0
B 0 1 0
B 1 2 0
B 0 2 0
B 2 3 0
regions
maintain prim
maintain kruskal
"""
TWO_CYCLES = """N 6 0
R 0 1 0
R 0 3 0
R 1 2 -4
R 2 1 1
R 3 4 -5
R 4 3 1
stamina 0 5
N 6 0
R 4 3 1
R 3 4 -5
R 2 1 1
R 1 2 -4
R 0 3 0
R 0 1 0
stamina 0 5
"""
ZERO_QUESTS = """N 1 4
P 0 1
P 0 2
P 1 3
P 2 3
unlock 3
N 1 3
Q 2 6
P 0 1
P 1 2
unlock 1
unlock 2
"""
TIE = """N 1 4
P 1 2
P 0 2
P 2 3
unlock 3
"""
LATER = """N 1 4
Q 1 5
Q 3 4
P 0 2
P 1 2
P 2 3
unlock 2
unlock 3
"""


def map_checks(binary, before=False):
    print("Empty bridge graph and source-to-self")
    print(run(binary, "N 4 0\nregions\nhops 1 1\nhops 1 2\n"))
    print("Bridge insertions")
    print(run(binary, MAP))


def forest_checks(binary, before=False):
    clean = run(binary, FOREST)
    print("Actual forest witnesses")
    print(clean)
    if before:
        return
    rows = clean.splitlines()

    u, v, _ = rows[4].split()
    for label, bad in (
        ("wrong weight", [*rows[:4], f"{u} {v} 1", *rows[5:]]),
        ("nonexistent bridge", [*rows[:4], "0 4 0", *rows[5:]]),
        ("reordered regions", [rows[0], rows[2], rows[1], *rows[3:]]),
        ("merged regions", ["regions 1", "0 1 2 3 4", *rows[3:]]),
        ("same-count cycle", [*rows[:4], "0 1 0", "0 2 0", "1 2 0", *rows[7:]]),
    ):
        try:
            validate(FOREST, "\n".join(bad))
        except (AssertionError, KeyError):
            print(f"Rejected {label}")
        else:
            raise AssertionError(f"Accepted {label}")

    alternate = [*rows[:4], "0 2 0", "1 2 0", "2 3 0", *rows[7:]]
    validate(FOREST, "\n".join(alternate))
    print("Accepted a distinct optimum witness 0-2, 1-2, 2-3")


def route_checks(binary, before=False):
    print("Reachable directed cycles under two input orders")
    print(run(binary, TWO_CYCLES))
    edges = [(0, 2, -1), (1, 0, 1), (1, 2, -3), (2, 1, 1)]
    dist, parent = [0, float("inf"), float("inf")], [-1] * 3
    for _ in range(2):
        for u, v, cost in edges:
            if dist[u] + cost < dist[v]:
                dist[v], parent[v] = dist[u] + cost, u
    assert dist == [0, -2, -3]
    u, v, cost = next(e for e in edges if dist[e[0]] + e[2] < dist[e[1]])
    assert (u, v) == (1, 0) and parent[v] == -1
    print(f"Parentless read-only probe from 1 to 0, labels {dist}")
    print(
        run(binary, "N 3 0\n" + "".join(f"R {u} {v} {w}\n" for u, v, w in edges) + "stamina 0 2\n")
    )
    print("Unreachable negative cycle stays invisible")
    print(run(binary, "N 4 0\nR 0 1 4\nR 2 3 -2\nR 3 2 1\nstamina 0 1\nstamina 0 3\n"))
    if not before:
        rows = [[(1, 5), (3, 2)], [(2, -5)], [(3, 1), (4, 0)], [], [(3, 0)]]
        dist = [0] + [float("inf")] * 4
        queue, improved, expanded = [(0, 0)], [], []
        while queue:
            value, u = heapq.heappop(queue)
            if value != dist[u]:
                continue
            if u == 3:
                expanded.append(value)
            for v, w in rows[u]:
                if value + w < dist[v]:
                    dist[v] = value + w
                    heapq.heappush(queue, (dist[v], v))
                    if v == 3:
                        improved.append(dist[v])
        assert improved == [2, 1, 0] and expanded == [2, 0]
        print(f"Heap trace, target labels {improved}; actual expansions {expanded}")


def schedule_checks(binary, before=False, mutant=None):
    print("All-zero diamond and zero prefix")
    print(run(binary, ZERO_QUESTS, verify=not before))
    print("Disconnected cycle and positive chain")
    print(
        run(binary, "N 1 5\nQ 0 3\nQ 1 2\nQ 2 4\nP 0 1\nP 1 2\nunlock 2\nP 3 4\nP 4 3\nunlock 2\n")
    )
    if before:
        return
    output = run(binary, TIE)
    print(f"Equal source alternatives, program: {output}")
    other = 1 - int(output.split()[3])
    alternate = f"unlock 3 0 {other} 2 3"
    validate(TIE, alternate)
    print(f"Independent checker accepted alternate: {alternate}")
    print("Later larger candidate")
    print(run(binary, LATER))
    print("Quest dependencies inserted between queries")
    print(run(binary, "N 1 3\nQ 0 2\nQ 1 3\nQ 2 4\nP 0 2\nunlock 2\nP 1 2\nunlock 2\n"))
    if mutant:
        output = run(mutant, LATER, verify=False)
        try:
            validate(LATER, output)
        except AssertionError:
            print("Rejected first-only mutation")
            print(output)
        else:
            raise AssertionError("The later-improvement test missed the mutation")


def check_quest_mutation(binary):
    with TemporaryDirectory() as directory:
        area = Path(directory)
        source = Path("src/quests.cpp").read_text()
        old = "schedule.critical[v] == -1 || finish > schedule.finish[v]"
        assert source.count(old) == 1
        variant = area / "quests.cpp"
        variant.write_text(source.replace(old, "schedule.critical[v] == -1"))
        files = [
            str(path) for path in sorted(Path("src").glob("*.cpp")) if path.name != variant.name
        ]
        executable = area / ("first-only.exe" if os.name == "nt" else "first-only")
        subprocess.run(
            [
                "g++",
                "-std=c++23",
                "-O2",
                "-DNDEBUG",
                "-Iinclude",
                "-Isrc",
                *files,
                str(variant),
                "-o",
                str(executable),
            ],
            check=True,
        )
        schedule_checks(binary, mutant=executable)


def main():
    parser = argparse.ArgumentParser(
        description="Run the small, claim-specific checks beside the general command oracle."
    )
    parser.add_argument("binary", type=Path)
    parser.add_argument(
        "role", choices=("all", "mira", "jonah", "nia", "leo"), default="all", nargs="?"
    )
    parser.add_argument("--draft", action="store_true")
    parser.add_argument("--mutate-quest", action="store_true")
    args = parser.parse_args()
    if args.mutate_quest and args.draft:
        parser.error("--mutate-quest requires the completed quest rule")
    binary = args.binary.resolve()
    headings = {
        "mira": "Map adjacency and queries",
        "jonah": "Minimum spanning forests",
        "nia": "Shortest paths and negative cycles",
        "leo": "Quest and event scheduling",
    }

    for role, check in [
        ("mira", map_checks),
        ("jonah", forest_checks),
        ("nia", route_checks),
        ("leo", schedule_checks),
    ]:
        if args.role in (role, "all"):
            print(headings[role])
            if role == "leo" and args.mutate_quest:
                check_quest_mutation(binary)
            else:
                check(binary, args.draft)


if __name__ == "__main__":
    main()
