#!/bin/bash

set -euo pipefail

function clone_googletest() {
	echo [clone_googletest]

    git clone https://github.com/google/googletest.git 
}

main() {
	clone_googletest
}

main $@