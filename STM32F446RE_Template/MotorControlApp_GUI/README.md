# Koford Motor Control App

## Overview
The **Koford Motor Control App** is a Windows-based application built using C# and Windows Forms. It provides an interface for communicating with and controlling a Koford Motor Controller via a serial connection (COM port). The app supports functionalities like starting/stopping the motor, setting duty cycle and speed, running in a triangle wave pattern, and visual status indicators.

---

## Features
### 1. Connection Controls
- Select available COM ports.
- Refresh list of available ports.
- Connect to or disconnect from a motor controller.
- Visual indicator showing connection status (Green: Connected, Red: Disconnected).

### 2. Motor Control
- **Run / Stop buttons** for motor control.
- Visual indicator showing motor status (Green: Running, Red: Stopped).

### 3. Triangle Wave Control
- Provides a triangular wave control mechanism for automated speed variation.
- Allows configuration of:
  - **Minimum Duty**
  - **Maximum Duty**
  - **Period** (Duration of the full cycle).
- **Start / Stop button** to begin or end the triangle wave operation.

### 4. Manual Control
- Manually set:
  - **Duty Cycle** (0 - 100)
  - **Speed (RPM)**
- Buttons to apply settings while the motor is running.

---

## Installation
1. Clone the repository to your local machine.
2. Open the solution in Visual Studio.
3. Restore NuGet packages if needed (e.g., `Newtonsoft.Json` for configuration handling).
4. Build and run the application.

---

## Usage
1. **Select COM Port** from the dropdown.
2. Click **Refresh Ports** to list available ports.
3. Click **Connect** to establish connection (Visual indicator will turn Green if successful).
4. Use the **Run / Stop buttons** to control motor operation.
5. Use **Set Duty / Set Speed buttons** for manual control.
6. Configure **Triangle Wave Control** settings and click **Start Triangle Wave** to initiate automated control.

---

## Configuration File (config.json)
This file allows pre-configuration of:
- `MinDuty`: Minimum duty cycle for triangle wave.
- `MaxDuty`: Maximum duty cycle for triangle wave.
- `Period`: Duration of the triangle wave cycle.
- `AccelerationRate`: Rate of speed increase for the motor.

---

## Dependencies
- `System.IO.Ports`
- `Newtonsoft.Json` (Ensure package is installed via NuGet).

---

## Future Improvements
- Adding PID control interface if needed.
- More precise speed handling via embedded control.
- Improving the layout and responsiveness of the UI.

---

## License
This project is licensed under the MIT License. See the `LICENSE` file for more information.

## UML
```mermaid
classDiagram
    %% Classes

    class MainForm {
        - BlowerMotor blower
        - SerialConnectionContext connection
        - bool motorRunning
        - bool isConnected
        - bool isLooping
        - Thread loopThread
        - float minDuty
        - float maxDuty
        - int period
        - float accelerationRate
        - Panel connectionStatusPanel, motorStatusPanel
        - Label connectionStatusLabel, motorStatusLabel
        - ComboBox comPortComboBox
        - TextBox dutyTextBox, speedTextBox, minDutyTextBox, maxDutyTextBox, periodTextBox
        - const string ConfigFile
        + MainForm()
        + RefreshComPorts()
        + LoadConfig()
        + UpdateMotorStatus(bool running)
        + UpdateConnectionStatus(bool connected)
        + TriangleWaveControl(object sender, EventArgs e)
        + SetDuty(object sender, EventArgs e)
        + SetSpeed(object sender, EventArgs e)
    }

    class BlowerMotor {
        - IMotorState _currentState
        - SerialConnectionContext _connection
        + BlowerMotor(string name, SerialConnectionContext connection)
        + Run()
        + Stop()
        + EmergencyStop()
        + SetDuty(float duty)
        + SetSpeed(int rpm)
        + SetState(IMotorState newState)
    }

    class SerialConnectionContext {
        - IConnectionState _state
        - SerialPort Port
        + SerialConnectionContext()
        + Connect(string portName)
        + Disconnect()
        + SendCommand(string cmd)
        + ChangeState(IConnectionState newState)
        + OpenPort(string portName)
        + ClosePort()
    }

    class MotorCommandBuilder {
        - CommandType _cmdType
        - float _value
        + MotorCommandBuilder()
        + SetCommandType(CommandType cmdType)
        + SetValue(float value)
        + Build() : string
    }

    class CommandType {
        <<enumeration>>
        None
        SetDuty
        SetRpm
        EmergencyStop
    }

    class IMotorState {
        <<interface>>
        + Run(BlowerMotor blower)
        + Stop(BlowerMotor blower)
        + EmergencyStop(BlowerMotor blower)
    }

    class RunningState {
        + Run(BlowerMotor blower)
        + Stop(BlowerMotor blower)
        + EmergencyStop(BlowerMotor blower)
    }

    class StoppedState {
        + Run(BlowerMotor blower)
        + Stop(BlowerMotor blower)
        + EmergencyStop(BlowerMotor blower)
    }

    class IConnectionState {
        <<interface>>
        + Connect(SerialConnectionContext ctx, string portName)
        + Disconnect(SerialConnectionContext ctx)
        + SendCommand(SerialConnectionContext ctx, string command)
    }

    class ConnectedState {
        + Connect(SerialConnectionContext ctx, string portName)
        + Disconnect(SerialConnectionContext ctx)
        + SendCommand(SerialConnectionContext ctx, string command)
    }

    class DisconnectedState {
        + Connect(SerialConnectionContext ctx, string portName)
        + Disconnect(SerialConnectionContext ctx)
        + SendCommand(SerialConnectionContext ctx, string command)
    }

    %% Relationships
    MainForm --> BlowerMotor
    MainForm --> SerialConnectionContext
    BlowerMotor --> SerialConnectionContext
    BlowerMotor --> IMotorState
    IMotorState <|-- RunningState
    IMotorState <|-- StoppedState
    SerialConnectionContext --> IConnectionState
    IConnectionState <|-- ConnectedState
    IConnectionState <|-- DisconnectedState
    BlowerMotor --> MotorCommandBuilder
    MotorCommandBuilder --> CommandType
    ```