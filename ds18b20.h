/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DS18B20_H_
#define DS18B20_H_

#include "driver/gpio.h"

#define SEARCH_ROM 0XF0
#define READ_ROM 0x33
#define MATCH_ROM 0x55
#define SKIP_ROM 0xCC
#define ALARM_SEARCH 0xEC
#define CONVERT_T 0x44
#define WRITE_SCRATCHPAD 0x4E
#define READ_SCRATCHPAD 0xBE
#define COPY_SCRATCHPAD 0x48
#define RECALL_E2 0xB8
#define READ_POWER_SUPPLY 0xB4


typedef enum {
    DS18B20_9BIT = 0x1F,
    DS18B20_10BIT = 0x3F,
    DS18B20_11BIT = 0x5F,
    DS18B20_12BIT = 0x7F
} DS18B20_Resolution_t;

typedef struct {
    gpio_num_t gpio_num;
    bool init;
    DS18B20_Resolution_t resolution;
} DS18B20_Info_t;

void ds18b20_send(gpio_num_t gpio_num, char bit);
char ds18b20_read(gpio_num_t gpio_num);
void ds18b20_send_byte(gpio_num_t gpio_num, char data);
unsigned char ds18b20_read_byte(gpio_num_t gpio_num);
unsigned char ds18b20_RST_PULSE(gpio_num_t gpio_num);
float ds18b20_get_temp(DS18B20_Info_t *info);
void ds18b20_init(DS18B20_Info_t *info);

#endif
