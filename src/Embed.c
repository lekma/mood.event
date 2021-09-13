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


#if EV_EMBED_ENABLE


/* --------------------------------------------------------------------------
   Embed
   -------------------------------------------------------------------------- */

static inline Embed *
__Embed_Alloc(PyTypeObject *type)
{
    Embed *self = NULL;

    if ((self = (Embed *)Watcher_Alloc(type))) {
        self->other = NULL;
    }
    return self;
}


static inline Embed *
__Embed_New(PyTypeObject *type)
{
    Embed *self = NULL;

    if ((self = __Embed_Alloc(type))) {
        PyObject_GC_Track(self);
        if (Watcher_Setup((Watcher *)self, EV_EMBED, sizeof(ev_embed))) {
            Py_CLEAR(self);
        }
    }
    return self;
}


static inline int
__Embed_Traverse(Embed *self, visitproc visit, void *arg)
{
    Py_VISIT(self->other);
    return Watcher_Traverse((Watcher *)self, visit, arg);
}


static inline int
__Embed_Clear(Embed *self)
{
    Py_CLEAR(self->other);
    return Watcher_Clear((Watcher *)self);
}


static inline int
__Embed_Set(Embed *self, Loop *other)
{
    if (!(ev_backend(other->loop) & ev_embeddable_backends())) {
        PyErr_SetString(Error, "'other' must be embeddable");
        return -1;
    }
    _Py_SET_MEMBER(self->other, other);
    ev_embed_set((ev_embed *)((Watcher *)self)->watcher, other->loop);
    return 0;
}


/* Embed_Type --------------------------------------------------------------- */

/* Embed_Type.tp_new */
static PyObject *
Embed_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Embed_New(type);
}


/* Embed_Type.tp_init */
static int
Embed_tp_init(Embed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "other",
        "loop", "callback", "data", "priority", NULL
    };

    Loop *other = NULL, *loop = NULL;
    PyObject *callback = Py_None, *data = Py_None;
    int priority = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args, kwargs, "O!O!|OOi:__init__", kwlist,
            &Loop_Type, &other,
            &Loop_Type, &loop, &callback, &data, &priority
        )
    ) {
        return -1;
    }
    if (Watcher_Init((Watcher *)self, loop, callback, data, priority)) {
        return -1;
    }
    return __Embed_Set(self, other);
}


/* Embed_Type.tp_traverse */
static int
Embed_tp_traverse(Embed *self, visitproc visit, void *arg)
{
    return __Embed_Traverse(self, visit, arg);
}


/* Embed_Type.tp_clear */
static int
Embed_tp_clear(Embed *self)
{
    return __Embed_Clear(self);
}


/* Embed_Type.tp_dealloc */
static void
Embed_tp_dealloc(Embed *self)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)self)) {
        return;
    }
    PyObject_GC_UnTrack(self);
    __Embed_Clear(self);
    Watcher_Dealloc((Watcher *)self);
}


/* Embed.set(other) */
static PyObject *
Embed_set(Embed *self, PyObject *args)
{
    Loop *other = NULL;

    if (
        Watcher_CannotSet((Watcher *)self) ||
        !PyArg_ParseTuple(args, "O!:set", &Loop_Type, &other) ||
        __Embed_Set(self, other)
    ) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Embed.sweep() */
static PyObject *
Embed_sweep(Watcher *self)
{
    ev_embed_sweep(self->loop->loop, (ev_embed *)self->watcher);
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Embed_Type.tp_methods */
static PyMethodDef Embed_tp_methods[] = {
    {
        "set", (PyCFunction)Embed_set,
        METH_VARARGS, "set(other)"
    },
    {
        "sweep", (PyCFunction)Embed_sweep,
        METH_NOARGS, "sweep()"
    },
    {NULL}  /* Sentinel */
};


/* Embed.other */
static PyObject *
Embed_other_getter(Embed *self, void *closure)
{
    return __Py_INCREF((PyObject *)self->other);
}


/* Embed_Type.tp_getsets */
static PyGetSetDef Embed_tp_getsets[] = {
    {
        "other", (getter)Embed_other_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {NULL}  /* Sentinel */
};


PyTypeObject Embed_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Embed",
    .tp_basicsize = sizeof(Embed),
    .tp_dealloc = (destructor)Embed_tp_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_doc = "Embed(other, loop[, callback=None, data=None, priority=0])",
    .tp_traverse = (traverseproc)Embed_tp_traverse,
    .tp_clear = (inquiry)Embed_tp_clear,
    .tp_methods = Embed_tp_methods,
    .tp_getset = Embed_tp_getsets,
    .tp_init = (initproc)Embed_tp_init,
    .tp_new = Embed_tp_new,
};


/* interface ---------------------------------------------------------------- */

Embed *
Embed_New(Loop *loop, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "other",
        "callback", "data", "priority", NULL
    };

    Loop *other = NULL;
    PyObject *callback = Py_None, *data = Py_None;
    int priority = 0;
    Embed *self = NULL;

    if (
        PyArg_ParseTupleAndKeywords(
            args, kwargs, "O!|OOi:embed", kwlist,
            &Loop_Type, &other,
            &callback, &data, &priority
        ) &&
        (self = __Embed_New(&Embed_Type)) &&
        (
            Watcher_Init((Watcher *)self, loop, callback, data, priority) ||
            __Embed_Set(self, other)
        )
    ) {
        Py_CLEAR(self);
    }
    return self;
}


#endif // !EV_EMBED_ENABLE

