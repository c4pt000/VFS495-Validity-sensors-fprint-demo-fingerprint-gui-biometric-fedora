

/*! @file vfsDriver.c
*******************************************************************************
* libfprint Interface Functions
*
* This file contains the libfprint interface functions for validity fingerprint sensor device.
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

#include <errno.h>
#include <string.h>
#include <glib.h>
#include <usb.h>
#include <fp_internal.h>
#include <stdio.h>
#include <dlfcn.h>
#include "vfsDriver.h"
#include "vfsWrapper.h"
#include <syslog.h>


static int dev_init( struct fp_img_dev *dev,
                     unsigned long driver_data );

static void dev_exit( struct fp_img_dev *dev );

static int capture( struct fp_img_dev *dev,
                    gboolean initial,
                    struct fp_img **img );

static int enroll( struct fp_dev *dev,
                   gboolean initial,
                   int stage,
                   struct fp_print_data **ret,
                   struct fp_img **_img );

static const struct usb_id id_table[] = {
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_301,  },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_451,  },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_5111,  },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_5011,  },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_471,  },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_5131,  },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_491,  },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_495,  },
    { 0, 0, 0, }, /* terminating entry */
};

struct fp_img_driver  validity_driver = {
    .driver = {
        .id = VALIDITY_DRIVER_ID,
        .name = VALIDITY_FP_COMPONENT,
        .full_name = VALIDITY_DRIVER_FULLNAME,
        .id_table = id_table,
    },
    .init = dev_init,
    .exit = dev_exit,
    .capture = capture,
};

/*!
*******************************************************************************
** Creates Validity's client context object. Waits until the sensor is
** ready for use or response time out of Validity's stack.
**
** @param[in,out]    dev,driver_data
**       Pointer to the device structure
**       Unsigned long  driver_data
**
** @return
**       -  0 on success
**       - -1 on failure 
*/

static int dev_init(struct fp_img_dev *dev, unsigned long driver_data)
{
    validity_dev* pValidityDriver = NULL;  
    void *handle = NULL;
    PtrVfsWaitForService ptrVfsWaitForService = NULL;
    int vfsWrapperResult = 0;

    if (NULL == dev)
    {
       fp_err("NULL device structure");
       vfsWrapperResult = -EINVAL;
       goto cleanup;
    }

    /* only look for presence of validity driver, else exit
     * open will happen again in capture */
    handle = dlopen ("libvfsFprintWrapper.so", RTLD_LAZY | RTLD_GLOBAL | RTLD_NODELETE);
    if (!handle) {
        fputs (dlerror(), stderr);
	return -ENODEV;
    }
   /* wait for validity device to come up and be ready to take a finger swipe
     * Wait will happen for a stipulated time(10s - 40s), then errors */
    ptrVfsWaitForService = dlsym(handle, "vfs_wait_for_service");
    if( ptrVfsWaitForService )
    {
        vfsWrapperResult = (*ptrVfsWaitForService)();
        if (vfsWrapperResult != VFS_RESULT_WRAPPER_OK)
        {
            fp_err("VFS module failed to wait for service");
            vfsWrapperResult = -EPERM;
            dlclose(handle);
            goto cleanup;
        }
    }
    dlclose(handle);

    pValidityDriver = g_malloc(sizeof(validity_dev));
    if (NULL == pValidityDriver)
    {
       vfsWrapperResult = -ENOMEM;
       goto cleanup;
    }
    memset(pValidityDriver, 0, sizeof(validity_dev));

    dev->priv = pValidityDriver;
    dev->dev->nr_enroll_stages = 1;
    dev->dev->drv->enroll = enroll;

cleanup:
    return vfsWrapperResult;
}

/*!
*******************************************************************************
** Closes the opened sensor. Destroys Validity's client context object. Frees
** allocated resources
**
** @param[in]    dev
**       Pointer to the device structure
**
** @return
*/

static void dev_exit(struct fp_img_dev *dev)
{
    validity_dev* pValidityDriver = NULL;

    if (NULL != dev)
    {
        if (NULL != dev->priv)
        {
            /* perform cleanup */
            pValidityDriver = dev->priv;
            g_free(pValidityDriver);
            pValidityDriver = NULL;
            dev->priv = NULL;
       }
    }
}

/*!
*******************************************************************************
** Captures the finger print and stores the same. Copies the image to the fp_img
** structure and raw data to the fp_print_data structure on enroll success.
**
**
** @param[in,in,in,out,out]    dev,initial,stage,_data,img
**       Pointer to the device structure
**       boolean flag for ditermining the initial state
**       int for determining the current stage of enrollment
**       Pointer to the finger print data
**       Pointer to the pointer of image structure
**
** @return
**       enum fp_enroll_result
*/
#define EXIT_ON_DLSYM_ERROR(x) if( !x )                \
                            {                             \
                                fputs (dlerror(), stderr);\
                                return -ENODEV;           \
                            }

static int capture( struct fp_img_dev *dev,
                    gboolean initial,
                    struct fp_img **img)
{
    validity_dev* pValidityDriver = NULL;
    struct fp_img *pImg = NULL;
    unsigned int data_len = 0;
    unsigned char* pImgData = NULL;
    int vfsWrapperResult = FP_CAPTURE_ERROR;
   
    void *handle = NULL;

    PtrVfsSetMatcherType ptrVfsSetMatcherType= NULL;
    PtrVfsCapture ptrVfsCapture = NULL;
    PtrVfsDevInit ptrVfsDevInit = NULL;
    PtrVfsGetImgDatasize ptrVfsGetImgDatasize = NULL;
    PtrVfsGetImgWidth ptrVfsGetImgWidth = NULL;
    PtrVfsGetImgHeight ptrVfsGetImgHeight = NULL;
    PtrVfsGetImgData ptrVfsGetImgData = NULL;
    PtrVfsFreeImgData ptrVfsFreeImgData = NULL;
    PtrVfsCleanHandles ptrVfsCleanHandles = NULL;
    PtrVfsDevExit ptrVfsDevExit = NULL;

    openlog("Validity-capture",LOG_CONS |LOG_NDELAY,LOG_DAEMON);

    handle = dlopen ("libvfsFprintWrapper.so", RTLD_LAZY | RTLD_GLOBAL | RTLD_NODELETE);
    if (!handle) {
        fputs (dlerror(), stderr);
        return -ENODEV;
    }

    syslog(LOG_WARNING,"Entry\n");
    if ( (NULL == dev) ||
        (NULL == dev->priv) )
    {
       vfsWrapperResult = -EINVAL;
       fp_err("NULL Validity device structure");
       goto cleanup;
    }
    pValidityDriver = dev->priv;

    /* Set the matcher type */
    ptrVfsSetMatcherType = dlsym(handle, "vfs_set_matcher_type");

    EXIT_ON_DLSYM_ERROR(ptrVfsSetMatcherType);

    (*ptrVfsSetMatcherType)(VFS_FPRINT_MATCHER);
 
    ptrVfsDevInit = dlsym(handle, "vfs_dev_init");
    
    EXIT_ON_DLSYM_ERROR(ptrVfsDevInit);
    
    vfsWrapperResult = (*ptrVfsDevInit)(pValidityDriver);
    
    if (vfsWrapperResult != VFS_RESULT_WRAPPER_OK)
    {
       fp_err("VFS module failed to initialize");
       vfsWrapperResult = -EPERM;
       goto cleanup;
    }
    
    
    ptrVfsCapture = dlsym(handle, "vfs_capture");
    EXIT_ON_DLSYM_ERROR(ptrVfsCapture);
    
    vfsWrapperResult =  (*ptrVfsCapture)(pValidityDriver,initial);

    if ( vfsWrapperResult == FP_CAPTURE_COMPLETE )
    {
        /* need to check here */

        ptrVfsGetImgDatasize = dlsym(handle, "vfs_get_img_datasize");

	EXIT_ON_DLSYM_ERROR(ptrVfsGetImgDatasize);

        data_len = (*ptrVfsGetImgDatasize)(pValidityDriver);
        if(data_len == 0)
        {
            fp_err("Zero image size");
            vfsWrapperResult = -ENODATA;
            goto cleanup;
        }

        pImg = fpi_img_new(data_len);
        if (NULL == pImg)
        {
            *img = NULL;
            fp_err("Failed allocate memory for finger print image");
            vfsWrapperResult = -ENOMEM;
            goto cleanup;
        }
        memset(pImg,0,data_len);
        pImg->length = data_len;

         ptrVfsGetImgWidth = dlsym(handle, "vfs_get_img_width");

        EXIT_ON_DLSYM_ERROR( ptrVfsGetImgWidth);
        pImg->width =(* ptrVfsGetImgWidth)(pValidityDriver);
          
         ptrVfsGetImgHeight = dlsym(handle, "vfs_get_img_height");

	EXIT_ON_DLSYM_ERROR(ptrVfsGetImgHeight);
        pImg->height =(*ptrVfsGetImgHeight)(pValidityDriver);

        syslog(LOG_WARNING,"%d x %d image returned\n", pImg->width, pImg->height );

        ptrVfsGetImgData = dlsym(handle, "vfs_get_img_data");

        EXIT_ON_DLSYM_ERROR(ptrVfsGetImgData);
        pImgData =(*ptrVfsGetImgData)(pValidityDriver);

        if (NULL != pImgData)
        {
            g_memmove(pImg->data,pImgData,data_len);
            *img = pImg;
              
            ptrVfsFreeImgData = dlsym(handle, "vfs_free_img_data");

	    EXIT_ON_DLSYM_ERROR(ptrVfsFreeImgData);
            (*ptrVfsFreeImgData)(pImgData);

            pImgData = NULL;
        }
        else
        {
            *img = NULL;
            vfsWrapperResult = -ENODATA;
            fp_err("Failed to get finger print image data");
            goto cleanup;
        }
    }
    
    if( vfsWrapperResult == FP_CAPTURE_ERROR )
    {
        fp_err("Invalid vfsWrapperResult ");
        vfsWrapperResult = -EIO;
    }

cleanup:

    ptrVfsCleanHandles = dlsym(handle, "vfs_clean_handles");
    EXIT_ON_DLSYM_ERROR(ptrVfsCleanHandles);
    

    (*ptrVfsCleanHandles)(pValidityDriver);

    if ( (FP_CAPTURE_FAIL== vfsWrapperResult) ||
         (-EPROTO ==        vfsWrapperResult) ||
         (-EIO ==           vfsWrapperResult) ||
         (-EINVAL ==        vfsWrapperResult) ||
         (-ENOMEM ==        vfsWrapperResult) ||
         (-ENODATA ==       vfsWrapperResult) )
    {
        if (NULL != pImg)
        {
            fp_img_free(pImg);
        }
    }

    ptrVfsDevExit = dlsym(handle, "vfs_dev_exit");
    EXIT_ON_DLSYM_ERROR(ptrVfsDevExit);

    (*ptrVfsDevExit)(pValidityDriver);

    dlclose(handle);

    
	syslog(LOG_WARNING,"Exit");

    if ( vfsWrapperResult == FP_CAPTURE_COMPLETE )
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

#define VAL_MIN_ACCEPTABLE_MINUTIAE (2*MIN_ACCEPTABLE_MINUTIAE)
#define VAL_DEFAULT_THRESHOLD 60

#define POP_MESSAGE_ENABLE 1
#define POP_MESSAGE(X)   if(getenv("DISPLAY") && fork()== 0)		\
                         {                                                                \
                             system("xmessage -timeout 2 -center "#X);                    \
                             _exit(0);                                                    \
                         }

static int enroll(struct fp_dev *dev, gboolean initial, int stage,
    struct fp_print_data **ret, struct fp_img **_img)
{
    struct fp_img *img = NULL;
    struct fp_img_dev *imgdev = dev->priv;
    struct fp_print_data *print = NULL;
    int r;

    /* we will make 6 capture attempts to capture at least 3 good prints */
    int ii, iCapt = 0, iGood = 0, iDiff = 0;
    struct fp_img *imgG[3] = {0};
    struct fp_print_data *printG[3] = {0};
    int match_score01, match_score12, match_score20;
    int thresh = VAL_DEFAULT_THRESHOLD;

    while( ( iCapt < 6 ) && ( iGood < 3 ) )
    {
#if POP_MESSAGE_ENABLE
        /* intimate user about previous unsuccessful swipe */
        if( ( iCapt - iGood ) > iDiff )
        {
            POP_MESSAGE("bad swipe, please try again ");
            iDiff = iCapt - iGood;
        }
#endif

        img = NULL;
        r = fpi_imgdev_capture(imgdev, 0, &img);
        iCapt++;

        /* If we got an image, standardize it. */
        if (img)
            fp_img_standardize(img);
        if (r)
        {
            if(img)
                fp_img_free(img);
            img = NULL;
            continue;
        }

        print = NULL;
        r = fpi_img_to_print_data(imgdev, img, &print);
        if (r < 0)
        {
            if(img)
                fp_img_free(img);
            img = NULL;
            if(print)
                fp_print_data_free(print);
            print = NULL;
            continue;
        }
        /*printf( "MINUTIAE - %d, min required - %d\n", img->minutiae->num, MIN_ACCEPTABLE_MINUTIAE );*/
        if (img->minutiae->num < VAL_MIN_ACCEPTABLE_MINUTIAE) {
            fp_dbg("not enough minutiae, %d/%d", r, VAL_MIN_ACCEPTABLE_MINUTIAE);

            if(img)
                fp_img_free(img);
            img = NULL;

            if(print)
                fp_print_data_free(print);
            print = NULL;

            continue;
        }

        /* save the good image and print */
        imgG[iGood] = img;
        printG[iGood] = print;
        iGood++;

#if POP_MESSAGE_ENABLE
        /* intimate user about successful swipe */
        if( iGood == 1 )
        {
            POP_MESSAGE("1 good swipe captured 2 to go ");
        }
        else if( iGood == 2 )
        {
            POP_MESSAGE("2 good swipes captured 1 to go ");
        }
        else if( iGood == 3 )
        {
            POP_MESSAGE("3 good swipes captured DONE ");
        }
#endif
    }

    if( iGood == 0 )
    {
        return -1;
    }

    if( iGood < 3 )
    {
        goto err;
    }

    /* 3 successful captures, get their match scores */
    match_score01 = fpi_img_compare_print_data( printG[0], printG[1] );
    match_score12 = fpi_img_compare_print_data( printG[1], printG[2] );
    match_score20 = fpi_img_compare_print_data( printG[2], printG[0] );

    if( ( match_score01 >= match_score12 ) && ( match_score20 >= match_score12 ) )
    {
        /* 0 is the best print, pick it and clean up others */
        if( ( match_score01 >= thresh ) || ( match_score20 >= thresh ) )
        {
            /* best print has at least one good match */
            if (_img)
                *_img = imgG[0];
            *ret = printG[0];

            fp_img_free(imgG[1]);
            fp_img_free(imgG[2]);

            fp_print_data_free(printG[1]);
            fp_print_data_free(printG[2]);
        }
        else
        {
            goto err;
        }
    }
    else if( ( match_score12 >= match_score20 ) && ( match_score01 >= match_score20 ) )
    {
        /* 1 is the best print, pick it and clean up others */
        if( ( match_score12 >= thresh ) || ( match_score01 >= thresh ) )
        {
            /* best print has at least one good match */
            if (_img)
                *_img = imgG[1];
            *ret = printG[1];

            fp_img_free(imgG[2]);
            fp_img_free(imgG[0]);

            fp_print_data_free(printG[2]);
            fp_print_data_free(printG[0]);
        }
        else
        {
            goto err;
        }
    }
    else if( ( match_score20 >= match_score01 ) && ( match_score12 >= match_score01 ) )
    {
        /* 2 is the best print, pick it and clean up others */
        if( ( match_score20 >= thresh ) || ( match_score12 >= thresh ) )
        {
            /* best print has at least one good match */
            if (_img)
                *_img = imgG[2];
            *ret = printG[2];

            fp_img_free(imgG[0]);
            fp_img_free(imgG[1]);

            fp_print_data_free(printG[0]);
            fp_print_data_free(printG[1]);
        }
        else
        {
            goto err;
        }
    }
    else
    {
        printf( "SHOULD NEVER BE HERE\n" );
        return -1;
    }

#if POP_MESSAGE_ENABLE
    /* intimate user about successful enrollment */
    POP_MESSAGE("Enrollment Success ");
#endif

    return FP_ENROLL_COMPLETE;

err:

#if POP_MESSAGE_ENABLE
    /* intimate user about unsuccessful enrollment */
    if( iGood < 3 )
    {
        POP_MESSAGE("Enrollment Failure, not enough good swipes ");
    }
    else
    {
        POP_MESSAGE("Enrollment Failure, inconsistent images ");
    }
#endif

    /* clean up */
    if (_img)
        *_img = imgG[0];

    for( ii = 1; ii < iGood; ii++ )
    {
        fp_img_free(imgG[ii]);
    }

    for( ii = 0; ii < iGood; ii++ )
    {
        fp_print_data_free(printG[ii]);
    }
    /* return */
    return FP_ENROLL_RETRY;
}
