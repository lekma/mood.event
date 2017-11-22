mood.event
==========

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

libev supports ``select``, ``poll``, the Linux-specific ``epoll``, the
BSD-specific ``kqueue`` and the Solaris-specific ``event port`` mechanisms for
file descriptor events (`Io`), Linux ``eventfd``/``signalfd`` (for faster and
cleaner inter-thread wakeup (`Async`)/signal handling (`Signal`)), relative
timers (`Timer`), absolute timers (`Periodic`), timers with customised
rescheduling (`Scheduler`), synchronous signals (`Signal`), process status
change events (`Child`), and event watchers dealing with the event loop
mechanism itself (`Idle`, `Embed`, `Prepare` and `Check` watchers) and even
limited support for fork events (`Fork`).

It also is quite `fast <http://libev.schmorp.de/bench.html>`_.

`libev <http://software.schmorp.de/pkg/libev.html>`_ is written and maintained
by Marc Lehmann.

