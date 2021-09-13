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


#if EV_CHECK_ENABLE


/* --------------------------------------------------------------------------
   Check
   -------------------------------------------------------------------------- */

static inline Watcher *
__Check_New(PyTypeObject *type)
{
    return Watcher_New(type, EV_CHECK, sizeof(ev_check));
}


/* Check_Type --------------------------------------------------------------- */

/* Check_Type.tp_new */
static PyObject *
Check_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Check_New(type);
}


PyTypeObject Check_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Check",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Check(loop, callback[, data=None, priority=0])",
    .tp_new = Check_tp_new,
};


/* interface ---------------------------------------------------------------- */

Watcher *
Check_New(Loop *loop, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "callback", "data", "priority", NULL
    };

    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;
    Watcher *self = NULL;

    if (
        PyArg_ParseTupleAndKeywords(
            args, kwargs, "O|Oi:check", kwlist,
            &callback, &data, &priority
        ) &&
        (self = __Check_New(&Check_Type)) &&
        Watcher_Init(self, loop, callback, data, priority)
    ) {
        Py_CLEAR(self);
    }
    return self;
}


#endif // !EV_CHECK_ENABLE

