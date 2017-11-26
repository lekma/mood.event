#include "watcher.h"


#if EV_IDLE_ENABLE


/* --------------------------------------------------------------------------
   Idle
   -------------------------------------------------------------------------- */

/* Idle_Type.tp_init */
static int
Idle_tp_init(Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "loop",
        "callback", "data", "priority", NULL
    };

    Loop *loop = NULL;
    PyObject *callback = Py_None, *data = Py_None;
    int priority = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args, kwargs, "O!|OOi:__init__", kwlist,
            &Loop_Type, &loop,
            &callback, &data, &priority
        ) ||
        Watcher_init(self, loop, callback, data, priority)
    ) {
        return -1;
    }
    return 0;
}


/* Idle_Type.tp_new */
static PyObject *
Idle_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return Watcher_new(type, EV_IDLE, sizeof(ev_idle));
}


/* -------------------------------------------------------------------------- */

PyTypeObject Idle_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Idle",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Idle(loop[, callback=None, data=None, priority=0])",
    .tp_init = (initproc)Idle_tp_init,
    .tp_new = (newfunc)Idle_tp_new,
};


#endif // !EV_IDLE_ENABLE
