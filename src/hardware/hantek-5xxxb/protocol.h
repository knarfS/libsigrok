/*
 * This file is part of the libsigrok project.
 *
 * This driver is based on the protocol description made by tinman,
 * from the mikrocontroller.net and eevblog.com forums:
 * https://www.mikrocontroller.net/articles/Hantek_Protokoll
 * https://elinux.org/Das_Oszi_Protocol
 *
 * Copyright (C) 2018-2021 Frank Stettner <frank-stettner@gmx.net>
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

#ifndef LIBSIGROK_HARDWARE_HANTEK_5XXXB_PROTOCOL_H
#define LIBSIGROK_HARDWARE_HANTEK_5XXXB_PROTOCOL_H

#include <stdint.h>
#include <glib.h>
#include <libsigrok/libsigrok.h>
#include "libsigrok-internal.h"

#define LOG_PREFIX "hantek-5xxxb"

#define HANTEK_5XXXB_USB_VENDOR             0x049f
#define HANTEK_5XXXB_USB_PRODUCT            0x505a
#define HANTEK_5XXXB_USB_INTERFACE          0
#define HANTEK_5XXXB_USB_EP_IN              0x82
#define HANTEK_5XXXB_USB_EP_OUT             0x01

#define HANTEK_5XXXB_USB_NORM_MSG           0x53
#define HANTEK_5XXXB_USB_DBG_MSG            0x43

#define HANTEK_5XXXB_CMD_ECHO               0x00
#define HANTEK_5XXXB_CMD_RD_SYSDATA         0x01
#define HANTEK_5XXXB_CMD_RD_SAMPLEDATA      0x02
#define HANTEK_5XXXB_CMD_SUB_RD_SAMPLEDATA  0x01 // TODO
#define HANTEK_5XXXB_CMD_RD_FILE            0x10
#define HANTEK_5XXXB_CMD_WR_SYSDATA         0x11
#define HANTEK_5XXXB_CMD_AQUISITION         0x12
#define HANTEK_5XXXB_CMD_SUB_AQUISITION     0x00 // TODO
#define HANTEK_5XXXB_CMD_LOCK               0x12
#define HANTEK_5XXXB_CMD_SUB_LOCK           0x01

/** 510 pixels */
#define HANTEK_5XXXB_NUM_VDIV               10.2
/** TODO */
#define HANTEK_5XXXB_NUM_VDIV_INT           11
/** 640 pixels */
#define HANTEK_5XXXB_NUM_HDIV_MENU_ON       16
/** 768 pixels */
#define HANTEK_5XXXB_NUM_HDIV_MENU_OFF      19.2
/** TODO */
#define HANTEK_5XXXB_NUM_HDIV_MENU_OFF_INT  20

enum states {
	IDLE,
	CAPTURE,
	STOPPING,
};

enum vertical_base {
	VB_2MV   = 0x00,
	VB_5MV   = 0x01,
	VB_10MV  = 0x02,
	VB_20MV  = 0x03,
	VB_50MV  = 0x04,
	VB_100MV = 0x05,
	VB_200MV = 0x06,
	VB_500MV = 0x07,
	VB_1V    = 0x08,
	VB_2V    = 0x09,
	VB_5V    = 0x0A,
};

/* [probe factor][][] */
static const uint64_t ch_vdiv[][11][2] = {
	/* Probe x1 */
	{
		{   2, 1000 },
		{   5, 1000 },
		{  10, 1000 },
		{  20, 1000 },
		{  50, 1000 },
		{ 100, 1000 },
		{ 200, 1000 },
		{ 500, 1000 },
		{   1, 1 },
		{   2, 1 },
		{   5, 1 },
	},
	/* Probe x10 */
	{
		{  20, 1000 },
		{  50, 1000 },
		{ 100, 1000 },
		{ 200, 1000 },
		{ 500, 1000 },
		{   1, 1 },
		{   2, 1 },
		{   5, 1 },
		{  10, 1 },
		{  20, 1 },
		{  50, 1 },
	},
	/* Probe x100 */
	{
		{ 200, 1000 },
		{ 500, 1000 },
		{   1, 1 },
		{   2, 1 },
		{   5, 1 },
		{  10, 1 },
		{  20, 1 },
		{  50, 1 },
		{ 100, 1 },
		{ 200, 1 },
		{ 500, 1 },
	},
	/* Probe x1000 */
	{
		{    2, 1 },
		{    5, 1 },
		{   10, 1 },
		{   20, 1 },
		{   50, 1 },
		{  100, 1 },
		{  200, 1 },
		{  500, 1 },
		{ 1000, 1 },
		{ 2000, 1 },
		{ 5000, 1 },
	},
};

static const char *ch_coupling[] = {
	"AC",  /* 0x00 */
	"DC",  /* 0x01 */
	"GND", /* 0x02 */
};

static const uint64_t probe_factor[] = {
	1,    /* 0x00 */
	10,   /* 0x01 */
	100,  /* 0x02 */
	1000, /* 0x03 */
};

static const uint64_t main_timebase[][2] = {
	/* nanoseconds */
	{ 200, 1000000000 }, /* 0x00 */
	{ 200, 1000000000 }, /* 0x01 */
	{ 200, 1000000000 }, /* 0x02 */
	{ 200, 1000000000 }, /* 0x03 */
	{ 200, 1000000000 }, /* 0x04 */
	{ 200, 1000000000 }, /* 0x05 */
	{ 200, 1000000000 }, /* 0x06 */
	{ 400, 1000000000 }, /* 0x07 */
	{ 800, 1000000000 }, /* 0x08 */
	/* microseonds */
	{   2, 1000000 },    /* 0x09 */
	{   4, 1000000 },    /* 0x0A */
	{   8, 1000000 },    /* 0x0B */
	{  20, 1000000 },    /* 0x0C */
	{  40, 1000000 },    /* 0x0D */
	{  80, 1000000 },    /* 0x0E */
	{ 200, 1000000 },    /* 0x0F */
	{ 400, 1000000 },    /* 0x10 */
	{ 800, 1000000 },    /* 0x11 */
	/* millisconds */
	{   2, 1000 },       /* 0x12 */
	{   4, 1000 },       /* 0x13 */
	{   8, 1000 },       /* 0x14 */
	{  20, 1000 },       /* 0x15 */
	{  40, 1000 },       /* 0x16 */
	{  80, 1000 },       /* 0x17 */
	{ 200, 1000 },       /* 0x18 */
	{ 400, 1000 },       /* 0x19 */
	{ 800, 1000 },       /* 0x1A */
	/* seconds */
	{  2, 1 },           /* 0x1B */
	{  4, 1 },           /* 0x1C */
	{  8, 1 },           /* 0x1D */
	{ 20, 1 },           /* 0x1E */
	{ 40, 1 },           /* 0x1F */
};

static const uint64_t win_timebase[][2] = {
	/* nanoseconds */
	{   2, 1000000000 }, /* 0x00 */
	{   4, 1000000000 }, /* 0x01 */
	{   8, 1000000000 }, /* 0x02 */
	{  20, 1000000000 }, /* 0x03 */
	{  40, 1000000000 }, /* 0x04 */
	{  80, 1000000000 }, /* 0x05 */
	{ 200, 1000000000 }, /* 0x06 */
	{ 400, 1000000000 }, /* 0x07 */
	{ 800, 1000000000 }, /* 0x08 */
	/* microseonds */
	{   2, 1000000 },    /* 0x09 */
	{   4, 1000000 },    /* 0x0A */
	{   8, 1000000 },    /* 0x0B */
	{  20, 1000000 },    /* 0x0C */
	{  40, 1000000 },    /* 0x0D */
	{  80, 1000000 },    /* 0x0E */
	{ 200, 1000000 },    /* 0x0F */
	{ 400, 1000000 },    /* 0x10 */
	{ 800, 1000000 },    /* 0x11 */
	/* millisconds */
	{   2, 1000 },       /* 0x12 */
	{   4, 1000 },       /* 0x13 */
	{   8, 1000 },       /* 0x14 */
	{  20, 1000 },       /* 0x15 */
	{  40, 1000 },       /* 0x16 */
	{  80, 1000 },       /* 0x17 */
	{ 200, 1000 },       /* 0x18 */
	{ 400, 1000 },       /* 0x19 */
	{ 800, 1000 },       /* 0x1A */
	/* seconds */
	{  2, 1 },           /* 0x1B */
	{  4, 1 },           /* 0x1C */
	{  8, 1 },           /* 0x1D */
	{ 20, 1 },           /* 0x1E */
	{ 40, 1 },           /* 0x1F */
};

/**
 * Maps the memory depth to sys_data store_depth value and to the index of
 * the sample_rate array.
 */
static const struct {
	uint64_t memory_depth;
	uint8_t sys_data_store_depth_map;
	size_t sample_rate_array_index_map;
} memory_depth_mapper[] = {
	/* 4k */
	{ (4 * 1024),        0x00, 0 },
	/* 40k */
	{ (40 * 1024),       0x04, 1 },
	/* 512k */
	{ (512 * 1024),      0x06, 2 },
	/* 1M */
	{ (1 * 1024 * 1024), 0x07, 3 },
	/* 20k is probably only valid for Tekway DST3xxxB models, but it's not used
	 * in this driver.*/
	/* { (20 * 1024),       0x02, 4 }, */
	/* 2M is on Handhelds, no idea what BM/BMV bench models are using for this,
	 * Tinman assumes it can be 0x08, but it's not used in this driver. */
	/* { (2 * 1024 * 1024), 0xFF, 5 }, */
};

static const char *trigger_source[] = {
	"CH1",     /* 0x00 */
	"CH2",     /* 0x01 */
	"Ext",     /* 0x02 */
	"Ext/5",   /* 0x03 */
	"AC Line", /* 0x04 */
};

static const char *trigger_slope[] = {
	"r",   /* 0x00 */
	"f",   /* 0x01 */
	"r+f", /* 0x02 */
};

/* [timebase][ch2][disp menu][memory depth] */
static const uint32_t sample_count[][2][2][4] = {
	/*  Ch1 On, Ch 2 Off                                               |    Ch1 On, Ch 2 On                                     */
	/*  Menu Off                     |   Menu On                       |    Menu Off                |   Menu On                 */
	/*    4k    40k    512k      1M        4k    40k    512k      1M          4k    40k    512k  1M       4k    40k    512k  1M */  /* Win TB */
	{{{  768,   768,    768,    768 }, {  640,   640,    640,    640 }}, {{  768,   768,    768, 0 }, {  640,   640,    640, 0 }}}, /* 2 ns   */
	{{{  768,   768,    768,    768 }, {  640,   640,    640,    640 }}, {{  768,   768,    768, 0 }, {  640,   640,    640, 0 }}}, /* 4 ns   */
	{{{  768,   768,    768,    768 }, {  640,   640,    640,    640 }}, {{  768,   768,    768, 0 }, {  640,   640,    640, 0 }}}, /* 8 ns   */
	{{{  768,   768,    768,    768 }, {  640,   640,    640,    640 }}, {{  768,   768,    768, 0 }, {  640,   640,    640, 0 }}}, /* 20 ns  */
	{{{  768,   768,    768,    768 }, {  640,   640,    640,    640 }}, {{  768,   768,    768, 0 }, {  640,   640,    640, 0 }}}, /* 40 ns  */
	{{{ 1536,   768,    768,    768 }, { 1280,   640,    640,    640 }}, {{  768,   768,    768, 0 }, {  640,   640,    640, 0 }}}, /* 80 ns  */
	{{{ 3072,  1536,   1536,   1536 }, { 2560,  1280,   1280,   1280 }}, {{ 1536,  1536,   1536, 0 }, { 1280,  1280,   1280, 0 }}}, /* 200 ns */
	{{{ 3072,  3072,   3072,   3072 }, { 2560,  2560,   2560,   2560 }}, {{ 3072,  3072,   3072, 0 }, { 2560,  2560,   2560, 0 }}}, /* 400 ns */
	{{{ 3072,  6144,   6144,   6144 }, { 2560,  5120,   5120,   5120 }}, {{ 3072,  3072,   3072, 0 }, { 2560,  2560,   2560, 0 }}}, /* 800 ns */
	{{{ 3840, 15360,  15360,  15360 }, { 3200, 12800,  12800,  12800 }}, {{ 3840,  7680,   7680, 0 }, { 3200,  6400,   6400, 0 }}}, /* 2 us   */
	{{{ 3840, 30720,  30720,  30720 }, { 3200, 25600,  25600,  25600 }}, {{ 3840, 15360,  15360, 0 }, { 3200, 12800,  12800, 0 }}}, /* 4 us   */
	{{{ 3840, 30720,  61440,  61440 }, { 3200, 25600,  51200,  51200 }}, {{ 3840, 30720,  30720, 0 }, { 3200, 25600,  25600, 0 }}}, /* 8 us   */
	{{{ 3840, 38400, 153600, 153600 }, { 3200, 32000, 128000, 128000 }}, {{ 3840, 38400,  76800, 0 }, { 3200, 32000,  64000, 0 }}}, /* 20 us  */
	{{{ 3840, 38400, 307200, 307200 }, { 3200, 32000, 256000, 256000 }}, {{ 3840, 38400, 153600, 0 }, { 3200, 32000, 128000, 0 }}}, /* 40 us  */
	{{{ 3840, 38400, 307200, 614400 }, { 3200, 32000, 256000, 512000 }}, {{ 3840, 38400, 307200, 0 }, { 3200, 32000, 256000, 0 }}}, /* 80 us  */
	{{{ 3840, 38400, 384000,      0 }, { 3200, 32000, 320000,      0 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 200 us */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 400 us */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 800 us */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 2 ms   */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 4 ms   */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 8 ms   */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 20 ms  */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 40 ms  */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 80 ms  */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 200 ms */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 400 ms */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 800 ms */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 2 s    */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 4 s    */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 8 s    */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 20 s   */
	{{{ 3840, 38400, 384000, 768000 }, { 3200, 32000, 320000, 640000 }}, {{ 3840, 38400, 384000, 0 }, { 3200, 32000, 320000, 0 }}}, /* 40 s   */
};

/* [timebase][ch2][memory depth] */
static const uint32_t sample_rate[][2][4] = {
	/* Ch1 On, Ch 2 Off                             |   Ch1 On, Ch 2 On                    */
	/*         4k        40k       512k         1M  |          4k        40k        512 1M */  /* Win TB */
	{{ 1000000000, 400000000, 400000000, 400000000 }, { 500000000, 200000000, 200000000, 0 }}, /* 2 ns   */
	{{ 1000000000, 400000000, 400000000, 400000000 }, { 500000000, 200000000, 200000000, 0 }}, /* 4 ns   */
	{{ 1000000000, 400000000, 400000000, 400000000 }, { 500000000, 200000000, 200000000, 0 }}, /* 8 ns   */
	{{  800000000, 400000000, 400000000, 400000000 }, { 400000000, 200000000, 200000000, 0 }}, /* 20 ns  */
	{{  800000000, 400000000, 400000000, 400000000 }, { 400000000, 200000000, 200000000, 0 }}, /* 40 ns  */
	{{  800000000, 400000000, 400000000, 400000000 }, { 400000000, 200000000, 200000000, 0 }}, /* 80 ns  */
	{{  800000000, 400000000, 400000000, 400000000 }, { 400000000, 200000000, 200000000, 0 }}, /* 200 ns */
	{{  400000000, 400000000, 400000000, 400000000 }, { 400000000, 200000000, 200000000, 0 }}, /* 400 ns */
	{{  200000000, 400000000, 400000000, 400000000 }, { 200000000, 200000000, 200000000, 0 }}, /* 800 ns */
	{{  100000000, 400000000, 400000000, 400000000 }, { 100000000, 200000000, 200000000, 0 }}, /* 2 us   */
	{{   50000000, 400000000, 400000000, 400000000 }, {  50000000, 200000000, 200000000, 0 }}, /* 4 us   */
	{{   25000000, 200000000, 400000000, 400000000 }, {  25000000, 200000000, 200000000, 0 }}, /* 8 us   */
	{{   10000000, 100000000, 400000000, 400000000 }, {  10000000, 100000000, 200000000, 0 }}, /* 20 us  */
	{{    5000000,  50000000, 400000000, 400000000 }, {   5000000,  50000000, 200000000, 0 }}, /* 40 us  */
	{{    2500000,  25000000, 200000000, 400000000 }, {   2500000,  25000000, 200000000, 0 }}, /* 80 us  */
	{{    1000000,  10000000, 100000000,         0 }, {   1000000,  10000000, 100000000, 0 }}, /* 200 us */
	{{     500000,   5000000,  50000000, 100000000 }, {    500000,   5000000,  50000000, 0 }}, /* 400 us */
	{{     250000,   2500000,  25000000,  50000000 }, {    250000,   2500000,  25000000, 0 }}, /* 800 us */
	{{     100000,   1000000,  10000000,  20000000 }, {    100000,   1000000,  10000000, 0 }}, /* 2 ms   */
	{{      50000,    500000,   5000000,  10000000 }, {     50000,    500000,   5000000, 0 }}, /* 4 ms   */
	{{      25000,    250000,   2500000,   5000000 }, {     25000,    250000,   2500000, 0 }}, /* 8 ms   */
	{{      10000,    100000,   1000000,   2000000 }, {     10000,    100000,   1000000, 0 }}, /* 20 ms  */
	{{       5000,     50000,    500000,   1000000 }, {      5000,     50000,    500000, 0 }}, /* 40 ms  */
	{{       2500,     25000,    250000,    500000 }, {      2500,     25000,    250000, 0 }}, /* 80 ms  */
	{{       1000,     10000,    100000,    200000 }, {      1000,     10000,    100000, 0 }}, /* 200 ms */
	{{        500,      5000,     50000,    100000 }, {       500,      5000,     50000, 0 }}, /* 400 ms */
	{{        250,      2500,     25000,     50000 }, {       250,      2500,     25000, 0 }}, /* 800 ms */
	{{        100,      1000,     10000,     20000 }, {       100,      1000,     10000, 0 }}, /* 2 s    */
	{{         50,       500,      5000,     10000 }, {        50,       500,      5000, 0 }}, /* 4 s    */
	{{         25,       250,      2500,      5000 }, {        25,       250,      2500, 0 }}, /* 8 s    */
	{{         10,       100,      1000,      2000 }, {        10,       100,      1000, 0 }}, /* 20 s   */
	{{          5,        50,       500,      1000 }, {         5,        50,       500, 0 }}, /* 40 s   */
};

/**
 * Structure for the vertical data of the SysDATA structure.
 *
 * NOTE: Don't change the order of the elements! This structure is
 * transmitted as is via USB.
 * Also __attribute__((packed)) is used to avoid padding!
 */
struct __attribute__((packed)) hantek_5xxxb_sys_data_vert_ch {
	/* Vertical Channel */
	uint8_t disp;
	uint8_t vb;
	uint8_t coup;
	uint8_t f20mhz;
	uint8_t fine;
	uint8_t probe;
	uint8_t rphase;
	uint8_t cnt_fine;
	int16_t pos;
};

/**
 * SysDATA structure for getting/setting data from/to the scope.
 *
 * NOTE: Don't change the order of the elements! This structure is
 * transmitted as is via USB.
 * Also __attribute__((packed)) is used to avoid padding!
 */
struct __attribute__((packed)) hantek_5xxxb_sys_data {
	/* Vertical Channel 1 + 2 */
	struct hantek_5xxxb_sys_data_vert_ch vert_ch[2];

	/* Trigger */
	uint8_t trig_state;
	uint8_t trig_type;
	uint8_t trig_src;
	uint8_t trig_mode;
	uint8_t trig_coup;
	int16_t trig_vpos;
	uint64_t trig_frequency;
	uint64_t trig_holdtime_min;
	uint64_t trig_holdtime_max;
	uint64_t trig_holdtime;
	uint8_t trig_edge_slope;
	uint8_t trig_video_neg;
	uint8_t trig_video_pal;
	uint8_t trig_video_syn;
	uint16_t trig_video_line;
	uint8_t trig_pulse_neg;
	uint8_t trig_pulse_when;
	uint64_t trig_pulse_time;
	uint8_t trig_slope_set;
	uint8_t trig_slope_win;
	uint8_t trig_slope_when;
	uint16_t trig_slope_v1;
	uint16_t trig_slope_v2;
	uint64_t trig_slope_time;

	uint8_t trig_swap_ch1_type;
	uint8_t trig_swap_ch1_mode;
	uint8_t trig_swap_ch1_coup;
	uint8_t trig_swap_ch1_edge_slope;
	uint8_t trig_swap_ch1_video_neg;
	uint8_t trig_swap_ch1_video_pal;
	uint8_t trig_swap_ch1_video_syn;
	uint16_t trig_swap_ch1_video_line;
	uint8_t trig_swap_ch1_pulse_neg;
	uint8_t trig_swap_ch1_pulse_when;
	uint64_t trig_swap_ch1_pulse_time;
	uint8_t trig_swap_ch1_slope_set;
	uint8_t trig_swap_ch1_slope_win;
	uint8_t trig_swap_ch1_slope_when;
	uint16_t trig_swap_ch1_slope_v1;
	uint16_t trig_swap_ch1_slope_v2;
	uint64_t trig_swap_ch1_slope_time;

	uint8_t trig_swap_ch2_type;
	uint8_t trig_swap_ch2_mode;
	uint8_t trig_swap_ch2_coup;
	uint8_t trig_swap_ch2_edge_slope;
	uint8_t trig_swap_ch2_video_neg;
	uint8_t trig_swap_ch2_video_pal;
	uint8_t trig_swap_ch2_video_syn;
	uint16_t trig_swap_ch2_video_line;
	uint8_t trig_swap_ch2_pulse_neg;
	uint8_t trig_swap_ch2_pulse_when;
	uint64_t trig_swap_ch2_pulse_time;
	uint8_t trig_swap_ch2_slope_set;
	uint8_t trig_swap_ch2_slope_win;
	uint8_t trig_swap_ch2_slope_when;
	uint16_t trig_swap_ch2_slope_v1;
	uint16_t trig_swap_ch2_slope_v2;
	uint64_t trig_swap_ch2_slope_time;

	uint8_t trig_overtime_neg;
	uint64_t trig_overtime_time;

	/* Horizontal */
	uint8_t horiz_tb;
	uint8_t horiz_win_tb;
	uint8_t horiz_win_state;
	int64_t horiz_trigtime; // TODO: == SR_CONF_HORIZ_TRIGGERPOS ???

	/* Math */
	uint8_t math_disp;
	uint8_t math_mode;
	uint8_t math_fft_src;
	uint8_t math_fft_win;
	uint8_t math_fft_factor;
	uint8_t math_fft_db;

	/* Display */
	uint8_t display_mode;
	uint8_t display_persist;
	uint8_t display_format;
	uint8_t display_contrast;
	uint8_t display_maxcontrast;
	uint8_t display_grid_kind;
	uint8_t display_grid_bright;
	uint8_t display_maxgrid_bright;

	/* Aquire. NOTE: Misspelling is also in the docs */
	uint8_t acqurie_mode;
	uint8_t acqurie_avg_cnt;
	uint8_t acqurie_type;
	uint8_t acqurie_store_depth;

	/* Measure */
	uint8_t measure_item1_src;
	uint8_t measure_item1;
	uint8_t measure_item2_src;
	uint8_t measure_item2;
	uint8_t measure_item3_src;
	uint8_t measure_item3;
	uint8_t measure_item4_src;
	uint8_t measure_item4;
	uint8_t measure_item5_src;
	uint8_t measure_item5;
	uint8_t measure_item6_src;
	uint8_t measure_item6;
	uint8_t measure_item7_src;
	uint8_t measure_item7;
	uint8_t measure_item8_src;
	uint8_t measure_item8;

	/* Control */
	uint8_t control_type;
	uint8_t control_menuid;
	uint8_t control_disp_menu;
	uint8_t control_mul_win;

	int16_t trig_swap_ch1_vpos;
	uint8_t trig_swap_ch1_overtime_neg;
	uint64_t trig_swap_ch1_overtime_time;
	int16_t trig_swap_ch2_vpos;
	uint8_t trig_swap_ch2_overtime_neg;
	uint64_t trig_swap_ch2_overtime_time;

	uint8_t math_fft_base;
	uint8_t math_fft_vrms;

	/*
	 * These are not used in the Voltcraft 1062D/3062C scopes, but in the
	 * handheld scopes Hantek DSO1202B/BV, DSO1102B/BV and DSO1062B/BV, but I
	 * can't verify this...
	 */
	/*
	uint8_t dmmctl_type;
	uint8_t dmmctl_a_ma_swi;
	uint8_t dmmctl_couple;
	uint8_t dmmctl_auto;
	uint8_t dmmctl_rel;
	uint8_t dmmctl_range_ohm;
	uint8_t dmmctl_range_ma;
	uint8_t dmmctl_range_volt;
	*/
};

struct dev_context {
	struct sr_sw_limits limits;
	int dev_state;
	GMutex rw_mutex;

	/* Oscilloscope settings. */
	struct hantek_5xxxb_sys_data *in_sys_data;
	struct hantek_5xxxb_sys_data *out_sys_data;
};

SR_PRIV uint64_t hantek_5xxxb_get_samplerate(
	const struct hantek_5xxxb_sys_data *sys_data);
SR_PRIV void hantek_5xxxb_set_timebase(const struct sr_dev_inst *sdi,
	int timebase_idx);

SR_PRIV uint64_t hantek_5xxxb_get_memory_depth_from_sys_data(
	uint8_t store_depth);
SR_PRIV uint8_t hantek_5xxxb_get_store_depth_from_memory_depth(
	uint64_t memory_depth);
SR_PRIV uint8_t hantek_5xxxb_get_store_depth_from_sample_rate_array_index(
	size_t sample_rate_array_index);
SR_PRIV size_t hantek_5xxxb_get_sample_rate_array_index_from_sys_data(
	uint8_t store_depth);

SR_PRIV float hantek_5xxxb_get_volts_per_div(const struct sr_dev_inst *sdi,
	int channel_idx);
SR_PRIV float hantek_5xxxb_get_value_from_vert_pos(
	const struct sr_dev_inst *sdi, int16_t pos, float vdiv, int channel_idx);
SR_PRIV int16_t hantek_5xxxb_get_vert_pos_from_value(
	const struct sr_dev_inst *sdi, float value, float vdiv, int channel_idx);

SR_PRIV int hantek_5xxxb_lock_panel(const struct sr_dev_inst *sdi,
	gboolean lock);
SR_PRIV int hantek_5xxxb_get_sys_data(const struct sr_dev_inst *sdi,
	struct hantek_5xxxb_sys_data *sys_data);
SR_PRIV int hantek_5xxxb_set_sys_data(const struct sr_dev_inst *sdi);
SR_PRIV int hantek_5xxxb_get_sample_data(const struct sr_dev_inst *sdi,
	int channel_idx);

SR_PRIV int hantek_5xxxb_receive_data(int fd, int revents, void *cb_data);

#endif
