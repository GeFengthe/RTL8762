#include <gpio_handle.h>
#include <trace.h>
#include <string.h>
#include <app_msg.h>
#include <app_task.h>
#include <board.h>


void GPIO_Int_Handler(void)
{
    //T_IO_MSG bee_io_msg;
    GPIO_MaskINTConfig(GPIO_Int_Pin, ENABLE);
    APP_PRINT_INFO0("Enter GPIO Interrupt");
    uint8_t keystatus =  GPIO_ReadInputDataBit(GPIO_Int_Pin);

    if (keystatus == 0)
    {

        GPIO->INTPOLARITY |= GPIO_Int_Pin;
        //bee_io_msg.type = IO_MSG_TYPE_GPIO;
        allowedEnterDlps = false;
        //bee_io_msg.subtype = MSG_KEY_SELECT_PRESS;
        //app_send_msg_to_apptask(&bee_io_msg);
    }

    else
    {
        GPIO->INTPOLARITY &= ~GPIO_Int_Pin;
        //bee_io_msg.type = IO_MSG_TYPE_GPIO;
        allowedEnterDlps = true;
        //bee_io_msg.subtype = MSG_KEY_SELECT_RELEASE;
        //app_send_msg_to_apptask(&bee_io_msg);
    }

    GPIO_ClearINTPendingBit(GPIO_Int_Pin);
    GPIO_MaskINTConfig(GPIO_Int_Pin, DISABLE);
}
