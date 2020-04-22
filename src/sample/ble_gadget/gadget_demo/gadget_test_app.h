/*
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * PROPRIETARY/CONFIDENTIAL.  USE IS SUBJECT TO LICENSE TERMS.
 */

#include "AMACommon.h"

// Interface into gadget test code from main demo applicatoin
void gadget_received_data(AMAData *data);

// Callback from gadget test code to main demo application
void gadget_send_data(AMAData *data);

// Called from the application to set the device name used in DiscoveryResponse.
void gadget_set_name(char *deviceName);

void vStartGadgetDemo(void);
