using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
    public interface IConnectionState
    {
        void Connect(SerialConnectionContext ctx, string portName);
        void Disconnect(SerialConnectionContext ctx);
        void SendCommand(SerialConnectionContext ctx, string command);
    }
}
