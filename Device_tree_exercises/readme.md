# Device tree on Orange pi zero 2W
## Create a new device tree
* On board, go to `/boot/dtb/allwinner` (specific to different boards).
* Copy `sun50i-h616-orangepi-zero2w.dtb` to other folder
* Decompile `sun50i-h616-orangepi-zero2w.dtb` to the `sun50i-h616-orangepi-zero2w.dts` file using
  ```C
  // Decompile the .dtb to .ts
  sudo dtc -I dtb -O dts sun50i-h616-orangepi-zero2w.dtb -o sun50i-h616-orangepi-zero2w.dts
  ```

* Create a new `.dtsi` file to add new devices, such as `manhtd24-device-orangepi-zero2w.dtsi`

* Include the  `.dtsi` above in the `.dts` file that created before as below:
  ```
  /dts-v1/;
  /include/ "manhtd24-device-orangepi-zero2w.dtsi"

  / {
    interrupt-parent = <0x01>;
    #address-cells = <0x02>;
    #size-cells = <0x02>;
    ...
  ```

* Then compile the modified .dts file into a new .dtb:
  ```C
  // Compile .dts back to .dtb
  sudo dtc -I dts -O dtb sun50i-h616-orangepi-zero2w.dts -o sun50i-h616-orangepi-zero2w.dtb
  ```

* Now, copy the new `.dtb` into the `/boot/dtb/allwinner`.
* Reboot

## Load driver
* Make kernel module using `make`, use cross-compile.
* Load into kernel using `sudo insmod manhtd24_driver.ko`.
* Now, to check if the device was match and initialize, use `dmesg`
  ```CSS
  dmesg | tail - 30` // to get the lastest 30 lines.
  ```
* Driver load successfull and devices are detected:
  ```
  [   79.686841] manhtd24_device_driver 1000.device: Register base: 0x1000, size: 0x100
  [   79.686851] manhtd24_device_driver 1000.device: Label: device1
  [   79.686857] manhtd24_device_driver 1000.device: Device size: 512 bytes
  [   79.686863] manhtd24_device_driver 1000.device: Device serial number: A3F5G7H2I9J0
  [   79.686868] manhtd24_device_driver 1000.device: Permission: ro
  [   79.687051] manhtd24_device_driver 1000.device: Device successfully matched and initialized
  [   79.687150] manhtd24_device_driver 2000.device: Register base: 0x2000, size: 0x100
  [   79.687159] manhtd24_device_driver 2000.device: Label: device2
  [   79.687164] manhtd24_device_driver 2000.device: Device size: 1024 bytes
  [   79.687170] manhtd24_device_driver 2000.device: Device serial number: X5K3W1D9L7P4
  [   79.687175] manhtd24_device_driver 2000.device: Permission: rw
  [   79.687242] manhtd24_device_driver 2000.device: Device successfully matched and initialized
  [   79.687297] manhtd24_device_driver 4000.device: Register base: 0x4000, size: 0x100
  [   79.687304] manhtd24_device_driver 4000.device: Label: device4
  [   79.687310] manhtd24_device_driver 4000.device: Device size: 2048 bytes
  [   79.687316] manhtd24_device_driver 4000.device: Device serial number: B4S7Y9J6V1R3
  [   79.687321] manhtd24_device_driver 4000.device: Permission: rw
  [   79.687383] manhtd24_device_driver 4000.device: Device successfully matched and initialized
  [   79.687435] manhtd24_device_driver 5000.device: Register base: 0x5000, size: 0x100
  [   79.687443] manhtd24_device_driver 5000.device: Label: device5
  [   79.687448] manhtd24_device_driver 5000.device: Device size: 1024 bytes
  [   79.687454] manhtd24_device_driver 5000.device: Device serial number: M8N6Q2Z1T0F5
  [   79.687459] manhtd24_device_driver 5000.device: Permission: rw
  [   79.687524] manhtd24_device_driver 5000.device: Device successfully matched and initialized
  [   79.687820] manhtd24 driver initialized
  ```

* Check device file:
  ```
  orangepi@nishimiya:~/Desktop/manhtd24/device_tree$ ls /dev/manhtd24_
  manhtd24_A3F5G7H2I9J0  manhtd24_M8N6Q2Z1T0F5
  manhtd24_B4S7Y9J6V1R3  manhtd24_X5K3W1D9L7P4
  ```

* Check class:
  ```
  orangepi@nishimiya:~/Desktop/manhtd24/device_tree$ ls /sys/class/manhtd24_device_class/manhtd24_
  manhtd24_A3F5G7H2I9J0/ manhtd24_M8N6Q2Z1T0F5/
  manhtd24_B4S7Y9J6V1R3/ manhtd24_X5K3W1D9L7P4/
  ``` 
* Driver unload:
  ```
  [  693.703884] Resources for device manhtd24_M8N6Q2Z1T0F5 cleaning up . . .
  [  693.704232] manhtd24_device_driver 5000.device: Device removed
  [  693.704333] Resources for device manhtd24_B4S7Y9J6V1R3 cleaning up . . .
  [  693.704494] manhtd24_device_driver 4000.device: Device removed
  [  693.704568] Resources for device manhtd24_X5K3W1D9L7P4 cleaning up . . .
  [  693.704774] manhtd24_device_driver 2000.device: Device removed
  [  693.704848] Resources for device manhtd24_A3F5G7H2I9J0 cleaning up . . .
  [  693.705003] manhtd24_device_driver 1000.device: Device removed
  [  693.705133] manhtd24 driver exited
  ```

* File operation
  * read: (The initial is set to idle)
  ```
  orangepi@nishimiya:~/Desktop/manhtd24/device_tree$ sudo cat /dev/manhtd24_A3F5G7H2I9J0 
  idle

  ```
  * write: (check `rw` both rw and `ro` device)
    * on `ro` permission device:
    ```
    orangepi@nishimiya:~/Desktop/manhtd24/device_tree$ echo "running" | sudo tee /dev/manhtd24_A3F5G7H2I9J0
    running
    orangepi@nishimiya:~/Desktop/manhtd24/device_tree$ dmesg | tail -10
    [ 3134.336795] Opened device manhtd24_A3F5G7H2I9J0
    [ 3134.336867] Device is read-only!
    [ 3134.336872] Device is read-only!
    [ 3134.336876] Device is read-only!
    [ 3134.336879] Device is read-only!
    [ 3134.336882] Device is read-only!
    [ 3134.336886] Device is read-only!
    [ 3134.336889] Device is read-only!
    [ 3134.336893] Device is read-only!
    [ 3134.336901] Device manhtd24_A3F5G7H2I9J0 closed
    ```

    * on `rw` permission device:
    ```
    orangepi@nishimiya:~/Desktop/manhtd24/device_tree$ echo "running" | sudo tee /dev/manhtd24_B4S7Y9J6V1R3
    running
    orangepi@nishimiya:~/Desktop/manhtd24/device_tree$ dmesg | tail -4
    [ 3248.465430] Opened device manhtd24_B4S7Y9J6V1R3
    [ 3248.465549] switch to running
    [ 3248.465560] Device manhtd24_B4S7Y9J6V1R3 done write
    [ 3248.465585] Device manhtd24_B4S7Y9J6V1R3 closed

    ```
## .dtsi
```C
  device1: device@1 {
    compatible = "manhtd24,device1";  // Name of the device
    reg = <0x0 0x1000 0x0 0x100>;   // Device ID, Base address, Size
    size = <512>;                   // Device buffer
    label = "device1";              // Label for the device
    serial = "A3F5G7H2I9J0";        // Device serial
    status = "okay";                // Device status (active)
    permission = "rw";              // Read/write permission for the device
  };
```
* `reg`:
  ``` C
  manhtd24_device_driver device@1: . . .   // without reg
  manhtd24_device_driver 1000.device:. . . // with reg
  ```

* `status`: This is not a must-have but they are used for defining the device's availability.
  * If in device tree not have status field, it will be automatically known as okay.
  * If status is set to "okay", driver will recognize, else is the status is diable, device will be disable.
  * Example: I set status of device3 to disable, and remove the status filed in device2:
  * `.dtsi`:
    ```C
    device2: device@2 {
      compatible = "manhtd24,device2";  // Name of the device
      reg = <0x0 0x2000 0x0 0x100>;   // Device ID, Base address, Size
      size = <1024>;                  // Device buffer
      label = "device2";              // Label for the device
      serial = "X5K3W1D9L7P4";        // Device serial
      permission = "rw";              // Read/write permission for the device
    };

    device3: device@3 {
      compatible = "manhtd24,device3";  // Name of the device
      reg = <0x0 0x3000 0x0 0x100>;   // Device ID, Base address, Size
      size = <512>;                   // Device buffer
      label = "device3";              // Label for the device
      serial = "P3D2L5O7A8X1";        // Device serial
      status = "disable";             // Device status (active)
      permission = "ro";              // Read-only permission for the device
    };
    ```
  * Result:
    ```
    [  128.205836] manhtd24_device_driver 2000.device: Register base: 0x2000, size: 0x100
    [  128.205843] manhtd24_device_driver 2000.device: Label: device2
    [  128.205849] manhtd24_device_driver 2000.device: Device size: 1024 bytes
    [  128.205854] manhtd24_device_driver 2000.device: Device serial number: X5K3W1D9L7P4
    [  128.205860] manhtd24_device_driver 2000.device: Permission: rw
    [  128.205864] manhtd24_device_driver 2000.device: Device successfully matched and initialized
    [  128.205917] manhtd24_device_driver 4000.device: Register base: 0x4000, size: 0x100
    [  128.205924] manhtd24_device_driver 4000.device: Label: device4
    [  128.205929] manhtd24_device_driver 4000.device: Device size: 2048 bytes
    [  128.205935] manhtd24_device_driver 4000.device: Device serial number: B4S7Y9J6V1R3
    [  128.205940] manhtd24_device_driver 4000.device: Permission: rw
    [  128.205945] manhtd24_device_driver 4000.device: Device successfully matched and initialized

    ```
  * As you can see, device3 are not shown.
