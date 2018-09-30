using System;
using System.Threading;
using System.IO;
using MonoBrick.NXT;//use this to run the example on the NXT

namespace Application
{
    public class NXTOperator 
    {

      public static void Main(string[] args)
      {
         NXTOperator op = new NXTOperator();
         op.sensorLoop();
         op.close();
      }

      public const int LIGHT_LEVEL_TO_STOP = 56;

      private bool careful;
      private bool moving;
      private Brick<NXTLightSensor,NXTLightSensor,Sensor,Sensor> brick;
      private NXTLightSensor light1, light2;
      private bool we_run;
      private readonly Object inMove;

      public NXTOperator()
      {
         careful = true;
         moving = false;
         inMove = new Object();
         Console.SetIn(new StreamReader(Console.OpenStandardInput()));
         brick = new Brick<NXTLightSensor,NXTLightSensor,Sensor,Sensor>("usb");
         brick.Connection.Open();
         light1 = new NXTLightSensor();
         light2 = new NXTLightSensor();
         brick.Sensor1 = light1;
         brick.Sensor2 = light2;
         brick.StopProgram();
         Thread.Sleep(1000);
         brick.StartProgram("senzors.rxe", true);
         we_run = true;
         Thread readThread = new Thread(new ThreadStart(readLoop));
         readThread.Start();
      }

      private void close()
      {
         brick.Connection.Close();
      }

      private void sensorLoop()
      {
         while (we_run)
         { 
           lock(inMove) Monitor.Wait(inMove);

           if (we_run && careful)
           {
              while (moving && (light1.ReadLightLevel() < LIGHT_LEVEL_TO_STOP) &&
                               (light2.ReadLightLevel() < LIGHT_LEVEL_TO_STOP))
              { if (!we_run) break; }

              if (moving) 
              {
                brake();
                Thread.Sleep(300);
                off();
                moving = false;
              }
           }
         }
      }

      private void readLoop()
      {
         while (we_run)
         {
            String cmd = Console.ReadLine();
            process_command(cmd);
         };
      }

      private void process_command(String cmd)
      {
         if (cmd.Equals("On")) on();
         else if (cmd.Equals("Sleep")) sleep();
         else if (cmd.Equals("Off")) off();
         else if (cmd.Equals("Brake")) brake();
         else if (cmd.Equals("Quit")) quit();
         else if (cmd.Equals("Careful")) careful = true;
         else if (cmd.Equals("Careless")) careful = false;
      }

      private void on()
      {
         sbyte arg;
         if (sbyte.TryParse(Console.ReadLine(), out arg))
         {
            brick.MotorA.On(arg);
            brick.MotorB.On(arg);
            brick.MotorC.On(arg);
            moving = true;
            lock(inMove) Monitor.Pulse(inMove);
         }
      }
      
      private void sleep()
      {
         sbyte arg;
         if (sbyte.TryParse(Console.ReadLine(), out arg))
           Thread.Sleep(arg);
      }
      
      private void off()
      { 
         brick.MotorA.Off();
         brick.MotorB.Off();
         brick.MotorC.Off();
         moving = false;
      }

      private void brake()
      {
         brick.MotorA.Brake();
         brick.MotorB.Brake();
         brick.MotorC.Brake();
         moving = false;
      }

      private void quit()
      {
         off();
         we_run = false;
         lock(inMove) Monitor.Pulse(inMove);
      }
    }
}

