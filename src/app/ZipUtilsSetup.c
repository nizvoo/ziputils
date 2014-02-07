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


#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>

#include "ZipUtilsSetup.h"

static HMODULE h;



typedef int (__stdcall *TCompressFile)    (const wchar_t* filename, const wchar_t* zip_filename);
typedef int (__stdcall *TCompressFolder)  (const wchar_t* path,     const wchar_t* zip_filename);
typedef int (__stdcall *TUnCompressFile)  (const wchar_t* filename, const wchar_t* path);

static TCompressFile    CompressFileProc   = 0;
static TCompressFolder  UnCompressFileProc = 0;
static TUnCompressFile  CompressFolderProc = 0;


bool InitZipUtils(const wchar_t* path)
{
  wchar_t filename[MAX_PATH + 1];
  filename[MAX_PATH] = 0;
  _sntprintf(filename, MAX_PATH, L"%s/ZipUtils.dll", path);

  h = LoadLibrary(filename);

 CompressFileProc   = (TCompressFile  )GetProcAddress(h, "CompressFile");
 UnCompressFileProc = (TUnCompressFile)GetProcAddress(h, "UnCompressFile");
 CompressFolderProc = (TCompressFolder)GetProcAddress(h, "CompressFolder");

 if (!CompressFileProc)    return false;
 if (!UnCompressFileProc)  return false;
 if (!CompressFolderProc)  return false;

 return true;
}

#define ERROR_NO (-9)

int CompressFile(const wchar_t* filename, const wchar_t* zip_filename)
{

  if (!CompressFileProc)
    return ERROR_NO;

  return CompressFileProc(filename, zip_filename);
}

int CompressFolder(const wchar_t* path, const wchar_t* zip_filename)
{

  if (!CompressFolderProc)
    return ERROR_NO;

  return CompressFolderProc(path, zip_filename);
}

int UnCompressFile(const wchar_t* filename, const wchar_t* extract_path)
{

  if (!UnCompressFileProc)
    return ERROR_NO;

  return UnCompressFileProc(filename, extract_path);
}

bool UnInitZipUtils()
{
  FreeLibrary(h);

  return true;
}