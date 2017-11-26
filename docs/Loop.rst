.. currentmodule:: mood.event

:py:class:`Loop` --- Event loop
===============================

.. py:class:: Loop([flags=EVFLAG_AUTO, callback=None, data=None, io_interval=0.0, timeout_interval=0.0])

    :param int flags: can be used to specify special :ref:`behaviour` or
        specific :ref:`backends` to use.

    :type callback: callable or None
    :param callback: if omitted or ``None`` the loop will fall back to its
        default behaviour of calling :py:meth:`invoke` when required.
        If it is a callable, then the loop will execute it instead and it
        becomes the user's responsibility to call :py:meth:`invoke` to invoke
        pending events. See also :py:attr:`callback`.

    :param object data: any Python object you might want to attach to the loop
        (will be stored in :py:attr:`data`).

    :param float io_interval: see :py:attr:`io_interval`.

    :param float timeout_interval: see :py:attr:`timeout_interval`.

    Instanciates a new event loop that is always distinct from the
    *default loop*. Unlike the *default loop*, it cannot handle
    :py:class:`Child` watchers, and attempts to do so will raise an
    :py:exc:`Error`.

    One common way to use libev with threads is indeed to create one
    :py:class:`Loop` per thread, and use the *default loop* (from
    :py:func:`loop`) in the "main" or "initial" thread.

    .. seealso::

        `FUNCTIONS CONTROLLING EVENT LOOPS
        <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#FUNCTIONS_CONTROLLING_EVENT_LOOPS>`_


    .. py:method:: start([flags])

        :param int flags: defaults to ``0``. See :ref:`Loop_start_flags`.

        :rtype: bool

        This method usually is called after you have initialised all your
        watchers and you want to start handling events.

        Returns ``False`` if there are no more active watchers (which usually
        means "all jobs done" or "deadlock"), and ``True`` in all other cases
        (which usually means you should call :py:meth:`start` again).

        .. note::

            An explicit :py:meth:`stop` is usually better than relying on all
            watchers being stopped when deciding if a program has finished
            (especially in interactive programs).


    .. py:method:: stop([how])

        :param int how: defaults to :py:data:`EVBREAK_ONE`. See
            :ref:`Loop_stop_how`.

        Can be used to make a call to :py:meth:`start` return early (but only
        after it has processed all outstanding events).


    .. py:method:: invoke

        This method will simply invoke all pending watchers while resetting
        their pending state. Normally, the loop does this automatically when
        required, but when setting the :py:attr:`callback` attribute this call
        comes in handy.


    .. py:method:: reset

        This method sets a flag that causes subsequent loop iterations to
        reinitialise the kernel state for backends that have one. You can call
        it anytime you are allowed to start or stop watchers (except inside a
        :py:class:`Prepare` callback), but it makes most sense after forking, in
        the child process. You **must** call it (or use
        :py:data:`EVFLAG_FORKCHECK`) in the child before calling
        :py:meth:`resume` or :py:meth:`start`. Again, you **have to call it**
        on **any** loop that you want to re-use after a fork, **even if you do
        not plan to use the loop in the parent**.

        In addition, if you want to reuse a loop (via this method or
        :py:data:`EVFLAG_FORKCHECK`), you also have to ignore ``SIGPIPE``.

        On the other hand, you only need to call this method in the child
        process if and only if you want to use the event loop in the child. If
        you just :py:func:`~os.fork` + :py:func:`exec*() <os.execl>` or create a
        new loop in the child, you don't have to call it at all.


    .. py:method:: now([update])

        :param bool update: defaults to ``False``.

        :rtype: float

        Returns the current "event loop time", which is the time the event loop
        received events and started processing them. This timestamp does not
        change as long as callbacks are being processed, and this is also the
        base time used for relative timers. You can treat it as the timestamp of
        the event occurring (or more correctly, libev finding out about it).

        When *update* is provided and ``True``, establishes the current time by
        querying the kernel, updating the time returned by :py:meth:`now` in the
        process. This is a costly operation and is usually done automatically
        within the loop.
        This parameter is rarely useful, but when some event callback runs for a
        very long time without entering the event loop, updating libev's idea of
        the current time is a good idea.

        .. seealso::

            `The special problem of time updates
            <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_time_updates>`_


    .. py:method:: suspend
                   resume

        These two methods suspend and resume an event loop, for use when the
        loop is not used for a while and timeouts should not be processed.
        A typical use case would be an interactive program such as a game: when
        the user presses :kbd:`Control-z` to suspend the game and resumes it an
        hour later it would be best to handle timeouts as if no time had
        actually passed while the program was suspended. This can be achieved by
        calling :py:meth:`suspend` in your ``SIGTSTP`` handler, sending yourself
        a ``SIGSTOP`` and calling :py:meth:`resume` directly afterwards to
        resume timer processing.
        Effectively, all :py:class:`Timer` watchers will be delayed by the time
        spent between :py:meth:`suspend` and :py:meth:`resume`, and all
        :py:class:`Periodic` watchers will be rescheduled (that is, they will
        lose any events that would have occurred while suspended).

        After calling :py:meth:`suspend` **you must not** call any method on the
        given loop other than :py:meth:`resume`, and **you must not** call
        :py:meth:`resume` without a previous call to :py:meth:`suspend`.

        .. note::

            Calling :py:meth:`suspend`/:py:meth:`resume` has the side effect of
            updating the event loop time (see :py:meth:`update`).


    .. py:method:: incref
                   decref

        :py:meth:`incref`/:py:meth:`decref` can be used to add or remove a
        reference count on the event loop: every watcher keeps one reference,
        and as long as the reference count is nonzero, the loop will not return
        on its own. This is useful when you have a watcher that you never intend
        to unregister, but that nevertheless should not keep the loop from
        returning. In such a case, call :py:meth:`decref` after starting, and
        :py:meth:`incref` before stopping it.
        As an example, libev itself uses this for its internal signal pipe: it
        is not visible to the user and should not keep the loop from exiting if
        no event watchers registered by it are active. It is also an excellent
        way to do this for generic recurring timers or from within third-party
        libraries.
        Just remember to :py:meth:`decref` after start and :py:meth:`incref`
        before stop (but only if the watcher wasn't active before, or was active
        before, respectively. Note also that libev might stop watchers itself
        (e.g. non-repeating timers) in which case you have to :py:meth:`incref`
        in the callback).

        .. note::

            These methods are not related to Python reference counting.


    .. py:method:: feed_fd_event(fd, revents)

        :type fd: int or object
        :param fd: can be an int or any Python object having a
            :py:meth:`~io.IOBase.fileno` method.

        :param int revents: See :ref:`Events_received` for valid values.

        Feed an event on the given fd, as if a file descriptor backend detected
        the given events.


    .. py:method:: feed_signal_event(signum)

        :param int signum: signal number to feed the event loop.

        Feed an event to the loop as if the given signal occurred.
        See also :py:func:`feed_signal`, which is async-safe.


    .. py:method:: verify

        This method only does something when :c:macro:`EV_VERIFY` support has
        been compiled in (which is the default for non-minimal builds). It tries
        to go through all internal structures and checks them for validity. If
        anything is found to be inconsistent, it will print an error message to
        standard error and call :c:func:`abort`.
        This can be used to catch bugs inside libev itself: under normal
        circumstances, this method should never abort.


    .. py:attribute:: callback

        The current invoke pending callback, its signature must be:

            .. py:function:: callback(loop)
                :noindex:

                :type loop: :py:class:`Loop`
                :param loop: this loop.

        This overrides the invoke pending functionality of the loop: instead of
        invoking all pending watchers when there are any, the loop will call
        this callback (use :py:meth:`invoke` if you want to invoke all pending
        watchers). This is useful, for example, when you want to invoke the
        actual watchers inside another context (another thread, etc.).

        If you want to reset the callback, set it to ``None``.

        .. warning::

            Any unhandled exception happening during execution of this callback
            will **stop the loop**.


    .. py:attribute:: data

        loop data.


    .. py:attribute:: io_interval
                      timeout_interval

        These two attributes influence the time that libev will spend waiting
        for events. Both time intervals are by default ``0.0``, meaning that
        libev will try to invoke :py:class:`Timer`/:py:class:`Periodic` and
        :py:class:`Io` callbacks with minimum latency.
        Setting these to a higher value (the interval must be >= ``0.0``) allows
        libev to delay invocation of :py:class:`Io` and :py:class:`Timer`/
        :py:class:`Periodic` callbacks to increase efficiency of loop iterations
        (or to increase power-saving opportunities).
        The idea is that sometimes your program runs just fast enough to handle
        one (or very few) event(s) per loop iteration. While this makes the
        program responsive, it also wastes a lot of CPU time to poll for new
        events, especially with backends like :manpage:`select(2)` which have a
        high overhead for the actual polling but can deliver many events at once.

        By setting a higher *io_interval* you allow libev to spend more time
        collecting :py:class:`Io` events, so you can handle more events per
        iteration, at the cost of increasing latency. Timeouts (both
        :py:class:`Periodic` and :py:class:`Timer`) will not be affected.
        Setting this to a non-zero value will introduce an additional
        :py:func:`sleep` call into most loop iterations. The sleep time ensures
        that libev will not poll for :py:class:`Io` events more often than once
        per this interval, on average (as long as the host time resolution is
        good enough).
        Many (busy) programs can usually benefit by setting the *io_interval* to
        a value near ``0.1`` or so, which is often enough for interactive
        servers (of course not for games), likewise for timeouts. It usually
        doesn't make much sense to set it to a value lower than ``0.01``, as
        this approaches the timing granularity of most systems. Note that if you
        do transactions with the outside world and you can't increase the
        parallelism, then this setting will limit your transaction rate (if you
        need to poll once per transaction and the *io_interval* is ``0.01``,
        then you can't do more than ``100`` transactions per second).

        Likewise, by setting a higher *timeout_interval* you allow libev to
        spend more time collecting timeouts, at the expense of increased
        latency/jitter/inexactness (the watcher callback will be called later).
        :py:class:`Io` watchers will not be affected. Setting this to a non-zero
        value will not introduce any overhead in libev.
        Setting the *timeout_interval* can improve the opportunity for saving
        power, as the program will "bundle" timer callback invocations that are
        "near" in time together, by delaying some, thus reducing the number of
        times the process sleeps and wakes up again. Another useful technique to
        reduce iterations/wake-ups is to use :py:class:`Periodic` watchers and
        make sure they fire on, say, one-second boundaries only.


    .. py:attribute:: default

        *Read only*

        ``True`` if the loop is the *default loop*, ``False`` otherwise.


    .. py:attribute:: iteration

        *Read only*

        The current iteration count for the loop, which is identical to the
        number of times libev did poll for new events. It starts at ``0`` and
        happily wraps around with enough iterations.
        This value can sometimes be useful as a generation counter of sorts (it
        "ticks" the number of loop iterations), as it roughly corresponds to
        :py:class:`Prepare` and :py:class:`Check` calls - and is incremented
        between the prepare and check phases.


    .. py:attribute:: depth

        *Read only*

        The number of times :py:meth:`start` was entered minus the number of
        times :py:meth:`start` was exited normally, in other words, the
        recursion depth.
        Outside :py:meth:`start`, this number is ``0``. In a callback, this
        number is ``1``, unless :py:meth:`start` was invoked recursively (or
        from another thread), in which case it is higher.


    .. py:attribute:: backend

        *Read only*

        One of the :ref:`backends` flags indicating the event backend in use.


    .. py:attribute:: pending

        *Read only*

        The number of pending watchers.


    The following methods are implemented as a convenience, they allow you to
    instantiate watchers directly attached to the loop:

    .. py:method:: __io__(fd, events, callback[, data=None, priority=0])

        :rtype: :py:class:`Io`


    .. py:method:: __timer__(after, repeat, callback[, data=None, priority=0])

        :rtype: :py:class:`Timer`


    .. py:method:: __periodic__(offset, interval, callback[, data=None, priority=0])

        :rtype: :py:class:`Periodic`


    .. py:method:: __scheduler__(reschedule, callback[, data=None, priority=0])

        :rtype: :py:class:`Scheduler`


    .. py:method:: __signal__(signum, callback[, data=None, priority=0])

        :rtype: :py:class:`Signal`


    .. py:method:: __child__(pid, trace, callback[, data=None, priority=0])

        :rtype: :py:class:`Child`


    .. py:method:: __idle__([callback=None, data=None, priority=0])

        :rtype: :py:class:`Idle`


    .. py:method:: __prepare__(callback[, data=None, priority=0])

        :rtype: :py:class:`Prepare`


    .. py:method:: __check__(callback[, data=None, priority=0])

        :rtype: :py:class:`Check`


    .. py:method:: __embed__(other[, callback=None, data=None, priority=0])

        :rtype: :py:class:`Embed`


    .. py:method:: __fork__(callback[, data=None, priority=0])

        :rtype: :py:class:`Fork`


    .. py:method:: __async__(callback[, data=None, priority=0])

        :rtype: :py:class:`Async`


.. _Loop_flags:

:py:class:`Loop` *flags*
------------------------


.. _behaviour:

behaviour
~~~~~~~~~

.. py:data:: EVFLAG_AUTO

    The default *flags* value.


.. py:data:: EVFLAG_NOENV

    If this flag bit is or'ed into the *flags* value (or the program runs
    :py:func:`~os.setuid` or :py:func:`~os.setgid`) then libev will not look at
    the environment variable :envvar:`LIBEV_FLAGS`. Otherwise (the default),
    :envvar:`LIBEV_FLAGS` will override the *flags* completely if it is found in
    the environment. This is useful to try out specific backends to test their
    performance, to work around bugs.


.. py:data:: EVFLAG_FORKCHECK

    Instead of calling :py:meth:`~Loop.reset` manually after a fork, you can
    also make libev check for a fork in each iteration by enabling this flag.
    This works by calling :c:func:`getpid` on every iteration of the loop, and
    thus this might slow down your event loop if you do a lot of loop iterations
    and little real work, but is usually not noticeable.
    The big advantage of this flag is that you can forget about fork (and forget
    about forgetting to tell libev about forking, although you still have to
    ignore ``SIGPIPE``) when you use it.
    This flag setting cannot be overridden or specified in the
    :envvar:`LIBEV_FLAGS` environment variable.


.. py:data:: EVFLAG_SIGNALFD

    When this flag is specified, then libev will attempt to use the
    :manpage:`signalfd(2)` API for its :py:class:`Signal`
    (and :py:class:`Child`) watchers. This API delivers signals synchronously,
    which makes it both faster and might make it possible to get the queued
    signal data. It can also simplify signal handling with threads, as long as
    you properly block signals in your threads that are not interested in
    handling them.
    :manpage:`signalfd(2)` will not be used by default as this changes your
    signal mask.


.. py:data:: EVFLAG_NOSIGMASK

    When this flag is specified, then libev will avoid modifying the signal
    mask. Specifically, this means you have to make sure signals are unblocked
    when you want to receive them.
    This behaviour is useful when you want to do your own signal handling, or
    want to handle signals only in specific threads and want to avoid libev
    unblocking the signals.
    It's also required by POSIX in a threaded program, as libev calls
    :c:func:`sigprocmask`, whose behaviour is officially unspecified.


.. py:data:: EVFLAG_NOTIMERFD

    When this flag is specified, then libev will avoid using a timerfd to detect
    time jumps. It will still be able to detect time jumps, but takes longer and
    has a lower accuracy in doing so, but saves a file descriptor per loop.
    The current implementation only tries to use a timerfd when the first
    :py:class:`Periodic` watcher is started and falls back on other methods if
    it cannot be created, but this behaviour might change in the future.


.. _backends:

backends
~~~~~~~~

.. py:data:: EVBACKEND_SELECT

    *Availability:* POSIX

    The standard :manpage:`select(2)` backend. Not completely standard, as libev
    tries to roll its own ``fd_set`` with no limits on the number of fds, but if
    that fails, expect a fairly low limit on the number of fds when using this
    backend. It doesn't scale too well (O(*highest_fd*)), but is usually the
    fastest backend for a low number of (low-numbered) fds.

    To get good performance out of this backend you need a high amount of
    parallelism (most of the file descriptors should be busy). If you are
    writing a server, you should :py:meth:`~socket.socket.accept` in a loop to
    accept as many connections as possible during one iteration. You might also
    want to have a look at :py:attr:`~Loop.io_interval` to increase the amount
    of readiness notifications you get per iteration.

    This backend maps :py:data:`EV_READ` to the ``readfds`` set and
    :py:data:`EV_WRITE` to the ``writefds`` set.


.. py:data:: EVBACKEND_POLL

    *Availability:* POSIX

    The :manpage:`poll(2)` backend. It's more complicated than
    :manpage:`select(2)`, but handles sparse fds better and has no artificial
    limit on the number of fds you can use (except it will slow down
    considerably with a lot of inactive fds).
    It scales similarly to :manpage:`select(2)`, i.e. O(*total_fds*).

    See :py:data:`EVBACKEND_SELECT` for performance tips.

    This backend maps :py:data:`EV_READ` to
    :c:macro:`POLLIN` | :c:macro:`POLLERR` | :c:macro:`POLLHUP`, and
    :py:data:`EV_WRITE` to
    :c:macro:`POLLOUT` | :c:macro:`POLLERR` | :c:macro:`POLLHUP`.


.. py:data:: EVBACKEND_EPOLL

    *Availability:* Linux

    Use the linux-specific :manpage:`epoll(7)` interface. For few fds, this
    backend is a little bit slower than :manpage:`poll(2)` and
    :manpage:`select(2)`, but it scales phenomenally better. While
    :manpage:`poll(2)` and :manpage:`select(2)` usually scale like
    O(*total_fds*) where *total_fds* is the total number of fds (or the highest
    fd), :manpage:`epoll(7)` scales either O(*1*) or O(*active_fds*).

    While stopping, setting and starting an :py:class:`Io` watcher in the same
    iteration will result in some caching, there is still a system call per such
    incident, so its best to avoid that. Also, :py:func:`~os.dup`'ed file
    descriptors might not work very well if you register events for both file
    descriptors.
    Best performance from this backend is achieved by not unregistering all
    watchers for a file descriptor until it has been closed, if possible, i.e.
    keep at least one watcher active per fd at all times. Stopping and starting
    a watcher (without re-setting it) also usually doesn't cause extra overhead.
    A fork can both result in spurious notifications as well as in libev having
    to destroy and recreate the epoll object (in both the parent and child
    processes), which can take considerable time (one syscall per file
    descriptor), is hard to detect, and thus should be avoided.
    All this means that, in practice, :manpage:`select(2)` can be as fast or
    faster than :manpage:`epoll(7)` for maybe up to a hundred file descriptors,
    depending on usage.

    While nominally embeddable in other event loops, this feature is broken in
    all kernel versions tested so far.

    This backend maps :py:data:`EV_READ` and :py:data:`EV_WRITE` the same way
    :py:data:`EVBACKEND_POLL` does.


.. py:data:: EVBACKEND_IOURING

    *Availability:* Linux

    Use the linux-specific io_uring backend. It offers an enourmous amount of
    features other than just I/O events, but is slower than epoll, so it is not
    used by default.

    Of note is that when sleeping in io_uring, the kernel counts that as disk
    I/O wait, keeping loadavg and a cpu core "virtually" busy, even if nothing
    actually waits for disk or uses CPU.

    If your application forks frequently, then this backend might be faster,
    as setting it up again after a fork is far more efficient with this backend,
    and it also doesn't suffer from the epoll flaw of receiving events for
    closed file descriptors.


.. py:data:: EVBACKEND_LINUXAIO

    *Availability:* Linux

    Use the Linux-specific Linux AIO (not :manpage:`aio(7)`
    but :manpage:`io_submit(2)`) event interface available in post-4.18 kernels
    (but libev only tries to use it in 4.19+).

    This backend maps :py:data:`EV_READ` and :py:data:`EV_WRITE` the same way
    :py:data:`EVBACKEND_POLL` does.


.. py:data:: EVBACKEND_KQUEUE

    *Availability:* most BSD clones

    Due to a number of bugs and inconsistencies between BSDs implementations,
    ``kqueue`` is not being "auto-detected" unless you explicitly specify it in
    the *flags* or libev was compiled on a known-to-be-good (-enough) system
    like NetBSD. It scales the same way the :manpage:`epoll(7)` backend does.

    While stopping, setting and starting an :py:class:`Io` watcher does never
    cause an extra system call as with :py:data:`EVBACKEND_EPOLL`, it still
    adds up to two event changes per incident. Support for :py:func:`~os.fork`
    is bad (you might have to leak fds on fork) and it drops fds silently in
    similarly hard to detect cases.
    This backend usually performs well under most conditions.

    You can still embed ``kqueue`` into a normal :manpage:`poll(2)` or
    :manpage:`select(2)` backend and use it only for sockets (after having made
    sure that sockets work with ``kqueue`` on the target platform). See
    :py:class:`Embed` watchers for more info.

    This backend maps :py:data:`EV_READ` into an :c:macro:`EVFILT_READ` kevent
    with ``NOTE_EOF``, and :py:data:`EV_WRITE` into an :c:macro:`EVFILT_WRITE`
    kevent with ``NOTE_EOF``.


.. py:data:: EVBACKEND_DEVPOLL

    *Availability:* Solaris 8

    This is not implemented yet (and might never be). According to reports,
    ``/dev/poll`` only supports sockets and is not embeddable, which would limit
    the usefulness of this backend immensely.


.. py:data:: EVBACKEND_PORT

    *Availability:* Solaris 10

    This uses the Solaris 10 ``event port`` mechanism. It's slow, but it scales
    very well (O(*active_fds*)).
    While this backend scales well, it requires one system call per active file
    descriptor per loop iteration. For small and medium numbers of file
    descriptors a "slow" :py:data:`EVBACKEND_SELECT` or
    :py:data:`EVBACKEND_POLL` backend might perform better.

    On the positive side, this backend actually performed fully to specification
    in all tests and is fully embeddable.

    This backend maps :py:data:`EV_READ` and :py:data:`EV_WRITE` the same way
    :py:data:`EVBACKEND_POLL` does.


.. py:data:: EVBACKEND_ALL

    Try all backends (even potentially broken ones that wouldn't be tried with
    :py:data:`EVFLAG_AUTO`). Since this is a mask, you can do stuff such as:

        .. code-block:: python

            EVBACKEND_ALL & ~EVBACKEND_KQUEUE

    It is definitely not recommended to use this flag, use whatever
    :py:func:`recommended_backends` returns, or simply do not specify a backend
    at all.


.. py:data:: EVBACKEND_MASK

    Not a backend at all, but a mask to select all backend bits from a *flags*
    value, in case you want to mask out any backends from *flags* (e.g. when
    modifying the :envvar:`LIBEV_FLAGS` environment variable).


.. _Loop_start_flags:

:py:meth:`~Loop.start` *flags*
------------------------------

If *flags* is omitted or specified as ``0``, it will keep handling events until
either no event watchers are active anymore or :py:meth:`~Loop.stop` was called.

.. py:data:: EVRUN_NOWAIT

    A *flags* value of :py:data:`EVRUN_NOWAIT` will look for new events, will
    handle those events and any already outstanding ones, but will not wait and
    block your process in case there are no events and will return after one
    iteration of the loop.
    This is sometimes useful to poll and handle new events while doing lengthy
    calculations, to keep the program responsive.

.. py:data:: EVRUN_ONCE

    A *flags* value of :py:data:`EVRUN_ONCE` will look for new events (waiting
    if necessary) and will handle those and any already outstanding ones. It
    will block your process until at least one new event arrives (which could be
    an event internal to libev itself, so there is no guarantee that a
    user-registered callback will be called), and will return after one
    iteration of the loop.
    This is useful if you are waiting for some external event in conjunction
    with something not expressible using other libev watchers. However, a pair
    of :py:class:`Prepare`/:py:class:`Check` watchers is usually a better
    approach for this kind of thing.


.. _Loop_stop_how:

:py:meth:`~Loop.stop` *how*
---------------------------

.. py:data:: EVBREAK_ONE

    If *how* is omitted or specified as :py:data:`EVBREAK_ONE` it will make the
    innermost :py:meth:`~Loop.start` call return.

.. py:data:: EVBREAK_ALL

    A *how* value of :py:data:`EVBREAK_ALL` will make all nested
    :py:meth:`~Loop.start` calls return.
