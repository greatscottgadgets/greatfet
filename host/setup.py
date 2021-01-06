import os
import sys
from setuptools import setup, find_packages

def read(fname):
    filename = os.path.join(os.path.dirname(__file__), fname)
    with open(filename, 'r') as f:
        return f.read()


# Handle python2 vs python3 requirements.
per_version_requirements = []
if sys.version_info < (3, 0):
    per_version_requirements.append('ipython<6')
else:
    per_version_requirements.append('ipython')


setup_req = []
setup_options = {}

# Deduce version, if possible.
if os.path.isfile('../VERSION'):
    setup_options['version'] = read('../VERSION').strip()
    pygreat_with_version = 'pygreat>={}'.format(setup_options['version'])
else:
    setup_options['version_config'] =  {
        "version_format": '{tag}.dev+git.{sha}',
        "starting_version": "2019.05.01"
    }
    setup_req.append('even-better-setuptools-git-version')
    pygreat_with_version = "pygreat"

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
            'greatfet_led = greatfet.commands.greatfet_led:main',
            'greatfet_logic = greatfet.commands.greatfet_logic:main',
            'greatfet_sdir = greatfet.commands.greatfet_sdir:main',
            'greatfet_spiflash = greatfet.commands.greatfet_spiflash:main',
            'greatfet_spi-flash = greatfet.commands.greatfet_spiflash:main',
            'greatfet_DS18B20 = greatfet.commands.greatfet_DS18B20:main',
            'greatfet_i2c = greatfet.commands.greatfet_i2c:main',
            'greatfet_shell = greatfet.commands.greatfet_shell:main',
            'greatfet_dmesg = greatfet.commands.greatfet_dmesg:main',
            'greatfet_msp430 = greatfet.commands.greatfet_msp430:main',
            'greatfet_host-tools = greatfet.commands.greatfet_host_tools:main',
            'greatfet_pattern = greatfet.commands.greatfet_pattern:main',
            'greatfet_loadable = greatfet.commands.greatfet_loadable:main',
            'greatfet_usb-capture = greatfet.commands.greatfet_usb_capture:main',
            'greatfet_uart = greatfet.commands.greatfet_uart:main',
            'greatfet_pirate = greatfet.commands.greatfet_pirate:main',
            'greatfet_jtag = greatfet.commands.greatfet_jtag:main',
            'greatfet_chipcon = greatfet.commands.greatfet_chipcon:main',
        ],
    },
    author='Great Scott Gadgets',
    author_email='ktemkin@greatscottgadgets.com',
    install_requires= [
        per_version_requirements,
        'pyusb',
        pygreat_with_version,
        'future',
        'pyfwup>=0.2',
        'tqdm',
        'cmsis_svd',
        'tabulate',
        'prompt_toolkit<3.1.0',
        'intelhex',
    ],
    description='Python library for hardware hacking with the GreatFET',
    long_description=read('README.md'),
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
