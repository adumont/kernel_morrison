/* arch/arm/mach-msm/rpc_server_handset.c
 *
 * Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can find it at http://www.fsf.org.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/switch.h>

#include <asm/mach-types.h>

#include <mach/msm_rpcrouter.h>
#include <mach/board.h>

#if defined(CONFIG_MACH_MOT)
struct input_dev *msm_keypad_get_input_dev(void);
#else
#include "keypad-surf-ffa.h"

#define DRIVER_NAME	"msm-handset"
#endif

#define HS_SERVER_PROG 0x30000062
#define HS_SERVER_VERS 0x00010001

#if !defined(CONFIG_MACH_MOT)
#define HS_RPC_PROG 0x30000091
#define HS_RPC_VERS 0x00010001

#define HS_RPC_CB_PROG 0x31000091
#define HS_RPC_CB_VERS 0x00010001

#define HS_SUBSCRIBE_SRVC_PROC 0x03
#define HS_EVENT_CB_PROC	1
#endif

#define RPC_KEYPAD_NULL_PROC 0
#define RPC_KEYPAD_PASS_KEY_CODE_PROC 2
#define RPC_KEYPAD_SET_PWR_KEY_STATE_PROC 3

#define HS_PWR_K		0x6F	/* Power key */
#define HS_END_K		0x51	/* End key or Power key */
#define HS_STEREO_HEADSET_K	0x82
#define HS_HEADSET_SWITCH_K	0x84
#define HS_REL_K		0xFF	/* key release */

#define KEY(hs_key, input_key) ((hs_key << 24) | input_key)

#if !defined(CONFIG_MACH_MOT)
struct hs_key_data {
	uint32_t ver;        /* Version number to track sturcture changes */
	uint32_t code;       /* which key? */
	uint32_t parm;       /* key status. Up/down or pressed/released */
};

enum hs_subs_srvc {
	HS_SUBS_SEND_CMD = 0, /* Subscribe to send commands to HS */
	HS_SUBS_RCV_EVNT,     /* Subscribe to receive Events from HS */
	HS_SUBS_SRVC_MAX
};

enum hs_subs_req {
	HS_SUBS_REGISTER,    /* Subscribe   */
	HS_SUBS_CANCEL,      /* Unsubscribe */
	HS_SUB_STATUS_MAX
};

enum hs_event_class {
	HS_EVNT_CLASS_ALL = 0, /* All HS events */
	HS_EVNT_CLASS_LAST,    /* Should always be the last class type   */
	HS_EVNT_CLASS_MAX
};

enum hs_cmd_class {
	HS_CMD_CLASS_LCD = 0, /* Send LCD related commands              */
	HS_CMD_CLASS_KPD,     /* Send KPD related commands              */
	HS_CMD_CLASS_LAST,    /* Should always be the last class type   */
	HS_CMD_CLASS_MAX
};

/*
 * Receive events or send command
 */
union hs_subs_class {
	enum hs_event_class	evnt;
	enum hs_cmd_class	cmd;
};

struct hs_subs {
	uint32_t                ver;
	enum hs_subs_srvc	srvc;  /* commands or events */
	enum hs_subs_req	req;   /* subscribe or unsubscribe  */
	uint32_t		host_os;
	enum hs_subs_req	disc;  /* discriminator    */
	union hs_subs_class      id;
};

struct hs_event_cb_recv {
	uint32_t cb_id;
	uint32_t hs_key_data_ptr;
	struct hs_key_data key;
};
#endif

static const uint32_t hs_key_map[] = {
#if defined(CONFIG_MACH_MOT)
	KEY(HS_END_K, KEY_POWER),
#else
	KEY(HS_PWR_K, KEY_POWER),
	KEY(HS_END_K, KEY_END),
	KEY(HS_STEREO_HEADSET_K, SW_HEADPHONE_INSERT),
#endif
	KEY(HS_HEADSET_SWITCH_K, KEY_MEDIA),
	0
};

#if defined(CONFIG_MACH_MOT)
static struct input_dev *kpdev;
#else
enum {
	NO_DEVICE	= 0,
	MSM_HEADSET	= 1,
};

struct msm_handset {
	struct input_dev *ipdev;
	struct switch_dev sdev;
};

static struct msm_rpc_client *rpc_client;
static struct msm_handset *hs;
#endif

#if defined(CONFIG_KERNEL_MOTOROLA)
static struct input_dev *kpdev;
extern struct input_dev *msm_keypad_get_input_dev(void);
#endif /* defined(CONFIG_KERNEL_MOTOROLA) */

static int hs_find_key(uint32_t hscode)
{
	int i, key;

	key = KEY(hscode, 0);

	for (i = 0; hs_key_map[i] != 0; i++) {
		if ((hs_key_map[i] & 0xff000000) == key)
			return hs_key_map[i] & 0x00ffffff;
	}
	return -1;
}

#if !defined(CONFIG_MACH_MOT)
static void
report_headset_switch(struct input_dev *dev, int key, int value)
{
	struct msm_handset *hs = input_get_drvdata(dev);

	input_report_switch(dev, key, value);
	switch_set_state(&hs->sdev, value);
}
#endif

/*
 * tuple format: (key_code, key_param)
 *
 * old-architecture:
 * key-press = (key_code, 0)
 * key-release = (0xff, key_code)
 *
 * new-architecutre:
 * key-press = (key_code, 0)
 * key-release = (key_code, 0xff)
 */
static void report_hs_key(uint32_t key_code, uint32_t key_parm)
{
	int key, temp_key_code;

	if (key_code == HS_REL_K)
		key = hs_find_key(key_parm);
	else
		key = hs_find_key(key_code);

	temp_key_code = key_code;

	if (key_parm == HS_REL_K)
		key_code = key_parm;

#if !defined(CONFIG_KERNEL_MOTOROLA) 
	kpdev = msm_keypad_get_input_dev();
#endif
	switch (key) {
	case KEY_POWER:
	case KEY_END:
#if defined(CONFIG_MACH_MOT)
		if (!kpdev) {
			printk(KERN_ERR "%s: No input device for reporting "
					"pwr/end key press\n", __func__);
			return;
		}
#endif
        
#if !defined(CONFIG_KERNEL_MOTOROLA)        
		input_report_key(kpdev, key, (key_code != HS_REL_K));
		input_sync(kpdev);
#else
       input_report_key(hs->ipdev, key, (key_code != HS_REL_K));
#endif
		break;
	case SW_HEADPHONE_INSERT:
		break;
	case KEY_MEDIA:
		break;
	default:
		printk(KERN_ERR "%s: Unhandled handset key %d\n", __func__,
				key);
	case -1:
		printk(KERN_ERR "%s: No mapping for remote handset event %d\n",
				 __func__, temp_key_code);
		return;
	}
#if !defined(CONFIG_KERNEL_MOTOROLA)    
    input_sync(kpdev);
#else
    input_sync(hs->ipdev);
#endif /* !defined(CONFIG_KERNEL_MOTOROLA) */    
}

static int handle_hs_rpc_call(struct msm_rpc_server *server,
			   struct rpc_request_hdr *req, unsigned len)
{
	struct rpc_keypad_pass_key_code_args {
		uint32_t key_code;
		uint32_t key_parm;
	};

#if defined(CONFIG_MACH_MOT)
	kpdev = msm_keypad_get_input_dev();
#endif
	switch (req->procedure) {
	case RPC_KEYPAD_NULL_PROC:
		return 0;

	case RPC_KEYPAD_PASS_KEY_CODE_PROC: {
		struct rpc_keypad_pass_key_code_args *args;

		args = (struct rpc_keypad_pass_key_code_args *)(req + 1);
		args->key_code = be32_to_cpu(args->key_code);
		args->key_parm = be32_to_cpu(args->key_parm);

		report_hs_key(args->key_code, args->key_parm);

		return 0;
	}

	case RPC_KEYPAD_SET_PWR_KEY_STATE_PROC:
		/* This RPC function must be available for the ARM9
		 * to function properly.  This function is redundant
		 * when RPC_KEYPAD_PASS_KEY_CODE_PROC is handled. So
		 * input_report_key is not needed.
		 */
		return 0;
	default:
		return -ENODEV;
	}
}

static struct msm_rpc_server hs_rpc_server = {
	.prog		= HS_SERVER_PROG,
	.vers		= HS_SERVER_VERS,
	.rpc_call	= handle_hs_rpc_call,
};

#if defined(CONFIG_MACH_MOT)
static int __init hs_rpc_server_init(void)
{
	return msm_rpc_create_server(&hs_rpc_server);
}
module_init(hs_rpc_server_init);
#else
static int process_subs_srvc_callback(struct hs_event_cb_recv *recv)
{
	if (!recv)
		return -ENODATA;

	report_hs_key(be32_to_cpu(recv->key.code), be32_to_cpu(recv->key.parm));

	return 0;
}

static void process_hs_rpc_request(uint32_t proc, void *data)
{
	if (proc == HS_EVENT_CB_PROC)
		process_subs_srvc_callback(data);
	else
		pr_err("%s: unknown rpc proc %d\n", __func__, proc);
}

static int hs_rpc_register_subs_arg(struct msm_rpc_client *client,
				    void *buffer, void *data)
{
	struct hs_subs_rpc_req {
		uint32_t hs_subs_ptr;
		struct hs_subs hs_subs;
		uint32_t hs_cb_id;
		uint32_t hs_handle_ptr;
		uint32_t hs_handle_data;
	};

	struct hs_subs_rpc_req *req = buffer;

	req->hs_subs_ptr	= cpu_to_be32(0x1);
	req->hs_subs.ver	= cpu_to_be32(0x1);
	req->hs_subs.srvc	= cpu_to_be32(HS_SUBS_RCV_EVNT);
	req->hs_subs.req	= cpu_to_be32(HS_SUBS_REGISTER);
	req->hs_subs.host_os	= cpu_to_be32(0x4); /* linux */
	req->hs_subs.disc	= cpu_to_be32(HS_SUBS_RCV_EVNT);
	req->hs_subs.id.evnt	= cpu_to_be32(HS_EVNT_CLASS_ALL);

	req->hs_cb_id		= cpu_to_be32(0x1);

	req->hs_handle_ptr	= cpu_to_be32(0x1);
	req->hs_handle_data	= cpu_to_be32(0x0);

	return sizeof(*req);
}

static int hs_rpc_register_subs_res(struct msm_rpc_client *client,
				    void *buffer, void *data)
{
	uint32_t result;

	result = be32_to_cpu(*((uint32_t *)buffer));
	pr_debug("%s: request completed: 0x%x\n", __func__, result);

	return 0;
}

static int hs_cb_func(struct msm_rpc_client *client, void *buffer, int in_size)
{
	int rc = -1;

	struct rpc_request_hdr *hdr = buffer;

	hdr->type = be32_to_cpu(hdr->type);
	hdr->xid = be32_to_cpu(hdr->xid);
	hdr->rpc_vers = be32_to_cpu(hdr->rpc_vers);
	hdr->prog = be32_to_cpu(hdr->prog);
	hdr->vers = be32_to_cpu(hdr->vers);
	hdr->procedure = be32_to_cpu(hdr->procedure);

	if (hdr->type != 0)
		return rc;
	if (hdr->rpc_vers != 2)
		return rc;
	if (hdr->prog != HS_RPC_CB_PROG)
		return rc;
	if (!msm_rpc_is_compatible_version(HS_RPC_CB_VERS,
				hdr->vers))
		return rc;

	process_hs_rpc_request(hdr->procedure,
			    (void *) (hdr + 1));

	msm_rpc_start_accepted_reply(client, hdr->xid,
				     RPC_ACCEPTSTAT_SUCCESS);
	rc = msm_rpc_send_accepted_reply(client, 0);
	if (rc) {
		pr_err("%s: sending reply failed: %d\n", __func__, rc);
		return rc;
	}

	return 0;
}

static int __init hs_rpc_cb_init(void)
{
	int rc = 0;

	rpc_client = msm_rpc_register_client("hs",
			HS_RPC_PROG, HS_RPC_VERS, 0, hs_cb_func);

	if (IS_ERR(rpc_client)) {
		pr_err("%s: couldn't open rpc client err %ld\n", __func__,
			 PTR_ERR(rpc_client));
		return PTR_ERR(rpc_client);
	}

	rc = msm_rpc_client_req(rpc_client, HS_SUBSCRIBE_SRVC_PROC,
				hs_rpc_register_subs_arg, NULL,
				hs_rpc_register_subs_res, NULL, -1);
	if (rc) {
		pr_err("%s: couldn't send rpc client request\n", __func__);
		msm_rpc_unregister_client(rpc_client);
	}

	return rc;
}

static int __devinit hs_rpc_init(void)
{
	int rc;

	if (machine_is_msm7x27_surf() || machine_is_msm7x27_ffa() ||
		machine_is_qsd8x50_surf() || machine_is_qsd8x50_ffa() ||
		machine_is_msm7x30_surf() || machine_is_msm7x30_ffa()) {
		rc = hs_rpc_cb_init();
		if (rc)
			pr_err("%s: failed to initialize\n", __func__);
	}

	rc = msm_rpc_create_server(&hs_rpc_server);
	if (rc < 0)
		pr_err("%s: failed to create rpc server\n", __func__);

	return 0;
}

static void __devexit hs_rpc_deinit(void)
{
	if (rpc_client)
		msm_rpc_unregister_client(rpc_client);
}

static ssize_t msm_headset_print_name(struct switch_dev *sdev, char *buf)
{
	switch (switch_get_state(&hs->sdev)) {
	case NO_DEVICE:
		return sprintf(buf, "No Device\n");
	case MSM_HEADSET:
		return sprintf(buf, "Headset\n");
	}
	return -EINVAL;
}

static int __devinit hs_probe(struct platform_device *pdev)
{
	int rc;
	struct input_dev *ipdev;

	hs = kzalloc(sizeof(struct msm_handset), GFP_KERNEL);
	if (!hs)
		return -ENOMEM;

	hs->sdev.name	= "h2w";
	hs->sdev.print_name = msm_headset_print_name;

	rc = switch_dev_register(&hs->sdev);
	if (rc)
		goto err_switch_dev_register;

	ipdev = input_allocate_device();
	if (!ipdev) {
		rc = -ENOMEM;
		goto err_alloc_input_dev;
	}
	input_set_drvdata(ipdev, hs);

	hs->ipdev = ipdev;

	if (pdev->dev.platform_data)
		ipdev->name = pdev->dev.platform_data;
	else
		ipdev->name	= DRIVER_NAME;

	ipdev->id.vendor	= 0x0001;
	ipdev->id.product	= 1;
	ipdev->id.version	= 1;

	input_set_capability(ipdev, EV_KEY, KEY_MEDIA);
	input_set_capability(ipdev, EV_SW, SW_HEADPHONE_INSERT);
	input_set_capability(ipdev, EV_KEY, KEY_POWER);
	input_set_capability(ipdev, EV_KEY, KEY_END);

	rc = input_register_device(ipdev);
	if (rc) {
		dev_err(&ipdev->dev,
				"hs_probe: input_register_device rc=%d\n", rc);
		goto err_reg_input_dev;
	}

	platform_set_drvdata(pdev, hs);

	rc = hs_rpc_init();
	if (rc)
		goto err_hs_rpc_init;

	return 0;

err_hs_rpc_init:
	input_unregister_device(ipdev);
	ipdev = NULL;
err_reg_input_dev:
	input_free_device(ipdev);
err_alloc_input_dev:
	switch_dev_unregister(&hs->sdev);
err_switch_dev_register:
	kfree(hs);
	return rc;
}

static int __devexit hs_remove(struct platform_device *pdev)
{
	struct msm_handset *hs = platform_get_drvdata(pdev);

	input_unregister_device(hs->ipdev);
	switch_dev_unregister(&hs->sdev);
	kfree(hs);
	hs_rpc_deinit();
	return 0;
}

static struct platform_driver hs_driver = {
	.probe		= hs_probe,
	.remove		= __devexit_p(hs_remove),
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init hs_init(void)
{
	return platform_driver_register(&hs_driver);
}
late_initcall(hs_init);

static void __exit hs_exit(void)
{
	platform_driver_unregister(&hs_driver);
}
module_exit(hs_exit);

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:msm-handset");
#endif
