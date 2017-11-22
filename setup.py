# -*- coding: utf-8 -*-

#
# Copyright © 2017 Malek Hadj-Ali
# All rights reserved.
#
# This file is part of mood.
#
# mood is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3
# as published by the Free Software Foundation.
#
# mood is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with mood.  If not, see <http://www.gnu.org/licenses/>.
#


from setuptools import setup, find_packages, Extension
from distutils.version import LooseVersion

from ctypes.util import find_library
from ctypes import cdll

from codecs import open
from os.path import abspath
from sys import argv


# pkg
pkg_name = "mood.event"
pkg_version = "1.0.6"
pkg_desc = "Python libev interface"

PKG_VERSION = ("PKG_VERSION", "\"{0}\"".format(pkg_version))

err_msg = "Aborted: {0}-{1} requires {{0}} >= {{1}}".format(pkg_name, pkg_version)

def check_version(current_version, minimum_version, name):
    if (not current_version or
        (LooseVersion(current_version) < LooseVersion(minimum_version))):
        raise SystemExit(err_msg.format(name, minimum_version))


# libev
libev_name = "ev"
libev_min_version = "4.24"

def libev_version():
    libev_dll_name = find_library(libev_name)
    if libev_dll_name:
        libev_dll = cdll.LoadLibrary(libev_dll_name)
        return "{0}.{1}".format(libev_dll.ev_version_major(),
                                libev_dll.ev_version_minor())


# setup
if "sdist" not in argv:
    check_version(libev_version(), libev_min_version, "libev")

setup(
      name=pkg_name,
      version=pkg_version,
      description=pkg_desc,
      long_description=open(abspath("README.rst"), encoding="utf-8").read(),

      url="https://github.com/lekma/mood.event",
      author="Malek Hadj-Ali",
      author_email="lekmalek@gmail.com",
      license="GNU General Public License v3 (GPLv3)",
      platforms=["POSIX"],
      keywords="libev event",

      setup_requires = ["setuptools>=24.2.0"],
      python_requires="~=3.5",
      packages=find_packages(),
      namespace_packages=["mood"],
      zip_safe=False,

      ext_package=pkg_name,
      ext_modules=[
                   Extension("_ev", ["src/helpers/helpers.c", "src/_ev.c"],
                             define_macros=[PKG_VERSION],
                             libraries=[libev_name])
      ],

      classifiers=[
                   "Development Status :: 5 - Production/Stable",
                   "Intended Audience :: Developers",
                   "Intended Audience :: System Administrators",
                   "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
                   "Operating System :: POSIX",
                   "Programming Language :: Python :: 3.5",
                   "Programming Language :: Python :: Implementation :: CPython"
      ]
)

