Hitagi
======

Custom and open-source RAMDLD (Motorola Flash Protocol) implementation for Motorola phones.

## Build

```bash
sudo apt install -y gcc-arm-none-eabi

cd Hitagi
make

make FLASH_TYPE=intel16
make FLASH_TYPE=amd16
```

## Run

Please use the **Flash Terminal** utility: **[https://github.com/EXL/FlashTerminal](https://github.com/EXL/FlashTerminal)**

```bash
cd FlashTerminal
sudo ./FlashTerminal.py -v -l
```

## Notes

1. It is better if the flashed chunk size is a multiple of `0x8000` (parameter blocks) or `0x20000` (main blocks) for Intel-like and AMD-like flash chips.

2. Flash modes can be switched by calling the method that sets the `ERASE` flag a different number of times.

   ```python
   # 0. Read-only mode. No `ERASE` flag is set.
   
   # 1. Read/Write word mode for the entire flash.
   mfp_cmd(er, ew, 'ERASE')
   
   # 2. Read/Write buffer mode for the entire flash.
   mfp_cmd(er, ew, 'ERASE')
   mfp_cmd(er, ew, 'ERASE')
   
   # 3. Erase-only mode for the entire flash.
   mfp_cmd(er, ew, 'ERASE')
   mfp_cmd(er, ew, 'ERASE')
   mfp_cmd(er, ew, 'ERASE')
   ```

## Credits & Thanks

* **[@muromec](https://github.com/muromec)**
* **[@EXL](https://github.com/EXL)**
* **[@Siesta](https://github.com/Siesta)** aka fkcoder, LAVMEN
* **[@Vilko](https://github.com/Vilko)**
* GanjaFuzz
* KAMTOS
* Oleg aka velocidad_absurda and MSS Box II developers
* P2K Easy Tool authors
* MotoFan.Ru developers
* Motorola developers and engineers
* Intel and AMD engineers and other
* DenK for testing Spansion memory chip flashing on Siemens CC75
