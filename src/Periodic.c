/* ----------------------------------------------------------------------------
 helpers
 ---------------------------------------------------------------------------- */

/* check offset and interval */
static int
_Periodic_CheckArgs(double offset, double interval)
{
    static const double INTERVAL_MIN = 1/8192;

    _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(interval, -1);
    if (interval > 0.0) {
        if (interval < INTERVAL_MIN) {
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


/* set the Periodic */
static int
_Periodic_Set(_Watcher *self, double offset, double interval)
{
    if (_Periodic_CheckArgs(offset, interval)) {
        return -1;
    }
    ev_periodic_set((ev_periodic *)self->watcher, offset, interval, 0);
    return 0;
}


/* ----------------------------------------------------------------------------
 PeriodicType
 ---------------------------------------------------------------------------- */

/* PeriodicType.tp_doc */
PyDoc_STRVAR(Periodic_tp_doc,
"Periodic(offset, interval, loop, callback[, data=None, priority=0])");


/* Periodic.set(offset, interval) */
PyDoc_STRVAR(Periodic_set_doc,
"set(offset, interval)");

static PyObject *
Periodic_set(_Watcher *self, PyObject *args)
{
    double offset, interval;

    __Watcher_Set(self);
    if (!PyArg_ParseTuple(args, "dd:set", &offset, &interval)) {
        return NULL;
    }
    if (_Periodic_Set(self, offset, interval)) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Periodic.reset() */
PyDoc_STRVAR(Periodic_reset_doc,
"reset()");

static PyObject *
Periodic_reset(_Watcher *self)
{
    ev_periodic_again(self->loop->loop,
                      (ev_periodic *)self->watcher);
    Py_RETURN_NONE;
}


/* PeriodicType.tp_methods */
static PyMethodDef Periodic_tp_methods[] = {
    {"set", (PyCFunction)Periodic_set,
     METH_VARARGS, Periodic_set_doc},
    {"reset", (PyCFunction)Periodic_reset,
     METH_NOARGS, Periodic_reset_doc},
    {NULL}  /* Sentinel */
};


/* Periodic.offset */
static PyObject *
Periodic_offset_get(_Watcher *self, void *closure)
{
    return PyFloat_FromDouble(((ev_periodic *)self->watcher)->offset);
}

static int
Periodic_offset_set(_Watcher *self, PyObject *value, void *closure)
{
    double offset = -1.0;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    offset = PyFloat_AsDouble(value);
    if (offset == -1.0 && PyErr_Occurred()) {
        return -1;
    }
    if (_Periodic_CheckArgs(offset, ((ev_periodic *)self->watcher)->interval)) {
        return -1;
    }
    ((ev_periodic *)self->watcher)->offset = offset;
    return 0;
}


/* Periodic.interval */
static PyObject *
Periodic_interval_get(_Watcher *self, void *closure)
{
    return PyFloat_FromDouble(((ev_periodic *)self->watcher)->interval);
}

static int
Periodic_interval_set(_Watcher *self, PyObject *value, void *closure)
{
    double interval = -1.0;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    interval = PyFloat_AsDouble(value);
    if (interval == -1.0 && PyErr_Occurred()) {
        return -1;
    }
    if (_Periodic_CheckArgs(((ev_periodic *)self->watcher)->offset, interval)) {
        return -1;
    }
    ((ev_periodic *)self->watcher)->interval = interval;
    return 0;
}


/* Periodic.at */
static PyObject *
Periodic_at_get(_Watcher *self, void *closure)
{
    return PyFloat_FromDouble(
        ev_periodic_at((ev_periodic *)self->watcher));
}


/* PeriodicType.tp_getsets */
static PyGetSetDef Periodic_tp_getsets[] = {
    {"offset", (getter)Periodic_offset_get,
     (setter)Periodic_offset_set, NULL, NULL},
    {"interval", (getter)Periodic_interval_get,
     (setter)Periodic_interval_set, NULL, NULL},
    {"at", (getter)Periodic_at_get,
     _Readonly_attribute_set, NULL, NULL},
    {NULL}  /* Sentinel */
};


/* PeriodicType.tp_init */
static int
Periodic_tp_init(_Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"offset", "interval",
                             "loop", "callback", "data", "priority", NULL};
    double offset, interval;
    Loop *loop;
    PyObject *callback, *data = Py_None;
    int priority = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ddO!O|Oi:__init__", kwlist,
            &offset, &interval,
            &LoopType, &loop, &callback, &data, &priority)) {
        return -1;
    }
    if (__Watcher_Init(self, loop, callback, data, priority)) {
        return -1;
    }
    return _Periodic_Set(self, offset, interval);
}


/* PeriodicType.tp_new */
static PyObject *
Periodic_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return __Watcher_New(type, EV_PERIODIC, sizeof(ev_periodic));
}


/* PeriodicType */
static PyTypeObject PeriodicType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mood.event.Periodic",                    /*tp_name*/
    sizeof(_Watcher),                         /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    0,                                        /*tp_dealloc*/
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_FINALIZE, /*tp_flags*/
    Periodic_tp_doc,                          /*tp_doc*/
    0,                                        /*tp_traverse*/
    0,                                        /*tp_clear*/
    0,                                        /*tp_richcompare*/
    0,                                        /*tp_weaklistoffset*/
    0,                                        /*tp_iter*/
    0,                                        /*tp_iternext*/
    Periodic_tp_methods,                      /*tp_methods*/
    0,                                        /*tp_members*/
    Periodic_tp_getsets,                      /*tp_getsets*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)Periodic_tp_init,               /*tp_init*/
    0,                                        /*tp_alloc*/
    Periodic_tp_new,                          /*tp_new*/
};
