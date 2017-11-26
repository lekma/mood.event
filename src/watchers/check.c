#include "watcher.h"


#if EV_CHECK_ENABLE


/* --------------------------------------------------------------------------
   Check
   -------------------------------------------------------------------------- */

/* Check_Type.tp_new */
static PyObject *
Check_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    return Watcher_new(type, EV_CHECK, sizeof(ev_check));
}


/* -------------------------------------------------------------------------- */

PyTypeObject Check_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mood.event.Check",
    .tp_basicsize = sizeof(Watcher),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Check(loop, callback[, data=None, priority=0])",
    .tp_new = (newfunc)Check_tp_new,
};


#endif // !EV_CHECK_ENABLE
