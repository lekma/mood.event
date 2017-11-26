#include "watcher.h"


#if EV_FORK_ENABLE


/* --------------------------------------------------------------------------
   Fork
   -------------------------------------------------------------------------- */

/* Fork_Type.tp_new */
static PyObject *
Fork_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return Watcher_new(type, EV_FORK, sizeof(ev_fork));
}


/* -------------------------------------------------------------------------- */

PyTypeObject Fork_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Fork",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Fork(loop, callback[, data=None, priority=0])",
    .tp_new = (newfunc)Fork_tp_new,
};


#endif // !EV_FORK_ENABLE
