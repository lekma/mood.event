/* ----------------------------------------------------------------------------
 helpers
 ---------------------------------------------------------------------------- */

#define _Loop_FullStop(l) ev_break((l), EVBREAK_ALL)

#define _Loop_Exit(l) \
    do { \
        _Loop_FullStop((l)); \
        return; \
    } while (0)


/* report errors or bail out if needed */
static void
_Loop_WarnOrStop(struct ev_loop *loop, PyObject *context)
{
    _Py_IDENTIFIER(__err_fatal__);
    Loop *self = ev_userdata(loop);

    if (_PyObject_HasAttrId(context, &PyId___err_fatal__) ||
        self->callback != Py_None ||
        !PyErr_ExceptionMatches(PyExc_Exception)) {
        _Loop_Exit(loop);
    }
    else {
        PyErr_WriteUnraisable(context);
    }
}


/* loop setup */
static void
_Loop_InvokePending(struct ev_loop *loop)
{
    Loop *self = ev_userdata(loop);
    PyObject *result = NULL;

    if (!(result = PyObject_CallFunctionObjArgs(self->callback, self, NULL))) {
        _Loop_Exit(loop);
    }
    Py_DECREF(result);
}

#define _Loop_SetCallback(L, cb) \
    do { \
        if ((cb) != Py_None) { \
            ev_set_invoke_pending_cb((L)->loop, _Loop_InvokePending); \
        } \
        else { \
            ev_set_invoke_pending_cb((L)->loop, ev_invoke_pending); \
        } \
        _Py_SET_MEMBER((L)->callback, (cb)); \
    } while (0)


#define _Loop_SetInterval(L, ival, t) \
    do { \
        ev_set_##t##_collect_interval((L)->loop, (ival)); \
        (L)->t##_ival = (ival); \
    } while (0)


static void
_Loop_Release(struct ev_loop *loop)
{
    ((Loop *)ev_userdata(loop))->tstate = PyEval_SaveThread();
}

static void
_Loop_Acquire(struct ev_loop *loop)
{
    PyEval_RestoreThread(((Loop *)ev_userdata(loop))->tstate);
}


static int
_Loop_Setup(Loop *self, PyObject *callback, PyObject *data,
            double io_ival, double timeout_ival)
{
    _Py_CHECK_CALLABLE_OR_NONE(callback, -1);
    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(io_ival, -1);
    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(timeout_ival, -1);

    _Loop_SetCallback(self, callback);
    _Py_SET_MEMBER(self->data, data);
    _Loop_SetInterval(self, io_ival, io);
    _Loop_SetInterval(self, timeout_ival, timeout);
    ev_set_loop_release_cb (self->loop, _Loop_Release, _Loop_Acquire);
    ev_set_userdata(self->loop, self);

    return 0;
}


/* loop instantiation */
#define _Loop_Alloc() __PyObject_Alloc(Loop, &LoopType)

static struct ev_loop *
_Py_ev_loop_new(unsigned int flags, int default_loop)
{
    struct ev_loop *loop = NULL;

    if (!(loop = default_loop ? ev_default_loop(flags) : ev_loop_new(flags))) {
        PyErr_SetString(Error, "could not create loop, bad 'flags'?");
    }
    return loop;
}


static PyObject *
_Loop_New(PyObject *args, PyObject *kwargs, int default_loop)
{
    static char *kwlist[] = {"flags", "callback", "data",
                             "io_interval", "timeout_interval", NULL};
    unsigned int flags = EVFLAG_AUTO;
    PyObject *callback = Py_None, *data = Py_None;
    double io_ival = 0.0, timeout_ival = 0.0;
    Loop *self = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|IOOddp:__new__", kwlist,
                                     &flags, &callback, &data,
                                     &io_ival, &timeout_ival)) {
        return NULL;
    }
    if ((self = _Loop_Alloc())) {
        PyObject_GC_Track(self);
        if (!(self->loop = _Py_ev_loop_new(flags, default_loop)) ||
            _Loop_Setup(self, callback, data, io_ival, timeout_ival)) {
            Py_CLEAR(self);
        }
    }
    return (PyObject *)self;
}


#if (EV_IDLE_ENABLE || EV_PREPARE_ENABLE || EV_CHECK_ENABLE || EV_FORK_ENABLE || EV_ASYNC_ENABLE)
static PyObject *
_Loop_Watcher(Loop *self, PyObject *args, const char *name, PyTypeObject *type)
{
    PyObject *callback, *data = Py_None, *priority = NULL;

    if (!PyArg_UnpackTuple(args, name, 1, 3,
                           &callback, &data, &priority)) {
        return NULL;
    }
    return PyObject_CallFunctionObjArgs((PyObject *)type,
                                        self, callback, data, priority, NULL);
}
#endif


/* ----------------------------------------------------------------------------
 LoopType
 ---------------------------------------------------------------------------- */

/* LoopType.tp_doc */
PyDoc_STRVAR(Loop_tp_doc,
"Loop([flags=EVFLAG_AUTO, callback=None, data=None,\n\
       io_interval=0.0, timeout_interval=0.0])");


/* LoopType.tp_finalize */
static void
Loop_tp_finalize(Loop *self)
{
    if (self->loop) {
        _Loop_FullStop(self->loop);
    }
}


/* LoopType.tp_traverse */
static int
Loop_tp_traverse(Loop *self, visitproc visit, void *arg)
{
    Py_VISIT(self->data);
    Py_VISIT(self->callback);
    return 0;
}


/* LoopType.tp_clear */
static int
Loop_tp_clear(Loop *self)
{
    Py_CLEAR(self->data);
    Py_CLEAR(self->callback);
    return 0;
}


/* LoopType.tp_dealloc */
static void
Loop_tp_dealloc(Loop *self)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)self)) {
        return;
    }
    PyObject_GC_UnTrack(self);
    Loop_tp_clear(self);
    if (self->loop) {
        if (ev_is_default_loop(self->loop)) {
            DefaultLoop = NULL;
        }
        ev_loop_destroy(self->loop);
        self->loop = NULL;
    }
    PyObject_GC_Del(self);
}


/* Loop.start([flags]) -> bool */
PyDoc_STRVAR(Loop_start_doc,
"start([flags]) -> bool");

static PyObject *
Loop_start(Loop *self, PyObject *args)
{
    int flags = 0, result;

    if (!PyArg_ParseTuple(args, "|i:start", &flags)) {
        return NULL;
    }
    result = ev_run(self->loop, flags);
    if (PyErr_Occurred()) {
        return NULL;
    }
    return PyBool_FromLong(result);
}


/* Loop.stop([how]) */
PyDoc_STRVAR(Loop_stop_doc,
"stop([how])");

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
PyDoc_STRVAR(Loop_invoke_doc,
"invoke()");

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
PyDoc_STRVAR(Loop_reset_doc,
"reset()");

static PyObject *
Loop_reset(Loop *self)
{
    ev_loop_fork(self->loop);
    Py_RETURN_NONE;
}


/* Loop.now() -> float */
PyDoc_STRVAR(Loop_now_doc,
"now() -> float");

static PyObject *
Loop_now(Loop *self)
{
    return PyFloat_FromDouble(ev_now(self->loop));
}


/* Loop.update() */
PyDoc_STRVAR(Loop_update_doc,
"update()");

static PyObject *
Loop_update(Loop *self)
{
    ev_now_update(self->loop);
    Py_RETURN_NONE;
}


/* Loop.suspend()/Loop.resume() */
PyDoc_STRVAR(Loop_suspend_resume_doc,
"suspend()/resume()");

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


/* Loop.unref()/Loop.ref() */
PyDoc_STRVAR(Loop_ref_unref_doc,
"unref()/ref()");

static PyObject *
Loop_unref(Loop *self)
{
    ev_unref(self->loop);
    Py_RETURN_NONE;
}

static PyObject *
Loop_ref(Loop *self)
{
    ev_ref(self->loop);
    Py_RETURN_NONE;
}


/* Loop.verify() */
PyDoc_STRVAR(Loop_verify_doc,
"verify()");

static PyObject *
Loop_verify(Loop *self)
{
    ev_verify(self->loop);
    Py_RETURN_NONE;
}


/* watcher methods */

/* Loop.io(fd, events, callback[, data, priority]) -> Io */
PyDoc_STRVAR(Loop_io_doc,
"io(fd, events, callback[, data, priority]) -> Io");

static PyObject *
Loop_io(Loop *self, PyObject *args)
{
    PyObject *fd, *events;
    PyObject *callback, *data = Py_None, *priority = NULL;

    if (!PyArg_UnpackTuple(args, "io", 3, 5, &fd, &events,
                           &callback, &data, &priority)) {
        return NULL;
    }
    return PyObject_CallFunctionObjArgs((PyObject *)&IoType, fd, events,
                                        self, callback, data, priority, NULL);
}


/* Loop.timer(after, repeat, callback[, data, priority]) -> Timer */
PyDoc_STRVAR(Loop_timer_doc,
"timer(after, repeat, callback[, data, priority]) -> Timer");

static PyObject *
Loop_timer(Loop *self, PyObject *args)
{
    PyObject *after, *repeat;
    PyObject *callback, *data = Py_None, *priority = NULL;

    if (!PyArg_UnpackTuple(args, "timer", 3, 5, &after, &repeat,
                           &callback, &data, &priority)) {
        return NULL;
    }
    return PyObject_CallFunctionObjArgs((PyObject *)&TimerType, after, repeat,
                                        self, callback, data, priority, NULL);
}


#if EV_PERIODIC_ENABLE
/* Loop.periodic(offset, interval, callback[, data, priority]) -> Periodic */
PyDoc_STRVAR(Loop_periodic_doc,
"periodic(offset, interval, callback[, data, priority]) -> Periodic");

static PyObject *
Loop_periodic(Loop *self, PyObject *args)
{
    PyObject *offset, *interval;
    PyObject *callback, *data = Py_None, *priority = NULL;

    if (!PyArg_UnpackTuple(args, "periodic", 3, 5, &offset, &interval,
                           &callback, &data, &priority)) {
        return NULL;
    }
    return PyObject_CallFunctionObjArgs((PyObject *)&PeriodicType, offset, interval,
                                        self, callback, data, priority, NULL);
}
#if EV_PREPARE_ENABLE
/* Loop.scheduler(scheduler, callback[, data, priority]) -> Scheduler */
PyDoc_STRVAR(Loop_scheduler_doc,
"scheduler(scheduler, callback[, data, priority]) -> Scheduler");

static PyObject *
Loop_scheduler(Loop *self, PyObject *args)
{
    PyObject *scheduler;
    PyObject *callback, *data = Py_None, *priority = NULL;

    if (!PyArg_UnpackTuple(args, "scheduler", 2, 4, &scheduler,
                           &callback, &data, &priority)) {
        return NULL;
    }
    return PyObject_CallFunctionObjArgs((PyObject *)&SchedulerType, scheduler,
                                        self, callback, data, priority, NULL);
}
#endif
#endif


#if EV_SIGNAL_ENABLE
/* Loop.signal(signum, callback[, data, priority]) -> Signal */
PyDoc_STRVAR(Loop_signal_doc,
"signal(signum, callback[, data, priority]) -> Signal");

static PyObject *
Loop_signal(Loop *self, PyObject *args)
{
    PyObject *signum;
    PyObject *callback, *data = Py_None, *priority = NULL;

    if (!PyArg_UnpackTuple(args, "signal", 2, 4, &signum,
                           &callback, &data, &priority)) {
        return NULL;
    }
    return PyObject_CallFunctionObjArgs((PyObject *)&SignalType, signum,
                                        self, callback, data, priority, NULL);
}
#endif


#if EV_CHILD_ENABLE
/* Loop.child(pid, trace, callback[, data, priority]) -> Child */
PyDoc_STRVAR(Loop_child_doc,
"child(pid, trace, callback[, data, priority]) -> Child");

static PyObject *
Loop_child(Loop *self, PyObject *args)
{
    PyObject *pid, *trace;
    PyObject *callback, *data = Py_None, *priority = NULL;

    if (!PyArg_UnpackTuple(args, "child", 3, 5, &pid, &trace,
                           &callback, &data, &priority)) {
        return NULL;
    }
    return PyObject_CallFunctionObjArgs((PyObject *)&ChildType, pid, trace,
                                        self, callback, data, priority, NULL);
}
#endif


#if EV_IDLE_ENABLE
/* Loop.idle(callback[, data, priority]) -> Idle */
PyDoc_STRVAR(Loop_idle_doc,
"idle(callback[, data, priority]) -> Idle");

static PyObject *
Loop_idle(Loop *self, PyObject *args)
{
    return _Loop_Watcher(self, args, "idle", &IdleType);
}
#endif


#if EV_PREPARE_ENABLE
/* Loop.prepare(callback[, data, priority]) -> Prepare */
PyDoc_STRVAR(Loop_prepare_doc,
"prepare(callback[, data, priority]) -> Prepare");

static PyObject *
Loop_prepare(Loop *self, PyObject *args)
{
    return _Loop_Watcher(self, args, "prepare", &PrepareType);
}
#endif


#if EV_CHECK_ENABLE
/* Loop.check(callback[, data, priority]) -> Check */
PyDoc_STRVAR(Loop_check_doc,
"check(callback[, data, priority]) -> Check");

static PyObject *
Loop_check(Loop *self, PyObject *args)
{
    return _Loop_Watcher(self, args, "check", &CheckType);
}
#endif


#if EV_EMBED_ENABLE
/* Loop.embed(other[, callback, data, priority]) -> Embed */
PyDoc_STRVAR(Loop_embed_doc,
"embed(other[, callback, data, priority]) -> Embed");

static PyObject *
Loop_embed(Loop *self, PyObject *args)
{
    PyObject *other;
    PyObject *callback = Py_None, *data = Py_None, *priority = NULL;

    if (!PyArg_UnpackTuple(args, "embed", 1, 4, &other,
                           &callback, &data, &priority)) {
        return NULL;
    }
    return PyObject_CallFunctionObjArgs((PyObject *)&EmbedType, other,
                                        self, callback, data, priority, NULL);
}
#endif


#if EV_FORK_ENABLE
/* Loop.fork(callback[, data, priority]) -> Fork */
PyDoc_STRVAR(Loop_fork_doc,
"fork(callback[, data, priority]) -> Fork");

static PyObject *
Loop_fork(Loop *self, PyObject *args)
{
    return _Loop_Watcher(self, args, "fork", &ForkType);
}
#endif


#if EV_ASYNC_ENABLE
/* Loop.async(callback[, data, priority]) -> Async */
PyDoc_STRVAR(Loop_async_doc,
"async(callback[, data, priority]) -> Async");

static PyObject *
Loop_async(Loop *self, PyObject *args)
{
    return _Loop_Watcher(self, args, "async", &AsyncType);
}
#endif


/* LoopType.tp_methods */
static PyMethodDef Loop_tp_methods[] = {
    {"start", (PyCFunction)Loop_start,
     METH_VARARGS, Loop_start_doc},
    {"stop", (PyCFunction)Loop_stop,
     METH_VARARGS, Loop_stop_doc},
    {"invoke", (PyCFunction)Loop_invoke,
     METH_NOARGS, Loop_invoke_doc},
    {"reset", (PyCFunction)Loop_reset,
     METH_NOARGS, Loop_reset_doc},
    {"now", (PyCFunction)Loop_now,
     METH_NOARGS, Loop_now_doc},
    {"update", (PyCFunction)Loop_update,
     METH_NOARGS, Loop_update_doc},
    {"suspend", (PyCFunction)Loop_suspend,
     METH_NOARGS, Loop_suspend_resume_doc},
    {"resume", (PyCFunction)Loop_resume,
     METH_NOARGS, Loop_suspend_resume_doc},
    {"unref", (PyCFunction)Loop_unref,
     METH_NOARGS, Loop_ref_unref_doc},
    {"ref", (PyCFunction)Loop_ref,
     METH_NOARGS, Loop_ref_unref_doc},
    {"verify", (PyCFunction)Loop_verify,
     METH_NOARGS, Loop_verify_doc},
    /* watcher methods */
    {"io", (PyCFunction)Loop_io,
     METH_VARARGS, Loop_io_doc},
    {"timer", (PyCFunction)Loop_timer,
     METH_VARARGS, Loop_timer_doc},
#if EV_PERIODIC_ENABLE
    {"periodic", (PyCFunction)Loop_periodic,
     METH_VARARGS, Loop_periodic_doc},
#if EV_PREPARE_ENABLE
    {"scheduler", (PyCFunction)Loop_scheduler,
     METH_VARARGS, Loop_scheduler_doc},
#endif
#endif
#if EV_SIGNAL_ENABLE
    {"signal", (PyCFunction)Loop_signal,
     METH_VARARGS, Loop_signal_doc},
#endif
#if EV_CHILD_ENABLE
    {"child", (PyCFunction)Loop_child,
     METH_VARARGS, Loop_child_doc},
#endif
#if EV_IDLE_ENABLE
    {"idle", (PyCFunction)Loop_idle,
     METH_VARARGS, Loop_idle_doc},
#endif
#if EV_PREPARE_ENABLE
    {"prepare", (PyCFunction)Loop_prepare,
     METH_VARARGS, Loop_prepare_doc},
#endif
#if EV_CHECK_ENABLE
    {"check", (PyCFunction)Loop_check,
     METH_VARARGS, Loop_check_doc},
#endif
#if EV_EMBED_ENABLE
    {"embed", (PyCFunction)Loop_embed,
     METH_VARARGS, Loop_embed_doc},
#endif
#if EV_FORK_ENABLE
    {"fork", (PyCFunction)Loop_fork,
     METH_VARARGS, Loop_fork_doc},
#endif
#if EV_ASYNC_ENABLE
    {"async", (PyCFunction)Loop_async,
     METH_VARARGS, Loop_async_doc},
#endif
    {NULL}  /* Sentinel */
};


/* Loop.callback */
static PyObject *
Loop_callback_get(Loop *self, void *closure)
{
    _Py_RETURN_OBJECT(self->callback);
}

static int
Loop_callback_set(Loop *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    _Py_CHECK_CALLABLE_OR_NONE(value, -1);
    _Loop_SetCallback(self, value);
    return 0;
}


/* Loop.data */
static PyObject *
Loop_data_get(Loop *self, void *closure)
{
    _Py_RETURN_OBJECT(self->data);
}

static int
Loop_data_set(Loop *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    _Py_SET_MEMBER(self->data, value);
    return 0;
}


/* Loop.io_interval/Loop.timeout_interval */
static PyObject *
Loop_interval_get(Loop *self, void *closure)
{
    return PyFloat_FromDouble(closure ? self->io_ival : self->timeout_ival);
}

static int
Loop_interval_set(Loop *self, PyObject *value, void *closure)
{
    double ival = -1.0;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    if ((ival = PyFloat_AsDouble(value)) == -1.0 && PyErr_Occurred()) {
        return -1;
    }
    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(ival, -1);
    if (closure) {
        _Loop_SetInterval(self, ival, io);
    }
    else {
        _Loop_SetInterval(self, ival, timeout);
    }
    return 0;
}


/* Loop.default */
static PyObject *
Loop_default_get(Loop *self, void *closure)
{
    return PyBool_FromLong(ev_is_default_loop(self->loop));
}


/* Loop.backend */
static PyObject *
Loop_backend_get(Loop *self, void *closure)
{
    return PyLong_FromUnsignedLong(ev_backend(self->loop));
}


/* Loop.pending */
static PyObject *
Loop_pending_get(Loop *self, void *closure)
{
    return PyLong_FromUnsignedLong(ev_pending_count(self->loop));
}


/* Loop.iteration */
static PyObject *
Loop_iteration_get(Loop *self, void *closure)
{
    return PyLong_FromUnsignedLong(ev_iteration(self->loop));
}


/* Loop.depth */
static PyObject *
Loop_depth_get(Loop *self, void *closure)
{
    return PyLong_FromUnsignedLong(ev_depth(self->loop));
}


/* LoopType.tp_getsets */
static PyGetSetDef Loop_tp_getsets[] = {
    {"callback", (getter)Loop_callback_get,
     (setter)Loop_callback_set, NULL, NULL},
    {"data", (getter)Loop_data_get,
     (setter)Loop_data_set, NULL, NULL},
    {"io_interval", (getter)Loop_interval_get,
     (setter)Loop_interval_set, NULL, (void *)1},
    {"timeout_interval", (getter)Loop_interval_get,
     (setter)Loop_interval_set, NULL, NULL},
    {"default", (getter)Loop_default_get,
     _Readonly_attribute_set, NULL, NULL},
    {"backend", (getter)Loop_backend_get,
     _Readonly_attribute_set, NULL, NULL},
    {"pending", (getter)Loop_pending_get,
     _Readonly_attribute_set, NULL, NULL},
    {"iteration", (getter)Loop_iteration_get,
     _Readonly_attribute_set, NULL, NULL},
    {"depth", (getter)Loop_depth_get,
     _Readonly_attribute_set, NULL, NULL},
    {NULL}  /* Sentinel */
};


/* LoopType.tp_new */
static PyObject *
Loop_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return _Loop_New(args, kwargs, 0);
}


/* LoopType */
static PyTypeObject LoopType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mood.event.Loop",                        /*tp_name*/
    sizeof(Loop),                             /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)Loop_tp_dealloc,              /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_HAVE_FINALIZE, /*tp_flags*/
    Loop_tp_doc,                              /*tp_doc*/
    (traverseproc)Loop_tp_traverse,           /*tp_traverse*/
    (inquiry)Loop_tp_clear,                   /*tp_clear*/
    0,                                        /*tp_richcompare*/
    0,                                        /*tp_weaklistoffset*/
    0,                                        /*tp_iter*/
    0,                                        /*tp_iternext*/
    Loop_tp_methods,                          /*tp_methods*/
    0,                                        /*tp_members*/
    Loop_tp_getsets,                          /*tp_getsets*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    0,                                        /*tp_init*/
    0,                                        /*tp_alloc*/
    Loop_tp_new,                              /*tp_new*/
    0,                                        /*tp_free*/
    0,                                        /*tp_is_gc*/
    0,                                        /*tp_bases*/
    0,                                        /*tp_mro*/
    0,                                        /*tp_cache*/
    0,                                        /*tp_subclasses*/
    0,                                        /*tp_weaklist*/
    0,                                        /*tp_del*/
    0,                                        /*tp_version_tag*/
    (destructor)Loop_tp_finalize,             /*tp_finalize*/
};
