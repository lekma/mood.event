.. currentmodule:: mood.event

========================
mood.event documentation
========================

Python libev interface

libev is an event loop: you register interest in certain events (such as a file
descriptor being readable or a timeout occurring), and it will manage these
event sources and provide your program with events.
To do this, it must take more or less complete control over your process (or
thread) by executing the event loop handler, and will then communicate events
via a callback mechanism.
You register interest in certain events by registering so-called event watchers,
which you initialise with the details of the event, and then hand over to libev
by starting the watcher.

libev supports :manpage:`select(2)`, :manpage:`poll(2)`, the Linux-specific
:manpage:`epoll(7)`, the BSD-specific ``kqueue`` and the Solaris-specific
``event port`` mechanisms for file descriptor events (:py:class:`Io`), Linux
:manpage:`eventfd(2)`/:manpage:`signalfd(2)` (for faster and cleaner
inter-thread wakeup (:py:class:`Async`)/signal handling (:py:class:`Signal`)),
relative timers (:py:class:`Timer`), absolute timers (:py:class:`Periodic`),
timers with customised rescheduling (:py:class:`Scheduler`), synchronous signals
(:py:class:`Signal`), process status change events (:py:class:`Child`), event
watchers dealing with the event loop mechanism itself (:py:class:`Idle`,
:py:class:`Embed`, :py:class:`Prepare` and :py:class:`Check` watchers) and even
limited support for fork events (:py:class:`Fork`).

It also is quite `fast <http://libev.schmorp.de/bench.html>`_.

`libev <http://software.schmorp.de/pkg/libev.html>`_ is written and maintained
by Marc Lehmann.

.. note::

    This documentation is largely adapted from `libev's documentation
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod>`_.


Useful links
============

* `Project Home <https://github.com/lekma/mood.event>`_
* `Download <https://github.com/lekma/mood.event/releases>`_
* `Bug Tracker <https://github.com/lekma/mood.event/issues>`_


Contents
========

.. toctree::
    :maxdepth: 3
    :titlesonly:

    module


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
