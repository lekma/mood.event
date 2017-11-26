.. currentmodule:: mood.event

:py:class:`Fork` --- Fork watcher
=================================

.. py:class:: Fork(loop, callback[, data=None, priority=0])

    :type loop: :py:class:`Loop`
    :param loop: loop object responsible for this watcher (accessible through
        :py:attr:`~Watcher.loop`).

    :param callable callback: see :py:attr:`~Watcher.callback`.

    :param object data: any Python object you might want to attach to the
        watcher (stored in :py:attr:`~Watcher.data`).

    :param int priority: see :py:attr:`~Watcher.priority`.

    :py:class:`Fork` watchers are called when a :py:func:`~os.fork` was detected
    (usually because whoever is a good citizen cared to tell libev about it by
    calling :py:meth:`Loop.reset`). The invocation is done before the event loop
    blocks next and before :py:class:`Check` watchers are being called, and only
    in the child after the fork. If whoever good citizen calling
    :py:meth:`Loop.reset` cheats and calls it in the wrong process, the fork
    handlers will be invoked, too, of course.

    .. seealso::

        `ev_fork - the audacity to resume the event loop after a fork
        <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_fork_code_the_audacity_to_re>`_

        * `The special problem of life after fork - how is it possible?
          <http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_life_after_fo>`_
