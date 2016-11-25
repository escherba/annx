import os
import sys
import re
import platform
import numpy
import warnings
import logging
import itertools
import subprocess
import distutils.sysconfig
from glob import glob
from setuptools import Command, setup, Extension
from setuptools.dist import Distribution
from Cython.Distutils import build_ext
from pkg_resources import resource_string

NAME = 'annx'
VERSION = '0.0.1'
SRC_ROOT = 'annx'
URL = 'https://github.com/escherba/annx'


# remove the "-Wstrict-prototypes" compiler option (not valid for C++)
CFG_VARS = distutils.sysconfig.get_config_vars()
for key, value in CFG_VARS.items():
    if isinstance(value, basestring):
        CFG_VARS[key] = value.replace("-Wstrict-prototypes", "")


class BinaryDistribution(Distribution):
    """
    Subclass the setuptools Distribution to flip the purity flag to false.
    See http://lucumr.pocoo.org/2014/1/27/python-on-wheels/
    """
    def is_pure(self):
        return False

try:
    from Cython.Build import cythonize
    has_cython = True
except ImportError:
    has_cython = False

is_dev = 'dev' in VERSION
use_cython = is_dev or '--cython' in sys.argv or '--with-cython' in sys.argv
if '--no-cython' in sys.argv:
    use_cython = False
    sys.argv.remove('--no-cython')
if '--without-cython' in sys.argv:
    use_cython = False
    sys.argv.remove('--without-cython')
if '--cython' in sys.argv:
    sys.argv.remove('--cython')
if '--with-cython' in sys.argv:
    sys.argv.remove('--with-cython')

use_openmp = '--openmp' in sys.argv or '--with-openmp' in sys.argv
if '--no-openmp' in sys.argv:
    use_openmp = False
    sys.argv.remove('--no-openmp')
if '--without-openmp' in sys.argv:
    use_openmp = False
    sys.argv.remove('--without-openmp')
if '--openmp' in sys.argv:
    sys.argv.remove('--openmp')
if '--with-openmp' in sys.argv:
    sys.argv.remove('--with-openmp')


if use_cython and not has_cython:
    if is_dev:
        raise RuntimeError('Cython required to build dev version of %s.' % NAME)
    warnings.warn('Cython not installed. Building without Cython.')
    use_cython = False


# dependency links
SKIP_RE = re.compile(r'^\s*(?:-\S+)\s+(.*)$')

# Regex groups: 0: URL part, 1: package name, 2: package version
EGG_RE = re.compile(r'^(git\+https?://[^#]+)(?:#egg=([a-z0-9_.]+)(?:-([a-z0-9_.-]+))?)?$')

# Regex groups: 0: URL part, 1: package name, 2: branch name
URL_RE = re.compile(r'^\s*(https?://[\w\.]+.*/([^\/]+)/archive/)([^\/]+).zip$')

# our custom way of specifying extra requirements in separate text files
EXTRAS_RE = re.compile(r'^extras\-(\w+)\-requirements\.txt$')


def parse_reqs(reqs):
    """Parse requirements.txt files into lists of requirements and dependencies
    """
    pkg_reqs = []
    dep_links = []
    for req in reqs:
        # find things like `--find-links <URL>`
        dep_link_info = SKIP_RE.match(req)
        if dep_link_info is not None:
            url = dep_link_info.group(1)
            dep_links.append(url)
            continue
        # add packages of form:
        # git+https://github.com/Livefyre/pymaptools#egg=pymaptools-0.0.3
        egg_info = EGG_RE.match(req)
        if egg_info is not None:
            url, _, _ = egg_info.group(0, 2, 3)
            # if version is None:
            #     pkg_reqs.append(egg)
            # else:
            #     pkg_reqs.append(egg + '==' + version)
            dep_links.append(url)
            continue
        # add packages of form:
        # https://github.com/escherba/matplotlib/archive/qs_fix_build.zip
        zip_info = URL_RE.match(req)
        if zip_info is not None:
            url, pkg = zip_info.group(0, 2)
            pkg_reqs.append(pkg)
            dep_links.append(url)
            continue
        pkg_reqs.append(req)
    return pkg_reqs, dep_links


def build_extras(glob_pattern):
    """Generate extras_require mapping
    """
    fnames = glob(glob_pattern)
    result = dict()
    dep_links = []
    for fname in fnames:
        extras_match = re.search(EXTRAS_RE, fname)
        if extras_match is not None:
            extras_file = extras_match.group(0)
            extras_name = extras_match.group(1)
            with open(extras_file, 'r') as fhandle:
                result[extras_name], deps = parse_reqs(fhandle.readlines())
                dep_links.extend(deps)
    return result, dep_links


INSTALL_REQUIRES, INSTALL_DEPS = parse_reqs(
    resource_string(__name__, 'requirements.txt').splitlines())
TESTS_REQUIRE, TESTS_DEPS = parse_reqs(
    resource_string(__name__, 'dev-requirements.txt').splitlines())
EXTRAS_REQUIRE, EXTRAS_DEPS = build_extras('extras-*-requirements.txt')
DEPENDENCY_LINKS = list(set(itertools.chain(
    INSTALL_DEPS,
    TESTS_DEPS,
    EXTRAS_DEPS
)))


# os.environ["CC"] = "gcc-4.9"
# os.environ["CXX"] = "g++-4.9"


CXXFLAGS_UNIX = u"""
-DNDEBUG
-O3
-std=c++11
-msse4.2
-ffast-math
-fexceptions
-Wall
-Wextra
-Wno-deprecated-declarations
-Wno-double-promotion
-Wno-float-equal
-Wno-format-nonliteral
-Wno-old-style-cast
-Wno-padded
-Wno-unused-value
-Wno-unused-function
-Wno-unused-parameter
""".split()


class Clean(Command):
    """
    Clean build files.
    """

    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):

        pth = os.path.dirname(os.path.abspath(__file__))
        pkg_pth = os.path.join(pth, SRC_ROOT)

        subprocess.call(['rm', '-rf', os.path.join(pth, 'dist')])
        subprocess.call(['rm', '-rf', os.path.join(pth, 'build')])
        subprocess.call(['rm', '-rf', os.path.join(pth, '*.egg-info')])
        subprocess.call(['find', pkg_pth, '-name', '*.pyc', '-type', 'f', '-delete'])
        subprocess.call(['find', pkg_pth, '-name', '*.so', '-type', 'f', '-delete'])


def set_compiler():
    """
    Try to find and use GCC on OSX for OpenMP support.
    """
    # For macports and homebrew
    cxx_patterns = [
        '/opt/local/bin/g++-mp-[0-9].[0-9]',
        '/opt/local/bin/g++-mp-[0-9]',
        '/usr/local/bin/g++-[0-9].[0-9]',
        '/usr/local/bin/g++-[0-9]'
    ]
    cc_patterns = [
        '/opt/local/bin/gcc-mp-[0-9].[0-9]',
        '/opt/local/bin/gcc-mp-[0-9]',
        '/usr/local/bin/gcc-[0-9].[0-9]',
        '/usr/local/bin/gcc-[0-9]'
    ]

    if 'darwin' in platform.platform().lower():
        cxx_binaries = []
        for pattern in cxx_patterns:
            cxx_binaries += glob(pattern)
        cxx_binaries.sort()

        cc_binaries = []
        for pattern in cc_patterns:
            cc_binaries += glob(pattern)
        cc_binaries.sort()

        global use_openmp
        if cc_binaries:
            _, cc = os.path.split(cc_binaries[-1])
            os.environ["CC"] = cc
        else:
            use_openmp = False
            logging.warning('No GCC available. Install gcc from Homebrew '
                            'using `brew install gcc`')

        if cxx_binaries:
            _, cxx = os.path.split(cxx_binaries[-1])
            os.environ["CXX"] = cxx
        else:
            use_openmp = False
            logging.warning('No G++ available. Install gcc from Homebrew '
                            'using `brew install gcc`')


def define_extensions(use_cython=False):
    if sys.platform.startswith("win"):
        # compile args from
        # https://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx
        compile_args = ['/O2', '/openmp']
        link_args = []
    else:
        compile_args = CXXFLAGS_UNIX
        link_args = []
        if use_openmp:
            compile_args.append("-fopenmp")
            link_args.append("-fopenmp")

        if 'anaconda' not in sys.version.lower():
            compile_args.append('-march=native')

    if use_cython:
        sources = ["annx/ext.pyx", "annx/gauss_lsh.pxd"]
    else:
        sources = ["annx/ext.cpp"]
    modules = [
        Extension(
            "annx.ext",
            [
                "src/c++/src/common/ann_util.cc",
            ] + sources,
            depends=[
                "src/c++/src/ann/gauss_lsh.h",
                "src/c++/src/ann/space.h",
                "src/c++/src/common/ann_util.h",
            ],
            language="c++",
            extra_compile_args=compile_args,
            include_dirs=[
                numpy.get_include(),
                "/usr/include/eigen3",
                "/usr/local/include/eigen3",
                "src/c++/src",
                "annx",
            ])
    ]

    if use_cython:
        return cythonize(modules)
    else:
        return modules

set_compiler()

setup(
    name=NAME,
    version=VERSION,
    author="Eugene Scherba",
    license="MIT",
    author_email="escherba@gmail.com",
    description=("Spatial indexing via locality-sensitive hashing"),
    url=URL,
    download_url=URL + "/tarball/master/" + VERSION,
    packages=[SRC_ROOT],
    install_requires=INSTALL_REQUIRES,
    tests_require=TESTS_REQUIRE,
    dependency_links=DEPENDENCY_LINKS,
    zip_safe=False,
    test_suite='nose.collector',
    cmdclass={'clean': Clean, 'build_ext': build_ext},
    keywords=['hash', 'hashing', 'lsh', 'spatial', 'index', 'indexing'],
    ext_modules=define_extensions(use_cython),
    classifiers=[
        'Development Status :: 4 - Beta',
        'License :: OSI Approved :: MIT License',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'Operating System :: OS Independent',
        'Programming Language :: Cython',
        'Programming Language :: Python :: 2.7',
        'Topic :: Scientific/Engineering',
        'Topic :: Scientific/Engineering :: Information Analysis',
    ],
    long_description=resource_string(__name__, 'README.rst'),
    distclass=BinaryDistribution,
)
