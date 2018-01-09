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
file descriptor events (`Io`_), Linux ``eventfd``/``signalfd`` (for faster and
cleaner inter-thread wakeup (`Async`_)/signal handling (`Signal`_)), relative
timers (`Timer`_), absolute timers (`Periodic`_), timers with customised
rescheduling (`Scheduler`_), synchronous signals (`Signal`_), process status
change events (`Child`_), and event watchers dealing with the event loop
mechanism itself (`Idle`_, `Embed`_, `Prepare`_ and `Check`_ watchers) and even
limited support for fork events (`Fork`_).

It also is quite `fast <http://libev.schmorp.de/bench.html>`_.

`libev <http://software.schmorp.de/pkg/libev.html>`_ is written and maintained
by Marc Lehmann.

**Note:** The following documentation is largely adapted from `libev's
documentation <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod>`_.


.. contents:: :local:
    :backlinks: none


Module Interface
----------------

default_loop([flags=EVFLAG_AUTO, callback=None, data=None, io_interval=0.0, timeout_interval=0.0]) -> 'default loop'
  This will instanciate the *default loop* if it hasn't been created yet and
  return it. If the *default loop* was already initialized this simply returns
  it (and ignores the arguments).

  The *default loop* is the only loop that can handle `Child`_ watchers, and to
  do this, it always registers a handler for ``SIGCHLD``. If this is a problem
  for your application you can either instanciate a `Loop`_ which doesn't do
  that, or you can simply overwrite the ``SIGCHLD`` signal handler.

  See `Loop`_ for details about the arguments.

  **Note:** If you don't know what loop to use, use the one returned from this
  function.

.. _@fatal:

@fatal
  A decorator indicating unhandled exceptions are fatal to the loop. A
  callback using this decorator will stop the loop when an unhandled exception
  happens during its execution.

time() -> float
  Returns the current time as libev would use it.

  **Note:** The `Loop.now()`_ method is usually faster and also
  often returns the timestamp you actually want to know.

.. _sleep():

sleep(interval)
  Sleep for the given *interval* (in seconds). The current thread will be
  blocked until either it is interrupted or the given time interval has passed
  (approximately - it might return a bit earlier even if not interrupted).
  Returns immediately if *interval* <= 0.0.

  **Note:** The range of *interval* is limited - libev only guarantees to work
  with sleep times of up to one day (*interval* <= 86400.0).

abi_version() -> (int, int)
  Returns a tuple of major, minor version numbers. These numbers represent the
  libev ABI version that this module is running.

  **Note:** This is not the same as libev version (although it might coincide).

.. _supported_backends():

supported_backends() -> int
  Return the set of all `backends`_ compiled into this binary of libev
  (independent of their availability on the system you are running on).

.. _recommended_backends():

recommended_backends() -> int
  Return the set of all `backends`_ compiled into this binary of libev and also
  recommended for this platform, meaning it will work for most file descriptor
  types. This set is often smaller than the one returned by
  `supported_backends()`_, as for example ``kqueue`` is broken on most BSDs and
  will not be auto-detected unless you explicitly request it.
  This is the set of backends that libev will probe for if you specify no
  backends explicitly.

embeddable_backends() -> int
  Returns the set of `backends`_ that are embeddable in other event loops. This
  value is platform-specific but can include backends not available on the
  current system. To find which embeddable backends might be supported on the
  current system, you would need to look at:

  .. code:: python

      embeddable_backends() & supported_backends()

  likewise for recommended ones:

  .. code:: python

      embeddable_backends() & recommended_backends()

  See `Embed`_ watchers for more information about embedding loops.

.. _feed_signal():

feed_signal(signum)
  * signum (int)
      signal number to feed libev.

  This function can be used to "simulate" a signal receive. It is completely
  safe to call this function at any time, from any context, including signal
  handlers or random threads. Its main use is to customise signal handling in
  your process, especially in the presence of threads.

  For example, you could ignore signals by default in all threads (and specify
  `EVFLAG_NOSIGMASK`_ when creating any loops), and in one thread, wait for
  signals, then "deliver" them to libev by calling `feed_signal()`_.

__version__
  mood.event's version.

.. _Error:

Error
  Raised when an error specific to mood.event happens.


Loop
----

Loop([flags=EVFLAG_AUTO, callback=None, data=None, io_interval=0.0, timeout_interval=0.0])
  * flags (int)
      Can be used to specify special behaviour or specific backends to use.
      See `Loop flags`_ for more details.

  * callback (callable or ``None``)
      If omitted or ``None`` the loop will fall back to its default behaviour of
      calling `Loop.invoke()`_ when required. If it is a callable, then the loop
      will execute it instead and it becomes the user's responsibility to call
      `Loop.invoke()`_ to invoke pending events. See also `Loop.callback`_.

  * data (object)
      Any Python object you might want to attach to the loop (will be stored in
      `Loop.data`_).

  * io_interval (float)
      See `Loop.io_interval`_.

  * timeout_interval (float)
      See `Loop.timeout_interval`_.

  Instanciates a new event loop that is always distinct from the *default loop*.
  Unlike the *default loop*, it cannot handle `Child`_ watchers, and attempts to
  do so will raise an `Error`_.

  One common way to use libev with threads is indeed to create one `Loop`_ per
  thread, and use the *default loop* in the "main" or "initial" thread.

  **See also:** `FUNCTIONS CONTROLLING EVENT LOOPS
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#FUNCTIONS_CONTROLLING_EVENT_LOOPS>`_

  .. _Loop.start():

  start([flags]) -> bool
    * flags (int: 0)
        If *flags* is omitted or specified as ``0``, it will keep handling
        events until either no event watchers are active anymore or
        `Loop.stop()`_ was called.

        * EVRUN_NOWAIT
            A *flags* value of ``EVRUN_NOWAIT`` will look for new events, will
            handle those events and any already outstanding ones, but will not
            wait and block your process in case there are no events and will
            return after one iteration of the loop.
            This is sometimes useful to poll and handle new events while doing
            lengthy calculations, to keep the program responsive.

        * EVRUN_ONCE
            A *flags* value of ``EVRUN_ONCE`` will look for new events (waiting
            if necessary) and will handle those and any already outstanding ones.
            It will block your process until at least one new event arrives
            (which could be an event internal to libev itself, so there is no
            guarantee that a user-registered callback will be called), and will
            return after one iteration of the loop.
            This is useful if you are waiting for some external event in
            conjunction with something not expressible using other libev
            watchers. However, a pair of `Prepare`_/`Check`_ watchers is usually
            a better approach for this kind of thing.

    This method usually is called after you have initialised all your watchers
    and you want to start handling events.

    Returns ``False`` if there are no more active watchers (which usually means
    "all jobs done" or "deadlock"), and ``True`` in all other cases (which
    usually means you should call `Loop.start()`_ again).

    **Note:** An explicit `Loop.stop()`_ is usually better than relying on all
    watchers being stopped when deciding if a program has finished (especially
    in interactive programs).

  .. _Loop.stop():

  stop([how])
    * how (int: EVBREAK_ONE)
        * EVBREAK_ONE
            If *how* is omitted or specified as ``EVBREAK_ONE`` it will make the
            innermost `Loop.start()`_ call return.

        * EVBREAK_ALL
            A *how* value of ``EVBREAK_ALL`` will make all nested
            `Loop.start()`_ calls return.

    Can be used to make a call to `Loop.start()`_ return early (but only after
    it has processed all outstanding events).

  .. _Loop.invoke():

  invoke()
    This method will simply invoke all pending watchers while resetting their
    pending state. Normally, the loop does this automatically when required, but
    when setting the `Loop.callback`_ attribute this call comes in handy.

  .. _Loop.reset():

  reset()
    This method sets a flag that causes subsequent loop iterations to
    reinitialise the kernel state for backends that have one. You can call it
    anytime you are allowed to start or stop watchers (except inside a
    `Prepare`_ callback), but it makes most sense after forking, in the child
    process. You **must** call it (or use `EVFLAG_FORKCHECK`_) in the child
    before calling `Loop.resume()`_ or `Loop.start()`_.

    In addition, if you want to reuse a loop (via this method or
    `EVFLAG_FORKCHECK`_), you also have to ignore ``SIGPIPE``.

    On the other hand, you only need to call this method in the child process if
    and only if you want to use the event loop in the child. If you just
    ``fork + exec`` or create a new loop in the child, you don't have to call it
    at all.

    **TODO:** add an example.

  .. _Loop.now():

  now() -> float
    Returns the current "event loop time", which is the time the event loop
    received events and started processing them. This timestamp does not change
    as long as callbacks are being processed, and this is also the base time
    used for relative timers. You can treat it as the timestamp of the event
    occurring (or more correctly, libev finding out about it).

  .. _Loop.update():

  update()
    Establishes the current time by querying the kernel, updating the time
    returned by `Loop.now()`_ in the process. This is a costly operation and is
    usually done automatically within the loop.
    This method is rarely useful, but when some event callback runs for a very
    long time without entering the event loop, updating libev's idea of the
    current time is a good idea.

    **See also:** `The special problem of time updates
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_time_updates>`_

  .. _Loop_suspend_resume:

  suspend()/resume()
    These two methods suspend and resume an event loop, for use when the loop is
    not used for a while and timeouts should not be processed.
    A typical use case would be an interactive program such as a game: when the
    user presses ``Control-z`` to suspend the game and resumes it an hour later
    it would be best to handle timeouts as if no time had actually passed while
    the program was suspended. This can be achieved by calling `Loop.suspend()`_
    in your ``SIGTSTP`` handler, sending yourself a ``SIGSTOP`` and calling
    `Loop.resume()`_ directly afterwards to resume timer processing.
    Effectively, all `Timer`_ watchers will be delayed by the time spent between
    `Loop.suspend()`_ and `Loop.resume()`_, and all `Periodic`_ watchers will be
    rescheduled (that is, they will lose any events that would have occurred
    while suspended).

    After calling `Loop.suspend()`_ you **must not** call any method on the
    given loop other than `Loop.resume()`_, and you **must not** call
    `Loop.resume()`_ without a previous call to `Loop.suspend()`_.

    **Note:** Calling `Loop.suspend()`_/`Loop.resume()`_ has the side effect of
    updating the event loop time (see `Loop.update()`_).

  .. _Loop.suspend(): `Loop_suspend_resume`_

  .. _Loop.resume(): `Loop_suspend_resume`_

  .. _Loop_unref_ref:

  unref()/ref()
    `Loop.unref()`_/`Loop.ref()`_ can be used to add or remove a reference count
    on the event loop: every watcher keeps one reference, and as long as the
    reference count is nonzero, the loop will not return on its own.
    This is useful when you have a watcher that you never intend to unregister,
    but that nevertheless should not keep the loop from returning. In such a
    case, call `Loop.unref()`_ after starting, and `Loop.ref()`_ before stopping
    it.
    As an example, libev itself uses this for its internal signal pipe: it is
    not visible to the user and should not keep the loop from exiting if no
    event watchers registered by it are active. It is also an excellent way to
    do this for generic recurring timers or from within third-party libraries.
    Just remember to `Loop.unref()`_ after start and `Loop.ref()`_ before stop
    (but only if the watcher wasn't active before, or was active before,
    respectively. Note also that libev might stop watchers itself (e.g.
    non-repeating timers) in which case you have to `Loop.ref()`_ in the
    callback).

    **Note:** These methods are not related to Python reference counting.

  .. _Loop.unref(): `Loop_unref_ref`_

  .. _Loop.ref(): `Loop_unref_ref`_

  verify()
    This method only does something when ``EV_VERIFY`` support has been compiled
    in (which is the default for non-minimal builds). It tries to go through all
    internal structures and checks them for validity. If anything is found to be
    inconsistent, it will print an error message to standard error and call
    ``abort``.
    This can be used to catch bugs inside libev itself: under normal
    circumstances, this method should never abort.

  .. _Loop.callback:

  callback
    The current invoke pending callback, its signature must be:

    callback(loop)
      * loop (`Loop`_)
          this loop.

    This overrides the invoke pending functionality of the loop: instead of
    invoking all pending watchers when there are any, the loop will call this
    callback instead (use `Loop.invoke()`_ if you want to invoke all pending
    watchers). This is useful, for example, when you want to invoke the actual
    watchers inside another context (another thread etc.).

    **Warning:** Any unhandled exception will **stop the loop**.

    If you want to reset the callback, set it to ``None``.

  .. _Loop.data:

  data
    loop data.

  .. _Loop_intervals:

  io_interval/timeout_interval
    These two attributes influence the time that libev will spend waiting for
    events. Both time intervals are by default ``0.0``, meaning that libev will
    try to invoke `Timer`_/`Periodic`_ and `Io`_ callbacks with minimum latency.
    Setting these to a higher value (the interval must be >= ``0.0``) allows
    libev to delay invocation of `Io`_ and `Timer`_/`Periodic`_ callbacks to
    increase efficiency of loop iterations (or to increase power-saving
    opportunities).
    The idea is that sometimes your program runs just fast enough to handle one
    (or very few) event(s) per loop iteration. While this makes the program
    responsive, it also wastes a lot of CPU time to poll for new events,
    especially with backends like ``select`` which have a high overhead for the
    actual polling but can deliver many events at once.

    By setting a higher *io_interval* you allow libev to spend more time
    collecting `Io`_ events, so you can handle more events per iteration, at the
    cost of increasing latency. Timeouts (both `Periodic`_ and `Timer`_) will
    not be affected. Setting this to a non-zero value will introduce an
    additional `sleep()`_ call into most loop iterations. The sleep time ensures
    that libev will not poll for `Io`_ events more often than once per this
    interval, on average (as long as the host time resolution is good enough).
    Many (busy) programs can usually benefit by setting the *io_interval* to a
    value near ``0.1`` or so, which is often enough for interactive servers (of
    course not for games), likewise for timeouts. It usually doesn't make much
    sense to set it to a value lower than ``0.01``, as this approaches the
    timing granularity of most systems. Note that if you do transactions with
    the outside world and you can't increase the parallelism, then this setting
    will limit your transaction rate (if you need to poll once per transaction
    and the *io_interval* is ``0.01``, then you can't do more than ``100``
    transactions per second).

    Likewise, by setting a higher *timeout_interval* you allow libev to spend
    more time collecting timeouts, at the expense of increased
    latency/jitter/inexactness (the watcher callback will be called later).
    `Io`_ watchers will not be affected. Setting this to a non-zero value will
    not introduce any overhead in libev.
    Setting the *timeout_interval* can improve the opportunity for saving power,
    as the program will "bundle" timer callback invocations that are "near" in
    time together, by delaying some, thus reducing the number of times the
    process sleeps and wakes up again. Another useful technique to reduce
    iterations/wake-ups is to use `Periodic`_ watchers and make sure they fire
    on, say, one-second boundaries only.

  .. _Loop.io_interval: `Loop_intervals`_

  .. _Loop.timeout_interval: `Loop_intervals`_

  default (read only)
    ``True`` if the loop is the *default loop*, ``False`` otherwise.

  backend (read only)
    One of the `backends`_ flags indicating the event backend in use.

  pending (read only)
    The number of pending watchers.

  iteration (read only)
    The current iteration count for the loop, which is identical to the number
    of times libev did poll for new events. It starts at ``0`` and happily wraps
    around with enough iterations.
    This value can sometimes be useful as a generation counter of sorts (it
    "ticks" the number of loop iterations), as it roughly corresponds to
    `Prepare`_ and `Check`_ calls - and is incremented between the prepare and
    check phases.

  depth (read only)
    The number of times `Loop.start()`_ was entered minus the number of times
    `Loop.start()`_ was exited normally, in other words, the recursion depth.
    Outside `Loop.start()`_, this number is ``0``. In a callback, this number is
    ``1``, unless `Loop.start()`_ was invoked recursively (or from another
    thread), in which case it is higher.

`Loop`_ *flags*
^^^^^^^^^^^^^^^

.. _EVFLAG_AUTO:

* EVFLAG_AUTO
    The default *flags* value.

* EVFLAG_NOENV
    If this flag bit is or'ed into the *flags* value (or the program runs
    ``setuid`` or ``setgid``) then libev will not look at the environment
    variable ``LIBEV_FLAGS``. Otherwise (the default), ``LIBEV_FLAGS`` will
    override the *flags* completely if it is found in the environment. This is
    useful to try out specific backends to test their performance, to work
    around bugs.

.. _EVFLAG_FORKCHECK:

* EVFLAG_FORKCHECK
    Instead of calling `Loop.reset()`_ manually after a fork, you can also make
    libev check for a fork in each iteration by enabling this flag.
    This works by calling ``getpid`` on every iteration of the loop, and thus
    this might slow down your event loop if you do a lot of loop iterations and
    little real work, but is usually not noticeable.
    The big advantage of this flag is that you can forget about fork (and forget
    about forgetting to tell libev about forking, although you still have to
    ignore ``SIGPIPE``) when you use it.
    This flag setting cannot be overridden or specified in the ``LIBEV_FLAGS``
    environment variable.

* EVFLAG_SIGNALFD
    When this flag is specified, then libev will attempt to use the ``signalfd``
    API for its `Signal`_ (and `Child`_) watchers. This API delivers signals
    synchronously, which makes it both faster and might make it possible to get
    the queued signal data. It can also simplify signal handling with threads,
    as long as you properly block signals in your threads that are not
    interested in handling them.
    ``signalfd`` will not be used by default as this changes your signal mask.

.. _EVFLAG_NOSIGMASK:

* EVFLAG_NOSIGMASK
    When this flag is specified, then libev will avoid modifying the signal
    mask. Specifically, this means you have to make sure signals are unblocked
    when you want to receive them
    This behaviour is useful when you want to do your own signal handling, or
    want to handle signals only in specific threads and want to avoid libev
    unblocking the signals.
    It's also required by POSIX in a threaded program, as libev calls
    ``sigprocmask``, whose behaviour is officially unspecified.
    This flag's behaviour will become the default in future versions of libev.

Backends
++++++++

.. _EVBACKEND_SELECT:

* EVBACKEND_SELECT
    *Availability:* POSIX

    The standard ``select`` backend. Not completely standard, as libev tries to
    roll its own ``fd_set`` with no limits on the number of fds, but if that
    fails, expect a fairly low limit on the number of fds when using this
    backend. It doesn't scale too well (O(*highest_fd*)), but is usually the
    fastest backend for a low number of (low-numbered) fds.

    To get good performance out of this backend you need a high amount of
    parallelism (most of the file descriptors should be busy). If you are
    writing a server, you should ``accept`` in a loop to accept as many
    connections as possible during one iteration. You might also want to have a
    look at `Loop.io_interval`_ to increase the amount of readiness
    notifications you get per iteration.

    This backend maps `EV_READ`_ to the ``readfds`` set and `EV_WRITE`_ to the
    ``writefds`` set.

.. _EVBACKEND_POLL:

* EVBACKEND_POLL
    *Availability:* POSIX

    The ``poll`` backend. It's more complicated than ``select``, but handles
    sparse fds better and has no artificial limit on the number of fds you can
    use (except it will slow down considerably with a lot of inactive fds).
    It scales similarly to select, i.e. O(*total_fds*).

    See `EVBACKEND_SELECT`_ for performance tips.

    This backend maps `EV_READ`_ to ``POLLIN | POLLERR | POLLHUP``, and
    `EV_WRITE`_ to ``POLLOUT | POLLERR | POLLHUP``.

.. _EVBACKEND_EPOLL:

* EVBACKEND_EPOLL
    *Availability:* Linux

    Use the linux-specific ``epoll`` interface. For few fds, this backend is a
    little bit slower than ``poll`` and ``select``, but it scales phenomenally
    better. While ``poll`` and ``select`` usually scale like O(*total_fds*)
    where *total_fds* is the total number of fds (or the highest fd), ``epoll``
    scales either O(*1*) or O(*active_fds*).

    While stopping, setting and starting an `Io`_ watcher in the same iteration
    will result in some caching, there is still a system call per such incident,
    so its best to avoid that. Also, ``dup``'ed file descriptors might not work
    very well if you register events for both file descriptors.
    Best performance from this backend is achieved by not unregistering all
    watchers for a file descriptor until it has been closed, if possible, i.e.
    keep at least one watcher active per fd at all times. Stopping and starting
    a watcher (without re-setting it) also usually doesn't cause extra overhead.
    A fork can both result in spurious notifications as well as in libev having
    to destroy and recreate the epoll object (in both the parent and child
    processes), which can take considerable time (one syscall per file
    descriptor), is hard to detect, and thus should be avoided.
    All this means that, in practice, ``select`` can be as fast or faster than
    ``epoll`` for maybe up to a hundred file descriptors, depending on usage.

    While nominally embeddable in other event loops, this feature is broken in
    all kernel versions tested so far.

    This backend maps `EV_READ`_ and `EV_WRITE`_ the same way `EVBACKEND_POLL`_
    does.

* EVBACKEND_KQUEUE
    *Availability:* most BSD clones

    Due to a number of bugs and inconsistencies between BSDs implementations,
    ``kqueue`` is not being "auto-detected" unless you explicitly specify it in
    the *flags* or libev was compiled on a known-to-be-good (-enough) system
    like NetBSD. It scales the same way the ``epoll`` backend does.

    While stopping, setting and starting an `Io`_ watcher does never cause an
    extra system call as with `EVBACKEND_EPOLL`_, it still adds up to two event
    changes per incident. Support for ``fork`` is bad (you might have to leak
    fds on fork) and it drops fds silently in similarly hard to detect cases.
    This backend usually performs well under most conditions.

    You can still embed ``kqueue`` into a normal ``poll`` or ``select`` backend
    and use it only for sockets (after having made sure that sockets work with
    ``kqueue`` on the target platform). See `Embed`_ watchers for more info.

    This backend maps `EV_READ`_ into an ``EVFILT_READ`` kevent with
    ``NOTE_EOF``, and `EV_WRITE`_ into an ``EVFILT_WRITE`` kevent with
    ``NOTE_EOF``.

* EVBACKEND_DEVPOLL
    *Availability:* Solaris 8

    This is not implemented yet (and might never be). According to reports,
    ``/dev/poll`` only supports sockets and is not embeddable, which would limit
    the usefulness of this backend immensely.

* EVBACKEND_PORT
    *Availability:* Solaris 10

    This uses the Solaris 10 ``event port`` mechanism. It's slow, but it scales
    very well (O(*active_fds*)).
    While this backend scales well, it requires one system call per active file
    descriptor per loop iteration. For small and medium numbers of file
    descriptors a "slow" `EVBACKEND_SELECT`_ or `EVBACKEND_POLL`_ backend might
    perform better.

    On the positive side, this backend actually performed fully to specification
    in all tests and is fully embeddable.

    This backend maps `EV_READ`_ and `EV_WRITE`_ the same way `EVBACKEND_POLL`_
    does.

* EVBACKEND_ALL
    Try all backends (even potentially broken ones that wouldn't be tried with
    `EVFLAG_AUTO`_). Since this is a mask, you can do stuff such as:

    .. code:: python

        EVBACKEND_ALL & ~EVBACKEND_KQUEUE

    It is definitely not recommended to use this flag, use whatever
    `recommended_backends()`_ returns, or simply do not specify a backend at all.

* EVBACKEND_MASK
    Not a backend at all, but a mask to select all backend bits from a *flags*
    value, in case you want to mask out any backends from *flags* (e.g. when
    modifying the ``LIBEV_FLAGS`` environment variable).

Watcher methods
^^^^^^^^^^^^^^^

The following methods are just a convenient way to instantiate watchers attached
to the loop (although they do not take keyword arguments).

io(fd, events, callback[, data, priority])
  Returns an `Io`_ watcher.

timer(after, repeat, callback[, data, priority])
  Returns a `Timer`_ watcher.

periodic(offset, interval, callback[, data, priority])
  Returns a `Periodic`_ watcher.

scheduler(scheduler, callback[, data, priority])
  Returns a `Scheduler`_ watcher.

signal(signum, callback[, data, priority])
  Returns a `Signal`_ watcher.

child(pid, trace, callback[, data, priority])
  Returns a `Child`_ watcher.

idle(callback[, data, priority])
  Returns an `Idle`_ watcher.

prepare(callback[, data, priority])
  Returns a `Prepare`_ watcher.

check(callback[, data, priority])
  Returns a `Check`_ watcher.

embed(other[, callback, data, priority])
  Returns an `Embed`_ watcher.

fork(callback[, data, priority])
  Returns a `Fork`_ watcher.

async(callback[, data, priority])
  Returns an `Async`_ watcher.


Watchers
--------

**See also:** `ANATOMY OF A WATCHER
<http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#ANATOMY_OF_A_WATCHER>`_


Common methods and attributes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _Watcher.start():

start()
  Starts (activates) the watcher. Only active watchers will receive events. If
  the watcher is already active nothing will happen.

.. _Watcher.stop():

stop()
  Stops the watcher if active, and clears the pending status (whether the
  watcher was active or not).
  It is possible that stopped watchers are pending - for example, non-repeating
  timers are being stopped when they become pending - but calling
  `Watcher.stop()`_ ensures that the watcher is neither active nor pending.

invoke(revents)
  * revents (int)
      See `Events received`_ for valid values.

  Invoke the watcher callback with the given *revents*.

clear() -> int
  If the watcher is pending, this method clears its pending status and returns
  its *revents* bitset (as if its callback was invoked). If the watcher isn't
  pending it does nothing and returns ``0``.
  Sometimes it can be useful to "poll" a watcher instead of waiting for its
  callback to be invoked, which can be accomplished with this method.

.. _Watcher.feed():

feed(revents)
  * revents (int)
      See `Events received`_ for valid values.

  Feeds the given *revents* set into the event loop, as if the specified event
  had happened for the watcher.

.. _Watcher.loop:

loop (read only)
  `Loop`_ object responsible for the watcher.

.. _Watcher.callback:

callback
  The current watcher callback, its signature must be:

  callback(watcher, revents)
    * watcher (one of `Watcher types`_)
        this watcher.

    * revents (int)
        See `Events received`_ for valid values.

  As a rule you should not let a callback return with unhandled exceptions. The
  loop "does not know" how to correctly handle an exception happening in **your**
  callback (it depends largely on what **you** are doing), so, by default, it
  will just print a warning and suppress it.
  If you want to act on an exception, you're better off doing it in the callback
  (where you are allowed to do anything needed, like logging, stopping,
  restarting the loop, etc.). Example:

  .. code:: python

      def mycallback(watcher, revents):
          try:
              pass # do something interesting
          except Exception as err:
              watcher.stop() # stop the watcher
              watcher.loop.stop() # stop the loop
              raise err # and finally raise err

  If you have a lot of callbacks, use decorators:

  .. code:: python

      import logging

      def mydecorator(func):
          def wrap(watcher, revents):
              try:
                  func(watcher, revents)
              except RuntimeError: # these are not fatal
                  logging.exception("stopping {0}".format(watcher)) # this will also log the traceback
                  watcher.stop() # stop the watcher but let the loop continue on its merry way
              except Exception as err: # all other exceptions are fatal
                  watcher.stop() # stop the watcher
                  watcher.loop.stop() # stop the loop
                  raise err # and finally raise err
          return wrap

      @mydecorator
      def mycallback(watcher, revents):
          pass #do something interesting

  **Note:** As a convenience mood.event provides a `@fatal`_ decorator. If a
  callback decorated with `@fatal`_ raises an exception the loop is stopped and
  the exception raised. Contrast:

  .. code:: python

      >>> from signal import SIGINT
      >>> from mood.event import Loop, EV_TIMER, EV_SIGNAL
      >>>
      >>> def mycallback(watcher, revents):
      ...     if (revents & EV_TIMER):
      ...         raise Exception("TEST")
      ...     elif (revents & EV_SIGNAL):
      ...         watcher.loop.stop()
      ...
      >>>
      >>> loop = Loop()
      >>> timer = loop.timer(0, 2, mycallback)
      >>> timer.start()
      >>> sig = loop.signal(SIGINT, mycallback) # will catch KeyboardInterrupt
      >>> sig.start()
      >>> loop.start()
      Exception ignored in: <function mycallback at 0x7f4a4b057f28>
      Traceback (most recent call last):
        File "<stdin>", line 3, in mycallback
      Exception: TEST
      Exception ignored in: <function mycallback at 0x7f4a4b057f28>
      Traceback (most recent call last):
        File "<stdin>", line 3, in mycallback
      Exception: TEST
      Exception ignored in: <function mycallback at 0x7f4a4b057f28>
      Traceback (most recent call last):
        File "<stdin>", line 3, in mycallback
      Exception: TEST
      ^CTrue
      >>>

  versus:

  .. code:: python

      >>> from mood.event import Loop, fatal
      >>>
      >>> @fatal
      ... def mycallback(watcher, revents):
      ...     raise Exception("TEST")
      ...
      >>>
      >>> loop = Loop()
      >>> timer = loop.timer(0, 2, mycallback)
      >>> timer.start()
      >>> loop.start()
      Traceback (most recent call last):
        File "<stdin>", line 1, in <module>
        File "<stdin>", line 3, in mycallback
      Exception: TEST
      >>>

.. _Watcher.data:

data
  watcher data.

.. _Watcher.priority:

priority
  Set and query the priority of the watcher. The priority is a small integer
  between `EV_MINPRI`_ and `EV_MAXPRI`_. Pending watchers with higher priority
  will be invoked before watchers with lower priority, but priority will not
  keep watchers from being executed. If you need to suppress invocation when
  higher priority events are pending you need to look at `Idle`_ watchers, which
  provide this functionality.

  Setting a priority outside the range of `EV_MINPRI`_ to `EV_MAXPRI`_ is fine,
  as long as you do not mind that the priority value you query might or might
  not have been clamped to the valid range.

  The default priority used by watchers when no priority has been set is always
  ``0``.

  **Note:** You must not change the priority of a watcher as long as it is
  active or pending.

  **See also:** `WATCHER PRIORITY MODELS
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#WATCHER_PRIORITY_MODELS>`_

active (read only)
  ``True`` if the watcher is active (i.e. it has been started and
  not yet been stopped), ``False`` otherwise.

  **Note:** As long as a watcher is active you must not modify it.

pending (read only)
  ``True`` if the watcher is pending (i.e. it has outstanding events but its
  callback has not yet been invoked), ``False`` otherwise.

  **Note:** As long as a watcher is pending (but not active) you must not change
  its priority.


Events received
^^^^^^^^^^^^^^^

.. _EV_READ:

* EV_IO/EV_READ
    The file descriptor in the `Io`_ watcher has become readable.

.. _EV_WRITE:

* EV_WRITE
    The file descriptor in the `Io`_ watcher has become writable.

* EV_TIMER
    The `Timer`_ watcher has timed out.

* EV_PERIODIC
    The `Periodic`_ watcher has timed out.

* EV_SIGNAL
    The signal specified in the `Signal`_ watcher has been received by a thread.

* EV_CHILD
    The pid specified in the `Child`_ watcher has received a status change.

* EV_IDLE
    The `Idle`_ watcher has determined that you have nothing better to do.

* EV_PREPARE/EV_CHECK
    All `Prepare`_ watchers are invoked just before the loop starts to gather
    new events, and all `Check`_ watchers are queued (not invoked) just after
    the loop has gathered them, but before it queues any callbacks for any
    received events. That means `Prepare`_ watchers are the last watchers
    invoked before the event loop sleeps or polls for new events, and `Check`_
    watchers will be invoked before any other watchers of the same or lower
    priority within an event loop iteration.
    Callbacks of both watcher types can start and stop as many watchers as they
    want, and all of them will be taken into account (for example, a `Prepare`_
    watcher might start an `Idle`_ watcher to keep the loop from blocking).

* EV_EMBED
    The embedded event loop specified in the `Embed`_ watcher needs attention.

* EV_FORK
    The event loop has been resumed in the child process after fork (see `Fork`_).

* EV_ASYNC
    The given `Async`_ watcher has been asynchronously notified.

* EV_CUSTOM
    Not ever sent (or otherwise used) by libev itself, but can be freely used by
    users to signal watchers (e.g. via `Watcher.feed()`_).

* EV_ERROR
    An unspecified error has occurred, the watcher has been stopped. This might
    happen because the watcher could not be properly started because libev ran
    out of memory, a file descriptor was found to be closed or any other problem.

    **Warning:** mood.event handle this event as a fatal error. On receiving
    this event the loop and the watcher **will be stopped** (the callback **will
    not be invoked**). In practice, users should never receive this event (still
    present for testing puposes).


Priorities
^^^^^^^^^^

.. _EV_MINPRI:

* EV_MINPRI
    default: ``-2``.

.. _EV_MAXPRI:

* EV_MAXPRI
    default: ``2``.


Watcher types
^^^^^^^^^^^^^

mood.event implements the following watcher types:


Io
++

Io(fd, events, loop, callback[, data=None, priority=0])
  * fd (int or object)
      The file descriptor to be monitored, can be an int or any Python object
      having a ``fileno`` method.

  * events (int)
      Either ``EV_READ``, ``EV_WRITE`` or ``EV_READ | EV_WRITE``.

  * loop (`Loop`_)
      Loop object responsible for this watcher (accessible through
      `Watcher.loop`_).

  * callback (callable)
      See `Watcher.callback`_.

  * data (object)
      Any Python object you might want to attach to the watcher (stored in
      `Watcher.data`_).

  * priority (int)
      See `Watcher.priority`_.

  `Io`_ watchers check whether a file descriptor is readable or writable in each
  iteration of the event loop, or, more precisely, when reading would not block
  the process and writing would at least be able to write some data. This
  behaviour is called level-triggering because you keep receiving events as long
  as the condition persists. Remember you can stop the watcher if you don't want
  to act on the event and neither want to receive future events.

  In general you can register as many read and/or write event watchers per fd as
  you want. Setting all file descriptors to non-blocking mode is also usually a
  good idea (but not required).

  Another thing you have to watch out for is that it is quite easy to receive
  "spurious" readiness notifications, that is, your callback might be called
  with `EV_READ`_ but a subsequent ``read`` will actually block because there is
  no data. It is very easy to get into this situation even with a relatively
  standard program structure. Thus it is best to always use non-blocking I/O: an
  extra ``read`` returning ``EAGAIN`` is far preferable to a program hanging
  until some data arrives.

  If you cannot run the fd in non-blocking mode, then you have to separately
  re-test whether a file descriptor is really ready with a known-to-be good
  interface such as ``poll``. Some people additionally use ``SIGALRM`` and an
  interval timer, just to be sure you won't block indefinitely.

  But really, best use non-blocking mode.

  **See also:** `ev_io - is this file descriptor readable or writable?
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_io_code_is_this_file_descrip>`_

  * `The special problem of disappearing file descriptors
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_disappearing_>`_
  * `The special problem of dup'ed file descriptors
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_dup_ed_file_d>`_
  * `The special problem of files
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_files>`_
  * `The special problem of fork
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_fork>`_
  * `The special problem of SIGPIPE
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_SIGPIPE>`_
  * `The special problem of accept()ing when you can't
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_accept_ing_wh>`_

  set(fd, events)
    * fd (int or object)
        The file descriptor to be monitored, can be an int or any Python object
        having a ``fileno`` method.

    * events (int)
        Either ``EV_READ``, ``EV_WRITE`` or ``EV_READ | EV_WRITE``.

    Configures the watcher.

  fd (read only)
    The file descriptor being watched.

  events (read only)
    The events being watched.


Timer
+++++

Timer(after, repeat, loop, callback[, data=None, priority=0])
  * after (float)
      Configure the timer to trigger after *after* seconds.

  * repeat (float)
      If *repeat* is ``0.0``, then it will automatically be stopped once the
      timeout is reached. If it is positive, then the timer will automatically
      be configured to trigger again every *repeat* seconds later, again and
      again, until stopped manually.

  * loop (`Loop`_)
      Loop object responsible for this watcher (accessible through
      `Watcher.loop`_).

  * callback (callable)
      See `Watcher.callback`_.

  * data (object)
      Any Python object you might want to attach to the watcher (stored in
      `Watcher.data`_).

  * priority (int)
      See `Watcher.priority`_.

  `Timer`_ watchers are simple relative timers that generate an event after a
  given time, and optionally repeating in regular intervals after that.

  The timers are based on real time, that is, if you register an event that
  times out after an hour and you reset your system clock to January last year,
  it will still time out after (roughly) one hour. "Roughly" because detecting
  time jumps is hard, and some inaccuracies are unavoidable.

  The callback is guaranteed to be invoked only **after** its timeout has passed
  (**not at**, so on systems with very low-resolution clocks this might
  introduce a small delay). If multiple timers become ready during the same loop
  iteration then the ones with earlier time-out values are invoked before ones
  of the same priority with later time-out values (but this is no longer true
  when a callback calls `Loop.start()`_ recursively).

  The timer itself will do a best-effort at avoiding drift, that is, if you
  configure a timer to trigger every 10 seconds, then it will normally trigger
  at exactly 10 second intervals. If, however, your program cannot keep up with
  the timer (because it takes longer than those 10 seconds to do stuff) the
  timer will not fire more than once per event loop iteration.

  **See also:** `ev_timer - relative and optionally repeating timeouts
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_timer_code_relative_and_opti>`_

  * `Be smart about timeouts
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Be_smart_about_timeouts>`_
  * `The special problem of being too early
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_being_too_ear>`_
  * `The special problem of time updates
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_time_updates>`_
  * `The special problem of unsynchronised clocks
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_unsynchronise>`_
  * `The special problems of suspended animation
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problems_of_suspended_an>`_

  set(after, repeat)
    * after (float)
        Configure the timer to trigger after *after* seconds.

    * repeat (float)
        If *repeat* is ``0.0``, then it will automatically be stopped once the
        timeout is reached. If it is positive, then the timer will automatically
        be configured to trigger again every *repeat* seconds later, again and
        again, until stopped manually.

    Configures the watcher.

  .. _Timer.reset():

  reset()
    This will act as if the timer timed out, and restarts it again if it is
    repeating. It basically works like calling `Watcher.stop()`_, updating the
    timeout to the *repeat* value and calling `Watcher.start()`_. The exact
    semantics are:

    * if the timer is pending, the pending status is always cleared.
    * if the timer is started but non-repeating, stop it (as if it timed out,
      without invoking it).
    * if the timer is repeating, make the *repeat* value the new timeout and
      start the timer, if necessary.

    **See also:** `Be smart about timeouts
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Be_smart_about_timeouts>`_
    for a usage example.

  repeat
    The current *repeat* value. Will be used each time the watcher times out or
    `Timer.reset()`_ is called, and determines the next timeout (if any), which
    is also when any modifications are taken into account.

  remaining (read only)
    The remaining time until a timer fires. If the timer is active, then this
    time is relative to the current event loop time, otherwise it's the timeout
    value currently configured.

    That is, after instanciating a `Timer`_ with an *after* value of ``5.0`` and
    a *repeat* value of ``7.0``, *remaining* is ``5.0``. When the timer is
    started and one second passes, *remaining* will be ``4.0``. When the
    timer expires and is restarted, it will be roughly ``7.0`` (likely slightly
    less as callback invocation takes some time, too), and so on.


Periodic
++++++++

Periodic(offset, interval, loop, callback[, data=None, priority=0])
  * offset (float)
      TODO.

  * interval (float)
      TODO.

  * loop (`Loop`_)
      Loop object responsible for this watcher (accessible through
      `Watcher.loop`_).

  * callback (callable)
      See `Watcher.callback`_.

  * data (object)
      Any Python object you might want to attach to the watcher (stored in
      `Watcher.data`_).

  * priority (int)
      See `Watcher.priority`_.

  TODO.

  **See also:** `ev_periodic - to cron or not to cron?
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_periodic_code_to_cron_or_not>`_

  set(offset, interval)
    * offset (float)
        TODO.

    * interval (float)
        TODO.

    Configures the watcher.

  reset()
    TODO.

  offset
    TODO.

  interval
    TODO.

  at (read only)
    TODO.

`Periodic`_ modes of operation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Absolute timer
##############

TODO.

Repeating interval timer
########################

TODO.


Scheduler
+++++++++

Scheduler(scheduler, loop[, callback=None, data=None, priority=0])
  * scheduler (callable)
      TODO.

  * loop (`Loop`_)
      Loop object responsible for this watcher (accessible through
      `Watcher.loop`_).

  * callback (callable)
      See `Watcher.callback`_.

  * data (object)
      Any Python object you might want to attach to the watcher (stored in
      `Watcher.data`_).

  * priority (int)
      See `Watcher.priority`_.

  TODO.

  **See also:** `ev_periodic - to cron or not to cron?
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_periodic_code_to_cron_or_not>`_

  reset()
    TODO.

  scheduler
    TODO.

  at (read only)
    TODO.


Signal
++++++

Signal(signum, loop, callback[, data=None, priority=0])
  * signum (int)
      TODO.

  * loop (`Loop`_)
      Loop object responsible for this watcher (accessible through
      `Watcher.loop`_).

  * callback (callable)
      See `Watcher.callback`_.

  * data (object)
      Any Python object you might want to attach to the watcher (stored in
      `Watcher.data`_).

  * priority (int)
      See `Watcher.priority`_.

  TODO.

  **See also:** `ev_signal - signal me when a signal gets signalled!
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_signal_code_signal_me_when_a>`_

  * `The special problem of inheritance over fork/execve/pthread_create
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_inheritance_o>`_
  * `The special problem of threads signal handling
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_threads_signa>`_

  set(signum)
    * signum (int)
        TODO.

    Configures the watcher.

  signum (read only)
    TODO.


Child
+++++

Child(pid, trace, loop, callback[, data=None, priority=0])
  * pid (int)
      TODO.

  * trace (bool)
      TODO.

  * loop (`Loop`_)
      Loop object responsible for this watcher (accessible through
      `Watcher.loop`_).

  * callback (callable)
      See `Watcher.callback`_.

  * data (object)
      Any Python object you might want to attach to the watcher (stored in
      `Watcher.data`_).

  * priority (int)
      See `Watcher.priority`_.

  TODO.

  **See also:** `ev_child - watch out for process status changes
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_child_code_watch_out_for_pro>`_

  * `Process Interaction
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Process_Interaction>`_
  * `Overriding the Built-In Processing
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Overriding_the_Built_In_Processing>`_
  * `Stopping the Child Watcher
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Stopping_the_Child_Watcher>`_

  set(pid, trace)
    * pid (int)
        TODO.

    * trace (bool)
        TODO.

    Configures the watcher.

  pid (read only)
    TODO.

  rpid
    TODO.

  rstatus
    TODO.


Idle
++++

Idle(loop, callback[, data=None, priority=0])
  * loop (`Loop`_)
      Loop object responsible for this watcher (accessible through
      `Watcher.loop`_).

  * callback (callable)
      See `Watcher.callback`_.

  * data (object)
      Any Python object you might want to attach to the watcher (stored in
      `Watcher.data`_).

  * priority (int)
      See `Watcher.priority`_.

  TODO.

  **See also:** `ev_idle - when you've got nothing better to do...
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_idle_code_when_you_ve_got_no>`_

  * `Abusing an ev_idle watcher for its side-effect
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Abusing_an_code_ev_idle_code_watcher>`_


Prepare/Check
+++++++++++++

Prepare(loop, callback[, data=None, priority=0])
  .. ..

Check(loop, callback[, data=None, priority=0])
  * loop (`Loop`_)
      Loop object responsible for this watcher (accessible through
      `Watcher.loop`_).

  * callback (callable)
      See `Watcher.callback`_.

  * data (object)
      Any Python object you might want to attach to the watcher (stored in
      `Watcher.data`_).

  * priority (int)
      See `Watcher.priority`_.

  TODO.

  **See also:** `ev_prepare and ev_check - customise your event loop!
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_prepare_code_and_code_ev_che>`_

  * `Abusing an ev_check watcher for its side-effect
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Abusing_an_code_ev_check_code_watche>`_

.. _Check: `Prepare/Check`_

.. _Prepare: `Prepare/Check`_


Embed
+++++

Embed(other, loop[, callback=None, data=None, priority=0])
  * other (`Loop`_)
      TODO.

  * loop (`Loop`_)
      Loop object responsible for this watcher (accessible through
      `Watcher.loop`_).

  * callback (callable or ``None``)
      See `Watcher.callback`_.

  * data (object)
      Any Python object you might want to attach to the watcher (stored in
      `Watcher.data`_).

  * priority (int)
      See `Watcher.priority`_.

  TODO.

  **See also:** `ev_embed - when one backend isn't enough...
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_embed_code_when_one_backend_>`_

  * `ev_embed and fork
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_embed_code_and_fork>`_

  set(other)
    * other (`Loop`_)
        TODO.

    Configures the watcher.

  sweep()
    TODO.

  callback
    TODO.

  other (read only)
    TODO.


Fork
++++

Fork(loop, callback[, data=None, priority=0])
  * loop (`Loop`_)
      Loop object responsible for this watcher (accessible through
      `Watcher.loop`_).

  * callback (callable)
      See `Watcher.callback`_.

  * data (object)
      Any Python object you might want to attach to the watcher (stored in
      `Watcher.data`_).

  * priority (int)
      See `Watcher.priority`_.

  TODO.

  **See also:** `ev_fork - the audacity to resume the event loop after a fork
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_fork_code_the_audacity_to_re>`_

  * `The special problem of life after fork - how is it possible?
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_life_after_fo>`_


Async
+++++

Async(loop, callback[, data=None, priority=0])
  * loop (`Loop`_)
      Loop object responsible for this watcher (accessible through
      `Watcher.loop`_).

  * callback (callable)
      See `Watcher.callback`_.

  * data (object)
      Any Python object you might want to attach to the watcher (stored in
      `Watcher.data`_).

  * priority (int)
      See `Watcher.priority`_.

  TODO.

  **See also:** `ev_async - how to wake up an event loop
  <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_async_code_how_to_wake_up_an>`_

  * `Queueing
    <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Queueing>`_

  send()
    TODO.

  sent (read only)
    TODO.

