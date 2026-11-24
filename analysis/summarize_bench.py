import csv
from collections import defaultdict
from math import ceil
from pathlib import Path
from statistics import mean, median

RESULTS = Path("results")
OUTPUT = Path("build")


def load(name):
    with (RESULTS / f"{name}.csv").open(newline="") as f:
        return list(csv.DictReader(f))


def write(name, header, rows):
    with (OUTPUT / f"{name}.csv").open("w", newline="") as f:
        out = csv.writer(f)
        out.writerow(header)
        out.writerows(rows)


def paired():
    rows = load("timings")
    by_method = defaultdict(dict)
    for r in rows:
        key = (r["family"], int(r["n"]), r["method"])
        by_method[key][int(r["seed"])] = r
    setup = []
    for f, n, m in by_method:
        base = (
            "Kruskal"
            if f in ("sparse", "dense", "grid")
            else "Dijkstra"
            if f.startswith("route-")
            else "both"
        )
        if m == base:
            continue
        sample = by_method[f, n, m]
        ref = by_method[f, n, base]
        assert set(sample) == set(ref) and len(sample) == 5
        assert all(sample[s]["m"] == ref[s]["m"] for s in sample)
        ratios = [float(sample[s]["ms"]) / float(ref[s]["ms"]) for s in sample]
        ticks = [float(r["ms"]) for r in sample.values()]
        ref_ticks = [float(r["ms"]) for r in ref.values()]
        metrics = [float(r["metric"]) for r in sample.values()]
        ref_metrics = [float(r["metric"]) for r in ref.values()]
        root = [float(r["root_links_per_find"]) for r in sample.values()]
        ref_root = [float(r["root_links_per_find"]) for r in ref.values()]
        setup.append(
            [
                f,
                n,
                m,
                base,
                len(sample),
                median(ratios),
                min(ratios),
                max(ratios),
                median(ticks),
                median(ref_ticks),
                median(metrics),
                median(ref_metrics),
                median(root),
                median(ref_root),
            ]
        )
    write(
        "paired_summary",
        [
            "family",
            "n",
            "method",
            "baseline",
            "seeds",
            "ratio",
            "low",
            "high",
            "ms",
            "baseline_ms",
            "metric",
            "baseline_metric",
            "root_links",
            "baseline_root_links",
        ],
        sorted(setup),
    )
    return setup


def layouts():
    samples = defaultdict(list)
    for r in load("layout_epochs"):
        k = r["graph"], r["phase"], r["variant"], int(r["seed"])
        samples[k].append(float(r["ms"]))
    result = []
    for g, p, v, s in sorted(samples):
        if s != 2183:
            continue
        base = "all" if p == "heap" else "plain"
        ratios = [
            median(samples[g, p, v, i]) / median(samples[g, p, base, i]) for i in (2183, 6211, 9011)
        ]
        result.append(
            [
                g,
                p,
                v,
                median(ratios),
                min(ratios),
                max(ratios),
                median(median(samples[g, p, v, i]) for i in (2183, 6211, 9011)),
            ]
        )
    write("layout_summary", ["graph", "phase", "variant", "ratio", "low", "high", "ms"], result)


def event_ratios():
    samples = defaultdict(list)
    for row in load("events"):
        for rule in ("reward", "finish", "density"):
            samples[row["model"], int(row["n"]), rule].append(int(row[rule]) / int(row["optimal"]))
    assert len(samples) == 18 and all(len(values) == 1000 for values in samples.values())
    write(
        "ratios",
        ["model", "n", "rule", "instances", "mean", "minimum"],
        [
            (*key, len(values), f"{mean(values):.6f}", f"{min(values):.6f}")
            for key, values in sorted(samples.items())
        ],
    )


def plot_forest(rows):
    from PIL import Image, ImageDraw, ImageFont

    def font(size, bold=False):
        for path in (
            ("C:/Windows/Fonts/calibrib.ttf", "DejaVuSans-Bold.ttf")
            if bold
            else ("C:/Windows/Fonts/calibri.ttf", "DejaVuSans.ttf")
        ):
            try:
                return ImageFont.truetype(path, size)
            except OSError:
                pass
        return ImageFont.load_default(size=size)

    families = [("sparse", "#075e57"), ("grid", "#42677c"), ("dense", "#9e5323")]
    right = max(8, 2 * ceil(max(r[7] for r in rows if r[0] in dict(families)) / 2))
    image = Image.new("RGB", (1380, 345), "white")
    draw = ImageDraw.Draw(image)
    for i, (family, color) in enumerate(families):
        data = sorted((r for r in rows if r[0] == family), key=lambda r: r[1])
        start = 30 + 455 * i
        zero = start + 105
        dx = 305 / right
        draw.text((start, 4), family.capitalize(), font=font(32, True), fill="#172d35")
        draw.line((zero + dx, 47, zero + dx, 299), fill="#566570", width=2)
        for j, r in enumerate(data):
            _, n, _, _, _, ratio, low, high, *_ = r
            y = 79 + j * 60
            draw.text((start, y), f"{n:,}", font=font(23), fill="#222222", anchor="lm")
            draw.line((zero + low * dx, y, zero + high * dx, y), fill=color, width=4)
            for endpoint in (low, high):
                x = zero + endpoint * dx
                draw.line((x, y - 7, x, y + 7), fill=color, width=3)
            x = zero + ratio * dx
            draw.ellipse((x - 7, y - 7, x + 7, y + 7), fill=color)
        for value in (0, 1, *range(2, right + 1, 2)):
            x = zero + value * dx
            draw.line((x, 300, x, 306), fill="#566570", width=1)
            draw.text((x, 329), str(value), font=font(20), fill="#384751", anchor="ms")
    image.save(OUTPUT / "mst-ratios.png")


if __name__ == "__main__":
    OUTPUT.mkdir(exist_ok=True)
    data = paired()
    layouts()
    event_ratios()
    plot_forest(data)
