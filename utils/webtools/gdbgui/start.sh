#!/bin/bash
while ! netcat -z server 4444 </dev/null; do sleep 10; done
sleep 3
until PURE_PYTHON=1 /root/.local/bin/gdbgui -r --port 8080; do
    echo "Server gdbgui crashed with exit code $?.  Respawning.." >&2
    sleep 1
done
#PURE_PYTHON=1 /root/.local/bin/gdbgui -r --port 8080 &
# --gdb-cmd="-target-select remote server:4444"