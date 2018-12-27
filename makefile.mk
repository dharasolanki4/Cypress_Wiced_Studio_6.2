#
# Copyright 2018, Cypress Semiconductor Corporation or a subsidiary of Cypress Semiconductor 
#  Corporation. All rights reserved. This software, including source code, documentation and  related 
# materials ("Software"), is owned by Cypress Semiconductor  Corporation or one of its 
#  subsidiaries ("Cypress") and is protected by and subject to worldwide patent protection  
# (United States and foreign), United States copyright laws and international treaty provisions. 
# Therefore, you may use this Software only as provided in the license agreement accompanying the 
# software package from which you obtained this Software ("EULA"). If no EULA applies, Cypress 
# hereby grants you a personal, nonexclusive, non-transferable license to  copy, modify, and 
# compile the Software source code solely for use in connection with Cypress's  integrated circuit 
# products. Any reproduction, modification, translation, compilation,  or representation of this 
# Software except as specified above is prohibited without the express written permission of 
# Cypress. Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO  WARRANTY OF ANY KIND, EXPRESS 
# OR IMPLIED, INCLUDING,  BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY 
# AND FITNESS FOR A PARTICULAR PURPOSE. Cypress reserves the right to make changes to 
# the Software without notice. Cypress does not assume any liability arising out of the application 
# or use of the Software or any product or circuit  described in the Software. Cypress does 
# not authorize its products for use in any products where a malfunction or failure of the 
# Cypress product may reasonably be expected to result  in significant property damage, injury 
# or death ("High Risk Product"). By including Cypress's product in a High Risk Product, the 
#  manufacturer of such system or application assumes  all risk of such use and in doing so agrees 
# to indemnify Cypress against all liability.
#

########################################################################
# Add Application sources here.
########################################################################

APP_SRC += hello_sensor.c
APP_SRC += wiced_bt_cfg.c

APP_SRC += tile_features.c
APP_SRC += tile_service.c
APP_SRC += tile_storage.c
APP_SRC += tile_led.c

APP_SRC += tile_lib/src/tileHash.c
APP_SRC += tile_lib/src/toa/queue.c
APP_SRC += tile_lib/src/toa/song.c
APP_SRC += tile_lib/src/toa/tdg.c
APP_SRC += tile_lib/src/toa/tdi.c
APP_SRC += tile_lib/src/toa/tdt.c
APP_SRC += tile_lib/src/toa/test.c
APP_SRC += tile_lib/src/toa/tka.c
APP_SRC += tile_lib/src/toa/tmd.c
APP_SRC += tile_lib/src/toa/toa.c
APP_SRC += tile_lib/src/toa/trm.c

APP_SRC += tile_lib/src/toa/tofu.c
APP_SRC += tile_lib/src/toa/tmf.c
APP_SRC += tile_lib/src/toa/tcu.c
APP_SRC += tile_lib/src/toa/tileTime.c
APP_SRC += tile_lib/src/toa/tpc.c

APP_SRC += tile_lib/src/drivers/tile_button.c
APP_SRC += tile_lib/src/drivers/tile_gap.c
APP_SRC += tile_lib/src/drivers/tile_random.c
APP_SRC += tile_lib/src/drivers/tile_timer.c
APP_SRC += tile_lib/src/crypto/hmac_sha256_ogay.c
APP_SRC += tile_lib/src/crypto/sha256_ogay.c

C_FLAGS += -DWICED_BT_TRACE_ENABLE
C_FLAGS += -DENABLE_HCI_TRACE

#BT_DEVICE_ADDRESS=20706A201234

########################################################################
################ DO NOT MODIFY FILE BELOW THIS LINE ####################
########################################################################
