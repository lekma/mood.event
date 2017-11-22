/* ----------------------------------------------------------------------------
 helpers
 ---------------------------------------------------------------------------- */

/* set the Embed */
static int
_Embed_Set(Embed *self, Loop *other)
{
    if (!(ev_backend(other->loop) & ev_embeddable_backends())) {
        PyErr_SetString(Error, "'other' must be embeddable");
        return -1;
    }
    _Py_SET_MEMBER(self->other, other);
    ev_embed_set((ev_embed *)((_Watcher *)self)->watcher, other->loop);
    return 0;
}


/* ----------------------------------------------------------------------------
 EmbedType
 ---------------------------------------------------------------------------- */

/* EmbedType.tp_doc */
PyDoc_STRVAR(Embed_tp_doc,
"Embed(other, loop[, callback=None, data=None, priority=0])");


/* EmbedType.tp_traverse */
static int
Embed_tp_traverse(Embed *self, visitproc visit, void *arg)
{
    Py_VISIT(self->other);
    return _Watcher_tp_traverse((_Watcher *)self, visit, arg);
}


/* EmbedType.tp_clear */
static int
Embed_tp_clear(Embed *self)
{
    Py_CLEAR(self->other);
    return _Watcher_tp_clear((_Watcher *)self);
}


/* EmbedType.tp_dealloc */
static void
Embed_tp_dealloc(Embed *self)
{
    if (PyObject_CallFinalizerFromDealloc((PyObject *)self)) {
        return;
    }
    PyObject_GC_UnTrack(self);
    Embed_tp_clear(self);
    __Watcher_Free((_Watcher *)self);
}


/* Embed.set(other) */
PyDoc_STRVAR(Embed_set_doc,
"set(other)");

static PyObject *
Embed_set(Embed *self, PyObject *args)
{
    Loop *other;

    __Watcher_Set((_Watcher *)self);
    if (!PyArg_ParseTuple(args, "O!:set", &LoopType, &other)) {
        return NULL;
    }
    if (_Embed_Set(self, other)) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* Embed.sweep() */
PyDoc_STRVAR(Embed_sweep_doc,
"sweep()");

static PyObject *
Embed_sweep(Embed *self)
{
    _Watcher *_watcher = (_Watcher *)self;

    ev_embed_sweep(_watcher->loop->loop, (ev_embed *)_watcher->watcher);
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}


/* EmbedType.tp_methods */
static PyMethodDef Embed_tp_methods[] = {
    {"set", (PyCFunction)Embed_set,
     METH_VARARGS, Embed_set_doc},
    {"sweep", (PyCFunction)Embed_sweep,
     METH_NOARGS, Embed_sweep_doc},
    {NULL}  /* Sentinel */
};


/* Embed.other */
static PyObject *
Embed_other_get(Embed *self, void *closure)
{
    _Py_RETURN_OBJECT(self->other);
}


/* EmbedType.tp_getsets */
static PyGetSetDef Embed_tp_getsets[] = {
    {"other", (getter)Embed_other_get,
     _Readonly_attribute_set, NULL, NULL},
    {NULL}  /* Sentinel */
};


/* EmbedType.tp_init */
static int
Embed_tp_init(Embed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"other",
                             "loop", "callback", "data", "priority", NULL};
    Loop *other, *loop;
    PyObject *callback = Py_None, *data = Py_None;
    int priority = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!|OOi:__init__", kwlist,
            &LoopType, &other,
            &LoopType, &loop, &callback, &data, &priority)) {
        return -1;
    }
    if (__Watcher_Init((_Watcher *)self, loop, callback, data, priority)) {
        return -1;
    }
    return _Embed_Set(self, other);
}


/* EmbedType.tp_new */
static PyObject *
Embed_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return __Watcher_New(type, EV_EMBED, sizeof(ev_embed));
}


/* EmbedType */
static PyTypeObject EmbedType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mood.event.Embed",                       /*tp_name*/
    sizeof(Embed),                            /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)Embed_tp_dealloc,             /*tp_dealloc*/
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
    Embed_tp_doc,                             /*tp_doc*/
    (traverseproc)Embed_tp_traverse,          /*tp_traverse*/
    (inquiry)Embed_tp_clear,                  /*tp_clear*/
    0,                                        /*tp_richcompare*/
    0,                                        /*tp_weaklistoffset*/
    0,                                        /*tp_iter*/
    0,                                        /*tp_iternext*/
    Embed_tp_methods,                         /*tp_methods*/
    0,                                        /*tp_members*/
    Embed_tp_getsets,                         /*tp_getsets*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)Embed_tp_init,                  /*tp_init*/
    0,                                        /*tp_alloc*/
    Embed_tp_new,                             /*tp_new*/
};
