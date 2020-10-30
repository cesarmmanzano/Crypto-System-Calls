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


/* ================================================== */

struct tcrypt_result
{
	struct completion completion;
	int err;
};

struct skcipher_def
{
	struct scatterlist sg;
	struct crypto_skcipher *skcipher;
	struct skcipher_request *req;
	struct tcrypt_result result;
};

/* ================================================== */

static int decrypt(char *message, int messageLength);
static void test_skcipher_cb(struct crypto_async_request *req, int error);
void clearMessage(char message[]);

/* ================================================== */

asmlinkage ssize_t sys_read_crypt(int fd, const void *buf, size_t nbytes)
{
	char message[256];
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	sys_read(fd, buf, nbytes);

	//transformar em char
	//decrypt(message, strlen(message));
		
	set_fs(old_fs);
	
	return 1;
}

/* ================================================== */

static int decrypt(char *message, int messageLength)
{
	struct skcipher_def sk;
	struct crypto_skcipher *skcipher = NULL;
	struct skcipher_request *req = NULL;
	int rc = 0;

	int ret = -EFAULT;

	char key[16] = "0123456789ABCDEF";
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

	if (crypto_skcipher_setkey(skcipher, key, 16))
	{
		pr_err("fail setting key");
		goto out;
	}

	/* ==================== */

	/* Set message */
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

	/* Decifrar / Decrypt */
	sg_init_one(&sk.sg, scratchpad, 16);
	skcipher_request_set_crypt(req, &sk.sg, &sk.sg, 16, NULL);
	init_completion(&sk.result.completion);

	rc = crypto_skcipher_decrypt(req);

	if (rc)
	{
		pr_info("skcipher encrypt returned with %d result %d\n", rc, sk.result.err);
		goto out;
	}

	init_completion(&sk.result.completion);

	result = sg_virt(&sk.sg);
	
	clearMessage(message);
	strcpy(message, result);

	printk("====================");
	print_hex_dump(KERN_DEBUG, "Result Data Decrypt: ", DUMP_PREFIX_NONE, 16, 1, result, 16, true);
	printk("====================");

	/* ==================== */

	out:
	if (skcipher)
		crypto_free_skcipher(skcipher);

	if (req)
		skcipher_request_free(req);

	if (scratchpad)
		vfree(scratchpad);

	return 0;
}

/* ================================================== */

/* Callback function */
static void test_skcipher_cb(struct crypto_async_request *req, int error)
{
	struct tcrypt_result *result = req->data;

	if (error == -EINPROGRESS)
		return;
	result->err = error;
	complete(&result->completion);
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


