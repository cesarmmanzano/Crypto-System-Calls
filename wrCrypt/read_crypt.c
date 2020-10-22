#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/stat.h>
#include <linux/crypto.h>
#include <linux/random.h>
#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <crypto/skcipher.h>
#include <linux/err.h>
#include <linux/vmalloc.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/file.h>
#include <linux/fcntl.h>
#include <linux/slab.h>

asmlinkage ssize_t sys_read_crypt(int fd, const void *buf, size_t nbytes)
{	
	return 1;
}

/* ================================================== */

void clearMessage(char message[])
{
	int i;
	for (i = 0; i < strlen(message); i++)
	{
		message[i] = '\0';
	}
}
