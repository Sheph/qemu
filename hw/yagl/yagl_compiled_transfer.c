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

#include "yagl_compiled_transfer.h"
#include "yagl_vector.h"
#include "yagl_thread.h"
#include "yagl_process.h"
#include "yagl_log.h"
#include "yagl_transport.h"
#include "exec/cpu-all.h"

#define YAGL_TARGET_PAGE_VA(addr) ((addr) & ~(TARGET_PAGE_SIZE - 1))

#define YAGL_TARGET_PAGE_OFFSET(addr) ((addr) & (TARGET_PAGE_SIZE - 1))

static hwaddr yagl_pa(target_ulong va)
{
    hwaddr ret =
        cpu_get_phys_page_debug(current_cpu, va);

    if (ret == -1) {
        return 0;
    } else {
        return ret;
    }
}

struct yagl_compiled_transfer
    *yagl_compiled_transfer_create(target_ulong va,
                                   uint32_t len,
                                   bool is_write)
{
    struct yagl_compiled_transfer *ct;

    ct = g_malloc0(sizeof(*ct));

    ct->va = va;
    ct->len = len;
    ct->is_write = is_write;

    QLIST_INSERT_HEAD(&cur_ts->t->compiled_transfers, ct, entry);

    ct->in_list = true;

    return ct;
}

void yagl_compiled_transfer_destroy(struct yagl_compiled_transfer *ct)
{
    int i;

    if (ct->in_list) {
        QLIST_REMOVE(ct, entry);
        ct->in_list = false;
    }

    for (i = 0; i < ct->num_sections; ++i) {
        cpu_physical_memory_unmap(ct->sections[i].map_base,
                                  ct->sections[i].map_len,
                                  0,
                                  ct->sections[i].map_len);
    }

    g_free(ct->sections);

    ct->sections = NULL;
    ct->num_sections = 0;

    g_free(ct);
}

void yagl_compiled_transfer_prepare(struct yagl_compiled_transfer *ct)
{
    struct yagl_vector v;
    target_ulong last_page_va = YAGL_TARGET_PAGE_VA(ct->va + ct->len - 1);
    target_ulong cur_va = ct->va;
    uint32_t len = ct->len;
    int i, num_sections;

    YAGL_LOG_FUNC_ENTER(yagl_compiled_transfer_prepare,
                        "va = 0x%X, len = 0x%X, is_write = %u",
                        (uint32_t)ct->va,
                        ct->len,
                        (uint32_t)ct->is_write);

    if (ct->in_list) {
        QLIST_REMOVE(ct, entry);
        ct->in_list = false;
    }

    yagl_vector_init(&v, sizeof(struct yagl_compiled_transfer_section), 0);

    while (len) {
        target_ulong start_page_va = YAGL_TARGET_PAGE_VA(cur_va);
        hwaddr start_page_pa = yagl_pa(start_page_va);
        target_ulong end_page_va;
        struct yagl_compiled_transfer_section section;

        if (!start_page_pa) {
            YAGL_LOG_ERROR("yagl_pa of va 0x%X failed", (uint32_t)start_page_va);
            goto fail;
        }

        end_page_va = start_page_va;

        while (end_page_va < last_page_va) {
            target_ulong next_page_va = end_page_va + TARGET_PAGE_SIZE;
            hwaddr next_page_pa = yagl_pa(next_page_va);

            if (!next_page_pa) {
                YAGL_LOG_ERROR("yagl_pa of va 0x%X failed", (uint32_t)next_page_va);
                goto fail;
            }

            /*
             * If the target pages are not linearly spaced, stop.
             */

            if ((next_page_pa < start_page_pa) ||
                ((next_page_pa - start_page_pa) >
                 (next_page_va - start_page_va))) {
                break;
            }

            end_page_va = next_page_va;
        }

        section.map_len = end_page_va + TARGET_PAGE_SIZE - start_page_va;
        section.map_base = cpu_physical_memory_map(start_page_pa, &section.map_len, 0);

        if (!section.map_base || !section.map_len) {
            YAGL_LOG_ERROR("cpu_physical_memory_map(0x%X, %u) failed",
                           (uint32_t)start_page_pa,
                           (uint32_t)section.map_len);
            goto fail;
        }

        section.len = end_page_va + TARGET_PAGE_SIZE - cur_va;

        if (section.len > len) {
            section.len = len;
        }

        section.base = (char*)section.map_base + YAGL_TARGET_PAGE_OFFSET(cur_va);

        yagl_vector_push_back(&v, &section);

        len -= section.len;
        cur_va += section.len;
    }

    ct->num_sections = yagl_vector_size(&v);
    ct->sections = yagl_vector_detach(&v);

    YAGL_LOG_FUNC_EXIT("num_sections = %d", ct->num_sections);

    return;

fail:
    num_sections = yagl_vector_size(&v);

    for (i = 0; i < num_sections; ++i) {
        struct yagl_compiled_transfer_section *section =
            (struct yagl_compiled_transfer_section*)
                ((char*)yagl_vector_data(&v) +
                 (i * sizeof(struct yagl_compiled_transfer_section)));

        cpu_physical_memory_unmap(section->map_base,
                                  section->map_len,
                                  0,
                                  section->map_len);
    }

    yagl_vector_cleanup(&v);

    YAGL_LOG_FUNC_EXIT(NULL);
}

void yagl_compiled_transfer_exec(struct yagl_compiled_transfer *ct, void* data)
{
    int i;

    for (i = 0; i < ct->num_sections; ++i) {
        void *base = ct->sections[i].base;
        uint32_t len = ct->sections[i].len;

        if (ct->is_write) {
            memcpy(base, data, len);
        } else {
            memcpy(data, base, len);
        }

        data = (char*)data + len;
    }
}
