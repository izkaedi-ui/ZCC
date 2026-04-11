#!/bin/bash
ulimit -v 512000
exec "$@"
