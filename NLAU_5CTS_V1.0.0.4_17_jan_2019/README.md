# Thermal Canyon 

Version 1.00

## tempsensor_v1
Source code for Thermal Canyon for the MSP430FR5969 microprocessor.

## Compiling and running

### Compiling
Requires the CCS IDE from Texas Instruments using a full license which exposes the TI optimized compiler. The standard GCC compiler will result in a code size too large to fit onto the MSP430FR5969. The CCS project settings included in this repository will load the required settings to run best on the Thermal Canyon device. Any changes made to the project settings may result in incorrect behaviour.

### Running
The binaries can be loaded with a JTAG interface connected to the board. Using CCS the code can be run simply by pressing the 'Run' button located in the toolbar or the equivalent keyboard shortcut. Other JTAG flashing software may be used to load and run the binaries. Please check the documentation for these alternatives to ensure that the MSP430 microprocessor family is supported and to get instructions for flashing.

Another method as opposed to flashing using a JTAG interface would be to use a BSL interface instead. Connect the BSL interface to the header on the rear of the board ensuring pin 1 on the board is aligned with pin 1 on the interface. Using BSL Scripter (Software and documentation available [here](http://www.ti.com/tool/MSPBSL)) a selection of options is provided to interact with the board in a similar way to JTAG. This includes and is not limited to erasing the device's memory and writing a new binary file to the device. NOTE: This has only been tested using the MSP430-BSL by Olimex.

#### Recommended BSL steps for flashing software
* Create a text file with the follwing lines:
```
MODE 5XX USB
MASS_ERASE
RX_DATA_BLOCK binary_file.txt
```
Where 'binary_file.txt' is the TI TXT output from CCS. In order to create a TI TXT file along with the standard output file then it must be enabled in CCS. Right click project, then check the box at *Properties->Build->MSP430 Hex Utility->Enable MSP430 Hex Utility*
* Connect the BSL interface to the board and your computer and ensure the that the header labelled 'J16' is shorted throughout the entire process. (Must be done on a machine running Windows)
* Switch the device on.
* Run BSL Scripter with the text file you just created as an argument in command prompt, this can be made into a batch file to avoid using command prompt. 
```
BSL_Scripter.exe <path/to/file.txt>
```
* Disconnect the header and BSL interface then proceed to reboot the device.

# Terms and conditions 
Copyright (c) 2015, Intel Corporation. All rights reserved.
INFORMATION IN THIS DOCUMENT IS PROVIDED IN CONNECTION WITH INTEL PRODUCTS. NO LICENSE, EXPRESS OR IMPLIED,
BY ESTOPPEL OR OTHERWISE, TO ANY INTELLECTUAL PROPERTY RIGHTS IS GRANTED BY THIS DOCUMENT. EXCEPT AS PROVIDED
IN INTEL'S TERMS AND CONDITIONS OF SALE FOR SUCH PRODUCTS, INTEL ASSUMES NO LIABILITY WHATSOEVER, AND INTEL
DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTY, RELATING TO SALE AND/OR USE OF INTEL PRODUCTS INCLUDING LIABILITY
OR WARRANTIES RELATING TO FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABILITY, OR INFRINGEMENT OF ANY PATENT,
COPYRIGHT OR OTHER INTELLECTUAL PROPERTY RIGHT.
UNLESS OTHERWISE AGREED IN WRITING BY INTEL, THE INTEL PRODUCTS ARE NOT DESIGNED NOR INTENDED FOR ANY APPLICATION
IN WHICH THE FAILURE OF THE INTEL PRODUCT COULD CREATE A SITUATION WHERE PERSONAL INJURY OR DEATH MAY OCCUR.
 
Intel may make changes to specifications and product descriptions at any time, without notice.
Designers must not rely on the absence or characteristics of any features or instructions marked
"reserved" or "undefined." Intel reserves these for future definition and shall have no responsibility
whatsoever for conflicts or incompatibilities arising from future changes to them. The information here
is subject to change without notice. Do not finalize a design with this information.
The products described in this document may contain design defects or errors known as errata which may
cause the product to deviate from published specifications. Current characterized errata are available on request.
 
This document contains information on products in the design phase of development.
All Thermal Canyons featured are used internally within Intel to identify products
that are in development and not yet publicly announced for release.  Customers, licensees
and other third parties are not authorized by Intel to use Thermal Canyons in advertising,
promotion or marketing of any product or services and any such use of Intel's internal
Thermal Canyons is at the sole risk of the user.
