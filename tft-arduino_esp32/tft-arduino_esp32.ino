/*
 * TFT PARALELA 8 BITS + WIFI - Descarga y muestra un JPG desde GitHub
 * ESP32 DevKit V1 | Libreria: Arduino_GFX (moononournation)
 * -------------------------------------------------------------------
 * Basado en el ejemplo oficial WiFiPhotoFrame, adaptado a:
 *   - bus paralelo 8 bits con tus pines
 *   - descarga directa de un archivo JPG por URL (GitHub raw)
 *
 * LIBRERIAS (Library Manager):
 *   - GFX Library for Arduino  (moononournation)
 *   - JPEGDEC                   (bitbank2)
 *
 * ARCHIVO AUXILIAR:
 *   - Copia "JpegFunc.h" desde el ejemplo WiFiPhotoFrame a esta carpeta.
 *
 * EDITAR: SSID, PASSWORD y el nombre de la imagen (IMG_PATH).
 */

/* ====================== WiFi ====================== */
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char *SSID_NAME     = "wifi-telefono";     // <<< EDITAR
const char *SSID_PASSWORD = "clave-wifi";   // <<< EDITAR

// Host fijo de GitHub raw + ruta de tu repo. Solo cambia el archivo final.
const char *HTTP_HOST = "raw.githubusercontent.com";
const uint16_t HTTP_PORT = 443; // HTTPS
const char *IMG_PATH  = "/cuentagithub/repositorio/main/archivo.jpg"; // <<< EDITAR (01..04)
const uint16_t HTTP_TIMEOUT = 30000;

WiFiClientSecure client;  // HTTPS: GitHub exige TLS
HTTPClient http;

/* ====================== Pantalla ====================== */
#include <Arduino_GFX_Library.h>

Arduino_DataBus *bus = new Arduino_ESP32PAR8(
  21 /* RS/DC */, 33 /* CS */, 4 /* WR */, 22 /* RD */,
  12 /* D0 */, 13 /* D1 */, 26 /* D2 */, 25 /* D3 */,
  17 /* D4 */, 16 /* D5 */, 27 /* D6 */, 14 /* D7 */
);
Arduino_GFX *gfx = new Arduino_ILI9341(bus, 32 /* RST */, 0 /* rotation */);

/* ====================== Decodificador JPG ====================== */
#include "JpegFunc.h"

// Callback: dibuja cada bloque decodificado
static int jpegDrawCallback(JPEGDRAW *pDraw) {
  gfx->draw16bitRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels,
                          pDraw->iWidth, pDraw->iHeight);
  return 1;
}

void mostrarImagen() {
  Serial.printf("[HTTP] GET https://%s%s\n", HTTP_HOST, IMG_PATH);
  http.setTimeout(HTTP_TIMEOUT);

  String url = String("https://") + HTTP_HOST + IMG_PATH;
  http.begin(client, url);
  int httpCode = http.GET();
  Serial.printf("[HTTP] code: %d\n", httpCode);

  if (httpCode != 200) {
    Serial.println("[HTTP] No OK");
    gfx->fillScreen(RGB565_BLACK);
    gfx->setCursor(10, 10);
    gfx->setTextColor(RGB565_RED);
    gfx->setTextSize(2);
    gfx->printf("HTTP %d", httpCode);
    http.end();
    return;
  }

  int len = http.getSize();
  Serial.printf("[HTTP] size: %d\n", len);
  int jpeg_result = 0;
  unsigned long start = millis();

  // Limpiar la pantalla (borra el texto "Conectando WiFi...")
  gfx->fillScreen(RGB565_BLACK);

  // Decodificamos DIRECTO desde el stream, sin reservar un buffer
  // grande en RAM. Esto evita que el malloc de ~86 KB falle o
  // fragmente la memoria y haga fallar el decode (resultado: 0).
  WiFiClient *stream = http.getStreamPtr();
  jpeg_result = jpegOpenHttpStream(stream, len, jpegDrawCallback);
  if (jpeg_result) {
    jpeg_result = jpegDraw(false /* useBigEndian */,
                           0, 0, gfx->width(), gfx->height());
  } else {
    Serial.println("jpegOpenHttpStream fallo al abrir el JPG");
  }

  Serial.printf("Tiempo: %lu ms | resultado: %d\n", millis() - start, jpeg_result);
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== TFT WiFi JPG ===");

  if (!gfx->begin()) Serial.println("gfx->begin() fallo");
  gfx->fillScreen(RGB565_BLACK);
  gfx->setCursor(10, 10);
  gfx->setTextColor(RGB565_WHITE);
  gfx->setTextSize(2);
  gfx->println("Conectando WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID_NAME, SSID_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK. IP: " + WiFi.localIP().toString());

  // GitHub usa HTTPS. Para simplificar, no validamos el certificado.
  client.setInsecure();

  mostrarImagen();
}

void loop() {
  // Vacio por ahora.
}
