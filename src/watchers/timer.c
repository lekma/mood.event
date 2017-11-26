#include "watcher.h"


/* --------------------------------------------------------------------------
   Timer
   -------------------------------------------------------------------------- */

static int
__Timer_set__(Watcher *self, double after, double repeat)
{
    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(repeat, -1);
    ev_timer_set(((ev_timer *)self->watcher), after, repeat);
    return 0;
}


/* -------------------------------------------------------------------------- */

/* Timer_Type.tp_init */
static int
Timer_tp_init(Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "loop",
        "after", "repeat",
        "callback", "data", "priority", NULL
    };

    Loop *loop = NULL;
    double after = 0.0, repeat = 0.0;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args, kwargs, "O!ddO|Oi:__init__", kwlist,
            &Loop_Type, &loop,
            &after, &repeat,
            &callback, &data, &priority
        ) ||
        Watcher_init(self, loop, callback, data, priority)
    ) {
        return -1;
    }
    return __Timer_set__(self, after, repeat);
}


/* Timer_Type.tp_new */
static PyObject *
Timer_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return Watcher_new(type, EV_TIMER, sizeof(ev_timer));
}


/* -------------------------------------------------------------------------- */

/* Timer.set(after, repeat) */
static PyObject *
Timer_set(Watcher *self, PyObject *args)
{
    double after = 0.0, repeat = 0.0;

    if (
        Watcher_check_set(self) ||
        !PyArg_ParseTuple(args, "dd:set", &after, &repeat) ||
        __Timer_set__(self, after, repeat)
    ) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Timer.reset() */
static PyObject *
Timer_reset(Watcher *self)
{
    ev_timer_again(self->loop->loop, ((ev_timer *)self->watcher));
    Py_RETURN_NONE;
}


/* Timer_Type.tp_methods */
static PyMethodDef Timer_tp_methods[] = {
    {
        "set",
        (PyCFunction)Timer_set,
        METH_VARARGS,
        "set(after, repeat)"
    },
    {
        "reset",
        (PyCFunction)Timer_reset,
        METH_NOARGS,
        "reset()"
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

/* Timer.repeat */
static PyObject *
Timer_repeat_getter(Watcher *self, void *closure)
{
    return PyFloat_FromDouble(((ev_timer *)self->watcher)->repeat);
}

static int
Timer_repeat_setter(Watcher *self, PyObject *value, void *closure)
{
    double repeat = -1.0;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    if (((repeat = PyFloat_AsDouble(value)) == -1.0) && PyErr_Occurred()) {
        return -1;
    }
    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(repeat, -1);
    ((ev_timer *)self->watcher)->repeat = repeat;
    return 0;
}


/* Timer.remaining */
static PyObject *
Timer_remaining_getter(Watcher *self, void *closure)
{
    return PyFloat_FromDouble(
        ev_timer_remaining(self->loop->loop, ((ev_timer *)self->watcher))
    );
}


/* Timer_Type.tp_getsets */
static PyGetSetDef Timer_tp_getsets[] = {
    {
        "repeat",
        (getter)Timer_repeat_getter,
        (setter)Timer_repeat_setter,
        NULL,
        NULL
    },
    {
        "remaining",
        (getter)Timer_remaining_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

PyTypeObject Timer_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Timer",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Timer(loop, after, repeat, callback[, data=None, priority=0])",
    .tp_methods = Timer_tp_methods,
    .tp_getset = Timer_tp_getsets,
    .tp_init = (initproc)Timer_tp_init,
    .tp_new = (newfunc)Timer_tp_new,
};
