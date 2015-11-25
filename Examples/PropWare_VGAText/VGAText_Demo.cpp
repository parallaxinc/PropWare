#include <PropWare/hmi/output/video/vgatext.h>

int main() {
    PropWare::VGAText Text;
    int i;

    Text.start(8);
    Text.str("\r   VGA Text Demo...\r\r\x0C\x05 OBJ and VAR require only 5.0KB \x0C\x01");
    for (i = 0; i < 14; i++) {
        Text.out(' ');
    }
    for (i = 0x0E; i <= 0xFF; i++) {
        Text.out(i);
    }
    Text.str("\x0C\x06     Uses internal ROM font     \x0C\x02");
    for (;;) {
      Text.str("\x0A\x0C\x0B\x0E");
      Text.hex(i++, 8);
    }
}
