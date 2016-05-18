#include <i2c.h>
#include <string.h>
#include "mraa_internal.h"
#include "mraa_internal_types.h"
#include "mraa/i2c.h"

static struct _i2c _internali2c[CONFIG_MRAA_I2C_COUNT];
static struct _i2c tmp_i2c_dev;


mraa_i2c_context
mraa_i2c_init(int bus)
{
    mraa_board_t* board = plat;
    if (board == NULL) {
        // syslog(LOG_ERR, "i2c: Platform Not Initialised");
        return NULL;
    }

    if (bus >= board->i2c_bus_count) {
        // syslog(LOG_ERR, "Above i2c bus count");
        return NULL;
    }

    if (board->i2c_bus[bus].bus_id == -1) {
        // syslog(LOG_ERR, "Invalid i2c bus, moving to default i2c bus");
        bus = board->def_i2c_bus;
    }
    if (!board->no_bus_mux) {
        int pos = board->i2c_bus[bus].sda;
        if (board->pins[pos].i2c.mux_total > 0) {
            if (mraa_setup_mux_mapped(board->pins[pos].i2c) != MRAA_SUCCESS) {
                // syslog(LOG_ERR, "i2c: Failed to set-up i2c sda multiplexer");
                return NULL;
            }
        }

        pos = board->i2c_bus[bus].scl;
        if (board->pins[pos].i2c.mux_total > 0) {
            if (mraa_setup_mux_mapped(board->pins[pos].i2c) != MRAA_SUCCESS) {
                // syslog(LOG_ERR, "i2c: Failed to set-up i2c scl multiplexer");
                return NULL;
            }
        }
    }

    mraa_i2c_context dev = &_internali2c[bus];
    mraa_i2c_context tmp = mraa_i2c_init_raw(plat->i2c_bus[bus].bus_id);
    if (tmp == NULL)
    	return NULL;
    memcpy(dev, tmp, sizeof(struct _i2c));
    printf("mraa_i2c_init(%d) done\n", bus);
    return dev;
}


mraa_i2c_context
mraa_i2c_init_raw(unsigned int bus)
{
	char device_name[8];
    mraa_i2c_context dev = &tmp_i2c_dev;
    sprintf(device_name, "I2C_%d", bus);
    dev->zdev = device_get_binding(device_name);
    if (dev->zdev == NULL) {
    	printf("Failed to get binding for %s\n", device_name);
    	return NULL;
    }
    dev->busnum = bus;
	dev->zcfg.raw = 0;
	dev->zcfg.bits.use_10_bit_addr = 0;
	dev->zcfg.bits.speed = I2C_SPEED_STANDARD;
	dev->zcfg.bits.is_master_device = 1;
	if (i2c_configure(dev->zdev, dev->zcfg.raw) != 0)
		return NULL;
    printf("mraa_i2c_init_raw(%d) done\n", bus);
    return dev;
}


mraa_result_t
mraa_i2c_frequency(mraa_i2c_context dev, mraa_i2c_mode_t mode)
{
    switch (mode) {
        case MRAA_I2C_STD: /**< up to 100Khz */
            dev->zcfg.bits.speed = I2C_SPEED_STANDARD;
            break;
        case MRAA_I2C_FAST: /**< up to 400Khz */
            dev->zcfg.bits.speed = I2C_SPEED_FAST;
            break;
        case MRAA_I2C_HIGH: /**< up to 3.4Mhz */
            dev->zcfg.bits.speed = I2C_SPEED_HIGH;
            break;
    }
	return i2c_configure(dev->zdev, dev->zcfg.raw) == 0 ? MRAA_SUCCESS : MRAA_ERROR_UNSPECIFIED;
}


mraa_result_t
mraa_i2c_address(mraa_i2c_context dev, uint8_t addr)
{
    dev->addr = (int)addr;
    return MRAA_SUCCESS;
}


int
mraa_i2c_read(mraa_i2c_context dev, uint8_t* data, int length)
{
	if (i2c_read(dev->zdev, data, length, dev->addr))
		return 0;
	else
		return length;
}

mraa_result_t
mraa_i2c_write(mraa_i2c_context dev, const uint8_t* data, int bytesToWrite)
{
	return i2c_write(dev->zdev, (uint8_t *)data, bytesToWrite, dev->addr) ? MRAA_SUCCESS : MRAA_ERROR_INVALID_HANDLE;
}


int
mraa_i2c_read_byte(mraa_i2c_context dev)
{
    uint8_t data;
    int bytes_read = mraa_i2c_read(dev, &data, 1);
    return bytes_read == 1 ? data : -1;
}

int
mraa_i2c_read_byte_data(mraa_i2c_context dev, uint8_t command)
{
    mraa_result_t status = mraa_i2c_write(dev, &command, 1);
    if (status != MRAA_SUCCESS)
    	return -1;
    uint8_t data;
    int bytes_read = mraa_i2c_read(dev, &data, 1);
    return (bytes_read == 1) ? data : -1;
}

int
mraa_i2c_read_word_data(mraa_i2c_context dev, uint8_t command)
{
    mraa_result_t status = mraa_i2c_write(dev, &command, 1);
    if (status != MRAA_SUCCESS)
    	return -1;
    int data;
    int bytes_read = mraa_i2c_read(dev, (uint8_t *)&data, 1);
    return (bytes_read == 2) ? data : -1;
}

int
mraa_i2c_read_bytes_data(mraa_i2c_context dev, uint8_t command, uint8_t* data, int length)
{
    int bytes_read = 0;
    int bytes_written = mraa_i2c_write(dev, &command, 1);
    if (bytes_written == 1)
       bytes_read = mraa_i2c_read(dev, data, length);
    return bytes_read;
}



mraa_result_t
mraa_i2c_write_byte(mraa_i2c_context dev, uint8_t data)
{
    mraa_result_t status = mraa_i2c_write(dev, &data, 1);
    return status;
}


mraa_result_t
mraa_i2c_write_byte_data(mraa_i2c_context dev, const uint8_t data, const uint8_t command)
{
    uint8_t buf[2];
    buf[0] = command;
    buf[1] = data;
    mraa_result_t status = mraa_i2c_write(dev, buf, 2);
    return status;
}

mraa_result_t
mraa_i2c_write_word_data(mraa_i2c_context dev, const uint16_t data, const uint8_t command)
{
    uint8_t buf[3];
    buf[0] = command;
    buf[1] = (uint8_t) data;
    buf[2] = (uint8_t)(data >> 8);
    mraa_result_t status = mraa_i2c_write(dev, buf, 3);
    return status;
}

mraa_result_t
mraa_i2c_stop(mraa_i2c_context dev)
{
    return MRAA_SUCCESS;
}
