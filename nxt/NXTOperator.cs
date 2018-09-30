using System;
using System.Threading;
using MonoBrick.NXT;//use this to run the example on the NXT

namespace Application
{
    public static class NXTOperator 
    {
      public static void Main(string[] args)
      {
            var brick = new Brick<Sensor,Sensor,Sensor,Sensor>("usb");
            brick.Connection.Open();
            bool we_run = true;
            do {
              String cmd = Console.ReadLine();
              if (cmd.Equals("On"))
              {
                sbyte arg;
                if (sbyte.TryParse(Console.ReadLine(), out arg))
                {
                  brick.MotorA.On(arg);
                  brick.MotorB.On(arg);
                  brick.MotorC.On(arg);
                }
              }
              else if (cmd.Equals("Off"))
              {
                brick.MotorA.Off();
                brick.MotorB.Off();
                brick.MotorC.Off();
              }
              else if (cmd.Equals("Brake"))
              {
                brick.MotorA.Brake();
                brick.MotorB.Brake();
                brick.MotorC.Brake();
              }
              else if (cmd.Equals("Quit"))
                we_run = false;
            } while (we_run);
            brick.Connection.Close();
      }
    }
}

