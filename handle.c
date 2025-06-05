#if defined(FTR_LTE)
void (*ramldr_start)() = (void*)0x03FD0010;
#elif defined(FTR_LTE2)
void (*ramldr_start)() = (void*)0x03FC8014;
#endif

char _hlo[] = "BE @ LE\0";
short _hlo_len = 8;

char _err[] = "baAAkaAaA\0";
short _err_len = 10;

char _version[] = "\x02RSVN\x1eHitagi 0.2\x03";
short _version_len = 17;

void handle_ack() {
  usb_write_ep2( (unsigned short*)_hlo, _hlo_len);
}

void handle_err() {
  usb_write_ep2( (unsigned short*)_err, _err_len);
}

void handle_version() {
  dump(_version, _version_len);
}

void handle_echo(char *arg) {
  usb_write_ep2( (unsigned short*)arg, strlen(arg));
}

// Function to convert a hex string to an integer
unsigned int hexStringToInt(const char *hexString) {
    unsigned int result = 0;
    while (*hexString) {
        char c = *hexString;
        int value;

        // Convert a single hex character to its integer value
        if (c >= '0' && c <= '9') {
            value = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            value = c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
            value = c - 'a' + 10;
        } else {
            return 0; // Return 0 for invalid input
        }

        // Combine the value into the result
        result = (result << 4) | value;
        hexString++;
    }
    return result;
}

void handle_dump(char *arg) {
    dump(hexStringToInt(arg), 0x800);
}

void handle_ramldr() {
  ramldr_start();
}


