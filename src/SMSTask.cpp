#include "SMSTask.h"
#include "GSMModule.h"
#include <Arduino.h>

void sendSMSTask(void *parameter) {
    String *message = (String *)parameter;

    // Send the SMS message
    sendSMS(*message);

    // Free the allocated memory for the message
    delete message;

    // Add a small delay to prevent issues right after memory is deallocated
    vTaskDelay(50 / portTICK_PERIOD_MS); 

    // Delete the task after completion
    vTaskDelete(NULL);
}
