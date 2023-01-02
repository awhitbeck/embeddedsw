#/******************************************************************************
#* Copyright (c) 2018 - 2023 Xilinx, Inc.  All rights reserved.
#* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
#* SPDX-License-Identifier: MIT
#******************************************************************************/


PARAMETER VERSION = 2.2.0


BEGIN OS
 PARAMETER OS_NAME = standalone
 PARAMETER STDIN =  *
 PARAMETER STDOUT = *
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilffs
 PARAMETER READ_ONLY = true
 PARAMETER USE_MKFS = false
 PARAMETER WORD_ACCESS = false
 PARAMETER ENABLE_MULTI_PARTITION = true
 PARAMETER NUM_LOGICAL_VOL = 10
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilpdi
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilplmi
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilloader
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilpm
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilsecure
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilsem
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilpuf
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilnvm
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilocp
END