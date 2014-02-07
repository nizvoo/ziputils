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

extern int zipfile  (const TCHAR* zipfilename, int count, const TCHAR* filenames[], const TCHAR* base_path);
extern int unzipfile(const TCHAR* zipfilename, const TCHAR* dirname               );


static
int CreateInstance()
{
  return 0;
}

static
int DestroyInstance()
{
  return 0;
}

static WIN32_FIND_DATA  ffd;
static wchar_t          mask[MAX_PATH];
static wchar_t          path[MAX_PATH];
static wchar_t          szDir[MAX_PATH];
static HANDLE           hFind       = 0;


static 
void
GetFilePath(const wchar_t* const _name, wchar_t *path)
{
	const wchar_t* orig_name = _name;
	wchar_t* path_name = path;
	wchar_t* last_slash = path;
  
  if( 0 == path )
		return;


	while( *orig_name != L'\0' ) {
		*path_name++ = *orig_name; // copy to path
		if( (*orig_name) == L'/' || (*orig_name) == L'\\' ) {
			last_slash = path_name;
		}

		++orig_name;
	}
	*last_slash = L'\0';
}

static
const TCHAR * GetFileExtName(const TCHAR * file_name)
{
  const TCHAR* ptr = _tcsstr(file_name, _T("."));

  return ++ptr;
}

static
void SearchFile(const TCHAR * path, const TCHAR * filter, TCHAR** list, int* index)
{
	WIN32_FIND_DATA find_data;
	HANDLE hFind;
	TCHAR search_path[_MAX_PATH]		= _T("");
	TCHAR next_path[_MAX_PATH]			= _T("");
	TCHAR file_name[_MAX_PATH]			= _T("");
  BOOL result;
	
	size_t last_index = _tcslen(path) - 1;

	if (path[last_index] == _T('\\') || path[last_index] == _T('/'))
		_stprintf(search_path, _T("%s*.*"), path);
	else
		_stprintf(search_path, _T("%s\\*.*"), path);

	hFind = FindFirstFile(search_path, &find_data);
	
  result = (hFind != INVALID_HANDLE_VALUE); 
	
	while (result)	{
    if (_tcscmp(find_data.cFileName, _T(".")) && _tcscmp(find_data.cFileName, _T(".."))) {
      if (path[last_index] == _T('\\') || path[last_index] == _T('/'))
        _stprintf(file_name, _T("%s%s"), path, find_data.cFileName);
      else
        _stprintf(file_name, _T("%s\\%s"), path, find_data.cFileName);

      if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        _stprintf(next_path, _T("%s\\"), file_name);
        SearchFile(next_path, filter, list, index);
      } else {
        if (_tcsicmp(filter, _T("*.*")) == 0) {
          TCHAR* ptr = malloc(sizeof(TCHAR) * MAX_PATH);
          _stprintf(ptr, _T("%s"), file_name);
          list[(*index)++] = ptr;
          

        } else {
          TCHAR ext_name[_MAX_PATH] = _T("");
          _stprintf(ext_name, _T("*.%s"), GetFileExtName(file_name));
          if (_tcsicmp(ext_name, filter) == 0) {
            TCHAR* ptr = malloc(sizeof(TCHAR) * MAX_PATH);
            _stprintf(ptr, _T("%s"), file_name);
            list[(*index)++] = ptr;
          }
        }
      }
    }

		result = FindNextFile(hFind, &find_data);
	}

	FindClose(hFind);

}



BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			CreateInstance();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			DestroyInstance();
			break;
    }
    return TRUE;
}


int  __stdcall CompressFile(const TCHAR* filename, const TCHAR* zip_filename)
{
  const TCHAR* filenames[] = {filename};
  TCHAR  path[MAX_PATH];
  size_t len = 0;
  size_t    i   = 0;


  _sntprintf(path, MAX_PATH, _T("%s\0"), filename);

  for (i = _tcslen(path) - 1; i >= 0; --i) {
    if (path[i] == _T('\\') || path[i] == _T('/')) {
      path[i] = 0;
      break;
     }
  }

  zipfile(zip_filename, 1, filenames, path);

	return 1;
}



int  __stdcall CompressFolder(const TCHAR* path, const TCHAR* zip_filename)
{
  int i;
  int status = 1;
  int index  = 0;

  TCHAR* list[0xefff];

  SearchFile(path, _T("*.*"), list, &index);

  if (index > 0)
    status = zipfile(zip_filename, index, list, path);

  //release the memory
  for (i = 0; i < index; ++i)
    free(list[i]);

	return (status == 0);
}

int  __stdcall UnCompressFile(const wchar_t* zip_filename, const wchar_t* extract_path)
{
  unzipfile(zip_filename, extract_path);

	return 1;
}