#include <PropWare/hmi/output/video/vgatext.h>
#include <PropWare/hmi/output/printer.h>

int main() {
    PropWare::VGAText vgaText;
    PropWare::Printer vgaPrinter(vgaText);
    vgaText.start(8);

    vgaPrinter << "\r   VGA Text Demo...\r\r\x0C\x05 OBJ and VAR require only 5.0KB \x0C\x01";

    for (char i = 0; i < 14; i++)
        vgaPrinter << ' ';

    for (char i = 0x0E; i <= 0xFF; i++)
        vgaPrinter << i;

    vgaPrinter << "\x0C\x06     Uses internal ROM font     \x0C\x02";

    uint16_t i = 0xFF;
    while (1) {
        vgaPrinter << "\x0A\x0C\x0B\x0E";
        vgaPrinter.printf("0x04X ", i++);
    }
}
