#!/bin/bash
gdb --batch -ex "run" -ex "bt" -ex "x/10i \$pc - 16" -ex "info registers" ./test_harness
