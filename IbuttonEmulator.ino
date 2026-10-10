#include "OneWireHub.h"
#include "DS2401.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <stdarg.h>

const char* ap_ssid = "ibutton";
const char* ap_password = "ibutton52";

#define iButtonPort 2
#define buttonPin 5

#define BRUTE_KEY_DURATION_MS 1500

AsyncWebServer server(80);

const int KEY_MAX = 64;
int keyCount = 0;
byte keys[KEY_MAX][8];
String keyNames[KEY_MAX];

const byte defaultKeys[34][8] = {
  {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F},
  {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0x00},
  {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F, 0xFF, 0x00},
  {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x9B},
  {0x01, 0xA9, 0xE4, 0x3C, 0x09, 0x00, 0x00, 0xE6},
  {0x01, 0xBE, 0x40, 0x11, 0x5A, 0x36, 0x00, 0xE1},
  {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3D},
  {0x01, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x9B},
  {0x01, 0xFF, 0xFF, 0xFD, 0x48, 0x00, 0x78, 0xD1},
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x14},
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xA0},
  {0x01, 0xBE, 0x40, 0x11, 0x5A, 0x36, 0x00, 0xE1},
  {0x01, 0x53, 0xD4, 0xFE, 0x00, 0x00, 0x7E, 0x88},
  {0x01, 0x19, 0x90, 0x00, 0x00, 0x00, 0x00, 0xFF},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0x01, 0x19, 0x90, 0x00, 0x00, 0x00, 0x00, 0xFF},
  {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3D},
  {0x01, 0x95, 0xA1, 0xAE, 0x01, 0x00, 0x00, 0x8E},
  {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F},
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
  {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0x00},
  {0x01, 0xFF, 0xFF, 0xFD, 0x48, 0x00, 0x78, 0xD1},
  {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x2F, 0xFF, 0x00},
  {0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00},
  {0x01, 0x90, 0x0E, 0x1C, 0x14, 0x00, 0x00, 0xD0},
  {0x01, 0x90, 0xE4, 0x38, 0x11, 0x00, 0x00, 0xFC},
  {0x01, 0x91, 0xB8, 0x29, 0x12, 0x00, 0x00, 0x2C},
  {0x01, 0x00, 0x00, 0x00, 0x00, 0x90, 0x19, 0xFF},
  {0x01, 0xC9, 0x9F, 0x4D, 0x01, 0x00, 0x00, 0x3A},
  {0x01, 0x90, 0x19, 0x04, 0x64, 0x00, 0x00, 0x00},
  {0x01, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0x01, 0xCA, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0x01, 0xAB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

const char* const defaultKeyNames[34] = {
  "Standard Universal Key",
  "Universal key 1F",
  "Universal key 2F",
  "UK-1 Metakom 2003",
  "Eltis",
  "UK-2 Vizit",
  "UK-3 Cyfral",
  "uk-4",
  "uk-5",
  "Metakom 98%, some cyfral",
  "Metakom 95%",
  "Vizit 99%",
  "Metakom, Cyfral",
  "un3",
  "Universal Key 1 (00:00:00:00:00:00:00:00)",
  "Universal Key 2 (01:19:90:00:00:00:00:FF)",
  "Universal Key 3 (01:00:00:00:00:00:00:3D)",
  "Universal Key 4 (01:95:A1:AE:01:00:00:8E)",
  "Universal Key 5 (01:FF:FF:FF:FF:FF:FF:2F)",
  "Universal Key 6 (FF:FF:FF:FF:FF:FF:FF:FF)",
  "Universal Key 7 (01:FF:FF:FF:FF:1F:FF:00)",
  "Universal Key 8 (01:FF:FF:FD:48:00:78:D1)",
  "Universal Key 9 (01:FF:FF:FF:FF:2F:FF:00)",
  "Universal Key 10 (01:FF:FF:FF:FF:FF:FF:00)",
  "Universal Key 11 (01:90:0E:1C:14:00:00:D0)",
  "Universal Key 12 (01:90:E4:38:11:00:00:FC)",
  "Universal Key 13 (01:91:B8:29:12:00:00:2C)",
  "komynal_5 (01:00:00:00:00:90:19:FF)",
  "komfort (01:C9:9F:4D:01:00:00:3A)",
  "Metakom MK20-TM service",
  "Blank variant low (01:67)",
  "Blank variant high (01:80)",
  "Variant CA (01:CA:16)",
  "Variant AB (01:AB)"
};

int currentKeyIndex = 0;
bool keyChanged = false;

byte pendingKey[8] = {0};
bool hasPendingKey = false;
bool readMode = false;

bool bruteMode = false;
int bruteIndex = 0;
unsigned long bruteStartTime = 0;

DS2401* ds1990A = nullptr;
auto hub = OneWireHub(iButtonPort);

const int LOG_LINES = 64;
const int LOG_LINE_LEN = 96;
char logBuf[LOG_LINES][LOG_LINE_LEN];
int logCount = 0;
int logStart = 0;

void logLine(const char* fmt, ...) {
  int writeIdx;
  if (logCount < LOG_LINES) {
    writeIdx = (logStart + logCount) % LOG_LINES;
    logCount++;
  } else {
    writeIdx = logStart;
    logStart = (logStart + 1) % LOG_LINES;
  }
  va_list args;
  va_start(args, fmt);
  vsnprintf(logBuf[writeIdx], LOG_LINE_LEN, fmt, args);
  va_end(args);
  Serial.println(logBuf[writeIdx]);
}

String getLogs() {
  String result;
  for (int i = 0; i < logCount; i++) {
    int idx = (logStart + i) % LOG_LINES;
    result += String(logBuf[idx]) + "\n";
  }
  return result;
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>iButton Emulator</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 0 auto; padding: 16px; max-width: 720px; }
    h1 { font-size: 1.4em; margin: 0 0 12px 0; }
    h2 { font-size: 1.1em; margin: 20px 0 8px 0; }
    .current { background: #e8f5e9; padding: 10px; border: 1px solid #c8e6c9; margin-bottom: 12px; }
    .alert { background: #fff9c4; padding: 10px; border: 1px solid #fbc02d; margin-bottom: 12px; }
    .key { border: 1px solid #ddd; padding: 8px; margin: 6px 0; }
    .key.active { background: #4CAF50; color: white; }
    .name { font-weight: bold; }
    .id { font-family: monospace; font-size: 0.85em; margin-top: 4px; }
    .btn { padding: 6px 10px; margin-left: 4px; cursor: pointer; }
    button:disabled { opacity: 0.4; cursor: not-allowed; }
    .controls { margin: 12px 0; }
    .controls .btn { margin-left: 0; margin-right: 4px; }
    .footer { text-align: center; margin-top: 24px; }
    .footer a { font-size: 0.85em; color: #666; text-decoration: none; }
  </style>
</head>
<body>
  <h1>iButton Emulator</h1>

  <div class="controls">
    <button class="btn" id="readBtn" onclick="startRead()">Read key</button>
    <button class="btn" id="bruteBtn" onclick="startBrute()">Brute</button>
  </div>

  <div id="readBox" class="alert" style="display:none">
    <b>Reading...</b> Apply a key to the probe now.
    <button class="btn" onclick="cancelRead()">Cancel</button>
  </div>

  <div id="bruteBox" class="alert" style="display:none">
    <b>Bruting</b> key <span id="brutePos">0/0</span> - <span id="bruteName"></span>
    <button class="btn" onclick="stopBrute()">Stop</button>
  </div>

  <div id="pendingBox" class="alert" style="display:none">
    <b>Key read:</b> <span id="pendingId" class="id"></span>
    <br>
    <button class="btn" onclick="addPending()">Add to list</button>
    <button class="btn" onclick="discardPending()">Discard</button>
  </div>

  <div id="current" class="current">
    <div><b>Current key:</b> <span id="curName">...</span></div>
    <div id="curId" class="id">...</div>
  </div>

  <h2>Keys (<span id="cnt">0</span>)</h2>
  <div id="list"></div>

  <div class="footer">
    <a href="/log">Logs</a>
  </div>

  <script>
    function load() {
      fetch('/current').then(function(r) { return r.json(); }).then(function(d) {
        document.getElementById('curName').textContent = d.name;
        document.getElementById('curId').textContent = d.key;
        document.getElementById('cnt').textContent = d.allKeys.length;

        var rb = document.getElementById('readBtn');
        var bb = document.getElementById('bruteBtn');
        var rbox = document.getElementById('readBox');
        var bbox = document.getElementById('bruteBox');
        var pbox = document.getElementById('pendingBox');

        if (d.readMode) {
          rbox.style.display = 'block';
          bbox.style.display = 'none';
          pbox.style.display = 'none';
          rb.disabled = true;
          bb.disabled = true;
        } else if (d.bruteMode) {
          rbox.style.display = 'none';
          bbox.style.display = 'block';
          pbox.style.display = 'none';
          rb.disabled = true;
          bb.disabled = true;
          document.getElementById('brutePos').textContent = (d.bruteIndex + 1) + '/' + d.bruteTotal;
          document.getElementById('bruteName').textContent = d.allKeys[d.bruteIndex].name;
        } else if (d.hasPendingKey) {
          rbox.style.display = 'none';
          bbox.style.display = 'none';
          pbox.style.display = 'block';
          rb.disabled = false;
          bb.disabled = false;
          document.getElementById('pendingId').textContent = d.pendingKey;
        } else {
          rbox.style.display = 'none';
          bbox.style.display = 'none';
          pbox.style.display = 'none';
          rb.disabled = false;
          bb.disabled = false;
        }

        var html = '';
        var dis = (d.readMode || d.bruteMode) ? ' disabled' : '';
        for (var i = 0; i < d.allKeys.length; i++) {
          var cls = (i === d.index) ? 'key active' : 'key';
          html += '<div class="' + cls + '">' +
                  '<span class="name">' + d.allKeys[i].name + '</span>' +
                  '<button class="btn" onclick="setKey(' + i + ')"' + dis + '>Select</button>' +
                  '<div class="id">' + d.allKeys[i].value + '</div>' +
                  '</div>';
        }
        document.getElementById('list').innerHTML = html;
      }).catch(function(e) { console.error(e); });
    }
    function startRead() { fetch('/readkey').then(load); }
    function cancelRead() { fetch('/cancelread').then(load); }
    function startBrute() { fetch('/startbrute').then(load); }
    function stopBrute() { fetch('/stopbrute').then(load); }
    function setKey(i) { fetch('/setkey?idx=' + i).then(load); }
    function addPending() { fetch('/addpending').then(load); }
    function discardPending() { fetch('/discardpending').then(load); }
    setInterval(load, 500);
    window.onload = load;
  </script>
</body>
</html>
)rawliteral";

const char log_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>iButton Logs</title>
  <style>
    body { font-family: monospace; padding: 16px; max-width: 900px; margin: 0 auto; }
    h1 { font-size: 1.2em; }
    pre { background: #f4f4f4; padding: 10px; border: 1px solid #ddd; white-space: pre-wrap; word-wrap: break-word; }
    a { display: inline-block; margin-bottom: 12px; font-family: Arial, sans-serif; }
  </style>
</head>
<body>
  <a href="/">&laquo; Back</a>
  <h1>Logs</h1>
  <pre id="logs">loading...</pre>
  <script>
    function loadLogs() {
      fetch('/logdata').then(function(r) { return r.text(); }).then(function(t) {
        document.getElementById('logs').textContent = t;
      }).catch(function(e) { console.error(e); });
    }
    setInterval(loadLogs, 1500);
    window.onload = loadLogs;
  </script>
</body>
</html>
)rawliteral";

String keyToString(byte* key) {
  String s;
  for (int i = 0; i < 8; i++) {
    if (key[i] < 0x10) s += "0";
    s += String(key[i], HEX);
    if (i < 7) s += ":";
  }
  return s;
}

byte calcCRC8(const byte *data, byte len) {
  byte crc = 0;
  for (byte i = 0; i < len; i++) {
    byte inbyte = data[i];
    for (byte j = 0; j < 8; j++) {
      byte mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix) crc ^= 0x8C;
      inbyte >>= 1;
    }
  }
  return crc;
}

void initDefaultKeys() {
  for (int i = 0; i < 34; i++) {
    memcpy(keys[i], defaultKeys[i], 8);
    keys[i][7] = calcCRC8(keys[i], 7);
    keyNames[i] = String(defaultKeyNames[i]);
  }
  keyCount = 34;
}

void rebuildDs1990() {
  if (ds1990A != nullptr) {
    hub.detach(*ds1990A);
    delete ds1990A;
  }
  ds1990A = new DS2401(
    keys[currentKeyIndex][0], keys[currentKeyIndex][1], keys[currentKeyIndex][2],
    keys[currentKeyIndex][3], keys[currentKeyIndex][4], keys[currentKeyIndex][5],
    keys[currentKeyIndex][6]
  );
  hub.attach(*ds1990A);
}

static inline void owRelease() {
  pinMode(iButtonPort, INPUT_PULLUP);
}

static inline void owLow() {
  digitalWrite(iButtonPort, LOW);
  pinMode(iButtonPort, OUTPUT);
}

static bool owReset() {
  noInterrupts();
  owLow();
  delayMicroseconds(480);
  owRelease();
  delayMicroseconds(70);
  bool presence = (digitalRead(iButtonPort) == LOW);
  delayMicroseconds(410);
  interrupts();
  return presence;
}

static void owWriteBit(uint8_t v) {
  noInterrupts();
  if (v) {
    owLow();
    delayMicroseconds(6);
    owRelease();
    delayMicroseconds(64);
  } else {
    owLow();
    delayMicroseconds(60);
    owRelease();
    delayMicroseconds(10);
  }
  interrupts();
}

static uint8_t owReadBit() {
  noInterrupts();
  owLow();
  delayMicroseconds(6);
  owRelease();
  delayMicroseconds(9);
  uint8_t v = (digitalRead(iButtonPort) == HIGH) ? 1 : 0;
  delayMicroseconds(55);
  interrupts();
  return v;
}

static void owWriteByte(uint8_t b) {
  for (uint8_t i = 0; i < 8; i++) {
    owWriteBit(b & 1);
    b >>= 1;
  }
}

static uint8_t owReadByte() {
  uint8_t b = 0;
  for (uint8_t i = 0; i < 8; i++) {
    b >>= 1;
    if (owReadBit()) b |= 0x80;
  }
  return b;
}

bool tryReadKey() {
  if (!owReset()) {
    return false;
  }
  owWriteByte(0x33);
  byte rom[8];
  for (int i = 0; i < 8; i++) {
    rom[i] = owReadByte();
  }
  if (rom[0] != 0x01) {
    logLine("[ibutton] wrong family: %02X", rom[0]);
    return false;
  }
  if (calcCRC8(rom, 7) != rom[7]) {
    logLine("[ibutton] CRC fail");
    return false;
  }
  memcpy(pendingKey, rom, 8);
  hasPendingKey = true;
  logLine("[ibutton] read ok: %s", keyToString(pendingKey).c_str());
  return true;
}

void startReadMode() {
  if (readMode) return;
  readMode = true;
  hasPendingKey = false;
  if (ds1990A != nullptr) hub.detach(*ds1990A);
  pinMode(iButtonPort, INPUT_PULLUP);
  logLine("[ibutton] read mode ON, waiting for key");
}

void endReadMode() {
  if (!readMode) return;
  readMode = false;
  pinMode(iButtonPort, INPUT_PULLUP);
  keyChanged = true;
  logLine("[ibutton] read mode OFF, emulation resumes");
}

void startBruteMode() {
  if (bruteMode) return;
  bruteMode = true;
  bruteIndex = 0;
  currentKeyIndex = 0;
  keyChanged = true;
  bruteStartTime = millis();
  logLine("[ibutton] brute started");
}

void stopBruteMode() {
  if (!bruteMode) return;
  bruteMode = false;
  logLine("[ibutton] brute stopped at key %d", bruteIndex);
}

String buildCurrentJson() {
  String j;
  j += "{";
  j += "\"index\":" + String(currentKeyIndex) + ",";
  j += "\"name\":\"" + String(keyNames[currentKeyIndex]) + "\",";
  j += "\"key\":\"" + keyToString(keys[currentKeyIndex]) + "\",";
  j += "\"readMode\":" + String(readMode ? "true" : "false") + ",";
  j += "\"bruteMode\":" + String(bruteMode ? "true" : "false") + ",";
  j += "\"bruteIndex\":" + String(bruteIndex) + ",";
  j += "\"bruteTotal\":" + String(keyCount) + ",";
  j += "\"hasPendingKey\":" + String(hasPendingKey ? "true" : "false") + ",";
  if (hasPendingKey) {
    j += "\"pendingKey\":\"" + keyToString(pendingKey) + "\",";
  } else {
    j += "\"pendingKey\":\"\",";
  }
  j += "\"allKeys\":[";
  for (int i = 0; i < keyCount; i++) {
    j += "{\"name\":\"" + String(keyNames[i]) + "\",";
    j += "\"value\":\"" + keyToString(keys[i]) + "\"}";
    if (i < keyCount - 1) j += ",";
  }
  j += "]}";
  return j;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  logLine("[ibutton] boot");

  pinMode(buttonPin, INPUT_PULLUP);

  initDefaultKeys();

  WiFi.persistent(false);
  WiFi.disconnect(true);
  delay(200);
  WiFi.mode(WIFI_AP);
  bool ap_ok = WiFi.softAP(ap_ssid, ap_password);
  IPAddress ap_ip = WiFi.softAPIP();
  String ap_ip_str = ap_ok ? ap_ip.toString() : String("N/A");
  logLine("[ibutton] AP %s, IP %s", ap_ok ? "ok" : "fail", ap_ip_str.c_str());

  rebuildDs1990();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/html", String(index_html));
  });

  server.on("/log", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/html", String(log_html));
  });

  server.on("/logdata", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/plain", getLogs());
  });

  server.on("/current", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "application/json", buildCurrentJson());
  });

  server.on("/setkey", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (readMode) { req->send(403, "text/plain", "Read mode is on"); return; }
    if (bruteMode) { req->send(403, "text/plain", "Brute mode is on"); return; }
    if (!req->hasParam("idx")) { req->send(400, "text/plain", "missing idx"); return; }
    int newIdx = req->getParam("idx")->value().toInt();
    if (newIdx < 0 || newIdx >= keyCount) { req->send(400, "text/plain", "invalid idx"); return; }
    currentKeyIndex = newIdx;
    keyChanged = true;
    req->send(200, "text/plain", "ok");
    logLine("[ibutton] key %d via web", newIdx);
  });

  server.on("/readkey", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (readMode) { req->send(403, "text/plain", "Already reading"); return; }
    if (bruteMode) { req->send(403, "text/plain", "Brute mode is on"); return; }
    startReadMode();
    req->send(200, "text/plain", "read started");
  });

  server.on("/cancelread", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (readMode) endReadMode();
    req->send(200, "text/plain", "read cancelled");
  });

  server.on("/startbrute", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (readMode) { req->send(403, "text/plain", "Read mode is on"); return; }
    if (bruteMode) { req->send(403, "text/plain", "Already bruting"); return; }
    startBruteMode();
    req->send(200, "text/plain", "brute started");
  });

  server.on("/stopbrute", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (bruteMode) stopBruteMode();
    req->send(200, "text/plain", "brute stopped");
  });

  server.on("/addpending", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (!hasPendingKey) { req->send(400, "text/plain", "no pending key"); return; }
    if (keyCount >= KEY_MAX) { req->send(400, "text/plain", "list full"); return; }
    memcpy(keys[keyCount], pendingKey, 8);
    keyNames[keyCount] = "Custom " + keyToString(pendingKey);
    currentKeyIndex = keyCount;
    keyChanged = true;
    keyCount++;
    hasPendingKey = false;
    req->send(200, "text/plain", "added");
    logLine("[ibutton] added key %d and activated", keyCount - 1);
  });

  server.on("/discardpending", HTTP_GET, [](AsyncWebServerRequest *req) {
    hasPendingKey = false;
    req->send(200, "text/plain", "discarded");
    logLine("[ibutton] pending discarded");
  });

  server.onNotFound([](AsyncWebServerRequest *req) {
    logLine("[ibutton] 404 %s", req->url().c_str());
    req->send(404, "text/plain", "not found");
  });

  server.begin();
  logLine("[ibutton] http up");
}

void loop() {
  if (keyChanged && !readMode) {
    rebuildDs1990();
    keyChanged = false;
    logLine("[ibutton] now key %d (%s)", currentKeyIndex, keyNames[currentKeyIndex].c_str());
  }

  static bool lastBtn = HIGH;
  bool btn = digitalRead(buttonPin);
  if (!readMode && !bruteMode && lastBtn == HIGH && btn == LOW) {
    currentKeyIndex = (currentKeyIndex + 1) % keyCount;
    keyChanged = true;
    logLine("[ibutton] button pressed");
  }
  lastBtn = btn;

  if (readMode) {
    if (tryReadKey()) {
      endReadMode();
    } else {
      delay(50);
    }
  } else if (bruteMode) {
    unsigned long now = millis();
    if (now - bruteStartTime >= BRUTE_KEY_DURATION_MS) {
      bruteIndex++;
      if (bruteIndex >= keyCount) {
        bruteMode = false;
        logLine("[ibutton] brute finished");
      } else {
        currentKeyIndex = bruteIndex;
        keyChanged = true;
        bruteStartTime = now;
        logLine("[ibutton] brute key %d/%d", bruteIndex + 1, keyCount);
      }
    }
    hub.poll();
    delay(5);
  } else {
    hub.poll();
    delay(10);
  }
}
