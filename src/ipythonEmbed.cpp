#include "ipythonEmbed.h"

static const char IPYTHON_EMBED_MODULE[] = "ipythonEmbed";
static const char IPYTHON_EMBED_START_METHOD_NAME[] = "start";
static const char QT_MODULE_NAME[] = "qtcore4.dll";
static const char EVENT_LOOP_FUNC_NAME[] = "?processEvents@QEventDispatcherWin32@QT@@UAE_NV?$QFlags@W4ProcessEventsFlag@QEventLoop@QT@@@2@@Z";

static PyObject* kernel_do_one_iteration = NULL;
static PyObject* commandline_args = NULL;

typedef int (__fastcall *tQEventDispatcherWin32)(void*, void*, int);
tQEventDispatcherWin32 pQEventDispatcherWin32 = NULL;

PyObject* start_ipython_kernel(PyObject* cmdline)
{
    PyObject *ipython_embed_module = NULL,
             *ipython_start_func = NULL,
             *ipython_kernel = NULL,
             *arglist = NULL;

    ipython_embed_module = PyImport_ImportModule(IPYTHON_EMBED_MODULE);
    if (ipython_embed_module == NULL) {
        goto error;
    }

    ipython_start_func = PyObject_GetAttrString(ipython_embed_module, IPYTHON_EMBED_START_METHOD_NAME);
    if (ipython_start_func == NULL) {
        goto error;
    }

    if (PyCallable_Check(ipython_start_func)) {
        if (cmdline != NULL) {
            arglist = Py_BuildValue("(O)", cmdline);
            ipython_kernel = PyObject_CallObject(ipython_start_func, arglist);
        } else {
            ipython_kernel = PyObject_CallObject(ipython_start_func, NULL);
        }
    }

    if (ipython_kernel == NULL || !PyCallable_Check(ipython_kernel)) {
        goto error;
    }

    goto cleanup;
error:
    ipython_kernel = NULL;
cleanup:
    Py_XDECREF(arglist);
    Py_XDECREF(ipython_embed_module);
    Py_XDECREF(ipython_start_func);
    return ipython_kernel;
}

void init_python(void)
{
    // Make sure the python is initialized
    if (!Py_IsInitialized()) {
        Py_Initialize();
    }
}

void init_ipython_kernel(void)
{
    init_python();
    kernel_do_one_iteration = start_ipython_kernel(commandline_args);
}

void ipython_embed_iteration()
{
    PyGILState_STATE state = PyGILState_Ensure();

    if (kernel_do_one_iteration == NULL) {
        init_ipython_kernel();
        //TODO: Report the error, call stack ect.
        if ( PyErr_Occurred() ) {
            msg("A Python Error Occurred trying to start the kernel!\n");
        }
    } else {
        PyObject_CallObject(kernel_do_one_iteration, NULL);
    }

    PyGILState_Release(state);

}

FARPROC eventloop_address()
{
    HMODULE qtmodule = GetModuleHandleA(QT_MODULE_NAME);
    FARPROC src = GetProcAddress(qtmodule, EVENT_LOOP_FUNC_NAME);
    return src;
}

int __fastcall DetourQEventDispatcherWin32(void* ecx, void* edx, int i)
{
    try {
        ipython_embed_iteration();
        return pQEventDispatcherWin32(ecx, edx, i);
    } catch (const std::exception& ex) {
        std::string error = ex.what();
        const char *cstr = error.c_str();
        warning(cstr);
    } catch (...) {
        warning("Something went wrong in the detour!");
    }

    return 0;
}

IPYTHONEMBED_STATUS ipython_embed_start(PyObject* cmdline)
{
    commandline_args = cmdline;

    if (MH_Initialize() != MH_OK) {
        return IPYTHONEMBED_MINHOOK_INIT_FAILED;
    }

    void* qt_eventloop = (void*)eventloop_address();
    if (MH_CreateHook(qt_eventloop,
                     &DetourQEventDispatcherWin32,
                     (LPVOID*)&pQEventDispatcherWin32) != MH_OK) {
        return IPYTHONEMBED_CREATE_HOOK_FAILED;
    }

    if (MH_EnableHook(qt_eventloop) != MH_OK) {
        return IPYTHONEMBED_ENABLE_HOOK_FAILED;
    }

    return IPYTHONEMBED_OK;
}

void ipython_embed_term()
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    Py_XDECREF(kernel_do_one_iteration);
    Py_XDECREF(commandline_args);
}
