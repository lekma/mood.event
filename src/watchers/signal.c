#include "watcher.h"


#if EV_SIGNAL_ENABLE


/* --------------------------------------------------------------------------
   Signal
   -------------------------------------------------------------------------- */

static int
__Signal_set__(Watcher *self, int signum)
{
    ev_signal_set(((ev_signal *)self->watcher), signum);
    return 0;
}


/* -------------------------------------------------------------------------- */

/* Signal_Type.tp_init */
static int
Signal_tp_init(Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "loop",
        "signum",
        "callback", "data", "priority", NULL
    };

    Loop *loop = NULL;
    int signum = 0;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args, kwargs, "O!iO|Oi:__init__", kwlist,
            &Loop_Type, &loop,
            &signum,
            &callback, &data, &priority
        ) ||
        Watcher_init(self, loop, callback, data, priority)
    ) {
        return -1;
    }
    return __Signal_set__(self, signum);
}


/* Signal_Type.tp_new */
static PyObject *
Signal_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return Watcher_new(type, EV_SIGNAL, sizeof(ev_signal));
}


/* -------------------------------------------------------------------------- */

/* Signal.set(signum) */
static PyObject *
Signal_set(Watcher *self, PyObject *args)
{
    int signum = 0;

    if (
        Watcher_check_set(self) ||
        !PyArg_ParseTuple(args, "i:set", &signum) ||
        __Signal_set__(self, signum)
    ) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Signal_Type.tp_methods */
static PyMethodDef Signal_tp_methods[] = {
    {
        "set",
        (PyCFunction)Signal_set,
        METH_VARARGS,
        "set(signum)"
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

/* Signal.signum */
static PyObject *
Signal_signum_getter(Watcher *self, void *closure)
{
    return PyLong_FromLong(((ev_signal *)self->watcher)->signum);
}


/* Signal_Type.tp_getsets */
static PyGetSetDef Signal_tp_getsets[] = {
    {
        "signum",
        (getter)Signal_signum_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

PyTypeObject Signal_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Signal",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Signal(loop, signum, callback[, data=None, priority=0])",
    .tp_methods = Signal_tp_methods,
    .tp_getset = Signal_tp_getsets,
    .tp_init = (initproc)Signal_tp_init,
    .tp_new = (newfunc)Signal_tp_new,
};


#endif // !EV_SIGNAL_ENABLE
