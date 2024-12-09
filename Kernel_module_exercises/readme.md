## Manual loading
1. Load Module A first:
	```
		sudo insmod module_a.ko
	```
2. Then load module B:
	``` 
		sudo insmod module_b.ko
	```
## Automatic Loading via a Tool
- Write a script to load Module A and then Module B:
	```
		#!/bin/bash
		sudo insmod module_a.ko
		sudo insmod module_b.ko
	```
- Run the script:
	```
		./load_modules.sh
	```
## Automatic Loading Without a Tool:
- Add Module B’s dependency on Module A using `modprobe`: Create a `modules.dep` file to specify dependencies:
	```
		echo "module_b: module_a" > modules.dep
	```
- This tells modprobe that module_b depends on module_a, 
  and modprobe will load module_a first if module_b is loaded.	
  
- After adding this, update the module dependencies
  ```
		sudo depmod
	```
- Copy the module_b to the kernel's module directory:
	```
		sudo cp module_b.ko /lib/modules/`uname -r`/kernel/
	```
- Load Module B using modprobe:
	```
		sudo modprobe module_b
	```
---
* Verify

	```
		lsmod
		sudo dmesg | tail
	```
