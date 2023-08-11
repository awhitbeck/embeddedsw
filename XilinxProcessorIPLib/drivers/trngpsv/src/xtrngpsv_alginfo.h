/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xtrngpsv_alginfo.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.4   mmd     07/04/23 Initial Release
*       dd      08/07/23 Updated doxygen comments
* </pre>
*
******************************************************************************/

#ifndef XTRNGPSV_ALGINFO_H
#define XTRNGPSV_ALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_cryptoalginfo.h"

/**************************** Constant Definitions ****************************/
#define XTRNGPSV_MAJOR_VERSION	1 /**< Major version of Trngpsv driver */
#define XTRNGPSV_MINOR_VERSION	4 /**< Minor version of Trngpsv driver */

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * This function returns the TRNG crypto algorithm information.
 *
 * @param	AlgInfo  Pointer to memory for holding the crypto algorithm information
 *
 ******************************************************************************/
static __attribute__((always_inline)) inline
void XTrngpsv_GetCryptoAlgInfo (Xil_CryptoAlgInfo *AlgInfo)
{
	AlgInfo->Version = XIL_BUILD_VERSION(XTRNGPSV_MAJOR_VERSION, XTRNGPSV_MINOR_VERSION);
	AlgInfo->NistStatus = NIST_COMPLIANT;
}

#ifdef __cplusplus
}
#endif

#endif /* XTRNGPSV_AESALGINFO_H */
