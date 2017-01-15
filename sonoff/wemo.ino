/*
Copyright (c) 2017 Theo Arends.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef USE_WEMO_EMULATION
#include "wemo.h"

Wemo::Wemo(ESP8266WebServer *server, byte device)
{
  _server = server;
  _device = device;

  /* Use of Lambda functions inspired by https://github.com/kakopappa/arduino-esp8266-alexa-wemo-switch */
  _server->on("/upnp/control/basicevent1", HTTP_POST, [&]() {
    handleUPnPevent();
  });

  _server->on("/eventservice.xml", [&]() {
    handleUPnPservice();
  });

  _server->on("/setup.xml", [&]() {
    handleUPnPsetup();
  });

  _server->onNotFound([&]() {
    handleNotFound2(_server);
  });
}

void Wemo::handleUPnPevent()
{
  char log[LOGSZ];
  snprintf_P(log, sizeof(log), PSTR("HTTP: Handle WeMo basic event %d"), _device);
  addLog(LOG_LEVEL_DEBUG, log);

  String request = _server->arg(0);
  if(request.indexOf("State>1</Binary") > 0) do_cmnd_power(_device, 1);
  if(request.indexOf("State>0</Binary") > 0) do_cmnd_power(_device, 0);
  _server->send(200, "text/plain", "");
}

void Wemo::handleUPnPservice()
{
  char log[LOGSZ];
  snprintf_P(log, sizeof(log), PSTR("HTTP: Handle WeMo event service %d"), _device);
  addLog(LOG_LEVEL_DEBUG, log);

  String eventservice_xml = FPSTR(WEMO_EVENTSERVICE_XML);
  _server->send(200, "text/plain", eventservice_xml);
}


void Wemo::handleUPnPsetup()
{
  char log[LOGSZ];
  snprintf_P(log, sizeof(log), PSTR("HTTP: Handle WeMo setup %d"), _device);
  addLog(LOG_LEVEL_DEBUG, log);

  String setup_xml = FPSTR(WEMO_SETUP_XML);
  setup_xml.replace("{x1}", sysCfg.friendlyname[_device-1]);
  setup_xml.replace("{x2}", getUuid());
  setup_xml.replace("{x3}", getSerial());
  _server->send(200, "text/xml", setup_xml);
}

String Wemo::getSerial()
{
  char serial[18];
  // 201612K prefix; 32-bit chip id encoded as 8 character hex string; then device number 
  snprintf_P(serial, sizeof(serial), PSTR("201612K%08x%d"), ESP.getChipId(), (int)_device);
  return String(serial);
}

String Wemo::getUuid()
{
  char uuid[29];
  snprintf_P(uuid, sizeof(uuid), PSTR("Socket-1_0-%s"), getSerial().c_str());
  return String(uuid);
}

void Wemo::respondToMSearch()
{
  char message[TOPSZ], portstr[3], log[LOGSZ];
  const char WEMO_MSEARCH[] =
    "HTTP/1.1 200 OK\r\n"
    "CACHE-CONTROL: max-age=86400\r\n"
    "DATE: Fri, 15 Apr 2016 04:56:29 GMT\r\n"
    "EXT:\r\n"
    "LOCATION: http://{r1}:{r2}/setup.xml\r\n"
    "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
    "01-NLS: b9200ebb-736d-4b93-bf03-835149d13983\r\n"
    "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
    "ST: urn:Belkin:device:**\r\n"
    "USN: uuid:{r3}::urn:Belkin:device:**\r\n"
    "X-User-Agent: redsonic\r\n"
    "\r\n";
  String response = FPSTR(WEMO_MSEARCH);
  response.replace("{r1}", WiFi.localIP().toString());
  snprintf_P(portstr, sizeof(portstr), PSTR("%d"), 79+_device);
  response.replace("{r2}", portstr);
  response.replace("{r3}", getUuid());
  if (portUDP.beginPacket(portUDP.remoteIP(), portUDP.remotePort())) {
    portUDP.write(response.c_str());
    portUDP.endPacket();
    snprintf_P(message, sizeof(message), PSTR("Response sent"));
  } else {
    snprintf_P(message, sizeof(message), PSTR("Failed to send response"));
  }
  snprintf_P(log, sizeof(log), PSTR("UPnP: %s to %s:%d"),
    message, portUDP.remoteIP().toString().c_str(), portUDP.remotePort());
  addLog(LOG_LEVEL_DEBUG, log);
}

#endif // USE_WEMO_EMULATION
