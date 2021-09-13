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


#if EV_FORK_ENABLE


/* --------------------------------------------------------------------------
   Fork
   -------------------------------------------------------------------------- */

static inline Watcher *
__Fork_New(PyTypeObject *type)
{
    return Watcher_New(type, EV_FORK, sizeof(ev_fork));
}


/* Fork_Type ---------------------------------------------------------------- */

/* Fork_Type.tp_new */
static PyObject *
Fork_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Fork_New(type);
}


PyTypeObject Fork_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Fork",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Fork(loop, callback[, data=None, priority=0])",
    .tp_new = Fork_tp_new,
};


/* interface ---------------------------------------------------------------- */

Watcher *
Fork_New(Loop *loop, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "callback", "data", "priority", NULL
    };

    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;
    Watcher *self = NULL;

    if (
        PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|Oi:fork", kwlist,
            &callback, &data, &priority
        ) &&
        (self = __Fork_New(&Fork_Type)) &&
        Watcher_Init(self, loop, callback, data, priority)
    ) {
        Py_CLEAR(self);
    }
    return self;
}


#endif // !EV_FORK_ENABLE

