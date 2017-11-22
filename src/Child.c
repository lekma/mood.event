/* ----------------------------------------------------------------------------
 helpers
 ---------------------------------------------------------------------------- */

/* set the Child */
static int
_Child_Set(_Watcher *self, int pid, int trace)
{
    ev_child_set((ev_child *)self->watcher, pid, trace);
    return 0;
}


/* ----------------------------------------------------------------------------
 ChildType
 ---------------------------------------------------------------------------- */

/* ChildType.tp_doc */
PyDoc_STRVAR(Child_tp_doc,
"Child(pid, trace, loop, callback[, data=None, priority=0])");


/* Child.set(pid, trace) */
PyDoc_STRVAR(Child_set_doc,
"set(pid, trace)");

static PyObject *
Child_set(_Watcher *self, PyObject *args)
{
    int pid, trace;

    __Watcher_Set(self);
    if (!PyArg_ParseTuple(args, "ip:set", &pid, &trace)) {
        return NULL;
    }
    if (_Child_Set(self, pid, trace)) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* ChildType.tp_methods */
static PyMethodDef Child_tp_methods[] = {
    {"set", (PyCFunction)Child_set,
     METH_VARARGS, Child_set_doc},
    {NULL}  /* Sentinel */
};


/* Child.pid */
static PyObject *
Child_pid_get(_Watcher *self, void *closure)
{
    return PyLong_FromLong(((ev_child *)self->watcher)->pid);
}


/* Child.rpid */
static PyObject *
Child_rpid_get(_Watcher *self, void *closure)
{
    return PyLong_FromLong(((ev_child *)self->watcher)->rpid);
}

static int
Child_rpid_set(_Watcher *self, PyObject *value, void *closure)
{
    long rpid = -1;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    rpid = PyLong_AsLong(value);
    _Py_CHECK_INT_ATTRIBUTE(rpid, -1);
    ((ev_child *)self->watcher)->rpid = rpid;
    return 0;
}


/* Child.rstatus */
static PyObject *
Child_rstatus_get(_Watcher *self, void *closure)
{
    return PyLong_FromLong(((ev_child *)self->watcher)->rstatus);
}

static int
Child_rstatus_set(_Watcher *self, PyObject *value, void *closure)
{
    long rstatus = -1;

    _Py_PROTECTED_ATTRIBUTE(value, -1);
    rstatus = PyLong_AsLong(value);
    _Py_CHECK_INT_ATTRIBUTE(rstatus, -1);
    ((ev_child *)self->watcher)->rstatus = rstatus;
    return 0;
}


/* ChildType.tp_getsets */
static PyGetSetDef Child_tp_getsets[] = {
    {"pid", (getter)Child_pid_get,
     _Readonly_attribute_set, NULL, NULL},
    {"rpid", (getter)Child_rpid_get,
     (setter)Child_rpid_set, NULL, NULL},
    {"rstatus", (getter)Child_rstatus_get,
     (setter)Child_rstatus_set, NULL, NULL},
    {NULL}  /* Sentinel */
};


/* ChildType.tp_init */
static int
Child_tp_init(_Watcher *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"pid", "trace",
                             "loop", "callback", "data", "priority", NULL};
    int pid, trace;
    Loop *loop;
    PyObject *callback, *data = Py_None;
    int priority = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ipO!O|Oi:__init__", kwlist,
            &pid, &trace,
            &LoopType, &loop, &callback, &data, &priority)) {
        return -1;
    }
    if (!ev_is_default_loop(loop->loop)) {
        PyErr_SetString(Error,
                        "Child watchers are only supported in the 'default loop'");
        return -1;
    }
    if (__Watcher_Init(self, loop, callback, data, priority)) {
        return -1;
    }
    return _Child_Set(self, pid, trace);
}


/* ChildType.tp_new */
static PyObject *
Child_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return __Watcher_New(type, EV_CHILD, sizeof(ev_child));
}


/* ChildType */
static PyTypeObject ChildType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mood.event.Child",                       /*tp_name*/
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
    Child_tp_doc,                             /*tp_doc*/
    0,                                        /*tp_traverse*/
    0,                                        /*tp_clear*/
    0,                                        /*tp_richcompare*/
    0,                                        /*tp_weaklistoffset*/
    0,                                        /*tp_iter*/
    0,                                        /*tp_iternext*/
    Child_tp_methods,                         /*tp_methods*/
    0,                                        /*tp_members*/
    Child_tp_getsets,                         /*tp_getsets*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)Child_tp_init,                  /*tp_init*/
    0,                                        /*tp_alloc*/
    Child_tp_new,                             /*tp_new*/
};
