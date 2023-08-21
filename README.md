# Underwater Sensor Node with Single Pair Ethernet
Prototype project to build a underwater sensor node, powered by and communicating with the surface via Single Pair Ethernet (10BASE-T1L + 802.3cg Type E Class 10-12).

TODO IMG

A more detailed write-up can be [read here]().



# Mechanical



## Housing

The body of the sensor node is a 2″ acrylic tube (*~58mm OD*), with a pair of aluminium end-caps. This provides a working volume constrained by the 50 mm internal diameter and ~118 mm maximum gap between caps.

TODO IMG/DRAWING

The top cap/bulkhead has 4×M10 threaded holes for accessories as part of BlueRobotic's hardware ecosystem. The assembled prototype is fitted with:

- 4.5mm Wetlink penetrator for the tether cable
- 10m rated pressure sensor
- White indicator LED
- Pressure relief/test valve

Whilst this prototype won't see properly deep water, this tube arrangement should be capable of  200m operation, and far deeper if an aluminium sleeve is used.

CAD models and assemblies are in `/mechanical/tube/`. Off-the-shelf parts are in `/mechanical/tube/cots/`.

## Internal brackets

A pair of 3D printed board mounts use the 4×M3 internal mounting holes in the bulkheads. The top mount abuses the solderable standoffs on the rear of the backplane board to mount, along with a key in the PCB to force alignment.

TODO IMG

The bottom mount is a tapered groove which receives the PCB.

CAD models and STL files are in `/mechanical/tube/`. 

## Mounting

The tube itself doesn't have any features for mounting, so a pair of custom tube clamps were printed with the general goal of providing generic strap or wire mounting points. 

TODO IMG

Files are in `/mechanical/mount/`.



# Electronics

Compatibility with the [Sparkfun MicroMod ecosystem](https://www.sparkfun.com/micromod) has been maintained where possible - building off of [Sparkfun's ADIN1110 function board](https://www.sparkfun.com/products/19038) for bring-up and early software tests.

PCB's were designed with KiCAD 7. Project files, gerbers and schematic PDF's are in `/electronics/`.

## Backplane

Acting as the MicroMod 'carrier', this board acts as the tube's electrical spine - providing the processor and function board connections, as well as regulation, I2C headers, and breakout pads for spare/conflicted IO.

TODO IMG

It's a two-layer 46 x 100 mm 1.6 mm 2-layer board with pretty relaxed fabrication requirements. 



## PD Function Board

This board implements the ADIN1110 SPE transceiver in a similar manner to Sparkfun's function board, but improves over the dev-kit by adding the necessary circuitry to act as a Class 10-12 PoDL (Power over Data Line) powered device aka PD.

TODO IMG

When the upstream source enables it's detection current source, the PD board provides a signature voltage in the 4.05-4.55V range which the source can use to check for a compliant PD before enabling the full bus voltage.

> This board only provides detection, not classification via SCCP, and as such runs in 'fast-start mode'.

Once the supply voltage is within spec, the onboard switch automatically turns on and allows the wide-input 5V bulk regulator to operate. The function board allows back-powering the carrier board, similar to Sparkfun's W5500 PoE board.

Also, a current shunt and amplifier allow for sink-side current measurement on the 5V rail.

## PSE Function Board

This board shares it's input coupling network and ADIN1110 design with the PD board, but replaces the detection signalling circuitry and bulk regulator with the necessary circuitry to test and detect valid downstream devices along with a switchable HV input.

TODO: IMG

By sourcing a constant current of 9-16mA with an open-loop voltage between 4.75-5.55V, a compliant downstream device can be detected by measuring 4.05-4.55V on the supply lines.

> This board only supports downstream PD devices which support detection, and does not implement classification via SCCP.  

Unlike the PD side, the PSE side requires a microcontroller to enable the current source, read detection status, and control the HV power switch.

Additionally, maintain full voltage signature (MVFS) behaviour needs the microcontroller to monitor the current sense signal for a minimum peak load of 11mA per 10ms span to keep the power supply active (though this can be set lower if needed).

As an aside, this PSE design probably makes more sense as it's own carrier board, or as a media converter to 10BaseT ethernet. Implementing it as a function board allowed for design re-use and opens up some flexibility in final use-cases though.

## Sensors

The project was designed without assuming any specific sensors would be used, but I used these I2C sensors:

- Temperature & Humidity - `SHTC3` via [Sparkfun `SEN-16467`](https://www.sparkfun.com/products/16467)
- Motion - `LSM6DSO` via [Sparkfun `SEN-18020`](https://www.sparkfun.com/products/18020)
- Depth/Pressure - `MS5837` via [BlueRobotics `Bar02`](https://bluerobotics.com/store/sensors-cameras/sensors/bar02-sensor-r1-rp/)

The Sparkfun breakouts mount directly to the standoffs on the backplane.

The `Bar02` comes with a JST-GH connector, so I re-terminated it with a JST-SH connector for consistency with the other boards and headers on the backplane PCB.

# Firmware





# User Interface

The user interface is built with [Electric UI](https://electricui.com), and can be built and run by invoking `arc start` from `/interface`.

TODO: IMG

It handles visualisation of data streaming off the sensor node, and provides additional statistics and logging.



