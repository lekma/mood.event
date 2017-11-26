#include "watcher.h"


#if EV_CHILD_ENABLE


/* helpers ------------------------------------------------------------------ */

static int
__ev_child_check_loop__(Loop *loop)
{
    if (!ev_is_default_loop(loop->loop)) {
        PyErr_SetString(
            EventError,
            "Child watchers are only supported in the 'default loop'"
        );
        return -1;
    }
    return 0;
}


/* --------------------------------------------------------------------------
   Child
   -------------------------------------------------------------------------- */

static int
__Child_set__(Watcher *self, int pid, int trace)
{
    ev_child_set(((ev_child *)self->watcher), pid, trace);
    return 0;
}


/* -------------------------------------------------------------------------- */

/* Child_Type.tp_init */
static int
Child_tp_init(Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "loop",
        "pid", "trace",
        "callback", "data", "priority", NULL
    };

    Loop *loop = NULL;
    int pid = 0, trace = 0;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args, kwargs, "O!ipO|Oi:__init__", kwlist,
            &Loop_Type, &loop,
            &pid, &trace,
            &callback, &data, &priority
        ) ||
        __ev_child_check_loop__(loop) ||
        Watcher_init(self, loop, callback, data, priority)
    ) {
        return -1;
    }
    return __Child_set__(self, pid, trace);
}


/* Child_Type.tp_new */
static PyObject *
Child_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return Watcher_new(type, EV_CHILD, sizeof(ev_child));
}


/* -------------------------------------------------------------------------- */

/* Child.set(pid, trace) */
static PyObject *
Child_set(Watcher *self, PyObject *args)
{
    int pid = 0, trace = 0;

    if (
        Watcher_check_set(self) ||
        !PyArg_ParseTuple(args, "ip:set", &pid, &trace) ||
        __Child_set__(self, pid, trace)
    ) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Child_Type.tp_methods */
static PyMethodDef Child_tp_methods[] = {
    {
        "set",
        (PyCFunction)Child_set,
        METH_VARARGS,
        "set(pid, trace)"
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

/* Child.pid */
static PyObject *
Child_pid_getter(Watcher *self, void *closure)
{
    return PyLong_FromLong(((ev_child *)self->watcher)->pid);
}


/* Child.rpid */
static PyObject *
Child_rpid_getter(Watcher *self, void *closure)
{
    return PyLong_FromLong(((ev_child *)self->watcher)->rpid);
}

static int
Child_rpid_setter(Watcher *self, PyObject *value, void *closure)
{
    int rpid = -1;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    if (((rpid = _PyLong_AsInt(value)) == -1) && PyErr_Occurred()) {
        return -1;
    }
    ((ev_child *)self->watcher)->rpid = rpid;
    return 0;
}


/* Child.rstatus */
static PyObject *
Child_rstatus_getter(Watcher *self, void *closure)
{
    return PyLong_FromLong(((ev_child *)self->watcher)->rstatus);
}

static int
Child_rstatus_setter(Watcher *self, PyObject *value, void *closure)
{
    int rstatus = -1;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    if (((rstatus = _PyLong_AsInt(value)) == -1) && PyErr_Occurred()) {
        return -1;
    }
    ((ev_child *)self->watcher)->rstatus = rstatus;
    return 0;
}


/* Child_Type.tp_getsets */
static PyGetSetDef Child_tp_getsets[] = {
    {
        "pid",
        (getter)Child_pid_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {
        "rpid",
        (getter)Child_rpid_getter,
        (setter)Child_rpid_setter,
        NULL,
        NULL
    },
    {
        "rstatus",
        (getter)Child_rstatus_getter,
        (setter)Child_rstatus_setter,
        NULL,
        NULL
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

PyTypeObject Child_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Child",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Child(loop, pid, trace, callback[, data=None, priority=0])",
    .tp_methods = Child_tp_methods,
    .tp_getset = Child_tp_getsets,
    .tp_init = (initproc)Child_tp_init,
    .tp_new = (newfunc)Child_tp_new,
};


#endif // !EV_CHILD_ENABLE
