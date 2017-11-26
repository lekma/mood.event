#ifndef Py_MOOD___WATCHER___H
#define Py_MOOD___WATCHER___H


#include "event.h"


#ifdef __cplusplus
extern "C" {
#endif


/* -------------------------------------------------------------------------- */

typedef struct {
    PyObject_HEAD
    int ev_type;
    ev_watcher *watcher;
    Loop *loop;
    PyObject *callback;
    PyObject *data;
} Watcher;


Watcher *__Watcher_alloc__(PyTypeObject *);
int __Watcher_post_alloc__(Watcher *, int , size_t);
void __Watcher_finalize__(Watcher *);
int __Watcher_traverse__(Watcher *, visitproc, void *);
int __Watcher_clear__(Watcher *);
void __Watcher_dealloc__(Watcher *);


int Watcher_check_active(Watcher *, const char *);
int Watcher_check_set(Watcher *);

PyObject *Watcher_new(PyTypeObject *, int , size_t);
int Watcher_init(Watcher *, Loop *, PyObject *, PyObject *, int);


/* -------------------------------------------------------------------------- */

#if EV_PERIODIC_ENABLE
#if EV_PREPARE_ENABLE
typedef struct {
    Watcher watcher;
    ev_prepare *prepare;
    PyObject *reschedule;
    PyObject *err_type;
    PyObject *err_value;
    PyObject *err_traceback;
    int err_fatal;
} Scheduler;
#endif
#endif


/* -------------------------------------------------------------------------- */

#if EV_EMBED_ENABLE
typedef struct {
    Watcher watcher;
    Loop *other;
} Embed;
#endif


/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif


#endif // !Py_MOOD___WATCHER___H
