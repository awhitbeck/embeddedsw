###############################################################################
# Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
#
###############################################################################
##############################################################################
#
# Modification History
#
# Ver   Who  Date     Changes
# ----- ---- -------- -----------------------------------------------
# 1.00a sdm  11/22/11 Created
#
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
    xdefine_zynq_include_file $drv_handle "xparameters.h" "XWdtPs" "NUM_INSTANCES" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_WDT_CLK_FREQ_HZ"

    xdefine_zynq_config_file $drv_handle "xwdtps_g.c" "XWdtPs" "DEVICE_ID" "C_S_AXI_BASEADDR"

    xdefine_zynq_canonical_xpars $drv_handle "xparameters.h" "XWdtPs" "DEVICE_ID" "C_S_AXI_BASEADDR" "C_S_AXI_HIGHADDR" "C_WDT_CLK_FREQ_HZ"

}
