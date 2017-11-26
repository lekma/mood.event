#include "watcher.h"


#if EV_PREPARE_ENABLE


/* --------------------------------------------------------------------------
   Prepare
   -------------------------------------------------------------------------- */

/* Prepare_Type.tp_new */
static PyObject *
Prepare_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return Watcher_new(type, EV_PREPARE, sizeof(ev_prepare));
}


/* -------------------------------------------------------------------------- */

PyTypeObject Prepare_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Prepare",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Prepare(loop, callback[, data=None, priority=0])",
    .tp_new = (newfunc)Prepare_tp_new,
};


#endif // !EV_PREPARE_ENABLE
