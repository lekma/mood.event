#include "watcher.h"


#if EV_ASYNC_ENABLE


/* --------------------------------------------------------------------------
   Async
   -------------------------------------------------------------------------- */

/* Async_Type.tp_new */
static PyObject *
Async_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return Watcher_new(type, EV_ASYNC, sizeof(ev_async));
}


/* -------------------------------------------------------------------------- */

/* Async.send() */
static PyObject *
Async_send(Watcher *self)
{
    ev_async_send(self->loop->loop, ((ev_async *)self->watcher));
    Py_RETURN_NONE;
}


/* Async_Type.tp_methods */
static PyMethodDef Async_tp_methods[] = {
    {
        "send",
        (PyCFunction)Async_send,
        METH_NOARGS,
        "send()"
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

/* Async.sent */
static PyObject *
Async_sent_getter(Watcher *self, void *closure)
{
    return PyBool_FromLong(ev_async_pending(((ev_async *)self->watcher)));
}


/* Async_Type.tp_getsets */
static PyGetSetDef Async_tp_getsets[] = {
    {
        "sent",
        (getter)Async_sent_getter,
        _Py_READONLY_ATTRIBUTE,
        NULL,
        NULL
    },
    {NULL}
};


/* -------------------------------------------------------------------------- */

PyTypeObject Async_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Async",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Async(loop, callback[, data=None, priority=0])",
    .tp_methods = Async_tp_methods,
    .tp_getset = Async_tp_getsets,
    .tp_new = (newfunc)Async_tp_new,
};


#endif // !EV_ASYNC_ENABLE
