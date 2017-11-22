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

from . import default_loop, Loop, EVFLAG_NOSIGMASK, EV_MAXPRI

