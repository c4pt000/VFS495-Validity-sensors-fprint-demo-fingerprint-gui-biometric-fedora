
/*! @file vfsWrapper.h
*******************************************************************************
* Helper functions for Validity driver interface functions
*
* This file contains the Helper functions for Validity driver interface functions
* and their definitions
*
* Copyright 2006 Validity Sensors, Inc. 

* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef __vfsWrapper_h__
#define __vfsWrapper_h__

int vfs_dev_init( void* pValidityDriver );

void vfs_dev_exit( void* pValidityDriver );

int vfs_capture( void* pValidityDriver,
   	            int initial);
int vfs_enroll( void* pValidityDriver,
   	            int initial,
   	            int stage,
	            unsigned char** ppFPrintData,
	            int *nFPrintDataSize );

int vfs_verify( void* pValidityDriver,
   	            unsigned char* pFPrintData,
	            int nFPrintDataSize );

int vfs_identify( void* pValDriver,
                  unsigned char **ppTemplatesData,
                  int *pTemplatesDataLengths,
                  int *pMatch_offset,
                  int nItems );

int vfs_get_img_width( void* pValidityContext );

int vfs_get_img_height( void* pValidityContext );

int vfs_get_img_datasize( void* pValidityContext );

unsigned char* vfs_get_img_data( void* pValidityContext );

void vfs_free_img_data( unsigned char* pImgData );

int vfs_get_matcher_type();

int vfs_set_matcher_type(int matcherType);  


typedef int (*PtrVfsDevInit)(void *);
typedef void (*PtrVfsCleanHandles)(void *);
typedef int (*PtrVfsWaitForService)(void);
typedef unsigned char* (*PtrVfsGetImgData)(void *);
typedef void (*PtrVfsFreeImgData)(unsigned char *);
typedef  int (*PtrVfsSetMatcherType)(int);
typedef int (*PtrVfsCapture)(void *, int);
typedef int (*PtrVfsGetImgDatasize) (void *);
typedef int (*PtrVfsGetImgWidth) (void *);
typedef int (*PtrVfsGetImgHeight) (void *);
typedef void (*PtrVfsDevExit)(void *);

#endif /*vfsWrapper */
