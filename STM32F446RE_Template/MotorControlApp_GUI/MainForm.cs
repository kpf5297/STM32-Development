﻿using System;
using System.Drawing;
using System.IO.Ports;
using System.Windows.Forms;
using System.Threading;
using System.IO;
using Newtonsoft.Json;

namespace KofordMotorControlApp
{

    public partial class MainForm : Form
    {
        private BlowerMotor blower;
        private SerialConnectionContext connection;
        private bool motorRunning = false; // Tracks if motor is running
        private bool isConnected = false; // Tracks if connection is established
        private bool isLooping = false; // Tracks if Triangle Wave control is active
        private Thread loopThread; // Thread for handling triangle wave control
        private float minDuty = 5.0f;
        private float maxDuty = 45.0f;
        private int period = 5000; // Period of the triangle wave in milliseconds
        private float accelerationRate = 1000f; // RPM per second
        private Panel connectionStatusPanel;
        private Label connectionStatusLabel;
        private Panel motorStatusPanel;
        private Label motorStatusLabel;
        private ComboBox comPortComboBox;
        private TextBox dutyTextBox, speedTextBox, minDutyTextBox, maxDutyTextBox, periodTextBox;
        private const string ConfigFile = "config.json";

        public MainForm()
        {
            SetupUI();
            LoadConfig();
            connection = new SerialConnectionContext();
            blower = new BlowerMotor("Koford CPAP Blower", connection);
            RefreshComPorts();
        }

        private void RefreshComPorts()
        {
            // Disconnect if connected
            if (isConnected)
            {
                connection.Disconnect();
                UpdateConnectionStatus(false);
            }

            comPortComboBox.Items.Clear();
            comPortComboBox.Items.Add("Select COM Port");
            comPortComboBox.SelectedIndex = 0;
            string[] ports = SerialPort.GetPortNames();
            foreach (string port in ports)
            {
                comPortComboBox.Items.Add(port);
            }
            if (ports.Length > 0)
            {
                comPortComboBox.SelectedIndex = 1; // Select the first available port
            }
        }

        private void LoadConfig()
        {
            if (File.Exists(ConfigFile))
            {
                var configJson = File.ReadAllText(ConfigFile);
                var config = JsonConvert.DeserializeObject<dynamic>(configJson);

                minDuty = (float)config.MinDuty;
                maxDuty = (float)config.MaxDuty;
                period = (int)config.Period;
                accelerationRate = (float)config.AccelerationRate;

                MessageBox.Show("Settings Loaded Successfully!");
            }
        }

        private void UpdateMotorStatus(bool running)
        {
            motorRunning = running;
            motorStatusLabel.Text = running ? "Running" : "Stopped";
            motorStatusPanel.BackColor = running ? Color.LightGreen : Color.LightCoral;
            UpdateUIState();
        }

        private void UpdateConnectionStatus(bool connected)
        {
            isConnected = connected;
            connectionStatusLabel.Text = connected ? "Connected" : "Disconnected";
            connectionStatusPanel.BackColor = connected ? Color.LightGreen : Color.LightCoral;
            UpdateUIState();
        }

        private void TriangleWaveControl(object sender, EventArgs e)
        {
            if (!isConnected)
            {
                MessageBox.Show("Please connect to the motor first.");
                return;
            }
            if (!isLooping)
            {
                if (float.TryParse(minDutyTextBox.Text, out minDuty) &&
                    float.TryParse(maxDutyTextBox.Text, out maxDuty) &&
                    int.TryParse(periodTextBox.Text, out period))
                {
                    isLooping = true;
                    loopThread = new Thread(TriangleWaveLoop) { IsBackground = true };
                    loopThread.Start();
                    (sender as Button).Text = "Stop Triangle Wave";
                }
                else
                {
                    MessageBox.Show("Invalid inputs for Triangle Wave control.");
                }
            }
            else
            {
                isLooping = false;
                loopThread?.Join();
                (sender as Button).Text = "Start Triangle Wave";
            }

            UpdateUIState();
        }

        private void TriangleWaveLoop()
        {
            while (isLooping)
            {
                for (float duty = minDuty; duty <= maxDuty && isLooping; duty += 1)
                {
                    blower.SetDuty(duty);
                    Thread.Sleep(period / (int)(maxDuty - minDuty) / 2);
                }
                for (float duty = maxDuty; duty >= minDuty && isLooping; duty -= 1)
                {
                    blower.SetDuty(duty);
                    Thread.Sleep(period / (int)(maxDuty - minDuty) / 2);
                }
            }
        }

        private void SetDuty(object sender, EventArgs e)
        {
            if (motorRunning && float.TryParse(dutyTextBox.Text, out float duty))
            {
                blower.SetDuty(duty);
                MessageBox.Show($"Duty cycle set to {duty}% successfully.");
            }
            else
            {
                MessageBox.Show("Motor must be running to set duty cycle.");
            }
        }

        private void SetSpeed(object sender, EventArgs e)
        {
            if (motorRunning && int.TryParse(speedTextBox.Text, out int speed))
            {
                blower.SetSpeed(speed);
                MessageBox.Show($"Speed set to {speed} RPM successfully.");
            }
            else
            {
                MessageBox.Show("Motor must be running to set speed.");
            }
        }

        private void UpdateUIState()
        {
            bool canControl = isConnected && motorRunning;
            bool triangleOn = isLooping;

            // COM Port controls - lock while connected
            comPortComboBox.Enabled = !isConnected;

            // Triangle Wave Controls
            minDutyTextBox.Enabled = canControl;
            maxDutyTextBox.Enabled = canControl;
            periodTextBox.Enabled = canControl;

            // Find triangle wave button and disable
            foreach (Control c in minDutyTextBox.Parent.Controls)
            {
                if (c is Button btn && btn.Text.Contains("Triangle"))
                {
                    btn.Enabled = canControl;
                }
            }

            // Manual Controls (duty/speed inputs + buttons)
            bool manualEnabled = canControl && !triangleOn;
            dutyTextBox.Enabled = manualEnabled;
            speedTextBox.Enabled = manualEnabled;

            foreach (Control c in dutyTextBox.Parent.Controls)
            {
                if (c is Button btn && (
                    btn.Text == "Set Duty" ||
                    btn.Text == "Set Speed" ||
                    btn.Text == "+1" ||
                    btn.Text == "-1"))
                {
                    btn.Enabled = manualEnabled;
                }
            }
        }

        private void SetupUI()
        {
            this.Text = "Koford Motor Control App";
            this.Size = new Size(1200, 300);
            this.Font = new Font("Segoe UI", 12);
            this.FormBorderStyle = FormBorderStyle.Sizable;

            var mainPanel = new Panel { Location = new Point(10, 10), Size = new Size(1160, 230), BorderStyle = BorderStyle.FixedSingle };

            // Connection Controls Group
            var connectionGroup = new GroupBox { Text = "Connection Controls", Location = new Point(10, 10), Size = new Size(300, 180) };
            comPortComboBox = new ComboBox { Location = new Point(10, 30), Size = new Size(150, 30) };
            comPortComboBox.DropDownStyle = ComboBoxStyle.DropDownList;
            comPortComboBox.Items.Add("Select COM Port");
            comPortComboBox.SelectedIndex = 0;
            comPortComboBox.SelectedIndexChanged += (s, e) =>
            {
                if (comPortComboBox.SelectedItem.ToString() != "Select COM Port")
                {
                    connection.Disconnect();
                    UpdateConnectionStatus(false);
                }
            };
            comPortComboBox.Items.AddRange(SerialPort.GetPortNames());
            var refreshPortsButton = new Button { Text = "Refresh Ports", Location = new Point(170, 30), Size = new Size(120, 30) };
            refreshPortsButton.Click += (s, e) => RefreshComPorts();

            var connectButton = new Button { Text = "Connect", Location = new Point(10, 70), Size = new Size(100, 30) };
            connectButton.Click += (s, e) =>
            {
                if (comPortComboBox.SelectedItem != null)
                {
                    connection.Connect(comPortComboBox.SelectedItem.ToString());
                    UpdateConnectionStatus(true);
                }
            };

            var disconnectButton = new Button { Text = "Disconnect", Location = new Point(120, 70), Size = new Size(100, 30) };
            disconnectButton.Click += (s, e) =>
            {
                if (isLooping)
                {
                    isLooping = false;
                    loopThread?.Join();
                }
                blower.Stop();
                UpdateMotorStatus(false);

                connection.Disconnect();
                UpdateConnectionStatus(false);
            };

            connectionStatusPanel = new Panel { Location = new Point(10, 110), Size = new Size(150, 40), BackColor = Color.LightCoral };
            connectionStatusLabel = new Label { Text = "Disconnected", Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleCenter, Font = new Font("Segoe UI", 10, FontStyle.Bold) };
            connectionStatusPanel.Controls.Add(connectionStatusLabel);

            connectionGroup.Controls.Add(comPortComboBox);
            connectionGroup.Controls.Add(refreshPortsButton);
            connectionGroup.Controls.Add(connectButton);
            connectionGroup.Controls.Add(disconnectButton);
            connectionGroup.Controls.Add(connectionStatusPanel);

            // Motor Control Group
            var motorControlGroup = new GroupBox { Text = "Motor Control", Location = new Point(320, 10), Size = new Size(300, 180) };
            var runButton = new Button { Text = "Run", Location = new Point(10, 30), Size = new Size(80, 30) };
            runButton.Click += (s, e) => { blower.Run(); UpdateMotorStatus(true); };

            var stopButton = new Button { Text = "Stop", Location = new Point(100, 30), Size = new Size(80, 30) };
            stopButton.Click += (s, e) => { blower.Stop(); UpdateMotorStatus(false); };

            motorStatusPanel = new Panel { Location = new Point(10, 80), Size = new Size(150, 40), BackColor = Color.LightCoral };
            motorStatusLabel = new Label { Text = "Stopped", Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleCenter, Font = new Font("Segoe UI", 10, FontStyle.Bold) };
            motorStatusPanel.Controls.Add(motorStatusLabel);

            motorControlGroup.Controls.Add(runButton);
            motorControlGroup.Controls.Add(stopButton);
            motorControlGroup.Controls.Add(motorStatusPanel);

            // Triangle Wave Control Group
            var triangleWaveGroup = new GroupBox { Text = "Triangle Wave Control", Location = new Point(630, 10), Size = new Size(300, 180) };
            minDutyTextBox = new TextBox { Location = new Point(10, 30), Size = new Size(80, 30), Text = "5.0" };
            maxDutyTextBox = new TextBox { Location = new Point(100, 30), Size = new Size(80, 30), Text = "45.0" };
            periodTextBox = new TextBox { Location = new Point(190, 30), Size = new Size(80, 30), Text = "5000" };

            var triangleWaveButton = new Button { Text = "Start Triangle Wave", Location = new Point(10, 70), Size = new Size(180, 40) };
            triangleWaveButton.Click += TriangleWaveControl;

            triangleWaveGroup.Controls.Add(minDutyTextBox);
            triangleWaveGroup.Controls.Add(maxDutyTextBox);
            triangleWaveGroup.Controls.Add(periodTextBox);
            triangleWaveGroup.Controls.Add(triangleWaveButton);

            // Manual Control Group
            var manualControlGroup = new GroupBox { Text = "Manual Control", Location = new Point(940, 10), Size = new Size(200, 180) };
            dutyTextBox = new TextBox { Location = new Point(10, 30), Size = new Size(80, 30), Text = "0.0" };
            speedTextBox = new TextBox { Location = new Point(10, 70), Size = new Size(80, 30), Text = "0" };

            var setDutyButton = new Button { Text = "Set Duty", Location = new Point(100, 30), Size = new Size(80, 30) };
            setDutyButton.Click += SetDuty;

            var setSpeedButton = new Button { Text = "Set Speed", Location = new Point(100, 70), Size = new Size(80, 30) };
            setSpeedButton.Click += SetSpeed;

            var increaseDutyButton = new Button { Text = "+1", Location = new Point(10, 110), Size = new Size(80, 30) };
            
            increaseDutyButton.Click += (s, e) =>
            {
                if (float.TryParse(dutyTextBox.Text, out float duty))
                {
                    duty = Math.Min(duty + 1, 100); // Clamp to 100%
                    dutyTextBox.Text = duty.ToString("F1");
                    if (motorRunning) blower.SetDuty(duty);
                }
            };

            var decreaseDutyButton = new Button { Text = "-1", Location = new Point(100, 110), Size = new Size(80, 30) };
            decreaseDutyButton.Click += (s, e) =>
            {
                if (float.TryParse(dutyTextBox.Text, out float duty))
                {
                    duty = Math.Max(duty - 1, 0); // Clamp to 0%
                    dutyTextBox.Text = duty.ToString("F1");
                    if (motorRunning) blower.SetDuty(duty);
                }
            };


            manualControlGroup.Controls.Add(dutyTextBox);
            manualControlGroup.Controls.Add(speedTextBox);
            manualControlGroup.Controls.Add(setDutyButton);
            manualControlGroup.Controls.Add(setSpeedButton);
            manualControlGroup.Controls.Add(increaseDutyButton);
            manualControlGroup.Controls.Add(decreaseDutyButton);


            mainPanel.Controls.Add(connectionGroup);
            mainPanel.Controls.Add(motorControlGroup);
            mainPanel.Controls.Add(triangleWaveGroup);
            mainPanel.Controls.Add(manualControlGroup);
            this.Controls.Add(mainPanel);
        }


    }
}
