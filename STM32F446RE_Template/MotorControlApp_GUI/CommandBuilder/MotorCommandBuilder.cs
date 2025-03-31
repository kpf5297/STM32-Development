using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
    public class MotorCommandBuilder
    {
        private CommandType _cmdType;
        private float _value;

        public MotorCommandBuilder SetCommandType(CommandType cmdType)
        {
            _cmdType = cmdType;
            return this;
        }

        public MotorCommandBuilder SetValue(float value)
        {
            _value = value;
            return this;
        }

        public string Build()
        {
            switch (_cmdType)
            {
                case CommandType.SetDuty:
                    return $"D{_value:F1}";
                case CommandType.SetRpm:
                    return $"R{(int)_value}";
                case CommandType.EmergencyStop:
                    return "ESTOP";
                default:
                    return string.Empty;
            }
        }
    }
}
