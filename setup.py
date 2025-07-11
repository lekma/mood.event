# -*- coding: utf-8 -*-


from setuptools import setup, find_packages, Extension
from distutils.version import LooseVersion

from ctypes.util import find_library
from ctypes import cdll

from codecs import open
from os.path import abspath
from sys import argv


# pkg
pkg_name = "mood.event"
pkg_version = "2.0.0"
pkg_desc = "Python libev interface"

PKG_VERSION = ("PKG_VERSION", "\"{0}\"".format(pkg_version))

err_msg = "Aborted: {0}-{1} requires {{0}} >= {{1}}".format(pkg_name, pkg_version)

def check_version(current_version, minimum_version, name):
    if (
        not current_version or
        (LooseVersion(current_version) < LooseVersion(minimum_version))
    ):
        raise SystemExit(err_msg.format(name, minimum_version))


# libev
libev_name = "ev"
libev_min_version = "4.33"

def libev_version():
    libev_dll_name = find_library(libev_name)
    if libev_dll_name:
        libev_dll = cdll.LoadLibrary(libev_dll_name)
        return "{0}.{1}".format(
            libev_dll.ev_version_major(), libev_dll.ev_version_minor()
        )


# setup
if "sdist" not in argv:
    check_version(libev_version(), libev_min_version, "libev")

setup(
    name=pkg_name,
    version=pkg_version,
    description=pkg_desc,
    long_description=open(abspath("README.txt"), encoding="utf-8").read(),
    long_description_content_type="text",

    url="https://github.com/lekma/mood.event",
    download_url="https://github.com/lekma/mood.event/releases",
    project_urls={
        "Documentation": "https://mood.readthedocs.io/projects/event/",
        "Bug Tracker": "https://github.com/lekma/mood.event/issues"
    },
    author="Malek Hadj-Ali",
    author_email="lekmalek@gmail.com",
    license="The Unlicense (Unlicense)",
    platforms=["POSIX"],
    keywords="libev event",

    setup_requires = ["setuptools>=24.2.0"],
    python_requires="~=3.10",
    packages=find_packages(),
    namespace_packages=["mood"],
    zip_safe=False,

    ext_package="mood",
    ext_modules=[
        Extension(
            "event",
            [
                "src/helpers/helpers.c",
                "src/Loop.c",
                "src/Watcher.c",
                "src/Io.c",
                "src/Timer.c",
                "src/Periodic.c",
                "src/Signal.c",
                "src/Child.c",
                "src/Idle.c",
                "src/Prepare.c",
                "src/Check.c",
                "src/Embed.c",
                "src/Fork.c",
                "src/Async.c",
                "src/event.c",
            ],
            define_macros=[PKG_VERSION],
            libraries=[libev_name]
        )
    ],

    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Intended Audience :: System Administrators",
        "License :: OSI Approved :: The Unlicense (Unlicense)",
        "Operating System :: POSIX",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: Implementation :: CPython"
    ]
)
