using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using WimpCS;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace WIMP_CS_Test_01
{
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
            WimpProcess childProcess = new WimpProcess();
            childProcess.Start("client", childArgs, "WIMP-CS-Test-04b");

            //Start reciever
            WimpReciever reciever = new WimpReciever("client", "127.0.0.1", Int32.Parse(childPort));
            reciever.Start();

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
