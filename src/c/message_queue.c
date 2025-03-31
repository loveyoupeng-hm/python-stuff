#include <Python.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <threads.h>

#include "one2onequeue.h"

enum Status
{
    New = 0,
    Running = 1,
    Stopped = 2
};

typedef struct
{
    PyObject_HEAD;
    int size;
    PyObject *callback; /* last name */
    volatile atomic_int status;
    One2OneQueue *queue;
    thrd_t consumer_thread;
    thrd_t producer_thread;
} MessageQueue;

static PyObject *MessageQueue_exit(MessageQueue *self, PyObject *args)
{
    atomic_store_explicit(&self->status, Stopped, memory_order_release);
    Py_RETURN_FALSE;
};
static void
MessageQueue_dealloc(MessageQueue *self)
{
    MessageQueue_exit(self, NULL);
    int result;
    thrd_join(self->consumer_thread, &result);
    thrd_join(self->producer_thread, &result);
    if (self->queue)
        free(self->queue);
    Py_XDECREF(self->callback);
    Py_XDECREF(self->queue);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *
MessageQueue_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    MessageQueue *self;
    self = (MessageQueue *)type->tp_alloc(type, 0);

    if (self != NULL)
    {
        self->queue = NULL;
        self->callback = NULL;
        atomic_store_explicit(&self->status, New, memory_order_relaxed);
    }

    return (PyObject *)self;
}

static int
MessageQueue_init(MessageQueue *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"callback", "size", NULL};
    PyObject *callback = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oi", kwlist,
                                     &callback,
                                     &self->size))

        return -1;

    Py_XINCREF(callback);
    self->callback = callback;
    self->queue = one2onequeue_new(self->size, sizeof(int));
    return 0;
}

static PyMemberDef MessageQueue_members[] = {
    {"size", Py_T_INT, offsetof(MessageQueue, size), 0,
     "size"},
    {NULL} /* Sentinel */
};

static int MessageQueue_consumer(void *self)
{

    MessageQueue *target = (MessageQueue *)self;
    int value = 0;

    atomic_load_explicit(&target->status, memory_order_acquire);
    void *result = NULL;
    int count = 0;
    while (target->status == Running)
    {
        result = NULL;
        value = -1;
        while (NULL == (result = one2onequeue_poll(target->queue)))
        {
            thrd_yield();
            atomic_load_explicit(&target->status, memory_order_acquire);
            if (target->status != Running)
            {
                return 0;
            }
                }
        value = *((int *)result);
        free(result);
        PyGILState_STATE gstate = PyGILState_Ensure();
        PyObject *arglist = Py_BuildValue("i", value);
        PyObject_CallOneArg(target->callback, arglist);
        Py_DECREF(arglist);
        PyGILState_Release(gstate);
        atomic_load_explicit(&target->status, memory_order_acquire);
    }

    return 0;
};

static int MessageQueue_generate(void *self)
{
    MessageQueue *target = (MessageQueue *)self;
    int value = 0;

    atomic_load_explicit(&target->status, memory_order_acquire);
    int *data = malloc(sizeof(int));
    while (target->status == Running)
    {
        *data = value;
        while (!one2onequeue_offer(target->queue, data))
        {
            thrd_yield();
            atomic_load_explicit(&target->status, memory_order_acquire);
            if (target->status != Running)
            {
                return 0;
            }
        }
        value++;
        atomic_load_explicit(&target->status, memory_order_acquire);
    }
    free(data);
    return 0;
}

static PyObject *
MessageQueue_start(MessageQueue *self, PyObject *Py_UNUSED(ignore))
{
    atomic_store_explicit(&self->status, Running, memory_order_release);
    thrd_create(&self->consumer_thread, MessageQueue_consumer, self);
    thrd_create(&self->producer_thread, MessageQueue_generate, self);
    Py_INCREF(Py_None);

    return Py_None;
};

static PyObject *MessageQueue_enter(MessageQueue *self)
{
    Py_INCREF(self);
    MessageQueue_start(self, NULL);
    return (PyObject *)self;
};

static PyMethodDef MessageQueue_methods[] = {
    {"start", (PyCFunction)MessageQueue_start, METH_NOARGS,
     "Start Processing"},
    {"__enter__", (PyCFunction)MessageQueue_enter, METH_NOARGS, "enter scope"},
    {"__exit__", (PyCFunction)MessageQueue_enter, METH_VARARGS, "enter scope"},
    {NULL} /* Sentinel */
};

static PyObject *
MessageQueue_repr(MessageQueue *self)
{
    return PyUnicode_FromFormat("MessageQueue(capacity=%d, size=%d)",
                                self->size, one2onequeue_size(self->queue));
}

static PyObject *
MessageQueue_str(MessageQueue *self)
{
    return MessageQueue_repr(self);
}

static PyTypeObject CustomType = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
                   .tp_name = "message_queue.MessageQueue",
    .tp_doc = PyDoc_STR("Message Queue"),
    .tp_basicsize = sizeof(MessageQueue),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = MessageQueue_new,
    .tp_init = (initproc)MessageQueue_init,
    .tp_dealloc = (destructor)MessageQueue_dealloc,
    .tp_members = MessageQueue_members,
    .tp_methods = MessageQueue_methods,
    .tp_repr = (reprfunc)MessageQueue_repr,
    .tp_str = (reprfunc)MessageQueue_str,
};

static PyModuleDef queuemodule = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "message_queue",
    .m_doc = "Example module that creates an extension type.",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_message_queue(void)
{
    PyObject *m;

    if (PyType_Ready(&CustomType) < 0)
        return NULL;

    m = PyModule_Create(&queuemodule);

    if (m == NULL)
        return NULL;

    if (PyModule_AddObjectRef(m, "MessageQueue", (PyObject *)&CustomType) < 0)
    {
        Py_DECREF(m);

        return NULL;
    }

    return m;
}
