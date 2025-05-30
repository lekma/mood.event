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


#if EV_IDLE_ENABLE


/* --------------------------------------------------------------------------
   Idle
   -------------------------------------------------------------------------- */

static inline Watcher *
__Idle_New(PyTypeObject *type)
{
    return Watcher_New(type, EV_IDLE, sizeof(ev_idle));
}


/* Idle_Type ---------------------------------------------------------------- */

/* Idle_Type.tp_new */
static PyObject *
Idle_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Idle_New(type);
}


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
        )
    ) {
        return -1;
    }
    return Watcher_Init(self, loop, callback, data, priority);
}


PyTypeObject Idle_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Idle",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Idle(loop[, callback=None, data=None, priority=0])",
    .tp_init = (initproc)Idle_tp_init,
    .tp_new = Idle_tp_new,
};


#endif // !EV_IDLE_ENABLE
