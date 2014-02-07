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

#include <windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "ZipUtilsSetup.h"

int main(int argc, char* argv[])
{
  TCHAR path[MAX_PATH];
  TCHAR filename[MAX_PATH];
  TCHAR zip_filename[MAX_PATH];
  TCHAR unzip_path[MAX_PATH];
  size_t i;
  size_t len;

  GetModuleFileName(0, path, MAX_PATH);
  len = _tcslen(path);

  for (i = len - 1; i >= 0; --i) {
    if (path[i] == _T('\\') || path[i] == _T('/')) {
      path[i] = _T('\0');
      break;
    }
  }
  
  printf("%ls\n", path);

  if (!InitZipUtils(path))
    printf("Initilization the zip dynamic library failed.\n");
  else
    printf("Initilization the zip dynamic library success.\n");


  //file
  _stprintf(filename,    _T("%s/ZipUtilsApp.c"), path);
  _stprintf(zip_filename,_T("%s/ZipUtilsApp.zip"), path);


  CompressFile(filename, zip_filename);

  //folder
  _stprintf(zip_filename, _T("%s.zip"), path);
  CompressFolder(path, zip_filename);

  //unzip
  _stprintf(unzip_path, _T("%s/unzip"), path);
  UnCompressFile(zip_filename, unzip_path);


  UnInitZipUtils();

  getch();

  return 0;
}