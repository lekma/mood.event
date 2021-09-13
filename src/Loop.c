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

static inline void
__Loop_FullStop(ev_loop *loop)
{
    ev_break(loop, EVBREAK_ALL);
}


static inline int
__Loop_Fatal(PyObject *context)
{
    PyObject *exc_type, *exc_value, *exc_traceback;
    _Py_IDENTIFIER(__err_fatal__);
    int fatal = 0;

    PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
    fatal = (_PyObject_HasAttrId(context, &PyId___err_fatal__) != 0);
    _PyErr_ChainExceptions(exc_type, exc_value, exc_traceback);
    return fatal;
}


static inline void
__Loop_WarnOrStop(ev_loop *loop, PyObject *context)
{
    Loop *self = ev_userdata(loop);

    if (
        !__Loop_Fatal(context) && (self->callback == Py_None) &&
        PyErr_ExceptionMatches(PyExc_Exception)
    ) {
        PyErr_WriteUnraisable(context);
    }
}


static inline int
__Loop_ExecCallback(ev_loop *loop)
{
    Loop *self = ev_userdata(loop);

    if (!_Py_INVOKE_VERIFY(self->callback, "loop callback")) {
        if (self->callback != Py_None) {
            Py_XDECREF(_PyObject_Callback(self->callback, self, NULL));
        }
        else {
            ev_invoke_pending(loop);
        }
    }
    return PyErr_Occurred() ? -1 : 0;
}


static void
__Loop_InvokePending(ev_loop *loop)
{
    PyGILState_STATE gstate = PyGILState_Ensure();

    if (__Loop_ExecCallback(loop)) {
        __Loop_FullStop(loop);
    }
    PyGILState_Release(gstate);
}


/* --------------------------------------------------------------------------
   Loop
   -------------------------------------------------------------------------- */

#define __Loop_SetInterval(L, t, ival) \
    do { \
        ev_set_##t##_collect_interval((L)->loop, (ival)); \
        (L)->t##_ival = (ival); \
    } while (0)


/* -------------------------------------------------------------------------- */

static inline Loop *
__Loop_Alloc(PyTypeObject *type)
{
    Loop *self = NULL;

    if ((self = PyObject_GC_NEW(Loop, type))) {
        self->loop = NULL;
        self->callback = NULL;
        self->data = NULL;
        self->io_ival = 0.0;
        self->timeout_ival = 0.0;
    }
    return self;
}


static inline int
__Loop_Setup(Loop *self, PyObject *args, PyObject *kwargs, int _default)
{
    static char *kwlist[] = {
        "flags", "callback", "data", "io_interval", "timeout_interval", NULL
    };
    unsigned int flags = EVFLAG_AUTO;
    PyObject *callback = Py_None, *data = Py_None;
    double io_ival = 0.0, timeout_ival = 0.0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args, kwargs, "|IOOdd:__new__", kwlist,
            &flags, &callback, &data, &io_ival, &timeout_ival
        )
    ) {
        return -1;
    }
    if (!(self->loop = _default ? ev_default_loop(flags) : ev_loop_new(flags))) {
        PyErr_SetString(Error, "could not create loop, bad 'flags'?");
        return -1;
    }
    _Py_CHECK_CALLABLE_OR_NONE(callback, -1);
    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(io_ival, -1);
    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(timeout_ival, -1);
    _Py_SET_MEMBER(self->callback, callback);
    _Py_SET_MEMBER(self->data, data);
    __Loop_SetInterval(self, io, io_ival);
    __Loop_SetInterval(self, timeout, timeout_ival);
    ev_set_invoke_pending_cb(self->loop, __Loop_InvokePending);
    ev_set_userdata(self->loop, self);
    return 0;
}


static inline PyObject *
__Loop_New(PyTypeObject *type, PyObject *args, PyObject *kwargs, int _default)
{
    Loop *self = NULL;

    if ((self = __Loop_Alloc(type))) {
        PyObject_GC_Track(self);
        if (__Loop_Setup(self, args, kwargs, _default)) {
            Py_CLEAR(self);
        }
    }
    return (PyObject *)self;
}


static inline void
__Loop_Finalize(Loop *self)
{
    if (self->loop) {
        __Loop_FullStop(self->loop);
    }
}


static inline int
__Loop_Traverse(Loop *self, visitproc visit, void *arg)
{
    Py_VISIT(self->data);
    Py_VISIT(self->callback);
    return 0;
}


static inline int
__Loop_Clear(Loop *self)
{
    Py_CLEAR(self->data);
    Py_CLEAR(self->callback);
    return 0;
}


static inline void
__Loop_Dealloc(Loop *self)
{
    if (self->loop) {
        if (ev_is_default_loop(self->loop)) {
            DefaultLoop = NULL;
        }
        ev_loop_destroy(self->loop);
        self->loop = NULL;
    }
    PyObject_GC_Del(self);
}


/* Loop_Type ---------------------------------------------------------------- */

/* Loop_Type.tp_new */
static PyObject *
Loop_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return __Loop_New(type, args, kwargs, 0);
}


/* Loop_Type.tp_finalize */
static void
Loop_tp_finalize(Loop *self)
{
    __Loop_Finalize(self);
}


/* Loop_Type.tp_traverse */
static int
Loop_tp_traverse(Loop *self, visitproc visit, void *arg)
{
    return __Loop_Traverse(self, visit, arg);
}


/* Loop_Type.tp_clear */
static int
Loop_tp_clear(Loop *self)
{
    return __Loop_Clear(self);
}


/* Loop_Type.tp_dealloc */
static void
Loop_tp_dealloc(Loop *self)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)self)) {
        return;
    }
    PyObject_GC_UnTrack(self);
    __Loop_Clear(self);
    __Loop_Dealloc(self);
}


/* Loop.start([flags]) -> bool */
static PyObject *
Loop_start(Loop *self, PyObject *args)
{
    int flags = 0, result = 0;

    if (!PyArg_ParseTuple(args, "|i:start", &flags)) {
        return NULL;
    }
    Py_BEGIN_ALLOW_THREADS
    result = ev_run(self->loop, flags);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) {
        return NULL;
    }
    return PyBool_FromLong(result);
}


/* Loop.stop([how]) */
static PyObject *
Loop_stop(Loop *self, PyObject *args)
{
    int how = EVBREAK_ONE;

    if (!PyArg_ParseTuple(args, "|i:stop", &how)) {
        return NULL;
    }
    ev_break(self->loop, how);
    Py_RETURN_NONE;
}


/* Loop.now([update]) -> float */
static PyObject *
Loop_now(Loop *self, PyObject *args)
{
    int update = 0;

    if (!PyArg_ParseTuple(args, "|p:now", &update)) {
        return NULL;
    }
    if (update) {
        ev_now_update(self->loop);
    }
    return PyFloat_FromDouble(ev_now(self->loop));
}


/* Loop.invoke() */
static PyObject *
Loop_invoke(Loop *self)
{
    ev_invoke_pending(self->loop);
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Loop.reset() */
static PyObject *
Loop_reset(Loop *self)
{
    ev_loop_fork(self->loop);
    Py_RETURN_NONE;
}


/* Loop.suspend()/Loop.resume() */
static PyObject *
Loop_suspend(Loop *self)
{
    ev_suspend(self->loop);
    Py_RETURN_NONE;
}

static PyObject *
Loop_resume(Loop *self)
{
    ev_resume(self->loop);
    Py_RETURN_NONE;
}


/* Loop.incref()/Loop.decref() */
static PyObject *
Loop_incref(Loop *self)
{
    ev_ref(self->loop);
    Py_RETURN_NONE;
}

static PyObject *
Loop_decref(Loop *self)
{
    ev_unref(self->loop);
    Py_RETURN_NONE;
}


/* Loop.verify() */
static PyObject *
Loop_verify(Loop *self)
{
    ev_verify(self->loop);
    Py_RETURN_NONE;
}


/* Loop_Type.tp_methods */
static PyMethodDef Loop_tp_methods[] = {
    {
        "start", (PyCFunction)Loop_start,
        METH_VARARGS, "start([flags]) -> bool"
    },
    {
        "stop", (PyCFunction)Loop_stop,
        METH_VARARGS, "stop([how])"
    },
    {
        "now", (PyCFunction)Loop_now,
        METH_VARARGS, "now([update]) -> float"
    },
    {
        "invoke", (PyCFunction)Loop_invoke,
        METH_NOARGS, "invoke()"
    },
    {
        "reset", (PyCFunction)Loop_reset,
        METH_NOARGS, "reset()"
    },
    {
        "suspend", (PyCFunction)Loop_suspend,
        METH_NOARGS, "suspend()"
    },
    {
        "resume", (PyCFunction)Loop_resume,
        METH_NOARGS, "resume()"
    },
    {
        "incref", (PyCFunction)Loop_incref,
        METH_NOARGS, "incref()"
    },
    {
        "decref", (PyCFunction)Loop_decref,
        METH_NOARGS, "decref()"
    },
    {
        "verify", (PyCFunction)Loop_verify,
        METH_NOARGS, "verify()"
    },
    /* watcher methods */
    {
        "io", (PyCFunction)Io_New,
        METH_VARARGS | METH_KEYWORDS,
        "io(fd, events, callback[, data=None, priority=0]) -> Io"
    },
    {
        "timer", (PyCFunction)Timer_New,
        METH_VARARGS | METH_KEYWORDS,
        "timer(after, repeat, callback[, data=None, priority=0]) -> Timer"
    },
#if EV_PERIODIC_ENABLE
    {
        "periodic", (PyCFunction)Periodic_New,
        METH_VARARGS | METH_KEYWORDS,
        "periodic(offset, interval, callback[, data=None, priority=0]) -> Periodic"
    },
#if EV_PREPARE_ENABLE
    {
        "scheduler", (PyCFunction)Scheduler_New,
        METH_VARARGS | METH_KEYWORDS,
        "scheduler(reschedule, callback[, data=None, priority=0]) -> Scheduler"
    },
#endif
#endif
#if EV_SIGNAL_ENABLE
    {
        "signal", (PyCFunction)Signal_New,
        METH_VARARGS | METH_KEYWORDS,
        "signal(signum, callback[, data=None, priority=0]) -> Signal"
    },
#endif
#if EV_CHILD_ENABLE
    {
        "child", (PyCFunction)Child_New,
        METH_VARARGS | METH_KEYWORDS,
        "child(pid, trace, callback[, data=None, priority=0]) -> Child"
    },
#endif
#if EV_IDLE_ENABLE
    {
        "idle", (PyCFunction)Idle_New,
        METH_VARARGS | METH_KEYWORDS,
        "idle(callback[, data=None, priority=0]) -> Idle"
    },
#endif
#if EV_PREPARE_ENABLE
    {
        "prepare", (PyCFunction)Prepare_New,
        METH_VARARGS | METH_KEYWORDS,
        "prepare(callback[, data=None, priority=0]) -> Prepare"
    },
#endif
#if EV_CHECK_ENABLE
    {
        "check", (PyCFunction)Check_New,
        METH_VARARGS | METH_KEYWORDS,
        "check(callback[, data=None, priority=0]) -> Check"
    },
#endif
#if EV_EMBED_ENABLE
    {
        "embed", (PyCFunction)Embed_New,
        METH_VARARGS | METH_KEYWORDS,
        "embed(other[, callback=None, data=None, priority=0]) -> Embed"
    },
#endif
#if EV_FORK_ENABLE
    {
        "fork", (PyCFunction)Fork_New,
        METH_VARARGS | METH_KEYWORDS,
        "fork(callback[, data=None, priority=0]) -> Fork"
    },
#endif
#if EV_ASYNC_ENABLE
    {
        "async", (PyCFunction)Async_New,
        METH_VARARGS | METH_KEYWORDS,
        "async(callback[, data=None, priority=0]) -> Async"
    },
#endif
    {NULL}  /* Sentinel */
};


/* Loop.callback */
static PyObject *
Loop_callback_getter(Loop *self, void *closure)
{
    return __Py_INCREF(self->callback);
}

static int
Loop_callback_setter(Loop *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    _Py_CHECK_CALLABLE_OR_NONE(value, -1);
    _Py_SET_MEMBER(self->callback, value);
    return 0;
}


/* Loop.data */
static PyObject *
Loop_data_getter(Loop *self, void *closure)
{
    return __Py_INCREF(self->data);
}

static int
Loop_data_setter(Loop *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    _Py_SET_MEMBER(self->data, value);
    return 0;
}


/* Loop.io_interval/Loop.timeout_interval */
static PyObject *
Loop_interval_getter(Loop *self, void *closure)
{
    return PyFloat_FromDouble(closure ? self->io_ival : self->timeout_ival);
}

static int
Loop_interval_setter(Loop *self, PyObject *value, void *closure)
{
    double ival = -1.0;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    if (((ival = PyFloat_AsDouble(value)) == -1.0) && PyErr_Occurred()) {
        return -1;
    }
    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(ival, -1);
    if (closure) {
        __Loop_SetInterval(self, io, ival);
    }
    else {
        __Loop_SetInterval(self, timeout, ival);
    }
    return 0;
}


/* Loop.default */
static PyObject *
Loop_default_getter(Loop *self, void *closure)
{
    return PyBool_FromLong(ev_is_default_loop(self->loop));
}


/* Loop.iteration */
static PyObject *
Loop_iteration_getter(Loop *self, void *closure)
{
    return PyLong_FromUnsignedLong(ev_iteration(self->loop));
}


/* Loop.depth */
static PyObject *
Loop_depth_getter(Loop *self, void *closure)
{
    return PyLong_FromUnsignedLong(ev_depth(self->loop));
}


/* Loop.backend */
static PyObject *
Loop_backend_getter(Loop *self, void *closure)
{
    return PyLong_FromUnsignedLong(ev_backend(self->loop));
}


/* Loop.pending */
static PyObject *
Loop_pending_getter(Loop *self, void *closure)
{
    return PyLong_FromUnsignedLong(ev_pending_count(self->loop));
}


/* Loop_Type.tp_getsets */
static PyGetSetDef Loop_tp_getsets[] = {
    {
        "callback", (getter)Loop_callback_getter,
        (setter)Loop_callback_setter, NULL, NULL
    },
    {
        "data", (getter)Loop_data_getter,
        (setter)Loop_data_setter, NULL, NULL
    },
    {
        "io_interval", (getter)Loop_interval_getter,
        (setter)Loop_interval_setter, NULL, Py_True
    },
    {
        "timeout_interval", (getter)Loop_interval_getter,
        (setter)Loop_interval_setter, NULL, NULL
    },
    {
        "default", (getter)Loop_default_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {
        "iteration", (getter)Loop_iteration_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {
        "depth", (getter)Loop_depth_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {
        "backend", (getter)Loop_backend_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {
        "pending", (getter)Loop_pending_getter,
        _Py_READONLY_ATTRIBUTE, NULL, NULL
    },
    {NULL}  /* Sentinel */
};


PyTypeObject Loop_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Loop",
    .tp_basicsize = sizeof(Loop),
    .tp_dealloc = (destructor)Loop_tp_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_FINALIZE,
    .tp_doc = "Loop([flags=EVFLAG_AUTO, callback=None, data=None, io_interval=0.0, timeout_interval=0.0])",
    .tp_traverse = (traverseproc)Loop_tp_traverse,
    .tp_clear = (inquiry)Loop_tp_clear,
    .tp_methods = Loop_tp_methods,
    .tp_getset = Loop_tp_getsets,
    .tp_new = Loop_tp_new,
    .tp_finalize = (destructor)Loop_tp_finalize,
};


/* interface ---------------------------------------------------------------- */

PyObject *
Loop_New(PyObject *args, PyObject *kwargs, int _default)
{
    return __Loop_New(&Loop_Type, args, kwargs, _default);
}


void
Loop_FullStop(ev_loop *loop)
{
    __Loop_FullStop(loop);
}


void
Loop_WarnOrStop(ev_loop *loop, PyObject *context)
{
    __Loop_WarnOrStop(loop, context);
}

