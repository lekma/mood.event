#include "event.h"


/* helpers ------------------------------------------------------------------ */

static void
__ev_loop_invoke__(ev_loop *loop)
{
    PyGILState_STATE gstate = PyGILState_Ensure();
    Loop *self = ev_userdata(loop);

    if (!_Py_Invoke_Verify(self->callback, "loop callback")) {
        if (self->callback != Py_None) {
            Py_XDECREF(_Py_Invoke_Callback(self->callback, self, NULL));
        }
        else {
            ev_invoke_pending(loop);
        }
    }
    if (PyErr_Occurred() || PyErr_CheckSignals()) {
        ev_loop_stop(loop);
    }
    PyGILState_Release(gstate);
}


/* --------------------------------------------------------------------------
   Loop
   -------------------------------------------------------------------------- */

#define __Loop_set_interval__(L, t, i) \
    do { \
        ev_set_##t##_collect_interval((L)->loop, (i)); \
        (L)->t##_ival = (i); \
    } while (0)


static Loop *
__Loop_alloc__(PyTypeObject *type)
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


static int
__Loop_post_alloc__(Loop *self, PyObject *args, PyObject *kwargs, int _default_)
{
    static char *kwlist[] = {
        "flags", "callback", "data", "io_interval", "timeout_interval", NULL
    };
    unsigned int flags = EVFLAG_AUTO;
    PyObject *callback = Py_None, *data = Py_None;
    double io_ival = 0.0, timeout_ival = 0.0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "|IOOdd:__init__",
            kwlist,
            &flags,
            &callback,
            &data,
            &io_ival,
            &timeout_ival
        )
    ) {
        return -1;
    }
    if (
        !(self->loop = _default_ ? ev_default_loop(flags) : ev_loop_new(flags))
    ) {
        PyErr_SetString(EventError, "could not create loop, bad 'flags'?");
        return -1;
    }
    _Py_CHECK_CALLABLE_OR_NONE(callback, -1);
    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(io_ival, -1);
    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(timeout_ival, -1);
    _Py_SET_MEMBER(self->callback, callback);
    _Py_SET_MEMBER(self->data, data);
    __Loop_set_interval__(self, io, io_ival);
    __Loop_set_interval__(self, timeout, timeout_ival);
    ev_set_userdata(self->loop, self);
    ev_set_invoke_pending_cb(self->loop, __ev_loop_invoke__);
    return 0;
}


static void
__Loop_finalize__(Loop *self)
{
    if (self->loop) {
        ev_loop_stop(self->loop);
    }
}


static int
__Loop_traverse__(Loop *self, visitproc visit, void *arg)
{
    Py_VISIT(self->data);
    Py_VISIT(self->callback);
    return 0;
}


static int
__Loop_clear__(Loop *self)
{
    Py_CLEAR(self->data);
    Py_CLEAR(self->callback);
    return 0;
}


static void
__Loop_dealloc__(Loop *self)
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


/* -------------------------------------------------------------------------- */

PyObject *
Loop_new(
    PyTypeObject *type, PyObject *args, PyObject *kwargs, int _default_
)
{
    Loop *self = NULL;

    if ((self = __Loop_alloc__(type))) {
        PyObject_GC_Track(self);
        if (__Loop_post_alloc__(self, args, kwargs, _default_)) {
            Py_CLEAR(self);
        }
    }
    return (PyObject *)self;
}


/* -------------------------------------------------------------------------- */

/* Loop_Type.tp_dealloc */
static void
Loop_tp_dealloc(Loop *self)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)self)) {
        return;
    }
    PyObject_GC_UnTrack(self);
    __Loop_clear__(self);
    __Loop_dealloc__(self);
}


/* Loop_Type.tp_repr */
static PyObject *
Loop_tp_repr(Loop *self)
{
    return PyUnicode_FromFormat(
        "<%s(default=%s, backend=%lu) at %p>",
        Py_TYPE(self)->tp_name,
        ev_is_default_loop(self->loop) ? "True" : "False",
        ev_backend(self->loop),
        self
    );
}


/* Loop_Type.tp_traverse */
static int
Loop_tp_traverse(Loop *self, visitproc visit, void *arg)
{
    return __Loop_traverse__(self, visit, arg);
}


/* Loop_Type.tp_clear */
static int
Loop_tp_clear(Loop *self)
{
    return __Loop_clear__(self);
}


/* Loop_Type.tp_init */
static int
Loop_tp_init(Loop *self, PyObject *args, PyObject *kwargs)
{
    return 0;
}


/* Loop_Type.tp_new */
static PyObject *
Loop_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return Loop_new(type, args, kwargs, 0);
}


/* Loop_Type.tp_finalize */
static void
Loop_tp_finalize(Loop *self)
{
    __Loop_finalize__(self);
}


/* -------------------------------------------------------------------------- */

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
    if (PyErr_Occurred() || PyErr_CheckSignals()) {
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


/* Loop.feed_fd_event(fd, revents) */
static PyObject *
Loop_feed_fd_event(Loop *self, PyObject *args)
{
    PyObject *fd = NULL;
    int revents = EV_NONE, fdnum = -1;

    if (!PyArg_ParseTuple(args, "Oi:feed_fd_event", &fd, &revents)) {
        return NULL;
    }
    if ((fdnum = PyObject_AsFileDescriptor(fd)) < 0) {
        return NULL;
    }
    ev_feed_fd_event(self->loop, fdnum, revents);
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}


#if EV_SIGNAL_ENABLE
/* Loop.feed_signal_event(signum) */
static PyObject *
Loop_feed_signal_event(Loop *self, PyObject *args)
{
    int signum;

    if (!PyArg_ParseTuple(args, "i:feed_signal_event", &signum)) {
        return NULL;
    }
    ev_feed_signal_event(self->loop, signum);
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}
#endif


/* Loop.verify() */
static PyObject *
Loop_verify(Loop *self)
{
    ev_verify(self->loop);
    Py_RETURN_NONE;
}


/* watcher methods ---------------------------------------------------------- */

static PyObject *
__Loop_Watcher__(
    Loop *self, PyTypeObject *type, PyObject *args, PyObject *kwargs
)
{
    PyObject *_args_ = NULL, *result = NULL;
    Py_ssize_t _size_ = PyTuple_GET_SIZE(args), i;

    if ((_args_ = PyTuple_New(_size_ + 1))) {
        PyTuple_SET_ITEM(_args_, 0, Py_NewRef(self));
        for (i = 0; i < _size_;) {
            PyObject *item = PyTuple_GET_ITEM(args, i++);
            PyTuple_SET_ITEM(_args_, i, Py_NewRef(item));
        }
        result = PyObject_Call((PyObject *)type, _args_, kwargs);
        Py_CLEAR(_args_);
    }
    return result;
}


/* Loop.__io__() */
static PyObject *
Loop___io__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Io_Type, args, kwargs);
}


/* Loop.__timer__() */
static PyObject *
Loop___timer__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Timer_Type, args, kwargs);
}


#if EV_PERIODIC_ENABLE
/* Loop.__periodic__() */
static PyObject *
Loop___periodic__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Periodic_Type, args, kwargs);
}
#if EV_PREPARE_ENABLE
/* Loop.__scheduler__() */
static PyObject *
Loop___scheduler__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Scheduler_Type, args, kwargs);
}
#endif
#endif


#if EV_SIGNAL_ENABLE
/* Loop.__signal__() */
static PyObject *
Loop___signal__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Signal_Type, args, kwargs);
}
#endif


#if EV_CHILD_ENABLE
/* Loop.__child__() */
static PyObject *
Loop___child__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Child_Type, args, kwargs);
}
#endif


#if EV_IDLE_ENABLE
/* Loop.__idle__() */
static PyObject *
Loop___idle__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Idle_Type, args, kwargs);
}
#endif


#if EV_PREPARE_ENABLE
/* Loop.__prepare__() */
static PyObject *
Loop___prepare__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Prepare_Type, args, kwargs);
}
#endif


#if EV_CHECK_ENABLE
/* Loop.__check__() */
static PyObject *
Loop___check__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Check_Type, args, kwargs);
}
#endif


#if EV_EMBED_ENABLE
/* Loop.__embed__() */
static PyObject *
Loop___embed__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Embed_Type, args, kwargs);
}
#endif


#if EV_FORK_ENABLE
/* Loop.__fork__() */
static PyObject *
Loop___fork__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Fork_Type, args, kwargs);
}
#endif


#if EV_ASYNC_ENABLE
/* Loop.__async__() */
static PyObject *
Loop___async__(Loop *self, PyObject *args, PyObject *kwargs)
{
    return __Loop_Watcher__(self, &Async_Type, args, kwargs);
}
#endif


/* Loop_Type.tp_methods */
static PyMethodDef Loop_tp_methods[] = {
    {
        "start",
        (PyCFunction)Loop_start,
        METH_VARARGS,
        "start([flags]) -> bool"
    },
    {
        "stop",
        (PyCFunction)Loop_stop,
        METH_VARARGS,
        "stop([how])"
    },
    {
        "invoke",
        (PyCFunction)Loop_invoke,
        METH_NOARGS,
        "invoke()"
    },
    {
        "reset",
        (PyCFunction)Loop_reset,
        METH_NOARGS,
        "reset()"
    },
    {
        "now",
        (PyCFunction)Loop_now,
        METH_VARARGS,
        "now([update]) -> float"
    },
    {
        "suspend",
        (PyCFunction)Loop_suspend,
        METH_NOARGS,
        "suspend()"
    },
    {
        "resume",
        (PyCFunction)Loop_resume,
        METH_NOARGS,
        "resume()"
    },
    {
        "incref",
        (PyCFunction)Loop_incref,
        METH_NOARGS,
        "incref()"
    },
    {
        "decref",
        (PyCFunction)Loop_decref,
        METH_NOARGS,
        "decref()"
    },
    {
        "feed_fd_event",
        (PyCFunction)Loop_feed_fd_event,
        METH_VARARGS,
        "feed_fd_event(fd, revents)"
    },
#if EV_SIGNAL_ENABLE
    {
        "feed_signal_event",
        (PyCFunction)Loop_feed_signal_event,
        METH_VARARGS,
        "feed_signal_event(signum)"
    },
#endif
    {
        "verify",
        (PyCFunction)Loop_verify,
        METH_NOARGS,
        "verify()"
    },
    /* watcher methods */
    {
        "__io__",
        (PyCFunction)Loop___io__,
        METH_VARARGS | METH_KEYWORDS,
        "__io__(fd, events, callback[, data=None, priority=0]) -> Io"
    },
    {
        "__timer__",
        (PyCFunction)Loop___timer__,
        METH_VARARGS | METH_KEYWORDS,
        "__timer__(after, repeat, callback[, data=None, priority=0]) -> Timer"
    },
#if EV_PERIODIC_ENABLE
    {
        "__periodic__",
        (PyCFunction)Loop___periodic__,
        METH_VARARGS | METH_KEYWORDS,
        "__periodic__(offset, interval, callback[, data=None, priority=0]) -> Periodic"
    },
#if EV_PREPARE_ENABLE
    {
        "__scheduler__",
        (PyCFunction)Loop___scheduler__,
        METH_VARARGS | METH_KEYWORDS,
        "__scheduler__(reschedule, callback[, data=None, priority=0]) -> Scheduler"
    },
#endif
#endif
#if EV_SIGNAL_ENABLE
    {
        "__signal__",
        (PyCFunction)Loop___signal__,
        METH_VARARGS | METH_KEYWORDS,
        "__signal__(signum, callback[, data=None, priority=0]) -> Signal"
    },
#endif
#if EV_CHILD_ENABLE
    {
        "__child__",
        (PyCFunction)Loop___child__,
        METH_VARARGS | METH_KEYWORDS,
        "__child__(pid, trace, callback[, data=None, priority=0]) -> Child"
    },
#endif
#if EV_IDLE_ENABLE
    {
        "__idle__",
        (PyCFunction)Loop___idle__,
        METH_VARARGS | METH_KEYWORDS,
        "__idle__([callback=None, data=None, priority=0]) -> Idle"
    },
#endif
#if EV_PREPARE_ENABLE
    {
        "__prepare__",
        (PyCFunction)Loop___prepare__,
        METH_VARARGS | METH_KEYWORDS,
        "__prepare__(callback[, data=None, priority=0]) -> Prepare"
    },
#endif
#if EV_CHECK_ENABLE
    {
        "__check__",
        (PyCFunction)Loop___check__,
        METH_VARARGS | METH_KEYWORDS,
        "__check__(callback[, data=None, priority=0]) -> Check"
    },
#endif
#if EV_EMBED_ENABLE
    {
        "__embed__",
        (PyCFunction)Loop___embed__,
        METH_VARARGS | METH_KEYWORDS,
        "__embed__(other[, callback=None, data=None, priority=0]) -> Embed"
    },
#endif
#if EV_FORK_ENABLE
    {
        "__fork__",
        (PyCFunction)Loop___fork__,
        METH_VARARGS | METH_KEYWORDS,
        "__fork__(callback[, data=None, priority=0]) -> Fork"
    },
#endif
#if EV_ASYNC_ENABLE
    {
        "__async__",
        (PyCFunction)Loop___async__,
        METH_VARARGS | METH_KEYWORDS,
        "__async__(callback[, data=None, priority=0]) -> Async"
    },
#endif
    {NULL}
};


/* -------------------------------------------------------------------------- */

/* Loop.callback */
static PyObject *
Loop_callback_getter(Loop *self, void *closure)
{
    return Py_NewRef(self->callback);
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
    return Py_NewRef(self->data);
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
        __Loop_set_interval__(self, io, ival);
    }
    else {
        __Loop_set_interval__(self, timeout, ival);
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
        "callback",
        (getter)Loop_callback_getter,
        (setter)Loop_callback_setter,
        NULL,
        NULL
    },
    {
        "data",
        (getter)Loop_data_getter,
        (setter)Loop_data_setter,
        NULL,
        NULL
    },
    {
        "io_interval",
        (getter)Loop_interval_getter,
        (setter)Loop_interval_setter,
        NULL,
        Py_True
    },
    {
        "timeout_interval",
        (getter)Loop_interval_getter,
        (setter)Loop_interval_setter,
        NULL,
        NULL
    },
    {
        "default",
        (getter)Loop_default_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {
        "iteration",
        (getter)Loop_iteration_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {
        "depth",
        (getter)Loop_depth_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {
        "backend",
        (getter)Loop_backend_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {
        "pending",
        (getter)Loop_pending_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

PyTypeObject Loop_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Loop",
    .tp_basicsize = sizeof(Loop),
    .tp_dealloc = (destructor)Loop_tp_dealloc,
    .tp_repr = (reprfunc)Loop_tp_repr,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_FINALIZE,
    .tp_doc = "Loop([flags=EVFLAG_AUTO, callback=None, data=None, io_interval=0.0, timeout_interval=0.0])",
    .tp_traverse = (traverseproc)Loop_tp_traverse,
    .tp_clear = (inquiry)Loop_tp_clear,
    .tp_methods = Loop_tp_methods,
    .tp_getset = Loop_tp_getsets,
    .tp_init = (initproc)Loop_tp_init,
    .tp_new = (newfunc)Loop_tp_new,
    .tp_finalize = (destructor)Loop_tp_finalize,
};
