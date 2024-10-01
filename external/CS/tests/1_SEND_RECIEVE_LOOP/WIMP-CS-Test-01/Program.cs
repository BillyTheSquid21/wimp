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
            reciever.Start("client", "127.0.0.1", process_port);

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

            //Instruction 4 - This simple tells the master to exit
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

        public struct PassMat
        {
            public PassMat(String name)
            {
                StepName = name;
                Status = false;
            }

            public static implicit operator PassMat(string name)
            {
                return new PassMat() { StepName = name, Status = false };
            }

            public String StepName;
            public bool Status;
        }

        public static PassMat[] Steps = {
            "PROCESS VALIDATION",
            "INSTRUCTION 1",
            "INSTRUCTION 2",
            "INSTRUCTION 3",
            "INSTRUCTION 4" 
        };

        public enum TEST_ENUMS
        {
            STEP_PROCESS_VALIDATION = 0,
            STEP_INSTRUCTION_1 = 1,
            STEP_INSTRUCTION_2 = 2,
            STEP_INSTRUCTION_3 = 3,
            STEP_INSTRUCTION_4 = 4,
        };

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

            //Init WIMP and the local server
            Wimp.Init();
            WimpServer.InitLocalServer("master", "127.0.0.1");

            //Assign child port
            String childPort = WimpServer.GetUnusedLocalPort().ToString();
            String parentPort = WimpServer.Port().ToString();

            string[] childArgs = { "--master-port", parentPort, "--process-port", childPort, };

            Thread childThread = new Thread(() => ChildProgram.Run(childArgs));
            childThread.Start();

            //Start reciever
            WimpReciever reciever = new WimpReciever("client", "127.0.0.1", Int32.Parse(childPort));
            reciever.Start("master", "127.0.0.1", Int32.Parse(parentPort));

            //Add process
            WimpServer.AddProcess(reciever, WimpCore.WimpRelation.Child);

            if (WimpServer.CheckProcessListening("client"))
            {
                WimpLog.Success("Process validated!\n");
                Steps[(int)TEST_ENUMS.STEP_PROCESS_VALIDATION].Status = true;
            }

            //Loop
            bool disconnect = false;
            while (!disconnect)
            {
                WimpServer.Lock();
                WimpInstructionNode currentnode = WimpServer.NextInstruction();
                while(!currentnode.IsNull())
                {
                    WimpInstructionNode.WimpInstrMeta meta = currentnode.GetMeta();
                    switch (meta.Instruction())
                    {
                        case (UInt64)Instructions.BLANK_INSTR:
                            WimpLog.Log("\n");
                            Steps[(int)TEST_ENUMS.STEP_INSTRUCTION_1].Status = true;
                            break;
                        case (UInt64)Instructions.SAY_HELLO:
                            WimpLog.Log("HELLO!\n");
                            Steps[(int)TEST_ENUMS.STEP_INSTRUCTION_2].Status = true;
                            break;
                        case (UInt64)Instructions.ECHO:
                            //Get the arguments
                            String echo_str = Marshal.PtrToStringAnsi(meta.Arguments());
                            WimpLog.Log(echo_str);

                            if (echo_str == "Echo!")
                            {
                                Steps[(int)TEST_ENUMS.STEP_INSTRUCTION_3].Status = true;
                            }
                            break;
                        case (UInt64)WimpCore.WIMPInstructionsCore.EXIT:
                            WimpLog.Log("\n");
                            Steps[(int)TEST_ENUMS.STEP_INSTRUCTION_4].Status = true;
                            disconnect = true;
                            break;
                        case (UInt64)WimpCore.WIMPInstructionsCore.LOG:
                            WimpLog.Log(Marshal.PtrToStringAnsi(meta.Arguments()));
                            break;
                        default:
                            break;
                    }

                    currentnode = WimpServer.NextInstruction();
                }
                WimpServer.Unlock();
            }

            Thread.Sleep(1000);
            WimpServer.CloseLocalServer();
            Wimp.Shutdown();

            bool passed = true;
            foreach(var step in Steps)
            {
                Console.WriteLine(step.StepName + ": " + step.Status.ToString());
                passed &= step.Status;
            }

            if (passed)
            {
                Console.WriteLine("Test passed!");
            }
            else
            {
                Console.WriteLine("Test failed!");
            }
            Thread.Sleep(5000);
        }
    }
}
