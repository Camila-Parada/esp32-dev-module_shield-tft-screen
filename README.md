# TFT paralela 2.4" + ESP32 + WiFi: mostrar imágenes JPG desde GitHub

Proyecto para mostrar imágenes JPG en una **pantalla TFT de 2.4" con bus paralelo de 8 bits**, usando una **ESP32 DevKit V1**, descargándolas por WiFi directamente desde un repositorio de GitHub.

La ESP32 se conecta a la red, baja una imagen `.jpg` desde una URL *raw* de GitHub, la decodifica sobre la marcha y la pinta en la pantalla.

![Foto del proyecto funcionando](docs/demo.jpg)

---

## Hardware

| Componente | Detalle |
|---|---|
| Microcontrolador | [ESP32 DevKit V1 (30 pines, módulo ESP-WROOM-32)](https://afel.cl/products/esp32-devkit-v1-30-pines-microusb) |

<img width="1536" height="752" alt="image" src="https://github.com/user-attachments/assets/dce5ceca-b734-44fc-8742-5b335bda32d6" />

| Pantalla | [Shield TFT 2.4", controlador **ILI9341**](https://afel.cl/products/shield-tft-pantalla-2-4-display-para-arduino), **bus paralelo 8 bits** |

<img width="800" height="800" alt="image" src="https://github.com/user-attachments/assets/4535f411-c8e3-43db-a0cc-d148e4bd7a6c" />

| Alimentación | Vía USB de la ESP32 |

> **Importante sobre la pantalla:** estas shields rojas de 2.4" formato Arduino UNO usan un **bus paralelo de 8 bits** (pines `LCD_D0`–`LCD_D7` + `WR/RD/RS/CS/RST`), **no SPI**. Los pines `SD_xx` de la placa son solo para la ranura microSD, no para el display. Confírmalo mirando las etiquetas serigrafiadas en la parte trasera.

---

## Conexiones

La shield es formato UNO, pero aquí **no se enchufa sobre un Arduino**: se cablea pin a pin a la ESP32 con cables dupont.

### Bus de datos (8 bits)

| Shield | ESP32 |
|---|---|
| LCD_D0 | GPIO 12 |
| LCD_D1 | GPIO 13 |
| LCD_D2 | GPIO 26 |
| LCD_D3 | GPIO 25 |
| LCD_D4 | GPIO 17 |
| LCD_D5 | GPIO 16 |
| LCD_D6 | GPIO 27 |
| LCD_D7 | GPIO 14 |

### Señales de control

| Shield | ESP32 |
|---|---|
| LCD_CS | GPIO 33 |
| LCD_RS (DC) | GPIO 21 |
| LCD_WR | GPIO 4 |
| LCD_RD | GPIO 22 |
| LCD_RST | GPIO 32 |

### Alimentación

| Shield | ESP32 |
|---|---|
| 3V3 | 3V3 |
| GND | GND |
| 5V | VIN |

> **El backlight se alimenta desde el pin `5V` de la shield.** Si dejas ese pin sin conectar, la pantalla queda **totalmente negra aunque el código funcione**. Conéctalo a **VIN** de la ESP32 (que entrega los ~5V del USB).

> **Evita los strapping pins.** No uses **GPIO 2** ni **GPIO 15** para señales del display: son *strapping pins* que la ESP32 lee durante el arranque, y tenerlos ocupados **impide subir código** (`Failed to communicate with the flash chip`). Por eso este proyecto usa GPIO 21 y 22 para RS y RD.

---

## Software

### Librerías necesarias (Arduino IDE → Library Manager)

- **GFX Library for Arduino** (de *moononournation*)
- **JPEGDEC** (de *bitbank2*)

### Archivo auxiliar

El sketch usa `JpegFunc.h`, un archivo helper que viene con los ejemplos de la librería GFX. Cópialo a la carpeta del sketch desde:

`Archivo → Ejemplos → GFX Library for Arduino → WiFiPhotoFrame → JpegFunc.h`

### Configuración del IDE

- **Placa:** `ESP32 Dev Module` (NO una variante S3, C3, etc.)
- **Upload Speed:** si al subir aparece `Serial data stream stopped`, baja a `115200`.
- **Core ESP32:** este proyecto usa el core **v3.x**. La librería GFX es compatible; otras librerías más viejas para estas pantallas (como MCUFRIEND_kbv) **no compilan** con el core v3.x (dan `'GPIO' was not declared`).

### Antes de subir

En el sketch, edita:

```cpp
const char *SSID_NAME     = "TU_RED_WIFI";
const char *SSID_PASSWORD = "TU_CLAVE_WIFI";
const char *IMG_PATH      = "/usuario/repo/main/imagen.jpg";
```

- Usa una red **WiFi de 2.4 GHz** (la ESP32 no se conecta a 5 GHz).
- `IMG_PATH` es la ruta *raw* de tu imagen: todo lo que va después de `raw.githubusercontent.com`.

### Truco para subir código

Si la subida no arranca (se queda en `Connecting....`):
1. Inicia la subida en el IDE.
2. Mantén presionado el botón **BOOT** de la ESP32.
3. Suéltalo cuando empiece a escribir (`Writing...`).

---

## Preparar las imágenes

**Este es el punto más importante para que funcione de forma estable.**

Las imágenes deben ser **pequeñas**. La pantalla es de 240×320 px, así que no tiene sentido subir fotos de varios megapíxeles: pesan mucho, la descarga puede hacer *timeout* y la decodificación puede quedarse sin memoria.

**Recomendación:** redimensiona cada imagen a **320 px en el lado mayor** y guárdala como JPG con calidad ~85%. Así cada archivo pesa entre 10 y 30 KB, se descarga rápido y se decodifica sin problemas.

Ejemplo con Python (Pillow):

```python
from PIL import Image
im = Image.open("original.jpg").convert("RGB")
im.thumbnail((320, 320), Image.LANCZOS)
im.save("salida.jpg", "JPEG", quality=85, optimize=True)
```

O con ImageMagick:

```bash
convert original.jpg -resize 320x320 -quality 85 salida.jpg
```

---

## Uso

1. Sube las imágenes ya redimensionadas a tu repo de GitHub (repo **público**).
2. Copia la URL *raw* de una imagen (botón "Raw" en GitHub) y pon la ruta en `IMG_PATH`.
3. Edita tu SSID y contraseña.
4. Sube el sketch a la ESP32 (usa el truco del BOOT si hace falta).
5. Abre el **Monitor Serie a 115200**. Deberías ver:

```
=== TFT WiFi JPG ===
WiFi OK. IP: 192.168.x.x
[HTTP] GET https://raw.githubusercontent.com/.../imagen.jpg
[HTTP] code: 200
[HTTP] size: 23456
Tiempo: 350 ms | resultado: 1
```

`resultado: 1` = imagen dibujada correctamente.

---

## Solución de problemas

| Síntoma | Causa probable | Solución |
|---|---|---|
| Pantalla totalmente negra, pero el serial dice que todo funciona | Backlight sin alimentar | Conecta el pin `5V` de la shield a `VIN` de la ESP32 |
| `Failed to communicate with the flash chip` al subir | Strapping pins ocupados (GPIO 2 / 15) | Usa GPIO 21 y 22 para RS y RD; o desconecta esos cables al subir |
| `Serial data stream stopped` al subir | Velocidad alta / cable USB / consumo | Baja Upload Speed a 115200; usa el truco del BOOT; prueba otro cable USB |
| `'GPIO' was not declared in this scope` al compilar | Librería incompatible con core v3.x | Usa la librería GFX de moononournation (no MCUFRIEND_kbv) |
| `'BLACK' was not declared` al compilar | Nombres de color de GFX llevan prefijo | Usa `RGB565_BLACK`, `RGB565_RED`, etc. |
| `[HTTP] code: -11` o `-1` (read timeout) | Imagen muy pesada, o **red con firewall** | Redimensiona las imágenes; si persiste, prueba con otra red (ej: hotspot del celular) |
| `resultado: 0` y la ESP32 se reinicia en bucle | `malloc` de imagen grande falla | Decodifica desde el stream (ya implementado); reduce el tamaño de la imagen |
| Colores invertidos (rojo↔azul) | Flag de color del panel | Ajusta el orden de color en la inicialización del driver |

> **Nota sobre redes institucionales/corporativas:** algunas redes (universidades, oficinas) bloquean o filtran las conexiones HTTPS salientes hacia GitHub, provocando errores de *timeout* intermitentes aunque el código sea correcto. Si la descarga falla solo en cierta red, prueba con el hotspot de tu celular para descartarlo.

---

## Créditos

Basado en el ejemplo `WiFiPhotoFrame` de la librería
[GFX Library for Arduino](https://github.com/moononournation/Arduino_GFX) de moononournation,
adaptado a bus paralelo de 8 bits y descarga directa desde GitHub.
