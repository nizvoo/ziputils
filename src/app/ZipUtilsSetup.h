/*
 * (C) Copyright  AOE Studio 2009 - All Rights Reserved.
 *
 * This software is the confidential and proprietary information
 * of  AOE Studio ("Confidential Information"). You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with  AOE Studio
 *
 * nizvoo (AT) gmail.com
 */

#ifndef __ZIP_UTILS_SETUP_H__
#define __ZIP_UTILS_SETUP_H__

#ifndef __cplusplus
#define bool int
#define false 0
#define true  1
#endif


bool InitZipUtils(const wchar_t* path);
bool UnInitZipUtils();

int CompressFile(const wchar_t* filename, const wchar_t* zip_filename);
int CompressFolder(const wchar_t* path, const wchar_t* zip_filename);
int UnCompressFile(const wchar_t* filename, const wchar_t* extract_path);

#endif