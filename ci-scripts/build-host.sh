#!/bin/bash
set -e
git submodule init
git submodule update
python3 -m venv testing-venv
source testing-venv/bin/activate
pip3 install pyyaml
pushd libgreat/host/
pip3 install .
popd
pushd host/
pip3 install .
popd
