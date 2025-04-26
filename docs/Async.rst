.. currentmodule:: mood.event

:py:class:`Async` --- Async watcher
===================================

.. py:class:: Async(loop, callback[, data=None, priority=0])

    :type loop: :py:class:`Loop`
    :param loop: loop object responsible for this watcher (accessible through
        :py:attr:`~Watcher.loop`).

    :param callable callback: see :py:attr:`~Watcher.callback`.

    :param object data: any Python object you might want to attach to the
        watcher (stored in :py:attr:`~Watcher.data`).

    :param int priority: see :py:attr:`~Watcher.priority`.

    In general, you cannot use a loop from multiple threads or other
    asynchronous sources such as signal handlers (as opposed to multiple event
    loops - those are of course safe to use in different threads).

    Sometimes, however, you need to wake up an event loop you do not control,
    for example because it belongs to another thread. This is what
    :py:class:`Async` watchers do: as long as the :py:class:`Async` watcher is
    active, you can signal it by calling :py:meth:`send`, which is thread- and
    signal safe.

    This functionality is very similar to :py:class:`Signal` watchers, as
    signals, too, are asynchronous in nature, and signals, too, will be
    compressed (i.e. the number of callback invocations may be less than the
    number of :py:meth:`send` calls). In fact, you could use signal watchers as
    a kind of "global async watchers" by using a watcher on an otherwise unused
    signal, and :py:func:`feed_signal` to signal this watcher from another
    thread, even without knowing which loop owns the signal.

    .. seealso::

        `ev_async - how to wake up an event loop
        <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_async_code_how_to_wake_up_an>`_

        * `Queueing
          <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#Queueing>`_


    .. py:method:: send

        Sends/signals/activates the watcher, that is, feeds an
        :py:data:`EV_ASYNC` event on the watcher into the event loop, and
        instantly returns. Unlike :py:meth:`~Watcher.feed`, this call is safe to
        do from other threads, signal or similar contexts.

        Note that, as with other watchers in libev, multiple events might get
        compressed into a single callback invocation (another way to look at
        this is that :py:class:`Async` watchers are level-triggered: they are
        set on :py:meth:`send`, reset when the event loop detects that).

        This call incurs the overhead of at most one extra system call per event
        loop iteration, if the event loop is blocked, and no syscall at all if
        the event loop (or your program) is processing events. That means that
        repeated calls are basically free (there is no need to avoid calls for
        performance reasons) and that the overhead becomes smaller (typically
        zero) under load.


    .. py:attribute:: sent

        *Read only*

        ``True`` if :py:meth:`send` has been called on the watcher but the event
        has not yet been processed (or even noted) by the event loop, ``False``
        otherwise.

        :py:meth:`send` sets a flag in the watcher and wakes up the loop. When
        the loop iterates next and checks for the watcher to have become active,
        it will reset the flag again. :py:attr:`sent` can be used to very
        quickly check whether invoking the loop might be a good idea.

        .. note::

            This does not check whether the watcher itself is pending, only
            whether it has been requested to make this watcher pending: there is
            a time window between the event loop checking and resetting the
            async notification, and the callback being invoked.
