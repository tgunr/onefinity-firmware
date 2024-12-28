#!/usr/bin/env python3

from setuptools import setup
import json
import os

pkg = json.load(open('package.json', 'r'))

# Get Python version
import sysconfig
python_version = sysconfig.get_python_version()

# Determine site-packages directory
if os.path.exists('/usr/local/lib/python3.5/dist-packages'):
    site_packages = '/usr/local/lib/python3.5/dist-packages'
else:
    site_packages = f'/usr/local/lib/python{python_version}/site-packages'

setup(
    name = pkg['name'],
    version = pkg['version'],
    description = 'Buildbotics Machine Controller',
    long_description = open('README.md', 'rt').read(),
    author = 'Joseph Coffland',
    author_email = 'joseph@buildbotics.org',
    platforms = ['any'],
    license = pkg['license'],
    url = pkg['homepage'],
    package_dir = {'': 'src/py'},
    packages = ['bbctrl', 'inevent', 'lcd', 'camotics', 'iw_parse'],
    package_data={
        'camotics': ['gplan.so', '__init__.py']
    },
    include_package_data = True,
    entry_points = {
        'console_scripts': [
            'bbctrl = bbctrl:run'
        ]
    },
    scripts = [
        'scripts/update-bbctrl',
        'scripts/upgrade-bbctrl',
        'scripts/sethostname',
        'scripts/reset-video',
        'scripts/config-wifi',
        'scripts/config-screen',
        'scripts/edit-config',
        'scripts/edit-boot-config',
        'scripts/browser',
    ],
    install_requires = 'tornado sockjs-tornado pyserial pyudev smbus2'.split(),
    data_files = [
        ('lib/modules/bbserial', ['src/bbserial/bbserial.ko']),
        ('boot/overlays', ['src/bbserial/overlays/bbserial.dtbo']),
        (os.path.join(site_packages, 'camotics'), ['src/py/camotics/gplan.so']),
    ],
    zip_safe = False,
)
