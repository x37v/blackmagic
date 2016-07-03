/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2016  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Provides an abstract 'target object', the 'methods' of which must be
 * implemented by a target driver when a supported device is detected.
 */

#ifndef __TARGET_H
#define __TARGET_H

#include <stdarg.h>

typedef struct target_s target;
typedef uint32_t target_addr;

int adiv5_swdp_scan(void);
int jtag_scan(const uint8_t *lrlens);

bool target_foreach(void (*cb)(int i, target *t, void *context), void *context);
void target_list_free(void);

enum target_open_flags {
	TARGET_O_RDONLY = 0,
	TARGET_O_WRONLY = 1,
	TARGET_O_RDWR = 2,
	TARGET_O_APPEND = 0x008,
	TARGET_O_CREAT = 0x200,
	TARGET_O_TRUNC = 0x400,
};

enum target_seek_flag {
	TARGET_SEEK_SET = 0,
	TARGET_SEEK_CUR = 1,
	TARGET_SEEK_END = 2,
};

struct target_controller {
	void (*destroy_callback)(struct target_controller *, target *t);
	void (*printf)(struct target_controller *, const char *fmt, va_list);

	/* Interface to host system calls */
	int (*open)(struct target_controller *,
	            target_addr path, unsigned path_len,
	            enum target_open_flags flags, mode_t mode);
	int (*close)(struct target_controller *, int fd);
	int (*read)(struct target_controller *,
	            int fd, target_addr buf, unsigned int count);
	int (*write)(struct target_controller *,
	             int fd, target_addr buf, unsigned int count);
	long (*lseek)(struct target_controller *,
	              int fd, long offset, enum target_seek_flag flag);
	int (*rename)(struct target_controller *,
	              target_addr oldpath, unsigned old_len,
	              target_addr newpath, unsigned new_len);
	int (*unlink)(struct target_controller *,
	              target_addr path, unsigned path_len);
	int (*stat)(struct target_controller *,
	            target_addr path, unsigned path_len, target_addr buf);
	int (*fstat)(struct target_controller *, int fd, target_addr buf);
	int (*gettimeofday)(struct target_controller *,
	                    target_addr tv, target_addr tz);
	int (*isatty)(struct target_controller *, int fd);
	int (*system)(struct target_controller *,
	              target_addr cmd, unsigned cmd_len);
	int errno_;
	bool interrupted;
};

/* Halt/resume functions */
target *target_attach(target *t, struct target_controller *);
target *target_attach_n(int n, struct target_controller *);
void target_detach(target *t);
bool target_check_error(target *t);
bool target_attached(target *t);

/* Memory access functions */
void target_mem_read(target *t, void *dest, target_addr src, size_t len);
void target_mem_write(target *t, target_addr dest, const void *src, size_t len);

/* Register access functions */
void target_regs_read(target *t, void *data);
void target_regs_write(target *t, const void *data);

/* Halt/resume functions */
void target_reset(target *t);
void target_halt_request(target *t);
int target_halt_wait(target *t);
void target_halt_resume(target *t, bool step);

/* Break-/watchpoint functions */
int target_set_hw_bp(target *t, target_addr addr, uint8_t len);
int target_clear_hw_bp(target *t, target_addr addr, uint8_t len);

int target_set_hw_wp(target *t, uint8_t type, target_addr addr, uint8_t len);
int target_clear_hw_wp(target *t, uint8_t type, target_addr addr, uint8_t len);
int target_check_hw_wp(target *t, target_addr *addr);

/* Flash memory access functions */
int target_flash_erase(target *t, target_addr addr, size_t len);
int target_flash_write(target *t, target_addr dest, const void *src, size_t len);
int target_flash_done(target *t);

/* Accessor functions */
int target_regs_size(target *t);
const char *target_tdesc(target *t);
const char *target_mem_map(target *t);
const char *target_driver_name(target *t);

/* Command interpreter */
void target_command_help(target *t);
int target_command(target *t, int argc, const char *argv[]);

#endif

