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


#if EV_PREPARE_ENABLE


/* --------------------------------------------------------------------------
   Prepare
   -------------------------------------------------------------------------- */

static inline Watcher *
__Prepare_New(PyTypeObject *type)
{
    return Watcher_New(type, EV_PREPARE, sizeof(ev_prepare));
}


/* Prepare_Type ------------------------------------------------------------- */

/* Prepare_Type.tp_new */
static PyObject *
Prepare_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Prepare_New(type);
}


PyTypeObject Prepare_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Prepare",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Prepare(loop, callback[, data=None, priority=0])",
    .tp_new = Prepare_tp_new,
};


#endif // !EV_PREPARE_ENABLE
