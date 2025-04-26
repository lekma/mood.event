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


/* helpers ------------------------------------------------------------------ */

static inline int
__Io_CheckEvents(int events)
{
    if (events & ~(EV__IOFDSET | EV_READ | EV_WRITE)) {
        PyErr_SetString(Error, "illegal event mask");
        return -1;
    }
    return 0;
}


/* --------------------------------------------------------------------------
   Io
   -------------------------------------------------------------------------- */

static inline Watcher *
__Io_New(PyTypeObject *type)
{
    return Watcher_New(type, EV_IO, sizeof(ev_io));
}


static inline int
__Io_Set(Watcher *self, PyObject *fd, int events)
{
    int fdnum = PyObject_AsFileDescriptor(fd);

    if ((fdnum < 0) || __Io_CheckEvents(events)) {
        return -1;
    }
    ev_io_set((ev_io *)self->watcher, fdnum, events);
    return 0;
}


/* Io_Type ------------------------------------------------------------------ */

/* Io_Type.tp_new */
static PyObject *
Io_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Io_New(type);
}


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
        )
    ) {
        return -1;
    }
    if (Watcher_Init(self, loop, callback, data, priority)) {
        return -1;
    }
    return __Io_Set(self, fd, events);
}


/* Io.set(fd, events) */
static PyObject *
Io_set(Watcher *self, PyObject *args)
{
    PyObject *fd = NULL;
    int events = 0;

    if (
        Watcher_CannotSet(self) ||
        !PyArg_ParseTuple(args, "Oi:set", &fd, &events) ||
        __Io_Set(self, fd, events)
    ) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Io_Type.tp_methods */
static PyMethodDef Io_tp_methods[] = {
    {
        "set", (PyCFunction)Io_set,
        METH_VARARGS, "set(fd, events)"
    },
    {NULL}  /* Sentinel */
};


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
    if (Watcher_IsActive(self, "set the 'events' of an Io")) {
        return -1;
    }
    if (
        (((events = _PyLong_AsInt(value)) == -1) && PyErr_Occurred()) ||
        __Io_CheckEvents(events)
    ) {
        return -1;
    }
    ev_io_modify((ev_io *)self->watcher, events);
    return 0;
}


/* Io.fd */
static PyObject *
Io_fd_getter(Watcher *self, void *closure)
{
    return PyLong_FromLong(((ev_io *)self->watcher)->fd);
}


/* Io_Type.tp_getsets */
static PyGetSetDef Io_tp_getsets[] = {
    {
        "events", (getter)Io_events_getter,
        (setter)Io_events_setter, NULL, NULL
    },
    {
        "fd", (getter)Io_fd_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {NULL}  /* Sentinel */
};


PyTypeObject Io_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Io",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Io(loop, fd, events, callback[, data=None, priority=0])",
    .tp_methods = Io_tp_methods,
    .tp_getset = Io_tp_getsets,
    .tp_init = (initproc)Io_tp_init,
    .tp_new = Io_tp_new,
};
