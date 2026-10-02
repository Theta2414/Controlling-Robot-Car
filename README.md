# Controlling Robot Car – STM32

A wireless robot car control system implemented using STM32. The project uses an STM32-based transmitter and receiver architecture to control the movement of a robot car remotely. Motion commands are generated from user input and transmitted wirelessly to the robot, where the received commands are processed to control the motors.

The project focuses on embedded system development, wireless communication, sensor interfacing, PWM-based motor control, and real-time control using the STM32 microcontroller.

## 1. Project Overview

This project implements a wireless control system for a robot car using STM32 microcontrollers.

The system is divided into two main units:

- **Transmitter (Tx):** Reads user input and generates movement commands.
- **Receiver (Rx):** Receives the commands wirelessly and controls the robot car motors.

The transmitter and receiver communicate in real time, allowing the user to control the robot car's movement remotely.

The system is designed to support basic vehicle movements such as:

- Forward
- Backward
- Turn left
- Turn right
- Stop

PWM signals are used to control the motor speed, while the motor driver handles the required motor direction and power control.

## 2. Learning Objectives

- Practice embedded system development using STM32.
- Understand wireless communication between two embedded systems.
- Interface sensors and input devices with STM32.
- Understand PWM generation and motor speed control.
- Implement real-time control logic on a microcontroller.
- Apply UART/I2C/GPIO/TIMER peripherals where required by the system.
- Understand the separation between transmitter-side and receiver-side firmware.
- Improve debugging, hardware integration, and system-level development skills.
- Gain practical experience in developing a complete embedded system from input acquisition to actuator control.

## 3. System Architecture

The system consists of two main parts: a transmitter unit and a receiver unit.

![System Architecture](readme_images/architecture-1.png)
*Figure 1. Overall system architecture.*

### 3.1 Transmitter Unit

The transmitter is responsible for obtaining the user's control input and converting it into movement commands.

The general processing flow is:

```text
User Input / Motion Sensor
          ↓
     STM32 Transmitter
          ↓
   Control Processing
          ↓
   Wireless Transmission

```

The transmitter continuously reads the input data, processes the corresponding control information, and sends the resulting command to the receiver.

### 3.2 Receiver Unit

The receiver is installed on the robot car.

It receives the wireless control commands and converts them into motor control signals.

The general processing flow is:

```text
Wireless Receiver
       ↓
STM32 Receiver
       ↓
Command Processing
       ↓
Direction + PWM Generation
       ↓
   Motor Driver
       ↓
    DC Motors

```

This architecture separates user interaction from the motor-control system and allows the robot to be controlled remotely.

## 4. Hardware Overview

The main hardware components used in the project include:

| Component                     | Purpose                    |
| ----------------------------- | -------------------------- |
| STM32F103C8T6                 | Main microcontroller       |
| MPU6050                       | Motion/tilt sensing        |
| Wireless communication module | Transmits control commands |
| Motor driver                  | Drives the DC motors       |
| DC motors                     | Provides robot movement    |
| Robot car chassis             | Mechanical platform        |
| Battery / Power supply        | Provides system power      |
| ST-Link                       | Programming and debugging  |

The STM32F103C8T6 is used as the main processing unit for both the transmitter and receiver sides.

The MPU6050 provides accelerometer and gyroscope measurements that can be processed to estimate the orientation of the transmitter.

## 5. Control Input and Motion Processing

The transmitter uses motion information to determine the desired movement of the robot car.

The MPU6050 provides accelerometer and gyroscope data. These measurements are processed to obtain stable Pitch and Roll angles.

A Kalman filter is used to combine information from the accelerometer and gyroscope and reduce measurement noise.

The general processing chain is:

```text
MPU6050
   ↓
Accelerometer + Gyroscope
   ↓
Sensor Processing
   ↓
Kalman Filter
   ↓
Pitch / Roll Angle
   ↓
Movement Decision
   ↓
Control Command

```

The resulting tilt angle is then mapped to a control value used by the system.

The operating angle is limited to:

```text
-45° ≤ θ ≤ +45°

```

and the corresponding control value is mapped to:

```text
-100 ≤ u ≤ +100

```

A dead zone is also applied around the neutral position to prevent small sensor fluctuations from unintentionally moving the robot.

For example:

```text
        Forward
           ↑
       Positive
           |
Left ←--- 0° ---→ Right
           |
       Negative
           ↓
        Backward

```

The exact movement mapping depends on the control logic implemented in the transmitter firmware.

## 6. Wireless Communication

The transmitter and receiver communicate wirelessly.

The transmitter converts the processed user input into a control packet or command and sends it to the receiver.

The receiver continuously monitors the incoming data and determines the corresponding robot movement.

The communication flow can be represented as:

```text
             TRANSMITTER
                  │
       MPU6050 / User Input
                  │
                  ↓
        Control Processing
                  │
                  ↓
       Wireless Transmission
                  │
            ~~~~~~~~~~~
             Wireless
           Communication
            ~~~~~~~~~~~
                  │
                  ↓
        Wireless Reception
                  │
                  ↓
              RECEIVER
                  │
                  ↓
        Command Processing
                  │
                  ↓
         Motor Control Logic
                  │
                  ↓
            PWM + Direction
                  │
                  ↓
            Motor Driver
                  │
                  ↓
             Robot Car

```

The communication system is designed for real-time command transmission so that changes in the transmitter input are reflected by the robot car with low delay.

## 7. Motor Control

The receiver controls the robot's DC motors through a motor driver.

The direction of each motor is determined by the received movement command, while PWM is used to regulate motor speed.

The basic control structure is:

```text
Received Command
       ↓
Movement Decision
       ↓
Direction Control
       +
PWM Generation
       ↓
Motor Driver
       ↓
DC Motors

```

Different combinations of motor direction and PWM duty cycle are used to achieve forward, backward, left, right, and stop movements.

PWM provides a convenient way to control motor speed while keeping the STM32 in charge of the overall motion-control logic.

## 8. Software Architecture

The firmware is divided into transmitter-side and receiver-side software.

### 8.1 Transmitter Firmware

The transmitter firmware is responsible for:

- Initializing the STM32 peripherals.
- Reading MPU6050 data.
- Processing accelerometer and gyroscope measurements.
- Estimating Pitch and Roll angles.
- Applying the Kalman filter.
- Mapping the resulting angles to control values.
- Applying the control dead zone.
- Generating movement commands.
- Transmitting commands wirelessly.

The simplified software flow is:

```text
System Initialization
        ↓
Read MPU6050
        ↓
Calculate Sensor Data
        ↓
Kalman Filter
        ↓
Calculate Pitch / Roll
        ↓
Angle Mapping
        ↓
Dead Zone
        ↓
Generate Command
        ↓
Transmit Command
        ↓
Repeat

```

The detailed transmitter flowchart is shown below:

![Transmitter Flowchart](readme_images/transmitter_flowchart-1.png)
*Figure 2. Transmitter firmware flowchart.*

### 8.2 Receiver Firmware

The receiver firmware is responsible for:

- Initializing the wireless communication interface.
- Receiving control commands.
- Decoding the received data.
- Determining the required movement.
- Generating motor direction signals.
- Generating PWM signals.
- Driving the motor driver.

The simplified software flow is:

```text
System Initialization
        ↓
Wait for Command
        ↓
Receive Data
        ↓
Decode Command
        ↓
Determine Movement
        ↓
Set Motor Direction
        ↓
Generate PWM
        ↓
Drive Motors
        ↓
Repeat

```

The detailed receiver flowchart is shown below:

![Receiver Flowchart](readme_images/receiver_flowchart-1.png)
*Figure 3. Receiver firmware flowchart.*

## 9. Control Algorithm

The control algorithm converts the measured tilt angle into a motor-control command.

The angle is first limited to the valid operating range:

```text
-45° ≤ θ ≤ +45°

```

Then it is mapped to the control range:

```text
-100 ≤ u ≤ +100

```

A simplified linear mapping can be expressed as:

```text
u = (θ / 45) × 100

```

where:

- `θ` is the measured tilt angle.
- `u` is the resulting control value.

A dead zone is applied around the neutral position:

```text
-5° < θ < +5°

```

When the measured angle is inside this range, the control output is set to zero.

This prevents small sensor noise and minor hand movements from causing unwanted motor movement.

The overall control process is therefore:

```text
Tilt Angle
    ↓
Limit to ±45°
    ↓
Map to ±100
    ↓
Apply Dead Zone
    ↓
Generate Movement Command
    ↓
Wireless Transmission
    ↓
Motor Control

```

## 10. Timing and Real-Time Control

Real-time behavior is important because the robot needs to respond quickly to changes in user input.

The STM32 timer peripherals are used where periodic processing is required.

The control system continuously performs the following operations:

1. Acquire sensor data.
2. Process the sensor measurements.
3. Calculate the current control value.
4. Generate a movement command.
5. Transmit the command.
6. Receive and decode the command.
7. Update motor direction and PWM.

The control loop is designed to run continuously so that the robot can react to changes in the transmitter input.

Timing consistency is especially important for sensor processing and PWM-based motor control.

## 11. Safety and Stop Behavior

A stop condition is included in the control system to prevent the robot from continuing to move when there is no valid movement command.

When the control input is inside the defined neutral region, the motor command is set to zero.

This behavior can be represented as:

```text
          Input
            ↓
      Is command valid?
        /          \
      No            Yes
      ↓              ↓
    STOP        Process command
                     ↓
               Motor Control

```

A communication timeout or invalid-data handling mechanism can also be added to improve the safety of the system.

## 12. Pinout and Hardware Connections

The project uses STM32 GPIO, timer, I2C, and communication interfaces to connect the main components.

The major connections include:

| Interface          | Connected Device | Purpose                            |
| ------------------ | ---------------- | ---------------------------------- |
| I2C                | MPU6050          | Sensor communication               |
| GPIO               | Motor Driver     | Motor direction control            |
| TIMER/PWM          | Motor Driver     | Motor speed control                |
| Wireless Interface | Wireless Module  | Transmitter/receiver communication |
| ST-Link            | STM32            | Programming and debugging          |

A detailed pinout schematic can be added here:

[**Controlling Robot Car – Pinout Schematic**](https://chatgpt.com/c/ADD_PINOUT_LINK_HERE)

## 13. Project Structure

The repository is organized into separate sections for the transmitter, receiver, documentation, presentation, and demonstration materials.

```text
Controlling-Robot-Car/
│
├── Demo/
│   └── Demonstration materials
│
├── Presentation/
│   └── Project presentation
│
├── Report/
│   └── Project report and documentation
│
├── Rx_car/
│   └── Receiver-side firmware
│
├── Tx_car/
│   └── Transmitter-side firmware
│
├── README.md
└── .gitignore

```

The separation between `Tx_car` and `Rx_car` makes it easier to develop, test, and maintain the two sides of the wireless control system independently.

## 14. Demo Video

A demonstration video showing the robot car and its wireless control system:

▶️ [**Controlling Robot Car – Demo Video**](https://chatgpt.com/c/ADD_DEMO_VIDEO_LINK_HERE)

The demonstration shows the response of the robot car to different control inputs and verifies the communication between the transmitter and receiver.

## 15. Build and Flash

The project is developed for the STM32 microcontroller using an STM32-compatible embedded development environment.

Programming and debugging are performed using an ST-Link programmer/debugger.

General development flow:

```text
Write / Modify Firmware
          ↓
        Build
          ↓
    Generate Binary
          ↓
      ST-Link
          ↓
   Flash STM32
          ↓
     Test System

```

The transmitter and receiver firmware are built and programmed separately.

## 15.1 Finite State Machines

The transmitter firmware is organized around initialization, calibration, and active operation states:

![Transmitter FSM](readme_images/transmitter_fsm-1.png)
*Figure 4. Transmitter finite state machine.*

The receiver firmware uses running and fail-safe states to handle normal operation and communication failure:

![Receiver FSM](readme_images/receiver_fsm-1.png)
*Figure 5. Receiver finite state machine.*

## 16. Development Tools

The project uses the following development tools and technologies:

- **MCU:** STM32F103C8T6
- **Programming Language:** C
- **IDE / Toolchain:** STM32 development environment
- **Programmer / Debugger:** ST-Link
- **Communication:** Wireless communication module
- **Sensor Interface:** I2C
- **Motor Control:** GPIO + PWM
- **Version Control:** Git / GitHub

## 17. Challenges and Design Considerations

Several practical challenges were considered during the development of the system.

### Sensor Noise

Raw accelerometer and gyroscope measurements contain noise and may produce unstable angle estimates.

The Kalman filter is therefore used to obtain a more stable estimation of the transmitter orientation.

### Control Sensitivity

Small changes in the measured angle can cause unwanted motor movement.

A dead zone around the neutral position is used to reduce this effect.

### Wireless Communication

The control system requires sufficiently responsive communication between the transmitter and receiver.

The transmitted data must also be interpreted correctly by the receiver to prevent incorrect movement.

### Motor Response

DC motors do not respond perfectly linearly to PWM changes. Factors such as friction, battery voltage, load, and motor characteristics can affect the actual movement of the robot.

Therefore, the PWM control range and movement thresholds may require practical adjustment during testing.

## 18. Limitations and Future Improvements

This project is primarily developed as an embedded systems learning project and therefore has several possible areas for improvement:

- Wireless communication reliability can be further improved.
- Sensor calibration can be enhanced for more consistent angle estimation.
- The control algorithm can be further tuned for smoother robot movement.
- Motor speed synchronization can be improved.
- A more robust communication protocol with packet validation can be implemented.
- Communication timeout and automatic emergency-stop mechanisms can be added.
- Battery voltage monitoring can be added.
- Additional sensors can be integrated for obstacle detection.
- The software architecture can be further modularized for easier maintenance.
- Closed-loop motor control using wheel encoders can be considered for more accurate movement.

Possible future extensions include:

```text
Current System
     │
     ├── Wireless Control
     ├── MPU6050
     ├── PWM Motor Control
     │
     ↓
Future Improvements
     │
     ├── Encoder Feedback
     ├── Closed-loop Control
     ├── Obstacle Detection
     ├── Battery Monitoring
     ├── Improved Communication Protocol
     └── More Advanced Control Algorithms

```

## 19. Conclusion

This project demonstrates the development of a wireless robot car control system using STM32.

The system combines sensor acquisition, motion estimation, wireless communication, command processing, and PWM-based motor control into a complete embedded application.

Through this project, the system provides practical experience in integrating multiple embedded peripherals and developing a real-time control system from the transmitter input to the physical movement of the robot.

## 20. Team Members

This project was developed collaboratively by:

- **Mạch Viễn An**
- **Phạm Đỗ Hồng Phúc**
- **Nguyễn Phúc Khải**
- **Nguyễn Công Tú**