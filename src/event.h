#ifndef Py_MOOD_EVENT_H
#define Py_MOOD_EVENT_H


#define PY_SSIZE_T_CLEAN
#include "Python.h"

#include "helpers/helpers.h"


#ifdef __cplusplus
extern "C" {
#endif


/* -------------------------------------------------------------------------- */

#define EV_COMPAT3 0
#include <ev.h>


/* -------------------------------------------------------------------------- */

#define _Py_CHECK_CALLABLE(cb, r) \
    do { \
        if (!PyCallable_Check((cb))) { \
            PyErr_SetString(PyExc_TypeError, "a callable is required"); \
            return (r); \
        } \
    } while (0)

#define _Py_CHECK_CALLABLE_OR_NONE(cb, r) \
    do { \
        if ((cb) != Py_None && !PyCallable_Check((cb))) { \
            PyErr_SetString(PyExc_TypeError, "a callable or None is required"); \
            return (r); \
        } \
    } while (0)

#define _Py_CHECK_POSITIVE_OR_ZERO_FLOAT(v, r) \
    do { \
        if ((v) < 0.0) { \
            PyErr_SetString( \
                PyExc_ValueError, "a positive float or 0.0 is required" \
            ); \
            return (r); \
        } \
    } while (0)


int _Py_Invoke_Verify(PyObject *, const char *);

#define _Py_Invoke_Callback PyObject_CallFunctionObjArgs


/* -------------------------------------------------------------------------- */

extern PyObject *EventError;
extern PyObject *DefaultLoop;

extern _Py_Identifier PyId___event_fatal__;


void ev_loop_stop(ev_loop *);
void ev_loop_warn(ev_loop *, PyObject *);


/* Loop */
typedef struct {
    PyObject_HEAD
    ev_loop *loop;
    PyObject *callback;
    PyObject *data;
    double io_ival;
    double timeout_ival;
} Loop;

extern PyTypeObject Loop_Type;


PyObject *Loop_new(PyTypeObject *, PyObject *, PyObject *, int);


/* watcher types */
extern PyTypeObject Watcher_Type;
extern PyTypeObject Io_Type;
extern PyTypeObject Timer_Type;
#if EV_PERIODIC_ENABLE
extern PyTypeObject Periodic_Type;
#if EV_PREPARE_ENABLE
extern PyTypeObject Scheduler_Type;
#endif
#endif
#if EV_SIGNAL_ENABLE
extern PyTypeObject Signal_Type;
#endif
#if EV_CHILD_ENABLE
extern PyTypeObject Child_Type;
#endif
#if EV_IDLE_ENABLE
extern PyTypeObject Idle_Type;
#endif
#if EV_PREPARE_ENABLE
extern PyTypeObject Prepare_Type;
#endif
#if EV_CHECK_ENABLE
extern PyTypeObject Check_Type;
#endif
#if EV_EMBED_ENABLE
extern PyTypeObject Embed_Type;
#endif
#if EV_FORK_ENABLE
extern PyTypeObject Fork_Type;
#endif
#if EV_ASYNC_ENABLE
extern PyTypeObject Async_Type;
#endif


/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif


#endif // !Py_MOOD_EVENT_H
