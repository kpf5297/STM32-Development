using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
    public interface IMotorState
    {
        void Run(BlowerMotor blower);
        void Stop(BlowerMotor blower);
        void EmergencyStop(BlowerMotor blower);
    }
}
