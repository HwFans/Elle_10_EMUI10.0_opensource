/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>

#include <product_config.h>

#include "mdrv.h"
#include <osl_thread.h>
#include "bsp_pm_om.h"
#include "bsp_slice.h"
//#include "bsp_nandc.h"

#include "rfile_balong.h"
#include <securec.h>
#include <bsp_print.h>
#define THIS_MODU mod_rfile
/*lint --e{830, 529, 533, 64, 732, 737,607, 501}*/


#define RFILE_AUTO_MKDIR
#define RFILE_PM_SIZE       257

struct bsp_rfile_main_stru g_stRfileMain = {EN_RFILE_INIT_INVALID, };


struct rfile_mntn_stru g_stRfileMntnInfo;

s8 * rfileerror = (s8*)"error";

s32 rfile_lpmcallback(int x)
{
    g_stRfileMain.lpmstate = 1;
	return 0;
}

s8 *rfile_getdirpath(s32 fp)
{
    struct list_head *p, *n;
    struct dir_list *tmp;

    list_for_each_safe(p,n,&(g_stRfileMain.dplist))
    {
        tmp = list_entry(p, struct dir_list ,stlist);
        if (tmp->dp == fp)
        {
            return tmp->name;
        }
    }

    return rfileerror;
}

s8 *rfile_getfilepath(s32 fp)
{
    struct list_head *p, *n;
    struct fp_list *tmp;

    list_for_each_safe(p,n,&(g_stRfileMain.fplist))
    {
        tmp = list_entry(p, struct fp_list ,stlist);
        if (tmp->fp == fp)
        {
            return tmp->name;
        }
    }

    return rfileerror;
}

#define RFILE_LPM_PRINT_PATH(op, path) \
do{ \
    s32 ret;\
	if(g_stRfileMain.lpmstate) \
    { \
        char rfile_pm[RFILE_PM_SIZE]={0};\
        ret = snprintf_s(rfile_pm,RFILE_PM_SIZE,RFILE_PM_SIZE-1,"op %d,path %s.\n",op,path);\
		if(ret != EOK){\
            bsp_err("<%s> snprintf_s err. ret =  %d.\n", __FUNCTION__, ret);\
        }\
        bsp_pm_log(PM_OM_ARFILE, strlen(rfile_pm),rfile_pm);\
        g_stRfileMain.lpmstate = 0; \
        bsp_info("[C SR] rfile op %d, path %s.\n", op, path); \
    } \
}while(0);

#define RFILE_LPM_PRINT_DIRPATH(op, fd) \
do{ \
    s32 ret;\
    if(g_stRfileMain.lpmstate) \
    { \
        char rfile_pm[RFILE_PM_SIZE]={0};\
        ret = snprintf_s(rfile_pm,RFILE_PM_SIZE,RFILE_PM_SIZE-1,"op %d,path %s.\n",op,rfile_getdirpath((s32)fd));\
		if(ret != EOK){\
            bsp_err("<%s> snprintf_s err. ret =  %d.\n", __FUNCTION__, ret);\
        }\
        bsp_pm_log(PM_OM_ARFILE, strlen(rfile_pm),rfile_pm);\
        g_stRfileMain.lpmstate = 0; \
        bsp_info("[C SR] rfile op %d, path %s.\n", op, rfile_getdirpath((s32)fd)); \
    } \
}while(0);

#define RFILE_LPM_PRINT_FILEPATH(op, fd) \
do{ \
    s32 ret;\
    if(g_stRfileMain.lpmstate) \
    { \
        char rfile_pm[RFILE_PM_SIZE]={0};\
        ret = snprintf_s(rfile_pm,RFILE_PM_SIZE,RFILE_PM_SIZE-1,"op %d,path %s.\n",op,rfile_getfilepath((s32)fd));\
		if(ret != EOK){\
            bsp_err("<%s> snprintf_s err. ret =  %d.\n", __FUNCTION__, ret);\
        }\
        bsp_pm_log(PM_OM_ARFILE, strlen(rfile_pm),rfile_pm);\
        g_stRfileMain.lpmstate = 0; \
        bsp_info("[C SR] rfile op %d, path %s.\n", op, rfile_getfilepath((s32)fd)); \
    } \
}while(0);

void rfile_MntnDotRecord(u32 line)
{
    u32 ptr = g_stRfileMntnInfo.stdot.ptr;
    g_stRfileMntnInfo.stdot.line[ptr] = line;
    g_stRfileMntnInfo.stdot.slice[ptr] = bsp_get_slice_value();
    g_stRfileMntnInfo.stdot.ptr = (g_stRfileMntnInfo.stdot.ptr+1)%RFILE_MNTN_DOT_NUM;
}


/* 查看输入的路径是否可访问，如不能访问则创建此目录 */
/* 本函数为递归调用函数 */
s32 __AccessCreate(char *pathtmp, s32 mode, u32 count)
{
    char *p;
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);
    ret = sys_access(pathtmp, 0);
    set_fs(old_fs);

    if(count > RFILE_STACK_MAX)
    {
        bsp_err("<%s> sys_mkdir %s stack %d over %d.\n", __FUNCTION__, pathtmp, count, RFILE_STACK_MAX);
        return -1;
    }

    if(ret != 0)
    {
        /* 路径中不包含'/'，返还失败 */
        p = strrchr(pathtmp, '/');
        if(NULL == p)
        {
            bsp_err("<%s> strrchr %s no '/'.\n", __FUNCTION__, pathtmp);
                
            return -1;
        }

        /* 已经不是根目录下的文件夹，判断上一级目录是否存在 */
        if(p != pathtmp)
        {
            /* 查看上一级目录是否存在，如果不存在则创建此目录 */
            *p = '\0';
		// cppcheck-suppress *
            ret = __AccessCreate(pathtmp, mode, count + 1);
            if(0 != ret)
            {
                return -1;
            }

            /* 创建当前目录 */
            *p = '/';
        }

        old_fs = get_fs();
        set_fs((mm_segment_t)KERNEL_DS);
        ret = sys_mkdir(pathtmp, 0775);
        set_fs(old_fs);

        if(0 != ret)
        {
            bsp_err( "<%s> sys_mkdir %s failed ret %d.\n", __FUNCTION__, pathtmp, ret);
               

            return -1;
        }
    }

    return 0;
}


s32 AccessCreate(char *pathtmp, s32 mode)
{
    return __AccessCreate(pathtmp, mode, 0);
}

void rfile_FpListDel(s32 fp)
{
    struct list_head *p, *n;
    struct fp_list *tmp;

    rfile_MntnDotRecord(__LINE__);

    list_for_each_safe(p,n,&(g_stRfileMain.fplist))
    {
        tmp = list_entry(p, struct fp_list ,stlist);
        if (tmp->fp == fp)
        {
            list_del(&tmp->stlist);
            Rfile_Free(tmp);
        }
    }
}

void rfile_FpListAdd(s32 fp, s8 *name)
{/*lint --e{429}*/
    struct fp_list *fp_elemt;
    u32 len;
    s32 ret;

    rfile_MntnDotRecord(__LINE__);

    fp_elemt = (struct fp_list *)Rfile_Malloc(sizeof(struct fp_list));
    if(fp_elemt == NULL)
    {
        bsp_err("<%s> malloc fp_elemt failed.\n", __FUNCTION__);

        return ;
    }
    ret = memset_s((void*)fp_elemt,sizeof(*fp_elemt),0,sizeof(struct fp_list));
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }
    fp_elemt->fp=fp;

    len = (u32)strlen(name);
    if(len > RFILE_NAME_MAX)
    {
        len = RFILE_NAME_MAX;
    }

    ret = memcpy_s(fp_elemt->name,RFILE_NAME_MAX ,name, (s32)len);
    if(ret != EOK){
        bsp_err("<%s> memcpy_s err. ret =  %d.\n", __FUNCTION__, ret);
    }
    fp_elemt->name[len] = '\0';

    list_add(&(fp_elemt->stlist), &(g_stRfileMain.fplist));

    return ;
}

void rfile_DpListDel(s32 dp)
{
    struct list_head *p, *n;
    struct dir_list *tmp;

    rfile_MntnDotRecord(__LINE__);

    list_for_each_safe(p,n,&(g_stRfileMain.dplist))
    {
        tmp = list_entry(p, struct dir_list ,stlist);
        if (tmp->dp == dp)
        {
            list_del(&tmp->stlist);
            Rfile_Free(tmp);
        }
    }
}

void rfile_DpListAdd(s32 dp, s8 *name)
{/*lint --e{429}*/
    struct dir_list *dp_elemt;
    u32 len;
    s32 ret;

    rfile_MntnDotRecord(__LINE__);

    dp_elemt=(struct dir_list *)Rfile_Malloc(sizeof(struct dir_list));
    if(dp_elemt == NULL)
    {
        bsp_err("<%s> malloc dp_elemt failed.\n", __FUNCTION__);

        return ;
    }
    ret = memset_s((void*)dp_elemt,sizeof(*dp_elemt),0,sizeof(struct dir_list));
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }
    dp_elemt->dp=dp;

    len = (u32)strlen(name);
    if(len > RFILE_NAME_MAX)
    {
        len = RFILE_NAME_MAX;
    }

    ret = memcpy_s(dp_elemt->name,RFILE_NAME_MAX,name, (s32)len);
    if(ret != EOK){
        bsp_err("<%s> memcpy_s err. ret =  %d.\n", __FUNCTION__, ret);
    }
    dp_elemt->name[len] = '\0';

    list_add(&(dp_elemt->stlist), &(g_stRfileMain.dplist));

    return ;
}

s32 bsp_fs_ok(void)
{
    if(g_stRfileMain.eInitFlag == EN_RFILE_INIT_INVALID){
        return -ENODEV;
    }
    return 0;
}

s32 bsp_open(const s8 *path, s32 flags, s32 mode)
{
    s32 ret;
    unsigned long old_fs;
    u32 time_stamp[3] = {0};
#ifdef RFILE_AUTO_MKDIR
    char *p;
    char pathtmp[256] = {0};
#endif
    time_stamp[0] = bsp_get_slice_value();

#ifdef RFILE_AUTO_MKDIR
    if(strlen(path) > 255)
    {
        return -1;
    }

    ret = memcpy_s(pathtmp, sizeof(pathtmp), (char *)path, strlen(path));
    if(ret != EOK){
        bsp_err("<%s> memcpy_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    /* 路径中包含'/'并且不在根目录，则检查当前目录是否存在，不存在则创建目录 */
    p = strrchr(pathtmp, '/');
    if((NULL != p) && (p != pathtmp))
    {
        /* 查看上一级目录是否存在，如果不存在则创建此目录 */
        *p = '\0';
        ret = AccessCreate(pathtmp, mode);
        if(ret)
        {
            bsp_err("<%s> AccessCreate failed.\n", __FUNCTION__);
                
        }
    }
#endif

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_open(path, flags, mode); /*lint !e734*/

    set_fs(old_fs);
    time_stamp[1] = bsp_get_slice_value();
    time_stamp[2] = get_timer_slice_delta(time_stamp[0], time_stamp[1]);
    if(time_stamp[2] > (bsp_get_slice_freq()*DELAY_TIME))
    {
        bsp_err("bsp_open file name %s out of time start 0x%x end 0x%x detal 0x%x\n", path, time_stamp[0],time_stamp[1],time_stamp[2]);
                
    }
    return ret;
}


s32 bsp_close(u32 fp)
{/*lint --e{449}*/
    s32 ret;
    unsigned long old_fs;
    u32 time_stamp[3] = {0};
    time_stamp[0] = bsp_get_slice_value();

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);


    ret = sys_close(fp);

    set_fs(old_fs);
    time_stamp[1] = bsp_get_slice_value();
    time_stamp[2] = get_timer_slice_delta(time_stamp[0], time_stamp[1]);
    if(time_stamp[2] > (bsp_get_slice_freq()*DELAY_TIME))
    {
        bsp_err("bsp_close %d out of time start 0x%x end 0x%x detal 0x%x\n", fp, time_stamp[0],time_stamp[1],time_stamp[2]);
                
    }

    return ret;
}



s32 bsp_write(u32 fd, const s8 *ptr, u32 size)
{
    s32 ret;
    unsigned long old_fs;
    u32 time_stamp[3] = {0};
    struct file *file_p=NULL;
    file_p = fget(fd);
    time_stamp[0] = bsp_get_slice_value();

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_write(fd, ptr, (s32)size);

    set_fs(old_fs);

    time_stamp[1] = bsp_get_slice_value();
    time_stamp[2] = get_timer_slice_delta(time_stamp[0], time_stamp[1]);
    if(time_stamp[2] > (bsp_get_slice_freq()*DELAY_TIME))
    {
        if(file_p)
        {
            bsp_err("bsp_write file name %s out of time start 0x%x end 0x%x detal 0x%x\n", file_p->f_path.dentry->d_iname,time_stamp[0],time_stamp[1],time_stamp[2]);
                    
        }
        else
        {
            bsp_err("bsp_write file fd %d out of time start 0x%x end 0x%x detal 0x%x\n", fd,time_stamp[0],time_stamp[1],time_stamp[2]);
        }
    }
    if(file_p)
	{
		fput(file_p);
	}
    return ret;

}


s32 bsp_write_sync(u32 fd, const s8 *ptr, u32 size)
{
    s32 ret;
    unsigned long old_fs;
    u32 time_stamp[3] = {0};
    struct file *file_p=NULL;
    file_p = fget(fd);
    time_stamp[0] = bsp_get_slice_value();

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_write(fd, ptr, (s32)size);

    (void)sys_fsync(fd);

    set_fs(old_fs);
    time_stamp[1] = bsp_get_slice_value();
    time_stamp[2] = get_timer_slice_delta(time_stamp[0], time_stamp[1]);
    if(time_stamp[2] > (bsp_get_slice_freq()*DELAY_TIME))
    {
        if(file_p)
        {
            bsp_err("bsp_write_sync file name %s out of time start 0x%x end 0x%x detal 0x%x\n", file_p->f_path.dentry->d_iname,time_stamp[0],time_stamp[1],time_stamp[2]);
                   
        }
        else
        {
            bsp_err("bsp_write_sync file fd %d out of time start 0x%x end 0x%x detal 0x%x\n", fd,time_stamp[0],time_stamp[1],time_stamp[2]);
                    
        }
    }
    if(file_p)
	{
		fput(file_p);
	}
    return ret;

}


s32 bsp_read(u32 fd, s8 *ptr, u32 size)
{
    s32 ret;
    unsigned long old_fs;
    u32 time_stamp[3] = {0};
    struct file *file_p=NULL;
    file_p = fget(fd);
    time_stamp[0] = bsp_get_slice_value();

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_read(fd, ptr, (s32)size);

    set_fs(old_fs);
    time_stamp[1] = bsp_get_slice_value();
    time_stamp[2] = get_timer_slice_delta(time_stamp[0], time_stamp[1]);
    if(time_stamp[2] > (bsp_get_slice_freq()*DELAY_TIME))
    {
        if(file_p)
        {
            bsp_err("bsp_read file name %s out of time start 0x%x end 0x%x detal 0x%x\n", file_p->f_path.dentry->d_iname,time_stamp[0],time_stamp[1],time_stamp[2]);
                    
        }
        else
        {
            bsp_err("bsp_read file fd %d out of time start 0x%x end 0x%x detal 0x%x\n", fd ,time_stamp[0],time_stamp[1],time_stamp[2]);
                    
        }
    }
    if(file_p)
	{
		fput(file_p);
	}
    return ret;

}



s32 bsp_lseek(u32 fd, long offset, s32 whence)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_lseek(fd, offset, (u32)whence);

    set_fs(old_fs);

    return ret;
}



long bsp_tell(u32 fd)
{
    s32 ret;
    loff_t offset = 0;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_llseek(fd, 0, 0, &offset, SEEK_CUR);
    if(0 != ret)
    {
    }

    set_fs(old_fs);

    return (long)offset;
}



s32 bsp_remove(const s8 *pathname)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_unlink(pathname);

    set_fs(old_fs);

    return ret;
}



s32 bsp_mkdir(s8 *dirName, s32 mode)
{
    s32 ret;
#ifdef RFILE_AUTO_MKDIR
    char pathtmp[255+1] = {0};

    if(strlen(dirName) > 255)
    {
        return -1;
    }

    ret = memcpy_s(pathtmp, sizeof(pathtmp), (char *)dirName, strlen(dirName));
    if(ret != EOK){
        bsp_err("<%s> memcpy_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    ret = AccessCreate((char *)pathtmp, mode);
    if(ret)
    {
        bsp_err("<%s> AccessCreate failed.\n", __FUNCTION__);
            
    }
#else
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_mkdir(dirName, mode); /*lint !e734*/

    set_fs(old_fs);
#endif

    return ret;

}



s32 bsp_rmdir(s8 *path)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_rmdir(path);

    set_fs(old_fs);

    return ret;

}


s32 bsp_opendir(s8 *dirName)
{
    s32 handle = 0;

    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    handle = sys_open(dirName, RFILE_RDONLY|RFILE_DIRECTORY, 0);

    set_fs(old_fs);

    return handle;
}



s32 bsp_readdir(u32 fd, void  *dirent, u32 count)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_getdents64(fd, dirent, count);

    set_fs(old_fs);

    return ret;
}

s32 bsp_closedir(s32 pDir)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_close((u32)pDir);

    set_fs(old_fs);

    return ret;
}


s32 bsp_access(s8 *path, s32 mode)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_access(path, mode);

    set_fs(old_fs);

    return ret;
}


void rfile_TransStat(struct rfile_stat_stru *pstRfileStat, struct kstat *pRfileKstat)
{
    /* 兼容 32位和64位 */
    pstRfileStat->ino               = (u64)pRfileKstat->ino          ;
    pstRfileStat->dev               = (u32)pRfileKstat->dev          ;
    pstRfileStat->mode              = (u16)pRfileKstat->mode         ;
    pstRfileStat->nlink             = (u32)pRfileKstat->nlink        ;
    pstRfileStat->uid               = *(u32*)&pRfileKstat->uid       ;
    pstRfileStat->gid               = *(u32*)&pRfileKstat->gid       ;
    pstRfileStat->rdev              = (u32)pRfileKstat->rdev         ;
    pstRfileStat->size              = (u64)pRfileKstat->size         ;
    pstRfileStat->atime.tv_sec      = (u64)pRfileKstat->atime.tv_sec ;
    pstRfileStat->atime.tv_nsec     = (u64)pRfileKstat->atime.tv_nsec;
    pstRfileStat->ctime.tv_sec      = (u64)pRfileKstat->ctime.tv_sec ;
    pstRfileStat->mtime.tv_nsec     = (u64)pRfileKstat->mtime.tv_nsec;
    pstRfileStat->mtime.tv_sec      = (u64)pRfileKstat->mtime.tv_sec ;
    pstRfileStat->ctime.tv_nsec     = (u64)pRfileKstat->ctime.tv_nsec;
    pstRfileStat->blksize           = (u32)pRfileKstat->blksize      ;
    pstRfileStat->blocks            = (u64)pRfileKstat->blocks       ;
}


s32 bsp_stat(s8 *name, void *pStat)
{
    s32 ret;
    struct kstat kstattmp = {0};
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = vfs_stat(name, &kstattmp);
    if(0 == ret)
    {
        rfile_TransStat(pStat, &kstattmp);
    }

    set_fs(old_fs);

    return ret;
}



s32 bsp_rename( const char * oldname, const char * newname )
{
    s32 ret;
    unsigned long old_fs;

    if((NULL == oldname)||(NULL == newname))
    {
        return -1;
    }

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_rename(oldname,newname);

    set_fs(old_fs);

    return ret;
}



void rfile_IccSend(void *pdata, u32 len, u32 ulId)
{
    s32 ret, i;

    rfile_MntnDotRecord(__LINE__);

    for(i = 0; i < RFILE_MAX_SEND_TIMES; i++)
    {
        if(g_stRfileMain.eInitFlag != EN_RFILE_INIT_FINISH)
        {
            return;
        }

        ret = bsp_icc_send(((RFILE_CCORE_ICC_WR_CHAN == ulId)?ICC_CPU_MODEM:ICC_CPU_MCU), ulId, (u8*)pdata, len);

        if(ICC_INVALID_NO_FIFO_SPACE == ret)
        {
            /* buffer满，延时后重发 */
            RFILE_SLEEP(50);
            continue;
        }
		else if(BSP_ERR_ICC_CCORE_RESETTING == ret)
        {
            bsp_err("<%s> icc  cannot use.\n", __FUNCTION__);
        }
        else if(len != (u32)ret)
        {
            bsp_err("<%s> icc_send failed.\n", __FUNCTION__);
            return;
        }
        else
        {
            return;
        }
    }
}


/*lint -save -e64*/


s32 rfile_AcoreOpenReq(struct bsp_rfile_open_req *pstRfileReq, u32 ulId)
{
    u32 ulNameLen;
    s32 ret;
    BSP_CHAR *pcName;
    struct bsp_rfile_open_cnf stRfileCnf = {0};

#ifdef RFILE_AUTO_MKDIR
    char *p;
    char pathtmp[256] = {0};
#endif

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;
    stRfileCnf.ret = -1;

    ulNameLen = (u32)(pstRfileReq->nameLen) ;

    if(ulNameLen > 255)
    {
        goto rfile_AcoreFopenCnf;
    }

    pcName = Rfile_Malloc(ulNameLen);
    if(!pcName)
    {
        goto rfile_AcoreFopenCnf;
    }
    ret = memset_s((void*)pcName,ulNameLen,0,ulNameLen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    pcName[0] = '\0' ;
    ret = strncat_s(pcName,ulNameLen, (char*)pstRfileReq->aucData, (unsigned long)(pstRfileReq->nameLen));
    if(ret != EOK){
        bsp_err("<%s> strncat_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

#ifdef RFILE_AUTO_MKDIR

    ret = memcpy_s(pathtmp, sizeof(pathtmp), pcName, ulNameLen);/*lint !e713 */
    if(ret != EOK){
        bsp_err("<%s> memcpy_s err. ret =  %d.\n", __FUNCTION__, ret);
    }


    /* 路径中包含'/'并且不在根目录，则检查当前目录是否存在，不存在则创建目录 */
    p = strrchr(pathtmp, '/');
    if((NULL != p) && (p != pathtmp))
    {
        /* 查看上一级目录是否存在，如果不存在则创建此目录 */
        *p = '\0';
        ret = AccessCreate(pathtmp, pstRfileReq->mode);
        if(ret)
        {
            bsp_err("<%s> AccessCreate failed.\n", __FUNCTION__);
                
        }
    }
#endif
    RFILE_LPM_PRINT_PATH(EN_RFILE_OP_OPEN, pcName);

    stRfileCnf.ret = bsp_open(pcName, pstRfileReq->flags, pstRfileReq->mode);

    if(stRfileCnf.ret >= 0)
    {
        rfile_FpListAdd(stRfileCnf.ret, pcName);
    }

    Rfile_Free(pcName);

rfile_AcoreFopenCnf:

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}



s32 rfile_AcoreCloseReq(struct bsp_rfile_close_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_common_cnf stRfileCnf = {0};

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;
    stRfileCnf.ret = BSP_ERROR;

    RFILE_LPM_PRINT_FILEPATH(EN_RFILE_OP_CLOSE, pstRfileReq->fd);

    stRfileCnf.ret = bsp_close(pstRfileReq->fd);

    rfile_FpListDel((s32)pstRfileReq->fd);

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}



s32 rfile_AcoreWriteReq(struct bsp_rfile_write_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_common_cnf stRfileCnf = {0};
    s32 ret;

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;
    stRfileCnf.ret = BSP_ERROR;

    RFILE_LPM_PRINT_FILEPATH(EN_RFILE_OP_WRITE, pstRfileReq->fd);

    stRfileCnf.ret = bsp_write(pstRfileReq->fd, (s8*)pstRfileReq->aucData, pstRfileReq->ulSize);

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    ret = memset_s((void*)pstRfileReq,RFILE_LEN_MAX,0,(pstRfileReq->ulSize+sizeof(struct bsp_rfile_write_req)));
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    return BSP_OK;
}


s32 rfile_AcoreWriteSyncReq(struct bsp_rfile_write_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_common_cnf stRfileCnf = {0};
    s32 ret;

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;
    stRfileCnf.ret = BSP_ERROR;

    RFILE_LPM_PRINT_FILEPATH(EN_RFILE_OP_WRITE_SYNC, pstRfileReq->fd);


    stRfileCnf.ret = bsp_write_sync(pstRfileReq->fd, (s8*)pstRfileReq->aucData, pstRfileReq->ulSize);

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    ret = memset_s((void*)pstRfileReq,RFILE_LEN_MAX,0,(pstRfileReq->ulSize+sizeof(struct bsp_rfile_write_req)));
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    return BSP_OK;
}



s32 rfile_AcoreReadReq(struct bsp_rfile_read_req *pstRfileReq, u32 ulId)
{
    u32 ulLen;
    s32 ret;

    struct bsp_rfile_read_cnf *pstRfileCnf;

    rfile_MntnDotRecord(__LINE__);
    if((u32)pstRfileReq->ulSize > RFILE_LEN_MAX)
    {
        bsp_err("<%s> pstRfileCnf->Size %d > RFILE_LEN_MAX.\n", __FUNCTION__, pstRfileReq->ulSize);
            
        return BSP_ERROR;
    }
    ulLen = sizeof(struct bsp_rfile_read_cnf) + pstRfileReq->ulSize;

    pstRfileCnf = Rfile_Malloc(ulLen);
    if(!pstRfileCnf)
    {
        bsp_err("<%s> Rfile_Malloc failed.\n", __FUNCTION__);
        return BSP_ERROR;
    }
    ret = memset_s((void*)pstRfileCnf,ulLen,0,ulLen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    pstRfileCnf->opType = pstRfileReq->opType;
    pstRfileCnf->pstlist = pstRfileReq->pstlist;

    RFILE_LPM_PRINT_FILEPATH(EN_RFILE_OP_READ, pstRfileReq->fd);

    pstRfileCnf->Size = bsp_read(pstRfileReq->fd, (s8*)pstRfileCnf->aucData, pstRfileReq->ulSize);

    /* 由C核请求的地方保证读取的数据长度不超过ICC最大长度限制 */

    rfile_IccSend(pstRfileCnf, ulLen, ulId);

    ret = memset_s((void*)pstRfileCnf,ulLen,0,ulLen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    Rfile_Free(pstRfileCnf);

    return BSP_OK;
}


s32 rfile_AcoreSeekReq(struct bsp_rfile_seek_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_common_cnf stRfileCnf = {0};

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;

    RFILE_LPM_PRINT_FILEPATH(EN_RFILE_OP_SEEK, pstRfileReq->fd);

    stRfileCnf.ret = bsp_lseek(pstRfileReq->fd, (long)pstRfileReq->offset, pstRfileReq->whence);

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}



s32 rfile_AcoreTellReq(struct bsp_rfile_tell_req *pstRfileReq, u32 ulId)
{
    s32 ret;
    struct bsp_rfile_common_cnf stRfileCnf = {0};

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;

    RFILE_LPM_PRINT_FILEPATH(EN_RFILE_OP_TELL, pstRfileReq->fd);

    ret = (s32)bsp_tell(pstRfileReq->fd);

    if(0!=ret)
    {
    }

    stRfileCnf.ret = (s32)ret;

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}


s32 rfile_AcoreRemoveReq(struct bsp_rfile_remove_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_common_cnf stRfileCnf = {0};
    BSP_CHAR *pcPath;
    u32 ulPathLen;
    s32 ret;

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;

    ulPathLen =  (u32)(pstRfileReq->pathLen) ;

    if(ulPathLen > 255)
    {
        goto rfile_AcoreRemoveCnf;
    }

    pcPath = Rfile_Malloc(ulPathLen);
    if(BSP_NULL == pcPath)
    {
        goto rfile_AcoreRemoveCnf;
    }
    ret = memset_s((void*)pcPath,ulPathLen,0,ulPathLen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    pcPath[0] = '\0' ;
    ret = strncat_s(pcPath,ulPathLen, (char*)pstRfileReq->aucData, (unsigned long)(pstRfileReq->pathLen));
    if(ret != EOK){
        bsp_err("<%s> strncat_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    RFILE_LPM_PRINT_PATH(EN_RFILE_OP_REMOVE, pcPath);

    stRfileCnf.ret = bsp_remove((s8*)pcPath);

    Rfile_Free(pcPath);

rfile_AcoreRemoveCnf:

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}



s32 rfile_AcoreMkdirReq(struct bsp_rfile_mkdir_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_common_cnf stRfileCnf = {0};
    BSP_CHAR *pcPath;
    u32 ulPathLen;
    s32 ret;

#ifdef RFILE_AUTO_MKDIR
    char pathtmp[255+1] = {0};
#endif

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;
    stRfileCnf.ret = BSP_ERROR;

    ulPathLen = (u32)(pstRfileReq->pathLen) ;

    if(ulPathLen > 255)
    {
        goto rfile_AcoreMkdirCnf;
    }

    pcPath = Rfile_Malloc(ulPathLen);
    if(BSP_NULL == pcPath)
    {
        goto rfile_AcoreMkdirCnf;
    }
    ret = memset_s((void*)pcPath,ulPathLen,0,ulPathLen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    pcPath[0] = '\0' ;
    ret = strncat_s(pcPath, ulPathLen,(char*)pstRfileReq->aucData, (unsigned long)(pstRfileReq->pathLen));
    if(ret != EOK){
        bsp_err("<%s> strncat_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    RFILE_LPM_PRINT_PATH(EN_RFILE_OP_MKDIR, pcPath);


#ifdef RFILE_AUTO_MKDIR

    ret = memcpy_s(pathtmp, sizeof(pathtmp), pcPath, ulPathLen);/*lint !e713 */
    if(ret != EOK){
        bsp_err("<%s> memcpy_s err. ret =  %d.\n", __FUNCTION__, ret);
    }


    stRfileCnf.ret = AccessCreate((char *)pathtmp, pstRfileReq->mode);
    if(stRfileCnf.ret)
    {
        bsp_err("<%s> AccessCreate failed.\n", __FUNCTION__);
            
    }
#else
    stRfileCnf.ret = bsp_mkdir(pcPath, pstRfileReq->mode); /*lint !e734*/
#endif
    Rfile_Free(pcPath);

rfile_AcoreMkdirCnf:

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}


s32 rfile_AcoreRmdirReq(struct bsp_rfile_rmdir_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_common_cnf stRfileCnf = {0};
    BSP_CHAR *pcPath;
    u32 ulPathLen;
    s32 ret;

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;
    stRfileCnf.ret = BSP_ERROR;

    ulPathLen =  (u32)(pstRfileReq->pathLen) ;

    if(ulPathLen > 255)
    {
        goto rfile_AcoreRmdirCnf;
    }

    pcPath = Rfile_Malloc(ulPathLen);
    if(BSP_NULL == pcPath)
    {
        goto rfile_AcoreRmdirCnf;
    }
    ret = memset_s((void*)pcPath,ulPathLen,0,ulPathLen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    pcPath[0] = '\0' ;
    ret = strncat_s(pcPath,ulPathLen ,(char*)pstRfileReq->aucData, (unsigned long)(pstRfileReq->pathLen));
    if(ret != EOK){
        bsp_err("<%s> strncat_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    RFILE_LPM_PRINT_PATH(EN_RFILE_OP_RMDIR, pcPath);

    stRfileCnf.ret = bsp_rmdir(pcPath);

    Rfile_Free(pcPath);

rfile_AcoreRmdirCnf:

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}



s32 rfile_AcoreOpendirReq(struct bsp_rfile_opendir_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_opendir_cnf stRfileCnf = {0};
    BSP_CHAR *pcPath;
    u32 ulPathLen;
    s32 ret;

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;
    stRfileCnf.dirhandle = BSP_ERROR;

    ulPathLen =  (u32)(pstRfileReq->nameLen) ;

    if(ulPathLen > 255)
    {
        goto rfile_AcoreOpendirCnf;
    }

    pcPath = Rfile_Malloc(ulPathLen);
    if(BSP_NULL == pcPath)
    {
        goto rfile_AcoreOpendirCnf;
    }
    ret = memset_s((void*)pcPath,ulPathLen,0,ulPathLen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    pcPath[0] = '\0' ;
    ret = strncat_s(pcPath, ulPathLen,(char*)pstRfileReq->aucData, (unsigned long)(pstRfileReq->nameLen));
    if(ret != EOK){
        bsp_err("<%s> strncat_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    RFILE_LPM_PRINT_PATH(EN_RFILE_OP_OPENDIR, pcPath);

    stRfileCnf.dirhandle = bsp_opendir(pcPath);

    if(stRfileCnf.dirhandle >= 0)
    {
        rfile_DpListAdd(stRfileCnf.dirhandle, pcPath);
    }

    Rfile_Free(pcPath);

rfile_AcoreOpendirCnf:

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}



s32 rfile_AcoreReaddirReq(struct bsp_rfile_readdir_req *pstRfileReq, u32 ulId)
{
    u32 ulLen;
    s32 ret;
    struct bsp_rfile_readdir_cnf *pstRfileCnf = NULL;

    rfile_MntnDotRecord(__LINE__);

    if(pstRfileReq->count >RFILE_RD_LEN_MAX)
    {
        return BSP_ERROR;
    }

    ulLen = sizeof(struct bsp_rfile_readdir_cnf) + pstRfileReq->count;

    pstRfileCnf = Rfile_Malloc(ulLen);
    if(!pstRfileCnf)
    {
        bsp_err("<%s> Rfile_Malloc failed.\n", __FUNCTION__);
        return BSP_ERROR;
    }
    ret = memset_s((void*)pstRfileCnf,ulLen,0,ulLen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    pstRfileCnf->opType = pstRfileReq->opType;
    pstRfileCnf->pstlist = pstRfileReq->pstlist;

    RFILE_LPM_PRINT_DIRPATH(EN_RFILE_OP_READDIR, pstRfileReq->dir);

    pstRfileCnf->Size = bsp_readdir(pstRfileReq->dir, (struct linux_dirent *)(pstRfileCnf->aucData), pstRfileReq->count);

    rfile_IccSend(pstRfileCnf, ulLen, ulId);

    Rfile_Free(pstRfileCnf);

    return BSP_OK;
}



s32 rfile_AcoreClosedirReq(struct bsp_rfile_closedir_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_common_cnf stRfileCnf = {0};

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;

    RFILE_LPM_PRINT_DIRPATH(EN_RFILE_OP_CLOSEDIR, pstRfileReq->dir);

    stRfileCnf.ret = bsp_closedir(pstRfileReq->dir);

    rfile_DpListDel(pstRfileReq->dir);

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}



s32 rfile_AcoreStatReq(struct bsp_rfile_stat_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_stat_cnf stRfileCnf = {0};
    BSP_CHAR *pcPath;
    u32 ulPathLen;
    s32 ret;

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;
    stRfileCnf.ret = BSP_ERROR;

    ulPathLen =  pstRfileReq->ulSize ;

    if(ulPathLen > 255)
    {
        goto rfile_AcoreStatCnf;
    }


    pcPath = Rfile_Malloc(ulPathLen);
    if(!pcPath)
    {
        goto rfile_AcoreStatCnf;
    }
    ret = memset_s((void*)pcPath,ulPathLen,0,ulPathLen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    pcPath[0] = '\0' ;
    ret = strncat_s(pcPath,ulPathLen ,(char*)pstRfileReq->aucData, (unsigned long)(pstRfileReq->ulSize));
    if(ret != EOK){
        bsp_err("<%s> strncat_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    RFILE_LPM_PRINT_PATH(EN_RFILE_OP_STAT, pcPath);

    stRfileCnf.ret = bsp_stat(pcPath, &(stRfileCnf.ststat));    /*lint !e740*/

    Rfile_Free(pcPath);

rfile_AcoreStatCnf:

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}



s32 rfile_AcoreRenameReq(struct bsp_rfile_rename_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_rename_cnf stRfileCnf = {0};
    char * oldname;
    char * newname;
    u32 uloldnamelen, ulnewnamelen;
    s32 ret;

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;
    stRfileCnf.ret = BSP_ERROR;

    uloldnamelen = (u32)(strlen((char*)(pstRfileReq->aucData)) + 1);
    if(uloldnamelen >= pstRfileReq->ulSize)
    {
        goto rfile_AcoreRenameCnf;
    }

    ulnewnamelen = (u32)(strlen((char*)(pstRfileReq->aucData+ uloldnamelen)) + 1);

    if((uloldnamelen + ulnewnamelen) != pstRfileReq->ulSize)
    {
        goto rfile_AcoreRenameCnf;
    }

    oldname = Rfile_Malloc(uloldnamelen);
    if(!oldname)
    {
        goto rfile_AcoreRenameCnf;
    }
    ret = memset_s((void*)oldname,uloldnamelen,0,uloldnamelen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    newname = Rfile_Malloc(ulnewnamelen);
    if(!newname)
    {
        Rfile_Free(oldname);
        goto rfile_AcoreRenameCnf;
    }
    ret = memset_s((void*)newname,ulnewnamelen,0,ulnewnamelen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    oldname[0] = '\0' ;
    ret = strncat_s(oldname,uloldnamelen ,(char*)pstRfileReq->aucData, (unsigned long)(strlen((char*)(pstRfileReq->aucData)) + 1));
    if(ret != EOK){
        bsp_err("<%s> strncat_s err. ret =  %d.\n", __FUNCTION__, ret);
    }
    newname[0] = '\0' ;
    ret = strncat_s(newname,ulnewnamelen ,(char*)(pstRfileReq->aucData + uloldnamelen ), (unsigned long)(strlen((char*)(pstRfileReq->aucData+ uloldnamelen)) + 1));
    if(ret != EOK){
        bsp_err("<%s> strncat_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    RFILE_LPM_PRINT_PATH(EN_RFILE_OP_RENAME, newname);

    stRfileCnf.ret = bsp_rename(oldname, newname);

    Rfile_Free(oldname);
    Rfile_Free(newname);

rfile_AcoreRenameCnf:

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}



s32 rfile_AcoreAccessReq(struct bsp_rfile_access_req *pstRfileReq, u32 ulId)
{
    struct bsp_rfile_common_cnf stRfileCnf = {0};
    BSP_CHAR *pcPath;
    u32 ulPathLen;
    s32 ret;

    rfile_MntnDotRecord(__LINE__);

    stRfileCnf.opType = pstRfileReq->opType;
    stRfileCnf.pstlist = pstRfileReq->pstlist;
    stRfileCnf.ret = BSP_ERROR;

    ulPathLen =  (u32)pstRfileReq->pathlen ;

    if(ulPathLen > 255)
    {
        goto rfile_AcoreAccessCnf;
    }

    pcPath = Rfile_Malloc(ulPathLen);
    if(!pcPath)
    {
        goto rfile_AcoreAccessCnf;
    }
    ret = memset_s((void*)pcPath,ulPathLen,0,ulPathLen);
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    pcPath[0] = '\0' ;
    ret = strncat_s(pcPath,ulPathLen, (char*)pstRfileReq->aucData, (unsigned long)(pstRfileReq->pathlen));
    if(ret != EOK){
        bsp_err("<%s> strncat_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    RFILE_LPM_PRINT_PATH(EN_RFILE_OP_ACCESS, pcPath);

    stRfileCnf.ret = bsp_access(pcPath, pstRfileReq->mode);

    Rfile_Free(pcPath);

rfile_AcoreAccessCnf:

    rfile_IccSend(&stRfileCnf, sizeof(stRfileCnf), ulId);

    return BSP_OK;
}



s32 rfile_AcoreMassrdReq(struct bsp_rfile_massread_req *pstRfileReq, u32 ulId)
{
    return BSP_ERROR;
}



s32 rfile_AcoreMasswrReq(struct bsp_rfile_masswrite_req *pstRfileReq, u32 ulId)
{
     return BSP_ERROR;
}
/*lint -restore*/



s32 bsp_RfileCallback(u32 channel_id, u32 len, void *context)
{/*lint --e{454}*/
    rfile_MntnDotRecord(__LINE__);

    if((RFILE_CCORE_ICC_RD_CHAN != channel_id) && (RFILE_MCORE_ICC_RD_CHAN != channel_id))
    {
        bsp_err("<%s> channel_id %d error.\n", __FUNCTION__, channel_id);

        return BSP_ERROR;
    }

    /* 如果rfile未初始化则利用icc的缓存机制保存数据 */
    if(EN_RFILE_INIT_FINISH != g_stRfileMain.eInitFlag)
    {
        bsp_wrn("<%s> initflag %d.\n",__FUNCTION__, g_stRfileMain.eInitFlag);
            

        return BSP_OK;
    }
    __pm_stay_awake(&g_stRfileMain.wake_lock);
    osl_sem_up(&g_stRfileMain.semTask);

    return BSP_OK;
}

void rfile_ResetProc(void)
{
    int ret;
    struct list_head *p = 0;
    struct list_head *n = 0;
    struct fp_list  *tmpfp;
    struct dir_list *tmpdir;

    /*close dir or files*/

    rfile_MntnDotRecord(__LINE__);

    list_for_each_safe(p,n,&(g_stRfileMain.fplist))
    {
        tmpfp = list_entry(p, struct fp_list ,stlist);

        ret = bsp_close(tmpfp->fp);
        if(ret != BSP_OK)
        {
            bsp_err( "[reset]: <%s> bsp_close fp  failed.\n", __FUNCTION__);
               
        }

        list_del(&tmpfp->stlist);
        Rfile_Free(tmpfp);
    }

    list_for_each_safe(p,n,&(g_stRfileMain.dplist))
    {
        tmpdir = list_entry(p, struct dir_list ,stlist);
        ret = bsp_closedir(tmpdir->dp);
        if(ret != BSP_OK)
        {
            bsp_err("[reset]: <%s> bsp_close dir failed.\n", __FUNCTION__);
                
        }

        list_del(&tmpdir->stlist);
        Rfile_Free(tmpdir);
    }

}
s32 rfile_AcoreReqFunc_part2(u32 enOptype, u32 channel_id)
{
    switch (enOptype) {
        case EN_RFILE_OP_OPENDIR: {
            return rfile_AcoreOpendirReq((struct bsp_rfile_opendir_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_READDIR: {
            return rfile_AcoreReaddirReq((struct bsp_rfile_readdir_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_CLOSEDIR: {
            return rfile_AcoreClosedirReq((struct bsp_rfile_closedir_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_STAT: {
            return rfile_AcoreStatReq((struct bsp_rfile_stat_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_ACCESS: {
            return rfile_AcoreAccessReq((struct bsp_rfile_access_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_MASSRD: {
            return rfile_AcoreMassrdReq((struct bsp_rfile_massread_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_MASSWR: {
            return rfile_AcoreMasswrReq((struct bsp_rfile_masswrite_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_RENAME: {
            return rfile_AcoreRenameReq((struct bsp_rfile_rename_req *)g_stRfileMain.data, channel_id);
        }

        default: {
            bsp_fatal("<%s> enOptype %d  is bigger than EN_RFILE_OP_BUTT.\n", __FUNCTION__, enOptype);
            return BSP_ERROR;
        }
    }
}

s32 rfile_AcoreReqFunc_part1(u32 enOptype, u32 channel_id)
{
    switch (enOptype) {
        case EN_RFILE_OP_OPEN: {
            return rfile_AcoreOpenReq((struct bsp_rfile_open_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_CLOSE: {
            return rfile_AcoreCloseReq((struct bsp_rfile_close_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_WRITE: {
            return rfile_AcoreWriteReq((struct bsp_rfile_write_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_WRITE_SYNC: {
            return rfile_AcoreWriteSyncReq((struct bsp_rfile_write_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_READ: {
            return rfile_AcoreReadReq((struct bsp_rfile_read_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_SEEK: {
            return rfile_AcoreSeekReq((struct bsp_rfile_seek_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_TELL: {
            return rfile_AcoreTellReq((struct bsp_rfile_tell_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_REMOVE: {
            return rfile_AcoreRemoveReq((struct bsp_rfile_remove_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_MKDIR: {
            return rfile_AcoreMkdirReq((struct bsp_rfile_mkdir_req *)g_stRfileMain.data, channel_id);
        }
        case EN_RFILE_OP_RMDIR: {
            return rfile_AcoreRmdirReq((struct bsp_rfile_rmdir_req *)g_stRfileMain.data, channel_id);
        }
        default: {
            return rfile_AcoreReqFunc_part2(enOptype, channel_id);
        }
    }
}


/*lint -save -e716*/

s32 rfile_TaskProc(void* obj)
{
    s32 ret;
    u32 enOptype;
    u32 channel_id;

    bsp_debug("<%s> entry.\n", __FUNCTION__);

    while(1)
    {
        osl_sem_down(&g_stRfileMain.semTask);

        if(EN_RFILE_INIT_SUSPEND == g_stRfileMain.eInitFlag)
        {
            /* 置为idle，不再处理新请求 */
            g_stRfileMain.opState = EN_RFILE_IDLE;

            /* 关闭文件，并阻塞单独复位任务完成 */
            rfile_ResetProc();
            g_stRfileMain.eInitFlag = EN_RFILE_INIT_SUSPEND_WAIT;
            osl_sem_up(&g_stRfileMain.semCloseFps);

            continue;
        }

        channel_id = RFILE_CCORE_ICC_RD_CHAN;
        /*未初始化完成或者处于睡眠状态则利用icc缓冲请求数据*/
        if(g_stRfileMain.eInitFlag != EN_RFILE_INIT_FINISH)
        {
            continue;
        }

        g_stRfileMain.opState = EN_RFILE_DOING;
        __pm_stay_awake(&g_stRfileMain.wake_lock);
        if(g_stRfileMain.pmState == EN_RFILE_SLEEP_STATE)
        {
            bsp_err("%s cur state in sleeping,wait for resume end!\n",__func__);
            continue;
        }

        /* 读取ICC-C通道，输入的长度是buff的size，返回值是实际读取的数据长度 */
        ret = bsp_icc_read(channel_id, g_stRfileMain.data, RFILE_LEN_MAX);
        if(((u32)ret > RFILE_LEN_MAX) || (ret <= 0))
        {
            bsp_debug("<%s> icc_read %d.\n", __FUNCTION__, ret);
            __pm_relax(&g_stRfileMain.wake_lock);
            g_stRfileMain.opState = EN_RFILE_IDLE;
            continue;   /* A-C通道没读到数据 */
        }


        /* 请求的第一个四字节对应的是 op type */
        enOptype = *(u32*)(g_stRfileMain.data);

        if(enOptype >= EN_RFILE_OP_BUTT)
        {
            bsp_fatal("<%s> enOptype %d  is bigger than EN_RFILE_OP_BUTT.\n", __FUNCTION__, enOptype);
        }
        else
        {
            ret = rfile_AcoreReqFunc_part1(enOptype, channel_id);
            if(BSP_OK != ret)
            {
                bsp_err("<%s> pFun failed %d.\n", __FUNCTION__, enOptype);
            }
        }
        __pm_relax(&g_stRfileMain.wake_lock);

        /* 处理结束后避免ICC通道中有缓存，再次启动读取 */
        osl_sem_up(&g_stRfileMain.semTask);
    }
}

/*lint -restore*/


s32 bsp_rfile_reset_cb(DRV_RESET_CB_MOMENT_E eparam, s32 userdata)    /*lint !e830*/
{
    if(bsp_fs_ok())
    {
         bsp_err("[reset]<%s> pFun failed  rfile states: EN_RFILE_INIT_INVALID \n", __FUNCTION__);
         return -1;
    }

    if(MDRV_RESET_CB_BEFORE == eparam)
    {
        /* 设置为suspend状态，待close打开的文件、目录后恢复为FINISH状态 */
        g_stRfileMain.eInitFlag = EN_RFILE_INIT_SUSPEND;

        /* 启动任务中的close处理 */
        osl_sem_up(&g_stRfileMain.semTask);

        osl_sem_down(&g_stRfileMain.semCloseFps);
    }

    if(MDRV_RESET_RESETTING == eparam)
    {
        /* 此时icc缓存已清空，可以接收复位后的C核rfile请求，设置为finish状态 */
        g_stRfileMain.eInitFlag = EN_RFILE_INIT_FINISH;
		/* 并主动唤醒任务，避免C核启动过程中请求丢失 */
        osl_sem_up(&g_stRfileMain.semTask);
    }

    return 0;
}


s32 bsp_rfile_init(void)
{
    s32 ret;
    struct sched_param sch_para;
    sch_para.sched_priority = 15;

    bsp_err("[init]start.\n");

    osl_sem_init(0, &(g_stRfileMain.semTask));
    osl_sem_init(0, &(g_stRfileMain.semCloseFps));

    wakeup_source_init(&g_stRfileMain.wake_lock, "rfile_wakelock");

    g_stRfileMain.taskid = kthread_run(rfile_TaskProc, BSP_NULL, "rfile");
    if (IS_ERR(g_stRfileMain.taskid))
    {
        bsp_err("[init]: <%s> kthread_run failed.\n", __FUNCTION__);
        return BSP_ERROR;
    }

    if (BSP_OK != sched_setscheduler(g_stRfileMain.taskid, SCHED_FIFO, &sch_para))
    {
        bsp_err("[init]: <%s> sched_setscheduler failed.\n", __FUNCTION__);
        return BSP_ERROR;
    }

    INIT_LIST_HEAD(&g_stRfileMain.fplist);
    INIT_LIST_HEAD(&g_stRfileMain.dplist);

    ret = memset_s((void*)&g_stRfileMntnInfo,sizeof(g_stRfileMntnInfo),0, sizeof(g_stRfileMntnInfo));
    if(ret != EOK){
        bsp_err("<%s> memset_s err. ret =  %d.\n", __FUNCTION__, ret);
    }

    g_stRfileMain.eInitFlag = EN_RFILE_INIT_FINISH;

    bsp_icc_debug_register(RFILE_CCORE_ICC_RD_CHAN, rfile_lpmcallback, 0);

    ret = bsp_icc_event_register(RFILE_CCORE_ICC_RD_CHAN, bsp_RfileCallback, NULL, NULL, NULL);
    if(ret)
    {
        bsp_err("[init]: <%s> bsp_icc_event_register failed.\n", __FUNCTION__);
        return BSP_ERROR;
    }

    ret = bsp_icc_event_register(RFILE_MCORE_ICC_RD_CHAN, bsp_RfileCallback, NULL, NULL, NULL);
    if(ret)
    {
        bsp_err("[init]: <%s> bsp_icc_event_register MCORE failed.\n", __FUNCTION__);
        return BSP_ERROR;
    }
    adp_rfile_init();

    bsp_err("[init]ok.\n");

    return BSP_OK;
}


s32 bsp_rfile_release(void)    /*lint !e830*/
{
    s32 ret;

    bsp_debug("<%s> entry.\n", __FUNCTION__);

    g_stRfileMain.eInitFlag = EN_RFILE_INIT_INVALID;

    osl_sema_delete(&g_stRfileMain.semTask);

    kthread_stop(g_stRfileMain.taskid);

    ret = bsp_icc_event_unregister(RFILE_CCORE_ICC_RD_CHAN);
    if(ret)
    {
        bsp_err("<%s> bsp_icc_event_unregister failed.\n", __FUNCTION__);
        return BSP_ERROR;
    }

    return BSP_OK;
}


EXPORT_SYMBOL(bsp_open);
EXPORT_SYMBOL(bsp_access);
EXPORT_SYMBOL(bsp_close);
EXPORT_SYMBOL(bsp_write);
EXPORT_SYMBOL(bsp_read);
EXPORT_SYMBOL(bsp_lseek);
EXPORT_SYMBOL(bsp_tell);
EXPORT_SYMBOL(bsp_remove);
EXPORT_SYMBOL(bsp_mkdir);
EXPORT_SYMBOL(bsp_rmdir);
EXPORT_SYMBOL(bsp_opendir);
EXPORT_SYMBOL(bsp_readdir);
EXPORT_SYMBOL(bsp_closedir);
EXPORT_SYMBOL(bsp_stat);

#if (FEATURE_ON == FEATURE_DELAY_MODEM_INIT)

static int  modem_rfile_probe(struct platform_device *dev)
{
    int ret;

    g_stRfileMain.pmState = EN_RFILE_WAKEUP_STATE;
    g_stRfileMain.opState = EN_RFILE_IDLE;

    ret= bsp_rfile_init();

    return ret;
}

static void  modem_rfile_shutdown(struct platform_device *dev)
{
    bsp_info("%s shutdown start \n",__func__);

    g_stRfileMain.eInitFlag = EN_RFILE_INIT_INVALID;
}
#ifdef CONFIG_PM
static s32 modem_rfile_suspend(struct device *dev)
{
    static s32 count = 0;
    if(g_stRfileMain.opState != EN_RFILE_IDLE)
    {
        bsp_err("[SR] %s modem rfile is in doing!\n",__func__);
        return -1;
    }
    g_stRfileMain.pmState = EN_RFILE_SLEEP_STATE;
    bsp_info("[SR]modem rfile enter suspend! %d times \n",++count);
    return 0;
}
static s32 modem_rfile_resume(struct device *dev)
{
    static s32 count = 0;
    g_stRfileMain.pmState = EN_RFILE_WAKEUP_STATE;
    if(g_stRfileMain.opState == EN_RFILE_DOING)
    {
        bsp_err("[SR]%s need to enter task proc!\n",__func__);
        osl_sem_up(&g_stRfileMain.semTask);
    }
    bsp_info("[SR]modem rfile enter resume! %d times \n",++count);
    return 0;
}
static const struct dev_pm_ops modem_rfile_pm_ops ={
	.suspend = modem_rfile_suspend,
    .resume  = modem_rfile_resume,
};

#define BALONG_RFILE_PM_OPS (&modem_rfile_pm_ops)
#else
#define BALONG_RFILE_PM_OPS  NULL
#endif
static struct platform_driver modem_rfile_drv = {
    .probe      = modem_rfile_probe,
    .shutdown   = modem_rfile_shutdown,
    .driver     = {
        .name     = "modem_rfile",
        .owner    = THIS_MODULE,
        .pm       = BALONG_RFILE_PM_OPS,
    },
};


static struct platform_device modem_rfile_device = {
    .name = "modem_rfile",
    .id = 0,
    .dev = {
    .init_name = "modem_rfile",
    },
};


int modem_rfile_init(void)
{
    int ret;

    ret = platform_device_register(&modem_rfile_device);
    if(ret)
    {
        bsp_err("[init]platform_device_register modem_rfile_device fail !\n");
        return -1;
    }

    ret = platform_driver_register(&modem_rfile_drv);
    if(ret)
    {
        bsp_err("[init]platform_device_register modem_rfile_drv fail !\n");
        platform_device_unregister(&modem_rfile_device);
        return -1;
    }

    return 0;
}

void  modem_rfile_exit(void)
{
    platform_device_unregister(&modem_rfile_device);
    platform_driver_unregister(&modem_rfile_drv);
}

//module_init(modem_rfile_init);
//module_exit(modem_rfile_exit);

#else
module_init(bsp_rfile_init);
#endif




