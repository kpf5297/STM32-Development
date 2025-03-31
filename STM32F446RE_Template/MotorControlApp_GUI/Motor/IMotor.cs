using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
    public interface IMotor
    {
        void Run();                // from Stopped -> Running
        void Stop();               // from Running -> Stopped
        void EmergencyStop();      // immediate stop
        void SetDuty(float duty);  // 0..100
        void SetSpeed(int rpm);    // (for demo, 1:1 => duty)
    }
}
