/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_keymgmt.h
* @addtogroup xil_ocpapis DeviceKeysMgmt APIs
* @{
*
* @cond xocp_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date         Changes
* ----- ---- ----------   -------------------------------------------------------
* 1.0   vns  07/08/2022   Initial release
*       vns  01/10/2023   Adds logic to generate the DEVA on subsystem based.
* 1.2   har  02/24/2023   Added macro XOCP_INVALID_USR_CFG_INDEX
*       vns  07/06/2023   Added DEVAK regenerate support and Data clear before shutdown
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XOCPKEYMGMT_SERVER_H
#define XOCPKEYMGMT_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xocp.h"
#include "xocp_common.h"
#include "xsecure_sha.h"
#include "xsecure_trng.h"

/************************** Constant Definitions *****************************/
#define XOCP_EFUSE_DEVICE_DNA_CACHE			(0xF1250020U)
#define XOCP_EFUSE_DEVICE_DNA_SIZE_WORDS		(4U)
#define XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES		(16U)
#define XOCP_CDI_SIZE_IN_BYTES				(48U)
#define XOCP_CDI_SIZE_IN_WORDS				(12U)
#define XOCP_DEVAK_GEN_TRNG_SEED_SIZE_IN_BYTES		(48U)
#define XOCP_TIMEOUT_MAX				(0x1FFFFU)

#define XOCP_PMC_GLOBAL_ZEROIZE_CTRL_ZEROIZE_MASK	(0x00000001U)
#define XOCP_PMC_GLOBAL_ZEROIZE_STATUS_PASS_MASK	(0x00000002U)
#define XOCP_PMC_GLOBAL_ZEROIZE_STATUS_DONE_MASK	(0x00000001U)

#define XOCP_MAX_DEVAK_SUPPORT				(3U)
#define XOCP_INVALID_DEVAK_INDEX			(0xFFFFFFFFU)
#define XOCP_INVALID_USR_CFG_INDEX			(0xFFFFFFFFU)

/* XilOCP Module Data Structure Ids*/
#define XOCP_DEVAK_SUBSYS_HASH_DS_ID		(1U)

/**************************** Type Definitions *******************************/
/**
 * OCP key management driver instance to store the states
 * A pointer to an instance data structure is passed around by functions
 * to refer to a specific driver instance.
 */
typedef struct {
	u32 IsDevKeyReady;	/**< Indicates device key is supported */
	u32 DevAkInputIndex;	/**< Points to the next empty dev AK */
} XOcp_KeyMgmt;

/**
 * DEVAK data storage for in place PLM update
 */
typedef struct {
	u32 SubSystemId;	/**< Corresponding Sub system ID */
	u8 SubSysHash[XSECURE_HASH_SIZE_IN_BYTES]; /**< Hash of the subsystem */
	u32 ValidData;		/**< Valid Data */
} XOcp_SubSysHash;

/**
 * DEV AK data structure
 */
typedef struct {
	u32 SubSystemId;	/**< Corresponding Sub system ID */
	u8 PerString[XSECURE_TRNG_PERS_STRING_LEN_IN_BYTES];/**< Personalised string */
	u8 SubSysHash[XSECURE_HASH_SIZE_IN_BYTES]; /**< Hash of the subsystem */
	u8 EccPrvtKey[XOCP_ECC_P384_SIZE_BYTES]; /**< ECC DEV AK private key */
	u8 EccX[XOCP_ECC_P384_SIZE_BYTES];	/**< ECC DEVAK publick key X */
	u8 EccY[XOCP_ECC_P384_SIZE_BYTES];	/**< ECC DEVAK publick key Y */
	u32 IsDevAkKeyReady; /**< Indicates Dev AK availability */
} XOcp_DevAkData;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
int XOcp_KeyInit(void);
int XOcp_DevAkInputStore(u32 SubSystemId, u8 *PerString);
u32 XOcp_GetSubSysReqDevAkIndex(u32 SubSystemId);
XOcp_DevAkData *XOcp_GetDevAkData(void);
int XOcp_GenerateDevAk(u32 SubSystemId);
int XOcp_GetX509Certificate(XOcp_X509Cert *XOcp_GetX509CertPtr, u32 SubSystemId);
int XOcp_AttestWithDevAk(XOcp_Attest *AttestWithDevAkPtr, u32 SubSystemId);
int XOcp_IsDevIkReady(void);
int XOcp_RegenSubSysDevAk(void);
int XOcp_DataZeroize(XPlmi_ModuleOp Op);
#ifdef __cplusplus
}
#endif

#endif  /* XOCPKEYMGMT_SERVER_H */
