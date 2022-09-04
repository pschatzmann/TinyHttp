#pragma once

#include "Arduino.h"
#include "FreeRTOS.h" /* Must come first. */

namespace tinyhttp {

// forward declaration
class HttpServer;

/**
 * @brief Extension which which supports RTOS Tasks. We start a task for the HttpServer so that we do not
 * need to call doLoop() in the loop processing of the sketch any more.
 * 
 */

class ExtensionESPTask : public Extension {

    public:
        ExtensionESPTask(int stackSize=50000, int priority=3, int core=1, int delayAfterProcess=100){
            task_priority = priority;
            stack_size = stackSize;
            core_no = core;
            delay_after_process = delayAfterProcess;
        }

        virtual void open(HttpServer *server) {
            Log.log(Info,"ExtensionESPTask", "open");

            BaseType_t xReturned = xTaskCreatePinnedToCore( 
                (TaskFunction_t) task,  /* global Task function. */
                "ExtensionESPTask",   /* String with name of task. */
                stack_size,            /* Stack size in bytes. */
                this,           /* Parameter passed as input of the task */
                task_priority,                /* Priority of the task. */
                &xHandle,
                core_no);        /* Task handle. */

            if( xReturned != pdPASS ){
                Log.log(Error, "The task could not be created");
            } else {
                Log.log(Info, "task created");
            }
        };

        virtual void doLoop() {};

        void stop(){
            if (xHandle!=0) {
                vTaskDelete( xHandle );
                xHandle = 0;
            }
        }

        HttpServer *server(){
            return p_server;
        }

        int getDelay() {
            return delay_after_process;
        }

    protected:
        TaskHandle_t xHandle = 0;
        int stack_size;
        int task_priority;
        int core_no;
        int delay_after_process;
        HttpServer *p_server;

        // Background Processing
        static void task(void* info){
            ExtensionESPTask *self = (ExtensionESPTask*)info;
            HttpServer *actualHttpServer = self->server();
            int delay_ms = self->getDelay();

            while(true){
                if (actualHttpServer!=nullptr && *actualHttpServer){
                    actualHttpServer->doLoop();
                    delay(delay_ms); // give others a chance to do some processing
                } else {
                    delay(1000);
                }                
            }            
        }

};

}

