## Needed to debug

In .vscode folder launch.json:
```bash
        {
            "name": "Debug with OpenOCD",
            "cwd": "${workspaceFolder}",
            "type": "cortex-debug",
            "executable": "${command:cmake.launchTargetPath}",
            // Let CMake extension decide executable: "${command:cmake.launchTargetPath}"       
            // Or fixed file path: "${workspaceFolder}/path/to/filename.elf"
            "request": "launch",
            "servertype": "openocd",
            "openOCDPath": "/opt/homebrew/bin/openocd",
            "configFiles": [
                "interface/stlink.cfg",
                // "target/stm32f4xx.cfg", for STM32F4xx series
                // "target/stm32f7x.cfg", for STM32F7xx series
                // "target/stm32h7x.cfg", for STM32H7xx series
                "target/stm32f1x.cfg"
            ]
        }
```
