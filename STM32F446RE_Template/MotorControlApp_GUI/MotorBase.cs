using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
    public abstract class MotorBase : IMotor
    {
        private string _name;
        protected IMotor inf;

        protected MotorBase(string name)
        {
            _name = name;
        }

        public string Name => _name;

        public void SetDelegate(IMotor motor)
        {
            inf = motor;
        }

        public abstract void Run();
        public abstract void Stop();
        public abstract void EmergencyStop();
        public abstract void SetDuty(float duty);
        public abstract void SetSpeed(int rpm);
    }
}
