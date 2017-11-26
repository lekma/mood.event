#include "watcher.h"


#if EV_PERIODIC_ENABLE


/* helpers ------------------------------------------------------------------ */

static int
__ev_periodic_check_args__(double offset, double interval)
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

static int
__Periodic_set__(Watcher *self, double offset, double interval)
{
    if (__ev_periodic_check_args__(offset, interval)) {
        return -1;
    }
    ev_periodic_set(((ev_periodic *)self->watcher), offset, interval, 0);
    return 0;
}


/* -------------------------------------------------------------------------- */

/* Periodic_Type.tp_init */
static int
Periodic_tp_init(Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "loop",
        "offset", "interval",
        "callback", "data", "priority", NULL
    };

    Loop *loop = NULL;
    double offset = 0.0, interval = 0.0;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args, kwargs, "O!ddO|Oi:__init__", kwlist,
            &Loop_Type, &loop,
            &offset, &interval,
            &callback, &data, &priority
        ) ||
        Watcher_init(self, loop, callback, data, priority)
    ) {
        return -1;
    }
    return __Periodic_set__(self, offset, interval);
}


/* Periodic_Type.tp_new */
static PyObject *
Periodic_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return Watcher_new(type, EV_PERIODIC, sizeof(ev_periodic));
}


/* -------------------------------------------------------------------------- */

/* Periodic.set(offset, interval) */
static PyObject *
Periodic_set(Watcher *self, PyObject *args)
{
    double offset = 0.0, interval = 0.0;

    if (
        Watcher_check_set(self) ||
        !PyArg_ParseTuple(args, "dd:set", &offset, &interval) ||
        __Periodic_set__(self, offset, interval)
    ) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Periodic.reset() */
static PyObject *
Periodic_reset(Watcher *self)
{
    ev_periodic_again(self->loop->loop, ((ev_periodic *)self->watcher));
    Py_RETURN_NONE;
}


/* Periodic_Type.tp_methods */
static PyMethodDef Periodic_tp_methods[] = {
    {
        "set",
        (PyCFunction)Periodic_set,
        METH_VARARGS,
        "set(offset, interval)"
    },
    {
        "reset",
        (PyCFunction)Periodic_reset,
        METH_NOARGS,
        "reset()"
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

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
    if (
        (((offset = PyFloat_AsDouble(value)) == -1.0) && PyErr_Occurred()) ||
        __ev_periodic_check_args__(
            offset, ((ev_periodic *)self->watcher)->interval
        )
    ) {
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
    if (
        (((interval = PyFloat_AsDouble(value)) == -1.0) && PyErr_Occurred()) ||
        __ev_periodic_check_args__(
            ((ev_periodic *)self->watcher)->offset, interval
        )
    ) {
        return -1;
    }
    ((ev_periodic *)self->watcher)->interval = interval;
    return 0;
}


/* Periodic.at */
static PyObject *
Periodic_at_getter(Watcher *self, void *closure)
{
    return PyFloat_FromDouble(ev_periodic_at(((ev_periodic *)self->watcher)));
}


/* Periodic_Type.tp_getsets */
static PyGetSetDef Periodic_tp_getsets[] = {
    {
        "offset",
        (getter)Periodic_offset_getter,
        (setter)Periodic_offset_setter,
        NULL,
        NULL
    },
    {
        "interval",
        (getter)Periodic_interval_getter,
        (setter)Periodic_interval_setter,
        NULL,
        NULL
    },
    {
        "at",
        (getter)Periodic_at_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

PyTypeObject Periodic_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Periodic",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Periodic(loop, offset, interval, callback[, data=None, priority=0])",
    .tp_methods = Periodic_tp_methods,
    .tp_getset = Periodic_tp_getsets,
    .tp_init = (initproc)Periodic_tp_init,
    .tp_new = (newfunc)Periodic_tp_new,
};


/* ========================================================================== */

#if EV_PREPARE_ENABLE


/* helpers ------------------------------------------------------------------ */

static void
__ev_scheduler_stop__(ev_loop *loop, ev_prepare *prepare, int revents)
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
        ev_loop_warn(loop, self->reschedule);
    }
    // reset err_ members because we might get restarted
    // (with another reschedule callback for example)
    self->err_type = NULL;
    self->err_value = NULL;
    self->err_traceback = NULL;
    self->err_fatal = 0;
    if (PyErr_Occurred()) {
        ev_loop_stop(loop);
    }
}


static double
__ev_scheduler_invoke__(ev_periodic *periodic, double now)
{
    PyGILState_STATE gstate = PyGILState_Ensure();
    Scheduler *self = periodic->data;
    PyObject *_now_ = NULL, *_result_ = NULL;
    double result = -1.0;

    if (
        _Py_Invoke_Verify(self->reschedule, "reschedule callback") ||
        !(_now_ = PyFloat_FromDouble(now))
    ) {
        self->err_fatal = 1;
        goto fail;
    }
    _result_ = _Py_Invoke_Callback(self->reschedule, self, _now_, NULL);
    Py_DECREF(_now_);
    if (!_result_) {
        goto fail;
    }
    result = PyFloat_AsDouble(_result_);
    Py_DECREF(_result_);
    if ((result == -1.0) && PyErr_Occurred()) {
        goto fail;
    }
    if (result < now) {
        PyErr_Format(
            EventError,
            "%R must return a value >= to the 'now' argument",
            self->reschedule
        );
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

static Scheduler *
__Scheduler_alloc__(PyTypeObject *type)
{
    Scheduler *self = NULL;

    if ((self = (Scheduler *)__Watcher_alloc__(type))) {
        self->prepare = NULL;
        self->reschedule = NULL;
        self->err_type = NULL;
        self->err_value = NULL;
        self->err_traceback = NULL;
        self->err_fatal = 0;
    }
    return self;
}


static int
__Scheduler_post_alloc__(Scheduler *self, int ev_type, size_t size)
{
    Watcher *watcher = (Watcher *)self;

    if (__Watcher_post_alloc__(watcher, ev_type, size)) {
        return -1;
    }
    if (!(self->prepare = PyMem_Malloc(sizeof(ev_prepare)))) {
        PyErr_NoMemory();
        return -1;
    }
    self->prepare->data = self;
    ev_prepare_init(self->prepare, __ev_scheduler_stop__);
    ev_set_priority(self->prepare, EV_MAXPRI);
    ev_periodic_set(
        ((ev_periodic *)watcher->watcher), .0, .0, __ev_scheduler_invoke__
    );
    return 0;
}


static void
__Scheduler_finalize__(Scheduler *self)
{
    Watcher *watcher = (Watcher *)self;

    if (self->prepare && watcher->loop && watcher->loop->loop) {
        ev_prepare_stop(watcher->loop->loop, self->prepare);
    }
    __Watcher_finalize__(watcher);
}


static int
__Scheduler_traverse__(Scheduler *self, visitproc visit, void *arg)
{
    Py_VISIT(self->err_traceback);
    Py_VISIT(self->err_value);
    Py_VISIT(self->err_type);
    Py_VISIT(self->reschedule);
    return __Watcher_traverse__((Watcher *)self, visit, arg);
}


static int
__Scheduler_clear__(Scheduler *self)
{
    Py_CLEAR(self->err_traceback);
    Py_CLEAR(self->err_value);
    Py_CLEAR(self->err_type);
    Py_CLEAR(self->reschedule);
    return __Watcher_clear__((Watcher *)self);
}


static void
__Scheduler_dealloc__(Scheduler *self)
{
    if (self->prepare) {
        PyMem_Free(self->prepare);
        self->prepare = NULL;
    }
    __Watcher_dealloc__((Watcher *)self);
}


/* -------------------------------------------------------------------------- */

static int
__Scheduler_set__(Scheduler *self, PyObject *reschedule)
{
    _Py_CHECK_CALLABLE(reschedule, -1);
    _Py_SET_MEMBER(self->reschedule, reschedule);
    return 0;
}


/* -------------------------------------------------------------------------- */

/* Scheduler_Type.tp_dealloc */
static void
Scheduler_tp_dealloc(Scheduler *self)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)self)) {
        return;
    }
    PyObject_GC_UnTrack(self);
    __Scheduler_clear__(self);
    __Scheduler_dealloc__(self);
}


/* Scheduler_Type.tp_traverse */
static int
Scheduler_tp_traverse(Scheduler *self, visitproc visit, void *arg)
{
    return __Scheduler_traverse__(self, visit, arg);
}


/* Scheduler_Type.tp_clear */
static int
Scheduler_tp_clear(Scheduler *self)
{
    return __Scheduler_clear__(self);
}


/* Scheduler_Type.tp_init */
static int
Scheduler_tp_init(Scheduler *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {
        "loop",
        "reschedule",
        "callback", "data", "priority", NULL
    };

    Loop *loop = NULL;
    PyObject *reschedule = NULL;
    PyObject *callback = NULL, *data = Py_None;
    int priority = 0;

    if (
        !PyArg_ParseTupleAndKeywords(
            args, kwargs, "O!OO|Oi:__init__", kwlist,
            &Loop_Type, &loop,
            &reschedule,
            &callback, &data, &priority
        ) ||
        Watcher_init((Watcher *)self, loop, callback, data, priority)
    ) {
        return -1;
    }
    return __Scheduler_set__(self, reschedule);
}


/* Scheduler_Type.tp_new */
static PyObject *
Scheduler_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    Scheduler *self = NULL;

    if ((self = __Scheduler_alloc__(type))) {
        PyObject_GC_Track(self);
        if (__Scheduler_post_alloc__(self, EV_PERIODIC, sizeof(ev_periodic))) {
            Py_CLEAR(self);
        }
    }
    return (PyObject *)self;
}


/* Scheduler_Type.tp_finalize */
static void
Scheduler_tp_finalize(Scheduler *self)
{
    __Scheduler_finalize__(self);
}


/* -------------------------------------------------------------------------- */

/* Scheduler_Type.tp_methods */
static PyMethodDef Scheduler_tp_methods[] = {
    {
        "reset",
        (PyCFunction)Periodic_reset,
        METH_NOARGS,
        "reset()"
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

/* Scheduler.reschedule */
static PyObject *
Scheduler_reschedule_getter(Scheduler *self, void *closure)
{
    return Py_NewRef(self->reschedule);
}

static int
Scheduler_reschedule_setter(Scheduler *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    return __Scheduler_set__(self, value);
}


/* Scheduler_Type.tp_getsets */
static PyGetSetDef Scheduler_tp_getsets[] = {
    {
        "reschedule",
        (getter)Scheduler_reschedule_getter,
        (setter)Scheduler_reschedule_setter,
        NULL,
        NULL
    },
    {
        "at",
        (getter)Periodic_at_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

PyTypeObject Scheduler_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Scheduler",
    .tp_basicsize = sizeof(Scheduler),
    .tp_dealloc = (destructor)Scheduler_tp_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_FINALIZE,
    .tp_doc = "Scheduler(loop, reschedule, callback[, data=None, priority=0])",
    .tp_traverse = (traverseproc)Scheduler_tp_traverse,
    .tp_clear = (inquiry)Scheduler_tp_clear,
    .tp_methods = Scheduler_tp_methods,
    .tp_getset = Scheduler_tp_getsets,
    .tp_init = (initproc)Scheduler_tp_init,
    .tp_new = (newfunc)Scheduler_tp_new,
    .tp_finalize = (destructor)Scheduler_tp_finalize,
};


#endif // !EV_PREPARE_ENABLE


#endif // !EV_PERIODIC_ENABLE
