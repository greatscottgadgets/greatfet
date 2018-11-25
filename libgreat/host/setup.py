import os
from setuptools import setup, find_packages

def read(fname):
    filename = os.path.join(os.path.dirname(__file__), fname)
    with open(filename, 'r') as f:
        return f.read()

setup(
    name='pygreat',
    version='0.0', #TODO: Derive this from the main module.
    url='https://greatscottgadgets.com/greatfet/',
    license='BSD',
    entry_points={
        'console_scripts': [],
    },
    author='Katherine J. Temkin',
    author_email='ktemkin@insomniasec.io',
    tests_require=[''],
    install_requires=['pyusb', 'future', 'backports.functools_lru_cache'],
    description='Python library for talking with libGreat devices',
    long_description=read('../README.md'),
    packages=find_packages(),
    include_package_data=True,
    platforms='any',
    classifiers = [
        'Programming Language :: Python',
        'Development Status :: 1 - Planning',
        'Natural Language :: English',
        'Environment :: Console',
        'Environment :: Plugins',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: BSD License',
        'Operating System :: OS Independent',
        'Topic :: Scientific/Engineering',
        'Topic :: Security',
        ],
    extras_require={}
)
