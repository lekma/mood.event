.. currentmodule:: mood.event

Watchers
========

.. toctree::
    :maxdepth: 1

    Io
    Timer
    Periodic
    Scheduler
    Signal
    Child
    Idle
    PrepareCheck
    Embed
    Fork
    Async


Common methods and attributes
-----------------------------

:py:class:`Watcher` is the base class for all watchers. Though it is not exposed
to users, its methods and attributes are described here.


.. py:class:: Watcher

    .. seealso::

        `ANATOMY OF A WATCHER
        <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#ANATOMY_OF_A_WATCHER>`_


    .. py:method:: start

        Starts (activates) the watcher. Only active watchers will receive
        events. If the watcher is already active nothing will happen.


    .. py:method:: stop

        Stops the watcher if active, and clears the pending status (whether the
        watcher was active or not).
        It is possible that stopped watchers are pending - for example,
        non-repeating timers are being stopped when they become pending - but
        :py:meth:`stop` ensures that the watcher is neither active nor pending.


    .. py:method:: invoke(revents)

        :param int revents: See :ref:`Events_received` for valid values.

        Invoke the watcher callback with the given *revents*.


    .. py:method:: feed(revents)

        :param int revents: See :ref:`Events_received` for valid values.

        Feeds the given *revents* set into the event loop, as if the specified
        event had happened for the watcher.


    .. py:method:: clear

        :rtype: int

        If the watcher is pending, this method clears its pending status and
        returns its *revents* bitset (as if its callback was invoked). If the
        watcher isn't pending it does nothing and returns ``0``.
        Sometimes it can be useful to "poll" a watcher instead of waiting for
        its callback to be invoked, which can be accomplished with this method.


    .. py:attribute:: loop

        *Read only*

        :py:class:`Loop` object responsible for the watcher.


    .. py:attribute:: callback

        The current watcher callback, its signature must be:

            .. py:function:: callback(watcher, revents)
                :noindex:

                :type watcher: a subclass of :py:class:`Watcher`
                :param watcher: this watcher.

                :param int revents: See :ref:`Events_received` for valid values.

        As a rule you should not let a callback return with unhandled
        exceptions. The loop "does not know" how to correctly handle an
        exception happening in **your** callback (it depends largely on what
        **you** are doing), so, by default, it will just print a warning and
        suppress it.
        If you want to act on an exception, you're better off doing it in the
        callback (where you are allowed to do anything needed, like logging,
        stopping, restarting the loop, etc.). Example:

            .. code-block:: python

                def mycallback(watcher, revents):
                    try:
                        pass # do something interesting
                    except Exception as err:
                        # stop the watcher
                        watcher.stop()
                        # stop the loop
                        watcher.loop.stop()
                        # and finally raise err
                        raise err

        If you have a lot of callbacks, use decorators:

            .. code-block:: python

                import logging

                def mydecorator(func):
                    def wrap(watcher, revents):
                        try:
                            func(watcher, revents)
                        except RuntimeError: # these are not fatal
                            # this will also log the traceback
                            logging.exception("stopping {0}".format(watcher))
                            # stop the watcher but let the loop continue on its merry way
                            watcher.stop()
                        except Exception as err: # all other exceptions are fatal
                            # stop the watcher
                            watcher.stop()
                            # stop the loop
                            watcher.loop.stop()
                            # and finally raise err
                            raise err
                    return wrap

                @mydecorator
                def mycallback(watcher, revents):
                    pass #do something interesting

        .. note::

            As a convenience mood.event provides a :py:func:`fatal` decorator.
            If a callback decorated with :py:func:`fatal` raises an exception
            the loop is stopped and the exception raised.

            Contrast:

                .. code-block:: python

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

            and:

                .. code-block:: python

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


    .. py:attribute:: data

        watcher data.


    .. py:attribute:: priority

        Set and query the priority of the watcher. The priority is a small
        integer between :py:data:`EV_MINPRI` and :py:data:`EV_MAXPRI`. Pending
        watchers with higher priority will be invoked before watchers with lower
        priority, but priority will not keep watchers from being executed. If
        you need to suppress invocation when higher priority events are pending
        you need to look at :py:class:`Idle` watchers, which provide this
        functionality.

        Setting a priority outside the range of :py:data:`EV_MINPRI` to
        :py:data:`EV_MAXPRI` is fine, as long as you do not mind that the
        priority value you query might or might not have been clamped to the
        valid range.

        The default priority used by watchers when no priority has been set is
        always ``0``.

        .. note::

            You **must not** change the priority of a watcher as long as it is
            active or pending.

        .. seealso::

            `WATCHER PRIORITY MODELS
            <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#WATCHER_PRIORITY_MODELS>`_


    .. py:attribute:: active

        *Read only*

        ``True`` if the watcher is active (i.e. it has been started and not yet
        been stopped), ``False`` otherwise.

        .. note::

            As long as a watcher is active you must not modify it.


    .. py:attribute:: pending

        *Read only*

        ``True`` if the watcher is pending (i.e. it has outstanding events but
        its callback has not yet been invoked), ``False`` otherwise.


.. _Events_received:

Events received
---------------

.. py:data:: EV_IO
             EV_READ

    The file descriptor in the :py:class:`Io` watcher has become readable.


.. py:data:: EV_WRITE

    The file descriptor in the :py:class:`Io` watcher has become writable.


.. py:data:: EV_TIMER

    The :py:class:`Timer` watcher has timed out.


.. py:data:: EV_PERIODIC

    The :py:class:`Periodic` or :py:class:`Scheduler` watcher has timed out.


.. py:data:: EV_SIGNAL

    The signal specified in the :py:class:`Signal` watcher has been received by
    a thread.


.. py:data:: EV_CHILD

    The pid specified in the :py:class:`Child` watcher has received a status
    change.


.. py:data:: EV_IDLE

    The :py:class:`Idle` watcher has determined that you have nothing better to
    do.


.. py:data:: EV_PREPARE
             EV_CHECK

    All :py:class:`Prepare` watchers are invoked just before the loop starts to
    gather new events, and all :py:class:`Check` watchers are queued (not
    invoked) just after the loop has gathered them, but before it queues any
    callbacks for any received events. That means :py:class:`Prepare` watchers
    are the last watchers invoked before the event loop sleeps or polls for new
    events, and :py:class:`Check` watchers will be invoked before any other
    watchers of the same or lower priority within an event loop iteration.
    Callbacks of both watcher types can start and stop as many watchers as they
    want, and all of them will be taken into account (for example, a
    :py:class:`Prepare` watcher might start an :py:class:`Idle` watcher to keep
    the loop from blocking).


.. py:data:: EV_EMBED

    The embedded event loop specified in the :py:class:`Embed` watcher needs
    attention.


.. py:data:: EV_FORK

    The event loop has been resumed in the child process after fork (see
    :py:class:`Fork`).


.. py:data:: EV_ASYNC

    The given :py:class:`Async` watcher has been asynchronously notified.


.. py:data:: EV_CUSTOM

    Not ever sent (or otherwise used) by libev itself, but can be freely used by
    users to signal watchers (e.g. via :py:meth:`~Watcher.feed`).


.. py:data:: EV_ERROR

    An unspecified error has occurred, the watcher has been stopped. This might
    happen because the watcher could not be properly started because libev ran
    out of memory, a file descriptor was found to be closed or any other problem.

    .. warning::

        mood.event handle this event as a fatal error. On receiving this event
        the loop and the watcher **will be stopped** (the callback **will not be
        invoked**). In practice, users should never receive this event (still
        present for testing puposes).



Priorities
----------

.. py:data:: EV_MINPRI

    default: ``-2``.


.. py:data:: EV_MAXPRI

    default: ``2``.
