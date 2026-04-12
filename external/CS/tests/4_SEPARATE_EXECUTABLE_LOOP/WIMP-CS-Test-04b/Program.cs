using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using WimpCS;
using System.Runtime.InteropServices;

namespace WIMP_CS_Test_01
{
    public class ChildProgram
    { 
        public static void Run(string[] args)
        {
            //Default values in case no arguments supplied
            String process_domain = "127.0.0.1";
            Int32 process_port = 8001;
            String master_domain = "127.0.0.1";
            Int32 master_port = 8000;

            //Read the args in the C way
            //We also check i + 1 < argc to ensure next argument was specified
            for (int i = 0; i < args.Length; ++i)
            {
                if (args[i] == "--master-port" && i + 1 < args.Length)
                {
                    master_port = Int32.Parse(args[i+1]);
                }
                else if (args[i] == "--process-port" && i + 1 < args.Length)
                {
                    process_port = Int32.Parse(args[i + 1]);
                }
            }

            Wimp.Init();
            WimpServer.InitLocalServer("client", "127.0.0.1", process_port);

            //Start reciever
            WimpReciever reciever = new WimpReciever("master", "127.0.0.1", master_port);
            reciever.Start();

            //Add process
            WimpServer.AddProcess(reciever, WimpCore.WimpRelation.Parent);

            //Send the instructions

            //Instruction 1 - This sends a simple instr that the master will ignore. It has no additional arguments
            WimpServer.AddInstruction("master", (UInt64)Program.Instructions.BLANK_INSTR, (IntPtr)0, 0);

            //Instruction 2 - This sends a simple instr that tells the master to say hello. It has no additional arguments
            WimpServer.AddInstruction("master", (UInt64)Program.Instructions.SAY_HELLO, (IntPtr)0, 0);

            //Instruction 3 - This sends a more complex instr, that tells the master to echo the string sent.
            String echo_string = "Echo!";
            IntPtr echo_string_marshalled = Marshal.StringToHGlobalAnsi(echo_string);
            WimpServer.AddInstruction("master", (UInt64)Program.Instructions.ECHO, echo_string_marshalled, (UInt64)(echo_string.Length + 1));
            Marshal.FreeHGlobal(echo_string_marshalled);

            //Instruction 4 - This tells the master to exit
            WimpServer.AddInstruction("master", (UInt64)WimpCore.WIMPInstructionsCore.EXIT, (IntPtr)0, 0);

            WimpServer.SendInstructions();
            Thread.Sleep(1000);

            WimpServer.CloseLocalServer();
            Wimp.Shutdown();
        }
    }

    class Program
    {
        public enum Instructions : UInt64
        {
            BLANK_INSTR,
            SAY_HELLO,
            ECHO
        }

        /// 
        /// Ensures the console prints in color
        /// 
        const int STD_OUTPUT_HANDLE = -11;
        const uint ENABLE_VIRTUAL_TERMINAL_PROCESSING = 4;

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern IntPtr GetStdHandle(int nStdHandle);

        [DllImport("kernel32.dll")]
        static extern bool GetConsoleMode(IntPtr hConsoleHandle, out uint lpMode);

        [DllImport("kernel32.dll")]
        static extern bool SetConsoleMode(IntPtr hConsoleHandle, uint dwMode);
        static void Main(string[] args)
        {

            var handle = GetStdHandle(STD_OUTPUT_HANDLE);
            uint mode;
            GetConsoleMode(handle, out mode);
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(handle, mode);

            ChildProgram.Run(args);
        }
    }
}
