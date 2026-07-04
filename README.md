# cy4611
USB2ATA - USB2IDE - USB2CF

Copy from http://www.siphec.com/project/USB2ATA/index.html

Directory `cy4611` contains content of `cy4611___high_speed_usb_to_ata_15.zip`

Directory `cy4611b` contains content of `cy4611b_AT2_PINOUT.zip`


## ToolChain:
    Connect hard disk with ribbon cable to IDE port of FX2LP Board.
    Download the firmware:
    Choose between the following options:
    Option 1:
    Download the firmware "cy4611___high_speed_usb_to_ata_15.zip". Use this local copy because the file is now longer available from the Cypress website.
    The target file for the EEPROM upload is "fx2_ata.iic".
    Option 2 (newer version):
    Download "FX2LP_ATA.zip" (~20 MByte) for CY4611b from cypress.com.
    Here the target files are "cy4611b_AT2_PINOUT.iic" or "cy4611b_AT2_PINOUT_STANDBY.iic". (*.iic files are in cy4611b_AT2_PINOUT.zip)
    The standby version powers the device, e.g. hard disk, down after plug off from usb line.
    Unzip the files.
    Unplug EEPROM jumper (open).
    Connect board via USB cable to the PC.
    Load Cypress Tool "CyConsole" from development kit
    Plug EEPROM jumper (shorten).
    Program e.g. upload "fx2_ata.iic" or "cy4611b_AT2_PINOUT_STANDBY.iic" to EEPROM using "EZ-USB interface" application (button "Lg EEPROM")
    Disconnect and reconnect USB cable of board or just press RESET button.
    Only if there is a HDD connected (as master) to the board, the onboard red hard disk led flashes shortly and a new disk drive is available and you can access the disk contents via Explorer or any other file manager
    If you have Windows98 or MacOS you need to download a system driver from Cypress!

