/*
 * yagl
 *
 * Copyright (c) 2000 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact:
 * Stanislav Vorobiov <s.vorobiov@samsung.com>
 * Jinhyung Jo <jinhyung.jo@samsung.com>
 * YeongKyoon Lee <yeongkyoon.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Contributors:
 * - S-Core Co., Ltd
 *
 */

#include "yagl_dyn_lib.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#ifdef _WIN32
typedef HINSTANCE yagl_dyn_lib_handle;
#else
typedef void* yagl_dyn_lib_handle;
#endif

struct yagl_dyn_lib
{
    yagl_dyn_lib_handle handle;
    char *last_error;
};

struct yagl_dyn_lib *yagl_dyn_lib_create(void)
{
    return g_malloc0(sizeof(struct yagl_dyn_lib));
}

void yagl_dyn_lib_destroy(struct yagl_dyn_lib *dyn_lib)
{
    yagl_dyn_lib_unload(dyn_lib);
    g_free(dyn_lib);
}

bool yagl_dyn_lib_load(struct yagl_dyn_lib *dyn_lib, const char *name)
{
    yagl_dyn_lib_handle handle;
#ifdef _WIN32
    handle = LoadLibraryA(name);

    if (!handle) {
        free(dyn_lib->last_error);
        dyn_lib->last_error = strdup("Unable to load library"); // TODO: replace with real error message
    }
#else
    handle = dlopen(name, RTLD_NOW | RTLD_GLOBAL);

    if (!handle) {
        const char *err;

        free(dyn_lib->last_error);

        err = dlerror();

        if (err) {
            dyn_lib->last_error = strdup(err);
        } else {
            dyn_lib->last_error = strdup("Unable to load library");
        }
    }
#endif
    else {
        yagl_dyn_lib_unload(dyn_lib);
        dyn_lib->handle = handle;
        free(dyn_lib->last_error);
        dyn_lib->last_error = NULL;
    }

    return (handle != 0);
}

void yagl_dyn_lib_unload(struct yagl_dyn_lib *dyn_lib)
{
    if (dyn_lib->handle != 0) {
#ifdef _WIN32
        FreeLibrary(dyn_lib->handle);
#else
        dlclose(dyn_lib->handle);
#endif
        dyn_lib->handle = 0;
    }
    free(dyn_lib->last_error);
    dyn_lib->last_error = NULL;
}

void *yagl_dyn_lib_get_sym(struct yagl_dyn_lib *dyn_lib, const char *sym_name)
{
    void *sym = NULL;

    if (!dyn_lib->handle) {
        free(dyn_lib->last_error);
        dyn_lib->last_error = strdup("Library not loaded");

        return NULL;
    }

#ifdef _WIN32
    sym = GetProcAddress(dyn_lib->handle, sym_name);

    if (!sym) {
        free(dyn_lib->last_error);
        dyn_lib->last_error = strdup("Unable to find symbol"); // TODO: replace with real error message
    }
#else
    sym = dlsym(dyn_lib->handle, sym_name);

    if (!sym) {
        const char *err;

        free(dyn_lib->last_error);

        err = dlerror();

        if (err) {
            dyn_lib->last_error = strdup(err);
        } else {
            dyn_lib->last_error = strdup("Unable to load library");
        }
    }
#endif
    return sym;
}

const char *yagl_dyn_lib_get_error(struct yagl_dyn_lib *dyn_lib)
{
    return dyn_lib->last_error;
}
