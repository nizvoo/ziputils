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
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef unix
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#else
# include <direct.h>
# include <io.h>
#endif

#include "zip.h"

#ifdef WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif



#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef WIN32
uLong filetime(f, tmzip, dt)
    TCHAR *f;                /* name of file to get info on */
    tm_zip *tmzip;             /* return value: access, modific. and creation times */
    uLong *dt;             /* dostime */
{
  int ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATA  ff32;

      hFind = FindFirstFile(f,&ff32);
      if (hFind != INVALID_HANDLE_VALUE)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  return ret;
}
#else
#ifdef unix
uLong filetime(f, tmzip, dt)
    TCHAR *f;               /* name of file to get info on */
    tm_zip *tmzip;         /* return value: access, modific. and creation times */
    uLong *dt;             /* dostime */
{
  int ret=0;
  struct stat s;        /* results of stat() */
  struct tm* filedate;
  time_t tm_t=0;

  if (_tcscmp(f,_T("-"))!=0)
  {
    char name[MAXFILENAME+1];
    int len = strlen(f);
    if (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesnt append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    if (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    if (stat(name,&s)==0)
    {
      tm_t = s.st_mtime;
      ret = 1;
    }
  }
  filedate = localtime(&tm_t);

  tmzip->tm_sec  = filedate->tm_sec;
  tmzip->tm_min  = filedate->tm_min;
  tmzip->tm_hour = filedate->tm_hour;
  tmzip->tm_mday = filedate->tm_mday;
  tmzip->tm_mon  = filedate->tm_mon ;
  tmzip->tm_year = filedate->tm_year;

  return ret;
}
#else
uLong filetime(f, tmzip, dt)
    TCHAR *f;                /* name of file to get info on */
    tm_zip *tmzip;             /* return value: access, modific. and creation times */
    uLong *dt;             /* dostime */
{
    return 0;
}
#endif
#endif




int check_exist_file(filename)
    const TCHAR* filename;
{
    FILE* ftestexist;
    int ret = 1;
    ftestexist = _tfopen(filename,_T("rb"));
    if (ftestexist==NULL)
        ret = 0;
    else
        fclose(ftestexist);
    return ret;
}

static const wchar_t*
GetFileName(const wchar_t* _name)
{
	// strip off the leading directory names
	const wchar_t* file_name = _name;
	const wchar_t* orig_name = _name;
	while( *orig_name != L'\0' ) {
		if( (*orig_name) == L'/' || (*orig_name) == L'\\' ) {
			file_name = orig_name+1;
		}
		++orig_name;
	}
	return file_name;
}

/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
int getFileCrc(const TCHAR* filenameinzip,void*buf,unsigned long size_buf,unsigned long* result_crc)
{
   unsigned long calculate_crc=0;
   int err=ZIP_OK;
   FILE * fin = _tfopen(filenameinzip,_T("rb"));
   unsigned long size_read = 0;
   unsigned long total_read = 0;
   if (fin==NULL)
   {
       err = ZIP_ERRNO;
   }

    if (err == ZIP_OK)
        do
        {
            err = ZIP_OK;
            size_read = (int)fread(buf,1,size_buf,fin);
            if (size_read < size_buf)
                if (feof(fin)==0)
            {
                printf("error in reading %ls\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            if (size_read>0)
                calculate_crc = crc32(calculate_crc,buf,size_read);
            total_read += size_read;

        } while ((err == ZIP_OK) && (size_read>0));

    if (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %x\n",filenameinzip,calculate_crc);
    return err;
}




int zipfile(const TCHAR* zipfilename,  int count, const TCHAR* filenames[], const TCHAR* base_path)
{
  int i = 0;
  int opt_overwrite=1;
  int opt_compress_level=Z_DEFAULT_COMPRESSION;
  int zipok;
  int err=0;
  int size_buf=0;
  void* buf=NULL;
  const char* password=NULL;

  TCHAR filename[MAX_PATH];
  TCHAR only_filename[MAX_PATH];

  size_buf = WRITEBUFFERSIZE;
  buf = (void*)malloc(size_buf);
  if (buf==NULL) {
    printf("Error allocating memory\n");
    return ZIP_INTERNALERROR;
  }
  zipok = 1;
  if (zipok) {


    zipFile zf;
    int errclose;
    

#ifdef USEWIN32IOAPI
    zlib_filefunc_def ffunc;
    fill_win32_filefunc(&ffunc);
    zf = zipOpen2(zipfilename,(opt_overwrite==2) ? 2 : 0,NULL,&ffunc);
#else

    zf = zipOpen(zipfilename,(opt_overwrite==2) ? 2 : 0);
#endif

    if (zf == NULL)  {
      printf("error opening %ls\n",zipfilename);
      err= ZIP_ERRNO;
    } else
      printf("creating %ls\n",zipfilename);


  for (i = 0; i < count; ++i) {

      FILE * fin;
      int size_read;
      size_t start_pos = -1;
      int len       = 0;


      zip_fileinfo zi;
      unsigned long crcFile=0;

      _sntprintf(filename, MAX_PATH, _T("%s\0"), filenames[i]);


      //_sntprintf(only_filename, MAX_PATH, _T("%s\0"), GetFileName(filename));

      //remove the base name
      //ptr = _tcsstr(filename, base_path);
      //ptr = only_filename;
      //for (i = _tcslen(base_path); i < _tcslen(filename); ++i) {
        //*(ptr++) = filename[i];
      //}

      start_pos = _tcslen(base_path);

      memcpy(only_filename, filename + start_pos + 1,
                            (_tcslen(filename) - start_pos) * sizeof(TCHAR));

      //_sntprintf(only_filename, MAX_PATH, _T("%s\0"), ptr);
      

      zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
        zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
      zi.dosDate = 0;
      zi.internal_fa = 0;
      zi.external_fa = 0;
      filetime(filename,&zi.tmz_date,&zi.dosDate);


      if ((password != NULL) && (err==ZIP_OK))
        err = getFileCrc(filename,buf,size_buf,&crcFile);




      err = zipOpenNewFileInZip3(zf,only_filename,&zi,
        NULL,0,NULL,0,NULL /* comment*/,
        (opt_compress_level != 0) ? Z_DEFLATED : 0,
        opt_compress_level,0,
        /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
        -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
        password,crcFile);

      if (err != ZIP_OK)
        printf("error in opening %ls in zipfile\n",filename);
      else {
        fin = _tfopen(filename, _T("rb"));
        if (fin==NULL)
        {
          err=ZIP_ERRNO;
          printf("error in opening %ls for reading\n",filename);
        }
      }

      if (err == ZIP_OK)
        do {
          err = ZIP_OK;
          size_read = (int)fread(buf,1,size_buf,fin);
          if (size_read < size_buf)
            if (feof(fin)==0)
            {
              printf("error in reading %ls\n",filename);
              err = ZIP_ERRNO;
            }

            if (size_read>0)
            {
              err = zipWriteInFileInZip (zf,buf,size_read);
              if (err<0)
              {
                printf("error in writing %ls in the zipfile\n",
                  filename);
              }

            }
        } while ((err == ZIP_OK) && (size_read>0));

        if (fin)
          fclose(fin);

        if (err<0)
          err=ZIP_ERRNO;
        else
        {
          err = zipCloseFileInZip(zf);
          if (err!=ZIP_OK)
            printf("error in closing %ls in the zipfile\n",
            filename);
        }
    }

  errclose = zipClose(zf,NULL);
  if (errclose != ZIP_OK)
    printf("error in closing %s\n",zipfilename);

  }

  free(buf);
  return 0;
}
