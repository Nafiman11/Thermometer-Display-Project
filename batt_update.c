#include "batt.h"

int set_batt_from_ports(batt_t *batt){
    // Uses the two global variables (ports) BATT_VOLTAGE_PORT and
// BATT_STATUS_PORT to set the fields of the parameter 'batt'.  If
// BATT_VOLTAGE_PORT is negative, then battery has been wired wrong;
// no fields of 'batt' are changed and 1 is returned to indicate an
// error.  Otherwise, sets fields of batt based on reading the voltage
// value and converting to precent using the provided formula. Returns
// 0 on a successful execution with no errors. This function DOES NOT
// modify any global variables but may access global variables.
//
// CONSTRAINT: Avoids the use of the division operation as much as
// possible. Makes use of shift operations in place of division where
// possible.
//
// CONSTRAINT: Uses only integer operations. No floating point
// operations are used as the target machine does not have a FPU.
// 
// CONSTRAINT: Limit the complexity of code as much as possible. Do
// not use deeply nested conditional structures. Seek to make the code
// as short, and simple as possible. Code longer than 40 lines may be
// penalized for complexity.

    if (BATT_VOLTAGE_PORT < 0){
        return 1;
    }
    batt->mlvolts = BATT_VOLTAGE_PORT >> 1;
    if (((batt->mlvolts - 3000) >> 3) > 100) {
        batt->percent = 100;
    } 
    else if (((batt->mlvolts - 3000) >> 3) < 0){
        batt->percent = 0;
    } 
    else {
        batt->percent = ((batt->mlvolts - 3000) >> 3);
    }
    if(BATT_STATUS_PORT & (1 << 4)) {
         batt->mode = 1;
    }
    else {
        batt->mode = 2;
    }
    return 0;
}


int set_display_from_batt(batt_t batt, int *display){
    // Alters the bits of integer pointed to by 'display' to reflect the
// data in struct param 'batt'.  Does not assume any specific bit
// pattern stored at 'display' and completely resets all bits in it on
// successfully completing.  Selects either to show Volts (mode=1) or
// Percent (mode=2). If Volts are displayed, only displays 3 digits
// rounding the lowest digit up or down appropriate to the last digit.
// Calculates each digit to display changes bits at 'display' to show
// the volts/percent according to the pattern for each digit. Modifies
// additional bits to show a decimal place for volts and a 'V' or '%'
// indicator appropriate to the mode. In both modes, places bars in
// the level display as indicated by percentage cutoffs in provided
// diagrams. This function DOES NOT modify any global variables but
// may access global variables. Always returns 0.
// 
// CONSTRAINT: Limit the complexity of code as much as possible. Do
// not use deeply nested conditional structures. Seek to make the code
// as short, and simple as possible. Code longer than 65 lines may be
// penalized for complexity.
    int bitmasks [] = {0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110, 0b1101101,0b1111101,
    0b0000111, 0b1111111, 0b1101111};
    *display = 0;
    int percentage = batt.percent;
    int voltage = batt.mlvolts;
    int extradigit = voltage %10;
    int right;
    int middle;
    int left;
    if (extradigit >= 5) {
        right = ((voltage/10) % 10) +1;
    } else {
        right = (voltage/10) % 10;
    }
    if (batt.mode == 2){ // volatage mode
        middle = (((voltage /10)/10)%10);
        left = (((voltage /10) /10)/10);
        *display = *display | (0b110<<0);  
        *display = *display | (bitmasks[right]<<3);
        *display = *display | (bitmasks[middle]<<10);
        *display = *display | (bitmasks[left]<<17);
    } else if (batt.percent == 100 && batt.mode == 1){ // percentage mode
        right = percentage %10;
        middle = ((percentage /10)%10);
        left = (((percentage /10) /10)%10); 
        *display = *display | (0b001<<0); 
        *display = *display | (bitmasks[right]<<3);
        *display = *display | (bitmasks[middle]<<10);
        *display = *display | (bitmasks[left]<<17);
    } else if (batt.percent >= 10 && batt.percent <= 99 && batt.mode == 1){
        right = percentage %10;
        middle = percentage/10;
        *display = *display | (0b001<<0); 
        *display = *display | (bitmasks[right]<<3);
        *display = *display | (bitmasks[middle]<<10);
        *display = *display | (0b0000000<<17);
    } else {
        right = percentage %10;
        *display = *display | (0b001<<0); 
        *display = *display | (bitmasks[right]<<3);
        *display = *display | (0b0000000<<10);
        *display = *display | (0b0000000<<17);
    }
    if (batt.percent >=5 && batt.percent <= 29){*display = *display | 0b00001<<24;}
    if (batt.percent >=30 && batt.percent <= 49){*display = *display | 0b00011<<24;}
    if (batt.percent >=50 && batt.percent <= 69){*display = *display | 0b00111<<24;}
    if (batt.percent >=70 && batt.percent <= 89){*display = *display | 0b01111<<24;}
    if (batt.percent >=90 && batt.percent <= 100){*display = *display | 0b11111<<24;}
    return 0;
}


int batt_update(){
    // Called to update the battery meter display.  Makes use of
    // set_batt_from_ports() and set_display_from_batt() to access battery
    // voltage sensor then set the display. Checks these functions and if
    // they indicate an error, does NOT change the display.  If functions
    // succeed, modifies BATT_DISPLAY_PORT to show current battery level.
    // 
    // CONSTRAINT: Does not allocate any heap memory as malloc() is NOT
    // available on the target microcontroller.  Uses stack and global
    // memory only.
    batt_t battery;
    if (set_batt_from_ports(&battery) == 1){
        return 1;
    } else {
        set_display_from_batt(battery, &BATT_DISPLAY_PORT);
    }
    return 0;
}
