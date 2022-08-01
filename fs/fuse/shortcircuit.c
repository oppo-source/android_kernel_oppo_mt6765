// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "fuse_shortcircuit.h"

#include <linux/aio.h>
#include <linux/fs_stack.h>

void fuse_setup_shortcircuit(struct fuse_conn *fc, struct fuse_req *req)
{
	int  fd, flags, open_out_index;
	struct file *rw_lower_file = NULL;
	struct fuse_open_out *open_out;

	req->private_lower_rw_file = NULL;

	if (!(fc->shortcircuit_io))
		return;

	if ((req->in.h.opcode != FUSE_OPEN) &&
		(req->in.h.opcode != FUSE_CREATE))
		return;

	open_out_index = req->in.numargs - 1;

	if ((open_out_index != 0 && open_out_index != 1) ||
		(req->out.args[open_out_index].size != sizeof(*open_out)))
		return;

	open_out = req->out.args[open_out_index].value;
	if (!open_out->fh)
		return;

	flags = open_out->open_flags;
	if ((flags & FOPEN_DIRECT_IO) || !(flags & FOPEN_KEEP_CACHE)) {
		pr_info("fuse bypass sct #flags:%d\n", flags);
		return;
	}

	if (get_user(fd, (int __user *)open_out->fh))
		return;

	if (fd <= 1 || fd >= current->signal->rlim[RLIMIT_NOFILE].rlim_max) {
		pr_info("fuse bypass sct:%d, %d\n", fd, flags);
		return;
	}

	rw_lower_file = fget_raw(fd);
	if (!rw_lower_file)
		return;

	req->private_lower_rw_file = rw_lower_file;
}

static ssize_t fuse_shortcircuit_read_write_iter(struct kiocb *iocb,
						 struct iov_iter *iter,
						 int do_write)
{
	struct file *fuse_filp = iocb->ki_filp;
	struct fuse_file *ff = fuse_filp->private_data;
	struct file *lower_file = ff->rw_lower_file;
	struct inode *fuse_inode, *shortcircuit_inode;
	ssize_t ret = -EIO;
	fuse_inode = fuse_filp->f_path.dentry->d_inode;
	shortcircuit_inode = file_inode(lower_file);
	iocb->ki_filp = lower_file;
	if (do_write) {
		if (!lower_file->f_op->write_iter)
			goto out;
		ret = call_write_iter(lower_file, iocb, iter);
		if (ret >= 0 || ret == -EIOCBQUEUED) {
			fsstack_copy_inode_size(fuse_inode, shortcircuit_inode);
			fsstack_copy_attr_times(fuse_inode, shortcircuit_inode);
		}
	} else {
		if (!lower_file->f_op->read_iter)
			goto out;
		ret = call_read_iter(lower_file, iocb, iter);
		if (ret >= 0 || ret == -EIOCBQUEUED)
			fsstack_copy_attr_atime(fuse_inode, shortcircuit_inode);
	}
out:
	iocb->ki_filp = fuse_filp;

	return ret;
}

ssize_t fuse_shortcircuit_read_iter(struct kiocb *iocb, struct iov_iter *to)
{
	return fuse_shortcircuit_read_write_iter(iocb, to, 0);
}

ssize_t fuse_shortcircuit_write_iter(struct kiocb *iocb, struct iov_iter *from)
{
	return fuse_shortcircuit_read_write_iter(iocb, from, 1);
}

ssize_t fuse_shortcircuit_mmap(struct file *file, struct vm_area_struct *vma)
{
	int ret;
	struct fuse_file *ff = file->private_data;
	struct file *lower_filp = ff->rw_lower_file;
	struct inode *fuse_inode = file_inode(file);
	struct inode *lower_inode = file_inode(lower_filp);

	if (!lower_filp->f_op->mmap)
		return -ENODEV;

	if (WARN_ON(file != vma->vm_file))
		return -EIO;

	vma->vm_file = get_file(lower_filp);
	ret = call_mmap(vma->vm_file, vma);

	if (ret)
		fput(lower_filp);
	else
		fput(file);

	if (lower_inode && !(file->f_flags & O_NOATIME)) {
		if ((!timespec64_equal(&fuse_inode->i_mtime, &lower_inode->i_mtime) ||
		     !timespec64_equal(&fuse_inode->i_ctime, &lower_inode->i_ctime))) {
			fuse_inode->i_mtime = lower_inode->i_mtime;
			fuse_inode->i_ctime = lower_inode->i_ctime;
		}
		touch_atime(&file->f_path);
	}

	return ret;
}

void fuse_shortcircuit_release(struct fuse_file *ff)
{
	if (!(ff->rw_lower_file))
		return;

	/* Release the lower file. */
	fput(ff->rw_lower_file);
	ff->rw_lower_file = NULL;
}
