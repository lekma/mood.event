#include "watcher.h"


/* helpers ------------------------------------------------------------------ */

static int
__ev_io_check_events__(int events)
{
    if (events & ~(EV__IOFDSET | EV_READ | EV_WRITE)) {
        PyErr_SetString(EventError, "illegal event mask");
        return -1;
    }
    return 0;
}


/* --------------------------------------------------------------------------
   Io
   -------------------------------------------------------------------------- */

static int
__Io_set__(Watcher *self, PyObject *fd, int events)
{
    int fdnum = PyObject_AsFileDescriptor(fd);

    if ((fdnum < 0) || __ev_io_check_events__(events)) {
        return -1;
    }
    ev_io_set(((ev_io *)self->watcher), fdnum, events);
    return 0;
}


/* -------------------------------------------------------------------------- */

/* Io_Type.tp_init */
static int
Io_tp_init(Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "loop",
        "fd", "events",
        "callback", "data", "priority", NULL
    };

    Loop *loop = NULL;
    PyObject *fd = NULL;
    int events = 0;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args, kwargs, "O!OiO|Oi:__init__", kwlist,
            &Loop_Type, &loop,
            &fd, &events,
            &callback, &data, &priority
        ) ||
        Watcher_init(self, loop, callback, data, priority)
    ) {
        return -1;
    }
    return __Io_set__(self, fd, events);
}


/* Io_Type.tp_new */
static PyObject *
Io_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return Watcher_new(type, EV_IO, sizeof(ev_io));
}


/* -------------------------------------------------------------------------- */

/* Io.set(fd, events) */
static PyObject *
Io_set(Watcher *self, PyObject *args)
{
    PyObject *fd = NULL;
    int events = 0;

    if (
        Watcher_check_set(self) ||
        !PyArg_ParseTuple(args, "Oi:set", &fd, &events) ||
        __Io_set__(self, fd, events)
    ) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Io_Type.tp_methods */
static PyMethodDef Io_tp_methods[] = {
    {
        "set",
        (PyCFunction)Io_set,
        METH_VARARGS,
        "set(fd, events)"
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

/* Io.fd */
static PyObject *
Io_fd_getter(Watcher *self, void *closure)
{
    return PyLong_FromLong(((ev_io *)self->watcher)->fd);
}


/* Io.events */
static PyObject *
Io_events_getter(Watcher *self, void *closure)
{
    return PyLong_FromLong(((ev_io *)self->watcher)->events);
}

static int
Io_events_setter(Watcher *self, PyObject *value, void *closure)
{
    int events = -1;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    if (
        Watcher_check_active(self, "set the 'events' of an Io") ||
        (((events = _PyLong_AsInt(value)) == -1) && PyErr_Occurred()) ||
        __ev_io_check_events__(events)
    ) {
        return -1;
    }
    ev_io_modify(((ev_io *)self->watcher), events);
    return 0;
}


/* Io_Type.tp_getsets */
static PyGetSetDef Io_tp_getsets[] = {
    {
        "fd",
        (getter)Io_fd_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {
        "events",
        (getter)Io_events_getter,
        (setter)Io_events_setter,
        NULL,
        NULL
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

PyTypeObject Io_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Io",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Io(loop, fd, events, callback[, data=None, priority=0])",
    .tp_methods = Io_tp_methods,
    .tp_getset = Io_tp_getsets,
    .tp_init = (initproc)Io_tp_init,
    .tp_new = (newfunc)Io_tp_new,
};
