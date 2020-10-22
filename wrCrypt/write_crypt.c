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

asmlinkage ssize_t sys_write_crypt(int fd, const void *buf, size_t nbytes)
{
	char message[256];
	mm_segment_t old_fs;
	
	sprintf(message, "%s", buf);
	printk("<<MSG>>: [%s]", message);
	encrypt(message, strlen(message));

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	sys_write(fd, message, nbytes);
	set_fs(old_fs);
	
	return 1;
}

/* ================================================== */

static int encrypt(char message[], int messageLength)
{
	struct skcipher_def sk;
	struct crypto_skcipher *skcipher = NULL;
	struct skcipher_request *req = NULL;
	int rc = 0;

	int ret = -EFAULT;

	char *key_encrypt = "0123456789ABCDEF";
	char *scratchpad = NULL;
	char *result = NULL;

	/* ==================== */

	/* Allocate a cipher handle for an skcipher */
	skcipher = crypto_alloc_skcipher("ecb(aes)", 0, 0);
	if (IS_ERR(skcipher))
	{
		pr_info("could not allocate skcipher handle\n");
		return PTR_ERR(skcipher);
	}

	/* Allocate the request data structure that must be used with the skcipher encrypt and decrypt API calls */
	req = skcipher_request_alloc(skcipher, GFP_KERNEL);
	if (!req)
	{
		pr_info("could not allocate skcipher request\n");
		ret = -ENOMEM;
		goto out;
	}

	skcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG, test_skcipher_cb, &sk.result);

	/* ==================== */

	if (crypto_skcipher_setkey(skcipher, key_encrypt, 16))
	{
		pr_err("fail setting key");
		goto out;
	}

	/* ==================== */

	scratchpad = vmalloc(messageLength);

	if (!scratchpad)
	{
		pr_info("Could not allocate scratchpad\n");
		goto out;
	}

	memcpy(scratchpad, message, messageLength);

	/* ==================== */

	/* Setando struct */
	sk.skcipher = skcipher;
	sk.req = req;

	/* Cifrar / Encrypt */
	sg_init_one(&sk.sg, scratchpad, 16);
	skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, iv_encrypt);
	init_completion(&sk.result.completion);

	rc = crypto_skcipher_encrypt(req);

	if (rc)
	{
		pr_info("skcipher encrypt returned with %d result %d\n", rc, sk.result.err);
		goto out;
	}

	init_completion(&sk.result.completion);

	result = sg_virt(&sk.sg);

	clearMessage(message);
	strcpy(message, result);

	printk("========================================");
	print_hex_dump(KERN_DEBUG, "Result Data Encrypt: ", DUMP_PREFIX_NONE, 16, 1, result, 16, true);
	printk("========================================");

	/* ==================== */

	out:
	if (skcipher)
		crypto_free_skcipher(skcipher);

	if (req)
		skcipher_request_free(req);

	if (key_encrypt)
		vfree(key_encrypt);

	if (scratchpad)
		vfree(scratchpad);

	return 0;
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
