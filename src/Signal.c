/*
#
# Copyright © 2021 Malek Hadj-Ali
# All rights reserved.
#
# This file is part of mood.
#
# mood is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3
# as published by the Free Software Foundation.
#
# mood is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with mood.  If not, see <http://www.gnu.org/licenses/>.
#
*/


#include "event.h"


#if EV_SIGNAL_ENABLE


/* --------------------------------------------------------------------------
   Signal
   -------------------------------------------------------------------------- */

static inline Watcher *
__Signal_New(PyTypeObject *type)
{
    return Watcher_New(type, EV_SIGNAL, sizeof(ev_signal));
}


static inline int
__Signal_Set(Watcher *self, int signum)
{
    ev_signal_set((ev_signal *)self->watcher, signum);
    return 0;
}


/* Signal_Type -------------------------------------------------------------- */

/* Signal_Type.tp_new */
static PyObject *
Signal_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Signal_New(type);
}


/* Signal_Type.tp_init */
static int
Signal_tp_init(Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "signum",
        "loop", "callback", "data", "priority", NULL
    };

    int signum = 0;
    Loop *loop = NULL;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args, kwargs, "iO!O|Oi:__init__", kwlist,
            &signum,
            &Loop_Type, &loop, &callback, &data, &priority
        )
    ) {
        return -1;
    }
    if (Watcher_Init(self, loop, callback, data, priority)) {
        return -1;
    }
    return __Signal_Set(self, signum);
}


/* Signal.set(signum) */
static PyObject *
Signal_set(Watcher *self, PyObject *args)
{
    int signum = 0;

    if (
        Watcher_CannotSet(self) ||
        !PyArg_ParseTuple(args, "i:set", &signum) ||
        __Signal_Set(self, signum)
    ) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Signal_Type.tp_methods */
static PyMethodDef Signal_tp_methods[] = {
    {
        "set", (PyCFunction)Signal_set,
        METH_VARARGS, "set(signum)"
    },
    {NULL}  /* Sentinel */
};


/* Signal.signum */
static PyObject *
Signal_signum_getter(Watcher *self, void *closure)
{
    return PyLong_FromLong(((ev_signal *)self->watcher)->signum);
}


/* Signal_Type.tp_getsets */
static PyGetSetDef Signal_tp_getsets[] = {
    {
        "signum", (getter)Signal_signum_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {NULL}  /* Sentinel */
};


PyTypeObject Signal_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Signal",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Signal(signum, loop, callback[, data=None, priority=0])",
    .tp_methods = Signal_tp_methods,
    .tp_getset = Signal_tp_getsets,
    .tp_init = (initproc)Signal_tp_init,
    .tp_new = Signal_tp_new,
};


/* interface ---------------------------------------------------------------- */

Watcher *
Signal_New(Loop *loop, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "signum",
        "callback", "data", "priority", NULL
    };

    int signum = 0;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;
    Watcher *self = NULL;

    if (
        PyArg_ParseTupleAndKeywords(
            args, kwargs, "iO|Oi:signal", kwlist,
            &signum,
            &callback, &data, &priority
        ) &&
        (self = __Signal_New(&Signal_Type)) &&
        (
            Watcher_Init(self, loop, callback, data, priority) ||
            __Signal_Set(self, signum)
        )
    ) {
        Py_CLEAR(self);
    }
    return self;
}


#endif // !EV_SIGNAL_ENABLE

