#!/usr/bin/env python3
"""Run Doxygen and create Meson's flat output stamp."""

import pathlib
import subprocess
import sys

subprocess.run([sys.argv[1], sys.argv[2]], check=True)
pathlib.Path(sys.argv[3]).touch()
