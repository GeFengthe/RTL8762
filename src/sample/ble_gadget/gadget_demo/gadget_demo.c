/*
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 * PROPRIETARY/CONFIDENTIAL.  USE IS SUBJECT TO LICENSE TERMS.
 */

#include <stdio.h>

#include "AMAManager.h"
#include "AMAFirmwareClient.h"
#include "gadget_test_app.h"
#include <mcu_vendor\ble_ama.h>

#define AMA_LOCAL_DEVICE_NAME   "AMA_Gadget"
#define AMA_LOCAL_DEVICE_SERIAL "1234567890"

/*
    Section: AMAManager
*/

static void AMAManager_DeviceStateChanged(AMAClient client, AMAStatus status, AMADeviceInfo *info,
                                          AMADeviceState state)
{
    if (state == AMA_DEVICE_STATE_AVAILABLE)
    {
        AMAManager_GetInterfaces()->ConnectDevice(client, info->device);
    }
}

static void AMAManager_DataReceived(AMAClient client, AMAData *data)
{
    gadget_received_data(data);
}

void gadget_send_data(AMAData *data)
{
    AMAManager_GetInterfaces()->SendData(0, data, NULL);
}

static AMAManagerCallbacks ama_cbs =
{
    NULL,
    AMAManager_DeviceStateChanged,
    NULL,
    NULL,
    AMAManager_DataReceived,
    NULL,
    NULL,
};

/*
    Section: AMAFirmwareClient
*/

static void AMAFirmwareClient_UpdateComponent(AMAFirmwareUpdate *update)
{
    if (update->offset + update->length == update->size)
    {
        AMAFirmwareClient_GetInterfaces()->ConfirmComponentUpdate(true);
    }
}

static void AMAFirmwareClient_CommitFirmware(AMAFirmwareInformation *info,
                                             AMAFirmwareComponent *components, uint8_t count, bool restart)
{
    AMAFirmwareClient_GetInterfaces()->ConfirmFirmwareCommit(true);
}

static AMAFirmwareClientCallbacks fw_cbs =
{
    NULL,
    AMAFirmwareClient_UpdateComponent,
    AMAFirmwareClient_CommitFirmware,
};

#include "trace.h"
extern void vStartGadgetDemo(void)
{

    // AMA Manager
    AMAManager_GetInterfaces()->Init(&ama_cbs);
    AMASystemInformation info;
    strncpy(info.name, AMA_LOCAL_DEVICE_NAME, AMA_NAME_LENGTH);
    strncpy(info.serial_number, AMA_LOCAL_DEVICE_SERIAL, AMA_SERIAL_NUMBER_LENGTH);
    AMAManager_GetInterfaces()->UpdateSystemInformation(&info);

    // Supports: AVS + OTA
    AMAManager_GetInterfaces()->UpdateFeaturesAndAttributes(3, 0);
    AMAManager_GetInterfaces()->RegisterClient(NULL);

    // AMA Firmware Client
    AMAFirmwareClient_GetInterfaces()->Register(&fw_cbs);

    AMAFirmwareInformation fw_info;
    fw_info.version = 9999;
    strncpy(fw_info.version_name, "default_g_name", AMA_FIRMWARE_NAME_LENGTH);
    strncpy(fw_info.locale, "locale", AMA_FIRMWARE_LOCALE_LENGTH);
    AMAFirmwareClient_GetInterfaces()->UpdateFirmwareInformation(&fw_info);

    AMAFirmwareComponent component;
    strncpy(component.name, "component", AMA_FIRMWARE_NAME_LENGTH);
    component.version = 1;
    AMAFirmwareClient_GetInterfaces()->PublishComponent(&component);

    // Transport
    bleAMATransport();
}
