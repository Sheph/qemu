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

#ifndef _QEMU_YAGL_REF_H
#define _QEMU_YAGL_REF_H

#include "yagl_types.h"

struct yagl_ref;

typedef void (*yagl_ref_destroy_func)(struct yagl_ref */*ref*/);

struct yagl_ref
{
    yagl_ref_destroy_func destroy;

    uint32_t count;
};

/*
 * Initializes ref count to 1.
 */
void yagl_ref_init(struct yagl_ref *ref, yagl_ref_destroy_func destroy);

void yagl_ref_cleanup(struct yagl_ref *ref);

/*
 * Increments ref count.
 */
void yagl_ref_acquire(struct yagl_ref *ref);

/*
 * Decrements ref count and releases when 0.
 */
void yagl_ref_release(struct yagl_ref *ref);

#endif
