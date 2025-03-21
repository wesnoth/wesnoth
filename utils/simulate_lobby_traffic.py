# This script launches wesnothd, followed by 20 copies of wesnoth running the simulate-lobby-activity.lua plugin.
# The idea is to use the script to simulate a high amount of lobby traffic, e.g. for performance testing.

import subprocess
from subprocess import DEVNULL
import os
import os.path
import sys
import time

PORT = 56321
NUM_CLIENTS = 20
EXIT_WAIT_TIME = 20.0

def is_in_path(filename):
  for d in os.get_exec_path():
    if os.path.exists(os.path.join(d, filename)):
      return True
  return False

if (os.name == "nt" and not is_in_path("SDL2.dll")):
  # Launching Wesnoth is not going to succeed
  sys.exit("Error: SDL2.dll is not in PATH. This suggests that you haven't added the external\\dll directory to your PATH.")

print("Launching processes... ", end="")

# Change the working directory to the parent directory of the directory where this script resides
os.chdir(os.path.dirname(os.path.dirname(__file__)))

# Wesnoth restarts itself on launch if OMP_WAIT_POLICY isn't set. That's problematic because it makes it impossible to poll
# when the client processes terminate. Thus, set OMP_WAIT_POLICY.
os.environ["OMP_WAIT_POLICY"] = "PASSIVE"

# Launch the server
server = subprocess.Popen(("wesnothd", "-p", str(PORT)), -1, None, DEVNULL, DEVNULL, DEVNULL)

# Launch the clients
clients = set()
for i in range(NUM_CLIENTS):
  clients.add(subprocess.Popen(("wesnoth", "--plugin=utils/simulate-lobby-activity.lua", "--server=localhost:%d" % PORT, "--username=%d" % i, "--nogui"),
    -1, None, DEVNULL, DEVNULL, DEVNULL))

input("done.\nPress Enter when you want to terminate all processes.")

server.terminate()

print("Waiting for clients to terminate...")
waiting_start_time = time.monotonic()
while len(clients) > 0 and time.monotonic() < waiting_start_time + EXIT_WAIT_TIME:
  time.sleep(1.0)
  clients_copy = list(clients)
  for c in clients_copy:
    if c.poll() is not None:
      # The process has terminated, remove it from the set.
      clients.remove(c)

# Make sure that we get rid of the remaining processes
for c in clients:
  c.kill()
