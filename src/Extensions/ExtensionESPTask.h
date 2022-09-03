#ifndef __EXTENSION_H__
#define __EXTENSION_H__

#include "FreeRTOS.h" /* Must come first. */

namespace tinyhttp {

// forward declaration
class HttpServer;

HttpServer *actualHttpServer;

/**
 * @brief Extension which which supports RTOS Tasks. We start a task for the HttpServer so that we do not
 * need to call doLoop() in the loop processing of the sketch any more.
 * 
 */

class ExtensionESPTask {

    public:
        ExtensionESPTask(){
            actualExtensionESPTask = this;
        }

        virtual void open(HttpServer *server) {
            actualHttpServer = server;

            // setup task method
            auto lambda = []() { 
                while(true){
                    if (actualHttpServer!=null){
                        actualHttpServer->doLoop();
                    }
                }
            };

            BaseType_t xReturned = xTaskCreate( 
                (TaskFunction_t) lambda,  /* global Task function. */
                "ExtensionESPTask",   /* String with name of task. */
                50000,            /* Stack size in bytes. */
                nullptr,          /* Parameter passed as input of the task */
                2,                /* Priority of the task. */
                &xHandle);        /* Task handle. */

            if( xReturned != pdPASS ){
                Serial.println("The dump task could not be created");
            }
        };

        virtual void doLoop() {};

        void stop(){
            if (xHandle!=0) {
                vTaskDelete( xHandle );
                xHandle = 0;
            }

        }
    protected:
        TaskHandle_t xHandle;

};

}

#endif // __EXTENSION_H__