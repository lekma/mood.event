#include "event.h"


#if EV_CHECK_ENABLE


/* --------------------------------------------------------------------------
   Check
   -------------------------------------------------------------------------- */

static inline Watcher *
__Check_New(PyTypeObject *type)
{
    return Watcher_New(type, EV_CHECK, sizeof(ev_check));
}


/* Check_Type --------------------------------------------------------------- */

/* Check_Type.tp_new */
static PyObject *
Check_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return (PyObject *)__Check_New(type);
}


PyTypeObject Check_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Check",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Check(loop, callback[, data=None, priority=0])",
    .tp_new = Check_tp_new,
};


#endif // !EV_CHECK_ENABLE
