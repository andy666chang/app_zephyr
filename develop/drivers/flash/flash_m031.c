
#define DT_DRV_COMPAT nuvoton_m031_flash_controller

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>

#include <NuMicro.h>

#define SOC_NV_FLASH_NODE	DT_INST(0, soc_nv_flash)
#define SOC_NV_FLASH_SIZE	DT_REG_SIZE(SOC_NV_FLASH_NODE)
#define SOC_NV_FLASH_ADDR	DT_REG_ADDR(SOC_NV_FLASH_NODE)
#define SOC_NV_FLASH_PRG_SIZE	DT_PROP(SOC_NV_FLASH_NODE, write_block_size)
#define SOC_NV_FLASH_ERA_SIZE	DT_PROP(SOC_NV_FLASH_NODE, erase-block-size)

struct flash_m031_data {
	struct k_sem mutex;
};

static struct flash_m031_data flash_data;

static const struct flash_parameters flash_m031_parameters = {
	.write_block_size = SOC_NV_FLASH_PRG_SIZE,
	.erase_value = 0xff,
};

bool flash_m031_valid_range(off_t offset, uint32_t len, bool write)
{
	if ((offset > SOC_NV_FLASH_SIZE) ||
	    ((offset + len) > SOC_NV_FLASH_SIZE)) {
		return false;
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
    if (!flash_m031_valid_range(offset, len, true)) {
		return -EINVAL;
	}

	k_sem_take(&dev_data->mutex, K_FOREVER);

	/* Enable FMC ISP function */
    FMC_Open();

    // read
    for (uint32_t u32Addr = u32StartAddr; u32Addr < u32EndAddr; u32Addr += 4) {
        uint32_t u32Pattern u32data = FMC_Read(u32Addr);   /* Read a flash word from address u32Addr. */

        if (g_FMC_i32ErrCode != 0)
        {
            printf("FMC_Read address 0x%x failed!\n", u32Addr);
            return -ENOEXEC;
        }

        memcpy((void *)data, (void *)&u32Pattern, sizeof(uint32_t));

        (uint32_t *)data++;
    }

    /* Disable FMC ISP function */
    FMC_Close();

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

	k_sem_take(&dev_data->mutex, K_FOREVER);

	/* Enable FMC ISP function */
    FMC_Open();

    /* Fill flash range from u32StartAddr to u32EndAddr. */
    for (uint32_t u32Addr = offset; u32Addr < offset+len; u32Addr += 4) {
        uint32_t u32Pattern = *(uint32_t *)data;

        if (FMC_Write(u32Addr, u32Pattern) != 0)          /* Program flash */
        {
            printf("FMC_Write address 0x%x failed!\n", u32Addr);
            return -ENOEXEC;
        }

        (uint32_t *)data++;
    }

    /* Disable FMC ISP function */
    FMC_Close();

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

    if (!flash_m031_valid_range(offset, len, true)) {
		return -EINVAL;
	}

	k_sem_take(&data->mutex, K_FOREVER);

    /* Enable FMC ISP function */
    FMC_Open();

    // Erase
    for (size_t addr = offset; addr < offset+size; addr += SOC_NV_FLASH_ERA_SIZE) {
        FMC_Erase(addr);
    }

    /* Disable FMC ISP function */
    FMC_Close();

	k_sem_give(&data->mutex);

	return ret;
}


static const struct flash_parameters*
flash_m031_get_parameters(const struct device *dev)
{
	ARG_UNUSED(dev);

	return &flash_m031_parameters;
}

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

	return 0;
}

DEVICE_DT_INST_DEFINE(0, flash_m031_init, NULL,
		      &flash_data, NULL, POST_KERNEL,
		      CONFIG_FLASH_INIT_PRIORITY, &flash_m031_driver_api);
