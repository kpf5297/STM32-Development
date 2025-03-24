using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
    public class DisconnectedState : IConnectionState
    {
        public void Connect(SerialConnectionContext ctx, string portName)
        {
            ctx.OpenPort(portName);
            if (ctx.Port != null && ctx.Port.IsOpen)
            {
                ctx.ChangeState(new ConnectedState());
            }
        }

        public void Disconnect(SerialConnectionContext ctx)
        {
            Console.WriteLine("[WARNING] Already disconnected.");
        }

        public void SendCommand(SerialConnectionContext ctx, string command)
        {
            Console.WriteLine("[WARNING] Can't send command - not connected!");
        }
    }
}
