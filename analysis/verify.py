import itertools
import math
import random
import subprocess
import sys
import tempfile
from pathlib import Path


def distances(n, arcs):
    d = [[math.inf] * n for _ in range(n)]
    for u in range(n):
        d[u][u] = 0
    for (u, v), w in arcs.items():
        d[u][v] = w
    for k in range(n):
        for u in range(n):
            for v in range(n):
                d[u][v] = min(d[u][v], d[u][k] + d[k][v])
    return d


def groups(n, bridges):
    labels = list(range(n))
    for a, b in bridges:
        first, second = labels[a], labels[b]
        labels = [first if label == second else label for label in labels]
    return sorted([i for i in range(n) if labels[i] == label] for label in set(labels))


def feasible(events, ids):
    ordered = sorted(events[i][:2] for i in ids)
    return all(a[1] <= b[0] for a, b in zip(ordered, ordered[1:]))


def minimum_on_every_cut(n, bridges, chosen):

    adj = [[] for _ in range(n)]
    for a, b in chosen:
        adj[a].append(b)
        adj[b].append(a)
    for a, b in chosen:
        side = {a}
        queue = [a]
        for u in queue:
            for v in adj[u]:
                if (u, v) == (a, b) or v in side:
                    continue
                side.add(v)
                queue.append(v)
        if any(w < bridges[a, b] and ((x in side) != (y in side)) for (x, y), w in bridges.items()):
            return False
    return True


def verify(exe, path):
    result = subprocess.run([exe, str(path)], text=True, capture_output=True, check=True)
    assert not result.stderr, result.stderr
    check_output(path, result.stdout)
    return result.stdout


def check_output(path, stdout):
    out = iter(stdout.splitlines())
    for command in path.read_text().splitlines():
        if not command.strip() or command.startswith("#"):
            continue
        tag, *tokens = command.split()
        if tag == "N":
            n, k = map(int, tokens)
            bridges, sea, events, pre, duration = {}, {}, [], [], [0] * k
        elif tag in ("R", "B"):
            u, v, w = map(int, tokens)
            if tag == "B":
                bridges[tuple(sorted((u, v)))] = w
            else:
                sea[u, v] = w
        elif tag == "E":
            events.append(tuple(map(int, tokens)))
        elif tag == "Q":
            q, hours = map(int, tokens)
            duration[q] = hours
        elif tag == "P":
            pre.append(tuple(map(int, tokens)))
        else:
            head, *answer = next(out).split()
            assert head == tag
            if tag == "regions":
                components = groups(n, bridges)
                assert int(answer[0]) == len(components)
                assert [list(map(int, next(out).split())) for _ in components] == components
            elif tag == "maintain":
                total, count = map(int, answer)
                components = groups(n, bridges)
                assert count == n - len(components)
                chosen = []
                for _ in range(count):
                    u, v, w = map(int, next(out).split())
                    key = tuple(sorted((u, v)))
                    assert bridges[key] == w
                    chosen.append(key)
                assert len(set(chosen)) == count
                assert groups(n, chosen) == components
                assert total == sum(bridges[e] for e in chosen)
                assert minimum_on_every_cut(n, bridges, chosen)
                if math.comb(len(bridges), count) <= 10000:
                    optimum = min(
                        sum(bridges[e] for e in subset)
                        for subset in itertools.combinations(bridges, count)
                        if groups(n, subset) == components
                    )
                    assert total == optimum
            elif tag in ("hops", "fastest", "stamina"):
                s, t = map(int, tokens)
                assert answer[:2] == tokens
                arcs = dict(sea)
                if tag != "stamina":
                    arcs = {
                        (a, b): 1 if tag == "hops" else w
                        for (u, v), w in bridges.items()
                        for a, b in ((u, v), (v, u))
                    }
                d = distances(n, arcs)
                if any(d[s][u] < math.inf and d[u][u] < 0 for u in range(n)):
                    assert answer[2] == "exploit"
                    cycle = list(map(int, answer[3:]))
                    assert len(cycle) >= 3 and len(set(cycle[:-1])) == len(cycle) - 1
                    assert cycle[0] == cycle[-1] and d[s][cycle[0]] < math.inf
                    assert sum(arcs[e] for e in zip(cycle, cycle[1:])) < 0
                elif d[s][t] == math.inf:
                    assert answer[2:] == ["unreachable"]
                else:
                    assert int(answer[2]) == d[s][t]
                    if tag == "hops":
                        assert len(answer) == 3
                    else:
                        route = list(map(int, answer[3:]))
                        assert route[0] == s and route[-1] == t
                        assert sum(arcs[e] for e in zip(route, route[1:])) == d[s][t]
            elif tag == "unlock":
                q = int(tokens[0])
                assert answer[0] == tokens[0]

                def chains(v, tail=()):
                    if v in tail:
                        raise ValueError("cycle")
                    incoming = [u for u, w in pre if w == v]
                    if not incoming:
                        yield (v,) + tail
                    for u in incoming:
                        yield from chains(u, (v,) + tail)

                try:
                    options = [list(chains(v)) for v in range(k)]
                except ValueError:
                    assert answer[1:] == ["cycle"]
                else:
                    chain = tuple(map(int, answer[2:]))
                    score = lambda c: sum(duration[v] for v in c)
                    assert chain in options[q] and int(answer[1]) == score(chain) == max(
                        map(score, options[q])
                    )
            else:
                ids = list(map(int, answer[1:]))
                assert ids == sorted(set(ids)) and all(0 <= i < len(events) for i in ids)
                assert feasible(events, ids)
                score = len if tag == "most" else lambda s: sum(events[i][2] for i in s)
                optimum = max(
                    score(s)
                    for size in range(len(events) + 1)
                    for s in itertools.combinations(range(len(events)), size)
                    if feasible(events, s)
                )
                assert int(answer[0]) == score(ids) == optimum
    assert next(out, None) is None


def random_world(rng, trial):
    n = rng.randint(2, 6)
    lines = [f"N {n} {n}"]
    bridges = list(itertools.combinations(range(n), 2))
    rng.shuffle(bridges)
    for u, v in bridges[: rng.randint(0, min(9, len(bridges)))]:
        w = rng.randrange(6)
        lines.append(f"B {u} {v} {w}")
        if trial % 2 == 0:
            lines += [f"R {u} {v} {w}", f"R {v} {u} {w}"]
    for u, v in itertools.permutations(range(n), 2):
        if trial % 2 and rng.random() < 0.32:
            lines.append(f"R {u} {v} {rng.randint(-5, 7)}")
        if (u < v or trial % 3 == 0) and rng.random() < 0.23:
            lines.append(f"P {u} {v}")
    for q in range(n):
        lines.append(f"Q {q} {rng.randrange(5)}")
    for _ in range(rng.randrange(10)):
        s = rng.randrange(8)
        lines.append(f"E {s} {s + rng.randint(1, 5)} {1 if trial % 2 == 0 else rng.randint(1, 9)}")
    lines += ["regions", "maintain prim", "maintain kruskal", "most", "best"]
    for v in range(n):
        lines += [f"unlock {v}", f"hops 0 {v}", f"fastest 0 {v}", f"stamina 0 {v}"]
    return "\n".join(lines) + "\n"


if __name__ == "__main__":
    binary = str(Path(sys.argv[1]).resolve())
    files = sorted([*Path("tests").glob("*.txt"), *Path("counterexamples").glob("*.txt")])
    for f in files:
        verify(binary, f)
    rng = random.Random(2183)
    with tempfile.TemporaryDirectory() as tmp:
        f = Path(tmp) / "world.txt"
        for trial in range(200):
            f.write_text(random_world(rng, trial))
            verify(binary, f)
    print(
        f"Passed {len(files)} command files and 200 random worlds, with independent witness checks."
    )
