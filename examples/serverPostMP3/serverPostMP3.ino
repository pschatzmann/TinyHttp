/**
 * A simple http server which plays an mp3 file that is posted.
 * Test e.g. with curl -F "file=@test.mp3;type=audio/mp3" -X POST http://192.168.1.33/play
 *
 */

#include "AudioTools.h"
#include "AudioCodecs/CodecMP3Helix.h"
#include "AudioLibs/AudioKit.h"
#include "HttpServer.h"

AudioKitStream i2s; // final output of decoded stream
EncodedAudioStream dec(&i2s, new MP3DecoderHelix()); // Decoding stream
StreamCopy copier;

const char *ssid = "SSID";
const char *password = "PASSWORD";
WiFiServer wifi;
HttpServer server(wifi);

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);  
  HttpLogger.begin(Serial, tinyhttp::Debug);

  dec.begin();
  i2s.begin();

  auto playMP3 = [](HttpServer *server_ptr, const char *requestPath,
                   HttpRequestHandlerLine *hl) {
    LOGI("Playing mp3");
    copier.begin(dec, server_ptr->client());
    copier.copyAll();
    server_ptr->replyOK();
  };

  server.on("/play", T_POST, playMP3);
  server.begin(80, ssid, password);
  HttpLogger.log(tinyhttp::Info, "server was started...");
}

void loop() { server.doLoop(); }