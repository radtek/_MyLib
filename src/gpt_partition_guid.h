﻿/**
 * @file    gpt_partition_guid.h
 * @brief   This file defines GUID for GPT partion type.
 *          refer https://msdn.microsoft.com/ko-kr/library/windows/desktop/aa365449(v=vs.85).aspx
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2015:12:31 08:12 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once
#include <guiddef.h>

static const GUID PARTITION_BASIC_DATA_GUID =
{
    0xebd0a0a2,
    0xb9e5,
    0x4433,
    { 0x87, 0xc0, 0x68, 0xb6, 0xb7, 0x26, 0x99, 0xc7 }
};

static const GUID PARTITION_ENTRY_UNUSED_GUID =
{
    0x00000000,
    0x0000,
    0x0000,
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

static const GUID PARTITION_SYSTEM_GUID =
{
    0xc12a7328,
    0xf81f,
    0x11d2,
    { 0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b }
};

static const GUID PARTITION_MSFT_RESERVED_GUID =
{
    0xe3c9e316,
    0x0b5c,
    0x4db8,
    { 0x81, 0x7d, 0xf9, 0x2d, 0xf0, 0x02, 0x15, 0xae }
};

static const GUID PARTITION_LDM_METADATA_GUID =
{
    0x5808c8aa,
    0x7e8f,
    0x42e0,
    { 0x85, 0xd2, 0xe1, 0xe9, 0x04, 0x34, 0xcf, 0xb3 }
};

static const GUID PARTITION_LDM_DATA_GUID =
{
    0xaf9b60a0,
    0x1431,
    0x4f62,
    { 0xbc, 0x68, 0x33, 0x11, 0x71, 0x4a, 0x69, 0xad }
};

static const GUID PARTITION_MSFT_RECOVERY_GUID =
{
    0xde94bba4,
    0x06d1,
    0x4d40,
    { 0xa1, 0x6a, 0xbf, 0xd5, 0x01, 0x79, 0xd6, 0xac }
};
