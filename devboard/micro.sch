EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:MITEVT_mcontrollers
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 3
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L LPC11C14 U201
U 1 1 5830CB42
P 5700 3600
F 0 "U201" H 4550 5450 60  0000 C CNN
F 1 "LPC11C14" H 6500 1800 60  0000 C CNN
F 2 "" H 5850 2400 60  0000 C CNN
F 3 "" H 5850 2400 60  0000 C CNN
	1    5700 3600
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR01
U 1 1 5830CC2F
P 5400 1050
F 0 "#PWR01" H 5400 900 50  0001 C CNN
F 1 "+3.3V" H 5400 1190 50  0000 C CNN
F 2 "" H 5400 1050 50  0000 C CNN
F 3 "" H 5400 1050 50  0000 C CNN
	1    5400 1050
	1    0    0    -1  
$EndComp
$Comp
L C C202
U 1 1 5830CC56
P 5900 1350
F 0 "C202" H 5925 1450 50  0000 L CNN
F 1 "0.1u" H 5925 1250 50  0000 L CNN
F 2 "" H 5938 1200 50  0000 C CNN
F 3 "" H 5900 1350 50  0000 C CNN
	1    5900 1350
	1    0    0    -1  
$EndComp
$Comp
L C C203
U 1 1 5830CC83
P 6250 1350
F 0 "C203" H 6275 1450 50  0000 L CNN
F 1 "0.1u" H 6275 1250 50  0000 L CNN
F 2 "" H 6288 1200 50  0000 C CNN
F 3 "" H 6250 1350 50  0000 C CNN
	1    6250 1350
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR02
U 1 1 5830CCBB
P 6250 1600
F 0 "#PWR02" H 6250 1350 50  0001 C CNN
F 1 "GND" H 6250 1450 50  0000 C CNN
F 2 "" H 6250 1600 50  0000 C CNN
F 3 "" H 6250 1600 50  0000 C CNN
	1    6250 1600
	1    0    0    -1  
$EndComp
Text Label 3900 2300 0    60   ~ 0
RST
$Comp
L C C201
U 1 1 5830CDE7
P 2650 2650
F 0 "C201" H 2675 2750 50  0000 L CNN
F 1 "0.1u" H 2675 2550 50  0000 L CNN
F 2 "" H 2688 2500 50  0000 C CNN
F 3 "" H 2650 2650 50  0000 C CNN
	1    2650 2650
	1    0    0    -1  
$EndComp
$Comp
L R R201
U 1 1 5830CE18
P 2650 2150
F 0 "R201" V 2730 2150 50  0000 C CNN
F 1 "10k" V 2650 2150 50  0000 C CNN
F 2 "" V 2580 2150 50  0000 C CNN
F 3 "" H 2650 2150 50  0000 C CNN
	1    2650 2150
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR03
U 1 1 5830CE40
P 2650 1850
F 0 "#PWR03" H 2650 1700 50  0001 C CNN
F 1 "+3.3V" H 2650 1990 50  0000 C CNN
F 2 "" H 2650 1850 50  0000 C CNN
F 3 "" H 2650 1850 50  0000 C CNN
	1    2650 1850
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR04
U 1 1 5830CE63
P 2650 2950
F 0 "#PWR04" H 2650 2700 50  0001 C CNN
F 1 "GND" H 2650 2800 50  0000 C CNN
F 2 "" H 2650 2950 50  0000 C CNN
F 3 "" H 2650 2950 50  0000 C CNN
	1    2650 2950
	1    0    0    -1  
$EndComp
Text HLabel 2300 2350 0    60   Input ~ 0
RST
Text Label 2400 2350 0    60   ~ 0
RST
$Comp
L Crystal Y201
U 1 1 5830CF6F
P 3950 2700
F 0 "Y201" H 3950 2850 50  0000 C CNN
F 1 "732-TX325-24F20G-AC3" H 3950 2550 50  0001 L CNN
F 2 "" H 3950 2700 50  0000 C CNN
F 3 "" H 3950 2700 50  0000 C CNN
	1    3950 2700
	1    0    0    -1  
$EndComp
$Comp
L TEST_1P W201
U 1 1 5830E8FC
P 1350 800
F 0 "W201" V 1350 1100 50  0000 C CNN
F 1 "SMD_TP" V 1450 850 50  0001 C CNN
F 2 "" H 1550 800 50  0000 C CNN
F 3 "" H 1550 800 50  0000 C CNN
	1    1350 800 
	0    -1   -1   0   
$EndComp
Text Label 1400 800  0    60   ~ 0
PRGM
$Comp
L TEST_1P W202
U 1 1 5830EA82
P 1350 900
F 0 "W202" V 1350 1200 50  0000 C CNN
F 1 "SMD_TP" V 1450 950 50  0001 C CNN
F 2 "" H 1550 900 50  0000 C CNN
F 3 "" H 1550 900 50  0000 C CNN
	1    1350 900 
	0    -1   -1   0   
$EndComp
Text Label 1400 900  0    60   ~ 0
RST
Text Label 3900 2200 0    60   ~ 0
PRGM
$Comp
L C C?
U 1 1 5830EF55
P 3750 2950
F 0 "C?" H 3775 3050 50  0000 L CNN
F 1 "18p" H 3775 2850 50  0000 L CNN
F 2 "" H 3788 2800 50  0000 C CNN
F 3 "" H 3750 2950 50  0000 C CNN
	1    3750 2950
	1    0    0    -1  
$EndComp
$Comp
L C C?
U 1 1 5830EF8E
P 4150 2950
F 0 "C?" H 4175 3050 50  0000 L CNN
F 1 "18p" H 4175 2850 50  0000 L CNN
F 2 "" H 4188 2800 50  0000 C CNN
F 3 "" H 4150 2950 50  0000 C CNN
	1    4150 2950
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 5830F1F2
P 3750 3200
F 0 "#PWR05" H 3750 2950 50  0001 C CNN
F 1 "GND" H 3750 3050 50  0000 C CNN
F 2 "" H 3750 3200 50  0000 C CNN
F 3 "" H 3750 3200 50  0000 C CNN
	1    3750 3200
	1    0    0    -1  
$EndComp
Text HLabel 3800 4700 0    60   Input ~ 0
CAN_RXD
Text HLabel 3800 4800 0    60   Output ~ 0
CAN_TXD
Text Label 3850 4700 0    60   ~ 0
CAN_RXD
Text Label 3850 4800 0    60   ~ 0
CAN_TXD
$Comp
L TEST_1P W?
U 1 1 5830F57E
P 1350 1000
F 0 "W?" V 1350 1300 50  0000 C CNN
F 1 "SMD_TP" V 1450 1050 50  0001 C CNN
F 2 "" H 1550 1000 50  0000 C CNN
F 3 "" H 1550 1000 50  0000 C CNN
	1    1350 1000
	0    -1   -1   0   
$EndComp
$Comp
L TEST_1P W?
U 1 1 5830F5AD
P 1350 1100
F 0 "W?" V 1350 1400 50  0000 C CNN
F 1 "SMD_TP" V 1450 1150 50  0001 C CNN
F 2 "" H 1550 1100 50  0000 C CNN
F 3 "" H 1550 1100 50  0000 C CNN
	1    1350 1100
	0    -1   -1   0   
$EndComp
Text Label 1400 1000 0    60   ~ 0
CAN_RXD
Text Label 1400 1100 0    60   ~ 0
CAN_TXD
Text Label 7000 2300 0    60   ~ 0
UART_RX
Text Label 7000 2400 0    60   ~ 0
UART_TX
$Comp
L TEST_1P W?
U 1 1 5830FB29
P 1350 1200
F 0 "W?" V 1350 1500 50  0000 C CNN
F 1 "SMD_TP" V 1450 1250 50  0001 C CNN
F 2 "" H 1550 1200 50  0000 C CNN
F 3 "" H 1550 1200 50  0000 C CNN
	1    1350 1200
	0    -1   -1   0   
$EndComp
$Comp
L TEST_1P W?
U 1 1 5830FB5E
P 1350 1300
F 0 "W?" V 1350 1600 50  0000 C CNN
F 1 "SMD_TP" V 1450 1350 50  0001 C CNN
F 2 "" H 1550 1300 50  0000 C CNN
F 3 "" H 1550 1300 50  0000 C CNN
	1    1350 1300
	0    -1   -1   0   
$EndComp
Text Label 1400 1200 0    60   ~ 0
UART_RX
Text Label 1400 1300 0    60   ~ 0
UART_TX
$Comp
L C C?
U 1 1 5830FDF9
P 1700 2650
F 0 "C?" H 1725 2750 50  0000 L CNN
F 1 "0.1u" H 1725 2550 50  0000 L CNN
F 2 "" H 1738 2500 50  0000 C CNN
F 3 "" H 1700 2650 50  0000 C CNN
	1    1700 2650
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 5830FDFF
P 1700 2150
F 0 "R?" V 1780 2150 50  0000 C CNN
F 1 "10k" V 1700 2150 50  0000 C CNN
F 2 "" V 1630 2150 50  0000 C CNN
F 3 "" H 1700 2150 50  0000 C CNN
	1    1700 2150
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR06
U 1 1 5830FE05
P 1700 1850
F 0 "#PWR06" H 1700 1700 50  0001 C CNN
F 1 "+3.3V" H 1700 1990 50  0000 C CNN
F 2 "" H 1700 1850 50  0000 C CNN
F 3 "" H 1700 1850 50  0000 C CNN
	1    1700 1850
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR07
U 1 1 5830FE0B
P 1700 2950
F 0 "#PWR07" H 1700 2700 50  0001 C CNN
F 1 "GND" H 1700 2800 50  0000 C CNN
F 2 "" H 1700 2950 50  0000 C CNN
F 3 "" H 1700 2950 50  0000 C CNN
	1    1700 2950
	1    0    0    -1  
$EndComp
Text HLabel 1350 2350 0    60   Input ~ 0
PRGM
Text Label 1400 2350 0    60   ~ 0
PRGM
$Comp
L GND #PWR08
U 1 1 583101BA
P 5600 5850
F 0 "#PWR08" H 5600 5600 50  0001 C CNN
F 1 "GND" H 5600 5700 50  0000 C CNN
F 2 "" H 5600 5850 50  0000 C CNN
F 3 "" H 5600 5850 50  0000 C CNN
	1    5600 5850
	1    0    0    -1  
$EndComp
Text Label 7050 2550 0    60   ~ 0
MISO0
Text Label 7050 2650 0    60   ~ 0
MOSI0
Text Label 7050 2750 0    60   ~ 0
SCK0
Text Label 7050 2850 0    60   ~ 0
SSEL0
Text Label 7100 3250 0    60   ~ 0
MISO1
Text Label 7100 3350 0    60   ~ 0
MOSI1
Text Label 7100 3450 0    60   ~ 0
SCK1
Text Label 7100 3550 0    60   ~ 0
SSEL1
Text Label 7100 3750 0    60   ~ 0
SCL
Text Label 7100 3850 0    60   ~ 0
SDA
$Comp
L R R?
U 1 1 58362A52
P 7800 3850
F 0 "R?" V 7880 3850 50  0000 C CNN
F 1 "0" V 7800 3850 50  0000 C CNN
F 2 "" V 7730 3850 50  0000 C CNN
F 3 "" H 7800 3850 50  0000 C CNN
	1    7800 3850
	1    0    0    -1  
$EndComp
$Comp
L R R?
U 1 1 58362A9C
P 7800 4400
F 0 "R?" V 7880 4400 50  0000 C CNN
F 1 "0" V 7800 4400 50  0000 C CNN
F 2 "" V 7730 4400 50  0000 C CNN
F 3 "" H 7800 4400 50  0000 C CNN
F 4 "NOSTUFF" V 7800 4400 60  0001 C CNN "Option"
	1    7800 4400
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR09
U 1 1 58362B43
P 7800 3600
F 0 "#PWR09" H 7800 3450 50  0001 C CNN
F 1 "+3.3V" H 7800 3740 50  0000 C CNN
F 2 "" H 7800 3600 50  0000 C CNN
F 3 "" H 7800 3600 50  0000 C CNN
	1    7800 3600
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR010
U 1 1 58362BF9
P 7800 4650
F 0 "#PWR010" H 7800 4400 50  0001 C CNN
F 1 "GND" H 7800 4500 50  0000 C CNN
F 2 "" H 7800 4650 50  0000 C CNN
F 3 "" H 7800 4650 50  0000 C CNN
	1    7800 4650
	1    0    0    -1  
$EndComp
Text Label 3900 3800 0    60   ~ 0
WAKEUP
Text Label 3900 3900 0    60   ~ 0
AD6
Text Label 3900 4000 0    60   ~ 0
AD7
Text HLabel 3750 3800 0    60   Input ~ 0
WAKEUP
Text HLabel 3750 3900 0    60   Input ~ 0
AD6
Text HLabel 3750 4000 0    60   Input ~ 0
AD7
Text HLabel 7350 2300 2    60   Input ~ 0
UART_RX
Text HLabel 7350 2400 2    60   Output ~ 0
UART_TX
Text HLabel 7350 2550 2    60   BiDi ~ 0
MISO0
Text HLabel 7350 2650 2    60   BiDi ~ 0
MOSI0
Text HLabel 7350 2750 2    60   BiDi ~ 0
SCK0
Text HLabel 7350 2850 2    60   BiDi ~ 0
SSEL0
Text HLabel 7350 3250 2    60   BiDi ~ 0
MISO1
Text HLabel 7350 3350 2    60   BiDi ~ 0
MOSI1
Text HLabel 7350 3450 2    60   BiDi ~ 0
SCK1
Text HLabel 7350 3550 2    60   BiDi ~ 0
SSEL1
Text HLabel 7350 3750 2    60   BiDi ~ 0
SCL
Text HLabel 7350 3850 2    60   BiDi ~ 0
SDA
$Comp
L 7805 U?
U 1 1 584C5D96
P 2900 850
F 0 "U?" H 3050 654 50  0000 C CNN
F 1 "7805" H 2900 1050 50  0000 C CNN
F 2 "" H 2900 850 50  0000 C CNN
F 3 "" H 2900 850 50  0000 C CNN
	1    2900 850 
	1    0    0    -1  
$EndComp
$Comp
L +3.3V #PWR011
U 1 1 584C5E93
P 3300 800
F 0 "#PWR011" H 3300 650 50  0001 C CNN
F 1 "+3.3V" H 3300 940 50  0000 C CNN
F 2 "" H 3300 800 50  0000 C CNN
F 3 "" H 3300 800 50  0000 C CNN
	1    3300 800 
	0    1    1    0   
$EndComp
$Comp
L +5V #PWR012
U 1 1 584C5ECD
P 2500 800
F 0 "#PWR012" H 2500 650 50  0001 C CNN
F 1 "+5V" H 2500 940 50  0000 C CNN
F 2 "" H 2500 800 50  0000 C CNN
F 3 "" H 2500 800 50  0000 C CNN
	1    2500 800 
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR013
U 1 1 584C5F07
P 2900 1100
F 0 "#PWR013" H 2900 850 50  0001 C CNN
F 1 "GND" H 2900 950 50  0000 C CNN
F 2 "" H 2900 1100 50  0000 C CNN
F 3 "" H 2900 1100 50  0000 C CNN
	1    2900 1100
	1    0    0    -1  
$EndComp
$Comp
L LED D?
U 1 1 584C6AFB
P 2950 5450
F 0 "D?" H 2950 5550 50  0000 C CNN
F 1 "LED" H 2950 5350 50  0000 C CNN
F 2 "" H 2950 5450 50  0000 C CNN
F 3 "" H 2950 5450 50  0000 C CNN
	1    2950 5450
	1    0    0    -1  
$EndComp
$Comp
L LED D?
U 1 1 584C6C6E
P 2950 5750
F 0 "D?" H 2950 5850 50  0000 C CNN
F 1 "LED" H 2950 5650 50  0000 C CNN
F 2 "" H 2950 5750 50  0000 C CNN
F 3 "" H 2950 5750 50  0000 C CNN
	1    2950 5750
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR014
U 1 1 584C6FE5
P 2750 5800
F 0 "#PWR014" H 2750 5550 50  0001 C CNN
F 1 "GND" H 2750 5650 50  0000 C CNN
F 2 "" H 2750 5800 50  0000 C CNN
F 3 "" H 2750 5800 50  0000 C CNN
	1    2750 5800
	1    0    0    -1  
$EndComp
Text Label 7350 4300 2    60   ~ 0
PIO1_5
Text Label 7350 4400 2    60   ~ 0
PIO1_8
Text Label 7350 4500 2    60   ~ 0
PIO2_6
Text Label 7350 4600 2    60   ~ 0
PIO2_7
Text Label 7350 4700 2    60   ~ 0
PIO2_8
Text Label 7350 4800 2    60   ~ 0
PIO2_10
Text Label 7350 4900 2    60   ~ 0
PIO3_0
Text Label 7350 5000 2    60   ~ 0
PIO3_1
Text Label 7350 5100 2    60   ~ 0
PIO3_2
Text Label 7350 5200 2    60   ~ 0
PIO3_3
Text Label 7350 3050 2    60   ~ 0
PIO0_10
Text Label 3850 4500 0    60   ~ 0
PIO1_9
Text Label 3850 4600 0    60   ~ 0
PIO2_4
Wire Wire Line
	5400 1050 5400 1600
Wire Wire Line
	5600 1400 5600 1600
Wire Wire Line
	5600 1400 5400 1400
Connection ~ 5400 1400
Wire Wire Line
	6250 1500 6250 1600
Wire Wire Line
	6250 1550 5900 1550
Wire Wire Line
	5900 1550 5900 1500
Connection ~ 6250 1550
Wire Wire Line
	6250 1100 6250 1200
Wire Wire Line
	5400 1100 6250 1100
Connection ~ 5400 1100
Wire Wire Line
	5900 1200 5900 1100
Connection ~ 5900 1100
Wire Wire Line
	3900 2300 4300 2300
Wire Wire Line
	2650 2950 2650 2800
Wire Wire Line
	2650 2300 2650 2500
Wire Wire Line
	2650 2000 2650 1850
Wire Wire Line
	2650 2350 2300 2350
Connection ~ 2650 2350
Wire Wire Line
	1350 800  1750 800 
Wire Wire Line
	1350 900  1750 900 
Wire Wire Line
	3900 2200 4300 2200
Wire Wire Line
	4300 2400 3750 2400
Wire Wire Line
	3750 2400 3750 2800
Wire Wire Line
	3750 2700 3800 2700
Wire Wire Line
	4100 2700 4150 2700
Wire Wire Line
	4150 2500 4150 2800
Wire Wire Line
	4150 2500 4300 2500
Connection ~ 4150 2700
Connection ~ 3750 2700
Wire Wire Line
	4150 3100 4150 3150
Wire Wire Line
	4150 3150 3750 3150
Wire Wire Line
	3750 3100 3750 3200
Connection ~ 3750 3150
Wire Wire Line
	4300 4700 3800 4700
Wire Wire Line
	4300 4800 3800 4800
Wire Wire Line
	1350 1000 1750 1000
Wire Wire Line
	1350 1100 1750 1100
Wire Wire Line
	6950 2300 7350 2300
Wire Wire Line
	6950 2400 7350 2400
Wire Wire Line
	1350 1200 1750 1200
Wire Wire Line
	1750 1300 1350 1300
Wire Wire Line
	1700 2950 1700 2800
Wire Wire Line
	1700 2300 1700 2500
Wire Wire Line
	1700 2000 1700 1850
Wire Wire Line
	1700 2350 1350 2350
Connection ~ 1700 2350
Wire Wire Line
	5500 5550 5500 5750
Wire Wire Line
	5500 5750 5700 5750
Wire Wire Line
	5700 5750 5700 5550
Wire Wire Line
	5600 5850 5600 5750
Connection ~ 5600 5750
Wire Wire Line
	6950 2550 7350 2550
Wire Wire Line
	6950 2650 7350 2650
Wire Wire Line
	6950 2750 7350 2750
Wire Wire Line
	6950 2850 7350 2850
Wire Wire Line
	6950 3250 7350 3250
Wire Wire Line
	6950 3350 7350 3350
Wire Wire Line
	6950 3450 7350 3450
Wire Wire Line
	6950 3550 7350 3550
Wire Wire Line
	6950 3750 7350 3750
Wire Wire Line
	6950 3850 7350 3850
Wire Wire Line
	4300 3800 3750 3800
Wire Wire Line
	6950 4100 7800 4100
Wire Wire Line
	7800 4650 7800 4550
Wire Wire Line
	7800 4000 7800 4250
Connection ~ 7800 4100
Wire Wire Line
	7800 3700 7800 3600
Wire Wire Line
	4300 3900 3750 3900
Wire Wire Line
	4300 4000 3750 4000
Wire Wire Line
	6950 4800 7350 4800
Wire Wire Line
	6950 4900 7350 4900
Wire Wire Line
	6950 5000 7350 5000
Wire Wire Line
	6950 5100 7350 5100
Wire Wire Line
	6950 5200 7350 5200
Wire Wire Line
	6950 4700 7350 4700
Wire Wire Line
	6950 4600 7350 4600
Wire Wire Line
	6950 4500 7350 4500
Wire Wire Line
	6950 4400 7350 4400
Wire Wire Line
	6950 4300 7350 4300
Wire Wire Line
	6950 3050 7350 3050
Wire Wire Line
	4300 4500 3850 4500
Wire Wire Line
	4300 4600 3850 4600
Wire Wire Line
	4300 4900 3850 4900
Wire Wire Line
	4300 5000 3850 5000
Wire Wire Line
	4300 5100 3850 5100
Wire Wire Line
	4300 5200 3850 5200
Text Label 3850 4900 0    60   ~ 0
PIO2_5
Text Label 3850 5000 0    60   ~ 0
PIO2_9
Text Label 3850 5100 0    60   ~ 0
PIO0_6
Text Label 3850 5200 0    60   ~ 0
PIO0_7
Wire Wire Line
	3150 5450 3450 5450
Text Label 3450 5450 2    60   ~ 0
PIO2_5
Wire Wire Line
	3150 5750 3450 5750
Text Label 3450 5750 2    60   ~ 0
PIO2_9
Wire Wire Line
	2750 5450 2750 5800
Connection ~ 2750 5750
$Comp
L CONN_01X12 P?
U 1 1 584C8D02
P 9400 1650
F 0 "P?" H 9400 2300 50  0000 C CNN
F 1 "CONN_01X12" V 9500 1650 50  0000 C CNN
F 2 "" H 9400 1650 50  0000 C CNN
F 3 "" H 9400 1650 50  0000 C CNN
	1    9400 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 1100 9200 1100
Wire Wire Line
	8900 1200 9200 1200
Wire Wire Line
	8900 1300 9200 1300
Wire Wire Line
	8900 1400 9200 1400
Wire Wire Line
	8900 1600 9200 1600
Wire Wire Line
	8900 1700 9200 1700
Wire Wire Line
	8900 1900 9200 1900
Wire Wire Line
	8900 2000 9200 2000
Wire Wire Line
	8900 2100 9200 2100
Wire Wire Line
	8900 2200 9200 2200
Text Label 8900 1100 2    60   ~ 0
PIO2_6
$Comp
L CONN_01X12 P?
U 1 1 584CA260
P 9400 3000
F 0 "P?" H 9400 3650 50  0000 C CNN
F 1 "CONN_01X12" V 9500 3000 50  0000 C CNN
F 2 "" H 9400 3000 50  0000 C CNN
F 3 "" H 9400 3000 50  0000 C CNN
	1    9400 3000
	1    0    0    -1  
$EndComp
Wire Wire Line
	8900 2450 9200 2450
Wire Wire Line
	8900 2550 9200 2550
Wire Wire Line
	8900 2650 9200 2650
Wire Wire Line
	8900 2750 9200 2750
Wire Wire Line
	8600 2850 9200 2850
Wire Wire Line
	8900 2950 9200 2950
Wire Wire Line
	8900 3050 9200 3050
Wire Wire Line
	8900 3150 9200 3150
Wire Wire Line
	8900 3250 9200 3250
Wire Wire Line
	8900 3350 9200 3350
Wire Wire Line
	8900 3450 9200 3450
Wire Wire Line
	8900 3550 9200 3550
$Comp
L CONN_01X12 P?
U 1 1 584CA2D8
P 10650 1650
F 0 "P?" H 10650 2300 50  0000 C CNN
F 1 "CONN_01X12" V 10750 1650 50  0000 C CNN
F 2 "" H 10650 1650 50  0000 C CNN
F 3 "" H 10650 1650 50  0000 C CNN
	1    10650 1650
	1    0    0    -1  
$EndComp
Wire Wire Line
	10150 1100 10450 1100
Wire Wire Line
	10150 1200 10450 1200
Wire Wire Line
	10150 1300 10450 1300
Wire Wire Line
	10150 1400 10450 1400
Wire Wire Line
	10000 1500 10450 1500
Wire Wire Line
	10150 1600 10450 1600
Wire Wire Line
	10150 1700 10450 1700
Wire Wire Line
	9850 1800 10450 1800
Wire Wire Line
	10150 1900 10450 1900
Wire Wire Line
	10150 2000 10450 2000
Wire Wire Line
	10150 2100 10450 2100
Wire Wire Line
	10150 2200 10450 2200
$Comp
L CONN_01X12 P?
U 1 1 584CA35B
P 10650 3000
F 0 "P?" H 10650 3650 50  0000 C CNN
F 1 "CONN_01X12" V 10750 3000 50  0000 C CNN
F 2 "" H 10650 3000 50  0000 C CNN
F 3 "" H 10650 3000 50  0000 C CNN
	1    10650 3000
	1    0    0    -1  
$EndComp
Wire Wire Line
	10150 2450 10450 2450
Wire Wire Line
	10150 2550 10450 2550
Wire Wire Line
	10150 2650 10450 2650
Wire Wire Line
	10150 2750 10450 2750
Wire Wire Line
	10150 2850 10450 2850
Wire Wire Line
	10150 2950 10450 2950
Wire Wire Line
	10150 3050 10450 3050
Wire Wire Line
	10150 3150 10450 3150
Wire Wire Line
	10150 3250 10450 3250
Wire Wire Line
	10150 3350 10450 3350
Wire Wire Line
	10150 3450 10450 3450
Wire Wire Line
	10150 3550 10450 3550
$Comp
L GND #PWR015
U 1 1 584CAB33
P 8750 1500
F 0 "#PWR015" H 8750 1250 50  0001 C CNN
F 1 "GND" H 8750 1350 50  0000 C CNN
F 2 "" H 8750 1500 50  0000 C CNN
F 3 "" H 8750 1500 50  0000 C CNN
	1    8750 1500
	0    1    1    0   
$EndComp
NoConn ~ 8900 1600
NoConn ~ 8900 1700
Text Label 8900 2200 2    60   ~ 0
PIO2_8
Text Label 8900 2100 2    60   ~ 0
PIO2_7
Text Label 8900 1900 2    60   ~ 0
PIO1_8
$Comp
L +3.3V #PWR016
U 1 1 584CB348
P 8600 1800
F 0 "#PWR016" H 8600 1650 50  0001 C CNN
F 1 "+3.3V" H 8600 1940 50  0000 C CNN
F 2 "" H 8600 1800 50  0000 C CNN
F 3 "" H 8600 1800 50  0000 C CNN
	1    8600 1800
	0    -1   -1   0   
$EndComp
Text Label 7300 4100 0    60   ~ 0
PIO0_3
Text Label 8900 2550 2    60   ~ 0
PIO0_3
$Comp
L +3.3V #PWR017
U 1 1 584CB65D
P 8600 2850
F 0 "#PWR017" H 8600 2700 50  0001 C CNN
F 1 "+3.3V" H 8600 2990 50  0000 C CNN
F 2 "" H 8600 2850 50  0000 C CNN
F 3 "" H 8600 2850 50  0000 C CNN
	1    8600 2850
	0    -1   -1   0   
$EndComp
Text Label 8900 2950 2    60   ~ 0
PIO2_4
Text Label 8900 3250 2    60   ~ 0
PIO2_5
Text Label 8900 3350 2    60   ~ 0
PIO0_6
Text Label 8900 3450 2    60   ~ 0
PIO0_7
Text Label 8900 3550 2    60   ~ 0
PIO2_9
Text Label 10150 2450 2    60   ~ 0
PIO2_10
Text Label 10150 2850 2    60   ~ 0
PIO0_10
Text Label 10150 2950 2    60   ~ 0
AD6
Text Label 10150 3050 2    60   ~ 0
SCK0
Wire Wire Line
	4300 3300 4200 3300
Wire Wire Line
	4300 3400 4200 3400
Wire Wire Line
	4300 3500 4200 3500
Wire Wire Line
	4300 3600 4200 3600
Wire Wire Line
	4300 3700 4200 3700
Text Label 4200 3300 2    60   ~ 0
AD0
Text Label 4200 3400 2    60   ~ 0
AD1
Text Label 4200 3500 2    60   ~ 0
AD2
Text Label 4200 3600 2    60   ~ 0
AD3
Text Label 4200 3700 2    60   ~ 0
AD4
Text Label 10150 3150 2    60   ~ 0
AD0
Text Label 10150 3250 2    60   ~ 0
AD1
Text Label 10150 3350 2    60   ~ 0
AD2
Text Label 10150 3450 2    60   ~ 0
AD3
Text Label 10150 3550 2    60   ~ 0
PIO3_0
Text Label 10150 1100 2    60   ~ 0
PIO3_1
Text Label 10150 1200 2    60   ~ 0
MOSI1
Text Label 10150 1300 2    60   ~ 0
AD4
Text Label 10150 1400 2    60   ~ 0
WAKEUP
$Comp
L GND #PWR018
U 1 1 584CC5D4
P 10000 1500
F 0 "#PWR018" H 10000 1250 50  0001 C CNN
F 1 "GND" H 10000 1350 50  0000 C CNN
F 2 "" H 10000 1500 50  0000 C CNN
F 3 "" H 10000 1500 50  0000 C CNN
	1    10000 1500
	0    1    1    0   
$EndComp
Text Label 8900 2000 2    60   ~ 0
SSEL0
Text Label 10150 1600 2    60   ~ 0
AD7
Text Label 10150 1700 2    60   ~ 0
PIO3_2
$Comp
L +3.3V #PWR019
U 1 1 584CC898
P 9850 1800
F 0 "#PWR019" H 9850 1650 50  0001 C CNN
F 1 "+3.3V" H 9850 1940 50  0000 C CNN
F 2 "" H 9850 1800 50  0000 C CNN
F 3 "" H 9850 1800 50  0000 C CNN
	1    9850 1800
	0    -1   -1   0   
$EndComp
Text Label 8900 1400 2    60   ~ 0
PRGM
Text Label 8900 1300 2    60   ~ 0
RST
Text Label 8900 1200 2    60   ~ 0
SSEL1
Text Label 8900 2450 2    60   ~ 0
SCK1
Text Label 8900 2650 2    60   ~ 0
SCL
Text Label 8900 2750 2    60   ~ 0
SDA
Text Label 8900 3050 2    60   ~ 0
CAN_RXD
Text Label 8900 3150 2    60   ~ 0
CAN_TXD
Wire Wire Line
	8750 1500 9200 1500
Wire Wire Line
	8600 1800 9200 1800
Text Label 10150 2550 2    60   ~ 0
MISO1
Text Label 10150 2650 2    60   ~ 0
MISO0
Text Label 10150 2750 2    60   ~ 0
MOSI1
Text Label 10150 1900 2    60   ~ 0
PIO1_5
Text Label 10150 2200 2    60   ~ 0
PIO3_3
Text Label 10150 2100 2    60   ~ 0
UART_TX
Text Label 10150 2000 2    60   ~ 0
UART_RX
$EndSCHEMATC
