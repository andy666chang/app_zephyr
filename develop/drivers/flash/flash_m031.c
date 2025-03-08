
#define DT_DRV_COMPAT nuvoton_m031_flash_controller

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>

#include <stdio.h>
#include <string.h>
#include <NuMicro.h>

#define APROM_FLASH_NODE	DT_NODELABEL(aprom)
#define APROM_FLASH_SIZE	DT_REG_SIZE(APROM_FLASH_NODE)
#define APROM_FLASH_ADDR	DT_REG_ADDR(APROM_FLASH_NODE)
#define APROM_FLASH_PRG_SIZE	DT_PROP(APROM_FLASH_NODE, write_block_size)
#define APROM_FLASH_ERA_SIZE	DT_PROP(APROM_FLASH_NODE, erase_block_size)

#define LDROM_FLASH_NODE	DT_NODELABEL(ldrom)
#define LDROM_FLASH_SIZE	DT_REG_SIZE(LDROM_FLASH_NODE)
#define LDROM_FLASH_ADDR	DT_REG_ADDR(LDROM_FLASH_NODE)
#define LDROM_FLASH_PRG_SIZE	DT_PROP(LDROM_FLASH_NODE, write_block_size)
#define LDROM_FLASH_ERA_SIZE	DT_PROP(LDROM_FLASH_NODE, erase_block_size)

struct flash_m031_data {
	struct k_sem mutex;
};

static struct flash_m031_data flash_data;

static const struct flash_parameters flash_m031_parameters = {
	.write_block_size = APROM_FLASH_PRG_SIZE,
	.erase_value = 0xff,
};

bool flash_m031_valid_range(off_t offset, uint32_t len, bool write)
{
	off_t start = offset, end = offset + end;

    // APROM valid
    if ((offset > APROM_FLASH_SIZE) || ((offset + len) > APROM_FLASH_SIZE)) {
        // LDROM valid
        if ((offset < LDROM_FLASH_ADDR) ||
            ((offset + len) > LDROM_FLASH_ADDR + LDROM_FLASH_SIZE)) {
			printf("Invalid range! offset: 0x%x, len: 0x%x\n", offset, len);
            return false;
        }
    }

    /* Check offset and len is word aligned. */
	if ((offset % sizeof(uint32_t)) ||
	    (len % sizeof(uint32_t))) {
	    return false;
	}

	return true;
}

static int flash_m031_read(const struct device *dev, off_t offset,
			   void *data, size_t len)
{
	int ret = 0;
	struct flash_m031_data *dev_data = dev->data;

    if (!flash_m031_valid_range(offset, len, true)) {
		return -EINVAL;
	}

	k_sem_take(&dev_data->mutex, K_FOREVER);

	/* Enable FMC ISP function */
	SYS_UnlockReg();
    FMC_Open();

    // read
    for (uint32_t u32Addr = offset; u32Addr < offset+len; u32Addr += 4) {
        uint32_t u32Pattern = 0;
		u32Pattern = FMC_Read(u32Addr);   /* Read a flash word from address u32Addr. */

        if (g_FMC_i32ErrCode != 0)
        {
            printf("FMC_Read address 0x%x failed!\n", u32Addr);
			FMC_Close();
			SYS_LockReg();
			k_sem_give(&dev_data->mutex);
            return -ENOEXEC;
        }

        memcpy((void *)data, (void *)&u32Pattern, sizeof(uint32_t));

        data += 4;
    }

    /* Disable FMC ISP function */
    FMC_Close();
	SYS_LockReg();
	k_sem_give(&dev_data->mutex);

	return 0;
}

static int flash_m031_write(const struct device *dev, off_t offset,
			    const void *data, size_t len)
{
	struct flash_m031_data *dev_data = dev->data;
	int ret = 0;

    if (len == 0U) {
		return 0;
	}

	if (!flash_m031_valid_range(offset, len, true)) {
		return -EINVAL;
	}

	ret = k_sem_take(&dev_data->mutex, K_SECONDS(1));
	if (ret) {
		printf("sem_take: %d\n", ret);
		return ret;
	}
	

	/* Enable FMC ISP function */
    SYS_UnlockReg();
    FMC_Open();
	if (offset>= LDROM_FLASH_ADDR)
		FMC_ENABLE_LD_UPDATE();
	

    /* Fill flash range from u32StartAddr to u32EndAddr. */
    for (uint32_t u32Addr = offset; u32Addr < offset+len; u32Addr += 4) {
        uint32_t u32Pattern = *(uint32_t *)data;

        if (FMC_Write(u32Addr, u32Pattern) != 0)          /* Program flash */
        {
            printf("FMC_Write address 0x%x failed!\n", u32Addr);
			FMC_Close();
			SYS_LockReg();
			k_sem_give(&dev_data->mutex);
            return -ENOEXEC;
        }

        data += 4;
    }

    /* Disable FMC ISP function */
	if (offset>= LDROM_FLASH_ADDR)
		FMC_DISABLE_LD_UPDATE();
    FMC_Close();
	SYS_LockReg();
	k_sem_give(&dev_data->mutex);

	return ret;
}

static int flash_m031_erase(const struct device *dev, off_t offset, size_t size)
{
	struct flash_m031_data *data = dev->data;
	int ret = 0;

	if (size == 0U) {
		return 0;
	}

    if (!flash_m031_valid_range(offset, size, true)) {
		return -EINVAL;
	}

	k_sem_take(&data->mutex, K_FOREVER);

    /* Enable FMC ISP function */
    SYS_UnlockReg();
    FMC_Open();
	if (offset>= LDROM_FLASH_ADDR)
		FMC_ENABLE_LD_UPDATE();

    // Erase
    for (size_t addr = offset; addr < offset+size; addr += APROM_FLASH_ERA_SIZE) {
        FMC_Erase(addr);
    }

    /* Disable FMC ISP function */
	if (offset>= LDROM_FLASH_ADDR)
		FMC_DISABLE_LD_UPDATE();
    FMC_Close();
	SYS_LockReg();
	k_sem_give(&data->mutex);

	return ret;
}


static const struct flash_parameters*
flash_m031_get_parameters(const struct device *dev)
{
	ARG_UNUSED(dev);

	return &flash_m031_parameters;
}

#ifdef CONFIG_FLASH_PAGE_LAYOUT
static const struct flash_pages_layout m031_fmc_layout[] = {
	{
	.pages_size = APROM_FLASH_ERA_SIZE,
	.pages_count = APROM_FLASH_SIZE / APROM_FLASH_ERA_SIZE,
	},
	{
	.pages_size = LDROM_FLASH_ERA_SIZE,
	.pages_count = LDROM_FLASH_SIZE / LDROM_FLASH_ERA_SIZE,
	},
	// ALL partition
	{
	.pages_size = LDROM_FLASH_ERA_SIZE,
	.pages_count = FMC_USER_CONFIG_2 / LDROM_FLASH_ERA_SIZE,
	}
};

void flash_m031_pages_layout(const struct device *dev,
			     const struct flash_pages_layout **layout,
			     size_t *layout_size)
{
	ARG_UNUSED(dev);

	*layout = m031_fmc_layout;
	*layout_size = ARRAY_SIZE(m031_fmc_layout);
}
#endif

static const struct flash_driver_api flash_m031_driver_api = {
	.read = flash_m031_read,
	.write = flash_m031_write,
	.erase = flash_m031_erase,
	.get_parameters = flash_m031_get_parameters,
#ifdef CONFIG_FLASH_PAGE_LAYOUT
	.page_layout = flash_m031_pages_layout,
#endif
};


static int flash_m031_init(const struct device *dev)
{
	struct flash_m031_data *data = dev->data;

	k_sem_init(&data->mutex, 1, 1);

	// k_sem_take(&data->mutex, K_FOREVER);
    // /* Enable FMC ISP function */
    // SYS_UnlockReg();
    // FMC_Open();

	// /* Read Data Flash base address */
    // uint32_t u32Data = FMC_ReadDataFlashBaseAddr();
    // printf("  Data Flash Base Address ............... [0x%08x]\n", u32Data);

	// /* Disable FMC ISP function */
    // FMC_Close();
	// SYS_LockReg();
	// k_sem_give(&data->mutex);

	return 0;
}

DEVICE_DT_INST_DEFINE(0, flash_m031_init, NULL,
		      &flash_data, NULL, POST_KERNEL,
		      CONFIG_FLASH_INIT_PRIORITY, &flash_m031_driver_api);
