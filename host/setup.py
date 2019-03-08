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


setup_req = []
setup_options = {}

# Deduce version, if possible.
if os.path.isfile('../VERSION'):
    setup_options['version'] = read('../VERSION').strip()
else:
    setup_options['version_config'] =  {
        "version_format": '{tag}.dev{commitcount}+git.{gitsha}',
        "starting_version": "2019.05.01"
    }
    setup_req.append('better-setuptools-git-version')

setup(
    name='GreatFET',
    setup_requires=setup_req,
    url='https://greatscottgadgets.com/greatfet/',
    license='BSD',
    entry_points={
        'console_scripts': [
            'greatfet = greatfet.commands.gf:main',
            'gf = greatfet.commands.gf:main',
            'greatfet_adc = greatfet.commands.greatfet_adc:main',
            'greatfet_dac = greatfet.commands.greatfet_dac:main',
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
            'greatfet_msp430 = greatfet.commands.greatfet_msp430:main',
        ],
    },
    author='Great Scott Gadgets', #TODO: Figure out whose name should go here!
    author_email='ktemkin@greatscottgadgets.com',
    tests_require=[''],
    install_requires=['pyusb', install_req, 'pygreat', 'future'],
    description='Python library for hardware hacking with the GreatFET',
    long_description=read('README'),
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
    extras_require={},
    **setup_options
)
