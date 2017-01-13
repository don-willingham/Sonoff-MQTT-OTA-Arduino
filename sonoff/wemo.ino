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

Wemo::Wemo(ESP8266WebServer *server, byte device, String friendlyname, String serial, String uuid)
{
  _server = server;
  _device = device;
  _friendlyname = friendlyname;
  _serial = serial;
  _uuid = uuid;

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

}

void Wemo::handleUPnPevent()
{
  addLog_P(LOG_LEVEL_DEBUG, PSTR("HTTP: Handle WeMo X basic event"));

  String request = _server->arg(0);
  if(request.indexOf("State>1</Binary") > 0) do_cmnd_power(_device, 1);
  if(request.indexOf("State>0</Binary") > 0) do_cmnd_power(_device, 0);
  _server->send(200, "text/plain", "");
}

void Wemo::handleUPnPservice()
{
  addLog_P(LOG_LEVEL_DEBUG, PSTR("HTTP: Handle WeMo event service"));

  String eventservice_xml = FPSTR(WEMO_EVENTSERVICE_XML);
  _server->send(200, "text/plain", eventservice_xml);
}


void Wemo::handleUPnPsetup()
{
  addLog_P(LOG_LEVEL_DEBUG, PSTR("HTTP: Handle WeMo setup"));

  String setup_xml = FPSTR(WEMO_SETUP_XML);
  setup_xml.replace("{x1}", _friendlyname);
  setup_xml.replace("{x2}", _uuid);
  setup_xml.replace("{x3}", _serial);
  _server->send(200, "text/xml", setup_xml);
}

#endif // USE_WEMO_EMULATION
