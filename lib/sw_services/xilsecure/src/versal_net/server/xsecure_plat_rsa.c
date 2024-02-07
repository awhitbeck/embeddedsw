/******************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_plat_rsa.c
* This file contains versalnet specific code for xilsecure rsa server.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.2   kpt     06/23/23 Initial release
* 5.3   am      09/28/23 Added wrapper functions for IPCore's RSA APIs
*       dd      10/11/23 MISRA-C violation Rule 10.3 fixed
*       dd      10/11/23 MISRA-C violation Rule 12.1 fixed
*       dd      10/11/23 MISRA-C violation Rule 8.13 fixed
*       kpt     12/13/23 Added RSA CRT support for RSA keyunwrap
* 5.3   ng      01/28/24 Added SDT support
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#ifdef SDT
#include "xsecure_config.h"
#include "xplmi_bsp_config.h"
#endif

#include "xparameters.h"
#ifndef PLM_RSA_EXCLUDE

#include "xsecure_plat_rsa.h"
#include "xsecure_rsa.h"
#include "xsecure_sha.h"
#include "xsecure_plat.h"
#include "xsecure_error.h"

/************************** Constant Definitions *****************************/

#define XSECURE_RSA_MAX_MSG_SIZE_IN_BYTES  (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - \
						(2U * XSECURE_SHA3_HASH_LENGTH_IN_BYTES) - 2U) /**< RSA maximum message size in bytes */

#define XSECURE_RSA_MAX_PS_SIZE_IN_BYTES   (XSECURE_RSA_MAX_MSG_SIZE_IN_BYTES) /**< RSA maximum PS size in bytes */
#define XSECURE_RSA_MAX_DB_SIZE_IN_BYTES   (XSECURE_RSA_MAX_PS_SIZE_IN_BYTES  + \
						XSECURE_SHA3_HASH_LENGTH_IN_BYTES + 1U) /**< RSA maximum DB size in bytes */

/************************** Function Prototypes ******************************/

static int XSecure_RsaOaepEncode(XSecure_RsaOaepParam *OaepParam, u64 OutputAddr);
static int XSecure_RsaOaepDecode(XSecure_RsaOaepParam *OaepParam, u64 InputDataAddr);

/************************** Variable Definitions *****************************/

#if (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES == XSECURE_RSA_3072_KEY_SIZE)
static u8 Modulus[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES] = {
		0xA5, 0x1E, 0xB7, 0x46, 0x95, 0x2D, 0x4A, 0x19, 0x7C, 0xA9, 0x18, 0x44, 0xF3, 0xB6, 0xBB, 0xE1,
		0xAD, 0x63, 0x47, 0x38, 0x02, 0x6C, 0x81, 0xA5, 0x3C, 0x4A, 0x00, 0xFF, 0x97, 0x8F, 0xFC, 0x52,
		0xDD, 0x44, 0x61, 0x6E, 0x87, 0xC4, 0x31, 0x38, 0xFF, 0x38, 0xA5, 0xFE, 0x2A, 0xAA, 0x47, 0xE2,
		0xA3, 0xF5, 0xA9, 0x53, 0x66, 0xD5, 0xFA, 0xFA, 0x17, 0x5B, 0xBC, 0x5D, 0x49, 0xC3, 0xFD, 0x86,
		0xDA, 0xBD, 0xF5, 0x32, 0x2D, 0x63, 0x0B, 0xEC, 0x89, 0xC6, 0x31, 0x25, 0xFF, 0x0B, 0xF9, 0x5F,
		0x6A, 0x1F, 0x39, 0x26, 0x87, 0x29, 0xF6, 0x3B, 0xD7, 0x2A, 0xA8, 0x02, 0x94, 0xA6, 0x1E, 0x2C,
		0x62, 0x4C, 0x6F, 0x58, 0xD0, 0xE7, 0xB6, 0x7D, 0x1E, 0xCE, 0x65, 0xB0, 0x50, 0x05, 0x5F, 0xC1,
		0x3F, 0xC3, 0x57, 0x7E, 0x28, 0xEA, 0x8E, 0x79, 0x1B, 0xFD, 0xEA, 0x09, 0xF5, 0x18, 0x84, 0x08,
		0x9A, 0x38, 0x8A, 0x76, 0x76, 0x14, 0xD5, 0x3A, 0x28, 0xD6, 0xD9, 0xC2, 0x0B, 0x46, 0xEC, 0x9E,
		0xEE, 0xDA, 0x57, 0xBB, 0xF1, 0x30, 0x7B, 0x99, 0x61, 0x4C, 0x9A, 0x4F, 0x26, 0xFA, 0xB6, 0xE9,
		0xEC, 0x4C, 0xCE, 0xF7, 0x62, 0x1B, 0x9D, 0x6A, 0xC0, 0x9A, 0x65, 0x19, 0xDC, 0xF3, 0x4E, 0x21,
		0x09, 0x36, 0x39, 0x43, 0x3C, 0xFC, 0x7C, 0x38, 0x46, 0x11, 0x99, 0xC2, 0x7B, 0x2E, 0x09, 0x17,
		0xFB, 0x99, 0x6A, 0xBE, 0xE1, 0xFC, 0xDB, 0x4A, 0xDC, 0xFC, 0x84, 0x59, 0xA2, 0xA9, 0x16, 0xE1,
		0xCB, 0x20, 0x6E, 0x7F, 0x4F, 0xF1, 0x02, 0xBF, 0xD6, 0xBA, 0x1F, 0x63, 0xB2, 0x04, 0x43, 0xF1,
		0xD6, 0x26, 0x0E, 0x41, 0xCD, 0x51, 0x81, 0x81, 0xF9, 0x90, 0xD6, 0x69, 0xDA, 0x1D, 0xEC, 0x90,
		0x75, 0x8F, 0xA4, 0xB2, 0x9B, 0x22, 0x8E, 0x1B, 0xCD, 0x83, 0xAD, 0x57, 0x2A, 0xBC, 0x23, 0x29,
		0x73, 0x22, 0xE0, 0x5D, 0x8A, 0xF9, 0x79, 0xC4, 0x93, 0xF8, 0x8F, 0x5D, 0x6A, 0x87, 0x5D, 0xE3,
		0xB3, 0x8A, 0x70, 0x54, 0x4C, 0x46, 0x6C, 0xCA, 0x55, 0x8A, 0x7E, 0x07, 0x45, 0x7E, 0x45, 0x24,
		0x56, 0x40, 0x43, 0x6B, 0x4E, 0x32, 0x2F, 0x16, 0x9E, 0x65, 0x49, 0x77, 0x25, 0xFC, 0x62, 0x58,
		0xFE, 0x20, 0x7D, 0xBD, 0x63, 0xD6, 0x6F, 0x81, 0x92, 0x54, 0x22, 0x1B, 0xD4, 0x6B, 0xEC, 0x44,
		0xB8, 0x3F, 0x31, 0x00, 0x4A, 0xB3, 0xDA, 0x3E, 0x4E, 0x2A, 0xF7, 0x92, 0x42, 0x01, 0x45, 0x5F,
		0x14, 0x92, 0xA7, 0x99, 0xF8, 0xA5, 0x51, 0xB3, 0x30, 0x63, 0x55, 0x74, 0x62, 0xDE, 0x79, 0x6F,
		0xB9, 0xD9, 0x35, 0xBD, 0x85, 0xD3, 0xD3, 0x5F, 0xD5, 0x7F, 0x36, 0x8B, 0x0A, 0x82, 0x46, 0x98,
		0x46, 0xAE, 0x7A, 0xD2, 0x16, 0x3B, 0xCF, 0xA2, 0x2E, 0xB6, 0x98, 0x3D, 0x04, 0xC2, 0x10, 0xE5
};
#endif

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function encodes the given message using PKCS #1 v2.0
 *          RSA Optimal Asymmetric Encryption Padding scheme.
 *              EM = 0x00 || maskedSeed || maskedDB
 *
 * @param	OaepParam is pointer to the XSecure_RsaOaepParam instance.
 * @param	OutputAddr is address where the encoded data is stored.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Error code on failure.
 *
 ******************************************************************************/
static int XSecure_RsaOaepEncode(XSecure_RsaOaepParam *OaepParam, u64 OutputAddr)
{
	volatile int Status = XST_FAILURE;
	u8 Seed[XSECURE_SHA3_HASH_LENGTH_IN_BYTES] = {0U};
	u8 DB[XSECURE_RSA_MAX_DB_SIZE_IN_BYTES] = {0U};
	u8 DBMask[XSECURE_RSA_MAX_DB_SIZE_IN_BYTES] = {0U};
	u8 SeedMask[XSECURE_SHA3_HASH_LENGTH_IN_BYTES] = {0U};
	u32 Index = 0U;
	u32 DBLen = 0U;
	u32 DiffHashLen = 0U;
	u32 ActualMsgLen = 0U;
	XSecure_MgfInput MgfParam;
	const XSecure_HashAlgInfo *HashPtr = XSecure_GetHashInstance(OaepParam->ShaType);

	if (HashPtr == NULL) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	if ((OaepParam->InputDataAddr == 0x00U) || (OaepParam->OutputDataAddr == 0x00U)) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	/* Get the actual message length based on the hash algorithm */
	if (HashPtr->HashLen > XSECURE_SHA3_HASH_LENGTH_IN_BYTES) {
		DiffHashLen = (HashPtr->HashLen - XSECURE_SHA3_HASH_LENGTH_IN_BYTES);
	} else {
		DiffHashLen = (XSECURE_SHA3_HASH_LENGTH_IN_BYTES - HashPtr->HashLen);
	}

	ActualMsgLen = (XSECURE_RSA_MAX_MSG_SIZE_IN_BYTES - (DiffHashLen * 2U));
	if  (OaepParam->InputDataSize > ActualMsgLen) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_MSG_LEN;
		goto END;
	}

	Status = HashPtr->ShaDigest(OaepParam->ShaType, OaepParam->ShaInstancePtr, OaepParam->OptionalLabelAddr, OaepParam->OptionalLabelSize,
				       (u64)(UINTPTR)DB);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Index = ActualMsgLen - OaepParam->InputDataSize;
	DB[HashPtr->HashLen + Index] = 0x01U;

	XSecure_MemCpy64((u64)(UINTPTR)&DB[HashPtr->HashLen + Index + 1U], OaepParam->InputDataAddr, OaepParam->InputDataSize);

	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_GetRandomNum, Seed, HashPtr->HashLen);

	DBLen = Index + OaepParam->InputDataSize + HashPtr->HashLen + 1U;

	MgfParam.Seed = Seed;
	MgfParam.SeedLen = HashPtr->HashLen;
	MgfParam.Output = DBMask;
	MgfParam.OutputLen = DBLen;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_MaskGenFunc, OaepParam->ShaType, OaepParam->ShaInstancePtr, &MgfParam);
	for (Index = 0U; Index < DBLen;  Index++) {
		DB[Index] ^= DBMask[Index];
	}

	MgfParam.Seed = DB;
	MgfParam.SeedLen = DBLen;
	MgfParam.Output = SeedMask;
	MgfParam.OutputLen = HashPtr->HashLen;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_MaskGenFunc, OaepParam->ShaType, OaepParam->ShaInstancePtr, &MgfParam);
	for (Index = 0U; Index < HashPtr->HashLen; Index++) {
		Seed[Index] ^= 	SeedMask[Index];
	}

	/* Encode the message in to OutputAddr */
	XSecure_OutByte64(OutputAddr, 0x00U);
	XSecure_MemCpy64((OutputAddr + 1U), (u64)(UINTPTR)Seed, HashPtr->HashLen);
	XSecure_MemCpy64((OutputAddr + HashPtr->HashLen + 1U), (u64)(UINTPTR)DB, DBLen);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function decodes the given message which is encoded with
 *              RSA Optimal Asymmetric Encryption Padding scheme i.e.
 *              EM = 0x00 || maskedSeed || maskedDB
 *
 * @param	OaepParam is pointer to the XSecure_RsaOaepParam instance.
 * @param	InputDataAddr is the address where decrypted output data is stored.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Error code on failure.
 *
 ******************************************************************************/
static int XSecure_RsaOaepDecode(XSecure_RsaOaepParam *OaepParam, u64 InputDataAddr)
{
	volatile int Status = XST_FAILURE;
	u8 Hash[XSECURE_SHA3_HASH_LENGTH_IN_BYTES] = {0U};
	u8 DB[XSECURE_RSA_MAX_DB_SIZE_IN_BYTES] = {0U};
	u8 DBMask[XSECURE_RSA_MAX_DB_SIZE_IN_BYTES] = {0U};
	u8 SeedMask[XSECURE_SHA3_HASH_LENGTH_IN_BYTES] = {0U};
	u8 Seed[XSECURE_SHA3_HASH_LENGTH_IN_BYTES] = {0U};
	volatile u8 DBTmp = 0xFFU;
	u8 IsPsZero = 0x00U;
	u32 Index = 0U;
	u32 ActualMsgLen = 0U;
	XSecure_MgfInput MgfParam;
	const XSecure_HashAlgInfo *HashPtr = XSecure_GetHashInstance(OaepParam->ShaType);

	if (HashPtr == NULL) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	if (XSecure_InByte64(InputDataAddr) != 0x00U) {
		Status = (int)XSECURE_RSA_OAEP_BYTE_MISMATCH_ERROR;
		goto END;
	}

	Status = HashPtr->ShaDigest(OaepParam->ShaType, OaepParam->ShaInstancePtr, OaepParam->OptionalLabelAddr, OaepParam->OptionalLabelSize,
				       (u64)(UINTPTR)Hash);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_OAEP_DATA_CPY_ERROR;
		goto END;
	}

	/**< Split the message in to EM = 0X00 || SeedMask || DBMask */
	XSecure_MemCpy64((u64)(UINTPTR)SeedMask, (InputDataAddr + 1U), HashPtr->HashLen);
	XSecure_MemCpy64((u64)(UINTPTR)DBMask, (InputDataAddr + 1U + HashPtr->HashLen),
			 (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U));

	/**< Extract seed from SeedMask */
	MgfParam.Seed = DBMask;
	MgfParam.SeedLen = (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U);
	MgfParam.Output = Seed;
	MgfParam.OutputLen = HashPtr->HashLen;
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_MaskGenFunc, OaepParam->ShaType, OaepParam->ShaInstancePtr, &MgfParam);
	for (Index = 0U; Index < HashPtr->HashLen; Index++) {
		Seed[Index] ^= SeedMask[Index];
	}

	/**< Extract DB from DBMask */
	MgfParam.Seed = Seed;
	MgfParam.SeedLen = HashPtr->HashLen;
	MgfParam.Output = DB;
	MgfParam.OutputLen = (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U);
	XSECURE_TEMPORAL_CHECK(END, Status, XSecure_MaskGenFunc, OaepParam->ShaType, OaepParam->ShaInstancePtr, &MgfParam);
	for (Index = 0U; Index < (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U); Index++) {
		DB[Index] ^= DBMask[Index];
	}

	Status = Xil_SMemCmp(DB, HashPtr->HashLen, Hash, HashPtr->HashLen, HashPtr->HashLen);
	if (Status != XST_SUCCESS) {
		Status = (int)XSECURE_RSA_OAEP_DATA_CMP_ERROR;
		goto END;
	}

	Status = XST_FAILURE;
	Index = HashPtr->HashLen;
	for (; Index < (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U); Index++) {
		DBTmp = DB[Index];
		if ((DB[Index] == 0x0U) && (DBTmp == 0x00U)) {
			IsPsZero |= DB[Index];
		}
		else {
			break;
		}
	}

	if ((DB[Index] != 0x01U) || (IsPsZero != 0x00U)) {
		Status = (int)XSECURE_RSA_OAEP_DB_MISMATCH_ERROR;
		goto END;
	}

	Index = Index + 1U;
	ActualMsgLen = (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - HashPtr->HashLen - 1U) - Index;
	if (ActualMsgLen > (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - (2U * HashPtr->HashLen) - 2U)) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_MSG_LEN;
		goto END;
	}

	OaepParam->OutputDataSize = ActualMsgLen;
	XSecure_MemCpy64(OaepParam->OutputDataAddr, (u64)(UINTPTR)&DB[Index], ActualMsgLen);
	Status = XST_SUCCESS;
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function encodes the given message using RSA OAEP and encrypts it.
 *
 * @param	InstancePtr is pointer to the XSecure_Rsa instance.
 * @param	OaepParam is pointer to the XSecure_RsaOaepParam instance.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Error code on failure.
 *
 ******************************************************************************/
int XSecure_RsaOaepEncrypt(XSecure_Rsa *InstancePtr, XSecure_RsaOaepParam *OaepParam)
{
	volatile int Status = XST_FAILURE;
	u8 Output[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES] = {0U};

	if ((InstancePtr == NULL) || (OaepParam == NULL)) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}

	Status = XSecure_RsaOaepEncode(OaepParam, (u64)(UINTPTR)Output);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XSecure_RsaPublicEncrypt_64Bit(InstancePtr, (u64)(UINTPTR)Output, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES,
						OaepParam->OutputDataAddr);

END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function decodes the given message and decrypts it using RSA OAEP.
 *
 * @param	PrivKey is pointer to the XSecure_RsaKey instance.
 * @param	OaepParam is pointer to the XSecure_RsaOaepParam instance.
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- Error code on failure.
 *
 ******************************************************************************/
int XSecure_RsaOaepDecrypt(XSecure_RsaKey *PrivKey, XSecure_RsaOaepParam *OaepParam)
{
	volatile int Status = XST_FAILURE;
	u8 Output[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES] = {0U};
	u8 Data;
	u32 Index;

	if ((PrivKey == NULL) || (OaepParam == NULL)) {
		Status = (int)XSECURE_RSA_OAEP_INVALID_PARAM;
		goto END;
	}


	Status = XSecure_RsaExpCRT((u8*)(UINTPTR)OaepParam->InputDataAddr, PrivKey->P, PrivKey->Q, PrivKey->DP, PrivKey->DQ, PrivKey->QInv, NULL,
					PrivKey->Modulus, XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES * 8U, (u8*)(UINTPTR)Output);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	for (Index = 0U; Index < (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES/2U); Index++) {
		Data = Output[Index];
		Output[Index] = Output[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - Index - 1U];
		Output[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES - Index - 1U] = Data;
	}

	Status = XSecure_RsaOaepDecode(OaepParam, (u64)(UINTPTR)Output);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function gets RSA private key.
 *
 * @return
 *        - Pointer to RSA private key or NULL otherwise.
 *
 ******************************************************************************/
XSecure_RsaKey *XSecure_GetRsaPrivateKey(void)
{
	static XSecure_RsaKey RsaPrivKey = {0U};
#if (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES == XSECURE_RSA_3072_KEY_SIZE)
	static u8 P[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES/2U] = {
		0xE5, 0x15, 0x60, 0xAE, 0x8E, 0xAB, 0x44, 0x48, 0x9F, 0xD7, 0x54, 0x98, 0xA5, 0x6D, 0xD8, 0x8C,
		0x29, 0x85, 0x1E, 0x9E, 0x52, 0xDB, 0x04, 0xC1, 0x50, 0x17, 0x43, 0xFF, 0xE9, 0x5C, 0xFA, 0x9F,
		0x84, 0x9E, 0xAC, 0x8B, 0x95, 0xB3, 0x35, 0xD3, 0x7F, 0x06, 0x76, 0x6E, 0x45, 0x28, 0x47, 0x83,
		0x9F, 0x24, 0x98, 0x51, 0xF9, 0xFB, 0x18, 0xA1, 0xDE, 0x06, 0x65, 0x4C, 0xD0, 0xAE, 0xE5, 0xE8,
		0x82, 0x4C, 0x6A, 0x57, 0x64, 0xCC, 0xD5, 0xFE, 0x95, 0x8C, 0x2A, 0x36, 0x5C, 0x53, 0x1B, 0xCF,
		0xB6, 0x3A, 0x79, 0xB0, 0xFC, 0x1C, 0xE9, 0x39, 0x26, 0xCB, 0x41, 0x17, 0x8D, 0x42, 0x9A, 0x65,
		0x2D, 0x9E, 0xE3, 0xDC, 0x43, 0xA3, 0x5D, 0xE8, 0x4B, 0xC4, 0x50, 0xEA, 0xFE, 0x5E, 0x18, 0x64,
		0x91, 0x3F, 0xF6, 0x6C, 0x81, 0x6F, 0xF7, 0x02, 0xBC, 0xCE, 0x87, 0x5C, 0xA4, 0x2C, 0x22, 0xCC,
		0x68, 0x6F, 0xEF, 0x66, 0x48, 0x27, 0x88, 0xB0, 0xDD, 0xBD, 0x7A, 0xCD, 0xAE, 0x69, 0xD7, 0x74,
		0x2B, 0x71, 0x51, 0x58, 0xD5, 0x57, 0x22, 0x08, 0xA4, 0x9B, 0x5D, 0xD5, 0x79, 0x82, 0x77, 0x6B,
		0x86, 0x05, 0x41, 0xDA, 0x97, 0x82, 0x15, 0x93, 0x0A, 0x96, 0x90, 0xF0, 0xC5, 0xE2, 0x5F, 0x04,
		0x62, 0xA6, 0xB6, 0x79, 0xB7, 0xD6, 0x10, 0x47, 0x7E, 0xB8, 0x1F, 0x5E, 0x04, 0xC5, 0xF5, 0xF9};
	static u8 Q[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES/2U] = {
		0xC1, 0x59, 0x62, 0x7B, 0x4B, 0xDB, 0xB8, 0xA6, 0xC4, 0x65, 0x74, 0x4E, 0x0C, 0xAF, 0xDD, 0xF6,
		0x80, 0xF3, 0xD9, 0x55, 0xF5, 0x33, 0x86, 0x6E, 0x62, 0x0A, 0xF3, 0x76, 0x2B, 0xA8, 0x53, 0x15,
		0x59, 0x20, 0xDE, 0xE4, 0x10, 0xFD, 0x40, 0x9B, 0xD0, 0xA6, 0x91, 0xCC, 0x08, 0x42, 0xDE, 0x2F,
		0x7E, 0xCE, 0x23, 0xB8, 0x2B, 0xB6, 0x10, 0x2C, 0x3F, 0x64, 0x1E, 0x9F, 0x21, 0x42, 0xBF, 0xD4,
		0x02, 0xF5, 0xB5, 0x6F, 0xB1, 0xBD, 0xAB, 0x09, 0xF0, 0x13, 0xBF, 0xCC, 0x31, 0x0E, 0x7C, 0x8F,
		0x42, 0x78, 0x13, 0x1C, 0x7E, 0xF1, 0x5F, 0x1C, 0x37, 0x97, 0x20, 0xAC, 0x9B, 0xD8, 0xC5, 0x58,
		0xB7, 0xDD, 0x76, 0x5A, 0xBD, 0x9B, 0xEB, 0xBF, 0xB3, 0xE2, 0xFD, 0xD5, 0x27, 0x51, 0xE7, 0x34,
		0x22, 0x11, 0x21, 0x7A, 0x20, 0xCA, 0xF2, 0xE9, 0x1F, 0xD0, 0x63, 0xDA, 0xFA, 0xEE, 0xCF, 0x08,
		0x07, 0x5E, 0x3C, 0xD6, 0xA5, 0xDD, 0xAB, 0x9B, 0xC2, 0xFD, 0x7A, 0xE4, 0x8B, 0x39, 0xE2, 0x64,
		0x94, 0xB1, 0xF1, 0x88, 0x13, 0xB1, 0xF4, 0x47, 0xF3, 0x2D, 0x55, 0x9F, 0x24, 0x42, 0x5C, 0xFE,
		0xA9, 0x4B, 0x68, 0xF3, 0x46, 0x2D, 0xC6, 0xF0, 0x03, 0x85, 0x82, 0xD6, 0xCD, 0xC0, 0xC7, 0xF6,
		0x01, 0xDA, 0x1C, 0xEC, 0xA3, 0x73, 0x84, 0x33, 0x07, 0x69, 0x36, 0xFC, 0x7D, 0xBC, 0x99, 0xEA};
	static u8 DP[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES/2U] = {
		0x59, 0x65, 0x49, 0x6E, 0xA8, 0x50, 0xD4, 0x5B, 0x95, 0x91, 0x38, 0xDE, 0x48, 0x6F, 0xD1, 0x2C,
		0xC2, 0xD8, 0x5C, 0x84, 0x3C, 0xA1, 0xF4, 0x5C, 0xEF, 0x0C, 0x64, 0x72, 0xA4, 0xC7, 0x06, 0x86,
		0xE1, 0x44, 0x12, 0x08, 0x52, 0x19, 0x39, 0x2E, 0x64, 0x1D, 0x59, 0xA0, 0x4E, 0xC3, 0x7A, 0x50,
		0x03, 0x0F, 0xF1, 0x5C, 0x48, 0x75, 0x57, 0x17, 0x5C, 0x2B, 0xB5, 0x61, 0xE0, 0x0D, 0xE0, 0xD0,
		0x91, 0x01, 0xB2, 0x86, 0x18, 0x9B, 0x5D, 0x11, 0x70, 0xEC, 0x80, 0x5B, 0xC5, 0x77, 0x54, 0x97,
		0x90, 0x8A, 0xAB, 0xB4, 0x22, 0x73, 0x8C, 0xEA, 0xBF, 0xB6, 0x7B, 0x8D, 0x8A, 0x42, 0xC2, 0xEC,
		0xEA, 0x88, 0x9D, 0xCA, 0x82, 0x05, 0xFE, 0xAA, 0x56, 0x56, 0x8E, 0x87, 0xA9, 0x88, 0xFB, 0xB5,
		0x1D, 0x29, 0x84, 0xEE, 0xA0, 0x4D, 0xD6, 0x07, 0x62, 0xC0, 0xAE, 0x70, 0xBC, 0x15, 0x5B, 0x97,
		0xE4, 0x95, 0x53, 0x33, 0x4C, 0x7B, 0xE8, 0xE3, 0xB4, 0x95, 0x3D, 0xC4, 0x78, 0x12, 0xAF, 0x5A,
		0x43, 0x5D, 0x54, 0x7E, 0x29, 0x7D, 0x56, 0xB2, 0x7A, 0xBA, 0x5C, 0xFF, 0x6D, 0x8A, 0xA2, 0x89,
		0x36, 0x44, 0x30, 0x99, 0x4B, 0x4B, 0x8B, 0xCD, 0x9E, 0x2D, 0x7E, 0xD9, 0xB5, 0x78, 0xAB, 0x3F,
		0x4D, 0x92, 0xB1, 0x70, 0x83, 0xC1, 0x24, 0xB4, 0x42, 0xD1, 0xC4, 0xC7, 0xDF, 0x01, 0xD4, 0x98};
	static u8 DQ[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES/2U] = {
		0x81, 0x03, 0xED, 0x07, 0x3F, 0xB0, 0xC0, 0xF7, 0xAE, 0x34, 0x4F, 0xA2, 0x80, 0x18, 0x5E, 0x76,
		0x5E, 0xB3, 0x98, 0x63, 0xA9, 0xFC, 0x22, 0xD7, 0xFC, 0x9F, 0x01, 0x03, 0xFE, 0xF3, 0xC3, 0x1B,
		0xBD, 0xDD, 0xD1, 0x57, 0x2D, 0x85, 0x25, 0xC8, 0xD2, 0x87, 0x06, 0x14, 0xE4, 0xBC, 0xDF, 0x64,
		0x2A, 0xE6, 0x7A, 0x24, 0x7E, 0x99, 0xB8, 0xC2, 0x11, 0xAA, 0xBF, 0xDC, 0x26, 0x51, 0x2F, 0x6B,
		0x93, 0x1C, 0x1A, 0xF4, 0xAB, 0x3D, 0xF6, 0xCA, 0x49, 0xD7, 0x98, 0xB6, 0x81, 0xB9, 0xD1, 0x6B,
		0xC8, 0x64, 0xE4, 0xA8, 0x19, 0x1B, 0x16, 0x5C, 0x4C, 0x66, 0xA2, 0x6D, 0x4B, 0xE1, 0xC8, 0x3A,
		0x6A, 0x1C, 0x2A, 0x73, 0xB2, 0xD5, 0x0D, 0x39, 0x1C, 0x89, 0x0F, 0x3E, 0x8F, 0x66, 0xFE, 0x7D,
		0xA5, 0xF0, 0xA7, 0x4F, 0x1A, 0x2D, 0x88, 0x71, 0x2E, 0x38, 0x0A, 0xC8, 0x60, 0xF1, 0x06, 0x31,
		0x16, 0xAE, 0x5D, 0x49, 0xE6, 0x82, 0xCC, 0x3E, 0xD6, 0xC4, 0xE5, 0x16, 0x0E, 0x53, 0x25, 0x96,
		0x83, 0x5E, 0x2E, 0x05, 0x7E, 0xFD, 0x24, 0x1E, 0x70, 0x5B, 0x44, 0x49, 0x0C, 0xDF, 0x45, 0x6E,
		0x79, 0x77, 0x37, 0x6F, 0x49, 0x70, 0x9B, 0x13, 0x63, 0x4F, 0xD0, 0xE6, 0xF1, 0x49, 0xF5, 0xCF,
		0x7A, 0xAF, 0x73, 0x2D, 0xB8, 0x07, 0x8B, 0xEE, 0xB8, 0x62, 0x6F, 0x73, 0x8D, 0x11, 0xA6, 0x9A};
	static u8 QInv[XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES/2U] = {
		0x66, 0x3E, 0xF3, 0xB9, 0x77, 0xC9, 0xE9, 0x65, 0x83, 0x1B, 0xE8, 0xBB, 0x9B, 0xEB, 0x64, 0xBD,
		0xC8, 0xF2, 0x30, 0x06, 0xFA, 0x9F, 0x2E, 0x91, 0xB3, 0x12, 0xA0, 0x5A, 0x05, 0xD3, 0x7A, 0x01,
		0x6D, 0x71, 0x9E, 0xB6, 0xBD, 0xCB, 0x74, 0x93, 0x74, 0x3F, 0xE1, 0x89, 0xED, 0xD6, 0x3C, 0x3B,
		0xF6, 0xBB, 0x98, 0x57, 0x5E, 0x91, 0x09, 0xE1, 0xE2, 0x07, 0x6F, 0xFB, 0x34, 0xC5, 0x86, 0x67,
		0xB9, 0xB3, 0xDA, 0x62, 0x43, 0xF5, 0xC7, 0x3F, 0x38, 0xD6, 0x44, 0xB6, 0xAE, 0xDB, 0xE3, 0x92,
		0xA6, 0x2C, 0x90, 0x9C, 0x2D, 0xA0, 0x80, 0x9E, 0x4A, 0x57, 0x63, 0x24, 0x1B, 0x6F, 0x72, 0x0A,
		0x08, 0x00, 0x5A, 0x3C, 0xEE, 0xFC, 0x62, 0x23, 0x4A, 0x06, 0xA5, 0x16, 0x3A, 0x90, 0x61, 0x8C,
		0xB3, 0xC8, 0x7A, 0xA4, 0xED, 0xAE, 0xCA, 0x98, 0x31, 0xD4, 0xAD, 0xAB, 0x75, 0x30, 0xE1, 0x0C,
		0xCA, 0xF4, 0xA4, 0x31, 0x1E, 0x67, 0x02, 0x27, 0x7D, 0x05, 0x33, 0x1D, 0xF0, 0x54, 0x9D, 0x83,
		0x37, 0xC4, 0xBE, 0x43, 0x95, 0x01, 0x30, 0x0F, 0xF7, 0x43, 0x47, 0x33, 0xA8, 0xC4, 0xC2, 0xC9,
		0x7A, 0x82, 0x6E, 0x66, 0x11, 0xD6, 0x32, 0x7E, 0x51, 0xD9, 0xAB, 0xEA, 0xE0, 0x15, 0xCB, 0xC4,
		0x56, 0x92, 0x36, 0x99, 0xDC, 0xB0, 0x4C, 0x4B, 0x4A, 0x93, 0x26, 0xC3, 0x7E, 0xD6, 0x69, 0x85};

	RsaPrivKey.Modulus = Modulus;
	RsaPrivKey.P = P;
	RsaPrivKey.Q = Q;
	RsaPrivKey.DP = DP;
	RsaPrivKey.DQ = DQ;
	RsaPrivKey.QInv = QInv;
#endif

	return &RsaPrivKey;
}

/*****************************************************************************/
/**
 * @brief	This function gets RSA public key.
 *
 * @return
 *		- Pointer to RSA public key or NULL otherwise
 *
 ******************************************************************************/
XSecure_RsaPubKey* XSecure_GetRsaPublicKey(void)
{
	static u32 PublicExp = 0x1000100U;
	static XSecure_RsaPubKey RsaPubKey = {0U};

#if (XSECURE_RSA_KEY_GEN_SIZE_IN_BYTES == XSECURE_RSA_3072_KEY_SIZE)
	RsaPubKey.Modulus = Modulus;
	RsaPubKey.Exponent = (u8 *)&PublicExp;
#endif

	return &RsaPubKey;
}

/*****************************************************************************/
/**
 * @brief	This function performs the RSA exponentiation using the
 *              Chinese remainder theorem(CRT).
 *
 * @param	Hash - is the Hash of the exponentiation.
 * @param	P    - is first factor, a positive integer.
 * @param	Q    - is second factor, a positive integer.
 * @param	Dp   - is first factor's CRT exponent, a positive integer.
 * @param	Dq   - is second factor's CRT exponent, a positive integer.
 * @param	Qinv - is (first) CRT coefficient, a positive integer.
 * @param	Pub  - is the public exponent to protect against the fault insertions.
 * @param	Mod  - is the public modulus (p*q), if NULL, calculated internally.
 * @param	Len  - is length of the full-length integer in bits.
 * @param	Res  - is result of exponentiation r = (h^e) mod n.
 *
 * @return
 *		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 ******************************************************************************/
int XSecure_RsaExpCRT(unsigned char *Hash, unsigned char *P, unsigned char *Q,
	unsigned char *Dp, unsigned char *Dq, unsigned char *Qinv, unsigned char *Pub,
	unsigned char *Mod, int Len, unsigned char *Res)
{
	volatile int Status = XST_FAILURE;

	if ((Hash == NULL) || (P == NULL) || (Q == NULL) || (Dp == NULL) ||
		(Dq == NULL) || (Qinv == NULL) || (Res == NULL)) {
		Status = (int)XSECURE_RSA_EXPONENT_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 0U);

	Status = RSA_ExpCrtQ(Hash, P, Q, Dp, Dq, Qinv, Pub, Mod, Len, Res);

	/** Reset the RSA engine */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function perofrms the RSA exponentiation.
 *
 * @param	Hash - is Hash of the exponentiation.
 * @param	Exp  - is exponent, a positive integer.
 * @param	Mod  - is public modulus (p*q), if NULL, calculated internally.
 * @param	P    - is first factor, a positive integer.
 * @param	Q    - is second factor, a positive integer.
 * @param	Pub  - is public exponent to protect against the fault insertions.
 * @param	Tot  - is totient, a secret value equal to (p-1)*(q-1).
 * @param	Len  - is length of the full-length integer in bits.
 * @param	Res  - is result of exponentiation r = (h^e) mod n.
 *
 * @return
 *		- XST_SUCCESS on success.
 * 		- Error code on failure.
 *
 ******************************************************************************/
int XSecure_RsaExp(unsigned char *Hash, unsigned char *Exp, unsigned char *Mod,
	unsigned char *P, unsigned char *Q, unsigned char *Pub, unsigned char *Tot,
	int Len, unsigned char *Res)
{
	volatile int Status = XST_FAILURE;

	if ((Hash == NULL) || (Exp == NULL) || (Mod == NULL) || (Res == NULL)) {
		Status = (int)XSECURE_RSA_EXPONENT_INVALID_PARAM;
		goto END;
	}

	/** Release the RSA engine from reset */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 0U);

	Status = RSA_ExpQ(Hash, Exp, Mod, P, Q, Pub, Tot, Len, Res);

	/** Reset the RSA engine */
	XSecure_Out32(XSECURE_ECDSA_RSA_SOFT_RESET, 1U);

END:
	return Status;
}

#endif
