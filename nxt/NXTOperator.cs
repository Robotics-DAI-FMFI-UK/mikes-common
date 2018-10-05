using System;
using System.Threading;
using System.IO;
using MonoBrick.NXT;

// a mediator between our system and NXT implemented using MonoBrick library

// external protocol (standard input):
//
//  Dock  - bring the grabber home
//  Line1 - extend the grabber over the first row of balls
//  Line2 - extend the grabber over the second row of balls
//  Line3 - extend the grabber over the third row of balls
//  On - start rotating the wheels
//  Off - stop rotating the wheels
//  Fast - set grabber movement speed to fast
//  Slow - set grabber movement speed to slow
//  Quit - close communication with NXT and quit
//  Key - read NXT button key
//  Sleep - wait (for script-testing purposes, millisec taken from next line)
//  Clear - clear display
//  <any_other_msg> - send msg to NXT
//
// responses (standard output):
//
//  Start - after NXT has been started by the user
//  Done - after position has been reached
//  Off - for each command request when NXT is not running
//  Stop - NXT has been stopped by the user
//  <Key> - Left, Right, Enter

// on NXT mikes.rxe program should be downloaded in advance,
// NXTOperator communicates with NXT using the following communication protocol:
//
// Pi->NXT / NXT->Pi
//  'A'  return grabber completely in / sensor 4 light on when ready
//  'B'  extend grabber completely out / sensor 4 light on when ready
//  'C'  extend grabber out till first mark / sensor 1 light on when ready
//  'D'  extend grabber out till second mark / sensor 1 light on when ready
//  'E'  set slow speed
//  'F'  set high speed
//  'M'  read key and send back / key entered (1, 2, or 3)
//  'O'  acknowledge key has been read
//  'X'  clear display
//
// example (command 'Line3'):
//  Pi->NXT: 'G'
//  Pi reset sensor 4 (now it should read 0)
//  Pi->NXT: 'B'
//  Pi reads sensor 4 until the value > 0
//  (NXTOperator reports 'Done')

//  internal architecture - main thread doing the work using state machine
//                          extra thread reading from standard input
//  states:
//    1. waiting for NXT power on (active waiting with small sleeps) 
//    2. waiting
//    3. executing command
//    4. quitting

//    1 -> 2 report 'Start' when NXT on and sensor 4 detected on
//    2 -> 1 report 'Stop' when NXT disconnects
//    2 -> 3 command arrived on stdin, start executing the command
//    3 -> 2 NXT reported done using sensor 1/4
//    any -> 4 'Quit' command arrived


namespace Application
{
    public class NXTOperator 
    {

      public static void Main(string[] args)
      {
         NXTOperator op = new NXTOperator();
         op.sensor_loop();
         op.close();
         Environment.Exit(0);
      }

      private const byte WAITING_FOR_NXT_POWER_ON = 0;
      private const byte WAITING = 1;
      private const byte EXECUTING = 2;
      private const byte QUITTING = 3;

      private byte state;
      private NXTLightSensor monitored_sensor;
      private bool connected;

      private Brick<NXTLightSensor,NXTLightSensor,Sensor,NXTLightSensor> brick;
      private NXTLightSensor light1, light2, light4;
      private bool we_run;
      private readonly Object inMove;

      public NXTOperator()
      {
         connected = false;
         state = WAITING_FOR_NXT_POWER_ON;
         inMove = new Object();
         Console.SetIn(new StreamReader(Console.OpenStandardInput()));
         //Console.SetOut(new StreamWriter(Console.OpenStandardOutput()));
         brick = new Brick<NXTLightSensor,NXTLightSensor,Sensor,NXTLightSensor>("usb");

         we_run = true;
         Thread readThread = new Thread(new ThreadStart(read_loop));
         readThread.Start();
     }

     private bool try_connect()
     {
         try {
           brick.Connection.Open();
         } catch (Exception) { return false; }

         configure_sensors();
         run_mikes_program();
         return try_wait_for_sensors_activated(); 
     }

     private void configure_sensors()
     {
         light1 = new NXTLightSensor();
         light2 = new NXTLightSensor();
         light4 = new NXTLightSensor();
         brick.Sensor1 = light1;
         brick.Sensor2 = light2;
         brick.Sensor4 = light4;
     }

     private void run_mikes_program()
     {
         brick.StopProgram();
         Thread.Sleep(1000);
         brick.StartProgram("mikes.rxe", true);
     }

     private bool try_wait_for_sensors_activated()
     {
         int cnt = 0;
         while (light4.ReadLightLevel() == 0)
         {
            Thread.Sleep(300);
            cnt ++;
            if (cnt > 20) 
            { 
               brick.Connection.Close();
               return false;
            }
         } 
         Console.WriteLine("Start");
         connected = true;
         return true;
      }

      private void close()
      {
         brick.Connection.Close();
      }

      private void sensor_loop()
      {
         while (we_run)
         { 
           if (state == WAITING_FOR_NXT_POWER_ON) 
           {
              if (try_connect()) state = WAITING;
              else Thread.Sleep(1000);
           }
           else 
           {
             lock(inMove) Monitor.Wait(inMove);

             while (we_run && (state == EXECUTING))
             {
               if (check_movement_completed()) state = WAITING;
               else Thread.Sleep(10);
             }
           }
         }
      }

      private bool check_movement_completed()
      {
         while (true)
         {
           if (monitored_sensor.ReadLightLevel() == 0) break;
           return false;
         }
         brick.Mailbox.Send("O", Box.Box0);
         Console.WriteLine("Done");
         return true;
      }

      private void read_loop()
      {
         while (we_run)
         {
            String cmd = Console.ReadLine();
            if (cmd.Equals("Quit")) quit();
            else if ((state == WAITING) || (state == EXECUTING))
            {
              if (light2.ReadLightLevel() == 0)
              {
                 connected = false;
                 state = WAITING_FOR_NXT_POWER_ON;
                 Console.WriteLine("Stop");
              }
              else process_command(cmd);
            }
         }
      }

      private void process_command(String cmd)
      {
         if (cmd.Equals("Dock")) line(0);
         else if (cmd.Equals("Sleep")) sleep();
         else if (cmd.Equals("On")) on();
         else if (cmd.Equals("Off")) off();
         else if (cmd.Equals("Line1")) line(1);
         else if (cmd.Equals("Line2")) line(2);
         else if (cmd.Equals("Line3")) line(3);
         else if (cmd.Equals("Slow")) set_slow_speed();
         else if (cmd.Equals("Fast")) set_fast_speed();
         else if (cmd.Equals("Key")) read_key();
         else if (cmd.Equals("Clear")) clear_display();
         else brick.Mailbox.Send(cmd, Box.Box0);
      }

      private void on()
      {
         brick.MotorA.On(100);
         brick.MotorB.On(100);
         brick.MotorC.On(100);
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
      }

      private void line(int ln)
      {
         String cmd = select_sensor_and_command_for_line(ln);
         brick.Mailbox.Send(cmd, Box.Box0);
         state = EXECUTING;
         Thread.Sleep(300);
         lock(inMove) Monitor.Pulse(inMove);
      }

      private String select_sensor_and_command_for_line(int line)
      {
         monitored_sensor = light1;
         if (line == 0)
         {
           monitored_sensor = light4;
           return "A";
         }
         if (line == 1) return "C";
         if (line == 2) return "D";
         if (line == 3)
         {
           monitored_sensor = light4;
           return "B";
         }
         return "*";
      }

      private void set_slow_speed()
      {
         brick.Mailbox.Send("E", Box.Box0);
      }

      private void set_fast_speed()
      {
         brick.Mailbox.Send("F", Box.Box0);
      }

      private void clear_display()
      {
         brick.Mailbox.Send("X", Box.Box0);
      }

      private void read_key()
      {
         brick.Mailbox.Send("M", Box.Box0);
         while ((light1.ReadLightLevel() > 0) && 
                (light2.ReadLightLevel() > 0) && 
                (light4.ReadLightLevel() > 0) && we_run) Thread.Sleep(10);
         if (light1.ReadLightLevel() == 0) Console.WriteLine("Left"); 
         else if (light2.ReadLightLevel() == 0) Console.WriteLine("Right"); 
         else if (light4.ReadLightLevel() == 0) Console.WriteLine("Enter"); 
         brick.Mailbox.Send("O", Box.Box0);
         Thread.Sleep(200);
      }

      private void quit()
      {
         if (connected) off(); 
         state = QUITTING;
         we_run = false;
         if (connected) brick.StopProgram();
         if (state != WAITING_FOR_NXT_POWER_ON) 
           lock(inMove) Monitor.Pulse(inMove);
      }
    }
}

