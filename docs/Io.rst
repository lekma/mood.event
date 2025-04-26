.. currentmodule:: mood.event

:py:class:`Io` --- I/O watcher
==============================

.. py:class:: Io(loop, fd, events, callback[, data=None, priority=0])

    :type loop: :py:class:`Loop`
    :param loop: loop object responsible for this watcher (accessible through
        :py:attr:`~Watcher.loop`).

    :type fd: int or object
    :param fd: the file descriptor to be monitored, can be an int or any
        Python object having a :py:meth:`~io.IOBase.fileno` method.

    :param int events: either :py:data:`EV_READ`, :py:data:`EV_WRITE` or
        :py:data:`EV_READ` | :py:data:`EV_WRITE`.

    :param callable callback: see :py:attr:`~Watcher.callback`.

    :param object data: any Python object you might want to attach to the
        watcher (stored in :py:attr:`~Watcher.data`).

    :param int priority: see :py:attr:`~Watcher.priority`.

    :py:class:`Io` watchers check whether a file descriptor is readable or
    writable in each iteration of the event loop, or, more precisely, when
    reading would not block the process and writing would at least be able to
    write some data. This behaviour is called level-triggering because you keep
    receiving events as long as the condition persists. Remember you can stop
    the watcher if you don't want to act on the event and neither want to
    receive future events.

    In general you can register as many read and/or write event watchers per fd
    as you want. Setting all file descriptors to non-blocking mode is also
    usually a good idea (but not required).

    Another thing you have to watch out for is that it is quite easy to receive
    "spurious" readiness notifications, that is, your callback might be called
    with :py:data:`EV_READ` but a subsequent :py:meth:`~io.RawIOBase.read` or
    :py:meth:`~socket.socket.recv` will actually block because there is no data.
    It is very easy to get into this situation even with a relatively standard
    program structure. Thus it is best to always use non-blocking I/O: an extra
    :py:meth:`~io.RawIOBase.read`/:py:meth:`~socket.socket.recv` returning
    :py:data:`~errno.EAGAIN` is far preferable to a program hanging until some
    data arrives.

    If you cannot run the fd in non-blocking mode, then you have to separately
    re-test whether a file descriptor is really ready with a known-to-be good
    interface such as :manpage:`poll(2)`. Some people additionally use
    ``SIGALRM`` and an interval timer, just to be sure you won't block
    indefinitely.

    But really, best use non-blocking mode.

    .. seealso::

        `ev_io - is this file descriptor readable or writable?
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


    .. py:method:: set(fd, events)

        :type fd: int or object
        :param fd: the file descriptor to be monitored, can be an int or any
            Python object having a :py:meth:`~io.IOBase.fileno` method.

        :param int events: either :py:data:`EV_READ`, :py:data:`EV_WRITE` or
            :py:data:`EV_READ` | :py:data:`EV_WRITE`.

        Reconfigures the watcher.


    .. py:attribute:: fd

        *Read only*

        The file descriptor being watched.


    .. py:attribute:: events

        The events being watched.
