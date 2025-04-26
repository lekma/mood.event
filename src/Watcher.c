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

#define __Watcher_CallMethod(m, t, l, w) t##_##m((l), (t *)(w))
#define __Watcher_CallStart(t, l, w) __Watcher_CallMethod(start, t, l, w)
#define __Watcher_CallStop(t, l, w) __Watcher_CallMethod(stop, t, l, w)


static inline void
__Watcher_Start(ev_loop *loop, ev_watcher *watcher, int ev_type)
{
    switch (ev_type) {
        case EV_IO:
            __Watcher_CallStart(ev_io, loop, watcher);
            break;
        case EV_TIMER:
            __Watcher_CallStart(ev_timer, loop, watcher);
            break;
#if EV_PERIODIC_ENABLE
        case EV_PERIODIC:
            __Watcher_CallStart(ev_periodic, loop, watcher);
            break;
#endif
#if EV_SIGNAL_ENABLE
        case EV_SIGNAL:
            __Watcher_CallStart(ev_signal, loop, watcher);
            break;
#endif
#if EV_CHILD_ENABLE
        case EV_CHILD:
            __Watcher_CallStart(ev_child, loop, watcher);
            break;
#endif
#if EV_IDLE_ENABLE
        case EV_IDLE:
            __Watcher_CallStart(ev_idle, loop, watcher);
            break;
#endif
#if EV_PREPARE_ENABLE
        case EV_PREPARE:
            __Watcher_CallStart(ev_prepare, loop, watcher);
            break;
#endif
#if EV_CHECK_ENABLE
        case EV_CHECK:
            __Watcher_CallStart(ev_check, loop, watcher);
            break;
#endif
#if EV_EMBED_ENABLE
        case EV_EMBED:
            __Watcher_CallStart(ev_embed, loop, watcher);
            break;
#endif
#if EV_FORK_ENABLE
        case EV_FORK:
            __Watcher_CallStart(ev_fork, loop, watcher);
            break;
#endif
#if EV_ASYNC_ENABLE
        case EV_ASYNC:
            __Watcher_CallStart(ev_async, loop, watcher);
            break;
#endif
        default:
            Py_FatalError("unknown watcher type");
            break;
    }
}


static inline void
__Watcher_Stop(ev_loop *loop, ev_watcher *watcher, int ev_type)
{
    switch (ev_type) {
        case EV_IO:
            __Watcher_CallStop(ev_io, loop, watcher);
            break;
        case EV_TIMER:
            __Watcher_CallStop(ev_timer, loop, watcher);
            break;
#if EV_PERIODIC_ENABLE
        case EV_PERIODIC:
            __Watcher_CallStop(ev_periodic, loop, watcher);
            break;
#endif
#if EV_SIGNAL_ENABLE
        case EV_SIGNAL:
            __Watcher_CallStop(ev_signal, loop, watcher);
            break;
#endif
#if EV_CHILD_ENABLE
        case EV_CHILD:
            __Watcher_CallStop(ev_child, loop, watcher);
            break;
#endif
#if EV_IDLE_ENABLE
        case EV_IDLE:
            __Watcher_CallStop(ev_idle, loop, watcher);
            break;
#endif
#if EV_PREPARE_ENABLE
        case EV_PREPARE:
            __Watcher_CallStop(ev_prepare, loop, watcher);
            break;
#endif
#if EV_CHECK_ENABLE
        case EV_CHECK:
            __Watcher_CallStop(ev_check, loop, watcher);
            break;
#endif
#if EV_EMBED_ENABLE
        case EV_EMBED:
            __Watcher_CallStop(ev_embed, loop, watcher);
            break;
#endif
#if EV_FORK_ENABLE
        case EV_FORK:
            __Watcher_CallStop(ev_fork, loop, watcher);
            break;
#endif
#if EV_ASYNC_ENABLE
        case EV_ASYNC:
            __Watcher_CallStop(ev_async, loop, watcher);
            break;
#endif
        default:
            Py_FatalError("unknown watcher type");
            break;
    }
}


/* -------------------------------------------------------------------------- */

static inline void
__Watcher_WarnStopped(Watcher *self)
{
    PyObject *exc_type, *exc_value, *exc_traceback;

    PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
    PyErr_WarnFormat(PyExc_RuntimeWarning, 1, "%R has been stopped", self);
    _PyErr_ChainExceptions(exc_type, exc_value, exc_traceback);
}


static inline int
___Watcher_ExecCallback(ev_loop *loop, ev_watcher *watcher, int revents)
{
    Watcher *self = watcher->data;
    PyObject *pyrevents = NULL, *pyresult = NULL;

    if (revents & EV_ERROR) {
        if (!PyErr_Occurred()) {
            if (errno) { // there's a high probability it is related
                _PyErr_SetFromErrno();
            }
            else {
                PyErr_SetString(Error, "unspecified libev error");
            }
        }
        // warn that we have been stopped
        __Watcher_WarnStopped(self);
    }
    else if (!_Py_INVOKE_VERIFY(self->callback, "watcher callback")) {
        if (self->callback != Py_None) {
            if ((pyrevents = PyLong_FromLong(revents))) {
                pyresult = _PyObject_Callback(
                    self->callback, self, pyrevents, NULL
                );
                if (pyresult) {
                    Py_DECREF(pyresult);
                }
                else {
                    Loop_WarnOrStop(loop, self->callback);
                }
                Py_DECREF(pyrevents);
            }
        }
#if EV_EMBED_ENABLE
        else if (revents & EV_EMBED) {
            ev_embed_sweep(loop, (ev_embed *)watcher);
        }
#endif
    }
    return PyErr_Occurred() ? -1 : 0;
}


static void
__Watcher_Callback(ev_loop *loop, ev_watcher *watcher, int revents)
{
    if (___Watcher_ExecCallback(loop, watcher, revents)) {
        Loop_FullStop(loop);
    }
}


/* --------------------------------------------------------------------------
   Watcher
   -------------------------------------------------------------------------- */

#define __Watcher_CheckState(s, W, a, r) \
    do { \
        if (ev_is_##s((W)->watcher)) { \
            PyErr_Format(Error, "cannot %s watcher while it is " #s, (a)); \
            return (r); \
        } \
    } while (0)

#define __Watcher_CheckActive(W, a, r) __Watcher_CheckState(active, W, a, r)
#define __Watcher_CheckPending(W, a, r) __Watcher_CheckState(pending, W, a, r)

#define __Watcher_CheckAllStates(W, a, r) \
    do { \
        __Watcher_CheckActive(W, a, r); \
        __Watcher_CheckPending(W, a, r); \
    } while (0)


#define __Watcher_CheckCallback(W, cb, r) \
    do { \
        if ((W)->ev_type == EV_EMBED) { \
            _Py_CHECK_CALLABLE_OR_NONE((cb), (r)); \
        } \
        else { \
            _Py_CHECK_CALLABLE((cb), (r)); \
        } \
    } while (0)


/* -------------------------------------------------------------------------- */

static inline Watcher *
__Watcher_Alloc(PyTypeObject *type)
{
    Watcher *self = NULL;

    if ((self = PyObject_GC_NEW(Watcher, type))) {
        self->ev_type = EV_NONE;
        self->watcher = NULL;
        self->loop = NULL;
        self->callback = NULL;
        self->data = NULL;
    }
    return self;
}


static inline int
__Watcher_Setup(Watcher *self, int ev_type, size_t size)
{
    self->ev_type = ev_type;
    if (!(self->watcher = PyMem_Malloc(size))) {
        PyErr_NoMemory();
        return -1;
    }
    ev_init(self->watcher, __Watcher_Callback);
    self->watcher->data = self;
    return 0;
}


static inline Watcher *
__Watcher_New(PyTypeObject *type, int ev_type, size_t size)
{
    Watcher *self = NULL;

    if ((self = __Watcher_Alloc(type))) {
        PyObject_GC_Track(self);
        if (__Watcher_Setup(self, ev_type, size)) {
            Py_CLEAR(self);
        }
    }
    return self;
}


static inline int
__Watcher_Init(
    Watcher *self, Loop *loop, PyObject *callback, PyObject *data, int priority
)
{
    __Watcher_CheckAllStates(self, "init a", -1);
    __Watcher_CheckCallback(self, callback, -1);
    _Py_SET_MEMBER(self->loop, loop);
    _Py_SET_MEMBER(self->callback, callback);
    _Py_SET_MEMBER(self->data, data);
    ev_set_priority(self->watcher, priority);
    return 0;
}


static inline void
__Watcher_Finalize(Watcher *self)
{
/*
    printf("\t__Watcher_Finalize for: ");
    PyObject_Print((PyObject *)self, stdout, 0);
    printf("\n");
*/

    if (self->watcher && self->loop && self->loop->loop) {
        __Watcher_Stop(self->loop->loop, self->watcher, self->ev_type);
    }
}


static inline int
__Watcher_Traverse(Watcher *self, visitproc visit, void *arg)
{
    Py_VISIT(self->data);
    Py_VISIT(self->callback);
    Py_VISIT(self->loop);
    return 0;
}


static inline int
__Watcher_Clear(Watcher *self)
{
    Py_CLEAR(self->data);
    Py_CLEAR(self->callback);
    Py_CLEAR(self->loop);
    return 0;
}


static inline void
__Watcher_Dealloc(Watcher *self)
{
    if (self->watcher) {
        PyMem_Free(self->watcher);
        self->watcher = NULL;
    }
    PyObject_GC_Del(self);
}


/* Watcher_Type ------------------------------------------------------------- */

/* Watcher_Type.tp_init */
static int
Watcher_tp_init(Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"loop", "callback", "data", "priority", NULL};
    Loop *loop = NULL;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args, kwargs, "O!O|Oi:__init__", kwlist,
            &Loop_Type, &loop, &callback, &data, &priority
        )
    ) {
        return -1;
    }
    return __Watcher_Init(self, loop, callback, data, priority);
}


/* Watcher_Type.tp_finalize */
static void
Watcher_tp_finalize(Watcher *self)
{
    __Watcher_Finalize(self);
}


/* Watcher_Type.tp_traverse */
static int
Watcher_tp_traverse(Watcher *self, visitproc visit, void *arg)
{
    return __Watcher_Traverse(self, visit, arg);
}


/* Watcher_Type.tp_clear */
static int
Watcher_tp_clear(Watcher *self)
{
    return __Watcher_Clear(self);
}


/* Watcher_Type.tp_dealloc */
static void
Watcher_tp_dealloc(Watcher *self)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)self)) {
        return;
    }
    PyObject_GC_UnTrack(self);
    __Watcher_Clear(self);
    __Watcher_Dealloc(self);
}


/* Watcher.start() */
static PyObject *
Watcher_start(Watcher *self)
{
    __Watcher_Start(self->loop->loop, self->watcher, self->ev_type);
    Py_RETURN_NONE;
}


/* Watcher.stop() */
static PyObject *
Watcher_stop(Watcher *self)
{
    __Watcher_Stop(self->loop->loop, self->watcher, self->ev_type);
    Py_RETURN_NONE;
}


/* Watcher.invoke(revents) */
static PyObject *
Watcher_invoke(Watcher *self, PyObject *args)
{
    int revents = EV_NONE;

    if (!PyArg_ParseTuple(args, "i:invoke", &revents)) {
        return NULL;
    }
    ev_invoke(self->loop->loop, self->watcher, revents);
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Watcher.feed(revents) */
static PyObject *
Watcher_feed(Watcher *self, PyObject *args)
{
    int revents = EV_NONE;

    if (!PyArg_ParseTuple(args, "i:feed", &revents)) {
        return NULL;
    }
    ev_feed_event(self->loop->loop, self->watcher, revents);
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Watcher.clear() -> int */
static PyObject *
Watcher_clear(Watcher *self)
{
    return PyLong_FromLong(ev_clear_pending(self->loop->loop, self->watcher));
}


/* Watcher_Type.tp_methods */
static PyMethodDef Watcher_tp_methods[] = {
    {
        "start", (PyCFunction)Watcher_start,
        METH_NOARGS, "start()"
    },
    {
        "stop", (PyCFunction)Watcher_stop,
        METH_NOARGS, "stop()"
    },
    {
        "invoke", (PyCFunction)Watcher_invoke,
        METH_VARARGS, "invoke(revents)"
    },
    {
        "feed", (PyCFunction)Watcher_feed,
        METH_VARARGS, "feed(revents)"
    },
    {
        "clear", (PyCFunction)Watcher_clear,
        METH_NOARGS, "clear() -> int"
    },
    {NULL}  /* Sentinel */
};


/* Watcher.callback */
static PyObject *
Watcher_callback_getter(Watcher *self, void *closure)
{
    return __Py_INCREF(self->callback);
}

static int
Watcher_callback_setter(Watcher *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    __Watcher_CheckCallback(self, value, -1);
    _Py_SET_MEMBER(self->callback, value);
    return 0;
}


/* Watcher.data */
static PyObject *
Watcher_data_getter(Watcher *self, void *closure)
{
    return __Py_INCREF(self->data);
}

static int
Watcher_data_setter(Watcher *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    _Py_SET_MEMBER(self->data, value);
    return 0;
}


/* Watcher.priority */
static PyObject *
Watcher_priority_getter(Watcher *self, void *closure)
{
    return PyLong_FromLong(ev_priority(self->watcher));
}

static int
Watcher_priority_setter(Watcher *self, PyObject *value, void *closure)
{
    int priority = -1;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    __Watcher_CheckAllStates(self, "set the 'priority' of a", -1);
    if (((priority = _PyLong_AsInt(value)) == -1) && PyErr_Occurred()) {
        return -1;
    }
    ev_set_priority(self->watcher, priority);
    return 0;
}


/* Watcher.loop */
static PyObject *
Watcher_loop_getter(Watcher *self, void *closure)
{
    return __Py_INCREF((PyObject *)self->loop);
}


/* Watcher.active */
static PyObject *
Watcher_active_getter(Watcher *self, void *closure)
{
    return PyBool_FromLong(ev_is_active(self->watcher));
}


/* Watcher.pending */
static PyObject *
Watcher_pending_getter(Watcher *self, void *closure)
{
    return PyBool_FromLong(ev_is_pending(self->watcher));
}


/* Watcher_Type.tp_getsets */
static PyGetSetDef Watcher_tp_getsets[] = {
    {
        "callback", (getter)Watcher_callback_getter,
        (setter)Watcher_callback_setter, NULL, NULL
    },
    {
        "data", (getter)Watcher_data_getter,
        (setter)Watcher_data_setter, NULL, NULL
    },
    {
        "priority", (getter)Watcher_priority_getter,
        (setter)Watcher_priority_setter, NULL, NULL
    },
    {
        "loop", (getter)Watcher_loop_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {
        "active", (getter)Watcher_active_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {
        "pending", (getter)Watcher_pending_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {NULL}  /* Sentinel */
};


PyTypeObject Watcher_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Watcher",
    .tp_basicsize = sizeof(Watcher),
    .tp_dealloc = (destructor)Watcher_tp_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_FINALIZE,
    .tp_traverse = (traverseproc)Watcher_tp_traverse,
    .tp_clear = (inquiry)Watcher_tp_clear,
    .tp_methods = Watcher_tp_methods,
    .tp_getset = Watcher_tp_getsets,
    .tp_init = (initproc)Watcher_tp_init,
    .tp_finalize = (destructor)Watcher_tp_finalize,
};


/* interface ---------------------------------------------------------------- */

Watcher *
Watcher_Alloc(PyTypeObject *type)
{
    return __Watcher_Alloc(type);
}


int
Watcher_Setup(Watcher *self, int ev_type, size_t size)
{
    return __Watcher_Setup(self, ev_type, size);
}


Watcher *
Watcher_New(PyTypeObject *type, int ev_type, size_t size)
{
    return __Watcher_New(type, ev_type, size);
}


int
Watcher_Init(
    Watcher *self, Loop *loop, PyObject *callback, PyObject *data, int priority
)
{
    return __Watcher_Init(self, loop, callback, data, priority);
}


void
Watcher_Finalize(Watcher *self)
{
    __Watcher_Finalize(self);
}


int
Watcher_Traverse(Watcher *self, visitproc visit, void *arg)
{
    return __Watcher_Traverse(self, visit, arg);
}


int
Watcher_Clear(Watcher *self)
{
    return __Watcher_Clear(self);
}


void
Watcher_Dealloc(Watcher *self)
{
    __Watcher_Dealloc(self);
}


int
Watcher_IsActive(Watcher *self, const char *action)
{
    __Watcher_CheckActive(self, action, -1);
    return 0;
}


int
Watcher_CannotSet(Watcher *self)
{
    return Watcher_IsActive(self, "set a");
}
