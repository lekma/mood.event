.. currentmodule:: mood.event

:py:class:`Signal` --- Signal watcher
=====================================

.. py:class:: Signal(loop, signum, callback[, data=None, priority=0])

    :type loop: :py:class:`Loop`
    :param loop: loop object responsible for this watcher (accessible through
        :py:attr:`~Watcher.loop`).

    :param int signum: the signal number to monitor.

    :param callable callback: see :py:attr:`~Watcher.callback`.

    :param object data: any Python object you might want to attach to the
        watcher (stored in :py:attr:`~Watcher.data`).

    :param int priority: see :py:attr:`~Watcher.priority`.

    :py:class:`Signal` watchers will trigger an event when the process receives
    a specific signal one or more times. Even though signals are very
    asynchronous, libev will try its best to deliver signals synchronously, i.e.
    as part of the normal event processing, like any other event.

    You can configure as many watchers as you like for the same signal, but only
    within the same loop, i.e. you can watch for ``SIGINT`` in your *default
    loop* and for ``SIGIO`` in another loop, but you cannot watch for ``SIGINT``
    in both the *default loop* and another loop at the same time. At the moment,
    ``SIGCHLD`` is permanently tied to the *default loop*. Only after the first
    watcher for a signal is started will libev actually register something with
    the kernel. It thus coexists with your own signal handlers as long as you
    don't register any with libev for the same signal.

    If possible and supported, libev will install its handlers with
    :c:data:`SA_RESTART` (or equivalent) behaviour enabled, so system calls
    should not be unduly interrupted. If you have a problem with system calls
    getting interrupted by signals you can block all signals in a
    :py:class:`Check` watcher and unblock them in a :py:class:`Prepare` watcher.

    .. seealso::

        `ev_signal - signal me when a signal gets signalled!
        <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_signal_code_signal_me_when_a>`_

        * `The special problem of inheritance over fork/execve/pthread_create
          <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_inheritance_o>`_
        * `The special problem of threads signal handling
          <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_threads_signa>`_


    .. py:method:: set(signum)

        :param int signum: the signal number to monitor.

        Reconfigures the watcher.


    .. py:attribute:: signum

        *Read only*

        The signal number being monitored.
