#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x5eadf54a, "module_layout" },
	{ 0x37a0cba, "kfree" },
	{ 0xb714e933, "cdev_del" },
	{ 0x9545af6d, "tasklet_init" },
	{ 0x48eb0c0d, "__init_waitqueue_head" },
	{ 0x9b604f4c, "cdev_add" },
	{ 0xe41b8ba6, "cdev_init" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x83699014, "kmem_cache_alloc_trace" },
	{ 0x85e90336, "kmalloc_caches" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0x2e60bace, "memcpy" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xfaef0ed, "__tasklet_schedule" },
	{ 0xe45f60d8, "__wake_up" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0x75bb675a, "finish_wait" },
	{ 0x622fa02a, "prepare_to_wait" },
	{ 0x4292364c, "schedule" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x81d0ec74, "kill_fasync" },
	{ 0x131ff057, "current_task" },
	{ 0xc4554217, "up" },
	{ 0xdd1a2871, "down" },
	{ 0x437b86d4, "fasync_helper" },
	{ 0x7d11c268, "jiffies" },
	{ 0x50eedeb8, "printk" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "0E4BAC6EFC4D9429522221D");
