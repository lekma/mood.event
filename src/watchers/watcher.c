#include "watcher.h"


/* helpers ------------------------------------------------------------------ */

#define __ev_watcher_call__(m, t, l, w) t##_##m((l), (t *)(w))
#define __ev_watcher_call_start__(t, l, w) __ev_watcher_call__(start, t, l, w)
#define __ev_watcher_call_stop__(t, l, w) __ev_watcher_call__(stop, t, l, w)


static void
__ev_watcher_start__(ev_loop *loop, ev_watcher *watcher, int ev_type)
{
    switch (ev_type) {
        case EV_IO:
            __ev_watcher_call_start__(ev_io, loop, watcher);
            break;
        case EV_TIMER:
            __ev_watcher_call_start__(ev_timer, loop, watcher);
            break;
#if EV_PERIODIC_ENABLE
        case EV_PERIODIC:
            __ev_watcher_call_start__(ev_periodic, loop, watcher);
            break;
#endif
#if EV_SIGNAL_ENABLE
        case EV_SIGNAL:
            __ev_watcher_call_start__(ev_signal, loop, watcher);
            break;
#endif
#if EV_CHILD_ENABLE
        case EV_CHILD:
            __ev_watcher_call_start__(ev_child, loop, watcher);
            break;
#endif
#if EV_IDLE_ENABLE
        case EV_IDLE:
            __ev_watcher_call_start__(ev_idle, loop, watcher);
            break;
#endif
#if EV_PREPARE_ENABLE
        case EV_PREPARE:
            __ev_watcher_call_start__(ev_prepare, loop, watcher);
            break;
#endif
#if EV_CHECK_ENABLE
        case EV_CHECK:
            __ev_watcher_call_start__(ev_check, loop, watcher);
            break;
#endif
#if EV_EMBED_ENABLE
        case EV_EMBED:
            __ev_watcher_call_start__(ev_embed, loop, watcher);
            break;
#endif
#if EV_FORK_ENABLE
        case EV_FORK:
            __ev_watcher_call_start__(ev_fork, loop, watcher);
            break;
#endif
#if EV_ASYNC_ENABLE
        case EV_ASYNC:
            __ev_watcher_call_start__(ev_async, loop, watcher);
            break;
#endif
        default:
            Py_FatalError("unknown watcher type");
            break;
    }
}


static void
__ev_watcher_stop__(ev_loop *loop, ev_watcher *watcher, int ev_type)
{
    switch (ev_type) {
        case EV_IO:
            __ev_watcher_call_stop__(ev_io, loop, watcher);
            break;
        case EV_TIMER:
            __ev_watcher_call_stop__(ev_timer, loop, watcher);
            break;
#if EV_PERIODIC_ENABLE
        case EV_PERIODIC:
            __ev_watcher_call_stop__(ev_periodic, loop, watcher);
            break;
#endif
#if EV_SIGNAL_ENABLE
        case EV_SIGNAL:
            __ev_watcher_call_stop__(ev_signal, loop, watcher);
            break;
#endif
#if EV_CHILD_ENABLE
        case EV_CHILD:
            __ev_watcher_call_stop__(ev_child, loop, watcher);
            break;
#endif
#if EV_IDLE_ENABLE
        case EV_IDLE:
            __ev_watcher_call_stop__(ev_idle, loop, watcher);
            break;
#endif
#if EV_PREPARE_ENABLE
        case EV_PREPARE:
            __ev_watcher_call_stop__(ev_prepare, loop, watcher);
            break;
#endif
#if EV_CHECK_ENABLE
        case EV_CHECK:
            __ev_watcher_call_stop__(ev_check, loop, watcher);
            break;
#endif
#if EV_EMBED_ENABLE
        case EV_EMBED:
            __ev_watcher_call_stop__(ev_embed, loop, watcher);
            break;
#endif
#if EV_FORK_ENABLE
        case EV_FORK:
            __ev_watcher_call_stop__(ev_fork, loop, watcher);
            break;
#endif
#if EV_ASYNC_ENABLE
        case EV_ASYNC:
            __ev_watcher_call_stop__(ev_async, loop, watcher);
            break;
#endif
        default:
            Py_FatalError("unknown watcher type");
            break;
    }
}


/* -------------------------------------------------------------------------- */

static void
__Watcher_warn__(Watcher *self)
{
    PyObject *exc_type, *exc_value, *exc_traceback;

    PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
    PyErr_WarnFormat(PyExc_RuntimeWarning, 1, "%R has been stopped", self);
    _PyErr_ChainExceptions(exc_type, exc_value, exc_traceback);
}


static void
__ev_watcher_invoke__(ev_loop *loop, ev_watcher *watcher, int revents)
{
    Watcher *self = watcher->data;
    PyObject *_revents_ = NULL, *_result_ = NULL;

    if (revents & EV_ERROR) {
        if (!PyErr_Occurred()) {
            if (errno) { // there's a high probability it is related
                _PyErr_SetFromErrno();
            }
            else {
                PyErr_SetString(EventError, "unspecified libev error");
            }
        }
        // warn that we have been stopped
        __Watcher_warn__(self);
    }
    else if (!_Py_Invoke_Verify(self->callback, "watcher callback")) {
        if (self->callback != Py_None) {
            if ((_revents_ = PyLong_FromLong(revents))) {
                _result_ = _Py_Invoke_Callback(
                    self->callback, self, _revents_, NULL
                );
                if (_result_) {
                    Py_DECREF(_result_);
                }
                else {
                    ev_loop_warn(loop, self->callback);
                }
                Py_DECREF(_revents_);
            }
        }
#if EV_EMBED_ENABLE
        else if (revents & EV_EMBED) {
            ev_embed_sweep(loop, (ev_embed *)watcher);
        }
#endif
    }
    if (PyErr_Occurred() || PyErr_CheckSignals()) {
        ev_loop_stop(loop);
    }
}


/* --------------------------------------------------------------------------
   Watcher
   -------------------------------------------------------------------------- */

#define __Watcher_check_state__(s, W, a, r) \
    do { \
        if (ev_is_##s(((W)->watcher))) { \
            PyErr_Format(EventError, "cannot %s watcher while it is " #s, (a)); \
            return (r); \
        } \
    } while (0)

#define __Watcher_check_active__(W, a, r) \
    __Watcher_check_state__(active, W, a, r)

#define __Watcher_check_pending__(W, a, r) \
    __Watcher_check_state__(pending, W, a, r)


/* -------------------------------------------------------------------------- */

#define __Watcher_check_states__(W, a, r) \
    do { \
        __Watcher_check_active__(W, a, r); \
        __Watcher_check_pending__(W, a, r); \
    } while (0)


#define __Watcher_check_callback__(W, cb, r) \
    do { \
        switch(((W)->ev_type)) { \
            case EV_IDLE: \
            case EV_EMBED: \
                _Py_CHECK_CALLABLE_OR_NONE((cb), (r)); \
                break; \
            default: \
                _Py_CHECK_CALLABLE((cb), (r)); \
                break; \
        } \
    } while (0)


/* -------------------------------------------------------------------------- */

Watcher *
__Watcher_alloc__(PyTypeObject *type)
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


int
__Watcher_post_alloc__(Watcher *self, int ev_type, size_t size)
{
    if (!(self->watcher = PyMem_Malloc(size))) {
        PyErr_NoMemory();
        return -1;
    }
    self->ev_type = ev_type;
    self->watcher->data = self;
    ev_init(self->watcher, __ev_watcher_invoke__);
    return 0;
}


void
__Watcher_finalize__(Watcher *self)
{
    if (self->watcher && self->loop && self->loop->loop) {
        __ev_watcher_stop__(self->loop->loop, self->watcher, self->ev_type);
    }
}


int
__Watcher_traverse__(Watcher *self, visitproc visit, void *arg)
{
    Py_VISIT(self->data);
    Py_VISIT(self->callback);
    Py_VISIT(self->loop);
    return 0;
}


int
__Watcher_clear__(Watcher *self)
{
    Py_CLEAR(self->data);
    Py_CLEAR(self->callback);
    Py_CLEAR(self->loop);
    return 0;
}


void
__Watcher_dealloc__(Watcher *self)
{
    if (self->watcher) {
        PyMem_Free(self->watcher);
        self->watcher = NULL;
    }
    PyObject_GC_Del(self);
}


/* -------------------------------------------------------------------------- */

int
Watcher_check_active(Watcher *self, const char *action)
{
    __Watcher_check_active__(self, action, -1);
    return 0;
}


int
Watcher_check_set(Watcher *self)
{
    return Watcher_check_active(self, "set a");
}


/* -------------------------------------------------------------------------- */

PyObject *
Watcher_new(PyTypeObject *type, int ev_type, size_t size)
{
    Watcher *self = NULL;

    if ((self = __Watcher_alloc__(type))) {
        PyObject_GC_Track(self);
        if (__Watcher_post_alloc__(self, ev_type, size)) {
            Py_CLEAR(self);
        }
    }
    return (PyObject *)self;
}


int
Watcher_init(
    Watcher *self, Loop *loop, PyObject *callback, PyObject *data, int priority
)
{
    __Watcher_check_states__(self, "init a", -1);
    __Watcher_check_callback__(self, callback, -1);
    _Py_SET_MEMBER(self->loop, loop);
    _Py_SET_MEMBER(self->callback, callback);
    _Py_SET_MEMBER(self->data, data);
    ev_set_priority(self->watcher, priority);
    return 0;
}


/* -------------------------------------------------------------------------- */

/* Watcher_Type.tp_dealloc */
static void
Watcher_tp_dealloc(Watcher *self)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)self)) {
        return;
    }
    PyObject_GC_UnTrack(self);
    __Watcher_clear__(self);
    __Watcher_dealloc__(self);
}


/* Watcher_Type.tp_traverse */
static int
Watcher_tp_traverse(Watcher *self, visitproc visit, void *arg)
{
    return __Watcher_traverse__(self, visit, arg);
}


/* Watcher_Type.tp_clear */
static int
Watcher_tp_clear(Watcher *self)
{
    return __Watcher_clear__(self);
}


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
            args,
            kwargs,
            "O!O|Oi:__init__",
            kwlist,
            &Loop_Type, &loop,
            &callback,
            &data,
            &priority
        )
    ) {
        return -1;
    }
    return Watcher_init(self, loop, callback, data, priority);
}


/* Watcher_Type.tp_finalize */
static void
Watcher_tp_finalize(Watcher *self)
{
    __Watcher_finalize__(self);
}


/* -------------------------------------------------------------------------- */

/* Watcher.start() */
static PyObject *
Watcher_start(Watcher *self)
{
    __ev_watcher_start__(self->loop->loop, self->watcher, self->ev_type);
    Py_RETURN_NONE;
}


/* Watcher.stop() */
static PyObject *
Watcher_stop(Watcher *self)
{
    __ev_watcher_stop__(self->loop->loop, self->watcher, self->ev_type);
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
        "start",
        (PyCFunction)Watcher_start,
        METH_NOARGS,
        "start()"
    },
    {
        "stop",
        (PyCFunction)Watcher_stop,
        METH_NOARGS,
        "stop()"
    },
    {
        "invoke",
        (PyCFunction)Watcher_invoke,
        METH_VARARGS,
        "invoke(revents)"
    },
    {
        "feed",
        (PyCFunction)Watcher_feed,
        METH_VARARGS,
        "feed(revents)"
    },
    {
        "clear",
        (PyCFunction)Watcher_clear,
        METH_NOARGS,
        "clear() -> int"
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

/* Watcher.loop */
static PyObject *
Watcher_loop_getter(Watcher *self, void *closure)
{
    return Py_NewRef(self->loop);
}


/* Watcher.callback */
static PyObject *
Watcher_callback_getter(Watcher *self, void *closure)
{
    return Py_NewRef(self->callback);
}

static int
Watcher_callback_setter(Watcher *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    __Watcher_check_callback__(self, value, -1);
    _Py_SET_MEMBER(self->callback, value);
    return 0;
}


/* Watcher.data */
static PyObject *
Watcher_data_getter(Watcher *self, void *closure)
{
    return Py_NewRef(self->data);
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
    __Watcher_check_states__(self, "set the 'priority' of a", -1);
    if (((priority = _PyLong_AsInt(value)) == -1) && PyErr_Occurred()) {
        return -1;
    }
    ev_set_priority(self->watcher, priority);
    return 0;
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
        "loop",
        (getter)Watcher_loop_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {
        "callback",
        (getter)Watcher_callback_getter,
        (setter)Watcher_callback_setter,
        NULL,
        NULL
    },
    {
        "data",
        (getter)Watcher_data_getter,
        (setter)Watcher_data_setter,
        NULL,
        NULL
    },
    {
        "priority",
        (getter)Watcher_priority_getter,
        (setter)Watcher_priority_setter,
        NULL,
        NULL
    },
    {
        "active",
        (getter)Watcher_active_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {
        "pending",
        (getter)Watcher_pending_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

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
