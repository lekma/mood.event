.. currentmodule:: mood.event

:py:class:`Periodic` --- Periodic watcher
=========================================

.. py:class:: Periodic(loop, offset, interval, callback[, data=None, priority=0])

    :type loop: :py:class:`Loop`
    :param loop: loop object responsible for this watcher (accessible through
        :py:attr:`~Watcher.loop`).

    :param float offset: see :ref:`Periodic_modes`.

    :param float interval: see :ref:`Periodic_modes`.

    :param callable callback: see :py:attr:`~Watcher.callback`.

    :param object data: any Python object you might want to attach to the
        watcher (stored in :py:attr:`~Watcher.data`).

    :param int priority: see :py:attr:`~Watcher.priority`.

    :py:class:`Periodic` watchers are also timers of a kind, but they are very
    versatile (and unfortunately a bit complex).

    Unlike :py:class:`Timer`, :py:class:`Periodic` watchers are not based on
    real time (or relative time, the physical time that passes) but on wall
    clock time (absolute time, the thing you can read on your calendar or clock).
    The difference is that wall clock time can run faster or slower than real
    time, and time jumps are not uncommon (e.g. when you adjust your wrist-watch).

    You can tell a :py:class:`Periodic` watcher to trigger after some specific
    point in time: for example, if you tell a :py:class:`Periodic` watcher to
    trigger "in 10 seconds" (by specifying e.g. :py:meth:`Loop.now` + 10.0,
    that is, an absolute time not a delay) and then reset your system clock to
    January of the previous year, then it will take a year or more to trigger
    the event (unlike a :py:class:`Timer`, which would still trigger roughly 10
    seconds after starting it, as it uses a relative timeout).

    As with timers, the callback is guaranteed to be invoked only when the point
    in time where it is supposed to trigger has passed. If multiple timers
    become ready during the same loop iteration then the ones with earlier
    time-out values are invoked before ones with later time-out values (but this
    is no longer true when a callback calls :py:meth:`Loop.start` recursively).

    .. seealso::

        `ev_periodic - to cron or not to cron?
        <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_periodic_code_to_cron_or_not>`_


    .. py:method:: set(offset, interval)

        :param float offset: see :ref:`Periodic_modes`.

        :param float interval: see :ref:`Periodic_modes`.

        Reconfigures the watcher.


    .. py:method:: reset

        Simply stops and restarts the watcher. This is only useful when you
        changed some attributes.


    .. py:attribute:: offset

        The current offset value (see :ref:`Periodic_modes`).
        Can be modified any time, but changes only take effect when the periodic
        timer fires or :py:meth:`reset` is called.


    .. py:attribute:: interval

        The current interval value (see :ref:`Periodic_modes`).
        Can be modified any time, but changes only take effect when the periodic
        timer fires or :py:meth:`reset` is called.


    .. py:attribute:: at

        *Read only*

        When active, this is the absolute time that the watcher is supposed to
        trigger next. This is not the same as the *offset* argument to
        :py:meth:`set` or :py:meth:`__init__() <Periodic>`, but indeed works
        even in :ref:`interval <Periodic_interval_mode>` mode.


.. _Periodic_modes:

Modes of operation
------------------

.. _Periodic_absolute_mode:

Absolute timer - *interval* = 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this mode the watcher triggers an event after the wall clock time *offset*
has passed. It will not repeat and will not adjust when a time jump occurs, that
is, if it is to be run at January 1st 2014 then it will be stopped and invoked
when the system clock reaches or surpasses this point in time.

Example, trigger an event on January 1st 2014 00:00:00 UTC:

    .. code-block:: python

        Periodic(loop, 1388534400.0, 0.0, callback)

In this mode:

* *offset* is an absolute time expressed in seconds since the epoch.
* *interval* **must be 0**.

.. _Periodic_interval_mode:

Repeating interval timer - *interval* > 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This can be used to create timers that do not drift with respect to the system
clock, for example, here is a :py:class:`Periodic` that triggers each hour, on
the hour (with respect to UTC):

    .. code-block:: python

        Periodic(loop, 0.0, 3600.0, callback)

This doesn't mean there will always be 3600 seconds in between triggers, but
only that the callback will be called when the system time shows a full hour
(UTC), or more correctly, when the system time is evenly divisible by 3600.

In this mode:

* *offset* is merely an offset into the *interval* periods and must be 0.0
  or something between 0.0 and *interval*.
* *interval* **must be positive**, and for numerical stability, should be higher
  than 1/8192 (which is around 120 microseconds).

Note also that there is an upper limit to how often a timer can fire (CPU speed
for example), so if *interval* is very small then timing stability will of
course deteriorate.
