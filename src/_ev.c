/*
#
# Copyright © 2017 Malek Hadj-Ali
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


#define PY_SSIZE_T_CLEAN
#include "Python.h"

#include "helpers/helpers.h"

#include <ev.h>


/* ----------------------------------------------------------------------------
 helpers
 ---------------------------------------------------------------------------- */

#define _Py_CHECK_CALLABLE(cb, r) \
    do { \
        if (!PyCallable_Check((cb))) { \
            PyErr_SetString(PyExc_TypeError, "a callable is required"); \
            return (r); \
        } \
    } while (0)

#define _Py_CHECK_CALLABLE_OR_NONE(cb, r) \
    do { \
        if ((cb) != Py_None && !PyCallable_Check((cb))) { \
            PyErr_SetString(PyExc_TypeError, "a callable or None is required"); \
            return (r); \
        } \
    } while (0)

#define _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(v, r) \
    do { \
        if ((v) < 0.0) { \
            PyErr_SetString(PyExc_ValueError, \
                            "a positive float or 0.0 is required"); \
            return (r); \
        } \
    } while (0)

#define _Py_CHECK_INT_ATTRIBUTE(v, r) \
    do { \
        if ((v) == -1 && PyErr_Occurred()) { \
            return (r); \
        } \
        else if ((v) > INT_MAX) { \
            PyErr_SetString(PyExc_OverflowError, \
                            "signed integer is greater than maximum"); \
            return (r); \
        } \
        else if ((v) < INT_MIN) { \
            PyErr_SetString(PyExc_OverflowError, \
                            "signed integer is less than minimum"); \
            return (r); \
        } \
    } while (0)


#define _Py_PROTECTED_ATTRIBUTE(v, r) \
    do { \
        if ((v) == NULL) { \
            PyErr_SetString(PyExc_TypeError, "cannot delete attribute"); \
            return (r); \
        } \
    } while (0)

static int
_Readonly_attribute_set(PyObject *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    PyErr_SetString(PyExc_AttributeError, "readonly attribute");
    return -1;
}


/* ----------------------------------------------------------------------------
 objects
 ---------------------------------------------------------------------------- */

static PyObject *Error = NULL;
static PyObject *DefaultLoop = NULL;


/* Loop */
typedef struct {
    PyObject_HEAD
    struct ev_loop *loop;
    PyThreadState *tstate;
    PyObject *callback;
    PyObject *data;
    double io_ival;
    double timeout_ival;
} Loop;


/* _Watcher base - not exposed */
typedef struct {
    PyObject_HEAD
    int ev_type;
    ev_watcher *watcher;
    Loop *loop;
    PyObject *callback;
    PyObject *data;
} _Watcher;


/* fwd declarations */
static PyTypeObject LoopType;
static PyTypeObject IoType;
static PyTypeObject TimerType;

#if EV_PERIODIC_ENABLE
static PyTypeObject PeriodicType;
#if EV_PREPARE_ENABLE
/* Scheduler */
typedef struct {
    _Watcher _watcher;
    ev_prepare *prepare;
    PyObject *scheduler;
    PyObject *err_type;
    PyObject *err_value;
    PyObject *err_traceback;
    int err_fatal;
} Scheduler;
static PyTypeObject SchedulerType;
#endif
#endif

#if EV_SIGNAL_ENABLE
static PyTypeObject SignalType;
#endif

#if EV_CHILD_ENABLE
static PyTypeObject ChildType;
#endif

#if EV_IDLE_ENABLE
static PyTypeObject IdleType;
#endif

#if EV_PREPARE_ENABLE
static PyTypeObject PrepareType;
#endif

#if EV_CHECK_ENABLE
static PyTypeObject CheckType;
#endif

#if EV_EMBED_ENABLE
/* Embed */
typedef struct {
    _Watcher _watcher;
    Loop *other;
} Embed;
static PyTypeObject EmbedType;
#endif

#if EV_FORK_ENABLE
static PyTypeObject ForkType;
#endif

#if EV_ASYNC_ENABLE
static PyTypeObject AsyncType;
#endif


/* ----------------------------------------------------------------------------
 types
 ---------------------------------------------------------------------------- */

/*
XXX: yeah, I know. But it makes my life so much easier and
     the size of the resulting module is so much smaller ¯\_(ツ)_/¯
*/

#include "Loop.c"
#include "_Watcher.c"
#include "Io.c"
#include "Timer.c"

#if EV_PERIODIC_ENABLE
#include "Periodic.c"
#if EV_PREPARE_ENABLE
#include "Scheduler.c"
#endif
#endif

#if EV_SIGNAL_ENABLE
#include "Signal.c"
#endif

#if EV_CHILD_ENABLE
#include "Child.c"
#endif

#if EV_IDLE_ENABLE
#include "Idle.c"
#endif

#if EV_PREPARE_ENABLE
#include "Prepare.c"
#endif

#if EV_CHECK_ENABLE
#include "Check.c"
#endif

#if EV_EMBED_ENABLE
#include "Embed.c"
#endif

#if EV_FORK_ENABLE
#include "Fork.c"
#endif

#if EV_ASYNC_ENABLE
#include "Async.c"
#endif


/* ----------------------------------------------------------------------------
 libev global callbacks
 ---------------------------------------------------------------------------- */

/* allocate memory from the Python heap */
static void *
_ev_allocator(void *ptr, long size)
{
    void *result = NULL;

    if (size || ptr) {
        // since https://hg.python.org/cpython/rev/ca78c974e938
        // the GIL is always needed
        PyGILState_STATE gstate = PyGILState_Ensure();
        if (size) {
            result = PyObject_Realloc(ptr, size);
        }
        else {
            PyObject_Free(ptr);
        }
        PyGILState_Release(gstate);
    }
    return result;
}

#define _ev_syserr_cb Py_FatalError


/* ----------------------------------------------------------------------------
 module
 ---------------------------------------------------------------------------- */

/* _ev_def.m_doc */
PyDoc_STRVAR(_ev_m_doc,
"Python libev interface");


/* _ev.default_loop([flags=EVFLAG_AUTO, callback=None, data=None,
                     io_interval=0.0, timeout_interval=0.0]) -> 'default loop' */
PyDoc_STRVAR(_ev_default_loop_doc,
"default_loop([flags=EVFLAG_AUTO, callback=None, data=None,\n\
               io_interval=0.0, timeout_interval=0.0]) -> 'default loop'");

static PyObject *
_ev_default_loop(PyObject *module, PyObject *args, PyObject *kwargs)
{
    if (!DefaultLoop) {
        DefaultLoop = _Loop_New(args, kwargs, 1);
    }
    else {
        if (PyErr_WarnEx(PyExc_RuntimeWarning,
                         "returning the 'default loop' created earlier, "
                         "arguments ignored (if provided).", 1)) {
            return NULL;
        }
        Py_INCREF(DefaultLoop);
    }
    return DefaultLoop;
}


/* _ev.fatal */
PyDoc_STRVAR(_ev_fatal_doc,
"fatal decorator");

static PyObject *
_ev_fatal(PyObject *module, PyObject *obj)
{
    _Py_IDENTIFIER(__err_fatal__);

    if (_PyObject_SetAttrId(obj, &PyId___err_fatal__, Py_True)) {
        return NULL;
    }
    _Py_RETURN_OBJECT(obj);
}


/* _ev.time() -> float */
PyDoc_STRVAR(_ev_time_doc,
"time() -> float");

static PyObject *
_ev_time(PyObject *module)
{
    return PyFloat_FromDouble(ev_time());
}


/* _ev.sleep(interval) */
PyDoc_STRVAR(_ev_sleep_doc,
"sleep(interval)");

static PyObject *
_ev_sleep(PyObject *module, PyObject *args)
{
    double interval;

    if (!PyArg_ParseTuple(args, "d:sleep", &interval)) {
        return NULL;
    }
    if (interval > 86400.0 &&
        PyErr_WarnEx(PyExc_RuntimeWarning,
                     "'interval' bigger than a day (86400.0), "
                     "this is not garanteed to work", 1)) {
        return NULL;
    }
    Py_BEGIN_ALLOW_THREADS
    ev_sleep(interval);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* _ev.abi_version() -> (int, int) */
PyDoc_STRVAR(_ev_abi_version_doc,
"abi_version() -> (int, int)");

static PyObject *
_ev_abi_version(PyObject *module)
{
    return Py_BuildValue("ii", ev_version_major(), ev_version_minor());
}


/* _ev.supported_backends() -> int */
PyDoc_STRVAR(_ev_supported_backends_doc,
"supported_backends() -> int");

static PyObject *
_ev_supported_backends(PyObject *module)
{
    return PyLong_FromUnsignedLong(ev_supported_backends());
}


/* _ev.recommended_backends() -> int */
PyDoc_STRVAR(_ev_recommended_backends_doc,
"recommended_backends() -> int");

static PyObject *
_ev_recommended_backends(PyObject *module)
{
    return PyLong_FromUnsignedLong(ev_recommended_backends());
}


/* _ev.embeddable_backends() -> int */
PyDoc_STRVAR(_ev_embeddable_backends_doc,
"embeddable_backends() -> int");

static PyObject *
_ev_embeddable_backends(PyObject *module)
{
    return PyLong_FromUnsignedLong(ev_embeddable_backends());
}


#if EV_SIGNAL_ENABLE
/* _ev.feed_signal(signum) */
PyDoc_STRVAR(_ev_feed_signal_doc,
"feed_signal(signum)");

static PyObject *
_ev_feed_signal(PyObject *module, PyObject *args)
{
    int signum;

    if (!PyArg_ParseTuple(args, "i:feed_signal", &signum)) {
        return NULL;
    }
    ev_feed_signal(signum);
    Py_RETURN_NONE;
}
#endif


/* _ev_def.m_methods */
static PyMethodDef _ev_m_methods[] = {
    {"default_loop", (PyCFunction)_ev_default_loop,
     METH_VARARGS | METH_KEYWORDS, _ev_default_loop_doc},
    {"fatal", (PyCFunction)_ev_fatal,
     METH_O, _ev_fatal_doc},
    {"time", (PyCFunction)_ev_time,
     METH_NOARGS, _ev_time_doc},
    {"sleep", (PyCFunction)_ev_sleep,
     METH_VARARGS, _ev_sleep_doc},
    {"abi_version", (PyCFunction)_ev_abi_version,
     METH_NOARGS, _ev_abi_version_doc},
    {"supported_backends", (PyCFunction)_ev_supported_backends,
     METH_NOARGS, _ev_supported_backends_doc},
    {"recommended_backends", (PyCFunction)_ev_recommended_backends,
     METH_NOARGS, _ev_recommended_backends_doc},
    {"embeddable_backends", (PyCFunction)_ev_embeddable_backends,
     METH_NOARGS, _ev_embeddable_backends_doc},
#if EV_SIGNAL_ENABLE
    {"feed_signal", (PyCFunction)_ev_feed_signal,
     METH_VARARGS, _ev_feed_signal_doc},
#endif
    {NULL} /* Sentinel */
};


/* _ev_def.m_traverse */
static int
_ev_m_traverse(PyObject *module, visitproc visit, void *arg)
{
    Py_VISIT(Error);
    return 0;
}


/* _ev_def.m_clear */
static int
_ev_m_clear(PyObject *module)
{
    Py_CLEAR(Error);
    return 0;
}


/* _ev_def.m_free */
static void
_ev_m_free(PyObject *module)
{
    _ev_m_clear(module);
}


/* _ev_def
   unfortunately this module cannot support sub-interpreters
   because there can be only one DefaultLoop per process */
static PyModuleDef _ev_def = {
    PyModuleDef_HEAD_INIT,
    "_ev",                                    /* m_name */
    _ev_m_doc,                                /* m_doc */
    -1,                                       /* m_size */
    _ev_m_methods,                            /* m_methods */
    NULL,                                     /* m_slots */
    (traverseproc)_ev_m_traverse,             /* m_traverse */
    (inquiry)_ev_m_clear,                     /* m_clear */
    (freefunc)_ev_m_free                      /* m_free */
};


/* module initialization */
PyMODINIT_FUNC
PyInit__ev(void)
{
    PyObject *module = NULL;

    if ((module = PyModule_Create(&_ev_def))) {
        if (
            PyModule_AddStringConstant(module, "__version__", PKG_VERSION) ||
            _PyModule_AddNewException(module, "Error", "mood.event", NULL, NULL, &Error) ||

            // Loop
            _PyModule_AddType(module, "Loop", &LoopType) ||
            _PyModule_AddUnsignedIntMacro(module, EVFLAG_AUTO) ||
            _PyModule_AddUnsignedIntMacro(module, EVFLAG_NOENV) ||
            _PyModule_AddUnsignedIntMacro(module, EVFLAG_FORKCHECK) ||
            _PyModule_AddUnsignedIntMacro(module, EVFLAG_SIGNALFD) ||
            _PyModule_AddUnsignedIntMacro(module, EVFLAG_NOSIGMASK) ||
            _PyModule_AddUnsignedIntMacro(module, EVBACKEND_SELECT) ||
            _PyModule_AddUnsignedIntMacro(module, EVBACKEND_POLL) ||
            _PyModule_AddUnsignedIntMacro(module, EVBACKEND_EPOLL) ||
            _PyModule_AddUnsignedIntMacro(module, EVBACKEND_KQUEUE) ||
            _PyModule_AddUnsignedIntMacro(module, EVBACKEND_DEVPOLL) ||
            _PyModule_AddUnsignedIntMacro(module, EVBACKEND_PORT) ||
            _PyModule_AddUnsignedIntMacro(module, EVBACKEND_ALL) ||
            _PyModule_AddUnsignedIntMacro(module, EVBACKEND_MASK) ||
            _PyModule_AddIntMacro(module, EVRUN_NOWAIT) ||
            _PyModule_AddIntMacro(module, EVRUN_ONCE) ||
            _PyModule_AddIntMacro(module, EVBREAK_ONE) ||
            _PyModule_AddIntMacro(module, EVBREAK_ALL) ||

            // _Watcher
            PyType_Ready(&_WatcherType) ||
            _PyModule_AddIntMacro(module, EV_MINPRI) ||
            _PyModule_AddIntMacro(module, EV_MAXPRI) ||

            // Io
            _PyModule_AddTypeWithBase(module, "Io",
                                      &IoType, &_WatcherType) ||
            _PyModule_AddIntMacro(module, EV_READ) ||
            _PyModule_AddIntMacro(module, EV_WRITE) ||
            _PyModule_AddIntMacro(module, EV_IO) ||

            // Timer
            _PyModule_AddTypeWithBase(module, "Timer",
                                      &TimerType, &_WatcherType) ||
            _PyModule_AddIntMacro(module, EV_TIMER) ||

#if EV_PERIODIC_ENABLE
            // Periodic
            _PyModule_AddTypeWithBase(module, "Periodic",
                                      &PeriodicType, &_WatcherType) ||
#if EV_PREPARE_ENABLE
            // Scheduler
            _PyModule_AddTypeWithBase(module, "Scheduler",
                                      &SchedulerType, &_WatcherType) ||
#endif
            _PyModule_AddIntMacro(module, EV_PERIODIC) ||
#endif

#if EV_SIGNAL_ENABLE
            // Signal
            _PyModule_AddTypeWithBase(module, "Signal",
                                      &SignalType, &_WatcherType) ||
            _PyModule_AddIntMacro(module, EV_SIGNAL) ||
#endif

#if EV_CHILD_ENABLE
            // Child
            _PyModule_AddTypeWithBase(module, "Child",
                                      &ChildType, &_WatcherType) ||
            _PyModule_AddIntMacro(module, EV_CHILD) ||
#endif

#if EV_IDLE_ENABLE
            // Idle
            _PyModule_AddTypeWithBase(module, "Idle",
                                      &IdleType, &_WatcherType) ||
            _PyModule_AddIntMacro(module, EV_IDLE) ||
#endif

#if EV_PREPARE_ENABLE
            // Prepare
            _PyModule_AddTypeWithBase(module, "Prepare",
                                      &PrepareType, &_WatcherType) ||
            _PyModule_AddIntMacro(module, EV_PREPARE) ||
#endif

#if EV_CHECK_ENABLE
            // Check
            _PyModule_AddTypeWithBase(module, "Check",
                                      &CheckType, &_WatcherType) ||
            _PyModule_AddIntMacro(module, EV_CHECK) ||
#endif

#if EV_EMBED_ENABLE
            // Embed
            _PyModule_AddTypeWithBase(module, "Embed",
                                      &EmbedType, &_WatcherType) ||
            _PyModule_AddIntMacro(module, EV_EMBED) ||
#endif

#if EV_FORK_ENABLE
            // Fork
            _PyModule_AddTypeWithBase(module, "Fork",
                                      &ForkType, &_WatcherType) ||
            _PyModule_AddIntMacro(module, EV_FORK) ||
#endif

#if EV_ASYNC_ENABLE
            // Async
            _PyModule_AddTypeWithBase(module, "Async",
                                      &AsyncType, &_WatcherType) ||
            _PyModule_AddIntMacro(module, EV_ASYNC) ||
#endif

            // additional events
            _PyModule_AddIntMacro(module, EV_CUSTOM) ||
            _PyModule_AddIntMacro(module, EV_ERROR)
           ) {
            Py_CLEAR(module);
        }
        else {
            // setup libev
            ev_set_allocator(_ev_allocator);
            ev_set_syserr_cb(_ev_syserr_cb);
        }
    }
    return module;
}

