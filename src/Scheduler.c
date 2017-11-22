/* ----------------------------------------------------------------------------
 helpers
 ---------------------------------------------------------------------------- */

static void
_Scheduler_Stop(struct ev_loop *loop, ev_prepare *prepare, int revents)
{
    Scheduler *self = prepare->data;
    int fatal;

    // stop the Scheduler watcher
    ev_periodic_stop(loop, (ev_periodic *)((_Watcher *)self)->watcher);
    // stop this watcher (prepare)
    ev_prepare_stop(loop, prepare);
    // warn that we have been stopped
    __Watcher_WarnStopped(self, loop);
    // set the exception back to the original one
    PyErr_Restore(self->err_type, self->err_value, self->err_traceback);
    // reset err_ members because we might get restarted
    // (with another scheduler for example)
    self->err_traceback = NULL;
    self->err_value = NULL;
    self->err_type = NULL;
    fatal = self->err_fatal;
    self->err_fatal = 0;
    if (fatal) {
        _Loop_Exit(loop);
    }
    else {
        _Loop_WarnOrStop(loop, self->scheduler);
    }
}


static double
_Scheduler_Schedule(ev_periodic *periodic, double now)
{
    Scheduler *self = periodic->data;
    PyObject *pynow = NULL, *pyresult = NULL;
    double result = -1.0;

    pynow = PyFloat_FromDouble(now);
    if (!pynow) {
        self->err_fatal = 1;
        goto fail;
    }
    pyresult = PyObject_CallFunctionObjArgs(self->scheduler, self, pynow, NULL);
    Py_DECREF(pynow);
    if (!pyresult) {
        goto fail;
    }
    result = PyFloat_AsDouble(pyresult);
    Py_DECREF(pyresult);
    if (result == -1.0 && PyErr_Occurred()) {
        goto fail;
    }
    if (result < now) {
        PyErr_SetString(Error, "returned value must be >= 'now' param");
        goto fail;
    }
    goto end;

fail:
    PyErr_Fetch(&self->err_type, &self->err_value, &self->err_traceback);
    ev_prepare_start(((_Watcher *)self)->loop->loop, self->prepare);
    result = now + 1e30;

end:
    return result;
}


/* set the Scheduler */
static int
_Scheduler_Set(Scheduler *self, PyObject *scheduler)
{
    _Py_CHECK_CALLABLE(scheduler, -1);
    _Py_SET_MEMBER(self->scheduler, scheduler);
    return 0;
}


/* ----------------------------------------------------------------------------
 SchedulerType
 ---------------------------------------------------------------------------- */

/* SchedulerType.tp_doc */
PyDoc_STRVAR(Scheduler_tp_doc,
"Scheduler(scheduler, loop[, callback=None, data=None, priority=0])");


/* SchedulerType.tp_finalize */
static void
Scheduler_tp_finalize(Scheduler *self)
{
    _Watcher *_watcher = (_Watcher *)self;

    if (self->prepare && _watcher->loop->loop) {
        ev_prepare_stop(_watcher->loop->loop, self->prepare);
    }
    _Watcher_tp_finalize(_watcher);
}


/* SchedulerType.tp_traverse */
static int
Scheduler_tp_traverse(Scheduler *self, visitproc visit, void *arg)
{
    Py_VISIT(self->err_traceback);
    Py_VISIT(self->err_value);
    Py_VISIT(self->err_type);
    Py_VISIT(self->scheduler);
    return _Watcher_tp_traverse((_Watcher *)self, visit, arg);
}


/* SchedulerType.tp_clear */
static int
Scheduler_tp_clear(Scheduler *self)
{
    Py_CLEAR(self->err_traceback);
    Py_CLEAR(self->err_value);
    Py_CLEAR(self->err_type);
    Py_CLEAR(self->scheduler);
    return _Watcher_tp_clear((_Watcher *)self);
}


/* SchedulerType.tp_dealloc */
static void
Scheduler_tp_dealloc(Scheduler *self)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)self)) {
        return;
    }
    PyObject_GC_UnTrack(self);
    Scheduler_tp_clear(self);
    if (self->prepare) {
        PyObject_Free(self->prepare);
        self->prepare = NULL;
    }
    __Watcher_Free((_Watcher *)self);
}


/* SchedulerType.tp_methods */
static PyMethodDef Scheduler_tp_methods[] = {
    {"reset", (PyCFunction)Periodic_reset,
     METH_NOARGS, Periodic_reset_doc},
    {NULL}  /* Sentinel */
};


/* Scheduler.scheduler */
static PyObject *
Scheduler_scheduler_get(Scheduler *self, void *closure)
{
    _Py_RETURN_OBJECT(self->scheduler);
}

static int
Scheduler_scheduler_set(Scheduler *self, PyObject *value, void *closure)
{
    _Py_PROTECTED_ATTRIBUTE(value, -1);
    return _Scheduler_Set(self, value);
}


/* SchedulerType.tp_getsets */
static PyGetSetDef Scheduler_tp_getsets[] = {
    {"scheduler", (getter)Scheduler_scheduler_get,
     (setter)Scheduler_scheduler_set, NULL, NULL},
    {"at", (getter)Periodic_at_get,
     _Readonly_attribute_set, NULL, NULL},
    {NULL}  /* Sentinel */
};


/* SchedulerType.tp_init */
static int
Scheduler_tp_init(Scheduler *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"scheduler",
                             "loop", "callback", "data", "priority", NULL};
    PyObject *scheduler;
    Loop *loop;
    PyObject *callback, *data = Py_None;
    int priority = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO!O|Oi:__init__", kwlist,
            &scheduler,
            &LoopType, &loop, &callback, &data, &priority)) {
        return -1;
    }
    if (__Watcher_Init((_Watcher *)self, loop, callback, data, priority)) {
        return -1;
    }
    return _Scheduler_Set(self, scheduler);
}


/* SchedulerType.tp_new */
static PyObject *
Scheduler_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    Scheduler *self = NULL;

    if ((self = (Scheduler *)__Watcher_New(type, EV_PERIODIC, sizeof(ev_periodic)))) {
        if (!(self->prepare = PyObject_Malloc(sizeof(ev_prepare)))) {
            PyErr_NoMemory();
            Py_CLEAR(self);
        }
        else {
            ev_prepare_init(self->prepare, _Scheduler_Stop);
            self->prepare->data = self;
            ev_periodic_set((ev_periodic *)((_Watcher *)self)->watcher,
                            0.0, 0.0, _Scheduler_Schedule);
        }
    }
    return (PyObject *)self;
}


/* SchedulerType */
static PyTypeObject SchedulerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mood.event.Scheduler",                   /*tp_name*/
    sizeof(Scheduler),                        /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)Scheduler_tp_dealloc,         /*tp_dealloc*/
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
    Scheduler_tp_doc,                         /*tp_doc*/
    (traverseproc)Scheduler_tp_traverse,      /*tp_traverse*/
    (inquiry)Scheduler_tp_clear,              /*tp_clear*/
    0,                                        /*tp_richcompare*/
    0,                                        /*tp_weaklistoffset*/
    0,                                        /*tp_iter*/
    0,                                        /*tp_iternext*/
    Scheduler_tp_methods,                     /*tp_methods*/
    0,                                        /*tp_members*/
    Scheduler_tp_getsets,                     /*tp_getsets*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)Scheduler_tp_init,              /*tp_init*/
    0,                                        /*tp_alloc*/
    Scheduler_tp_new,                         /*tp_new*/
    0,                                        /*tp_free*/
    0,                                        /*tp_is_gc*/
    0,                                        /*tp_bases*/
    0,                                        /*tp_mro*/
    0,                                        /*tp_cache*/
    0,                                        /*tp_subclasses*/
    0,                                        /*tp_weaklist*/
    0,                                        /*tp_del*/
    0,                                        /*tp_version_tag*/
    (destructor)Scheduler_tp_finalize,        /*tp_finalize*/
};
