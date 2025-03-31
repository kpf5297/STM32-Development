using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
    public enum CommandType
    {
        None,
        SetDuty,
        SetRpm,        
        EmergencyStop
    }
}
