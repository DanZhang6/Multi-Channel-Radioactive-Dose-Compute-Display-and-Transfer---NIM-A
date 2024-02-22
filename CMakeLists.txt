cmake_minimum_required(VERSION 3.26)
project(Multi_Channel_Radioactive_Dose_Compute_Display_and_Transfer___NIM_A C)

set(CMAKE_C_STANDARD 11)

include_directories("C:/Keil_v5/C51/INC")

add_executable(Multi_Channel_Radioactive_Dose_Compute_Display_and_Transfer___NIM_A
        8253count.c
        at24c16.c
        collateral.c
        config.h
        lcd.c
        main.c
        Masterdisplay.c
        measure.c
        reset.c
        serial.c
        Serial1Dip.c
        setinput.c
        setpara.c
        settime.c
        STC15F2K60S2.h
        time0.c)
