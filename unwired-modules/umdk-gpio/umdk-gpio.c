/*
 * Copyright (C) 2016 Unwired Devices LLC [info@unwds.com]
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup
 * @ingroup
 * @brief
 * @{
 * @file
 * @brief
 * @author      Evgeny Ponomarev
 */

#ifdef __cplusplus
extern "C" {
#endif

/* define is autogenerated, do not change */
#undef _UMDK_MID_
#define _UMDK_MID_ UNWDS_GPIO_MODULE_ID

/* define is autogenerated, do not change */
#undef _UMDK_NAME_
#define _UMDK_NAME_ "gpio"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "periph/gpio.h"

#include "board.h"
#include "unwds-common.h"

#include "umdk-gpio.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static bool set(int num, bool one)
{
    gpio_t gpio = unwds_gpio_pin(num);
    
    if (gpio == 0) {
        return false;
    }

    gpio_init(gpio, GPIO_OUT);
    
    if (one) {
        gpio_set(gpio);
    }
    else {
        gpio_clear(gpio);
    }

    return true;
}

static int get(int num)
{
    gpio_t gpio = unwds_gpio_pin(num);
    return gpio_read(gpio);
}

static int get_status(int num)
{
    gpio_t gpio = unwds_gpio_pin(num);    
    return gpio_get_status(gpio);
}

static bool toggle(int num)
{
    gpio_t gpio = unwds_gpio_pin(num);

    if (gpio == 0) {
        return false;
    }

    gpio_toggle(gpio);

    return true;
}

int umdk_gpio_shell_cmd(int argc, char **argv) {
    if (argc == 1) {
        puts (_UMDK_NAME_ " get <N> - get GPIO value");
        puts (_UMDK_NAME_ " set <N> <0|1> - set GPIO value");
        return 0;
    }
    
    char *cmd = argv[1];
    
    if (strcmp(cmd, "get") == 0) {
        char *pin = argv[2];
        printf("[umdk-" _UMDK_NAME_ "], pin %d is %d\n", atoi(pin), get(atoi(pin)));
    }
    
    if (strcmp(cmd, "set") == 0) {
        char *pin = argv[2];
        char *val = argv[3];
        set(atoi(pin), atoi(val));
        printf("[umdk-" _UMDK_NAME_ "], pin %d set to %d\n", atoi(pin), atoi(val));
    }
    
    return 1;
}

void umdk_gpio_init(uint32_t *non_gpio_pin_map, uwnds_cb_t *event_callback)
{
    (void) non_gpio_pin_map;
    (void) event_callback;
    
    unwds_add_shell_command(_UMDK_NAME_, "type '" _UMDK_NAME_ "' for commands list", umdk_gpio_shell_cmd);
}

static inline void do_reply(module_data_t *reply, umdk_gpio_reply_t reply_code)
{
    reply->length = UMDK_GPIO_DATA_LEN + 1;
    reply->data[0] = _UMDK_MID_;
    reply->data[1] = reply_code;
}

static bool check_pin(module_data_t *reply, int pin)
{
    /* Is pin is occupied by other module */
    if (unwds_is_pin_occupied(pin)) {
        do_reply(reply, UMDK_GPIO_REPLY_ERR_PIN);
        return false;
    }

    /* Gpio pin not in range */
    if (pin < 0 || unwds_gpio_pin(pin) == 0) {
        do_reply(reply, UMDK_GPIO_REPLY_ERR_PIN);
        return false;
    }

    return true;
}

static bool gpio_cmd(module_data_t *cmd, module_data_t *reply, bool with_reply)
{
    if (cmd->length != UMDK_GPIO_DATA_LEN) {
        if (with_reply) {
            do_reply(reply, UMDK_GPIO_REPLY_ERR_FORMAT);
        }
        return false;
    }

    uint8_t value = cmd->data[0];
    uint8_t pin = value & UMDK_GPIO_PIN_MASK;
    umdk_gpio_action_t act = (value & UMDK_GPIO_ACT_MASK) >> UMDK_GPIO_ACT_SHIFT;

    if ((pin != 0) && (!check_pin(reply, pin))) {
        DEBUG("umdk-gpio: pin check failed, pin %d\n", (int)pin);
        return false;
    }
    
    DEBUG("umdk-gpio: pin %d, act %d\n", (int)pin, (int)act);

    switch (act) {
        case UMDK_GPIO_GET_ALL:
            DEBUG("umdk-gpio: GPIO GET ALL command\n");
            reply->length = 2;
            reply->data[0] = _UMDK_MID_;
            reply->data[1] = UMDK_GPIO_REPLY_OK_ALL;

            int i = 0;
            for (i = 0; i < unwds_gpio_pins_total(); i++) {
                pin = unwds_gpio_pin(i);
                uint8_t combined = 0;
                
                /* bit 0: pin value, bit 1: 0 for IN, 1 for OUT, bit 3: AF/AIN, bit 4: reserved */
                /* 0b111 means pin not used, 0b110 — AIN, 0b100 — AF */
                
                if (pin == 0) {
                    combined = 0b111;
                } else {
                    int status = get_status(i);
                    if (status > 0x01) {
                        pin = 0;
                    } else {
                        pin = get(i) & 0x1;
                    }
                    combined = (status << 1) | pin;
                }
                DEBUG("Pin: %d, status: %d, value: %d\n", i, combined >> 1, combined & 0x1);
                reply->data[2 + i/2] |= combined << (4 * (i%2));
            }
            reply->length += (i+1)/2;
            break;
        case UMDK_GPIO_GET:
            DEBUG("umdk-gpio: GPIO GET command\n");
            if (with_reply) {
                switch (get(pin)) {
                    case 1:
                        do_reply(reply, UMDK_GPIO_REPLY_OK_1);
                        break;
                    case 0:
                        do_reply(reply, UMDK_GPIO_REPLY_OK_0);
                        break;
                    case -1:
                        do_reply(reply, UMDK_GPIO_REPLY_OK_AINAF);
                        break;
                }
            }
            break;
            
        case UMDK_GPIO_SET_0:
        case UMDK_GPIO_SET_1:
            DEBUG("umdk-gpio: GPIO SET command\n");
            if (set(pin, act == UMDK_GPIO_SET_1)) {
                if (with_reply) {
                    do_reply(reply, UMDK_GPIO_REPLY_OK);
                }
            } else {
                if (with_reply) {
                    do_reply(reply, UMDK_GPIO_REPLY_ERR_PIN);
                }
            }
            break;

        case UMDK_GPIO_TOGGLE:
            DEBUG("umdk-gpio: GPIO TOGGLE command\n");
            if (toggle(pin)) {
                if (with_reply) {
                    do_reply(reply, UMDK_GPIO_REPLY_OK);
                }
            } else {
                if (with_reply) {
                    do_reply(reply, UMDK_GPIO_REPLY_ERR_PIN);
                }
            }

            break;
    }

    return true;
}

bool umdk_gpio_broadcast(module_data_t *cmd, module_data_t *reply) {
	return gpio_cmd(cmd, reply, false); /* Don't reply on broadcasts */
}

bool umdk_gpio_cmd(module_data_t *cmd, module_data_t *reply)
{
    return gpio_cmd(cmd, reply, true);
}

#ifdef __cplusplus
}
#endif
