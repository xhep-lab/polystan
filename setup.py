import os
import sysconfig
import warnings

from setuptools import setup

PTH_FILE = os.path.join(sysconfig.get_path('purelib'), "polystan.pth")
CWD = os.path.dirname(os.path.realpath(__file__))
PKG_DIR = os.path.join(CWD, "contrib")

REQS = [
    'arviz',
    'matplotlib',
    'numpy',
    'pytest',
    'pytest-mpl',
    'rpy2',
    'sigfig',
    'syrupy'
]

try:
    with open(PTH_FILE, 'w') as f:
        print(PKG_DIR, file=f)
except PermissionError as e:
    warnings.warn(e)

setup(
    name='polystan',
    version='1.0.0',
    author='Andrew Fowlie',
    author_email='andrew.fowlie@xjtlu.edu.cn',
    description='Thin wrapper around polystan programs',
    install_requires=REQS)
