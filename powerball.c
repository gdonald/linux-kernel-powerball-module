
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/sort.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greg Donald");
MODULE_DESCRIPTION("Powerball");

#define MAX_WHITES 5
#define MAX_WHITE 59
#define MAX_RED 35

static char *qp;
static int ready;

static int in_array(int list[], int x, int n)
{
	int y;

	for (y = 0; y < MAX_WHITES; y++) {
		if (x == y)
			continue;

		if (list[y] == n)
			return 1;
	}

	return 0;
}

static int get_rand_int(int max)
{
	int n;

	get_random_bytes(&n, sizeof(n));
	n = abs(n % max) + 1;

	return n;
}

static int compare(const void *lhs, const void *rhs)
{
	int lhs_int = *(const int *)(lhs);
	int rhs_int = *(const int *)(rhs);

	if (lhs_int < rhs_int)
		return -1;
	if (lhs_int > rhs_int)
		return 1;

	return 0;
}

static void u32_swap(void *a, void *b, int size)
{
	u32 t = *(u32 *)a;
	*(u32 *)a = *(u32 *)b;
	*(u32 *)b = t;
}

static char *get_quick_pick(void)
{
	int n[MAX_WHITES] = {0, 0, 0, 0, 0};
	int pb, x;

	for (x = 0; x < MAX_WHITES; x++)
		do {
			n[x] = get_rand_int(MAX_WHITE);
		} while (in_array(n, x, n[x]));

	sort(n, MAX_WHITES, sizeof(int), &compare, u32_swap);

	pb = get_rand_int(MAX_RED);

	snprintf(qp, 19,
		 "%02d %02d %02d %02d %02d %02d\n",
		 n[0], n[1], n[2], n[3], n[4], pb);

	return qp;
}

static ssize_t powerball_read(struct file *fp, char __user *buf, size_t size,
			      loff_t *off)
{
	size_t count = size;
	size_t len = strlen(qp);
	ssize_t retval = 0;
	unsigned long ret = 0;

	if (ready == 0) {
		ready = 1;
		strncpy(qp, get_quick_pick(), 95);
	}

	if (*off >= len) {
		ready = 0;
		goto out;
	}

	if (*off + size > len)
		count = len - *off;

	if (buf)
		ret = copy_to_user(buf, qp, count);

	*off += count - ret;
	retval = count - ret;

out:
	return retval;
}

static const struct file_operations powerball_fops = {
	.read = powerball_read
};

static struct miscdevice powerball_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "powerball",
	.fops = &powerball_fops,
	.mode = S_IRUGO,
};

static int __init powerball_init(void)
{
	int ret;

	ready = 0;
	qp = kmalloc(19, GFP_KERNEL);
	strncpy(qp, get_quick_pick(), 19);

	ret = misc_register(&powerball_dev);
	if (ret)
		pr_info("Failed to register powerball_dev\n");

	return ret;
}

static void __exit powerball_exit(void)
{
	misc_deregister(&powerball_dev);
}

module_init(powerball_init);
module_exit(powerball_exit);
