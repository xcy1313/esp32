#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include "aes.h"

/**
 * [AES_Encrypt AES加密]
 * @param  inDate  [待加密的数据]
 * @param  inLen   [待加密数据的长度]
 * @param  outData [加密后的数据]
 * @return         [加密后的数据长度，为0则加密失败]
 */
int AES_Encrypt(uint8_t* inDate, int inLen, uint8_t** outData);

/**
 * [AES_Decrypt AES解密]
 * @param  inDate  [待解密的数据]
 * @param  inLen   [待解密数据的长度]
 * @param  outData [解密后的数据]
 * @return         [解密后的数据长度，为0则解密失败]
 */
int AES_Decrypt(uint8_t* inDate, int inLen, uint8_t** outData);

#endif
