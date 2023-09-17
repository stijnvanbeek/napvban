/*
 *  This file is part of vban.
 *  Copyright (c) 2015 by Benoît Quiniou <quiniouben@yahoo.fr>
 *
 *  vban is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  vban is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with vban.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VBAN_H__
#define __VBAN_H__

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VBAN_HEADER_SIZE            (4 + 4 + 16 + 4)
#define VBAN_HEADER_FOURC           'NABV'
#define VBAN_STREAM_NAME_SIZE       16
#define VBAN_PROTOCOL_MAX_SIZE      1464
#define VBAN_DATA_MAX_SIZE          (VBAN_PROTOCOL_MAX_SIZE - VBAN_HEADER_SIZE)
#define VBAN_CHANNELS_MAX_NB        256
#define VBAN_SAMPLES_MAX_NB         256

#ifdef _MSC_VER
#pragma pack(push,1)
struct VBanHeader
#else
struct VBanHeader
#endif
{
	uint32_t    vban;                               /* contains 'V' 'B', 'A', 'N' */
	uint8_t     format_SR;                          /* SR index (see SRList above) */
	uint8_t     format_nbs;                         /* nb sample per frame (1 to 256) */
	uint8_t     format_nbc;                         /* nb channel (1 to 256) */
	uint8_t     format_bit;                         /* mask = 0x07 (nb Byte integer from 1 to 4) */
	char        streamname[VBAN_STREAM_NAME_SIZE];  /* stream name */
	uint32_t    nuFrame;                            /* growing frame number. */
#ifndef _MSC_VER
} __attribute__((packed));
#else
};
#pragma pack(pop)
#endif


#define VBAN_SR_MASK                0x1F
#define VBAN_SR_MAXNUMBER           21
static long const VBanSRList[VBAN_SR_MAXNUMBER]=
{
    6000, 12000, 24000, 48000, 96000, 192000, 384000,
    8000, 16000, 32000, 64000, 128000, 256000, 512000,
    11025, 22050, 44100, 88200, 176400, 352800, 705600
};

#define VBAN_PROTOCOL_MASK          0xE0
enum VBanProtocol
{
    VBAN_PROTOCOL_AUDIO         =   0x00,
    VBAN_PROTOCOL_SERIAL        =   0x20,
    VBAN_PROTOCOL_TXT           =   0x40,
    VBAN_PROTOCOL_UNDEFINED_1   =   0x80,
    VBAN_PROTOCOL_UNDEFINED_2   =   0xA0,
    VBAN_PROTOCOL_UNDEFINED_3   =   0xC0,
    VBAN_PROTOCOL_UNDEFINED_4   =   0xE0
};

#define VBAN_BIT_RESOLUTION_MASK    0x07
enum VBanBitResolution
{
    VBAN_BITFMT_8_INT = 0,
    VBAN_BITFMT_16_INT,
    VBAN_BITFMT_24_INT,
    VBAN_BITFMT_32_INT,
    VBAN_BITFMT_32_FLOAT,
    VBAN_BITFMT_64_FLOAT,
    VBAN_BITFMT_12_INT,
    VBAN_BITFMT_10_INT,
    VBAN_BIT_RESOLUTION_MAX
};

static int const VBanBitResolutionSize[VBAN_BIT_RESOLUTION_MAX] =
{
    1, 2, 3, 4, 4, 8,
};

#define VBAN_RESERVED_MASK          0x08

#define VBAN_CODEC_MASK             0xF0
enum VBanCodec
{
    VBAN_CODEC_PCM              =   0x00,
    VBAN_CODEC_VBCA             =   0x10,
    VBAN_CODEC_VBCV             =   0x20,
    VBAN_CODEC_UNDEFINED_3      =   0x30,
    VBAN_CODEC_UNDEFINED_4      =   0x40,
    VBAN_CODEC_UNDEFINED_5      =   0x50,
    VBAN_CODEC_UNDEFINED_6      =   0x60,
    VBAN_CODEC_UNDEFINED_7      =   0x70,
    VBAN_CODEC_UNDEFINED_8      =   0x80,
    VBAN_CODEC_UNDEFINED_9      =   0x90,
    VBAN_CODEC_UNDEFINED_10     =   0xA0,
    VBAN_CODEC_UNDEFINED_11     =   0xB0,
    VBAN_CODEC_UNDEFINED_12     =   0xC0,
    VBAN_CODEC_UNDEFINED_13     =   0xD0,
    VBAN_CODEC_UNDEFINED_14     =   0xE0,
    VBAN_CODEC_USER             =   0xF0
};

#ifdef __cplusplus
}
#endif

#endif /*__VBAN_H__*/




