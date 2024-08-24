#!/bin/sh

run_taclebench -w -v -t 1 -L 0.15 -X MSRP -p 2 -q 3 -O 0.4  3   5 &
run_taclebench -w -v -t 2 -L 0.15 -X MSRP -p 3 -q 2 -O 0.4  3   5 &
run_taclebench -w -v -t 3 -L 0.15 -X MSRP -p 0 -q 1 -O 0.9  2.5 5 &
run_taclebench -w -v -t 4 -L 0.15 -X MSRP -p 1 -q 4 -O 1.3  3.5 5 &
run_taclebench -w -v -t 5 -p 2 -q 7 -O 0.15 3.6 5 &
run_taclebench -w -v -t 6 -p 3 -q 5 -O 0.25 3.6 5 &
run_taclebench -w -v -t 7 -p 2 -q 6 -O 0.25 3.6 5 &

sleep 1

release_ts && wait

tmux swap-pane -U