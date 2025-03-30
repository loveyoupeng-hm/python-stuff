#include <Python.h>
#include <stdatomic.h>
#include <stdio.h>
#include <threads.h>

enum Status
{
    New = 0,
    Running,
    Stopped
};

typedef struct
{
    PyObject_HEAD;
    int size;
    PyObject *callback; /* last name */
    volatile atomic_long head;
    volatile atomic_int status;
    int *queue;
    long cached_head;
    volatile atomic_long tail;
    long cached_tail;
    thrd_t consumer_thread;
    thrd_t producer_thread;
} MessageQueue;

static void
MessageQueue_dealloc(MessageQueue *self)
{
    Py_XDECREF(self->callback);
    Py_XDECREF(self->queue);
    Py_TYPE(self)->tp_free((PyObject *)self);
    self->status = Stopped;
    int result;
    thrd_join(self->producer_thread, &result);
    thrd_detach(self->consumer_thread);
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
        self->status = New;
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

static int MessageQueue_consumer(void *self)
{

    MessageQueue *target = (MessageQueue *)self;
    PyObject *arglist;

    while (target->status == Running)
    {
        long next = target->tail + 1;
        if (next >= target->cached_head)
        {
            do
            {
                if (target->status != Running)
                {
                    return 0;
                }
                target->cached_head = target->head;
            } while (next >= target->cached_head);
        }
        arglist = Py_BuildValue("i", target->queue[next % target->size]);
        target->tail = next;
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();
        PyObject_CallOneArg(target->callback, arglist);
        PyGILState_Release(gstate);
    }

    return 0;
};

static int MessageQueue_generate(void *self)
{
    MessageQueue *target = (MessageQueue *)self;
    int value = 0;
    while (target->status == Running)
    {
        long next = target->head + 1;
        if (next - target->cached_tail >= target->size - 1)
        {
            do
            {
                if (target->status != Running)
                {
                    return 0;
                }
                target->cached_tail = target->tail;
            } while (next - target->cached_tail >= target->size - 1);
        }
        target->queue[next % target->size] = value;
        value++;
        target->head = next;
    }

    return 0;
}

static PyObject *
MessageQueue_start(MessageQueue *self, PyObject *Py_UNUSED(ignore))
{
    self->status = Running;
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

static PyObject *MessageQueue_exit(MessageQueue *self, PyObject *args)
{
    self->status = Stopped;
    Py_RETURN_FALSE;
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