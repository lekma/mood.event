/* ----------------------------------------------------------------------------
 helpers
 ---------------------------------------------------------------------------- */

#define __Watcher_CheckState(state, W, m, r) \
    do { \
        if (ev_is_##state((W)->watcher)) { \
            PyErr_Format(Error, "cannot %s a watcher while it is " #state, (m)); \
            return (r); \
        } \
    } while (0)

#define __Watcher_CheckActive(W, m, r) __Watcher_CheckState(active, W, m, r)
#define __Watcher_CheckPending(W, m, r) __Watcher_CheckState(pending, W, m, r)
#define __Watcher_Set(W) __Watcher_CheckActive(W, "set", NULL)


#define __Watcher_Call(t, m, W) t##_##m((W)->loop->loop, (t *)(W)->watcher)
#define __Watcher_CallStart(t, W) __Watcher_Call(t, start, W)
#define __Watcher_CallStop(t, W) __Watcher_Call(t, stop, W)

static void
__Watcher_Start(_Watcher *self)
{
    switch (self->ev_type) {
        case EV_IO:
            __Watcher_CallStart(ev_io, self);
            break;
        case EV_TIMER:
            __Watcher_CallStart(ev_timer, self);
            break;
#if EV_PERIODIC_ENABLE
        case EV_PERIODIC:
            __Watcher_CallStart(ev_periodic, self);
            break;
#endif
#if EV_SIGNAL_ENABLE
        case EV_SIGNAL:
            __Watcher_CallStart(ev_signal, self);
            break;
#endif
#if EV_CHILD_ENABLE
        case EV_CHILD:
            __Watcher_CallStart(ev_child, self);
            break;
#endif
#if EV_IDLE_ENABLE
        case EV_IDLE:
            __Watcher_CallStart(ev_idle, self);
            break;
#endif
#if EV_PREPARE_ENABLE
        case EV_PREPARE:
            __Watcher_CallStart(ev_prepare, self);
            break;
#endif
#if EV_CHECK_ENABLE
        case EV_CHECK:
            __Watcher_CallStart(ev_check, self);
            break;
#endif
#if EV_EMBED_ENABLE
        case EV_EMBED:
            __Watcher_CallStart(ev_embed, self);
            break;
#endif
#if EV_FORK_ENABLE
        case EV_FORK:
            __Watcher_CallStart(ev_fork, self);
            break;
#endif
#if EV_ASYNC_ENABLE
        case EV_ASYNC:
            __Watcher_CallStart(ev_async, self);
            break;
#endif
        default:
            Py_FatalError("unknown watcher type");
            break;
    }
}

static void
__Watcher_Stop(_Watcher *self)
{
    switch (self->ev_type) {
        case EV_IO:
            __Watcher_CallStop(ev_io, self);
            break;
        case EV_TIMER:
            __Watcher_CallStop(ev_timer, self);
            break;
#if EV_PERIODIC_ENABLE
        case EV_PERIODIC:
            __Watcher_CallStop(ev_periodic, self);
            break;
#endif
#if EV_SIGNAL_ENABLE
        case EV_SIGNAL:
            __Watcher_CallStop(ev_signal, self);
            break;
#endif
#if EV_CHILD_ENABLE
        case EV_CHILD:
            __Watcher_CallStop(ev_child, self);
            break;
#endif
#if EV_IDLE_ENABLE
        case EV_IDLE:
            __Watcher_CallStop(ev_idle, self);
            break;
#endif
#if EV_PREPARE_ENABLE
        case EV_PREPARE:
            __Watcher_CallStop(ev_prepare, self);
            break;
#endif
#if EV_CHECK_ENABLE
        case EV_CHECK:
            __Watcher_CallStop(ev_check, self);
            break;
#endif
#if EV_EMBED_ENABLE
        case EV_EMBED:
            __Watcher_CallStop(ev_embed, self);
            break;
#endif
#if EV_FORK_ENABLE
        case EV_FORK:
            __Watcher_CallStop(ev_fork, self);
            break;
#endif
#if EV_ASYNC_ENABLE
        case EV_ASYNC:
            __Watcher_CallStop(ev_async, self);
            break;
#endif
        default:
            Py_FatalError("unknown watcher type");
            break;
    }
}


#define __Watcher_WarnStopped(W, l) \
    do { \
        if (PyErr_WarnFormat(PyExc_RuntimeWarning, 1, \
                             "%R has been stopped", (W))) { \
            _Loop_Exit((l)); \
        } \
    } while (0)

/* watcher callback */
static void
__Watcher_Callback(struct ev_loop *loop, ev_watcher *watcher, int revents)
{
    PyObject *err_type, *err_value, *err_traceback;
    PyObject *pyrevents = NULL, *pyresult = NULL;
    _Watcher *self = watcher->data;

    if (revents & EV_ERROR) {
        PyErr_Fetch(&err_type, &err_value, &err_traceback);
        // warn that we have been stopped
        __Watcher_WarnStopped(self, loop);
        PyErr_Restore(err_type, err_value, err_traceback);
        if (!PyErr_Occurred()) {
            if (errno) { // there's a high probability it is related
                PyErr_SetFromErrno(PyExc_OSError);
            }
            else {
                PyErr_SetString(Error, "unspecified libev error");
            }
        }
        _Loop_Exit(loop);
    }
    else if (self->callback != Py_None) {
        if ((pyrevents = PyLong_FromLong(revents))) {
            pyresult = PyObject_CallFunctionObjArgs(self->callback, self,
                                                    pyrevents, NULL);
            Py_DECREF(pyrevents);
            if (pyresult) {
                Py_DECREF(pyresult);
            }
            else {
                _Loop_WarnOrStop(loop, self->callback);
            }
        }
        else {
            _Loop_Exit(loop);
        }
    }
#if EV_EMBED_ENABLE
    else if (revents & EV_EMBED) {
        ev_embed_sweep(loop, (ev_embed *)watcher);
    }
#endif
}


/* watcher instantiation */
#define __Watcher_Alloc(type) __PyObject_Alloc(_Watcher, type)

static PyObject *
__Watcher_New(PyTypeObject *type, int ev_type, size_t size)
{
    _Watcher *self = NULL;

    if ((self = __Watcher_Alloc(type))) {
        PyObject_GC_Track(self);
        if (!(self->watcher = PyObject_Malloc(size))) {
            PyErr_NoMemory();
            Py_CLEAR(self);
        }
        else {
            self->ev_type = ev_type;
            ev_init(self->watcher, __Watcher_Callback);
            self->watcher->data = self;
        }
    }
    return (PyObject *)self;
}


/* watcher initialization */
#define __Watcher_CheckAllStates(W, m) \
    do { \
        __Watcher_CheckActive(W, m, -1); \
        __Watcher_CheckPending(W, m, -1); \
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

static int
__Watcher_Init(_Watcher *self,
               Loop *loop, PyObject *callback, PyObject *data, int priority)
{
    __Watcher_CheckAllStates(self, "init");
    __Watcher_CheckCallback(self, callback, -1);
    _Py_SET_MEMBER(self->loop, loop);
    _Py_SET_MEMBER(self->callback, callback);
    _Py_SET_MEMBER(self->data, data);
    ev_set_priority(self->watcher, priority);
    return 0;
}


static void
__Watcher_Free(_Watcher *self)
{
    if (self->watcher) {
        PyObject_Free(self->watcher);
        self->watcher = NULL;
    }
    PyObject_GC_Del(self);
}


/* ----------------------------------------------------------------------------
 _WatcherType
 ---------------------------------------------------------------------------- */

/* _WatcherType.tp_finalize */
static void
_Watcher_tp_finalize(_Watcher *self)
{
    if (self->watcher && self->loop) {
        __Watcher_Stop(self);
    }
}


/* _WatcherType.tp_traverse */
static int
_Watcher_tp_traverse(_Watcher *self, visitproc visit, void *arg)
{
    Py_VISIT(self->data);
    Py_VISIT(self->callback);
    Py_VISIT(self->loop);
    return 0;
}


/* _WatcherType.tp_clear */
static int
_Watcher_tp_clear(_Watcher *self)
{
    Py_CLEAR(self->data);
    Py_CLEAR(self->callback);
    Py_CLEAR(self->loop);
    return 0;
}


/* _WatcherType.tp_dealloc */
static void
_Watcher_tp_dealloc(_Watcher *self)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)self)) {
        return;
    }
    PyObject_GC_UnTrack(self);
    _Watcher_tp_clear(self);
    __Watcher_Free(self);
}


/* _Watcher.start() */
PyDoc_STRVAR(_Watcher_start_doc,
"start()");

static PyObject *
_Watcher_start(_Watcher *self)
{
    __Watcher_Start(self);
    Py_RETURN_NONE;
}


/* _Watcher.stop() */
PyDoc_STRVAR(_Watcher_stop_doc,
"stop()");

static PyObject *
_Watcher_stop(_Watcher *self)
{
    __Watcher_Stop(self);
    Py_RETURN_NONE;
}


/* _Watcher.invoke(revents) */
PyDoc_STRVAR(_Watcher_invoke_doc,
"invoke(revents)");

static PyObject *
_Watcher_invoke(_Watcher *self, PyObject *args)
{
    int revents;

    if (!PyArg_ParseTuple(args, "i:invoke", &revents)) {
        return NULL;
    }
    ev_invoke(self->loop->loop, self->watcher, revents);
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* _Watcher.clear() -> int */
PyDoc_STRVAR(_Watcher_clear_doc,
"clear() -> int");

static PyObject *
_Watcher_clear(_Watcher *self)
{
    return PyLong_FromLong(ev_clear_pending(self->loop->loop, self->watcher));
}


/* _Watcher.feed(revents) */
PyDoc_STRVAR(_Watcher_feed_doc,
"feed(revents)");

static PyObject *
_Watcher_feed(_Watcher *self, PyObject *args)
{
    int revents;

    if (!PyArg_ParseTuple(args, "i:feed", &revents)) {
        return NULL;
    }
    ev_feed_event(self->loop->loop, self->watcher, revents);
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* _WatcherType.tp_methods */
static PyMethodDef _Watcher_tp_methods[] = {
    {"start", (PyCFunction)_Watcher_start,
     METH_NOARGS, _Watcher_start_doc},
    {"stop", (PyCFunction)_Watcher_stop,
     METH_NOARGS, _Watcher_stop_doc},
    {"invoke", (PyCFunction)_Watcher_invoke,
     METH_VARARGS, _Watcher_invoke_doc},
    {"clear", (PyCFunction)_Watcher_clear,
     METH_NOARGS, _Watcher_clear_doc},
    {"feed", (PyCFunction)_Watcher_feed,
     METH_VARARGS, _Watcher_feed_doc},
    {NULL}  /* Sentinel */
};


/* _Watcher.loop */
static PyObject *
_Watcher_loop_get(_Watcher *self, void *closure)
{
    _Py_RETURN_OBJECT(self->loop);
}


/* _Watcher.callback */
static PyObject *
_Watcher_callback_get(_Watcher *self, void *closure)
{
    _Py_RETURN_OBJECT(self->callback);
}

static int
_Watcher_callback_set(_Watcher *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    __Watcher_CheckCallback(self, value, -1);
    _Py_SET_MEMBER(self->callback, value);
    return 0;
}


/* _Watcher.data */
static PyObject *
_Watcher_data_get(_Watcher *self, void *closure)
{
    _Py_RETURN_OBJECT(self->data);
}

static int
_Watcher_data_set(_Watcher *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    _Py_SET_MEMBER(self->data, value);
    return 0;
}


/* _Watcher.priority */
static PyObject *
_Watcher_priority_get(_Watcher *self, void *closure)
{
    return PyLong_FromLong(ev_priority(self->watcher));
}

static int
_Watcher_priority_set(_Watcher *self, PyObject *value, void *closure)
{
    long priority = -1;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    __Watcher_CheckAllStates(self, "set the priority of");
    priority = PyLong_AsLong(value);
    _Py_CHECK_INT_ATTRIBUTE(priority, -1);
    ev_set_priority(self->watcher, priority);
    return 0;
}


/* _Watcher.active */
static PyObject *
_Watcher_active_get(_Watcher *self, void *closure)
{
    return PyBool_FromLong(ev_is_active(self->watcher));
}


/* _Watcher.pending */
static PyObject *
_Watcher_pending_get(_Watcher *self, void *closure)
{
    return PyBool_FromLong(ev_is_pending(self->watcher));
}


/* _WatcherType.tp_getsets */
static PyGetSetDef _Watcher_tp_getsets[] = {
    {"loop", (getter)_Watcher_loop_get,
     _Readonly_attribute_set, NULL, NULL},
    {"callback", (getter)_Watcher_callback_get,
     (setter)_Watcher_callback_set, NULL, NULL},
    {"data", (getter)_Watcher_data_get,
     (setter)_Watcher_data_set, NULL, NULL},
    {"priority", (getter)_Watcher_priority_get,
     (setter)_Watcher_priority_set, NULL, NULL},
    {"active", (getter)_Watcher_active_get,
     _Readonly_attribute_set, NULL, NULL},
    {"pending", (getter)_Watcher_pending_get,
     _Readonly_attribute_set, NULL, NULL},
    {NULL}  /* Sentinel */
};


/* _WatcherType.tp_init */
static int
_Watcher_tp_init(_Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"loop", "callback", "data", "priority", NULL};
    Loop *loop;
    PyObject *callback, *data = Py_None;
    int priority = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O|Oi:__init__", kwlist,
            &LoopType, &loop, &callback, &data, &priority)) {
        return -1;
    }
    return __Watcher_Init(self, loop, callback, data, priority);
}


/* _WatcherType */
static PyTypeObject _WatcherType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mood.event._Watcher",                    /*tp_name*/
    sizeof(_Watcher),                         /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)_Watcher_tp_dealloc,          /*tp_dealloc*/
    0,                                        /*tp_print*/
    0,                                        /*tp_getattr*/
    0,                                        /*tp_setattr*/
    0,                                        /*tp_compare*/
    0,                                        /*tp_repr*/
    0,                                        /*tp_as_number*/
    0,                                        /*tp_as_sequence*/
    0,                                        /*tp_as_mapping*/
    0,                                        /*tp_hash */
    0,                                        /*tp_call*/
    0,                                        /*tp_str*/
    0,                                        /*tp_getattro*/
    0,                                        /*tp_setattro*/
    0,                                        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_FINALIZE, /*tp_flags*/
    0,                                        /*tp_doc*/
    (traverseproc)_Watcher_tp_traverse,       /*tp_traverse*/
    (inquiry)_Watcher_tp_clear,               /*tp_clear*/
    0,                                        /*tp_richcompare*/
    0,                                        /*tp_weaklistoffset*/
    0,                                        /*tp_iter*/
    0,                                        /*tp_iternext*/
    _Watcher_tp_methods,                      /*tp_methods*/
    0,                                        /*tp_members*/
    _Watcher_tp_getsets,                      /*tp_getsets*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)_Watcher_tp_init,               /*tp_init*/
    0,                                        /*tp_alloc*/
    0,                                        /*tp_new*/
    0,                                        /*tp_free*/
    0,                                        /*tp_is_gc*/
    0,                                        /*tp_bases*/
    0,                                        /*tp_mro*/
    0,                                        /*tp_cache*/
    0,                                        /*tp_subclasses*/
    0,                                        /*tp_weaklist*/
    0,                                        /*tp_del*/
    0,                                        /*tp_version_tag*/
    (destructor)_Watcher_tp_finalize,         /*tp_finalize*/
};
