/*
#
# Copyright © 2020 Malek Hadj-Ali
# All rights reserved.
#
# This file is part of mood.
#
# mood is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3
# as published by the Free Software Foundation.
#
# mood is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with mood.  If not, see <http://www.gnu.org/licenses/>.
#
*/


#ifndef Py_MOOD_EVENT_H
#define Py_MOOD_EVENT_H


#define PY_SSIZE_T_CLEAN
#include "Python.h"

#include "helpers/helpers.h"


#ifdef __cplusplus
extern "C" {
#endif


#define EV_COMPAT3 0
#include <ev.h>


/* helpers */

#define _PyObject_Callback PyObject_CallFunctionObjArgs


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
            PyErr_SetString(PyExc_ValueError, \
                            "a positive float or 0.0 is required"); \
            return (r); \
        } \
    } while (0)


int _Py_INVOKE_VERIFY(PyObject *, const char *);


/* global objects */

extern PyObject *Error;
extern PyObject *DefaultLoop;


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
PyObject *Loop_New(PyObject *, PyObject *, int);
void Loop_FullStop(ev_loop *);
void Loop_WarnOrStop(ev_loop *, PyObject *);


/* Watcher base - not exposed */

typedef struct {
    PyObject_HEAD
    int ev_type;
    ev_watcher *watcher;
    Loop *loop;
    PyObject *callback;
    PyObject *data;
} Watcher;

extern PyTypeObject Watcher_Type;
Watcher *Watcher_Alloc(PyTypeObject *);
int Watcher_Setup(Watcher *, int, size_t);
Watcher *Watcher_New(PyTypeObject *, int, size_t);
int Watcher_Init(Watcher *, Loop *, PyObject *, PyObject *, int);
void Watcher_Finalize(Watcher *);
int Watcher_Traverse(Watcher *, visitproc, void *);
int Watcher_Clear(Watcher *);
void Watcher_Dealloc(Watcher *);
int Watcher_IsActive(Watcher *, const char *);
int Watcher_CannotSet(Watcher *);


/* watcher types */

extern PyTypeObject Io_Type;
Watcher *Io_New(Loop *, PyObject *, PyObject *);

extern PyTypeObject Timer_Type;
Watcher *Timer_New(Loop *, PyObject *, PyObject *);

#if EV_PERIODIC_ENABLE
extern PyTypeObject Periodic_Type;
Watcher *Periodic_New(Loop *, PyObject *, PyObject *);
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

extern PyTypeObject Scheduler_Type;
Scheduler *Scheduler_New(Loop *, PyObject *, PyObject *);
#endif
#endif

#if EV_SIGNAL_ENABLE
extern PyTypeObject Signal_Type;
Watcher *Signal_New(Loop *, PyObject *, PyObject *);
#endif

#if EV_CHILD_ENABLE
extern PyTypeObject Child_Type;
Watcher *Child_New(Loop *, PyObject *, PyObject *);
#endif

#if EV_IDLE_ENABLE
extern PyTypeObject Idle_Type;
Watcher *Idle_New(Loop *, PyObject *, PyObject *);
#endif

#if EV_PREPARE_ENABLE
extern PyTypeObject Prepare_Type;
Watcher *Prepare_New(Loop *, PyObject *, PyObject *);
#endif

#if EV_CHECK_ENABLE
extern PyTypeObject Check_Type;
Watcher *Check_New(Loop *, PyObject *, PyObject *);
#endif

#if EV_EMBED_ENABLE
typedef struct {
    Watcher watcher;
    Loop *other;
} Embed;

extern PyTypeObject Embed_Type;
Embed *Embed_New(Loop *, PyObject *, PyObject *);
#endif

#if EV_FORK_ENABLE
extern PyTypeObject Fork_Type;
Watcher *Fork_New(Loop *, PyObject *, PyObject *);
#endif

#if EV_ASYNC_ENABLE
extern PyTypeObject Async_Type;
Watcher *Async_New(Loop *, PyObject *, PyObject *);
#endif


#ifdef __cplusplus
}
#endif


#endif // !Py_MOOD_EVENT_H

