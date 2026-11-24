# Kestrel Isles four-role exemplar

Codex authored this post-project study. Mira, Jonah, Nia and Leo are fictional work roles. The study begins from the course starter dcf4e78. The integrated program preserves the behavior of master at 077adeb.

The complete project and [Word report](report/Kestrel_Isles_End_to_End_Study.docx) are on team-study/leo-scheduling. The other three branches preserve their scoped histories. The [log index](ai_logs/README.md) opens the four independent seven-exchange consultations. The [evidence index](study/evidence.md) connects each finding to a log line, a recorded result and a review commit. The [roles and provenance](study/roles.md) describe the saved drafts and the review.

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

The command verifier checks the ten team files, the supplied example, the three counterexamples and 200 generated worlds. The focused checks cover command rejection, alternate and forged witnesses, a reachable cycle with a downstream tail, and touching events with tied finish times. The optional quest mutation compiles a temporary variant and checks the later-improvement failure. The direct route oracle covers three-vertex directed graphs with absent arcs or weights −2, 0 or 2 across three sources and two orders.

## Experiments

The report uses fresh measurements from Windows 10 build 19045, an Intel Core i5-13600KF and MinGW-w64 GCC 14.2.0 with C++23 and -O2 -DNDEBUG. The nanobench source is included. The ratio and figure script additionally needs Pillow.

~~~sh
make bench
./build/study
./build/study --control
./build/study --revisits > results/union-find-revisits.csv
./build/layout
./build/layout --storage > results/adjacency-capacity.csv
./build/event_study
python3 -B analysis/summarize_bench.py
~~~

The benchmarks refresh the raw data in results/. The summary script writes the derived tables and plot in build/. The fixed-state control uses seed 2183 and final Dijkstra labels for 32 scans. The revisit control counts links followed in both traversals of find on a copied rank tree. The storage control counts owned container-array capacity. Prim and layout counters run outside timed calls. See the report for graph sizes, seeds and pairing.

The ZIP contains a Git bundle holding all four branches. To inspect the history separately, run git clone -b team-study/leo-scheduling history.bundle team-study-history in the extracted archive.
