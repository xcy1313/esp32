/**
 * @file encryptor.c
 * @brief 加密器，对数据进行加密解密
 * @author WatWu
 * @date 2018-06-04
 */

#include "encryptor.h"

static const uint8_t AES_KEY[] = {0x6c, 0x6c, 0x77, 0x61, 0x6e, 0x74, 0x61, 0x65, 0x73, 0x6b, 0x65, 0x79, 0x31, 0x2e, 0x30, 0x31};
static const uint8_t AES_IV[]  = {0x6c, 0x6c, 0x77, 0x61, 0x6e, 0x74, 0x61, 0x65, 0x73, 0x69, 0x76, 0x76, 0x31, 0x2e, 0x30, 0x31};

/**
 * [AES_Encrypt AES加密]
 * @param  inDate  [待加密的数据]
 * @param  inLen   [待加密数据的长度]
 * @param  outData [加密后的数据]
 * @return         [加密后的数据长度，为0则加密失败]
 */
int AES_Encrypt(uint8_t* inDate, int inLen, uint8_t** outData)
{
	int outLen;
	if(inLen % 16)
		outLen = inLen + 16 - inLen % 16;
	else
		outLen = inLen;

	//以输出数据的长度为准，16的倍数
	uint8_t* output = malloc(outLen);
	if(NULL == output)
	{
		return 0;
	}

	memset(output, 0, outLen);
	memcpy(output, inDate, inLen);

	AES128_CBC_encrypt_buffer(output, output, outLen, AES_KEY, AES_IV);

	*outData = output;

	return outLen;
}

/**
 * [AES_Decrypt AES解密]
 * @param  inDate  [待解密的数据]
 * @param  inLen   [待解密数据的长度]
 * @param  outData [解密后的数据]
 * @return         [解密后的数据长度，为0则解密失败]
 */
int AES_Decrypt(uint8_t* inDate, int inLen, uint8_t** outData)
{
	int outLen;
	if(inLen % 16)
		outLen = inLen + 16 - inLen % 16;
	else
		outLen = inLen;

	//以输出数据的长度为准，16的倍数
	uint8_t* input = malloc(outLen);
	if(NULL == input)
		return 0;
	uint8_t* output = malloc(outLen);
	if(NULL == output)
	{
		free(input);
		return 0;
	}

	memset(input, 0, outLen);
	memset(output, 0, outLen);

	memcpy(input, inDate, inLen);

	AES128_CBC_decrypt_buffer(output, input, outLen, AES_KEY, AES_IV);

	*outData = output;

	free(input);

	return outLen;
}
