.. currentmodule:: mood.event

:py:class:`Scheduler` --- Scheduler watcher
===========================================

.. py:class:: Scheduler(loop, reschedule, callback[, data=None, priority=0])

    :type loop: :py:class:`Loop`
    :param loop: loop object responsible for this watcher (accessible through
        :py:attr:`~Watcher.loop`).

    :param callable reschedule: :py:attr:`reschedule` callback.

    :param callable callback: see :py:attr:`~Watcher.callback`.

    :param object data: any Python object you might want to attach to the
        watcher (stored in :py:attr:`~Watcher.data`).

    :param int priority: see :py:attr:`~Watcher.priority`.

    :py:class:`Scheduler` watchers are specialised :py:class:`Periodic` watchers.
    Each time the :py:class:`Scheduler` watcher gets scheduled, the
    :py:attr:`reschedule` callback is called with the watcher as first, and the
    current time as second argument. Example:

        .. code-block:: python

            def myreschedule(watcher, now):
                return now + 60.0

            Scheduler(loop, myreschedule, callback)

    This can be used to create very complex timers, such as a timer that
    triggers on "next midnight, local time". To do this, you would calculate the
    next midnight after *now* and return the timestamp value for this. This
    cannot be done with :py:class:`Timer` watchers, as those cannot react to
    time jumps.

    .. seealso::

        `ev_periodic - to cron or not to cron?
        <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_periodic_code_to_cron_or_not>`_


    .. py:method:: reset

        Simply stops and restarts the watcher. This is only useful when
        :py:attr:`reschedule` would return a different time than the last time
        it was called (e.g. in a crond like program when the crontabs have
        changed).


    .. py:attribute:: reschedule

        The current reschedule callback, its signature must be:

            .. py:function:: reschedule(watcher, now)
                :noindex:

                :type watcher: :py:class:`Scheduler`
                :param watcher: this watcher.

                :param float now: the current time.

                :rtype: float
                :return: the next time to trigger.

        It **must** always return a :py:class:`float` greater than or equal to
        the *now* argument to indicate the next time the watcher
        :py:attr:`~Watcher.callback` should be invoked. It will usually be
        called just before the :py:attr:`~Watcher.callback` is triggered, but
        might be called at other times, too.

        .. warning::

            * This callback **must not** stop or destroy any watcher, ever, or
              make **any** other event loop modifications whatsoever. If you
              need to stop it, return *now* + 10\ :sup:`30`\  and stop it
              afterwards (e.g. by starting a :py:class:`Prepare` watcher, which
              is the only event loop modification you are allowed to do).
            * If this callback raises an exception, or returns anything but a
              :py:class:`float`, the module will stop the watcher.


    .. py:attribute:: at

        *Read only*

        When active, this is the absolute time that the watcher is supposed to
        trigger next.
