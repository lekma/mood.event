#include "event.h"


/* global objects */

PyObject *EventError = NULL;
PyObject *DefaultLoop = NULL;

_Py_Identifier PyId___event_fatal__ = _Py_static_string_init("__event_fatal__");


/* helpers ------------------------------------------------------------------ */

int
_Py_Invoke_Verify(PyObject *callback, const char *alt)
{
    PyObject *exc_type, *exc_value, *exc_traceback;
    PyObject *repr = NULL;

    if (PyErr_Occurred() || PyErr_CheckSignals()) {
        if (PyErr_ExceptionMatches(PyExc_Exception)) {
            if (callback && (callback != Py_None)) {
                PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
                repr = PyObject_Repr(callback);
                _PyErr_ChainExceptions(exc_type, exc_value, exc_traceback);
            }
            _PyErr_FormatFromCause(
                PyExc_SystemError,
                "trying to invoke %V with an error set",
                repr,
                alt ? alt : "a callback"
            );
            Py_XDECREF(repr);
        }
        return -1;
    }
    return 0;
}


static int
_Py_Fatal_Context(PyObject *context)
{
    PyObject *exc_type, *exc_value, *exc_traceback, *tmp = NULL;
    int fatal = 0;

    PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
    fatal = _PyObject_LookupAttrId(context, &PyId___event_fatal__, &tmp);
    if (fatal == 1) {
        fatal = PyObject_IsTrue(tmp);
        Py_DECREF(tmp);
    }
    _PyErr_ChainExceptions(exc_type, exc_value, exc_traceback);
    return fatal;
}


/* -------------------------------------------------------------------------- */

void
ev_loop_stop(ev_loop *loop)
{
    ev_break(loop, EVBREAK_ALL);
}


void
ev_loop_warn(ev_loop *loop, PyObject *context)
{
    Loop *self = ev_userdata(loop);

    if (
        !_Py_Fatal_Context(context) &&
        (self->callback == Py_None) &&
        PyErr_ExceptionMatches(PyExc_Exception)
    ) {
        PyErr_WriteUnraisable(context);
    }
}


/* --------------------------------------------------------------------------
    module
   -------------------------------------------------------------------------- */

static void *
Py_Allocator(void *ptr, long size)
{
    PyGILState_STATE gstate = PyGILState_Ensure();
    void *result = NULL;

    if (size) {
        result = PyMem_Realloc(ptr, size);
    }
    else {
        PyMem_Free(ptr);
    }
    PyGILState_Release(gstate);
    return result;
}


/* -------------------------------------------------------------------------- */

/*
event.loop(
    [
        flags=EVFLAG_AUTO,
        callback=None,
        data=None,
        io_interval=0.0,
        timeout_interval=0.0
    ]
) -> 'default loop'
*/
static PyObject *
event_loop(PyObject *module, PyObject *args, PyObject *kwargs)
{
    if (!DefaultLoop) {
        DefaultLoop = Loop_new(&Loop_Type, args, kwargs, 1);
    }
    else {
        if (
            PyErr_WarnEx(
                PyExc_RuntimeWarning,
                "returning the 'default loop' created earlier, "
                "arguments ignored (if provided).",
                1
            )
        ) {
            return NULL;
        }
        Py_INCREF(DefaultLoop);
    }
    return DefaultLoop;
}


/* event.time() -> float */
static PyObject *
event_time(PyObject *module)
{
    return PyFloat_FromDouble(ev_time());
}


/* event.sleep(interval) */
static PyObject *
event_sleep(PyObject *module, PyObject *args)
{
    static const double day = 86400.0;
    double interval;

    if (!PyArg_ParseTuple(args, "d:sleep", &interval)) {
        return NULL;
    }
    if (
        (interval > day) &&
        PyErr_WarnEx(
            PyExc_RuntimeWarning,
            "'interval' bigger than a day (86400.0), "
            "this is not garanteed to work",
            1
        )
    ) {
        return NULL;
    }
    Py_BEGIN_ALLOW_THREADS
    ev_sleep(interval);
    Py_END_ALLOW_THREADS
    if (PyErr_Occurred() || PyErr_CheckSignals()) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* event.abi_version() -> (int, int) */
static PyObject *
event_abi_version(PyObject *module)
{
    return Py_BuildValue("ii", ev_version_major(), ev_version_minor());
}


/* event.supported_backends() -> int */
static PyObject *
event_supported_backends(PyObject *module)
{
    return PyLong_FromUnsignedLong(ev_supported_backends());
}


/* event.recommended_backends() -> int */
static PyObject *
event_recommended_backends(PyObject *module)
{
    return PyLong_FromUnsignedLong(ev_recommended_backends());
}


/* event.embeddable_backends() -> int */
static PyObject *
event_embeddable_backends(PyObject *module)
{
    return PyLong_FromUnsignedLong(ev_embeddable_backends());
}


/* event.feed_signal(signum) */
#if EV_SIGNAL_ENABLE
static PyObject *
event_feed_signal(PyObject *module, PyObject *args)
{
    int signum;

    if (!PyArg_ParseTuple(args, "i:feed_signal", &signum)) {
        return NULL;
    }
    ev_feed_signal(signum);
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}
#endif


/* event.fatal() */
static PyObject *
event_fatal(PyObject *module, PyObject *obj)
{
    PyObject *result = NULL;

    if (PyCallable_Check(obj)) {
        if (!_PyObject_SetAttrId(obj, &PyId___event_fatal__, Py_True)) {
            result = Py_NewRef(obj);
        }
    }
    else {
        PyErr_Format(
            PyExc_TypeError,
            "expected a callable, got %.200s",
            Py_TYPE(obj)->tp_name
        );
    }
    return result;
}


/* event_def.m_methods */
static PyMethodDef event_m_methods[] = {
    {
        "loop",
        (PyCFunction)event_loop,
        METH_VARARGS | METH_KEYWORDS,
        "loop("
            "["
                "flags=EVFLAG_AUTO, "
                "callback=None, "
                "data=None, "
                "io_interval=0.0, "
                "timeout_interval=0.0"
            "]"
        ") -> 'default loop'"
    },
    {
        "time",
        (PyCFunction)event_time,
        METH_NOARGS,
        "time() -> float"
    },
    {
        "sleep",
        (PyCFunction)event_sleep,
        METH_VARARGS,
        "sleep(interval)"
    },
    {
        "abi_version",
        (PyCFunction)event_abi_version,
        METH_NOARGS,
        "abi_version() -> (int, int)"
    },
    {
        "supported_backends",
        (PyCFunction)event_supported_backends,
        METH_NOARGS,
        "supported_backends() -> int"
    },
    {
        "recommended_backends",
        (PyCFunction)event_recommended_backends,
        METH_NOARGS,
        "recommended_backends() -> int"
    },
    {
        "embeddable_backends",
        (PyCFunction)event_embeddable_backends,
        METH_NOARGS,
        "embeddable_backends() -> int"
    },
#if EV_SIGNAL_ENABLE
    {
        "feed_signal",
        (PyCFunction)event_feed_signal,
        METH_VARARGS,
        "feed_signal(signum)"
    },
#endif
    {
        "fatal",
        (PyCFunction)event_fatal,
        METH_O,
        "fatal(callable) -> callable"
    },
    {NULL}
};


/* event_def.m_traverse */
static int
event_m_traverse(PyObject *module, visitproc visit, void *arg)
{
    Py_VISIT(EventError);
    return 0;
}


/* event_def.m_clear */
static int
event_m_clear(PyObject *module)
{
    Py_CLEAR(EventError);
    return 0;
}


/* event_def.m_free */
static void
event_m_free(PyObject *module)
{
    event_m_clear(module);
}


/* event_def */
static PyModuleDef event_def = {
    PyModuleDef_HEAD_INIT,
    .m_name = "event",
    .m_doc = "mood event module",
    .m_size = -1,
    .m_methods = event_m_methods,
    .m_traverse = (traverseproc)event_m_traverse,
    .m_clear = (inquiry)event_m_clear,
    .m_free = (freefunc)event_m_free,
};


/* module initialization */
static inline int
__module_init__(PyObject *module)
{
    if (
        _PyModule_AddNewException(
            module, "EventError", "mood.event", NULL, NULL, &EventError
        ) ||
        // Loop
        PyModule_AddType(module, &Loop_Type) ||
        _PyModule_AddUnsignedIntMacro(module, EVFLAG_AUTO) ||
        _PyModule_AddUnsignedIntMacro(module, EVFLAG_NOENV) ||
        _PyModule_AddUnsignedIntMacro(module, EVFLAG_FORKCHECK) ||
        _PyModule_AddUnsignedIntMacro(module, EVFLAG_SIGNALFD) ||
        _PyModule_AddUnsignedIntMacro(module, EVFLAG_NOSIGMASK) ||
        _PyModule_AddUnsignedIntMacro(module, EVFLAG_NOTIMERFD) ||
        _PyModule_AddUnsignedIntMacro(module, EVBACKEND_SELECT) ||
        _PyModule_AddUnsignedIntMacro(module, EVBACKEND_POLL) ||
        _PyModule_AddUnsignedIntMacro(module, EVBACKEND_EPOLL) ||
        _PyModule_AddUnsignedIntMacro(module, EVBACKEND_KQUEUE) ||
        _PyModule_AddUnsignedIntMacro(module, EVBACKEND_DEVPOLL) ||
        _PyModule_AddUnsignedIntMacro(module, EVBACKEND_PORT) ||
        _PyModule_AddUnsignedIntMacro(module, EVBACKEND_LINUXAIO) ||
        _PyModule_AddUnsignedIntMacro(module, EVBACKEND_IOURING) ||
        _PyModule_AddUnsignedIntMacro(module, EVBACKEND_ALL) ||
        _PyModule_AddUnsignedIntMacro(module, EVBACKEND_MASK) ||
        _PyModule_AddIntMacro(module, EVRUN_NOWAIT) ||
        _PyModule_AddIntMacro(module, EVRUN_ONCE) ||
        _PyModule_AddIntMacro(module, EVBREAK_ONE) ||
        _PyModule_AddIntMacro(module, EVBREAK_ALL) ||
        // Watcher
        PyType_Ready(&Watcher_Type) ||
        _PyModule_AddIntMacro(module, EV_MINPRI) ||
        _PyModule_AddIntMacro(module, EV_MAXPRI) ||
        // Io
        _PyModule_AddTypeWithBase(module, &Io_Type, &Watcher_Type) ||
        _PyModule_AddIntMacro(module, EV_READ) ||
        _PyModule_AddIntMacro(module, EV_WRITE) ||
        _PyModule_AddIntMacro(module, EV_IO) ||
        // Timer
        _PyModule_AddTypeWithBase(module, &Timer_Type, &Watcher_Type) ||
        _PyModule_AddIntMacro(module, EV_TIMER) ||
#if EV_PERIODIC_ENABLE
        // Periodic
        _PyModule_AddTypeWithBase(module, &Periodic_Type, &Watcher_Type) ||
#if EV_PREPARE_ENABLE
        // Scheduler
        _PyModule_AddTypeWithBase(module, &Scheduler_Type, &Watcher_Type) ||
#endif
        _PyModule_AddIntMacro(module, EV_PERIODIC) ||
#endif
#if EV_SIGNAL_ENABLE
        // Signal
        _PyModule_AddTypeWithBase(module, &Signal_Type, &Watcher_Type) ||
        _PyModule_AddIntMacro(module, EV_SIGNAL) ||
#endif
#if EV_CHILD_ENABLE
        // Child
        _PyModule_AddTypeWithBase(module, &Child_Type, &Watcher_Type) ||
        _PyModule_AddIntMacro(module, EV_CHILD) ||
#endif
#if EV_IDLE_ENABLE
        // Idle
        _PyModule_AddTypeWithBase(module, &Idle_Type, &Watcher_Type) ||
        _PyModule_AddIntMacro(module, EV_IDLE) ||
#endif
#if EV_PREPARE_ENABLE
        // Prepare
        _PyModule_AddTypeWithBase(module, &Prepare_Type, &Watcher_Type) ||
        _PyModule_AddIntMacro(module, EV_PREPARE) ||
#endif
#if EV_CHECK_ENABLE
        // Check
        _PyModule_AddTypeWithBase(module, &Check_Type, &Watcher_Type) ||
        _PyModule_AddIntMacro(module, EV_CHECK) ||
#endif
#if EV_EMBED_ENABLE
        // Embed
        _PyModule_AddTypeWithBase(module, &Embed_Type, &Watcher_Type) ||
        _PyModule_AddIntMacro(module, EV_EMBED) ||
#endif
#if EV_FORK_ENABLE
        // Fork
        _PyModule_AddTypeWithBase(module, &Fork_Type, &Watcher_Type) ||
        _PyModule_AddIntMacro(module, EV_FORK) ||
#endif
#if EV_ASYNC_ENABLE
        // Async
        _PyModule_AddTypeWithBase(module, &Async_Type, &Watcher_Type) ||
        _PyModule_AddIntMacro(module, EV_ASYNC) ||
#endif
        // additional events
        _PyModule_AddIntMacro(module, EV_CUSTOM) ||
        _PyModule_AddIntMacro(module, EV_ERROR) ||
        // version
        PyModule_AddStringConstant(module, "__version__", PKG_VERSION)
    ) {
        Py_CLEAR(EventError);
        return -1;
    }
    // setup libev
    ev_set_allocator(Py_Allocator);
    ev_set_syserr_cb(Py_FatalError);
    return 0;
}

PyMODINIT_FUNC
PyInit_event(void)
{
    PyObject *module = NULL;

    if ((module = PyState_FindModule(&event_def))) {
        Py_INCREF(module);
    }
    else if (
        (module = PyModule_Create(&event_def)) && __module_init__(module)
    ) {
        Py_CLEAR(module);
    }
    return module;
}
