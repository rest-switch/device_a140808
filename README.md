```
minipro tl866a to a140808


             2     4      6
           +----+------+------+
  a140808  | v+ | mosi | sck  |
           +----+------+------+
           | g  | rst  | miso |
           +----+------+------+
             1     3      5


           +-----+----+---+------+------+-----+
  minipro  | rst | v+ | g | mosi | miso | sck |
           +-----+----+---+------+------+-----+
              1    2    3    4      5      6


             +------+
  minipro  1 | rst  | 3  a140808
             +------+
           2 |  v+  | 2
             +------+
           3 |  g   | 1
             +------+
           4 | mosi | 4
             +------+
           5 | miso | 5
             +------+
           6 | sck  | 6
             +------+

AVR Resources:
http://www.atmel.com/Images/Atmel-8155-8-bit-Microcontroller-AVR-ATmega32A_Datasheet.pdf
http://www.atmel.com/webdoc/AVRLibcReferenceManual/group__demo__project_1demo_project_compile.html

Minipro TL866A Resources: 
http://www.autoelectric.cn/EN/TL866_main.html
https://github.com/inindev/minipro

AVRISP mkii
http://www.atmel.com/webdoc/avrispmkii
http://www.amazon.com/Compatible-AVRISP-In-System-Programmer-interface/dp/B00C7VV6E4
```
