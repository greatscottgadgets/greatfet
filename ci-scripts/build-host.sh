#!/bin/bash
git submodule init
git submodule update
python3 -m venv testing-venv
source testing-venv/bin/activate
pushd libgreat/host/
python3 setup.py build
python3 setup.py install
popd
pushd host/
python3 setup.py build
python3 setup.py install
popd