/*
#
# Copyright © 2020 Malek Hadj-Ali
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


#if EV_PERIODIC_ENABLE


/* helpers ------------------------------------------------------------------ */

static inline int
__Periodic_CheckArgs(double offset, double interval)
{
    static const double interval_min = 1/8192;

    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(interval, -1);
    if (interval > 0.0) {
        if (interval < interval_min) {
            PyErr_SetString(PyExc_ValueError, "'interval' too small");
            return -1;
        }
        _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(offset, -1);
        if (offset > interval) {
            PyErr_SetString(PyExc_ValueError, "'offset' bigger than 'interval'");
            return -1;
        }
    }
    return 0;
}


/* --------------------------------------------------------------------------
   Periodic
   -------------------------------------------------------------------------- */

static inline Watcher *
__Periodic_New(PyTypeObject *type)
{
    return Watcher_New(type, EV_PERIODIC, sizeof(ev_periodic));
}


static inline int
__Periodic_Set(Watcher *self, double offset, double interval)
{
    if (__Periodic_CheckArgs(offset, interval)) {
        return -1;
    }
    ev_periodic_set((ev_periodic *)self->watcher, offset, interval, 0);
    return 0;
}


/* Periodic_Type ------------------------------------------------------------ */

/* Periodic_Type.tp_new */
static PyObject *
Periodic_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Periodic_New(type);
}


/* Periodic_Type.tp_init */
static int
Periodic_tp_init(Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"offset", "interval",
                             "loop", "callback", "data", "priority", NULL};

    double offset = 0.0, interval = 0.0;
    Loop *loop = NULL;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ddO!O|Oi:__init__", kwlist,
            &offset, &interval,
            &Loop_Type, &loop, &callback, &data, &priority)) {
        return -1;
    }
    if (Watcher_Init(self, loop, callback, data, priority)) {
        return -1;
    }
    return __Periodic_Set(self, offset, interval);
}


/* Periodic.set(offset, interval) */
static PyObject *
Periodic_set(Watcher *self, PyObject *args)
{
    double offset = 0.0, interval = 0.0;

    if (Watcher_CannotSet(self) ||
        !PyArg_ParseTuple(args, "dd:set", &offset, &interval) ||
        __Periodic_Set(self, offset, interval)) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Periodic.reset() */
PyDoc_STRVAR(Periodic_reset_doc,
"reset()");

static PyObject *
Periodic_reset(Watcher *self)
{
    ev_periodic_again(self->loop->loop, (ev_periodic *)self->watcher);
    Py_RETURN_NONE;
}


/* Periodic_Type.tp_methods */
static PyMethodDef Periodic_tp_methods[] = {
    {"set", (PyCFunction)Periodic_set, METH_VARARGS,
     "set(offset, interval)"},
    {"reset", (PyCFunction)Periodic_reset, METH_NOARGS,
     Periodic_reset_doc},
    {NULL}  /* Sentinel */
};


/* Periodic.offset */
static PyObject *
Periodic_offset_getter(Watcher *self, void *closure)
{
    return PyFloat_FromDouble(((ev_periodic *)self->watcher)->offset);
}

static int
Periodic_offset_setter(Watcher *self, PyObject *value, void *closure)
{
    double offset = -1.0;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    if (((offset = PyFloat_AsDouble(value)) == -1.0) && PyErr_Occurred()) {
        return -1;
    }
    if (__Periodic_CheckArgs(offset, ((ev_periodic *)self->watcher)->interval)) {
        return -1;
    }
    ((ev_periodic *)self->watcher)->offset = offset;
    return 0;
}


/* Periodic.interval */
static PyObject *
Periodic_interval_getter(Watcher *self, void *closure)
{
    return PyFloat_FromDouble(((ev_periodic *)self->watcher)->interval);
}

static int
Periodic_interval_setter(Watcher *self, PyObject *value, void *closure)
{
    double interval = -1.0;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    if (((interval = PyFloat_AsDouble(value)) == -1.0) && PyErr_Occurred()) {
        return -1;
    }
    if (__Periodic_CheckArgs(((ev_periodic *)self->watcher)->offset, interval)) {
        return -1;
    }
    ((ev_periodic *)self->watcher)->interval = interval;
    return 0;
}


/* Periodic.at */
static PyObject *
Periodic_at_getter(Watcher *self, void *closure)
{
    return PyFloat_FromDouble(ev_periodic_at((ev_periodic *)self->watcher));
}


/* Periodic_Type.tp_getsets */
static PyGetSetDef Periodic_tp_getsets[] = {
    {"offset", (getter)Periodic_offset_getter,
     (setter)Periodic_offset_setter, NULL, NULL},
    {"interval", (getter)Periodic_interval_getter,
     (setter)Periodic_interval_setter, NULL, NULL},
    {"at", (getter)Periodic_at_getter,
     _Py_READONLY_ATTRIBUTE, NULL, NULL},
    {NULL}  /* Sentinel */
};


PyTypeObject Periodic_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Periodic",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Periodic(offset, interval, loop, callback[, data=None, priority=0])",
    .tp_methods = Periodic_tp_methods,
    .tp_getset = Periodic_tp_getsets,
    .tp_init = (initproc)Periodic_tp_init,
    .tp_new = Periodic_tp_new,
};


/* interface ---------------------------------------------------------------- */

Watcher *
Periodic_New(Loop *loop, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"offset", "interval",
                             "callback", "data", "priority", NULL};

    double offset = 0.0, interval = 0.0;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;
    Watcher *self = NULL;

    if (PyArg_ParseTupleAndKeywords(args, kwargs, "ddO|Oi:periodic", kwlist,
            &offset, &interval,
            &callback, &data, &priority) &&
        (self = __Periodic_New(&Periodic_Type)) &&
        (Watcher_Init(self, loop, callback, data, priority) ||
         __Periodic_Set(self, offset, interval))) {
        Py_CLEAR(self);
    }
    return self;
}


#if EV_PREPARE_ENABLE


/* helpers ------------------------------------------------------------------ */

static inline int
__Scheduler_StopWatcher(ev_loop *loop, ev_prepare *prepare)
{
    Scheduler *self = prepare->data;

    // stop this watcher (prepare)
    ev_prepare_stop(loop, prepare);
    // stop the Scheduler watcher
    ev_periodic_stop(loop, (ev_periodic *)((Watcher *)self)->watcher);
    // warn that we have been stopped
    if (PyErr_WarnFormat(PyExc_RuntimeWarning, 1, "%R has been stopped", self)) {
        self->err_fatal = 1;
    }
    // restore the exception back to the original one
    _PyErr_ChainExceptions(self->err_type, self->err_value, self->err_traceback);
    if (!self->err_fatal) {
        Loop_WarnOrStop(loop, self->reschedule);
    }
    // reset err_ members because we might get restarted
    // (with another reschedule callback for example)
    self->err_type = NULL;
    self->err_value = NULL;
    self->err_traceback = NULL;
    self->err_fatal = 0;
    return PyErr_Occurred() ? -1 : 0;
}


static void
__Scheduler_Stop(ev_loop *loop, ev_prepare *prepare, int revents)
{
    if (__Scheduler_StopWatcher(loop, prepare)) {
        Loop_FullStop(loop);
    }
}


static double
__Scheduler_Reschedule(ev_periodic *periodic, double now)
{
    PyGILState_STATE gstate = PyGILState_Ensure();
    Scheduler *self = periodic->data;
    PyObject *pynow = NULL, *pyresult = NULL;
    double result = -1.0;

    if (_Py_INVOKE_VERIFY(self->reschedule, "reschedule callback") ||
        !(pynow = PyFloat_FromDouble(now))) {
        self->err_fatal = 1;
        goto fail;
    }
    pyresult = _PyObject_Callback(self->reschedule, self, pynow, NULL);
    Py_DECREF(pynow);
    if (!pyresult) {
        goto fail;
    }
    result = PyFloat_AsDouble(pyresult);
    Py_DECREF(pyresult);
    if ((result == -1.0) && PyErr_Occurred()) {
        goto fail;
    }
    if (result < now) {
        PyErr_Format(Error, "%R must return a value >= to the 'now' argument",
                     self->reschedule);
        goto fail;
    }
    goto end;

fail:
    PyErr_Fetch(&self->err_type, &self->err_value, &self->err_traceback);
    ev_prepare_start(((Watcher *)self)->loop->loop, self->prepare);
    result = now + 1e30;

end:
    PyGILState_Release(gstate);
    return result;
}


/* --------------------------------------------------------------------------
   Scheduler
   -------------------------------------------------------------------------- */

static inline Scheduler *
__Scheduler_Alloc(PyTypeObject *type)
{
    Scheduler *self = NULL;

    if ((self = (Scheduler *)Watcher_Alloc(type))) {
        self->prepare = NULL;
        self->reschedule = NULL;
        self->err_type = NULL;
        self->err_value = NULL;
        self->err_traceback = NULL;
        self->err_fatal = 0;
    }
    return self;
}


static inline int
__Scheduler_Setup(Scheduler *self, int ev_type, size_t size)
{
    Watcher *watcher = (Watcher *)self;

    if (Watcher_Setup(watcher, ev_type, size)) {
        return -1;
    }
    if (!(self->prepare = PyMem_Malloc(sizeof(ev_prepare)))) {
        PyErr_NoMemory();
        return -1;
    }
    ev_prepare_init(self->prepare, __Scheduler_Stop);
    ev_set_priority(self->prepare, EV_MAXPRI);
    self->prepare->data = self;
    ev_periodic_set((ev_periodic *)watcher->watcher, .0, .0, __Scheduler_Reschedule);
    return 0;
}


static inline Scheduler *
__Scheduler_New(PyTypeObject *type)
{
    Scheduler *self = NULL;

    if ((self = __Scheduler_Alloc(type))) {
        PyObject_GC_Track(self);
        if (__Scheduler_Setup(self, EV_PERIODIC, sizeof(ev_periodic))) {
            Py_CLEAR(self);
        }
    }
    return self;
}


static inline void
__Scheduler_Finalize(Scheduler *self)
{
    Watcher *watcher = (Watcher *)self;

    if (self->prepare && watcher->loop && watcher->loop->loop) {
        ev_prepare_stop(watcher->loop->loop, self->prepare);
    }
    Watcher_Finalize(watcher);
}


static inline int
__Scheduler_Traverse(Scheduler *self, visitproc visit, void *arg)
{
    Py_VISIT(self->err_traceback);
    Py_VISIT(self->err_value);
    Py_VISIT(self->err_type);
    Py_VISIT(self->reschedule);
    return Watcher_Traverse((Watcher *)self, visit, arg);
}


static inline int
__Scheduler_Clear(Scheduler *self)
{
    Py_CLEAR(self->err_traceback);
    Py_CLEAR(self->err_value);
    Py_CLEAR(self->err_type);
    Py_CLEAR(self->reschedule);
    return Watcher_Clear((Watcher *)self);
}


static inline void
__Scheduler_Dealloc(Scheduler *self)
{
    if (self->prepare) {
        PyMem_Free(self->prepare);
        self->prepare = NULL;
    }
    Watcher_Dealloc((Watcher *)self);
}


static inline int
__Scheduler_Set(Scheduler *self, PyObject *reschedule)
{
    _Py_CHECK_CALLABLE(reschedule, -1);
    _Py_SET_MEMBER(self->reschedule, reschedule);
    return 0;
}


/* Scheduler_Type ----------------------------------------------------------- */

/* Scheduler_Type.tp_new */
static PyObject *
Scheduler_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Scheduler_New(type);
}


/* Scheduler_Type.tp_init */
static int
Scheduler_tp_init(Scheduler *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"reschedule",
                             "loop", "callback", "data", "priority", NULL};

    PyObject *reschedule = NULL;
    Loop *loop = NULL;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO!O|Oi:__init__", kwlist,
            &reschedule,
            &Loop_Type, &loop, &callback, &data, &priority)) {
        return -1;
    }
    if (Watcher_Init((Watcher *)self, loop, callback, data, priority)) {
        return -1;
    }
    return __Scheduler_Set(self, reschedule);
}


/* Scheduler_Type.tp_finalize */
static void
Scheduler_tp_finalize(Scheduler *self)
{
    __Scheduler_Finalize(self);
}


/* Scheduler_Type.tp_traverse */
static int
Scheduler_tp_traverse(Scheduler *self, visitproc visit, void *arg)
{
    return __Scheduler_Traverse(self, visit, arg);
}


/* Scheduler_Type.tp_clear */
static int
Scheduler_tp_clear(Scheduler *self)
{
    return __Scheduler_Clear(self);
}


/* Scheduler_Type.tp_dealloc */
static void
Scheduler_tp_dealloc(Scheduler *self)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)self)) {
        return;
    }
    PyObject_GC_UnTrack(self);
    __Scheduler_Clear(self);
    __Scheduler_Dealloc(self);
}


/* Scheduler_Type.tp_methods */
static PyMethodDef Scheduler_tp_methods[] = {
    {"reset", (PyCFunction)Periodic_reset, METH_NOARGS,
     Periodic_reset_doc},
    {NULL}  /* Sentinel */
};


/* Scheduler.reschedule */
static PyObject *
Scheduler_reschedule_getter(Scheduler *self, void *closure)
{
    return __Py_INCREF(self->reschedule);
}

static int
Scheduler_reschedule_setter(Scheduler *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    return __Scheduler_Set(self, value);
}


/* Scheduler_Type.tp_getsets */
static PyGetSetDef Scheduler_tp_getsets[] = {
    {"reschedule", (getter)Scheduler_reschedule_getter,
     (setter)Scheduler_reschedule_setter, NULL, NULL},
    {"at", (getter)Periodic_at_getter,
     _Py_READONLY_ATTRIBUTE, NULL, NULL},
    {NULL}  /* Sentinel */
};


PyTypeObject Scheduler_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Scheduler",
    .tp_basicsize = sizeof(Scheduler),
    .tp_dealloc = (destructor)Scheduler_tp_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_FINALIZE,
    .tp_doc = "Scheduler(reschedule, loop, callback[, data=None, priority=0])",
    .tp_traverse = (traverseproc)Scheduler_tp_traverse,
    .tp_clear = (inquiry)Scheduler_tp_clear,
    .tp_methods = Scheduler_tp_methods,
    .tp_getset = Scheduler_tp_getsets,
    .tp_init = (initproc)Scheduler_tp_init,
    .tp_new = Scheduler_tp_new,
    .tp_finalize = (destructor)Scheduler_tp_finalize,
};


/* interface ---------------------------------------------------------------- */

Scheduler *
Scheduler_New(Loop *loop, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"reschedule",
                             "callback", "data", "priority", NULL};

    PyObject *reschedule = NULL;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;
    Scheduler *self = NULL;

    if (PyArg_ParseTupleAndKeywords(args, kwargs, "OO|Oi:scheduler", kwlist,
            &reschedule,
            &callback, &data, &priority) &&
        (self = __Scheduler_New(&Scheduler_Type)) &&
        (Watcher_Init((Watcher *)self, loop, callback, data, priority) ||
         __Scheduler_Set(self, reschedule))) {
        Py_CLEAR(self);
    }
    return self;
}


#endif // !EV_PREPARE_ENABLE


#endif // !EV_PERIODIC_ENABLE

