#include <Python.h>

static PyObject *
spam_system(PyObject *self, PyObject *args)
{
    return PyLong_FromLong(10);
}

static PyMethodDef SpamMethods[] = {
    /* ... */
    {"system", spam_system, METH_VARARGS,
     "Execute a shell command."},
    /* ... */
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static struct PyModuleDef spammodule = {
    PyModuleDef_HEAD_INIT,
    "spam",                                         /* name of module */
    PyDoc_STR("Documentation for the spam module"), /* module documentation, may be NULL */
    -1,                                             /* size of per-interpreter state of the module,
                                                   or -1 if the module keeps state in global variables. */
    SpamMethods,
    NULL,
    NULL,
    NULL,
    NULL,
};

PyMODINIT_FUNC
PyInit_spam(void)
{
    PyObject *m;
    m = PyModule_Create(&spammodule);
    if (m == NULL)
        return NULL;
    return m;
}