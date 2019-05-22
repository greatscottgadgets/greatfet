# GreatFET Python Package and Utilities

This software is developed for python3.  It should be compatible with python2,
but we do less testing with python2 and plan to drop python2 support in the
future.

## Installing or Updating Software

Unless you are developing software or testing software from git, you can
install or update GreatFET software with:
```
sudo pip3 install --upgrade greatfet
```

## Building and Installing Software

To build and install GreatFET software without pip, you will require:

* https://github.com/greatscottgadgets/libgreat

If you are using git, the preferred way to install libgreat is to use the
submodule. From inside the cloned greatfet directory:
```
git submodule init
git submodule update
```

Install libgreat:
```
cd libgreat/host
python3 setup.py build
sudo python3 setup.py install
cd ../..
```

Install greatfet:
```
cd host
python3 setup.py build
sudo python3 setup.py install
```
