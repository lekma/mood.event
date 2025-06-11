#include "event.h"


#if EV_PREPARE_ENABLE


/* --------------------------------------------------------------------------
   Prepare
   -------------------------------------------------------------------------- */

static inline Watcher *
__Prepare_New(PyTypeObject *type)
{
    return Watcher_New(type, EV_PREPARE, sizeof(ev_prepare));
}


/* Prepare_Type ------------------------------------------------------------- */

/* Prepare_Type.tp_new */
static PyObject *
Prepare_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Prepare_New(type);
}


PyTypeObject Prepare_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Prepare",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Prepare(loop, callback[, data=None, priority=0])",
    .tp_new = Prepare_tp_new,
};


#endif // !EV_PREPARE_ENABLE
