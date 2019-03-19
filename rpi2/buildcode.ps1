
Clear-Host;

$exec=$args[0]
$ErrorActionPreference = "Stop"

Write-Host "Building..."

& "C:\Program Files (x86)\GNU Tools ARM Embedded\8 2018-q4-major\bin\arm-none-eabi-gcc.exe" `
	-mcpu=cortex-a7 `
	-fpic `
	-ffreestanding `
  --specs=nosys.specs `
	-std=gnu99 `
	-T linkercode.ld  `
	srccode/sample.c `
  -o output/app.elf `
	-O0	`
	-lc -lm -lg -lgcc

Write-Host "Success"

Write-Host "Converting ELF to binary..."

& 'C:\Program Files (x86)\GNU Tools ARM Embedded\8 2018-q4-major\bin\arm-none-eabi-objcopy.exe' output/app.elf -O binary output/app.bin

Write-Host "Writing Dissasembly.."
& 'C:\Program Files (x86)\GNU Tools ARM Embedded\8 2018-q4-major\bin\arm-none-eabi-objdump.exe' -D .\output\app.elf | Out-File -filepath output/app.lss -Encoding ASCII
& 'C:\Program Files (x86)\GNU Tools ARM Embedded\8 2018-q4-major\bin\arm-none-eabi-objdump.exe' -s .\output\app.elf | Out-File -filepath output/app.dump -Encoding ASCII