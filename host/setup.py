import os
import sys
from setuptools import setup, find_packages

def read(fname):
    filename = os.path.join(os.path.dirname(__file__), fname)
    with open(filename, 'r') as f:
        return f.read()

install_req = ['ipython']
if sys.version_info[0] < 3 and 'bdist_wheel' not in sys.argv:
    install_req.remove('ipython')
    install_req.append('ipython<6')

setup(
    name='GreatFET',
    version='0.0', #TODO: Derive this from the main module.
    url='https://greatscottgadgets.com/greatfet/',
    license='BSD',
    entry_points={
        'console_scripts': [
            'greatfet = greatfet.commands.gf:main',
            'gf = greatfet.commands.gf:main',
            'greatfet_adc = greatfet.commands.greatfet_adc:main',
            'greatfet_adf7242 = greatfet.commands.greatfet_adf7242:main',
            'greatfet_firmware = greatfet.commands.greatfet_firmware:main',
            'greatfet_fw = greatfet.commands.greatfet_firmware:main',
            'greatfet_info = greatfet.commands.greatfet_info:main',
            'greatfet_logic = greatfet.commands.greatfet_logic:main',
            'greatfet_sdir = greatfet.commands.greatfet_sdir:main',
            'greatfet_spiflash = greatfet.commands.greatfet_spiflash:main',
            'greatfet_DS18B20 = greatfet.commands.greatfet_DS18B20:main',
            'greatfet_i2c = greatfet.commands.greatfet_i2c:main',
            'greatfet_shell = greatfet.commands.greatfet_shell:main',
            'greatfet_dmesg = greatfet.commands.greatfet_dmesg:main',
        ],
    },
    #author='', #TODO: Figure out whose name should go here!
    #author_email='',
    tests_require=[''],
    install_requires=['pyusb', install_req, 'pygreat', 'future'],
    description='Python library for hardware hacking with the GreatFET',
    long_description=read('../README.md'),
    packages=find_packages(),#['greatfet'],
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
