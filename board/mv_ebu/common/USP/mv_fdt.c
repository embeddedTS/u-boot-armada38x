/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.

*******************************************************************************/

#include <common.h>
#include <command.h>
#include "mvOs.h"
#include "mv_amp.h"
#include "mvCpuIfRegs.h"
#include "uart/mvUart.h"
#include "libfdt.h"
#include "fdt_support.h"
#include "mvBoardEnvLib.h"
#include "ethSwitch/mvSwitchRegs.h"
#include "ethSwitch/mvSwitch.h"

/*******************************************************************************
* fdt_env_setup
*
* DESCRIPTION:
*
* INPUT:
*	fdtfile.
*
* OUTPUT:
*	None.
*
* RETURN:
*	None.
*
*******************************************************************************/
void fdt_env_setup(char *fdtfile, MV_BOOL runUpdate)
{
#if CONFIG_OF_LIBFDT
	char *env;

	env = getenv("fdt_skip_update"); /* if set to yes, automatic board setup will be skipped */
	if (!env)
		setenv("fdt_skip_update", runUpdate ? "no" : "yes");

	env = getenv("fdtaddr");
	if (!env)
		setenv("fdtaddr", "0x1000000");

	env = getenv("fdtfile");
	if (!env)
		setenv("fdtfile", fdtfile);

	/* boot command to fetch DT file, update DT (if fdt_skip_update=no) and bootz LSP zImage */
	char bootcmd_fdt[] = "tftpboot 0x2000000 $image_name;tftpboot $fdtaddr $fdtfile;"
		"setenv bootargs $console $nandEcc $mtdparts $bootargs_root nfsroot=$serverip:$rootpath "
		"ip=$ipaddr:$serverip$bootargs_end $mvNetConfig video=dovefb:lcd0:$lcd0_params "
		"clcd.lcd0_enable=$lcd0_enable clcd.lcd_panel=$lcd_panel; bootz 0x2000000 - $fdtaddr;";

	/* boot command to and bootz LSP zImage (after DT already fetch and set) */
	env = getenv("bootcmd_fdt_boot");
	if (!env)
		setenv("bootcmd_fdt_boot", "tftpboot 0x2000000 $image_name; setenv bootargs $console $nandEcc "
			"$mtdparts $bootargs_root nfsroot=$serverip:$rootpath ip=$ipaddr:$serverip$bootargs_end "
			"$mvNetConfig video=dovefb:lcd0:$lcd0_params clcd.lcd0_enable=$lcd0_enable "
			"clcd.lcd_panel=$lcd_panel; bootz 0x2000000 - $fdtaddr;");

	/* boot command to fetch DT file, and set bootcmd_fdt_boot as new bootcmd (manually edit DT & run 'boot') */
	env = getenv("bootcmd_fdt_edit");
	if (!env)
		setenv("bootcmd_fdt_edit", "tftpboot $fdtaddr $fdtfile; fdt addr $fdtaddr; setenv bootcmd $bootcmd_fdt_boot");

	env = getenv("bootcmd_fdt");
	if (!env)
		setenv("bootcmd_fdt",bootcmd_fdt);

#if defined (CONFIG_OF_LIBFDT_IS_DEFAULT)
	env = getenv("bootcmd");
	if (!env)
		setenv("bootcmd", bootcmd_fdt);
#endif /* (CONFIG_OF_LIBFDT_IS_DEFAULT) */
#endif
}


#ifdef CONFIG_OF_BOARD_SETUP

#define MV_FDT_MAXDEPTH 32

static int mv_fdt_find_node(void *fdt, const char *name);
static int mv_fdt_board_compatible_name_update(void *fdt);
static int mv_fdt_board_model_name_update(void *fdt);
static int mv_fdt_update_serial(void *fdt);
#ifdef CONFIG_PIC_GPIO
static int mv_fdt_update_pic_gpio(void *fdt);
#endif
static int mv_fdt_update_cpus(void *fdt);
static int mv_fdt_update_pex(void *fdt);
#ifdef MV_INCLUDE_SATA
static int mv_fdt_update_sata(void *fdt);
#endif
#ifdef MV_INCLUDE_USB
static int mv_fdt_update_usb(void *fdt, MV_UNIT_ID unitType);
static int mv_fdt_update_usb2(void *fdt);
static int mv_fdt_update_usb3(void *fdt);
#endif
static int mv_fdt_update_ethnum(void *fdt);
static int mv_fdt_update_flash(void *fdt);
static int mv_fdt_set_node_prop(void *fdt, const char *node, const char *prop, const char *prop_val);
static int mv_fdt_remove_node(void *fdt, const char *path);
static int mv_fdt_scan_and_set_alias(void *fdt, const char *name, const char *alias);
static int mv_fdt_nand_mode_fixup(void *fdt);
static int mv_fdt_update_pinctrl(void *fdt);
static int mv_fdt_update_mpp_config(void *fdt);
#ifdef MV_INCLUDE_AUDIO
static int mv_fdt_update_audio(void *fdt);
#endif
#ifdef MV_INCLUDE_SWITCH
static int mv_fdt_update_switch(void *fdt);
#endif
static int mv_fdt_debug;
#ifdef CONFIG_MMC
static int mv_fdt_update_sd(void *fdt);
#endif
#ifdef CONFIG_SWITCHING_SERVICES
static int mv_fdt_update_prestera(void *fdt);
#endif
#ifdef MV_INCLUDE_TDM
static int mv_fdt_update_tdm(void *fdt);
#endif
#ifdef MV_USB_VBUS_CYCLE
static int mv_fdt_update_usb_vbus(void *fdt);
#endif
#ifdef MV_INCLUDE_USB_DEVICE
static int mv_fdt_update_usb_device(void *fdt);
#endif
#ifdef MV_CM3
static int mv_fdt_update_cm3(void *fdt);
#endif
#ifdef MV_PP_SMI
static int mv_fdt_update_external_smi(void *fdt);
#endif

#if 0 /* not compiled, since this routine is currently not in use  */
static int mv_fdt_remove_prop(void *fdt, const char *path,
				const char *name, int nodeoff);

#endif

typedef int (update_fnc_t)(void *);
update_fnc_t *update_sequence[] = {
	mv_fdt_update_cpus,			/* Get number of CPUs and update dt */
	mv_fdt_update_pex,			/* Get number of active PEX port and update DT */
#ifdef MV_INCLUDE_SATA
	mv_fdt_update_sata,			/* Get number of active SATA units and update DT */
#endif
#ifdef MV_INCLUDE_USB
	mv_fdt_update_usb2,			/* Get number of active USB2.0 units and update DT */
	mv_fdt_update_usb3,			/* Get number of active USB3.0 units and update DT */
#endif
	mv_fdt_update_ethnum,			/* Get number of active ETH port and update DT */
	mv_fdt_update_mpp_config,		/*Update FDT entries related to mpp configuration*/
#ifdef CONFIG_MMC
	mv_fdt_update_sd,			/* Update SDHCI/SDIO driver settings in DT */
#endif
#ifdef CONFIG_SWITCHING_SERVICES
	mv_fdt_update_prestera,			/* Update prestera driver DT settings - for AMC board */
#endif
#ifdef MV_INCLUDE_TDM
	mv_fdt_update_tdm,			/* Update tdm node in DT */
#endif
#ifdef MV_USB_VBUS_CYCLE
	mv_fdt_update_usb_vbus,
#endif
	mv_fdt_update_flash,			/* Get number of active flash devices and update DT */
	mv_fdt_nand_mode_fixup,			/* Update NAND controller ECC settings in DT */
	mv_fdt_update_pinctrl,			/* Update pinctrl driver settings in DT */
	mv_fdt_board_compatible_name_update,	/* Update compatible (board name) in DT */
	mv_fdt_board_model_name_update,		/* Update model node in DT */
	mv_fdt_update_serial,			/* Update serial/UART nodes in DT */
#ifdef CONFIG_PIC_GPIO
	mv_fdt_update_pic_gpio,
#endif
#ifdef MV_INCLUDE_AUDIO
	mv_fdt_update_audio,			/* Update audio-controller+sound nodes in DT */
#endif
#ifdef MV_INCLUDE_SWITCH
	mv_fdt_update_switch,
#endif
#ifdef MV_INCLUDE_USB_DEVICE
	mv_fdt_update_usb_device,
#endif
#ifdef MV_CM3
	mv_fdt_update_cm3,
#endif
#ifdef MV_PP_SMI
	mv_fdt_update_external_smi,
#endif
	NULL,
};

enum nfc_driver_type {
	MV_FDT_NFC_PXA3XX,	/* mainline pxa3xx-nand */
	MV_FDT_NFC_HAL,		/* mvebu HAL-based NFC */
	MV_FDT_NFC_HAL_A375,	/* mvebu HAL-based NFC for A375 */
	MV_FDT_NFC_NONE,
};

#define mv_fdt_dprintf(...)		\
	if (mv_fdt_debug)		\
		printf(__VA_ARGS__);

#define mv_fdt_modify(fdt, err, function)				\
	do {								\
		err = function; /* Try requested routine */		\
		while (err == -FDT_ERR_NOSPACE) {			\
			mv_fdt_dprintf("Resize fdt...\n");		\
			fdt_resize(fdt);				\
			err = function; /* Retry requested routine*/	\
		}							\
	} while (0)

/*******************************************************************************
* ft_board_setup
*
* DESCRIPTION:
*
* INPUT:
*	blob.
*	bd.
*
* OUTPUT:
*	None.
*
* RETURN:
*	None.
*
*******************************************************************************/
void ft_board_setup(void *blob, bd_t *bd)
{
	int err, skip = 1;		/* error number */
	char *env;			/* env value */
	update_fnc_t **update_fnc_ptr;

	/* Debug information will be printed if variable enaFDTdebug=yes */
	env = getenv("enaFDTdebug");
	if (env && ((strncmp(env, "yes", 3) == 0)))
		mv_fdt_debug = 1;
	else
		mv_fdt_debug = 0;

	env = getenv("fdt_skip_update");
	if (env && ((strncmp(env, "yes", 3) == 0))) {
		skip = 1;
		mv_fdt_dprintf("\n   Skipping Device Tree update ('fdt_skip_update' = yes)\n");
	}
	else {
		skip = 0;
		mv_fdt_dprintf("\n   Starting Device Tree update ('fdt_skip_update' = no)\n");
	}

	/* Update dt information for all SoCs */
	/* Update dt bootargs from commandline */
	fdt_resize(blob);
	mv_fdt_modify(blob, err, fdt_chosen(blob, 1));
	if (err < 0) {
		mv_fdt_dprintf("Updating DT bootargs failed\n");
		goto bs_fail;
	}
	mv_fdt_dprintf("DT bootargs updated from commandline\n");

	/* Update ethernet aliases with nodes' names and update mac addresses
	   - PP3 driver uses 2 aliases:
		 mac alias, to update PHY mode, MAC address
		 nic@ alias, to update status.
	   - Neta driver use 1 alias (ethernet@) to update PHY mode, MAC address, and status */
#ifdef CONFIG_NET_COMPLEX
	err = mv_fdt_scan_and_set_alias(blob, "mac", "mac");
	if (err < 0)
		goto bs_fail;
	err = mv_fdt_scan_and_set_alias(blob, "nic@", "ethernet");
#else
	err = mv_fdt_scan_and_set_alias(blob, "ethernet@", "ethernet");
#endif
	if (err < 0)
		goto bs_fail;
	fdt_fixup_ethernet(blob);
	mv_fdt_dprintf("DT ethernet MAC addresses updated from environment\n");

	/* Update memory node */
	fixup_memory_node(blob);
	mv_fdt_dprintf("Memory node updated\n");

	if (!skip) {
		/* Make updates of functions in update_sequence */
		for (update_fnc_ptr = update_sequence; *update_fnc_ptr; ++update_fnc_ptr) {
			err = (*update_fnc_ptr)(blob);
			if (err < 0)
				goto bs_fail;
		}
		mv_fdt_dprintf("Updating device tree successful\n");
	}

	return;
bs_fail:
	mv_fdt_dprintf("Updating device tree failed\n");
	return;
}

/*******************************************************************************
* mv_fdt_find_node
*
* DESCRIPTION:
*
* INPUT:
*	fdt.
*	name.
*
* OUTPUT:
*	None.
*
* RETURN:
*	node offset or -1 if node not found.
*
*******************************************************************************/
static int mv_fdt_find_node(void *fdt, const char *name)
{
	int nodeoffset;		/* current node's offset from libfdt */
	int nextoffset;		/* next node's offset from libfdt */
	int level = 0;		/* current fdt scanning depth */
	uint32_t tag;		/* device tree tag at given offset */
	const char *node;	/* node name */

	/* Set scanning starting point to '/' */
	nodeoffset = fdt_path_offset(fdt, "/");
	if (nodeoffset < 0) {
		mv_fdt_dprintf("%s: failed to get '/' nodeoffset\n",
			       __func__);
		return -1;
	}
	/* Scan device tree for node = 'name' and return its offset */
	while (level >= 0) {
		tag = fdt_next_tag(fdt, nodeoffset, &nextoffset);
		switch (tag) {
		case FDT_BEGIN_NODE:
			node = fdt_get_name(fdt, nodeoffset, NULL);
			if (strncmp(node, name, strlen(name)+1) == 0)
				return nodeoffset;
			level++;
			if (level >= MV_FDT_MAXDEPTH) {
				mv_fdt_dprintf("%s: Nested too deep, "
					       "aborting.\n", __func__);
				return -1;
			}
			break;
		case FDT_END_NODE:
			level--;
			if (level == 0)
				level = -1;		/* exit the loop */
			break;
		case FDT_PROP:
		case FDT_NOP:
			break;
		case FDT_END:
			mv_fdt_dprintf("Device tree scanning failed - end of "
				       "tree tag\n");
			return -1;
		default:
			mv_fdt_dprintf("Device tree scanning failed - unknown "
				       "tag\n");
			return -1;
		}
		nodeoffset = nextoffset;
	}

	/* Node not found in the device tree */
	return -1;
}

/*******************************************************************************
* mv_fdt_update_cpus
*
* DESCRIPTION:
* target		: remove excessice cpu nodes.
* node			: cpus
* dependencies		: CPU core num in SOC_COHERENCY_FABRIC_CFG_REG.
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_cpus(void *fdt)
{
	int cpusnodes = 0;	/* number of cpu nodes */
	int nodeoffset;		/* node offset from libfdt */
	int err;		/* error number */
	const char *node;	/* node name */
	char *p;		/* auxiliary pointer */
	char *lastcpu;		/* pointer to last valid cpu */
	MV_U8 cpusnum;		/* number of cpus */
	int depth = 2;

	/* Get CPUs number and remove unnecessary nodes */
	cpusnum = mvCtrlGetCpuNum();
	mv_fdt_dprintf("Number of CPUs detected: %d\n", cpusnum);
	/* Find cpus node */
	node = "cpus";
	nodeoffset = mv_fdt_find_node(fdt, node);
	if (nodeoffset < 0) {
		mv_fdt_dprintf("Lack of '%s' node in device tree\n", node);
		return -1;
	}
	p = malloc(strlen("cpu@x")+1);
	lastcpu = malloc(strlen("cpu@x")+1);
	while (strncmp(node, "cpu", 3) == 0) {
		cpusnodes++;
		/* Remove excessive cpu nodes */
		if (cpusnodes > cpusnum + 1) {
			strcpy(p, node);
			err = mv_fdt_remove_node(fdt, (const char *)p);
				if (err < 0) {
					mv_fdt_dprintf("Failed to remove %s\n",
						       node);
					free(p);
					free(lastcpu);
					return -1;
				}
			node = lastcpu;
			nodeoffset = mv_fdt_find_node(fdt, node);
		} else {
			strcpy(lastcpu, node);
		}
		nodeoffset = fdt_next_node(fdt, nodeoffset, &depth);
		node = fdt_get_name(fdt, nodeoffset, NULL);
	}
	free(p);
	free(lastcpu);

	return 0;
}

#ifdef MV_INCLUDE_SATA
/*******************************************************************************
* mv_fdt_update_sata
*
* DESCRIPTION:
* target                : Update status field of SATA nodes.
* node, properties      : -property status @ node SATA
* dependencies          : sata unit status in sataUnitActive array.
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_sata(void *fdt)
{
	int i;
	char propval[10];				/* property value */
	const char *prop = "status";			/* property name */
	char node[64];					/* node name */
	MV_U32 sataUnitNum = mvCtrlSataMaxUnitGet();	/* number of interfaces */

	mv_fdt_dprintf("Maximum SATA Units = %d\n, Active SATA Units:", sataUnitNum);
	for (i = 0; i < sataUnitNum; i++) {
		if (mvCtrlIsActiveSataUnit(i))
			sprintf(propval, "okay"); /* Enable active SATA interfaces */
		else
			sprintf(propval, "disabled"); /* disable NON active SATA units */

		sprintf(node, "sata@%x", mvCtrlSataRegBaseGet(i));
		if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
			mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
			return 0;
		}
	}

	return 0;
}
#endif /* MV_INCLUDE_SATA */

#ifdef MV_INCLUDE_TDM
/*******************************************************************************
* mv_fdt_update_tdm
*
* DESCRIPTION:
* target		: update status field of tdm@X node.
* node, properties	: -property status @ node tdm@x.
* dependencies		: isTdmConnected entry in board structure.
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_tdm(void *fdt)
{
	int err, nodeoffset;				/* nodeoffset: node offset from libfdt */
	char propval[10];				/* property value */
	const char *prop = "status";			/* property name */
	char node[64];					/* node name */

	if (mvBoardIsTdmConnected() == MV_TRUE)
		sprintf(propval, "okay");
	else
		sprintf(propval, "disabled");

	sprintf(node, "tdm@%x", MV_TDM_OFFSET);
	nodeoffset = mv_fdt_find_node(fdt, node);

	if (nodeoffset < 0) {
		mv_fdt_dprintf("Lack of '%s' node in device tree\n", node);
		return 0;
	}

	if (strncmp(fdt_get_name(fdt, nodeoffset, NULL), node, strlen(node)) == 0) {
		mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop,
						propval, strlen(propval)+1));
		if (err < 0) {
			mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", prop, node);
			return 0;
		}
		mv_fdt_dprintf("Set '%s' property to '%s' in '%s' node\n", prop, propval, node);
	}

	return 0;
}
#endif

#ifdef MV_CM3
/*******************************************************************************
* mv_fdt_update_cm3
*
* DESCRIPTION:
* target		: update status and compatible field of cm3 node.
* node, properties	: -property status @ node cm3.
* dependencies		: isCm3 entry in board structure.
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_cm3(void *fdt)
{
	char propval[128];				/* property value */
	const char *prop;				/* property name */
	const char *node = "cm3";			/* node name */

	/* update status */
	if (mvBoardIsCm3() == MV_TRUE)
		sprintf(propval, "okay");
	else
		sprintf(propval, "disabled");

	prop = "status";
	if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
		mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
		return 0;
	}

	if (mvBoardIsCm3() == MV_FALSE)
		return 0;

	/* update cm3 compatible string */
	prop = "compatible";
	mvBoardCm3CompatibleNameGet(propval);
	if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
		mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
		return 0;
	}

	return 0;
}
#endif /* MV_CM3 */

#ifdef MV_PP_SMI
/*******************************************************************************
* mv_fdt_update_external_smi
*
* DESCRIPTION:
* target		: update the SMI register base for the OOB PHY.
* node, properties	: -ranges @ node switch.
* dependencies		: smiExternalPpIndex entry in board structure.
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_external_smi(void *fdt)
{
	uint32_t smi_base_size[2];			/* property value */
	MV_U32 smi_external_pp_index;			/* smi external pp index */
	/* Since extern SMI register belongs to switch MG register region */
	/* it is under '/switch' node out of '/soc' node */
	const char *node_path = "/switch/mdio";		/* node path */
	MV_U32 nodeoffset;

	nodeoffset = fdt_path_offset(fdt, node_path);
	if (mvBoardIsPpSmi() == MV_FALSE)
		return 0;

	/* update external smi register base and size */
	mvBoardPPSmiIndexGet(&smi_external_pp_index);
	smi_base_size[0] = htonl(MV_PP_SMI_BASE(smi_external_pp_index));	/* set external smi register base */
	smi_base_size[1] = htonl(8);		/* external smi remap size is fixed to 8 now */

	if (fdt_setprop(fdt, nodeoffset, "reg", smi_base_size, sizeof(smi_base_size)) < 0)
		mv_fdt_dprintf("Failed to set property 'reg' of node '%s' in device tree\n", node_path);

	return 0;
}
#endif /* MV_PP_SMI */

#ifdef MV_USB_VBUS_CYCLE
/*******************************************************************************
* mv_fdt_update_usb_vbus
*
* DESCRIPTION:
* target		: update status field of usb3_phy and usb3-vbus-gpio/usb3-vbus-exp1 node.
* node, properties	: -property status @ node usb3_phy and usb3-vbus-gpio/usb3-vbus-exp1 node.
* dependencies		: BOARD_GPP_USB_VBUS entry in BoardGppInfo --> usb3-vbus-gpio node
* 			  MV_IO_EXPANDER_USB_VBUS entry in BoardIoExpPinInfo --> usb3-vbus-exp1
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_usb_vbus(void *fdt)
{
	int err, nodeoffset;				/* nodeoffset: node offset from libfdt */
	char propval[10];				/* property value */
	const char *prop = "status";			/* property name */
	const char *node = NULL, *phy_node = NULL;	/* node name */
	MV_BOARD_IO_EXPANDER_TYPE_INFO ioInfo;		/* empty stub - not to be used */

	sprintf(propval, "okay");

	if (mvBoardIoExpanderTypeGet(MV_IO_EXPANDER_USB_VBUS, &ioInfo) != MV_ERROR) {
		node = "usb3-vbus-exp1";
		phy_node = "usb3-phy-exp1";
	} else {
		/* remove invalid i2c pins information, to avoid unexpected Linux behaviour*/
		mv_fdt_remove_node(fdt, "i2c-pins-0");
	}
	if (mvBoardUSBVbusGpioPinGet(0) != MV_ERROR) {
			node = "usb3-vbus-gpio";
			phy_node = "usb3-phy-gpio";
	} else {
		/* remove invalid vbus gpio pins information, to avoid unexpected Linux behaviour*/
		mv_fdt_remove_node(fdt, "xhci0-vbus-pins");
	}

	if (!node && !phy_node)
		return 0;

	nodeoffset = mv_fdt_find_node(fdt, node);

	if (nodeoffset < 0) {
		mv_fdt_dprintf("Lack of '%s' node in device tree\n", node);
		return 0;
	}
	if (strncmp(fdt_get_name(fdt, nodeoffset, NULL), node, strlen(node)) == 0) {
		mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop,
						propval, strlen(propval)+1));
		if (err < 0) {
			mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", prop, node);
			return 0;
		}
		mv_fdt_dprintf("Set '%s' property to '%s' in '%s' node\n", prop, propval, node);
	}

	/* enable 'usb3_phy' node - for Linux regulator usage */
	nodeoffset = mv_fdt_find_node(fdt, phy_node);

	if (nodeoffset < 0) {
		mv_fdt_dprintf("Lack of '%s' node in device tree\n", phy_node);
		return 0;
	}

	if (strncmp(fdt_get_name(fdt, nodeoffset, NULL), phy_node, strlen(phy_node)) == 0) {
		mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop,
						propval, strlen(propval)+1));
		if (err < 0) {
			mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", prop, phy_node);
			return 0;
		}
		mv_fdt_dprintf("Set '%s' property to '%s' in '%s' node\n", prop, propval, phy_node);
	}	return 0;
	return 0;
}
#endif /* MV_USB_VBUS_CYCLE */

#ifdef CONFIG_MMC
/*******************************************************************************
* mv_fdt_update_sd
*
* DESCRIPTION:
* target		: update status field for SDHCI/SDIO and dat3 for SDHCI.
* node, properties	: - property status and dat3-cd @ node sdhci@X.
*			  - property status @ node mvsdio
* dependencies		: status of MMC in isSdMmcConnected entry in board structure.
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_sd(void *fdt)
{
	int err, nodeoffset;				/* nodeoffset: node offset from libfdt */
	char propval[10];				/* property value */
	const char *prop = "status";			/* property name */
	char node[64];					/* node name */
	char *env;

	if (mvBoardisSdioConnected())
		sprintf(propval, "okay");
	else
		sprintf(propval, "disabled");

	sprintf(node, "%s@%x", MV_MMC_FDT_NODE_NAME, MV_SDMMC_REGS_OFFSET);
	nodeoffset = mv_fdt_find_node(fdt, node);

	if (nodeoffset < 0) {
		mv_fdt_dprintf("Lack of '%s' node in device tree\n", node);
		return 0;
	}

	if (strncmp(fdt_get_name(fdt, nodeoffset, NULL), node, strlen(node)) == 0) {
		mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop,
						propval, strlen(propval)+1));
		if (err < 0) {
			mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", prop, node);
			return 0;
		}
		mv_fdt_dprintf("Set '%s' property to '%s' in '%s' node\n", prop, propval, node);
	}

	/* for SDHCI only:
	   add DAT3 detection */
	env = getenv("sd_detection_dat3");

	if (!env || strcmp(env, "yes") != 0)
		return 0;

	prop = "dat3-cd";

	/* add empty property 'dat3-cd' */
	if (fdt_appendprop(fdt, nodeoffset, prop, NULL, 0) < 0)
		mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);

	return 0;
}
#endif

#ifdef MV_INCLUDE_AUDIO
/*******************************************************************************
* mv_fdt_update_audio
*
* DESCRIPTION: update audio-controller and sound nodes (SPDIF/I2S)
*
* INPUT: fdt.
* OUTPUT: None.
* RETURN: -1 on error os 0 otherwise.
*******************************************************************************/
static int mv_fdt_update_audio(void *fdt)
{
	int err, nodeoffset, i;				/* nodeoffset: node offset from libfdt */
	char propval[10];				/* property value */
	const char *prop = "status";			/* property name */
	char nodes[2][64];				/* node names */

	if (mvBoardIsAudioConnected() == MV_TRUE)
		sprintf(propval, "okay");
	else
		sprintf(propval, "disabled");

	sprintf(nodes[0], "audio-controller@%x", MV_AUDIO_OFFSET);
	sprintf(nodes[1], "sound");

	for (i = 0; i < 2; ++i) {
		nodeoffset = mv_fdt_find_node(fdt, nodes[i]);
		if (nodeoffset < 0) {
			mv_fdt_dprintf("Lack of '%s' node in device tree\n", nodes[i]);
			return 0;
		}

		if (strncmp(fdt_get_name(fdt, nodeoffset, NULL), nodes[i], strlen(nodes[i])) == 0) {
			mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop,
							propval, strlen(propval)+1));
			if (err < 0) {
				mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", prop, nodes[i]);
				return 0;
			}
			mv_fdt_dprintf("Set '%s' property to '%s' in '%s' node\n", prop, propval, nodes[i]);
		}
	}

	return 0;
}
#endif

#ifdef CONFIG_SWITCHING_SERVICES
/*******************************************************************************
* mv_fdt_update_prestera
*
* DESCRIPTION:
* target		: update status field of prestera node by checking if the board is AMC.
* node, properties	: -property status @ node prestera.
* dependencies		: AMC status saved in isAmc entry in board structure
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_prestera(void *fdt)
{
	int err, nodeoffset;					/* nodeoffset: node offset from libfdt */
	char propval[10];					/* property value */
	const char *prop = "status";				/* property name */
	const char *node = "prestera";				/* node name */
	MV_BOOL isAmc;

	isAmc = mvBoardisAmc();
	if (isAmc)
		sprintf(propval, "okay");
	else
		sprintf(propval, "disabled");

	nodeoffset = mv_fdt_find_node(fdt, node);

	if (nodeoffset < 0) {
		mv_fdt_dprintf("Lack of '%s' node in device tree\n", node);
		return 0;
	}

	mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop,
					propval, strlen(propval)+1));
	if (err < 0) {
		mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", prop, node);
		return -1;
	}
	mv_fdt_dprintf("Set '%s' property to '%s' in '%s' node\n", prop, propval, node);

	return 0;
}
#endif

/*******************************************************************************
* mv_fdt_update_pex
*
* DESCRIPTION:
* target		: update status fields of pcieX nodes.
* node, properties	: -property status @ node pcieX.
* dependencies		: boardPexInfo structure entry in board structure.
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_pex(void *fdt)
{
	MV_U32 pexnum;				/* number of interfaces */
	MV_BOARD_PEX_INFO *boardPexInfo;	/* pex info */
	int err;				/* error number */
	int nodeoffset;				/* node offset from libfdt */
	char propval[10];			/* property value */
	const char *prop = "status";		/* property name */
	const char *node = "pcie-controller";	/* node name */
	int i = 0;
	int k = 0;
	int depth = 1;

	/* Get number of active pex interfaces */
	boardPexInfo = mvBoardPexInfoGet();
	pexnum = boardPexInfo->boardPexIfNum;
	mv_fdt_dprintf("Number of active PEX ports detected = %d\n", pexnum);
	mv_fdt_dprintf("Active PEX HW interfaces: ");
	for (k = 0; k < pexnum; k++)
		mv_fdt_dprintf("%d, ", boardPexInfo->pexMapping[k]);
	mv_fdt_dprintf("\n");
	/* Set controller and 'pexnum' number of interfaces' status to 'okay'.
	 * Rest of them are disabled */
	nodeoffset = mv_fdt_find_node(fdt, node);
	if (nodeoffset < 0) {
		mv_fdt_dprintf("Lack of '%s' node in device tree\n", node);
		return 0;
	}
	while (strncmp(node, "pcie", 4) == 0) {
		for (k = 0; k <= pexnum; k++)
			if (i == boardPexInfo->pexMapping[k]) {
				sprintf(propval, "okay");
				goto pex_ok;
			}
		sprintf(propval, "disabled");
pex_ok:
		if (strncmp(node, "pcie-controller", 15) != 0)
			i++;
		mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop,
						propval, strlen(propval)+1));
		if (err < 0) {
			mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n",
				       prop, node);
			return 0;
		}
		mv_fdt_dprintf("Set '%s' property to '%s' in '%s' node\n", prop,
			       propval, node);
		nodeoffset = fdt_next_node(fdt, nodeoffset, &depth);
		if (nodeoffset < 0) {
			mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n",
				       prop, node);
			return 0;
		}
		node = fdt_get_name(fdt, nodeoffset, NULL);
	}
	return 0;
}
#ifdef MV_INCLUDE_USB
/*******************************************************************************
* mv_fdt_update_usb
*
* DESCRIPTION: update USB2.0(eHCI) & USB3.0(xHCI) status
*
* INPUT: fdt.
* OUTPUT: None.
* RETURN: -1 on error os 0 otherwise.
*******************************************************************************/
static int mv_fdt_update_usb(void *fdt, MV_UNIT_ID unitType)
{
	char propval[10];				/* property value */
	const char *prop = "status";			/* property name */
	char node[64];					/* node name */
	int i, maxUsbPorts = unitType == USB_UNIT_ID ? MV_USB_MAX_PORTS : MV_USB3_MAX_HOST_PORTS;

	/* update USB2.0 ports status */
	for (i = 0; i < maxUsbPorts; i++) {
		if (mvBoardIsUsbPortConnected(unitType, i))
			sprintf(propval, "okay"); /* Enable active SATA interfaces */
		else
			sprintf(propval, "disabled"); /* disable NON active SATA units */

		sprintf(node, "usb%s@%x", unitType == USB_UNIT_ID ? "" : "3", MV_USB2_USB3_REGS_OFFSET(unitType, i));
		if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
			mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
			return 0;
		}
	}

	return 0;
}

/*******************************************************************************
* mv_fdt_update_usb2
*
* DESCRIPTION: update USB2.0(eHCI) status, this function is wrapper function
* to mv_fdt_update_usb, In order to add it to update_sequence array
*
* INPUT: fdt.
* OUTPUT: None.
* RETURN: -1 on error os 0 otherwise.
*******************************************************************************/
static int mv_fdt_update_usb2(void *fdt)
{
	return mv_fdt_update_usb(fdt, USB_UNIT_ID);
}

/*******************************************************************************
* mv_fdt_update_usb3
*
* DESCRIPTION: update USB3.0(xHCI) status, this function is wrapper function
* to mv_fdt_update_usb, In order to add it to update_sequence array
*
* INPUT: fdt.
* OUTPUT: None.
* RETURN: -1 on error os 0 otherwise.
*******************************************************************************/
static int mv_fdt_update_usb3(void *fdt)
{
	return mv_fdt_update_usb(fdt, USB3_UNIT_ID);
}
#endif /* MV_INCLUDE_USB */

#ifdef MV_INCLUDE_USB_DEVICE
/*******************************************************************************
* mv_fdt_update_usb_device
*
* DESCRIPTION: usb3 device status
* target		: update status field of u3d and udc nodes.
* node, properties	: property status @ nodes usb3@X, udc@X and udc@X.
* dependencies		: S@R field 'usb3port0' or 'usb3port1'.
*
* INPUT: fdt.
* OUTPUT: None.
* RETURN: -1 on error os 0 otherwise.
*******************************************************************************/
static int mv_fdt_update_usb_device(void *fdt)
{
	char propval[10];				/* property value */
	const char *prop = "status";			/* property name */
	char node[64];					/* node name */
	int i;
	MV_BOOL isDevice = MV_FALSE;

	/* disable usb3 nodes */
	for (i = 0; i < MV_USB3_MAX_HOST_PORTS; i++) {
		if (mvBoardIsUsb3PortDevice(i)) {
			/* if Port is Device, disable corresponding Host node */
			sprintf(propval, "disabled");
			isDevice = MV_TRUE;
			break;
		}
	}

	if (isDevice == MV_FALSE)
		return 0;

	sprintf(node, "usb3@%x", MV_USB2_USB3_REGS_OFFSET(USB3_UNIT_ID, i));
	if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0)
		mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);

	/* enable 'u3d' and 'udc' nodes */
	sprintf(propval, "okay");
	sprintf(node, "u3d@%x", MV_USB3_DEVICE_REGS_OFFSET);
	if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
		mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
		return 0;
	}
	sprintf(node, "udc@%x", MV_USB3_DEVICE_USB2_REGS_OFFSET);
	if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
		mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
		return 0;
	}
	return 0;
}
#endif /* MV_INCLUDE_USB_DEVICE */
/*******************************************************************************
* mv_fdt_update_pinctrl
*
* DESCRIPTION:
* target		: update compatible field of pinctrl node.
* node, properties	: -property compatible @ node pinctrl.
* dependencies		: - for a38x/a39x: according to DEV_ID_REG value "device model type"
*			  - for MSYS: currently static for all devices
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_pinctrl(void *fdt)
{
	char propval[128];				/* property value */
	const char *prop = "compatible";		/* property name */
	const char *node = "pinctrl";			/* node name */

	/* update pinctrl driver 'compatible' property, according to device model type */
	mvBoardPinCtrlNameGet(propval);

	if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
		mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
		return 0;
	}

	return 0;
}

/*******************************************************************************
* mv_fdt_update_ethnum
*
* DESCRIPTION:
* target		: update ethernet nodes for PP3/Neta drivers.
* node, properties	: PP3 Driver	: -property status @ node nic@X.
*					  -property phy-mode @ node macX and reg @ node phy @ node macX.
*			  Neta Driver	: -property status and phy-mode @ node ethernet@X.
*					  -property reg @ node ethernet-phy@X.
*					  -property phy @ node ethernet@X (if PP SMI is used)
*
* dependencies		: pBoardNetComplexInfo entry in board structure.
*			  pBoardMacInfo entry in board structure.
*	     		  For PP3/Neta driver we get ethernet nodes via "nic@/ethernet@" aliasses.
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_ethnum(void *fdt)
{
	MV_32 phyAddr;
	int port;
	int len = 0;			/* property length */
	int ethPortsNum;		/* nodes' counter */
	int nodeoffset;			/* node offset from libfdt */
	int aliasesoffset;		/* aliases node offset from libfdt */
	int phyoffset;
#ifdef CONFIG_NET_COMPLEX
	int emacoffset;
#endif
	const uint32_t *phandle;
	char prop[20];			/* property name */
	char propval[20];		/* property value */
	const char *node;		/* node name */
	int depth = 1;
	int err;
	MV_BOOL phy_node_update;


	/* Get number of ethernet nodes */
	aliasesoffset = mv_fdt_find_node(fdt, "aliases");
	if (aliasesoffset < 0) {
		mv_fdt_dprintf("Lack of 'aliases' node in device tree\n");
		return 0;
	}

	/* Walk trough all aliases and count Ethernet ports entries */
	ethPortsNum = 0;
	nodeoffset = aliasesoffset;
	do {
		nodeoffset = fdt_next_node(fdt, nodeoffset, &depth);
		node = fdt_get_name(fdt, nodeoffset, NULL);
	/* For PP3 driver node name is 'nic@X', for neta driver node name is 'ethernet@X'
	   Node name can be NULL, but it's OK */
#ifdef CONFIG_NET_COMPLEX
		if (strncmp(node, "nic@", 4) == 0)
#else
		if (strncmp(node, "ethernet@", 9) == 0)
#endif
			ethPortsNum++;
	} while (node != NULL);
	mv_fdt_dprintf("Number of ethernet nodes in DT = %d\n", ethPortsNum);

	/* Update board configuration for FDT update if needed */
	mvBoardUpdateConfigforDT();

	/* Get path to ethernet node from property value */
	for (port = 0; port < ethPortsNum; port++) {

		/* Get path to ethernet node from property value */
		sprintf(prop, "ethernet%d", port);
		node = fdt_getprop(fdt, aliasesoffset, prop, &len);
		if (node == NULL) {
			mv_fdt_dprintf("Lack of '%s' property in 'aliases' node\n", prop);
			return 0;
		}
		if (len == 0) {
			mv_fdt_dprintf("Empty property value\n");
			return 0;
		}
		/* Alias points to the ETH port node using full DT path */
		nodeoffset = fdt_path_offset(fdt, node);

		/* Set ethernet port status to 'disabled' */
		/* Enable valid ports and configure their parametrs, disable non valid ones */
		if (mvBoardIsEthConnected(port) != MV_TRUE)
			sprintf(propval, "disabled");
		else {
			/* Configure PHY address */
			phyAddr = mvBoardPhyAddrGet(port);
			if (phyAddr == -1)
				phyAddr = 999; /* Inband management - see mv_netdev.c for details */

#ifdef MV_PP_SMI
			/* - 'phy' property must point to the corresponding 'ethernet-phy@' node in the
			 *   internal/external mdio node
			 * - by default 'phy' points to internal mdio node (/soc/internal-regs/mdio)
			 * - when using external PP smi, point 'phy' to the correct 'indirect-phy'
			 *   external mdio node (/soc/mdio)
			 */

			/* get the address (phandle) of indirect-phy node */
			phandle = fdt_getprop(fdt, nodeoffset, "indirect-phy", &len);
			if (len == 0) {
				mv_fdt_dprintf("Empty \"indirect-phy\" property value\n");
				return 0;
			}
			sprintf(prop, "phy");
			mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop, phandle, 4));
#endif
			/* The ETH node we found contains a pointer (phandle) to its PHY
			   The phandle is a unique numeric identifier of a specific node.
			   For PP3 driver the phy address under 'phy' field of 'mac' node
			   For NETA driver the phy address under 'phy' field */
#ifdef CONFIG_NET_COMPLEX
			phandle = fdt_getprop(fdt, nodeoffset, "emac-data", &len);
			if (phandle == NULL || len == 0) {
				mv_fdt_dprintf("Empty \"emac-data\" property value\n");
				return 0;
			}
			emacoffset = fdt_node_offset_by_phandle(fdt, ntohl(*phandle));
			if (emacoffset < 0) {
				mv_fdt_dprintf("Failed to get PHY node by phandle %x\n", ntohl(*phandle));
				return 0;
			}
			phandle = fdt_getprop(fdt, emacoffset, "phy", &len);
#else
			phandle = fdt_getprop(fdt, nodeoffset, "phy", &len);
#endif
			if (phandle == NULL || len == 0) {
				mv_fdt_dprintf("Empty \"phy\" property value for port %d\n", port);
#ifdef CONFIG_NET_COMPLEX
				/* nic0 has no SMI phy but XSMI phy, there is no phy property for nic0.
				    for nic0, we can't update the phy property, but we need to update the other
				    attributes, e,g. "phy-mode" and "status" */
				phy_node_update = MV_FALSE;
#else
				return 0;
#endif
			} else
				phy_node_update = MV_TRUE;

			if (MV_TRUE == phy_node_update) {
				phyoffset = fdt_node_offset_by_phandle(fdt, ntohl(*phandle));
				if (phyoffset < 0) {
					mv_fdt_dprintf("Failed to get PHY node by phandle %x\n", ntohl(*phandle));
					return 0;
				}

				/* Setup PHY address in DT in "reg" property of an appropriate PHY node for
				   Neta/PP3 driver.
				   This value is HEX number, not a string, and uses network byte order */
				phyAddr = htonl(phyAddr);
				sprintf(prop, "reg");

				mv_fdt_modify(fdt, err, fdt_setprop(fdt, phyoffset, prop, &phyAddr, sizeof(phyAddr)));
				if (err < 0) {
					mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n",
								prop, fdt_get_name(fdt, phyoffset, NULL));
					return 0;
				}
				mv_fdt_dprintf("Set property '%s' of node '%s' to %#010x\n",
							prop, fdt_get_name(fdt, phyoffset, NULL), ntohl(phyAddr));
			}

			/* Configure PHY mode */
			switch (mvBoardPortTypeGet(port)) {
			case MV_PORT_TYPE_QSGMII:
#ifdef CONFIG_NET_COMPLEX
				sprintf(propval, "qsgmii");
				break;
#endif
			case MV_PORT_TYPE_SGMII:/* the ETH driver in Linux expect QSGMII to init the ports */
				sprintf(propval, "sgmii");
				break;
			case MV_PORT_TYPE_RGMII:
				sprintf(propval, "rgmii");
				break;
#ifdef CONFIG_NET_COMPLEX
			case MV_PORT_TYPE_RXAUI:
				sprintf(propval, "rxaui");
				break;
			case MV_PORT_TYPE_XAUI:
				sprintf(propval, "xaui");
				break;
#endif /*CONFIG_NET_COMPLEX*/
			default:
				mv_fdt_dprintf("Bad port type received for interface %d\n", port);
				return 0;
			}

			/* Setup PHY connection type in DT.
			   PP3 driver update the type under 'mac' node (emacoffset point to the node)
			   NETA driver update the type under 'ethernet@' node (nodeoffset, point to the node) */
			sprintf(prop, "phy-mode");
#ifdef CONFIG_NET_COMPLEX
			mv_fdt_modify(fdt, err, fdt_setprop(fdt, emacoffset, prop, propval, strlen(propval)+1));
#else
			mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop, propval, strlen(propval)+1));
#endif
			if (err < 0) {
				mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
				return 0;
			}
			mv_fdt_dprintf("Set '%s' property to '%s' in '%s' node\n", prop, propval, node);

#if defined(CONFIG_ARMADA_39X) && defined(CONFIG_CMD_BOARDCFG)
			MV_U32 gpConfig = mvBoardSysConfigGet(MV_CONFIG_GP_CONFIG);
			u32 phySpeed = htonl(2500);
			/* Temporary fix for A39x switch boards, see full description in mvBoardUpdateConfigforDT function */
			if (mvBoardIdGet() == A39X_RD_69XX_ID) {
				/* In HGW 2.5G configurations for 395 board, phy speed is 2.5Gb,
				   update phy-speed entry in mac0 node to 2500 */
				if ((port == 0) && (gpConfig == MV_GP_CONFIG_HGW_AP_2_5G ||
						gpConfig == MV_GP_CONFIG_HGW_AP_2_5G_SATA)) {
					sprintf(prop, "phy-speed");
					mv_fdt_modify(fdt, err,
						fdt_setprop(fdt, emacoffset, prop, &phySpeed, sizeof(phySpeed)));
				}
				if ((port == 3) && (gpConfig == MV_GP_CONFIG_EAP_10G || gpConfig == MV_GP_CONFIG_EAP_1G)) {
					sprintf(prop, "force-link");
					sprintf(propval, "yes");
					mv_fdt_modify(fdt, err,
						fdt_setprop(fdt, emacoffset, prop, propval, strlen(propval)+1));
				} else if ((port == 2) && (gpConfig == MV_GP_CONFIG_HGW_AP_2_5G ||
						gpConfig == MV_GP_CONFIG_HGW_AP_2_5G_SATA)) {
					sprintf(prop, "force-link");
					sprintf(propval, "yes");
					mv_fdt_modify(fdt, err,
						fdt_setprop(fdt, emacoffset, prop, propval, strlen(propval)+1));
				}
			}
#endif
			/* Last property to set is the "status" - common for valid and non-valid ports */
			sprintf(propval, "okay");
		}
		sprintf(prop, "status");
		mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop, propval, strlen(propval)+1));
		if (err < 0) {
			mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
			return 0;
		}
		mv_fdt_dprintf("Set '%s' property to '%s' in '%s' node\n", prop, propval, node);

	} /* For all ports in board sctructure */

	return 0;
}
/*******************************************************************************
* mv_fdt_update_mpp_config
*
* DESCRIPTION:
*   Update FDT entries related to mpp configuration , i.e: pinctrl-xxx
*
* INPUT:
*       fdt             - FDT
*
* OUTPUT:
*       None.
*
* RETURN:
*       -1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_mpp_config(void *fdt)
{
#ifdef CONFIG_NET_COMPLEX
	if ((!mvBoardIsEthConnected(1)) || (mvBoardPortTypeGet(1) != MV_PORT_TYPE_RGMII)) {
		/*By default we have the pin ctrl properties in fdt, if MAC1 not connected to RGMII we disable it*/
		fdt_delprop(fdt, mv_fdt_find_node(fdt, "pp3_platform"), "pinctrl-0");
		fdt_delprop(fdt, mv_fdt_find_node(fdt, "pp3_platform"), "pinctrl-names");
	}
#endif
	return 0;
}
/*******************************************************************************
* mv_fdt_update_flash
*
* DESCRIPTION:
*   Update FDT entires related to board flash devices according to board sctructures.
*   The board sctructures are scanned and initialized at uboot startup using board
*   boot source and bord type. This function obtails flash entries marked as active
*   during the board initalization process and activates appropriate nodes in FDT.
*   Nodes corresponding to non-active flash devices are disabled in FDT.
*
* INPUT:
*	fdt		- FDT
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_flash(void *fdt)
{
	MV_U32 device;
	char propval[10];				/* property value */
	const char *prop = "status";	/* property name */
	char node[64];					/* node name */
	MV_U32 unit, maxUnits;			/* number of interface controller units */
	MV_U32 chipSel;
	MV_BOOL interfaceIsActive;

	/* start with SPI flashes */
	maxUnits = mvCtrlSocUnitInfoNumGet(SPI_UNIT_ID);
	/* This loops assume that the board flash mapping is fully mapped.
	   i.e.: if SoC has 2 SPI flash units, it's board mapping describe
	   unit status for both. */
	for (unit = 0; unit < maxUnits; unit++) {
		interfaceIsActive = MV_FALSE;
		for (device = 0; device < maxUnits; device++) {
			/* Only devices related to current bus/unit */
			if (mvBoardGetDevBusNum(device, BOARD_DEV_SPI_FLASH) != unit)
				continue;

			if (mvBoardGetDevState(device, BOARD_DEV_SPI_FLASH) == MV_TRUE) {
				interfaceIsActive = MV_TRUE; /* One active device is enough */
				break;
			}
		} /* SPI devices */

		if (interfaceIsActive == MV_TRUE)
			sprintf(propval, "okay"); /* Enable active SPI unit/bus */
		else
			sprintf(propval, "disabled"); /* disable NON active SPI unit/bus */

		sprintf(node, "spi@%x", MV_SPI_REGS_OFFSET(unit));
		if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
			mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
			return 0;
		}
	} /* SPI units/buses */

	/* handle NAND flashes - there is only one NAND unit, but different CS are possible */
	maxUnits = mvCtrlSocUnitInfoNumGet(NAND_UNIT_ID);
	interfaceIsActive = MV_FALSE;
	for (device = 0; device < maxUnits; device++) {
		if (mvBoardGetDevState(device, BOARD_DEV_NAND_FLASH) == MV_TRUE) {
			interfaceIsActive = MV_TRUE; /* One active device is enough */
			/* Once a NAND node updated, there is no reason to search for other devices */
			break;
		}
	} /* NAND devices */

	if (interfaceIsActive == MV_TRUE)
		sprintf(propval, "okay"); /* Enable NAND interface if found active device */
	else
		sprintf(propval, "disabled"); /* disable NAND interface if NOT found active devices */

	sprintf(node, "%s%x", MV_NFC_FDT_NODE_NAME, MV_NFC_REGS_OFFSET);
	if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
		mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
		return 0;
	}

	/* handle NOR flashes - there is only one NOR unit, but different CS are possible */
	maxUnits = mvCtrlSocUnitInfoNumGet(DEVBUS_UNIT_ID);
	chipSel = 0xFFFF;
	for (device = 0;  device < maxUnits; device++) {
		if (mvBoardGetDevState(device, BOARD_DEV_NOR_FLASH) != MV_TRUE)
			continue;

		chipSel = mvBoardGetDevCSNum(device, BOARD_DEV_NOR_FLASH);
		if (chipSel == DEV_BOOCS)	/* Special case - this value is not close to the rest of Device Bus */
			chipSel = 4;			/* Chip Selects (0 to 3) in target definitiond enum, */
		else						/* so it will be hadled just as a 4th chip select */
			chipSel -= DEVICE_CS0;	/* Substract the base value for getting 0-3 CS range */

		/* Once an active NOR flash entry found, there is no reason to search for others */
		break;
	} /* NOR devices */

	/* Walk through Device Bus entries and update them */
	for (device = 0; device < 5; device++) {
		if (device == chipSel)
			sprintf(propval, "okay"); /* Enable NOR interface if found active device */
		else
			sprintf(propval, "disabled"); /* disable NOR interface if NOT found active devices */

		if (device == 4)	/* Fourth CS is a Boot CS - see the comment above */
			sprintf(node, "devbus-bootcs");
		else
			sprintf(node, "devbus-cs%d", device);

		if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
			mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
			return 0;
		}
	}
	return 0;
}

/*******************************************************************************
* mv_fdt_set_node_prop
*
* DESCRIPTION:
* 	Set a named node property to a specific value
*
* INPUT:
*	fdt		- FDT
*	node	- FDT node name
*	prop	- FDT node property name
*	propval	- property value to set
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_set_node_prop(void *fdt, const char *node, const char *prop, const char *propval)
{
	int err, nodeoffset = 0; /* nodeoffset: node offset from libfdt */

	if (node != NULL) { /* node == NULL --> search property in root node */
		nodeoffset = mv_fdt_find_node(fdt, node);
		if (nodeoffset < 0) {
			mv_fdt_dprintf("Lack of '%s' node in device tree\n", node);
			return -1;
		}
	}

	if (strncmp(fdt_get_name(fdt, nodeoffset, NULL), node, strlen(node)) == 0) {
		mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop, propval, strlen(propval)+1));
		if (err < 0) {
			mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", prop, node);
			return -1;
		}
	}
	mv_fdt_dprintf("Set '%s' property to '%s' in '%s' node\n", prop, propval, node);

	return 0;
}

#if 0 /* not compiled, since this routine is currently not in use  */
/*******************************************************************************
* mv_fdt_remove_prop
*
* DESCRIPTION:
*
* INPUT:
*	fdt.
*	path.
*	name.
*	nodeoff.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_remove_prop(void *fdt, const char *path,
				const char *name, int nodeoff)
{
	int error;

	error = fdt_delprop(fdt, nodeoff, name);
	if (error == -FDT_ERR_NOTFOUND) {
		mv_fdt_dprintf("'%s' already removed from '%s' node\n",
			       name, path);
		return 0;
	} else if (error < 0) {
		mv_fdt_dprintf("Removing '%s' from '%s' node failed\n",
			       name, path);
		return error;
	} else {
		mv_fdt_dprintf("Removing '%s' from '%s' node succeeded\n",
			       name, path);
		return 0;
	}
}
#endif

/*******************************************************************************
* mv_fdt_remove_node
*
* DESCRIPTION:
*
* INPUT:
*	fdt.
*	path.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_remove_node(void *fdt, const char *path)
{
	int error;
	int nodeoff;

	nodeoff = mv_fdt_find_node(fdt, path);
	error = fdt_del_node(fdt, nodeoff);
	if (error == -FDT_ERR_NOTFOUND) {
		mv_fdt_dprintf("'%s' node already removed from device tree\n",
			       path);
		return 0;
	} else if (error < 0) {
		mv_fdt_dprintf("Removing '%s' node from device tree failed\n",
			       path);
		return error;
	} else {
		mv_fdt_dprintf("Removing '%s' node from device tree "
			       "succeeded\n", path);
		return 0;
	}
}

/*******************************************************************************
* mv_fdt_scan_and_set_alias
*
* DESCRIPTION:
*
* INPUT:
*	fdt.
*	name.
*	alias.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_scan_and_set_alias(void *fdt,
					const char *name, const char *alias)
{
	int i = 0;		/* alias index */
	int nodeoffset;		/* current node's offset from libfdt */
	int nextoffset;		/* next node's offset from libfdt */
	int delta;		/* difference between next and current offset */
	int err = 0;		/* error number */
	int level = 0;		/* current fdt scanning depth */
	char aliasname[128];	/* alias name to be stored in '/aliases' node */
	char path[256] = "";	/* full path to current node */
	char tmp[256];		/* auxiliary char array for extended node name*/
	char *cut;		/* auxiliary char pointer */
	const char *node;	/* node name */
	uint32_t tag;		/* device tree tag at given offset */
	const struct fdt_property *prop;

	/* Check if '/aliases' node exist. Otherwise create it */
	nodeoffset = mv_fdt_find_node(fdt, "aliases");
	if (nodeoffset < 0) {
		mv_fdt_modify(fdt, err, fdt_add_subnode(fdt, 0, "aliases"));
		if (err < 0) {
			mv_fdt_dprintf("Creating '/aliases' node failed\n");
			return -1;
		}
		nodeoffset = fdt_path_offset(fdt, "/aliases");
	}

	/* Check if there are pre-defined aliases and rely on them
	 * instead of scanning the Device Tree
	 */
	for (nextoffset = fdt_first_property_offset(fdt, nodeoffset);
	     (nextoffset >= 0);
	     (nextoffset = fdt_next_property_offset(fdt, nextoffset))) {

		prop = fdt_get_property_by_offset(fdt, nextoffset, NULL);
		if (!prop) {
			nextoffset = -FDT_ERR_INTERNAL;
			break;
		}
		if (strncmp(fdt_string(fdt, fdt32_to_cpu(prop->nameoff)),
			    alias, strlen(alias)) == 0) {
			mv_fdt_dprintf("'%s' aliases exist. Skip scanning DT for '%s' nodes\n",
				       alias, name);
			return 0;
		}
	}

	/* Scan device tree for nodes that that contain 'name' substring and
	 * create their 'alias' with respective number */

	nodeoffset = 0;
	while (level >= 0) {
		tag = fdt_next_tag(fdt, nodeoffset, &nextoffset);
		switch (tag) {
		case FDT_BEGIN_NODE:
			node = fdt_get_name(fdt, nodeoffset, NULL);
			sprintf(tmp, "/%s", node);
			if (nodeoffset != 0)
				strcat(path, tmp);
			if (strstr(node, name) != NULL) {
				delta = nextoffset - nodeoffset;
				sprintf(aliasname, "%s%d", alias, i);
				nodeoffset = mv_fdt_find_node(fdt, "aliases");
				if (nodeoffset < 0)
					goto alias_fail;
				mv_fdt_modify(fdt, err, fdt_setprop(fdt,
							nodeoffset, aliasname,
							path, strlen(path)+1));
				if (err < 0)
					goto alias_fail;
				nodeoffset = fdt_path_offset(fdt, path);
				if (nodeoffset < 0)
					goto alias_fail;
				nextoffset = nodeoffset + delta;
				mv_fdt_dprintf("Set alias %s=%s\n", aliasname,
					       path);
				i++;
			}
			level++;
			if (level >= MV_FDT_MAXDEPTH) {
				mv_fdt_dprintf("%s: Nested too deep, "
					       "aborting.\n", __func__);
				return -1;
			}
			break;
alias_fail:
			mv_fdt_dprintf("Setting alias %s=%s failed\n",
				       aliasname, path);
			return -1;
		case FDT_END_NODE:
			level--;
			cut = strrchr(path, '/');
			*cut = '\0';
			if (level == 0)
				level = -1;		/* exit the loop */
			break;
		case FDT_PROP:
		case FDT_NOP:
			break;
		case FDT_END:
			mv_fdt_dprintf("Device tree scanning failed - end of "
				       "tree tag\n");
			return -1;
		default:
			mv_fdt_dprintf("Device tree scanning failed - unknown "
				       "tag\n");
			return -1;
		}
		nodeoffset = nextoffset;
	}

	return 0;
}

/*******************************************************************************
* mv_fdt_nfc_driver_type
*
* DESCRIPTION:
*
* INPUT:
*	fdt.
*	offset.
*	check_status.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_nfc_driver_type(void *fdt, int *offset,
				  int check_status)
{
	int nodeoffset, type;
	const void *status;
	const char *compat[3] = {"marvell,armada370-nand",
				 "marvell,armada-nand",
				 "marvell,armada-375-nand"};

	for (type = 0; type < 3; type++) {
		nodeoffset = fdt_node_offset_by_compatible(fdt, -1,
							   compat[type]);
		if (nodeoffset < 0)
			continue;

		if (!check_status) {
			mv_fdt_dprintf("Detected NFC driver - %s\n",
				       compat[type]);
			break;
		}

		status = fdt_getprop(fdt, nodeoffset, "status", NULL);
		if (!status || strncmp(status, "okay", 4) == 0) {
			mv_fdt_dprintf("Enabled NFC driver - %s\n",
				       compat[type]);
			break;
		}
	}

	*offset = nodeoffset;
	return type;
}

/*******************************************************************************
* mv_fdt_nand_mode_fixup
*
* DESCRIPTION:
*   Obtain the NAND ECC value from u-boot environment and upodate FDT NAND node
*   accordingly.
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_nand_mode_fixup(void *fdt)
{
	u32 ecc_val;
	int nfcoffset, nfc_driver_hal, err;
	char *nfc_config;
	char prop[128];
	char propval[7];

	/* Search for enabled NFC driver in DT */
	nfc_driver_hal = mv_fdt_nfc_driver_type(fdt, &nfcoffset, 1);
	if (nfc_driver_hal == MV_FDT_NFC_NONE) {
		mv_fdt_dprintf("No NFC driver enabled in device tree\n");
		return 0;
	}

	/* Search for 'nandEcc' parameter in the environment */
	nfc_config = getenv("nandEcc");
	if (!nfc_config) {
		mv_fdt_dprintf("Keep default NFC configuration\n");
		return 0;
	}

	nfc_config += strlen("nfcConfig") + 1;

	/* Check for ganged mode */
	if (strncmp(nfc_config, "ganged", 6) == 0) {
		nfc_config += 7;

		if (!nfc_driver_hal) {
			mv_fdt_dprintf("NFC update: pxa3xx-nand driver "
				       "doesn't support ganged mode\n");
			goto check_ecc;
		}

		sprintf(prop, "%s", "nfc,nfc-mode");
		sprintf(propval, "%s", "ganged");
		mv_fdt_modify(fdt, err, fdt_setprop(fdt, nfcoffset,
						    prop, propval,
						    strlen(propval) + 1));
		if (err < 0) {
			mv_fdt_dprintf("NFC update: fail to modify '%s'\n",
				       prop);
			return 0;
		}
		mv_fdt_dprintf("NFC update: set '%s' property to '%s'\n",
			       prop, propval);
	}

check_ecc:
	/* Check for ECC type directive */
	if (strcmp(nfc_config, "1bitecc") == 0)
		ecc_val = nfc_driver_hal ? MV_NFC_ECC_HAMMING : 1;
	else if (strcmp(nfc_config, "4bitecc") == 0)
		ecc_val = nfc_driver_hal ? MV_NFC_ECC_BCH_2K : 4;
	else if (strcmp(nfc_config, "8bitecc") == 0)
		ecc_val = nfc_driver_hal ? MV_NFC_ECC_BCH_1K : 8;
	else if (strcmp(nfc_config, "12bitecc") == 0)
		ecc_val = nfc_driver_hal ? MV_NFC_ECC_BCH_704B : 12;
	else if (strcmp(nfc_config, "16bitecc") == 0)
		ecc_val = nfc_driver_hal ? MV_NFC_ECC_BCH_512B : 16;
	else {
		mv_fdt_dprintf("NFC update: invalid nfcConfig ECC parameter - \"%s\"\n", nfc_config);
		return 0;
	}

	if (!nfc_driver_hal)
		sprintf(prop, "%s", "nand-ecc-strength");
	else
		sprintf(prop, "%s", "nfc,ecc-type");

	mv_fdt_modify(fdt, err, fdt_setprop_u32(fdt, nfcoffset, prop,
						ecc_val));
	if (err < 0) {
		mv_fdt_dprintf("NFC update: fail to modify'%s'\n", prop);
		return -1;
	}
	mv_fdt_dprintf("NFC update: set '%s' property to %d\n", prop, ecc_val);

	return 0;
}

static int mv_fdt_board_compatible_name_update(void *fdt)
{
	char propval[256];				/* property value */
	const char *prop = "compatible";		/* property name */
	char node[64];					/* node name */
	MV_U8 len, err;

	/* set compatible property as concatenated strings.
	 * (i.e: compatible = "marvell,a385-db", "marvell,armada385", "marvell,armada38x";
	 * concatenated strings are appended after the NULL character of the previous
	 * sprintf wrote.  This is how a DT stores multiple strings in a property.
	 * as a result, strlen is not relevant, and len has to be calculated  */
	len = mvBoardCompatibleNameGet(propval);

	if (len > 0)
		mv_fdt_modify(fdt, err, fdt_setprop(fdt,0 , prop, propval, len));
	if (len <= 0 || err < 0) {
		mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", prop, node);
		return 0;
	}
	mv_fdt_dprintf("Set '%s' property to Root node\n", prop);

	return 0;
}

/*******************************************************************************
* mv_fdt_board_model_name_update
*
* DESCRIPTION:
* target		: Update model property in root node
* node, properties:	: -property model @ root node
* dependencies		: modelName in board structure.
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_board_model_name_update(void *fdt)
{
	char propval[128];				/* property value */
	const char *prop = "model";			/* property name */
	MV_U8 err;

	mvBoardGetModelName(propval);

	mv_fdt_modify(fdt, err, fdt_setprop(fdt, 0, prop, propval, strlen(propval) + 1));

	if (err < 0) {
		mv_fdt_dprintf("Modifying '%s' in root node failed\n", prop);
		return 0;
	}
	mv_fdt_dprintf("Set '%s' property to Root node\n", prop);

	return 0;
}

/*******************************************************************************
* mv_fdt_update_serial
*
* DESCRIPTION:
* target		: Update status field of serial nodes.
* node, properties:	: -property status @ node serial
* dependencies		: checks if it is a active serial port via mvUartPortGet function.
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_serial(void *fdt)
{
	char propval[10];				/* property value */
	const char *prop = "status";	/* property name */
	char node[64];					/* node name */
	int i, defaultSerialPort = mvUartPortGet();

	for (i = 0; i < MV_UART_MAX_CHAN; i++) {
		if (i == defaultSerialPort)
			sprintf(propval, "okay"); /* Enable active Serial port node */
		else
			sprintf(propval, "disabled"); /* Disable non-active Serial port node */

		sprintf(node, "serial@%x", MV_UART_REGS_OFFSET(i));
		if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
			mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
			return 0;
		}
	}

	return 0;
}

#ifdef CONFIG_PIC_GPIO
/*******************************************************************************
* mv_fdt_update_pic_gpio
*
* DESCRIPTION: GPIO information for peripheral integrated controller (PIC)
*
*  PIC GPIO pins are represented in 2 different nodes in Device Tree:
* 1. pinctrl/pic-pins-0/marvell,pins = "mpp33", "mpp34";
* 2. pm_pic/ctrl-gpios = <0x0000000c 0x00000001 0x00000001 0x0000000c 0x00000002 0x00000001>;
*    Each GPIO is represented in 'ctrl-gpios' property with 3 values of 32 bits:
*    '<gpios_phandle[gpio_group_num]   gpio_pin_num_in_group   ACTIVE_LOW>'
*    i.e : for mpp 33, and given pinctrl-0 = <0x0000000b> :
*    gpio_group_num	= 33/32 = 1
*    gpio_pin_num_in_group= 33%32 = 1
*    ACTIVE_LOW = 1
*    DT entry is : '<0x0000000c 0x00000001 0x00000001>';
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
#define MAX_GPIO_NUM 10
static int mv_fdt_update_pic_gpio(void *fdt)
{
	const char *picPinsNode 	= "pic-pins-0";
	const char *marvell_pins_prop 	= "marvell,pins";
	const char *pm_picNode  	= "pm_pic";
	const char *ctrl_gpios_prop 	= "ctrl-gpios";
	MV_U32 picGpioInfo[MAX_GPIO_NUM];
	MV_U32 ctrl_gpios_prop_value[3*MAX_GPIO_NUM]; /* 3*32bit is required per GPIO */
	char propval[256] = "";
	int err, len = 0, nodeoffset, i, gpioMaxNum = mvBoardPICGpioGet(picGpioInfo);
	MV_U32 gpios_phandle[2];	/* phandle values of gpio nodes */
	MV_U32 gpio_offset;
	char gpio_node[30] = "";

	/* if board has no dedicated PIC MPP Pins: remove 'pm_pic' & 'pinctrl/pic-pins-0' */
	if (gpioMaxNum <= 0) {
		mv_fdt_dprintf("'pic-pins-0' & 'pm_pic' nodes removed: no dedicated PIC GPIO Pins\n");
		if (mv_fdt_remove_node(fdt, picPinsNode))
			mv_fdt_dprintf("Failed to remove %s\n", picPinsNode);
		if (mv_fdt_remove_node(fdt, pm_picNode))
			mv_fdt_dprintf("Failed to remove %s\n", pm_picNode);
		return 0;
	}

	/* fetch gpio nodes' phandle.
	 * every GPIO group have GPIO node id DT
	 * and for each GPIO pin, an entry is added to the ctrl-gpios,
	 * in order to link between the entry and the relevant gpio node for the relevant
	 * group, the phandle of the gpio node is added to the entry */
	for (i = 0; i < 2; ++i) {
		sprintf(gpio_node, "gpio@%X", MV_GPP_REGS_OFFSET(i));
		gpio_offset = mv_fdt_find_node(fdt, gpio_node);
		gpios_phandle[i] = fdt_get_phandle(fdt, gpio_offset);
		if (!gpios_phandle[i]) {
			mvOsPrintf("Failed updating suspend-resume GPIO pin settings in DT\n");
			return 0;
		}
	}

	/* set pinctrl/pic-pins-0/marvell,pins property as concatenated strings.
	 * (i.e : marvell,pins = "mpp33", "mpp34", "mpp35")
	 * append next string after the NULL character that the previous
	 * sprintf wrote.  This is how a DT stores multiple strings in a property.*/
	for (i = 0 ; i < gpioMaxNum ; i++)
		len += sprintf(propval + len, "mpp%d", picGpioInfo[i]) + 1;

	nodeoffset = mv_fdt_find_node(fdt, picPinsNode); 	/* find 'pic-pins-0' node */
	mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, marvell_pins_prop, propval, len));
	if (err < 0) {
		mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", marvell_pins_prop, picPinsNode);
		return 0;
	}
	mv_fdt_dprintf("Set '%s' property in '%s' node\n", marvell_pins_prop, picPinsNode);


	/* set pm_pic/ctrl-gpios property: */
	nodeoffset = mv_fdt_find_node(fdt, pm_picNode); 	/* find 'pm_pic' node */

	if (len == 0) {
		printf("Empty property value\n");
		return 0;
	}
	/* Each GPIO is represented in 'ctrl-gpios' property with 3 values of 32 bits:
	 * '<gpios_phandle[gpio_group_num]   gpio_pin_num_in_group   ACTIVE_LOW>'
	 * i.e : for mpp 33, and given pinctrl-0 = <0x0000000b> :
	 * gpio_group_num	= 33/32 = 1
	 * gpio_pin_num_in_group= 33%32 = 1
	 * ACTIVE_LOW = 1
	 * DT entry is : '<0x0000000c 0x00000001 0x00000001>'; */
	for (i = 0 ; i < gpioMaxNum ; i++) {
		/* pinctrl_0_base +gpio Group num */
		ctrl_gpios_prop_value[3*i] = htonl(gpios_phandle[picGpioInfo[i] / 32]);

		/* 32 pins in each group */
		ctrl_gpios_prop_value[3*i + 1] = htonl(picGpioInfo[i] % 32);

		/* GPIO_ACTIVE_LOW = 1 */
		ctrl_gpios_prop_value[3*i + 2] = htonl(0x1);
	}

	mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, ctrl_gpios_prop, ctrl_gpios_prop_value, 4*gpioMaxNum*3));
	if (err < 0) {
		mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", ctrl_gpios_prop, pm_picNode);
		return 0;
	}
	mv_fdt_dprintf("Set '%s' property in '%s' node\n", ctrl_gpios_prop, pm_picNode);

	return 0;

}
#endif /* CONFIG_PIC_GPIO */
#ifdef MV_INCLUDE_SWITCH
/*******************************************************************************
* mv_fdt_update_switch
*
* DESCRIPTION:
*  This routines updates switch node and ethernet-phy node on status and rgmii_(t)rx_timing_delay properties
*  according to Switch existence and Device ID detection, this route also set ethernet-phy@i node on reg property
*  if ethi is connected to switch
*
* INPUT:
*	fdt.
*
* OUTPUT:
*	None.
*
* RETURN:
*	-1 on error os 0 otherwise.
*
*******************************************************************************/
static int mv_fdt_update_switch(void *fdt)
{
	char propval[10];				/* property value */
	u32 propval_u32 = 0;
	char prop[50];					/* property name */
	char node[64];					/* node name */
	MV_U16 switch_device_id;
	int nodeoffset;					/* node offset from libfdt */
	int err;

	switch_device_id = mvEthSwitchGetDeviceID();

	/*if switch module is connected and detect correct switch device ID, enable switch in fdt*/
	if (switch_device_id > 0)
		sprintf(propval, "okay");
	else
		sprintf(propval, "disabled");

	sprintf(node, "switch");
	sprintf(prop, "status");

	if (mv_fdt_set_node_prop(fdt, node, prop, propval) < 0) {
		mv_fdt_dprintf("Failed to set property '%s' of node '%s' in device tree\n", prop, node);
		return 0;
	}

	if (mvBoardIsSwitchConnected()) {
		int i = 0;
		switch_device_id >>= QD_REG_SWITCH_ID_PRODUCT_NUM_OFFSET;

		for (i = 0; i < mvCtrlEthMaxPortGet(); i++) {
			/*change the switch connected eth ports' phy to 999*/
			/*999 mean that ethernet is not connected to phy*/
			if (mvBoardSwitchConnectedPortGet(i) != -1) {
				sprintf(node, "ethernet-phy@%d", i);
				sprintf(prop, "reg");
				propval_u32 = 999;

				nodeoffset = mv_fdt_find_node(fdt, node);
				if (nodeoffset < 0) {
					mv_fdt_dprintf("Lack of '%s' node in device tree\n", node);
					return -1;
				}

				/* Setup PHY address in DT in "reg" property of an appropriate PHY node.
				This value is HEX number, not a string, and uses network byte order */
				propval_u32 = htonl(propval_u32);
				mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop,
					&propval_u32, sizeof(propval_u32)));
				if (err < 0) {
					mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", prop, node);
					return -1;
				}
			}
		}

		/*for E6172 switch, rgmii_rx_timing_delay and rgmii_tx_timing_delay should be enabled*/
		if (switch_device_id == MV_E6172_PRODUCT_NUM) {
			/*set rgmii_rx_timing_delay and rgmii_tx_timing_delay to be enabled*/
			sprintf(node, "switch");
			sprintf(prop, "rgmii_rx_timing_delay");
			/*enable delay to RXCLK for IND inputs when port is in RGMII mode*/
			propval_u32 = 1;

			nodeoffset = mv_fdt_find_node(fdt, node);
			if (nodeoffset < 0) {
				mv_fdt_dprintf("Lack of '%s' node in device tree\n", node);
				return -1;
			}

			/* Setup rgmii_rx_timing_delay in DT.
			This value is HEX number, not a string, and uses network byte order */
			propval_u32 = htonl(propval_u32);
			mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop, &propval_u32, sizeof(propval_u32)));
			if (err < 0) {
				mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", prop, node);
				return -1;
			}

			sprintf(node, "switch");
			sprintf(prop, "rgmii_tx_timing_delay");
			/*enable delay to GTXCLK for OUTD outputs when port is in RGMII mode*/
			propval_u32 = 1;
			nodeoffset = mv_fdt_find_node(fdt, node);
			if (nodeoffset < 0) {
				mv_fdt_dprintf("Lack of '%s' node in device tree\n", node);
				return -1;
			}

			/* Setup rgmii_tx_timing_delay in DT.
			This value is HEX number, not a string, and uses network byte order */
			propval_u32 = htonl(propval_u32);
			mv_fdt_modify(fdt, err, fdt_setprop(fdt, nodeoffset, prop, &propval_u32, sizeof(propval_u32)));
			if (err < 0) {
				mv_fdt_dprintf("Modifying '%s' in '%s' node failed\n", prop, node);
				return -1;
			}
		}
	}

	return 0;
}
#endif
#endif

