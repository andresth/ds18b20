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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "ds18b20.h"


// Sends one bit to bus
void ds18b20_send(gpio_num_t gpio_num, char bit){
    gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_num, 0);

    // 6.5µs low for binary 1, 61.5µs low for a binary 0
    // measured with signal analyzer
    if(bit == 1) {
        ets_delay_us(6);
        gpio_set_level(gpio_num, 1);
        ets_delay_us(53);
    } else {
        ets_delay_us(61);
        gpio_set_level(gpio_num, 1);
    }

    // end of slot, 1µs recovery
    ets_delay_us(1);
}

// Reads one bit from bus
char ds18b20_read(gpio_num_t gpio_num) {
    // issue read time slot
    gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_num, 0);
    ets_delay_us(2);
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);

    // read data from bus
    unsigned char duration = 0;
    while (gpio_get_level(gpio_num) == 0) {
        ets_delay_us(1);
        duration++;
    }


    // wait for time slot to be 61µs long
    // different values for 0 and 1 because ets_delay_us waits a little bit to long
    if (duration < 15) {
        ets_delay_us(56 - duration);
        return 1;
    } else{
        ets_delay_us(51 - duration);
        return 0;
    }
}

// Sends one byte to bus
void ds18b20_send_byte(gpio_num_t gpio_num, char data){
    unsigned char x;
    for(char i=0; i < 8; i++) {
        x = data >> i;
        x &= 0x01;
        ds18b20_send(gpio_num, x);
    }
}

// Reads one byte from bus
unsigned char ds18b20_read_byte(gpio_num_t gpio_num){
    unsigned char data = 0;
    for (char i = 0; i < 8; i++) {
        if(ds18b20_read(gpio_num)) {
            data |= 0x01 << i;
        }
    }
    return(data);
}

// Sends reset pulse
unsigned char ds18b20_RST_PULSE(gpio_num_t gpio_num){
    unsigned char duration = 0;
    uint16_t slot_duration = 0;
    gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_num, 0);
    ets_delay_us(480);
    gpio_set_direction(gpio_num, GPIO_MODE_INPUT);
    while (slot_duration <= 500) {  // longer sampling on presence results in a quicker response
        if(gpio_get_level(gpio_num) == 0) {
            duration++;
        }
        ets_delay_us(1);
        slot_duration++;
    }

    if ((duration >= 60) && (duration <= 240)) {
        return 1;
    } else {
        return 0;
    }
}

// Returns temperature from sensor
float ds18b20_get_temp(DS18B20_Info_t *info) {
    if(info->init) {
        unsigned char check;
        char temp1=0, temp2=0;
        check = ds18b20_RST_PULSE(info->gpio_num);
        if(check == 1) {
            ds18b20_send_byte(info->gpio_num, SKIP_ROM);
            ds18b20_send_byte(info->gpio_num, CONVERT_T);

            while (ds18b20_read(info->gpio_num) <= 0) {}
            check = ds18b20_RST_PULSE(info->gpio_num);
            ds18b20_send_byte(info->gpio_num, SKIP_ROM);
            ds18b20_send_byte(info->gpio_num, READ_SCRATCHPAD);
            temp1 = ds18b20_read_byte(info->gpio_num);
            temp2 = ds18b20_read_byte(info->gpio_num);
            float temp = 0;
            temp = (float)(temp1 + (temp2 * 256)) / 16;
            return temp;
        }
        else {
            return 0;
        }
    } else {
        return 0;
    }
}

// Init sensor
void ds18b20_init(DS18B20_Info_t *info) {
    char alarm_h = 0, alarm_l = 0;
    gpio_pad_select_gpio(info->gpio_num);
    gpio_set_pull_mode(info->gpio_num, GPIO_PULLUP_ONLY);
    // wait for pin to reach high level
    ets_delay_us(50);
    if (ds18b20_RST_PULSE(info->gpio_num) == 1) {
        info->init = true;
    } else {
        info->init = false;
    }

    ds18b20_send_byte(info->gpio_num, SKIP_ROM);
    ds18b20_send_byte(info->gpio_num, READ_SCRATCHPAD);
    ds18b20_read_byte(info->gpio_num);
    ds18b20_read_byte(info->gpio_num);
    alarm_h = ds18b20_read_byte(info->gpio_num);
    alarm_l = ds18b20_read_byte(info->gpio_num);

    ds18b20_RST_PULSE(info->gpio_num);

    ds18b20_send_byte(info->gpio_num, SKIP_ROM);
    ds18b20_send_byte(info->gpio_num, WRITE_SCRATCHPAD);
    ds18b20_send_byte(info->gpio_num, alarm_h);
    ds18b20_send_byte(info->gpio_num, alarm_l);
    ds18b20_send_byte(info->gpio_num, info->resolution);
}
