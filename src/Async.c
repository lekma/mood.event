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


#if EV_ASYNC_ENABLE


/* --------------------------------------------------------------------------
   Async
   -------------------------------------------------------------------------- */

static inline Watcher *
__Async_New(PyTypeObject *type)
{
    return Watcher_New(type, EV_ASYNC, sizeof(ev_async));
}


/* Async_Type --------------------------------------------------------------- */

/* Async_Type.tp_new */
static PyObject *
Async_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Async_New(type);
}


/* Async.send() */
static PyObject *
Async_send(Watcher *self)
{
    ev_async_send(self->loop->loop, (ev_async *)self->watcher);
    Py_RETURN_NONE;
}


/* Async_Type.tp_methods */
static PyMethodDef Async_tp_methods[] = {
    {
        "send", (PyCFunction)Async_send,
        METH_NOARGS, "send()"
    },
    {NULL}  /* Sentinel */
};


/* Async.sent */
static PyObject *
Async_sent_getter(Watcher *self, void *closure)
{
    return PyBool_FromLong(ev_async_pending((ev_async *)self->watcher));
}


/* Async_Type.tp_getsets */
static PyGetSetDef Async_tp_getsets[] = {
    {
        "sent", (getter)Async_sent_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {NULL}  /* Sentinel */
};


PyTypeObject Async_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Async",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Async(loop, callback[, data=None, priority=0])",
    .tp_methods = Async_tp_methods,
    .tp_getset = Async_tp_getsets,
    .tp_new = Async_tp_new,
};


#endif // !EV_ASYNC_ENABLE
