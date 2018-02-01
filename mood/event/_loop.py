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


from os import getpid
from sys import exit
from signal import pthread_sigmask, SIG_BLOCK, SIG_UNBLOCK, NSIG, SIGTERM, SIGINT
from collections import deque
from abc import ABCMeta, abstractmethod

from . import default_loop, Loop, EVFLAG_AUTO, EVFLAG_SIGNALFD, EVFLAG_NOSIGMASK, EVBREAK_ALL, EV_MAXPRI


__all__ = ["ServerLoop", "ClientLoop", "ParentLoop", "ChildLoop"]


# ------------------------------------------------------------------------------
# private
# ------------------------------------------------------------------------------

class _BaseLoop(metaclass=ABCMeta):

    def __init__(self, logger, flags=EVFLAG_AUTO):
        self._loop = self._LoopCtor(flags=(flags | EVFLAG_NOSIGMASK))
        self._logger = logger
        self._watchers = deque()
        self._stopping = False
        self._pid = getpid()

    def __repr__(self):
        return "<{0}.{1} pid={2}>".format(self.__class__.__module__,
                                          self.__class__.__name__,
                                          self._pid)

    def _on_error(self, message, exc_info=True):
        try:
            suffix = " -> stopping" if not self._stopping else ""
            self._logger.critical("{0}: {1}{2}".format(self, message, suffix),
                                  exc_info=exc_info)
        finally:
            self.stop() # stop on error

    def _prepare(self, watcher, revents): # watcher callback
        try:
            watcher.stop()
            self._watchers.remove(watcher)
            self.starting()
            self._logger.info("{0}: started".format(self))
        except Exception:
            self._on_error("error while starting")

    def _setup(self):
        self._watchers.append(self._loop.prepare(self._prepare))

    def start(self, *args, **kwargs):
        if self.stopped:
            self._logger.info("{0}: starting...".format(self))
            self._setup()
            self.setup(*args, **kwargs)
            for watcher in self._watchers:
                watcher.start()
            try:
                while self._loop.start():
                    pass
            finally:
                self._logger.info("{0}: stopped".format(self))

    def stop(self, *args): # watcher callback
        if not self.stopped and not self._stopping:
            self._stopping = True
            self._logger.info("{0}: stopping...".format(self))
            while self._watchers:
                self._watchers.pop().stop()
            try:
                self.stopping()
            except Exception:
                self._on_error("error while stopping")
            finally:
                self._loop.stop(EVBREAK_ALL)
                self._stopping = False

    @property
    def stopped(self):
        return self._loop.depth == 0

    @abstractmethod
    def setup(self, *args, **kwargs):
        raise NotImplementedError

    @abstractmethod
    def starting(self):
        raise NotImplementedError

    @abstractmethod
    def stopping(self):
        raise NotImplementedError


class _SignalLoop(_BaseLoop):

    def __init__(self, *args, **kwargs):
        flags = kwargs.pop("flags", EVFLAG_AUTO)
        super().__init__(*args, flags=(flags | EVFLAG_SIGNALFD), **kwargs)

    def _setup(self, signals=(SIGTERM, SIGINT)):
        super()._setup()
        pthread_sigmask(SIG_UNBLOCK, signals)
        self._watchers.extend((self._loop.signal(signal, self.stop,
                                                 None, EV_MAXPRI)
                               for signal in signals))


# ------------------------------------------------------------------------------
# basic loops
# ------------------------------------------------------------------------------

class ServerLoop(_SignalLoop):

    _LoopCtor = default_loop


class ClientLoop(_SignalLoop):

    _LoopCtor = Loop


# ------------------------------------------------------------------------------
# parent/child loops
# ------------------------------------------------------------------------------

class ParentLoop(ServerLoop):

    def _setup(self, **kwargs):
        super()._setup(**kwargs)
        self._watchers.append(self._loop.child(0, False, self._supervise))

    def _supervise(self, watcher, revents): # watcher callback
        try:
            self.handle_child(watcher.rpid, watcher.rstatus)
        except Exception:
            self._on_error("error while handling a child")

    @abstractmethod
    def handle_child(self, pid, status):
        raise NotImplementedError


class ChildLoop(ClientLoop):

    def __init__(self, *args, **kwargs):
        pthread_sigmask(SIG_BLOCK, range(1, NSIG))
        super().__init__(*args, **kwargs)

    def _setup(self, signals=(SIGTERM,)):
        super()._setup(signals=signals)

    def _on_error(self, *args, **kwargs):
        try:
            super()._on_error(*args, **kwargs)
        finally:
            exit(125) # do NOT use 127 nor 126

