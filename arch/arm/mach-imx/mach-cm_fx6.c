/*
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/module.h>
#include <linux/of_platform.h>

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include "common.h"
#include "hardware.h"
#include <linux/string.h>


extern unsigned int system_rev;
static unsigned int _system_rev;

#define ANADIG_DIGPROG		0x260
#define ANADIG_DIGPROG_IMX6SL	0x280

extern unsigned int __mxc_cpu_type;

static void revision_from_anatop(void)
{
	struct device_node *np;
	void __iomem *anatop_base;
	u32 cpu_type;
	u16 offset = ANADIG_DIGPROG;
	u32 fsl_system_rev = 0;

	np = of_find_compatible_node(NULL, NULL, "fsl,imx6q-anatop");
	anatop_base = of_iomap(np, 0);
	WARN_ON(!anatop_base);
	if (of_device_is_compatible(np, "fsl,imx6sl-anatop"))
		offset = ANADIG_DIGPROG_IMX6SL;
	cpu_type = readl_relaxed(anatop_base + offset);
	iounmap(anatop_base);

	/* Chip Silicon ID */
	fsl_system_rev = ((cpu_type >> 16) & 0xFF) << 12;
	/* Chip silicon major revision */
	fsl_system_rev |= ((cpu_type >> 8) & 0xFF) << 4;
	fsl_system_rev += 0x10;
	/* Chip silicon minor revision */
	fsl_system_rev |= cpu_type & 0xFF;

	/*
	 * Move the CompuLab board revision to a different variable,
	 * so we can use it anytime it is needed.
	 * Put the Freescale silicon revision information to the place where
	 * the userspace video libraries expect it to be.
	 */
	system_rev = fsl_system_rev;
}

static int cm_fx6_init(void)
{
	struct device_node *np;

	np = of_find_compatible_node(NULL, NULL, "compulab,cm-fx6");

	if (!np)
		return -EINVAL;

	_system_rev = system_rev;
	revision_from_anatop();

	return 0;
}

static void __exit cm_fx6_exit(void)
{
	system_rev = _system_rev;
}
module_init(cm_fx6_init);
module_exit(cm_fx6_exit);

MODULE_AUTHOR("CompuLab, Ltd.");
MODULE_DESCRIPTION("CompuLab CM-FX6 machine driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:cm-fx6");
