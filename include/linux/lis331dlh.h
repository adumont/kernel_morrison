/*
 * Copyright (c) 2008-2009, Motorola, All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __LIS331DLH_H__
#define __LIS331DLH_H__

#include <linux/ioctl.h>  /* For IOCTL macros */
#include <linux/types.h> 

/** This define controls compilation of the master device interface */
/*#define LIS331DLH_MASTER_DEVICE*/

#ifndef CONFIG_MACH_MOT
#define LIS331DLH_IOCTL_BASE 77
#else
#define LIS331DLH_IOCTL_BASE 'g'
#endif
/** The following define the IOCTL command values via the ioctl macros */
#define LIS331DLH_IOCTL_SET_DELAY	_IOW(LIS331DLH_IOCTL_BASE, 0, int)
#define LIS331DLH_IOCTL_GET_DELAY	_IOR(LIS331DLH_IOCTL_BASE, 1, int)
#define LIS331DLH_IOCTL_SET_ENABLE	_IOW(LIS331DLH_IOCTL_BASE, 2, int)
#define LIS331DLH_IOCTL_GET_ENABLE	_IOR(LIS331DLH_IOCTL_BASE, 3, int)
#define LIS331DLH_IOCTL_SET_G_RANGE	_IOW(LIS331DLH_IOCTL_BASE, 4, int)
#define LIS331DLH_IOCTL_TRIG_ROTATION   _IOW(LIS331DLH_IOCTL_BASE, 11, int)

#define LIS331DLH_G_2G 0x00
#define LIS331DLH_G_4G 0x10
#define LIS331DLH_G_8G 0x30

#ifdef CONFIG_MACH_MOT
#define LIS331DLH_MODE_OFF    0
#define LIS331DLH_MODE_ACCEL  0x01
#define LIS331DLH_MODE_ROTATE 0x02
#define LIS331DLH_ENABLE 0x0100
#define LIS331DLH_DISABLE 0x0000
#endif /* CONFIG_MACH_MOT */

#ifdef __KERNEL__
struct lis331dlh_platform_data {
    int irq1;
    int irq2;
	int poll_interval;
	int min_interval;

	u8 g_range;

	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;

	u8 negate_x;
	u8 negate_y;
	u8 negate_z;

	int (*init)(void);
	void (*exit)(void);
	int (*power_on)(void);
	int (*power_off)(void);

};
#endif /* __KERNEL__ */

#endif  /* __LIS331DLH_H__ */

