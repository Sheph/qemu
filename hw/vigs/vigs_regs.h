/*
 * vigs
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

#ifndef _QEMU_VIGS_REGS_H
#define _QEMU_VIGS_REGS_H

#define VIGS_REG_EXEC 0
#define VIGS_REG_CON 8
#define VIGS_REG_INT 16
#define VIGS_REG_FENCE_LOWER 24
#define VIGS_REG_FENCE_UPPER 32

#define VIGS_REG_CON_MASK 1
#define VIGS_REG_CON_VBLANK_ENABLE 1

#define VIGS_REG_INT_MASK 3
#define VIGS_REG_INT_VBLANK_PENDING 1
#define VIGS_REG_INT_FENCE_ACK_PENDING 2

#endif
