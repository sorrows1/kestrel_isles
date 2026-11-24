# Kestrel Isles four-role exemplar

Codex authored this post-project study. Mira, Jonah, Nia and Leo are fictional work roles. The study begins from the course starter dcf4e78. The integrated program preserves the behavior of master at 077adeb.

The complete project and Word report are on team-study/leo-scheduling. The other three branches preserve their scoped histories. See [roles and provenance](study/roles.md), the [log index](ai_logs/README.md), and [executed evidence](study/evidence.md).

## Verification

Use GCC with C++23, GNU make and Python 3. On Windows add .exe to the built executables. The Linux debug build uses ASan and UBSan.

~~~sh
make
python3 -B analysis/verify.py ./isles
python3 -B analysis/review_checks.py ./isles all --mutate-quest
make bench
g++ -std=c++23 -O2 -Iinclude analysis/route_checks.cpp src/routes.cpp -o build/route_checks
./build/route_checks
make debug
python3 -B analysis/verify.py ./isles_debug
python3 -B analysis/review_checks.py ./isles_debug
~~~

The command verifier checks the ten team files, the supplied example, the three counterexamples and 200 generated worlds. The focused script checks corrupted witnesses, a distinct optimum bridge set, cycle direction and reachability, and the quest update rule. Its optional mutation compiles a temporary variant and checks the observed failure. The direct route oracle covers all three-vertex directed graphs with absent arcs or weights −2, 0 or 2 across three sources and two orders.

## Experiments

The report uses fresh measurements from Windows 10 build 19045, an Intel Core i5-13600KF and MinGW-w64 GCC 14.2.0 with C++23 and -O2 -DNDEBUG. The nanobench source is included. The ratio and figure script additionally needs Pillow.

~~~sh
make bench
./build/study
./build/study --control
./build/layout
./build/event_study
python3 -B analysis/summarize_bench.py
~~~

The benchmarks refresh the raw data in results/. The summary script writes the derived tables and plot in build/. The fixed-state control uses seed 2183 and final Dijkstra labels for 32 scans. Prim and layout counters run outside timed calls. See the data and method in the report for graph sizes, seeds and pairing.

The ZIP contains a Git bundle holding all four branches. To inspect the history separately, run git clone -b team-study/leo-scheduling history.bundle team-study-history in the extracted archive.
