/*
 * llid.c
 *
 * Low-Level Interface Driver
 *
 * COPYRIGHT(C) Samsung Electronics Co.Ltd. 2009-2010 All Right Reserved.
 *
 * 2009.02 - First editing by SungHwan.yun <sunghwan.yun@samsung.com> @LDK@
 *
 * 2009.03 - Currently managed by  SungHwan.yun <sunghwan.yun@samsung.com> @LDK@
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include "j4fs.h"

#if defined(J4FS_USE_XSR)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28))
#include "./Inc/XsrTypes.h"
#include "./Inc/STL.h"
#else
#include "../../drivers/txsr/Inc/XsrTypes.h"
#include "../../drivers/txsr/Inc/STL.h"
#endif

#elif defined(J4FS_USE_FSR)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28))
#include "../../fsr/Inc/FSR.h"
#include "../../fsr/Inc/FSR_STL.h"
#else
#include "../fsr/Inc/FSR.h"
#include "../fsr/Inc/FSR_STL.h"
#endif

// J4FS for moviNAND merged from ROSSI
#elif defined(J4FS_USE_MOVI)
/* j4fs device node name */
#define J4FS_DEVNAME	"/dev/block/mmcblk0p4"
static struct file *j4fs_filp;
// J4FS for moviNAND merged from ROSSI

// J4FS for Block Devices fron NiTRo
#elif defined(J4FS_USE_BLK)
unsigned int jfs4_single_mount=0;
// J4FS for Block Devices fron NiTRo
#else

#error
#endif

extern j4fs_device_info device_info;
extern unsigned int j4fs_traceMask;

/*
  * Fills the specified buffer with the number of bytes defined by length from the device's absolute physical address
  * parameters
  *   offset  : start physical page number
  *   length  : bytes to write
  *   buffer  : buffer to write data
  * return value
  *   The return code should indicate ERROR (1) or OK(0). If the return code indicates ERROR, the dev_ptr->status field should indicate
  *   the return error code.
  */
int FlashDevRead(j4fs_device_info *dev_ptr, DWORD offset, DWORD length, BYTE *buffer)
{
// J4FS for Block Devices fron NiTRo
#ifndef J4FS_USE_BLK
    	DWORD nVol=0;
	int part_id=dev_ptr->device;
// J4FS for moviNAND merged from ROSSI
	int ret=-1;
#endif

// J4FS for Block Devices fron NiTRo
// J4FS for moviNAND merged from ROSSI
#if defined(J4FS_USE_MOVI)
// J4FS for Block Devices fron NiTRo
	mm_segment_t oldfs;
#elif defined(J4FS_USE_BLK)
	struct buffer_head *bh;
	unsigned long boffset;
	struct super_block *sb = dev_ptr->device;
#endif
// J4FS for moviNAND merged from ROSSI

#if defined(J4FS_USE_XSR)
	ret = STL_Read(nVol, part_id, offset/512, length/512, buffer);
	if (ret != STL_SUCCESS) {
		T(J4FS_TRACE_ALWAYS,("%s %d: Error(offset,length,j4fs_end,nErr)=(0x%x,0x%x,0x%x,0x%x)\n",__FUNCTION__,__LINE__,offset,length,device_info.j4fs_end,ret));
   		return J4FS_FAIL;
	}
#elif defined(J4FS_USE_FSR)
	ret = FSR_STL_Read(nVol, part_id, offset/512, length/512, buffer, FSR_STL_FLAG_DEFAULT);
	if (ret != FSR_STL_SUCCESS) {
		T(J4FS_TRACE_ALWAYS,("%s %d: Error(offset,length,j4fs_end,nErr)=(0x%x,0x%x,0x%x,0x%x)\n",__FUNCTION__,__LINE__,offset,length,device_info.j4fs_end,ret));
   		return J4FS_FAIL;
	}
// J4FS for moviNAND merged from ROSSI
// J4FS for Block Devices fron NiTRo
#elif defined(J4FS_USE_MOVI)
// J4FS for Block Devices fron NiTRo
	if (!j4fs_filp) {
		printk("J4FS not available\n");
		return J4FS_FAIL;
	}
	j4fs_filp->f_flags |= O_NONBLOCK;
	oldfs = get_fs(); set_fs(get_ds());
	ret = j4fs_filp->f_op->llseek(j4fs_filp, offset, SEEK_SET);
	ret = j4fs_filp->f_op->read(j4fs_filp, buffer, length, &j4fs_filp->f_pos);
	set_fs(oldfs);
	j4fs_filp->f_flags &= ~O_NONBLOCK;
	if (ret < 0) {
		printk("j4fs_filp->read() failed: %d\n", ret);
		return J4FS_FAIL;
	}
// J4FS for moviNAND merged from ROSSI
#elif defined(J4FS_USE_BLK)
	boffset = offset / sb->s_blocksize;
	bh = sb_bread(dev_ptr->device, boffset);
	if (!bh) {
		T(J4FS_TRACE_ALWAYS,("%s %d: cannot read block=%lu, length=%lu, bh=%p\n",__FUNCTION__,__LINE__,boffset,(unsigned long)length,bh));
		return J4FS_FAIL;
	}
	memcpy(buffer, bh->b_data, length);
	brelse(bh);
#else
#error
#endif

	return J4FS_SUCCESS;
}

/*
  * This function writes length bytes of data from a specified buffer to the destination address within the device
  * parameters
  *   offset  : start physical page number
  *   length  : bytes to write
  *   buffer  : buffer to write data
  * return value
  *   The return code should indicate ERROR (1) or OK(0). If the return code indicates ERROR, the dev_ptr->status field should indicate
  *   the return error code.
  */
int FlashDevWrite(j4fs_device_info *dev_ptr, DWORD offset, DWORD length, BYTE *buffer)
{
// J4FS for Block Devices fron NiTRo
#ifndef J4FS_USE_BLK
        DWORD nVol=0;
        int part_id=dev_ptr->device;
// J4FS for moviNAND merged from ROSSI
	int ret=-1;
#endif

// J4FS for Block Devices fron NiTRo
#if defined(J4FS_USE_MOVI)
// J4FS for Block Devices fron NiTRo
	mm_segment_t oldfs;
#elif defined(J4FS_USE_BLK)
	struct buffer_head *bh;
	unsigned long boffset;
	struct super_block *sb = dev_ptr->device;
#endif
// J4FS for moviNAND merged from ROSSI

#if defined(J4FS_USE_XSR)
	ret = STL_Write(nVol, part_id, offset/512, length/512, buffer);
	if (ret != STL_SUCCESS) {
		T(J4FS_TRACE_ALWAYS,("%s %d: Error(offset,length,j4fs_end,nErr)=(0x%x,0x%x,0x%x,0x%x)\n",__FUNCTION__,__LINE__,offset,length,device_info.j4fs_end,ret));
   		return J4FS_FAIL;
	}
#elif defined(J4FS_USE_FSR)
	ret = FSR_STL_Write(nVol, part_id, offset/512, length/512, buffer, FSR_STL_FLAG_DEFAULT);
	if (ret != FSR_STL_SUCCESS) {
		T(J4FS_TRACE_ALWAYS,("%s %d: Error(offset,length,j4fs_end,nErr)=(0x%x,0x%x,0x%x,0x%x)\n",__FUNCTION__,__LINE__,offset,length,device_info.j4fs_end,ret));
   		return J4FS_FAIL;
	}
// J4FS for Block Devices fron NiTRo
#elif defined(J4FS_USE_MOVI)
// J4FS for Block Devices fron NiTRo
	if (!j4fs_filp) {
		printk("J4FS not available\n");
		return J4FS_FAIL;
	}
	j4fs_filp->f_flags |= O_NONBLOCK;
	oldfs = get_fs(); set_fs(get_ds());
	ret = j4fs_filp->f_op->llseek(j4fs_filp, offset, SEEK_SET);
	ret = j4fs_filp->f_op->write(j4fs_filp, buffer, length, &j4fs_filp->f_pos);
	set_fs(oldfs);
	j4fs_filp->f_flags &= ~O_NONBLOCK;
	if (ret < 0) {
		printk("j4fs_filp->write() failed: %d\n", ret);
		return J4FS_FAIL;
	}
// J4FS for moviNAND merged from ROSSI
#elif defined(J4FS_USE_BLK)
	boffset = offset / sb->s_blocksize;
	bh = sb_getblk(dev_ptr->device, boffset);
	if (!bh) {
		T(J4FS_TRACE_ALWAYS,("%s %d: cannot get block=%lu, length=%lu, bh=%p\n",__FUNCTION__,__LINE__,boffset,(unsigned long)length,bh));
		return J4FS_FAIL;
	}
	lock_buffer(bh);
	bh->b_end_io = end_buffer_write_sync;
	get_bh(bh);
	memcpy(bh->b_data, buffer, length);
	if (!buffer_uptodate(bh))
	        set_buffer_uptodate(bh);
	if (buffer_dirty(bh))
	        clear_buffer_dirty(bh);
	submit_bh(WRITE_SYNC | WRITE_FLUSH_FUA, bh);
	wait_on_buffer(bh);
	if (unlikely(!buffer_uptodate(bh))) {
		T(J4FS_TRACE_ALWAYS,("%s %d: cannot write block %lu\n",__FUNCTION__,__LINE__,boffset));
		return J4FS_FAIL;
	}
	brelse(bh);
#else
#error
#endif

	return J4FS_SUCCESS;
}

/*
  * In order to reuse the flash media, an erase command must be provided for the FSD. This command erases a single flash erase-block beginning
  * at the address specified by the aux field in the DEVICE_INFO structure. The blocksize field of the DEVICE_INFO structure is used to force the aux ptr
  * to a block boundary.
  * parameters
  *   dev_ptr->aux : start block address to be erased
  */
int FlashDevErase(j4fs_device_info *dev_ptr)
{
	// TODO
	return J4FS_SUCCESS;
}

int FlashDevSpecial(j4fs_device_info *dev_ptr, DWORD scmd)
{
	switch(scmd)
	{
		case J4FS_INIT:
			// Initialize the internal FSD structures to use a device
// J4FS for Block Devices fron NiTRo
#ifndef J4FS_USE_BLK
			FlashDevMount();
#endif
// J4FS for Block Devices fron NiTRo
			break;

		case J4FS_GET_INFO:
			break;

// J4FS for moviNAND merged from ROSSI
		case J4FS_EXIT:
// J4FS for Block Devices fron NiTRo
#ifndef J4FS_USE_BLK
			FlashDevUnmount();
#endif
// J4FS for Block Devices fron NiTRo
// J4FS for moviNAND merged from ROSSI

		default:
			break;
	}

	return J4FS_SUCCESS;
}

// J4FS for Block Devices fron NiTRo
#if defined(J4FS_USE_BLK)
int FlashDevMount(struct super_block *sb)
#else
// J4FS for Block Devices fron NiTRo
int FlashDevMount()
// J4FS for Block Devices fron NiTRo
#endif
// J4FS for Block Devices fron NiTRo
{

	DWORD media_status_table_size=1;		//  Media Status Table occupys 1 block

// J4FS for Block Devices fron NiTRo
#if defined(J4FS_USE_BLK)
	if(jfs4_single_mount){
		printk("FlashDevMount : J4FS already mounted!\n");
		return J4FS_FAIL;
	}
#endif
#if defined(J4FS_USE_MOVI)
// J4FS for Block Devices fron NiTRo
// J4FS for moviNAND merged from ROSSI
	j4fs_filp = filp_open(J4FS_DEVNAME, O_RDWR|O_SYNC, 0);
// J4FS for Block Devices fron NiTRo
	if (IS_ERR(j4fs_filp)) {
		printk("FlashDevMount : filp_open() failed~!: %ld\n", PTR_ERR(j4fs_filp));
		return J4FS_FAIL;
	}
	printk("FlashDevMount : filp_open() OK....!\n");
#endif
// J4FS for moviNAND merged from ROSSI
#if defined(J4FS_USE_BLK) 
	device_info.device = sb
#else
	device_info.device=J4FS_PARTITION_ID;
#endif
	device_info.blocksize=PHYSICAL_BLOCK_SIZE;
	device_info.pagesize=PHYSICAL_PAGE_SIZE;
	device_info.j4fs_offset=media_status_table_size*device_info.blocksize;	 // j4fs_offset follows the Media Status Table.
// J4FS for Block Devices fron NiTRo
#if defined(J4FS_USE_BLK)
	jfs4_single_mount=1;
#endif
// J4FS for Block Devices fron NiTRo
	return J4FS_SUCCESS;
}

int FlashDevUnmount()
{
// ROSSI Projecct(hyunkwon.kim) 2010.09.06 Add J4FS for Parameter Infomation
// J4FS for Block Devices fron NiTRo
#if defined(J4FS_USE_MOVI)
// J4FS for Block Devices fron NiTRo
	filp_close(j4fs_filp, NULL);
	printk("FlashDevUnmount : filp_close() OK....!\n");
	jfs4_single_mount=0;
#endif
// ROSSI Projecct(hyunkwon.kim) 2010.09.06 End

	return J4FS_SUCCESS;
}
