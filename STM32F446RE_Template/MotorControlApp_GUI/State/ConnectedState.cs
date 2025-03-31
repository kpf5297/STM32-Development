using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KofordMotorControlApp
{
    public class ConnectedState : IConnectionState
    {
        public void Connect(SerialConnectionContext ctx, string portName)
        {
            Console.WriteLine("[WARNING] Already connected.");
        }

        public void Disconnect(SerialConnectionContext ctx)
        {
            ctx.ClosePort();
            ctx.ChangeState(new DisconnectedState());
            Console.WriteLine("[INFO] Disconnected from port.");
            
        }

        public void SendCommand(SerialConnectionContext ctx, string command)
        {
            if (ctx.Port != null && ctx.Port.IsOpen)
            {
                try
                {
                    ctx.Port.WriteLine(command);
                    Console.WriteLine($"[TX] {command}");
                }
                catch (System.Exception)
                {
                    Console.WriteLine("[ERROR] Failed to send command.");
                    throw;
                }
                
            }
            else
            {
                Console.WriteLine("[ERROR] Port is not open??");
            }
        }
    }
}
