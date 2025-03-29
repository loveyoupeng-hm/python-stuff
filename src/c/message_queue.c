#include <Python.h>
#include <stdatomic.h>
#include <stdio.h>
#include <threads.h>

typedef struct
{
    PyObject_HEAD int size;
    PyObject *callback; /* last name */
    atomic_long head;
    int *queue;
    long cached_head;
    atomic_long tail;
    long cached_tail;
    thrd_t thread;
} MessageQueue;

static void
MessageQueue_dealloc(MessageQueue *self)
{
    Py_XDECREF(self->callback);
    Py_XDECREF(self->queue);
    Py_TYPE(self)->tp_free((PyObject *)self);
    if (self->queue)
        free(self->queue);
}

static PyObject *
MessageQueue_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    MessageQueue *self;
    self = (MessageQueue *)type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->cached_head = 0;
        self->cached_tail = 0;
        self->head = 0;
        self->tail = 0;
        self->queue = NULL;
        self->callback = NULL;
    }
    return (PyObject *)self;
}

static int
MessageQueue_init(MessageQueue *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"callback", "size", NULL};
    PyObject *callback = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Oi", kwlist,
                                     &callback,
                                     &self->size))
        return -1;

    if (callback)
    {
        Py_XSETREF(self->callback, Py_NewRef(callback));
    }
    self->queue = (int *)malloc(sizeof(int) * self->size);
    return 0;
}

static PyMemberDef MessageQueue_members[] = {
    {"size", Py_T_INT, offsetof(MessageQueue, size), 0,
     "size"},
    {NULL} /* Sentinel */
};

static int MessageQueue_run(void *self)
{

    MessageQueue *target = (MessageQueue *)self;
    PyObject *arglist;

    for (int i = 0; i < 10; ++i)
    {
        printf("in c publish value %d\n", i);
        arglist = Py_BuildValue("i", i);
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();
        PyObject_CallOneArg(target->callback, arglist);
        /* Release the thread. No Python API allowed beyond this point. */
        PyGILState_Release(gstate);
    }

    return 0;
}

static PyObject *
MessageQueue_start(MessageQueue *self, PyObject *Py_UNUSED(ignore))
{
    printf("run in start\n");
    thrd_create(&self->thread, MessageQueue_run, self);
    Py_INCREF(Py_None);
    return Py_None;
};

static PyMethodDef MessageQueue_methods[] = {
    {"start", (PyCFunction)MessageQueue_start, METH_NOARGS,
     "Start Processing"},
    {NULL} /* Sentinel */
};

static PyObject *
MessageQueue_repr(MessageQueue *self)
{
    return PyUnicode_FromFormat("MessageQueue(capacity=%d, size=%d)",
                                self->size, self->head - self->tail);
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